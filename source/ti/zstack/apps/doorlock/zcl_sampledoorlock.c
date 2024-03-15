/**************************************************************************************************
  Filename:       zcl_sampledoorlock.c
  Revised:        $Date: 2014-10-24 16:04:46 -0700 (Fri, 24 Oct 2014) $
  Revision:       $Revision: 40796 $


  Description:    Zigbee Cluster Library - sample device application.


  Copyright 2013-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
  This application implements a ZigBee Door Lock, based on Z-Stack 3.0.

  This application is based on the common sample-application user interface. Please see the main
  comment in zcl_sampleapp_ui.c. The rest of this comment describes only the content specific for
  this sample application.

  Application-specific UI peripherals being used:

  - LEDs:
    LED1:
      - Off: Indicates door is unlocked.
      - On: Indicates door is locked.

  Application-specific menu system:
    <DISCOVER> Sets Door Lock device into Identify mode. Once the discovery process is done,
      Door Lock Controller can open/close the Door Lock.

    <CHANGE PIN> Change the PIN used in the Lock/Unlock command
      The user may type the digit using the number keys OR use Up/Down arrow keys to change the
        value of the current digit.

    <LOCK DOOR> Sends the Lock Door command

    <UNLOCK DOOR> Sends the Unlock Door command

    The APP Info line will display the following information:
      [Lock State]
        Locked/Unlocked/Not fully locked - current local lock state
      [Current PIN]
        XXXX - current PIN used in the Lock/Unlock command

*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "rom_jt_154.h"
#include "zcomdef.h"
#include <string.h>

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "bdb_interface.h"
#ifdef BDB_REPORTING
#include "bdb_reporting.h"
#endif
#include "zcl_closures.h"

#include "zcl_sampledoorlock.h"
#include "ti_drivers_config.h"

#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>

#ifndef CUI_DISABLE
#include "zcl_sampleapps_ui.h"
#include "zcl_sample_app_def.h"
#endif

#include "nvintf.h"
#include "zstackmsg.h"
#include "zcl_port.h"

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>
#include "zstackapi.h"
#include "util_timer.h"
#include "mac_util.h"

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
#include "gp_common.h"
#endif

#if defined ( BDB_TL_INITIATOR )
#include "touchlink_initiator_app.h"
#elif defined ( BDB_TL_TARGET )
#include "touchlink_target_app.h"
#endif

/*********************************************************************
 * MACROS
 */

#define UI_CHANGE_PIN            1
#define UI_TOGGLE_LOCK           2
#define UI_DOORLOCK_DISCOVERABLE 3

#define APP_TITLE "    DoorLock    "

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
// Semaphore used to post events to the application thread
static Semaphore_Handle appSemHandle;
static Semaphore_Struct appSem;

/* App service ID used for messaging with stack service task */
static uint8_t  appServiceTaskId;
/* App service task events, set by the stack service task when sending a message */
static uint32_t appServiceTaskEvents;
static endPointDesc_t  zclSampleDoorLockEpDesc = {0};

#if ZG_BUILD_ENDDEVICE_TYPE
static ClockP_Handle EndDeviceRejoinClkHandle;
static ClockP_Struct EndDeviceRejoinClkStruct;
#endif

// Passed in function pointers to the NV driver
static NVINTF_nvFuncts_t *pfnZdlNV = NULL;


#ifndef CUI_DISABLE
static uint16_t zclSampleDoorLock_BdbCommissioningModes;
#endif

#ifdef BDB_REPORTING
#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 8
  static uint8_t reportableChange[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif
#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 4
  static uint8_t reportableChange[] = {0x01, 0x00, 0x00, 0x00};
#endif
#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 2
  static uint8_t reportableChange[] = {0x01, 0x00};
#endif
#endif

// Master PIN code for DoorLock
static char aiDoorLockMasterPINCode[] = {4,0x31,0x32,0x33,0x34};

static LED_Handle gRedLedHandle;

#ifndef CUI_DISABLE
CONST char zclSampleDoorLock_appStr[] = APP_TITLE_STR;
CUI_clientHandle_t gCuiHandle;
static uint32_t gSampleDoorLockInfoLine;
#endif

CONST char EnterPin[] = "Enter PIN:";

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void zclSampleDoorLock_initialization(void);
static void zclSampleDoorLock_process_loop(void);
static void zclSampleDoorLock_initParameters(void);
static void zclSampleDoorLock_processZStackMsgs(zstackmsg_genericReq_t *pMsg);
static void SetupZStackCallbacks(void);
static void zclSampleDoorLock_processAfIncomingMsgInd(zstack_afIncomingMsgInd_t *pInMsg);
static void zclSampleDoorLock_initializeClocks(void);
#if ZG_BUILD_ENDDEVICE_TYPE
static void zclSampleDoorLock_processEndDeviceRejoinTimeoutCallback(UArg a0);
#endif

static void zclSampleDoorLock_Init( void );



static void zclSampleDoorLock_BasicResetCB( void );
#ifndef CUI_DISABLE
static void zclSampleDoorLock_processKey(uint8_t key, Button_EventMask buttonEvents);
static void zclSampleDoorLock_RemoveAppNvmData(void);
static void zclSampleDoorLock_InitializeStatusLine(CUI_clientHandle_t gCuiHandle);
static void zclSampleDoorLock_UpdateStatusLine(void);
#endif
static uint8_t zclSampleDoorLock_SceneStoreCB(zclSceneReq_t *pReq);
static void zclSampleDoorLock_SceneRecallCB(zclSceneReq_t *pReq);

static void zclSampleDoorLock_ProcessCommissioningStatus(bdbCommissioningModeMsg_t* bdbCommissioningModeMsg);

// Functions to process ZCL Foundation incoming Command/Response messages
static ZStatus_t zclSampleDoorLock_DoorLockCB ( zclIncoming_t *pInMsg, zclDoorLock_t *pInCmd );
static ZStatus_t zclSampleDoorLock_DoorLockActuator ( uint8_t newDoorLockState );


static void zclSampleDoorLock_UpdateLedState(void);

static ZStatus_t zclSampleDoorLock_ReadWriteAttrCB( uint16_t clusterId, uint16_t attrId, uint8_t oper,
                                          uint8_t *pValue, uint16_t *pLen );


/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclSampleDoorLock_CmdCallbacks =
{
  zclSampleDoorLock_BasicResetCB,         // Basic Cluster Reset command
  NULL,                                   // Identify command
  NULL,                                   // Identify Query command
  NULL,                                   // Identify Query Response command
  NULL,                                   // Identify Trigger Effect command
#ifdef ZCL_ON_OFF
  NULL,                                   // On/Off cluster commands
  NULL,                                   // On/Off cluster enhanced command Off with Effect
  NULL,                                   // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                   // On/Off cluster enhanced command On with Timed Off
#endif
#ifdef ZCL_LEVEL_CTRL
  NULL,                                               // Level Control Move to Level command
  NULL,                                               // Level Control Move command
  NULL,                                               // Level Control Step command
  NULL,                                               // Level Control Stop command
  NULL,                                               // Level Control Move to Closest Frequency command
#endif
#ifdef ZCL_GROUPS
  NULL,                                   // Group Response commands
#endif
#ifdef ZCL_SCENES
  zclSampleDoorLock_SceneStoreCB,         // Scene Store Request command
  zclSampleDoorLock_SceneRecallCB,        // Scene Recall Request command
  NULL,                                   // Scene Response command
#endif
#if ZCL_ALARMS
  NULL,                                   // Alarm (Response) commands
#endif
#ifdef SE_UK_EXT
  NULL,                                   // Get Event Log command
  NULL,                                   // Publish Event Log command
#endif
  NULL,                                   // RSSI Location command
  NULL                                    // RSSI Location Response command
};


/*********************************************************************
 * ZCL Closure cluster Callback table
 */
static zclClosures_DoorLockAppCallbacks_t zclSampleDoorLock_DoorLockCmdCallbacks =
{
  zclSampleDoorLock_DoorLockCB,                           // DoorLock cluster command
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

/*********************************************************************
 * CONSTANTS
 */


/*******************************************************************************
 * @fn          sampleApp_task
 *
 * @brief       Application task entry point for the Z-Stack
 *              Sample Application
 *
 * @param       pfnNV - pointer to the NV functions
 *
 * @return      none
 */
void sampleApp_task(NVINTF_nvFuncts_t *pfnNV)
{
  // Save and register the function pointers to the NV drivers
  pfnZdlNV = pfnNV;
  zclport_registerNV(pfnZdlNV, ZCL_PORT_SCENE_TABLE_NV_ID);

  // Initialize application
  zclSampleDoorLock_initialization();

  // No return from task process
  zclSampleDoorLock_process_loop();
}



/*******************************************************************************
 * @fn          zclSampleDoorLock_initialization
 *
 * @brief       Initialize the application
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleDoorLock_initialization(void)
{
    /* Initialize user clocks */
    zclSampleDoorLock_initializeClocks();

    /* create semaphores for messages / events
     */
    Semaphore_Params semParam;
    Semaphore_Params_init(&semParam);
    semParam.mode = ti_sysbios_knl_Semaphore_Mode_COUNTING;
    Semaphore_construct(&appSem, 0, &semParam);
    appSemHandle = Semaphore_handle(&appSem);

    appServiceTaskId = OsalPort_registerTask(Task_self(), appSemHandle, &appServiceTaskEvents);


    //Initialize stack
    zclSampleDoorLock_Init();
}



/*******************************************************************************
 * @fn      SetupZStackCallbacks
 *
 * @brief   Setup the Zstack Callbacks wanted
 *
 * @param   none
 *
 * @return  none
 */
static void SetupZStackCallbacks(void)
{
    zstack_devZDOCBReq_t zdoCBReq = {0};

    // Register for Callbacks, turn on:
    //  Device State Change,
    //  ZDO Match Descriptor Response,
    zdoCBReq.has_devStateChange = true;
    zdoCBReq.devStateChange = true;

    (void)Zstackapi_DevZDOCBReq(appServiceTaskId, &zdoCBReq);
}



/*********************************************************************
 * @fn          zclSampleDoorLock_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleDoorLock_Init( void )
{

#ifdef BDB_REPORTING
      zstack_bdbRepAddAttrCfgRecordDefaultToListReq_t Req = {0};
#endif

  //Register Endpoint
  zclSampleDoorLockEpDesc.endPoint = SAMPLEDOORLOCK_ENDPOINT;
  zclSampleDoorLockEpDesc.simpleDesc = &zclSampleDoorLock_SimpleDesc;
  zclport_registerEndpoint(appServiceTaskId, &zclSampleDoorLockEpDesc);


  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( SAMPLEDOORLOCK_ENDPOINT, &zclSampleDoorLock_CmdCallbacks );

  //Register the ZCL DoorLock Cluster Library callback function
  zclClosures_RegisterDoorLockCmdCallbacks( SAMPLEDOORLOCK_ENDPOINT, &zclSampleDoorLock_DoorLockCmdCallbacks );

  // Register the application's attribute list and reset to default values
  zclSampleDoorLock_ResetAttributesToDefaultValues();
  zcl_registerAttrList( SAMPLEDOORLOCK_ENDPOINT, zclSampleDoorLock_NumAttributes, zclSampleDoorLock_Attrs );

  zcl_registerReadWriteCB(SAMPLEDOORLOCK_ENDPOINT,zclSampleDoorLock_ReadWriteAttrCB,NULL);

  //Write the bdb initialization parameters
  zclSampleDoorLock_initParameters();

  //Setup ZDO callbacks
  SetupZStackCallbacks();

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
  gp_endpointInit(appServiceTaskId);
#endif

  // Initialize NVM for storing PIN information
  if(pfnZdlNV)
  {
      NVINTF_itemID_t nvId;
      uint32_t nvErr = NVINTF_NOTFOUND;

      // Fill in the NV ID header
      nvId.systemID = NVINTF_SYSID_APP;
      nvId.itemID = (uint16_t)DLSAPP_NV_DOORLOCK_PIN;
      nvId.subID = (uint16_t)0;

      // Read the PIN from NV
      if(pfnZdlNV->readItem)
      {
          nvErr = pfnZdlNV->readItem(nvId, 0, DLSAPP_NV_DOORLOCK_PIN_LEN,
                                     aiDoorLockMasterPINCode);
      }

      // If the PIN doesn't exist in NV, create it
      if((nvErr == NVINTF_NOTFOUND) && pfnZdlNV->createItem)
      {
          pfnZdlNV->createItem(nvId, DLSAPP_NV_DOORLOCK_PIN_LEN,
                               aiDoorLockMasterPINCode);
      }
  }


#ifdef BDB_REPORTING
  //Adds the default configuration values for the doorlock attribute of the ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK cluster, for endpoint SAMPLEDOORLOCK_ENDPOINT
  //Default maxReportingInterval value is 10 seconds
  //Default minReportingInterval value is 0 seconds
  //reportableChange is set to value 1, which would be any change
  Req.attrID = ATTRID_DOOR_LOCK_LOCK_STATE;
  Req.cluster = ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK;
  Req.endpoint = SAMPLEDOORLOCK_ENDPOINT;
  Req.maxReportInt = 10;
  Req.minReportInt = 0;
  //reportableChange is set to value 1, which would be any change
  OsalPort_memcpy(Req.reportableChange,reportableChange,BDBREPORTING_MAX_ANALOG_ATTR_SIZE);

  Zstackapi_bdbRepAddAttrCfgRecordDefaultToListReq(appServiceTaskId,&Req);

#endif


#ifndef CUI_DISABLE
  // set up default application BDB commissioning modes based on build type
  if(ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE)
  {
    zclSampleDoorLock_BdbCommissioningModes = BDB_COMMISSIONING_MODE_NWK_FORMATION | BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_FINDING_BINDING;
  }
  else if (ZG_BUILD_JOINING_TYPE && ZG_DEVICE_JOINING_TYPE)
  {
    zclSampleDoorLock_BdbCommissioningModes = BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_FINDING_BINDING;
  }


  gCuiHandle = UI_Init( appServiceTaskId,                     // Application Task ID
           &appServiceTaskEvents,                // The events processed by the sample application
           appSemHandle,                         // Semaphore to post the events in the application thread
           &zclSampleDoorLock_IdentifyTime,
           &zclSampleDoorLock_BdbCommissioningModes,   // A pointer to the application's bdbCommissioningModes
           zclSampleDoorLock_appStr,                   // A pointer to the app-specific name string
           zclSampleDoorLock_processKey,               // A pointer to the app-specific key process function
           zclSampleDoorLock_RemoveAppNvmData          // A pointer to the app-specific NV Item reset function
           );

  //Request the Red LED for App
  LED_Params ledParams;
  LED_Params_init(&ledParams);
  gRedLedHandle = LED_open(CONFIG_LED_RED, &ledParams);

  //Initialize the SampleDoorLock UI status line
  zclSampleDoorLock_InitializeStatusLine(gCuiHandle);
#endif



#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
  app_Green_Power_Init(appServiceTaskId, &appServiceTaskEvents, appSemHandle, SAMPLEAPP_PROCESS_GP_DATA_SEND_EVT,
                       SAMPLEAPP_PROCESS_GP_EXPIRE_DUPLICATE_EVT, SAMPLEAPP_PROCESS_GP_TEMP_MASTER_EVT);
#endif

#if defined ( BDB_TL_INITIATOR )
    touchLinkInitiatorApp_Init(appServiceTaskId);
#elif defined ( BDB_TL_TARGET )
    touchLinkTargetApp_Init(appServiceTaskId);
#endif

  zclSampleDoorLock_UpdateLedState();

  // Call BDB initialization. Should be called once from application at startup to restore
  // previous network configuration, if applicable.
  zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;
  zstack_bdbStartCommissioningReq.commissioning_mode = 0;
  Zstackapi_bdbStartCommissioningReq(appServiceTaskId,&zstack_bdbStartCommissioningReq);
}

#ifndef CUI_DISABLE
/*********************************************************************
 * @fn          zclSampleDoorLock_RemoveAppNvmData
 *
 * @brief       Callback when Application performs reset to Factory New Reset.
 *              Application must restore the application to default values
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleDoorLock_RemoveAppNvmData(void)
{
#ifdef ZCL_GROUPS
    uint8_t numGroups;
    uint16_t groupList[APS_MAX_GROUPS];
    uint8_t i;

    numGroups = aps_FindAllGroupsForEndpoint( SAMPLEDOORLOCK_ENDPOINT, groupList );
#if defined ( ZCL_SCENES )
    for ( i = 0; i < numGroups; i++ )
    {
      zclGeneral_RemoveAllScenes( SAMPLEDOORLOCK_ENDPOINT, groupList[i] );
    }
#endif
    if (numGroups > 0)
    {
      aps_RemoveAllGroup( SAMPLEDOORLOCK_ENDPOINT );
    }
#endif
      NVINTF_itemID_t nvId;

      nvId.systemID = NVINTF_SYSID_APP;
      nvId.itemID = (uint16_t)DLSAPP_NV_DOORLOCK_PIN;
      nvId.subID = (uint16_t)0;

      // Read the PIN from NV
      if(pfnZdlNV->deleteItem)
      {
          pfnZdlNV->deleteItem(nvId);
      }

}
#endif

static void zclSampleDoorLock_initParameters(void)
{
    zstack_bdbSetAttributesReq_t zstack_bdbSetAttrReq;

    zstack_bdbSetAttrReq.bdbCommissioningGroupID              = BDB_DEFAULT_COMMISSIONING_GROUP_ID;
    zstack_bdbSetAttrReq.bdbPrimaryChannelSet                 = BDB_DEFAULT_PRIMARY_CHANNEL_SET;
    zstack_bdbSetAttrReq.bdbScanDuration                      = BDB_DEFAULT_SCAN_DURATION;
    zstack_bdbSetAttrReq.bdbSecondaryChannelSet               = BDB_DEFAULT_SECONDARY_CHANNEL_SET;
    zstack_bdbSetAttrReq.has_bdbCommissioningGroupID          = TRUE;
    zstack_bdbSetAttrReq.has_bdbPrimaryChannelSet             = TRUE;
    zstack_bdbSetAttrReq.has_bdbScanDuration                  = TRUE;
    zstack_bdbSetAttrReq.has_bdbSecondaryChannelSet           = TRUE;
#if (ZG_BUILD_COORDINATOR_TYPE)
    zstack_bdbSetAttrReq.has_bdbJoinUsesInstallCodeKey        = TRUE;
    zstack_bdbSetAttrReq.has_bdbTrustCenterNodeJoinTimeout    = TRUE;
    zstack_bdbSetAttrReq.has_bdbTrustCenterRequireKeyExchange = TRUE;
    zstack_bdbSetAttrReq.bdbJoinUsesInstallCodeKey            = BDB_DEFAULT_JOIN_USES_INSTALL_CODE_KEY;
    zstack_bdbSetAttrReq.bdbTrustCenterNodeJoinTimeout        = BDB_DEFAULT_TC_NODE_JOIN_TIMEOUT;
    zstack_bdbSetAttrReq.bdbTrustCenterRequireKeyExchange     = BDB_DEFAULT_TC_REQUIRE_KEY_EXCHANGE;
#endif
#if (ZG_BUILD_JOINING_TYPE)
    zstack_bdbSetAttrReq.has_bdbTCLinkKeyExchangeAttemptsMax  = TRUE;
    zstack_bdbSetAttrReq.has_bdbTCLinkKeyExchangeMethod       = TRUE;
    zstack_bdbSetAttrReq.bdbTCLinkKeyExchangeAttemptsMax      = BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_ATTEMPS_MAX;
    zstack_bdbSetAttrReq.bdbTCLinkKeyExchangeMethod           = BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_METHOD;
#endif

    Zstackapi_bdbSetAttributesReq(appServiceTaskId, &zstack_bdbSetAttrReq);
}

/*******************************************************************************
 * @fn      zclSampleDoorLock_initializeClocks
 *
 * @brief   Initialize Clocks
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleDoorLock_initializeClocks(void)
{
#if ZG_BUILD_ENDDEVICE_TYPE
    // Initialize the timers needed for this application
    EndDeviceRejoinClkHandle = UtilTimer_construct(
    &EndDeviceRejoinClkStruct,
    zclSampleDoorLock_processEndDeviceRejoinTimeoutCallback,
    SAMPLEAPP_END_DEVICE_REJOIN_DELAY,
    0, false, 0);
#endif

}

#if ZG_BUILD_ENDDEVICE_TYPE
/*******************************************************************************
 * @fn      zclSampleDoorLock_processEndDeviceRejoinTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleDoorLock_processEndDeviceRejoinTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    appServiceTaskEvents |= SAMPLEAPP_END_DEVICE_REJOIN_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}
#endif


/*******************************************************************************
 * @fn      zclSampleDoorLock_process_loop
 *
 * @brief   Application task processing start.
 *
 * @param   none
 *
 * @return  void
 */
static void zclSampleDoorLock_process_loop(void)
{
    /* Forever loop */
    for(;;)
    {
        zstackmsg_genericReq_t *pMsg = NULL;
        bool msgProcessed = FALSE;

        /* Wait for response message */
        if(Semaphore_pend(appSemHandle, BIOS_WAIT_FOREVER ))
        {
            /* Retrieve the response message */
            if( (pMsg = (zstackmsg_genericReq_t*) OsalPort_msgReceive( appServiceTaskId )) != NULL)
            {
                /* Process the message from the stack */
                zclSampleDoorLock_processZStackMsgs(pMsg);
                // Free any separately allocated memory
                msgProcessed = Zstackapi_freeIndMsg(pMsg);
            }

            if((msgProcessed == FALSE) && (pMsg != NULL))
            {
                OsalPort_msgDeallocate((uint8_t*)pMsg);
            }

#ifndef CUI_DISABLE
            //Process the events that the UI may have
            zclsampleApp_ui_event_loop();
#endif


#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
            if(appServiceTaskEvents & SAMPLEAPP_PROCESS_GP_DATA_SEND_EVT)
            {
                if(zgGP_ProxyCommissioningMode == TRUE)
                {
                  zcl_gpSendCommissioningNotification();
                }
                else
                {
                  zcl_gpSendNotification();
                }
                appServiceTaskEvents &= ~SAMPLEAPP_PROCESS_GP_DATA_SEND_EVT;
            }

            if(appServiceTaskEvents & SAMPLEAPP_PROCESS_GP_EXPIRE_DUPLICATE_EVT)
            {
                gp_expireDuplicateFiltering();
                appServiceTaskEvents &= ~SAMPLEAPP_PROCESS_GP_EXPIRE_DUPLICATE_EVT;
            }

            if(appServiceTaskEvents & SAMPLEAPP_PROCESS_GP_TEMP_MASTER_EVT)
            {
                gp_returnOperationalChannel();
                appServiceTaskEvents &= ~SAMPLEAPP_PROCESS_GP_TEMP_MASTER_EVT;
            }
#endif

#if ZG_BUILD_ENDDEVICE_TYPE
            if ( appServiceTaskEvents & SAMPLEAPP_END_DEVICE_REJOIN_EVT )
            {
              zstack_bdbRecoverNwkRsp_t zstack_bdbRecoverNwkRsp;

              Zstackapi_bdbRecoverNwkReq(appServiceTaskId,&zstack_bdbRecoverNwkRsp);

              appServiceTaskEvents &= ~SAMPLEAPP_END_DEVICE_REJOIN_EVT;
            }
#endif


        }
    }
}




/*******************************************************************************
 * @fn      zclSampleDoorLock_processZStackMsgs
 *
 * @brief   Process event from Stack
 *
 * @param   pMsg - pointer to incoming ZStack message to process
 *
 * @return  void
 */
static void zclSampleDoorLock_processZStackMsgs(zstackmsg_genericReq_t *pMsg)
{
      switch(pMsg->hdr.event)
      {
          case zstackmsg_CmdIDs_BDB_NOTIFICATION:
              {
                  zstackmsg_bdbNotificationInd_t *pInd;
                  pInd = (zstackmsg_bdbNotificationInd_t*)pMsg;
                  zclSampleDoorLock_ProcessCommissioningStatus(&(pInd->Req));
              }
              break;

          case zstackmsg_CmdIDs_BDB_IDENTIFY_TIME_CB:
              {
#ifndef CUI_DISABLE
                zstackmsg_bdbIdentifyTimeoutInd_t *pInd;
                pInd = (zstackmsg_bdbIdentifyTimeoutInd_t*) pMsg;
                uiProcessIdentifyTimeChange(&(pInd->EndPoint));
#endif
              }
              break;

          case zstackmsg_CmdIDs_BDB_BIND_NOTIFICATION_CB:
              {
#ifndef CUI_DISABLE
                zstackmsg_bdbBindNotificationInd_t *pInd;
                pInd = (zstackmsg_bdbBindNotificationInd_t*) pMsg;
                uiProcessBindNotification(&(pInd->Req));
#endif
              }
              break;

          case zstackmsg_CmdIDs_AF_INCOMING_MSG_IND:
              {
                  // Process incoming data messages
                  zstackmsg_afIncomingMsgInd_t *pInd;
                  pInd = (zstackmsg_afIncomingMsgInd_t *)pMsg;
                  zclSampleDoorLock_processAfIncomingMsgInd( &(pInd->req) );
              }
              break;




#if (ZG_BUILD_JOINING_TYPE)
          case zstackmsg_CmdIDs_BDB_CBKE_TC_LINK_KEY_EXCHANGE_IND:
          {
            zstack_bdbCBKETCLinkKeyExchangeAttemptReq_t zstack_bdbCBKETCLinkKeyExchangeAttemptReq;
            /* Z3.0 has not defined CBKE yet, so lets attempt default TC Link Key exchange procedure
             * by reporting CBKE failure.
             */

            zstack_bdbCBKETCLinkKeyExchangeAttemptReq.didSuccess = FALSE;

            Zstackapi_bdbCBKETCLinkKeyExchangeAttemptReq(appServiceTaskId,
                                                         &zstack_bdbCBKETCLinkKeyExchangeAttemptReq);
          }
          break;

          case zstackmsg_CmdIDs_BDB_FILTER_NWK_DESCRIPTOR_IND:

           /*   User logic to remove networks that do not want to join
            *   Networks to be removed can be released with Zstackapi_bdbNwkDescFreeReq
            */

            Zstackapi_bdbFilterNwkDescComplete(appServiceTaskId);
          break;

#endif
          case zstackmsg_CmdIDs_DEV_STATE_CHANGE_IND:
          {
#ifndef CUI_DISABLE
            // The ZStack Thread is indicating a State change
            zstackmsg_devStateChangeInd_t *pInd =
                (zstackmsg_devStateChangeInd_t *)pMsg;
                  UI_DeviceStateUpdated(&(pInd->req));
#endif
          }
          break;



          /*
           * These are messages/indications from ZStack that this
           * application doesn't process.  These message can be
           * processed by your application, remove from this list and
           * process them here in this switch statement.
           */

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
          case zstackmsg_CmdIDs_GP_DATA_IND:
          {
              zstackmsg_gpDataInd_t *pInd;
              pInd = (zstackmsg_gpDataInd_t*)pMsg;
              gp_processDataIndMsg( &(pInd->Req) );
          }
          break;

          case zstackmsg_CmdIDs_GP_SECURITY_REQ:
          {
              zstackmsg_gpSecReq_t *pInd;
              pInd = (zstackmsg_gpSecReq_t*)pMsg;
              gp_processSecRecMsg( &(pInd->Req) );
          }
          break;

          case zstackmsg_CmdIDs_GP_CHECK_ANNCE:
          {
              zstackmsg_gpCheckAnnounce_t *pInd;
              pInd = (zstackmsg_gpCheckAnnounce_t*)pMsg;
              gp_processCheckAnnceMsg( &(pInd->Req) );
          }
          break;

          case zstackmsg_CmdIDs_GP_COMMISSIONING_MODE_IND:
          {
#ifndef CUI_DISABLE
            zstackmsg_gpCommissioningModeInd_t *pInd;
            pInd = (zstackmsg_gpCommissioningModeInd_t*)pMsg;
            UI_SetGPPCommissioningMode( &(pInd->Req) );
#endif
          }
          break;
#endif

#ifdef BDB_TL_TARGET
          case zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_ENABLE_IND:
          {
            zstackmsg_bdbTouchLinkTargetEnableInd_t *pInd =
              (zstackmsg_bdbTouchLinkTargetEnableInd_t*)pMsg;

            uiProcessTouchlinkTargetEnable(pInd->Enable);
          }
          break;
#endif

          case zstackmsg_CmdIDs_DEV_PERMIT_JOIN_IND:
          case zstackmsg_CmdIDs_BDB_TC_LINK_KEY_EXCHANGE_NOTIFICATION_IND:
          case zstackmsg_CmdIDs_AF_DATA_CONFIRM_IND:
          case zstackmsg_CmdIDs_ZDO_DEVICE_ANNOUNCE:
          case zstackmsg_CmdIDs_ZDO_NWK_ADDR_RSP:
          case zstackmsg_CmdIDs_ZDO_IEEE_ADDR_RSP:
          case zstackmsg_CmdIDs_ZDO_NODE_DESC_RSP:
          case zstackmsg_CmdIDs_ZDO_POWER_DESC_RSP:
          case zstackmsg_CmdIDs_ZDO_SIMPLE_DESC_RSP:
          case zstackmsg_CmdIDs_ZDO_ACTIVE_EP_RSP:
          case zstackmsg_CmdIDs_ZDO_COMPLEX_DESC_RSP:
          case zstackmsg_CmdIDs_ZDO_USER_DESC_RSP:
          case zstackmsg_CmdIDs_ZDO_USER_DESC_SET_RSP:
          case zstackmsg_CmdIDs_ZDO_SERVER_DISC_RSP:
          case zstackmsg_CmdIDs_ZDO_END_DEVICE_BIND_RSP:
          case zstackmsg_CmdIDs_ZDO_BIND_RSP:
          case zstackmsg_CmdIDs_ZDO_UNBIND_RSP:
          case zstackmsg_CmdIDs_ZDO_MGMT_NWK_DISC_RSP:
          case zstackmsg_CmdIDs_ZDO_MGMT_LQI_RSP:
          case zstackmsg_CmdIDs_ZDO_MGMT_RTG_RSP:
          case zstackmsg_CmdIDs_ZDO_MGMT_BIND_RSP:
          case zstackmsg_CmdIDs_ZDO_MGMT_LEAVE_RSP:
          case zstackmsg_CmdIDs_ZDO_MGMT_DIRECT_JOIN_RSP:
          case zstackmsg_CmdIDs_ZDO_MGMT_PERMIT_JOIN_RSP:
          case zstackmsg_CmdIDs_ZDO_MGMT_NWK_UPDATE_NOTIFY:
          case zstackmsg_CmdIDs_ZDO_SRC_RTG_IND:
          case zstackmsg_CmdIDs_ZDO_CONCENTRATOR_IND:
          case zstackmsg_CmdIDs_ZDO_LEAVE_CNF:
          case zstackmsg_CmdIDs_ZDO_LEAVE_IND:
          case zstackmsg_CmdIDs_SYS_RESET_IND:
          case zstackmsg_CmdIDs_AF_REFLECT_ERROR_IND:
          case zstackmsg_CmdIDs_ZDO_TC_DEVICE_IND:
              break;

          default:
              break;
      }

}



/*******************************************************************************
 *
 * @fn          zclSampleDoorLock_processAfIncomingMsgInd
 *
 * @brief       Process AF Incoming Message Indication message
 *
 * @param       pInMsg - pointer to incoming message
 *
 * @return      none
 *
 */
static void zclSampleDoorLock_processAfIncomingMsgInd(zstack_afIncomingMsgInd_t *pInMsg)
{
    afIncomingMSGPacket_t afMsg;

    /*
     * All incoming messages are passed to the ZCL message processor,
     * first convert to a structure that ZCL can process.
     */
    afMsg.groupId = pInMsg->groupID;
    afMsg.clusterId = pInMsg->clusterId;
    afMsg.srcAddr.endPoint = pInMsg->srcAddr.endpoint;
    afMsg.srcAddr.panId = pInMsg->srcAddr.panID;
    afMsg.srcAddr.addrMode = (afAddrMode_t)pInMsg->srcAddr.addrMode;
    if( (afMsg.srcAddr.addrMode == afAddr16Bit)
        || (afMsg.srcAddr.addrMode == afAddrGroup)
        || (afMsg.srcAddr.addrMode == afAddrBroadcast) )
    {
        afMsg.srcAddr.addr.shortAddr = pInMsg->srcAddr.addr.shortAddr;
    }
    else if(afMsg.srcAddr.addrMode == afAddr64Bit)
    {
        OsalPort_memcpy(afMsg.srcAddr.addr.extAddr, &(pInMsg->srcAddr.addr.extAddr), 8);
    }
    afMsg.macDestAddr = pInMsg->macDestAddr;
    afMsg.endPoint = pInMsg->endpoint;
    afMsg.wasBroadcast = pInMsg->wasBroadcast;
    afMsg.LinkQuality = pInMsg->linkQuality;
    afMsg.correlation = pInMsg->correlation;
    afMsg.rssi = pInMsg->rssi;
    afMsg.SecurityUse = pInMsg->securityUse;
    afMsg.timestamp = pInMsg->timestamp;
    afMsg.nwkSeqNum = pInMsg->nwkSeqNum;
    afMsg.macSrcAddr = pInMsg->macSrcAddr;
    afMsg.radius = pInMsg->radius;
    afMsg.cmd.DataLength = pInMsg->n_payload;
    afMsg.cmd.Data = pInMsg->pPayload;

    zcl_ProcessMessageMSG(&afMsg);
}



/*********************************************************************
 * @fn      zclSampleDoorLock_ProcessCommissioningStatus
 *
 * @brief   Callback in which the status of the commissioning process are reported
 *
 * @param   bdbCommissioningModeMsg - Context message of the status of a commissioning process
 *
 * @return  none
 */
static void zclSampleDoorLock_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg)
{
  switch(bdbCommissioningModeMsg->bdbCommissioningMode)
  {
    case BDB_COMMISSIONING_FORMATION:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        //YOUR JOB:
      }
      else
      {
        //Want to try other channels?
        //try with bdb_setChannelAttribute
      }
    break;
    case BDB_COMMISSIONING_NWK_STEERING:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        //YOUR JOB:
        //We are on the nwk, what now?
      }
      else
      {
        //See the possible errors for nwk steering procedure
        //No suitable networks found
        //Want to try other channels?
        //try with bdb_setChannelAttribute
      }
    break;
    case BDB_COMMISSIONING_FINDING_BINDING:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        //YOUR JOB:
      }
      else
      {
        //YOUR JOB:
        //retry?, wait for user interaction?
      }
    break;
    case BDB_COMMISSIONING_INITIALIZATION:
      //Initialization notification can only be successful. Failure on initialization
      //only happens for ZED and is notified as BDB_COMMISSIONING_PARENT_LOST notification

      //YOUR JOB:
      //We are on a network, what now?

    break;
#if ZG_BUILD_ENDDEVICE_TYPE
    case BDB_COMMISSIONING_PARENT_LOST:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_NETWORK_RESTORED)
      {
        //We did recover from losing parent
      }
      else
      {
        //Parent not found, attempt to rejoin again after a fixed delay
        UtilTimer_setTimeout( EndDeviceRejoinClkHandle, SAMPLEAPP_END_DEVICE_REJOIN_DELAY );
        UtilTimer_start(&EndDeviceRejoinClkStruct);
      }
    break;
#endif
  }

#ifndef CUI_DISABLE
  UI_UpdateBdbStatusLine(bdbCommissioningModeMsg);
#endif
}



/*********************************************************************
 * @fn      zclSampleDoorLock_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to default values.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleDoorLock_BasicResetCB( void )
{
  //Reset every attribute in all supported cluster to their default value.

  zclSampleDoorLock_ResetAttributesToDefaultValues();

  zclSampleDoorLock_UpdateLedState();

#ifndef CUI_DISABLE
  zclSampleDoorLock_UpdateStatusLine();
#endif
}


/*********************************************************************
 * @fn      zclSampleDoorLock_SceneRecallCB
 *
 * @brief   Callback from the ZCL Scenes Cluster Library
 *          to recall a set of attributes from a stored scene.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleDoorLock_SceneRecallCB( zclSceneReq_t *pReq )
{
    zclGeneral_Scene_extField_t extField;
    uint8_t *pBuf;
    uint8_t extLen = 0;

    pBuf = pReq->scene->extField;
    extField.AttrLen = pBuf[2];

    while(extLen < ZCL_GENERAL_SCENE_EXT_LEN)
    {
        //Parse ExtField
        extField.ClusterID = BUILD_UINT16(pBuf[0],pBuf[1]);
        extField.AttrLen = pBuf[2];
        extField.AttrBuf = &pBuf[3];

        if(extField.AttrLen == 0xFF || extField.ClusterID == 0xFFFF)
        {
          break;
        }

        //If Lock State then retrieve the attribute
        if(extField.ClusterID == ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK)
        {
          uint8_t tempState = *extField.AttrBuf;
          zclSampleDoorLock_updateLockStateAttribute(tempState);
        }

        //Move to the next extension field (increase pointer by clusterId, Attribute len field, and attribute)
        pBuf += sizeof(uint16_t) + sizeof(uint8_t) + extField.AttrLen;
        extLen += sizeof(uint16_t) + sizeof(uint8_t) + extField.AttrLen;  //increase ExtField
    }

    //Update scene attributes
    zclSampleDoorLock_ScenesValid = TRUE;
    zclSampleDoorLock_ScenesCurrentGroup = pReq->scene->groupID;
    zclSampleDoorLock_ScenesCurrentScene = pReq->scene->ID;

    //update the UI
    zclSampleDoorLock_UpdateLedState();
#ifndef CUI_DISABLE
  //Update status line
  zclSampleDoorLock_UpdateStatusLine();
#endif
}


/*********************************************************************
 * @fn      zclSampleDoorLock_SceneStoreCB
 *
 * @brief   Callback from the ZCL Scenes Cluster Library
 *          to store a set of attributes to a specific scene.
 *
 * @param   none
 *
 * @return  none
 */
static uint8_t zclSampleDoorLock_SceneStoreCB( zclSceneReq_t *pReq )
{
    zclGeneral_Scene_extField_t extField;
    uint8_t *pBuf;
    uint8_t extLen = 0;
    uint8_t sceneChanged = FALSE;

    pBuf = pReq->scene->extField;
    extField.AttrLen = pBuf[2];

    while(extLen < ZCL_GENERAL_SCENE_EXT_LEN)
    {
        //Parse ExtField
        extField.ClusterID = BUILD_UINT16(pBuf[0],pBuf[1]);
        extField.AttrLen = pBuf[2];
        extField.AttrBuf = &pBuf[3];

        if(extField.AttrLen == 0xFF || extField.ClusterID == 0xFFFF)
        {
            break;
        }

        //If Lock State then store attribute
        if(extField.ClusterID == ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK)
        {
            uint8_t tempState = zclSampleDoorLock_getLockStateAttribute();
            if(*extField.AttrBuf != tempState )
            {
                sceneChanged = TRUE;
            }
            *extField.AttrBuf = tempState;
        }

        //Move to the next extension field (increase pointer by clusterId, Attribute len field, and attribute)
        pBuf += sizeof(uint16_t) + sizeof(uint8_t) + extField.AttrLen;
        extLen += sizeof(uint16_t) + sizeof(uint8_t) + extField.AttrLen;  //increase ExtField
    }

    //Update scene attributes
    zclSampleDoorLock_ScenesValid = TRUE;
    zclSampleDoorLock_ScenesCurrentGroup = pReq->scene->groupID;
    zclSampleDoorLock_ScenesCurrentScene = pReq->scene->ID;

    return sceneChanged;
}





/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/


/*********************************************************************
 * @fn      zclSampleDoorLock_ReadWriteAttrCB
 *
 * @brief   Handle Scenes attributes.
 *
 * @param   clusterId - cluster that attribute belongs to
 * @param   attrId - attribute to be read or written
 * @param   oper - ZCL_OPER_LEN, ZCL_OPER_READ, or ZCL_OPER_WRITE
 * @param   pValue - pointer to attribute value, OTA endian
 * @param   pLen - length of attribute value read, native endian
 *
 * @return  status
 */
static ZStatus_t zclSampleDoorLock_ReadWriteAttrCB( uint16_t clusterId, uint16_t attrId, uint8_t oper,
                                         uint8_t *pValue, uint16_t *pLen )
{
    if(clusterId == ZCL_CLUSTER_ID_GENERAL_SCENES)
    {
        if(attrId == ATTRID_SCENES_SCENE_COUNT)
        {
           return zclGeneral_ReadSceneCountCB(clusterId,attrId,oper,pValue,pLen);
        }
    }

    return ZSuccess;
}


/*********************************************************************
 * @fn      zclSampleDoorLock_DoorLockCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Door Lock cluster Command for this application.
 *
 * @param   pInMsg - process incoming message
 * @param   pInCmd - PIN/RFID code of command
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSampleDoorLock_DoorLockCB ( zclIncoming_t *pInMsg, zclDoorLock_t *pInCmd )
{
  ZStatus_t rc = ZInvalidParameter;
  uint8_t newDoorLockState;

  if ( memcmp( aiDoorLockMasterPINCode, pInCmd->pPinRfidCode, 5 ) == SUCCESS )
  {
    // Lock the door
    if ( pInMsg->hdr.commandID == COMMAND_DOOR_LOCK_LOCK_DOOR )
    {
      newDoorLockState = CLOSURES_LOCK_STATE_LOCKED;
    }
    // Unlock the door
    else if ( pInMsg->hdr.commandID == COMMAND_DOOR_LOCK_UNLOCK_DOOR )
    {
      newDoorLockState = CLOSURES_LOCK_STATE_UNLOCKED;
    }
    // Toggle the door
    else if ( pInMsg->hdr.commandID == COMMAND_DOOR_LOCK_TOGGLE )
    {
      if (zclSampleDoorLock_getLockStateAttribute() == CLOSURES_LOCK_STATE_LOCKED)
      {
        newDoorLockState = CLOSURES_LOCK_STATE_UNLOCKED;
      }
      else
      {
        newDoorLockState = CLOSURES_LOCK_STATE_LOCKED;
      }
    }
    else
    {
      return ( ZCL_STATUS_FAILURE );  // invalid command
    }

    rc = zclSampleDoorLock_DoorLockActuator( newDoorLockState );

    zclClosures_SendDoorLockStatusResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                              pInMsg->hdr.commandID,
                                              rc, TRUE, pInMsg->hdr.transSeqNum ); // ZCL_STATUS_SUCCESS and ZCL_STATUS_FAILURE share the values of ZSuccess and ZFailure, respectively.


    return ( ZCL_STATUS_CMD_HAS_RSP );
  }
  else
  {
    // incorrect PIN received
    zclClosures_SendDoorLockStatusResponse( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                                            pInMsg->hdr.commandID,
                                            ZCL_STATUS_INVALID_VALUE, FALSE, pInMsg->hdr.transSeqNum );

    zclSampleDoorLock_UpdateLedState();
#ifndef CUI_DISABLE
  zclSampleDoorLock_UpdateStatusLine();
#endif
    return ( ZCL_STATUS_CMD_HAS_RSP );
  }
}


static ZStatus_t zclSampleDoorLock_DoorLockActuator ( uint8_t newDoorLockState )
{
  if ( newDoorLockState == zclSampleDoorLock_getLockStateAttribute() )
  {
    return ZFailure;
  }

  //Update the doorlock attribute
  zclSampleDoorLock_updateLockStateAttribute(newDoorLockState);

#ifdef BDB_REPORTING
  bdb_RepChangedAttrValue(SAMPLEDOORLOCK_ENDPOINT, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK, ATTRID_DOOR_LOCK_LOCK_STATE);
#endif

  zclSampleDoorLock_UpdateLedState();
#ifndef CUI_DISABLE
  zclSampleDoorLock_UpdateStatusLine();
#endif

  return ZSuccess;
}

void zclSampleDoorLock_UpdateLedState(void)
{
  if ( zclSampleDoorLock_getLockStateAttribute() == CLOSURES_LOCK_STATE_LOCKED )
  {
      LED_stopBlinking(gRedLedHandle);
      LED_setOn(gRedLedHandle, LED_BRIGHTNESS_MAX);
  }
  else if ( zclSampleDoorLock_getLockStateAttribute() == CLOSURES_LOCK_STATE_UNLOCKED )
  {
      LED_stopBlinking(gRedLedHandle);
      LED_setOff(gRedLedHandle);
  }
}

#ifndef CUI_DISABLE
void zclSampleDoorLock_UiActionDoorLockDiscoverable(const int32_t _itemEntry)
{

    zstack_sysNwkInfoReadRsp_t  Rsp;

    zstack_getZCLFrameCounterRsp_t zclCounterRsp;
    afAddrType_t dstAddr;

    Zstackapi_getZCLFrameCounterReq(appServiceTaskId, &zclCounterRsp);

    //Get our short address
    Zstackapi_sysNwkInfoReadReq(appServiceTaskId, &Rsp);

    dstAddr.endPoint = SAMPLEDOORLOCK_ENDPOINT;
    dstAddr.addrMode = afAddr16Bit;
    dstAddr.addr.shortAddr = Rsp.nwkAddr;

    zclGeneral_SendIdentify(SAMPLEDOORLOCK_ENDPOINT, &dstAddr,60, TRUE, zclCounterRsp.zclFrameCounter);
}

void zclSampleDoorLock_UiActionLock(const int32_t _itemEntry)
{
  zclSampleDoorLock_DoorLockActuator(CLOSURES_LOCK_STATE_LOCKED);
}

void zclSampleDoorLock_UiActionUnlock(const int32_t _itemEntry)
{
  zclSampleDoorLock_DoorLockActuator(CLOSURES_LOCK_STATE_UNLOCKED);
}

void zclSampleDoorLock_UiActionEnterPin(const char _input, char* _lines[3], CUI_cursorInfo_t * _curInfo)
{
    static uint8_t index;

    switch (_input)
    {
        case CUI_ITEM_INTERCEPT_START:
        {
            _curInfo->col = 1;
            _curInfo->row = strlen(EnterPin);
            index = 1;
        }
        break;

        case  CUI_ITEM_PREVIEW:
        break;

        case CUI_INPUT_UP:
            aiDoorLockMasterPINCode[index]++;
            if(aiDoorLockMasterPINCode[index] > '9')
            {
                aiDoorLockMasterPINCode[index] = '0';
            }
        break;
        case CUI_INPUT_DOWN:
            aiDoorLockMasterPINCode[index]--;
            if(aiDoorLockMasterPINCode[index] < '0')
            {
                aiDoorLockMasterPINCode[index] = '9';
            }
        break;
        case CUI_INPUT_RIGHT:
            if(index < aiDoorLockMasterPINCode[0])
            {
                index++;
            }
        break;
        case CUI_INPUT_LEFT:
            if(index != 1)
            {
                index--;
            }
        break;

        case CUI_ITEM_INTERCEPT_STOP:
        case CUI_INPUT_ESC:
        case CUI_INPUT_EXECUTE:
            // store PIN to NVM
            if(pfnZdlNV && pfnZdlNV->writeItem)
            {
                NVINTF_itemID_t nvId;

                nvId.systemID = NVINTF_SYSID_APP;
                nvId.itemID = (uint16_t)DLSAPP_NV_DOORLOCK_PIN;
                nvId.subID = (uint16_t)0;

                pfnZdlNV->writeItem(nvId, DLSAPP_NV_DOORLOCK_PIN_LEN,
                                      aiDoorLockMasterPINCode);
            }
            zclSampleDoorLock_UpdateStatusLine();
        break;
        default:
        {
            // is input valid?
            if(CUI_IS_INPUT_NUM(_input))
            {
                aiDoorLockMasterPINCode[index] = _input;

                if(index < aiDoorLockMasterPINCode[0])
                {
                    index++;
                }
            }
            break;
        }
    }

    _curInfo->row = 2;
    _curInfo->col = strlen(EnterPin) + index;
    strcpy(_lines[1],"Enter PIN:");
    strncat(_lines[1],&aiDoorLockMasterPINCode[1],aiDoorLockMasterPINCode[0]);

    strcpy(_lines[2], "   CHANGE PIN   ");

}

/****************************************************************************
****************************************************************************/



// #ifndef CUI_DISABLE
/*********************************************************************
 * @fn      zclSampleDoorLock_processKey
 *
 * @brief   Key event handler function
 *
 * @param   keysPressed - keys to be process in application context
 *
 * @return  none
 */
static void zclSampleDoorLock_processKey(uint8_t key, Button_EventMask buttonEvents)
{
    if (buttonEvents & Button_EV_CLICKED)
    {
        if(key == CONFIG_BTN_LEFT)
        {
            zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;

            zstack_bdbStartCommissioningReq.commissioning_mode = zclSampleDoorLock_BdbCommissioningModes;
            Zstackapi_bdbStartCommissioningReq(appServiceTaskId, &zstack_bdbStartCommissioningReq);
        }
        if(key == CONFIG_BTN_RIGHT)
        {
            //unused
        }
    }
}

static void zclSampleDoorLock_InitializeStatusLine(CUI_clientHandle_t gCuiHandle)
{
    /* Request Async Line for Light application Info */
    CUI_statusLineResourceRequest(gCuiHandle, "   APP Info"CUI_DEBUG_MSG_START"1"CUI_DEBUG_MSG_END, false, &gSampleDoorLockInfoLine);

    zclSampleDoorLock_UpdateStatusLine();
}


static void zclSampleDoorLock_UpdateStatusLine(void)
{
    char lineFormat[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};
    uint8_t strLen = 0;
    uint8_t lockState = zclSampleDoorLock_getLockStateAttribute();

    strcpy(lineFormat, "["CUI_COLOR_YELLOW"Lock State"CUI_COLOR_RESET"] ");

    if(lockState == CLOSURES_LOCK_STATE_LOCKED)
    {
        strcat(lineFormat, CUI_COLOR_RED"Locked"CUI_COLOR_RESET);
    }
    else if (lockState == CLOSURES_LOCK_STATE_UNLOCKED)
    {
        strcat(lineFormat, CUI_COLOR_GREEN"Unlocked"CUI_COLOR_RESET);
    }
    else
    {
        strcat(lineFormat, "Not fully locked");
    }


    strcat(lineFormat, " ["CUI_COLOR_YELLOW"Current PIN"CUI_COLOR_RESET"] ");

    //Length of the string before adding the PIN
    strLen = strlen(lineFormat);

    //copy the PIN
    memcpy(&lineFormat[strlen(lineFormat)],&aiDoorLockMasterPINCode[1],aiDoorLockMasterPINCode[0]);

    //Add the end of string character after the PIN
    lineFormat[strLen + aiDoorLockMasterPINCode[0]] = '\0';

    CUI_statusLinePrintf(gCuiHandle, gSampleDoorLockInfoLine, lineFormat);

}



#endif

