/**************************************************************************************************
  Filename:       zcl_sampledoorlockcontroller.c
  Revised:        $Date: 2014-10-24 16:04:46 -0700 (Fri, 24 Oct 2014) $
  Revision:       $Revision: 40796 $

  Description:    Zigbee Cluster Library - sample device application.


  Copyright 2013 Texas Instruments Incorporated. All rights reserved.

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
  This application implements a ZigBee Door Lock Controller, based on Z-Stack 3.0.

  This application is based on the common sample-application user interface. Please see the main
  comment in zcl_sampleapp_ui.c. The rest of this comment describes only the content specific for
  this sample application.

  Application-specific UI peripherals being used:

  - LEDs:
    LED1:
      - Off: Indicates remote door is unlocked.
      - On: Indicates remote door is locked.

  Application-specific menu system:
    <DISCOVER> Sends Identify Query to start discovery mechanism for creating bind to a Door Lock.
      Once the discovery process is done, Door Lock Controller can open/close the Door Lock.

    <CHANGE PIN> Change the PIN used in the Lock/Unlock command
      The user may type the digit using the number keys OR use Up/Down arrow keys to change the
        value of the current digit.

    <LOCK DOOR> Sends the Lock Door command to remote Door Lock device

    <UNLOCK DOOR> Sends the Unlock Door command to remote Door Lock device

    The APP Info line will display the following information:
      [Lock State]
        Locked/Unlocked - current remote door lock state
      [Current PIN]
        XXXX - current PIN used in the Lock/Unlock command
      [Last Command Status]
        SUCCESSFUL/FAILURE/WRONG PIN/UNKNOWN - status of last command sent to remote door lock

*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "rom_jt_154.h"
#include <string.h>

#include "zcomdef.h"

#include "ti_drivers_config.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_closures.h"

#include "zcl_sampledoorlockcontroller.h"
#include "bdb_interface.h"

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

#if defined ( BDB_TL_INITIATOR )
#include "touchlink_initiator_app.h"
#elif defined ( BDB_TL_TARGET )
#include "touchlink_target_app.h"
#endif
#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
#include "gp_common.h"
#endif

/*********************************************************************
 * MACROS
 */

#define UI_CHANGE_PIN          1
#define UI_TOGGLE_LOCK         2
#define UI_DOORLOCK_DISCOVERY  3

#define APP_TITLE " DoorLock Cntrl "

/*********************************************************************
 * CONSTANTS
 */

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
static endPointDesc_t  zclSampleDoorLockControllerEpDesc = {0};

#if ZG_BUILD_ENDDEVICE_TYPE
static ClockP_Handle EndDeviceRejoinClkHandle;
static ClockP_Struct EndDeviceRejoinClkStruct;
#endif

static ClockP_Handle DiscoveryClkHandle;
static ClockP_Struct DiscoveryClkStruct;

// Passed in function pointers to the NV driver
static NVINTF_nvFuncts_t *pfnZdlNV = NULL;

#ifndef CUI_DISABLE
static uint16_t zclSampleDoorLockController_BdbCommissioningModes;
#endif

afAddrType_t zclSampleDoorLockController_DstAddr;

uint8_t giDoorLockPINCount = 0;

uint8_t giDoorLockPINColumnCount = 0;

char aiDoorLockPIN[] = {4, 0x31, 0x32, 0x33, 0x34};   // default PIN (size 4) of 1,2,3,4 (ASCII)

static bool remoteDoorIsLocked = TRUE;
static uint16_t remoteDoorAddr = 0xFFFF;
static ZStatus_t LastCmdStatus = ZInvalidParameter;   //ZInvalidParameter, ZSuccess, ZFailed

static uint8_t endPointDiscovered = 0x00;

//Discovery in progress state. This last 3 seconds to get the responses.
static uint8_t discoveryInprogress = 0x00;
#define DISCOVERY_IN_PROGRESS_TIMEOUT   3000

static LED_Handle gRedLedHandle;

#ifndef CUI_DISABLE
CONST char zclSampleDoorLockController_appStr[] = APP_TITLE_STR;
CUI_clientHandle_t gCuiHandle;
static uint32_t gSampleDoorLockControllerInfoLine1;
static uint32_t gSampleDoorLockControllerInfoLine2;
#endif

CONST char EnterPin[] = "Enter PIN:";

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void zclSampleDoorLockController_initialization(void);
static void zclSampleDoorLockController_process_loop(void);
static void zclSampleDoorLockController_initParameters(void);
static void zclSampleDoorLockController_processZStackMsgs(zstackmsg_genericReq_t *pMsg);
static void SetupZStackCallbacks(void);
static void zclSampleDoorLockController_processAfIncomingMsgInd(zstack_afIncomingMsgInd_t *pInMsg);
static void zclSampleDoorLockController_initializeClocks(void);
#if ZG_BUILD_ENDDEVICE_TYPE
static void zclSampleDoorLockController_processEndDeviceRejoinTimeoutCallback(UArg a0);
#endif
static void zclSampleDoorLockController_processDiscoveryTimeoutCallback(UArg a0);

#ifndef CUI_DISABLE
static void zclSampleDoorLockController_processKey(uint8_t key, Button_EventMask buttonEvents);
static void zclSampleDoorLockController_RemoveAppNvmData(void);
static void zclSampleDoorLockController_InitializeStatusLine(CUI_clientHandle_t gCuiHandle);
static void zclSampleDoorLockController_UpdateStatusLine(void);
#endif


static void zclSampleDoorLockController_Init( void );
static void zclSampleDoorLockController_BasicResetCB( void );
static void zclSampleDoorLockController_IdentifyQueryRspCB(zclIdentifyQueryRsp_t *pRsp);
static void zclSampleDoorLockController_ProcessCommissioningStatus(bdbCommissioningModeMsg_t* bdbCommissioningModeMsg);



// Functions to process ZCL Foundation incoming Command/Response messages
static uint8_t zclSampleDoorLockController_ProcessIncomingMsg( zclIncoming_t *msg );
#if (! defined ZCL_REPORT_DESTINATION_DEVICE)
#ifdef ZCL_READ
static uint8_t zclSampleDoorLockController_ProcessInReadRspCmd( zclIncoming_t *pInMsg );
#endif
#endif
static uint8_t zclSampleDoorLockController_ProcessInDefaultRspCmd( zclIncoming_t *pInMsg );
#ifdef ZCL_REPORT_DESTINATION_DEVICE
static void zclSampleDoorLockController_ProcessInReportCmd( zclIncoming_t *pInMsg );
#endif
#ifdef ZCL_DISCOVER
static uint8_t zclSampleDoorLockController_ProcessInDiscAttrsRspCmd( zclIncoming_t *pInMsg );
#endif

static ZStatus_t zclSampleDoorLockController_DoorLockRspCB ( zclIncoming_t *pInMsg, uint8_t status );
void zclSampleDoorLockController_UpdateLedState(void);



/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclSampleDoorLockController_CmdCallbacks =
{
  zclSampleDoorLockController_BasicResetCB,           // Basic Cluster Reset command
  NULL,                                               // Identfiy cmd
  NULL,                                               // Identify Query command
  zclSampleDoorLockController_IdentifyQueryRspCB,     // Identify Query Response command
  NULL,                                               // Identify Trigger Effect command
#ifdef ZCL_ON_OFF
  NULL,                                               // On/Off cluster commands
  NULL,                                               // On/Off cluster enhanced command Off with Effect
  NULL,                                               // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                               // On/Off cluster enhanced command On with Timed Off
#endif
#ifdef ZCL_LEVEL_CTRL
  NULL,                                               // Level Control Move to Level command
  NULL,                                               // Level Control Move command
  NULL,                                               // Level Control Step command
  NULL,                                               // Level Control Stop command
  NULL,                                               // Level Control Move to Closest Frequency command
#endif
#ifdef ZCL_GROUPS
  NULL,                                               // Group Response commands
#endif
#ifdef ZCL_SCENES
  NULL,                                               // Scene Store Request command
  NULL,                                               // Scene Recall Request command
  NULL,                                               // Scene Response command
#endif
#ifdef ZCL_ALARMS
  NULL,                                               // Alarm (Response) commands
#endif
#ifdef SE_UK_EXT
  NULL,                                               // Get Event Log command
  NULL,                                               // Publish Event Log command
#endif
  NULL,                                               // RSSI Location command
  NULL                                                // RSSI Location Response command
};


/*********************************************************************
 * ZCL Closure cluster Callback table
 */
static zclClosures_DoorLockAppCallbacks_t zclSampleDoorLockController_DoorLockCmdCallbacks =
{
  NULL,                                                  // DoorLock cluster command
  zclSampleDoorLockController_DoorLockRspCB,             // DoorLock Response
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
  zclSampleDoorLockController_initialization();

  // No return from task process
  zclSampleDoorLockController_process_loop();
}



/*******************************************************************************
 * @fn          zclSampleDoorLockController_initialization
 *
 * @brief       Initialize the application
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleDoorLockController_initialization(void)
{
    /* Initialize user clocks */
    zclSampleDoorLockController_initializeClocks();

    /* create semaphores for messages / events
     */
    Semaphore_Params semParam;
    Semaphore_Params_init(&semParam);
    semParam.mode = ti_sysbios_knl_Semaphore_Mode_COUNTING;
    Semaphore_construct(&appSem, 0, &semParam);
    appSemHandle = Semaphore_handle(&appSem);

    appServiceTaskId = OsalPort_registerTask(Task_self(), appSemHandle, &appServiceTaskEvents);

    //Initialize stack
    zclSampleDoorLockController_Init();
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
    zdoCBReq.has_matchDescRsp = true;
    zdoCBReq.matchDescRsp = true;
    zdoCBReq.has_ieeeAddrRsp = true;
    zdoCBReq.ieeeAddrRsp = true;

    (void)Zstackapi_DevZDOCBReq(appServiceTaskId, &zdoCBReq);
}



/*********************************************************************
 * @fn          zclSampleDoorLockController_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleDoorLockController_Init( void )
{

  // Set destination address to indirect
  zclSampleDoorLockController_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
  zclSampleDoorLockController_DstAddr.endPoint = 0;
  zclSampleDoorLockController_DstAddr.addr.shortAddr = 0;

  //Register Endpoint
  zclSampleDoorLockControllerEpDesc.endPoint = SAMPLEDOORLOCKCONTROLLER_ENDPOINT;
  zclSampleDoorLockControllerEpDesc.simpleDesc = &zclSampleDoorLockController_SimpleDesc;
  zclport_registerEndpoint(appServiceTaskId, &zclSampleDoorLockControllerEpDesc);


  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, &zclSampleDoorLockController_CmdCallbacks );

  //Register the ZCL DoorLock Cluster Library callback function
  zclClosures_RegisterDoorLockCmdCallbacks( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, &zclSampleDoorLockController_DoorLockCmdCallbacks );

  // Register the application's attribute list and reset to default values
  zclSampleDoorLockController_ResetAttributesToDefaultValues();
  zcl_registerAttrList( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, zclSampleDoorLockController_NumAttributes, zclSampleDoorLockController_Attrs );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zclport_registerZclHandleExternal(SAMPLEDOORLOCKCONTROLLER_ENDPOINT, zclSampleDoorLockController_ProcessIncomingMsg);

  //Write the bdb initialization parameters
  zclSampleDoorLockController_initParameters();

  //Setup ZDO callbacks
  SetupZStackCallbacks();

#if defined ( BDB_TL_INITIATOR )
  zclSampleDoorLockController_BdbCommissioningModes |= BDB_COMMISSIONING_MODE_INITIATOR_TL;
#endif

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
                                   aiDoorLockPIN);
    }

    // If the PIN doesn't exist in NV, create it
    if((nvErr == NVINTF_NOTFOUND) && pfnZdlNV->createItem)
    {
        pfnZdlNV->createItem(nvId, DLSAPP_NV_DOORLOCK_PIN_LEN,
                             aiDoorLockPIN);
    }
  }

#ifndef CUI_DISABLE
  // set up default application BDB commissioning modes based on build type
  if(ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE)
  {
    zclSampleDoorLockController_BdbCommissioningModes = BDB_COMMISSIONING_MODE_NWK_FORMATION | BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_FINDING_BINDING;
  }
  else if (ZG_BUILD_JOINING_TYPE && ZG_DEVICE_JOINING_TYPE)
  {
    zclSampleDoorLockController_BdbCommissioningModes = BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_FINDING_BINDING;
  }


  gCuiHandle = UI_Init( appServiceTaskId,                     // Application Task ID
            &appServiceTaskEvents,                // The events processed by the sample application
            appSemHandle,                         // Semaphore to post the events in the application thread
            &zclSampleDoorLockController_IdentifyTime,
            &zclSampleDoorLockController_BdbCommissioningModes,   // A pointer to the application's bdbCommissioningModes
            zclSampleDoorLockController_appStr,                   // A pointer to the app-specific name string
            zclSampleDoorLockController_processKey,               // A pointer to the app-specific key process function
            zclSampleDoorLockController_RemoveAppNvmData          // A pointer to the app-specific NV Item reset function
            );

  //Request the Red LED for App
  LED_Params ledParams;
  LED_Params_init(&ledParams);
  gRedLedHandle = LED_open(CONFIG_LED_RED, &ledParams);

  //Initialize the SampleDoorLock UI status line
  zclSampleDoorLockController_InitializeStatusLine(gCuiHandle);
#endif


#if defined ( BDB_TL_INITIATOR )
    touchLinkInitiatorApp_Init(appServiceTaskId);
#elif defined ( BDB_TL_TARGET )
    touchLinkTargetApp_Init(appServiceTaskId);
#endif

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
  app_Green_Power_Init(appServiceTaskId, &appServiceTaskEvents, appSemHandle, SAMPLEAPP_PROCESS_GP_DATA_SEND_EVT,
                       SAMPLEAPP_PROCESS_GP_EXPIRE_DUPLICATE_EVT, SAMPLEAPP_PROCESS_GP_TEMP_MASTER_EVT);
#endif

  // Call BDB initialization. Should be called once from application at startup to restore
  // previous network configuration, if applicable.
  zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;
  zstack_bdbStartCommissioningReq.commissioning_mode = 0;
  Zstackapi_bdbStartCommissioningReq(appServiceTaskId,&zstack_bdbStartCommissioningReq);
}

#ifndef CUI_DISABLE
/*********************************************************************
 * @fn          zclSampleDoorLockController_RemoveAppNvmData
 *
 * @brief       Callback when Application performs reset to Factory New Reset.
 *              Application must restore the application to default values
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleDoorLockController_RemoveAppNvmData(void)
{
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

static void zclSampleDoorLockController_initParameters(void)
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
 * @fn      zclSampleDoorLockController_initializeClocks
 *
 * @brief   Initialize Clocks
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleDoorLockController_initializeClocks(void)
{
#if ZG_BUILD_ENDDEVICE_TYPE
    // Initialize the timers needed for this application
    EndDeviceRejoinClkHandle = UtilTimer_construct(
    &EndDeviceRejoinClkStruct,
    zclSampleDoorLockController_processEndDeviceRejoinTimeoutCallback,
    SAMPLEAPP_END_DEVICE_REJOIN_DELAY,
    0, false, 0);
#endif

    // Initialize the timers needed for this application
    DiscoveryClkHandle = UtilTimer_construct(
    &DiscoveryClkStruct,
    zclSampleDoorLockController_processDiscoveryTimeoutCallback,
    DISCOVERY_IN_PROGRESS_TIMEOUT,
    0, false, 0);
}

#if ZG_BUILD_ENDDEVICE_TYPE
/*******************************************************************************
 * @fn      zclSampleDoorLockController_processEndDeviceRejoinTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleDoorLockController_processEndDeviceRejoinTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    appServiceTaskEvents |= SAMPLEAPP_END_DEVICE_REJOIN_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}
#endif

/*******************************************************************************
 * @fn      zclSampleDoorLockController_processDiscoveryTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleDoorLockController_processDiscoveryTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    appServiceTaskEvents |= SAMPLEAPP_DISCOVERY_TIMEOUT_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}


/*******************************************************************************
 * @fn      zclSampleDoorLockController_process_loop
 *
 * @brief   Application task processing start.
 *
 * @param   none
 *
 * @return  void
 */
static void zclSampleDoorLockController_process_loop(void)
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
                zclSampleDoorLockController_processZStackMsgs(pMsg);
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
            if ( appServiceTaskEvents & SAMPLEAPP_DISCOVERY_TIMEOUT_EVT )
            {
              discoveryInprogress = FALSE;

              appServiceTaskEvents &= ~SAMPLEAPP_DISCOVERY_TIMEOUT_EVT;
            }



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
 * @fn      zclSampleDoorLockController_processZStackMsgs
 *
 * @brief   Process event from Stack
 *
 * @param   pMsg - pointer to incoming ZStack message to process
 *
 * @return  void
 */
static void zclSampleDoorLockController_processZStackMsgs(zstackmsg_genericReq_t *pMsg)
{
      switch(pMsg->hdr.event)
      {
          case zstackmsg_CmdIDs_BDB_NOTIFICATION:
              {
                  zstackmsg_bdbNotificationInd_t *pInd;
                  pInd = (zstackmsg_bdbNotificationInd_t*)pMsg;
                  zclSampleDoorLockController_ProcessCommissioningStatus(&(pInd->Req));
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
                  zclSampleDoorLockController_processAfIncomingMsgInd( &(pInd->req) );
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
#ifndef CUI_DISABLE
          {
            // The ZStack Thread is indicating a State change
            zstackmsg_devStateChangeInd_t *pInd =
                (zstackmsg_devStateChangeInd_t *)pMsg;
                  UI_DeviceStateUpdated(&(pInd->req));
          }
#endif
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
#ifndef CUI_DISABLE
          {
            zstackmsg_gpCommissioningModeInd_t *pInd;
            pInd = (zstackmsg_gpCommissioningModeInd_t*)pMsg;
            UI_SetGPPCommissioningMode( &(pInd->Req) );
          }
#endif
          break;

#endif

          case zstackmsg_CmdIDs_ZDO_MATCH_DESC_RSP:
          {
              zstackmsg_zdoMatchDescRspInd_t *pInd =
                      (zstackmsg_zdoMatchDescRspInd_t *)pMsg;
              if((pInd->rsp.status == zstack_ZdpStatus_SUCCESS) && (pInd->rsp.n_matchList))
              {
                  zstack_zdoIeeeAddrReq_t Req;

                  while((*(pInd->rsp.pMatchList) != endPointDiscovered) && (pInd->rsp.pMatchList != NULL))
                  {
                      pInd->rsp.pMatchList++;
                  }

                  if(pInd->rsp.pMatchList != NULL)
                  {
                      Req.startIndex = 0;
                      Req.type = zstack_NwkAddrReqType_SINGLE_DEVICE;
                      Req.nwkAddr = pInd->rsp.nwkAddrOfInterest;

                      Zstackapi_ZdoIeeeAddrReq(appServiceTaskId, &Req);
                  }
                  else
                  {
                      endPointDiscovered = 0;
                  }
              }

          }
          break;

          case zstackmsg_CmdIDs_ZDO_IEEE_ADDR_RSP:
          {
              zstackmsg_zdoIeeeAddrRspInd_t *pInd =
                      (zstackmsg_zdoIeeeAddrRspInd_t *)pMsg;

              if(pInd->rsp.status == zstack_ZdpStatus_SUCCESS)
              {
                  zstack_zdoBindReq_t Req;
                  zstack_sysNwkInfoReadRsp_t  Rsp;

                  //Get our IEEE address
                  Zstackapi_sysNwkInfoReadReq(appServiceTaskId, &Rsp);


                  //fill in bind request to self
                  Req.nwkAddr = Rsp.nwkAddr;
                  OsalPort_memcpy(Req.bindInfo.srcAddr, Rsp.ieeeAddr, Z_EXTADDR_LEN);
                  Req.bindInfo.dstAddr.addrMode = zstack_AFAddrMode_EXT;
                  OsalPort_memcpy(Req.bindInfo.dstAddr.addr.extAddr, pInd->rsp.ieeeAddr, Z_EXTADDR_LEN);
                  Req.bindInfo.dstAddr.endpoint = endPointDiscovered;
                  Req.bindInfo.clusterID = ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK;
                  Req.bindInfo.srcEndpoint = SAMPLEDOORLOCKCONTROLLER_ENDPOINT;

                  //create the bind to that device
                  Zstackapi_ZdoBindReq(appServiceTaskId, &Req);
              }

          }
          break;


#ifdef BDB_TL_TARGET
          case zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_ENABLE_IND:
          {
            zstackmsg_bdbTouchLinkTargetEnableInd_t *pInd =
              (zstackmsg_bdbTouchLinkTargetEnableInd_t*)pMsg;

            uiProcessTouchlinkTargetEnable(pInd->Enable);
          }
          break;
#endif
          case zstackmsg_CmdIDs_BDB_TC_LINK_KEY_EXCHANGE_NOTIFICATION_IND:
          case zstackmsg_CmdIDs_AF_DATA_CONFIRM_IND:
          case zstackmsg_CmdIDs_ZDO_DEVICE_ANNOUNCE:
          case zstackmsg_CmdIDs_ZDO_NWK_ADDR_RSP:
          case zstackmsg_CmdIDs_DEV_PERMIT_JOIN_IND:
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
 * @fn          zclSampleDoorLockController_processAfIncomingMsgInd
 *
 * @brief       Process AF Incoming Message Indication message
 *
 * @param       pInMsg - pointer to incoming message
 *
 * @return      none
 *
 */
static void zclSampleDoorLockController_processAfIncomingMsgInd(zstack_afIncomingMsgInd_t *pInMsg)
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
 * @fn      zclSampleDoorLockController_ProcessCommissioningStatus
 *
 * @brief   Callback in which the status of the commissioning process are reported
 *
 * @param   bdbCommissioningModeMsg - Context message of the status of a commissioning process
 *
 * @return  none
 */
static void zclSampleDoorLockController_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg)
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
 * @fn      zclSampleDoorLockController_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to default values.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleDoorLockController_BasicResetCB( void )
{
  //Reset every attribute in all supported cluster to their default value.

  zclSampleDoorLockController_ResetAttributesToDefaultValues();

  // update the display
#ifndef CUI_DISABLE
  zclSampleDoorLockController_UpdateStatusLine();
#endif
}


/*********************************************************************
 * @fn      zclSampleDoorLockController_IdentifyQueryRspCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to process a Indentify Query Rsp.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleDoorLockController_IdentifyQueryRspCB(zclIdentifyQueryRsp_t *pRsp)
{
    zstack_zdoMatchDescReq_t Req;
    uint16_t  DoorlockCluster = ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK;

    if(discoveryInprogress)
    {
        Req.dstAddr = pRsp->srcAddr->addr.shortAddr;
        Req.nwkAddrOfInterest = pRsp->srcAddr->addr.shortAddr;
        Req.profileID = ZCL_HA_PROFILE_ID;

        endPointDiscovered = pRsp->srcAddr->endPoint;

        Req.n_inputClusters = 1;
        Req.n_outputClusters = 0;

        Req.pInputClusters = &DoorlockCluster;

        Zstackapi_ZdoMatchDescReq(appServiceTaskId, &Req);
    }
}


/*********************************************************************
 * @fn      zclSampleDoorLockController_DoorLockRspCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Door Lock response for this application.
 *
 * @param   pInMsg - process incoming message
 * @param   status - response status of requesting door lock cmd
 *
 * @return  status
 */
static ZStatus_t zclSampleDoorLockController_DoorLockRspCB ( zclIncoming_t *pInMsg, uint8_t status )
{
  // door lock: lock/unlock response
  if ( status == ZSuccess )
  {
    // if we're using BDB reporting on the doorlock, we don't need to send read attribute
    // because the doorlock should send a report after its own state changes
#ifndef ZCL_REPORT_DESTINATION_DEVICE
    zclReadCmd_t readCmd;

    readCmd.numAttr = 1;
    readCmd.attrID[0] = ATTRID_DOOR_LOCK_LOCK_STATE;

    pInMsg->hdr.transSeqNum += 1;
    zcl_SendRead( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr,
                  ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK, &readCmd,
                  ZCL_FRAME_CLIENT_SERVER_DIR, TRUE, pInMsg->hdr.transSeqNum );
#endif
  }

#ifndef CUI_DISABLE
  LastCmdStatus = status;
  //Update status line
  zclSampleDoorLockController_UpdateStatusLine();
#endif
  return ( ZCL_STATUS_SUCCESS );
}


/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/


/*********************************************************************
 * @fn      zclSampleDoorLockController_ProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  uint8_t - TRUE if got handled
 */
static uint8_t zclSampleDoorLockController_ProcessIncomingMsg( zclIncoming_t *pInMsg)
{
  uint8_t handled = FALSE;
  switch ( pInMsg->hdr.commandID)
  {
#if (! defined ZCL_REPORT_DESTINATION_DEVICE)
#ifdef ZCL_READ
    case ZCL_CMD_READ_RSP:
      zclSampleDoorLockController_ProcessInReadRspCmd( pInMsg );
      handled = TRUE;
      break;
#endif
#endif
#ifdef ZCL_REPORT
    // See ZCL Test Applicaiton (zcl_testapp.c) for sample code on Attribute Reporting
    case ZCL_CMD_CONFIG_REPORT:
      //zclSampleDoorLockController_ProcessInConfigReportCmd( pInMsg );
      break;

    case ZCL_CMD_CONFIG_REPORT_RSP:
      //zclSampleDoorLockController_ProcessInConfigReportRspCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG:
      //zclSampleDoorLockController_ProcessInReadReportCfgCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG_RSP:
      //zclSampleDoorLockController_ProcessInReadReportCfgRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_REPORT_DESTINATION_DEVICE
    case ZCL_CMD_REPORT:
      zclSampleDoorLockController_ProcessInReportCmd( pInMsg );
      handled = TRUE;
      break;
#endif
#ifdef ZCL_DISCOVER
    case ZCL_CMD_DISCOVER_ATTRS_RSP:
      zclSampleDoorLockController_ProcessInDiscAttrsRspCmd( pInMsg );
      handled = TRUE;
      break;
#endif
    case ZCL_CMD_DEFAULT_RSP:
      zclSampleDoorLockController_ProcessInDefaultRspCmd( pInMsg );
      handled = TRUE;
      break;

    default:
      break;
  }

return handled;
}

#ifdef ZCL_REPORT_DESTINATION_DEVICE
static void zclSampleDoorLockController_ProcessInReportCmd( zclIncoming_t *pInMsg )
{
  zclReportCmd_t *pInDoorLockReport;

  pInDoorLockReport = (zclReportCmd_t *)pInMsg->attrCmd;

  if ( pInDoorLockReport->attrList[0].attrID != ATTRID_DOOR_LOCK_LOCK_STATE )
  {
    return;
  }

  // read the lock state and display the information
  if ( pInDoorLockReport->attrList[0].attrData[0] == CLOSURES_LOCK_STATE_LOCKED )
  {
    // Locked
    remoteDoorIsLocked = TRUE;
  }
  else if ( pInDoorLockReport->attrList[0].attrData[0] == CLOSURES_LOCK_STATE_UNLOCKED )
  {
    // Unlocked
    remoteDoorIsLocked = FALSE;
  }
  remoteDoorAddr = pInMsg->msg->srcAddr.addr.shortAddr;


#ifndef CUI_DISABLE
  //Update status line
  zclSampleDoorLockController_UpdateStatusLine();
#endif
}
#endif // ZCL_REPORT_DESTINATION_DEVICE

#if (! defined ZCL_REPORT_DESTINATION_DEVICE)
#ifdef ZCL_READ
/*********************************************************************
 * @fn      zclSampleDoorLockController_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleDoorLockController_ProcessInReadRspCmd( zclIncoming_t *pInMsg )
{
  zclReadRspCmd_t *readRspCmd;

  readRspCmd = (zclReadRspCmd_t *)pInMsg->attrCmd;

  // read the lock state and display the information
  // (door lock: read attribute response)
  if ( readRspCmd->attrList[0].data[0] == 1 )
  {
    // Locked
    remoteDoorIsLocked = TRUE;
  }
  else if ( readRspCmd->attrList[0].data[0] == 2 )
  {
    // Unlocked
    remoteDoorIsLocked = FALSE;
  }
  remoteDoorAddr = pInMsg->msg->srcAddr.addr.shortAddr;

#ifndef CUI_DISABLE
  //Update status line
  zclSampleDoorLockController_UpdateStatusLine();
#endif

  return TRUE;
}
#endif // ZCL_READ
#endif // (! defined ZCL_REPORT_DESTINATION_DEVICE)


#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      zclSampleDoorLockController_ProcessInDiscAttrsRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleDoorLockController_ProcessInDiscAttrsRspCmd( zclIncoming_t *pInMsg )
{
  zclDiscoverAttrsRspCmd_t *discoverRspCmd;
  uint8_t i;

  discoverRspCmd = (zclDiscoverAttrsRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return TRUE;
}
#endif // ZCL_DISCOVER

/*********************************************************************
 * @fn      zclSampleDoorLockController_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleDoorLockController_ProcessInDefaultRspCmd( zclIncoming_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;

  // Device is notified of the Default Response command.
  (void)pInMsg;

  return TRUE;
}




void zclSampleDoorLockController_UiActionDoorLockDiscovery(const int32_t _itemEntry)
{

    afAddrType_t destAddr;
    zstack_getZCLFrameCounterRsp_t Rsp;
    discoveryInprogress = TRUE;


    UtilTimer_setTimeout( DiscoveryClkHandle, DISCOVERY_IN_PROGRESS_TIMEOUT );
    UtilTimer_start(&DiscoveryClkStruct);

    destAddr.endPoint = 0xFF;
    destAddr.addrMode = afAddr16Bit;
    destAddr.addr.shortAddr = 0xFFFF;

    Zstackapi_getZCLFrameCounterReq(appServiceTaskId, &Rsp);

    zclGeneral_SendIdentifyQuery(SAMPLEDOORLOCKCONTROLLER_ENDPOINT, &destAddr, FALSE, Rsp.zclFrameCounter);

}




void zclSampleDoorLockController_UpdateLedState(void)
{
  if ( remoteDoorIsLocked )
  {
    LED_stopBlinking(gRedLedHandle);
    LED_setOn(gRedLedHandle, LED_BRIGHTNESS_MAX);
  }
  else
  {
    LED_stopBlinking(gRedLedHandle);
    LED_setOff(gRedLedHandle);
  }
}

#ifndef CUI_DISABLE
void zclSampleDoorLockController_UiActionLock(const int32_t _itemEntry)
{
    zclDoorLock_t cmd;
    zstack_getZCLFrameCounterRsp_t Rsp;

    Zstackapi_getZCLFrameCounterReq(appServiceTaskId, &Rsp);
    cmd.pPinRfidCode = (uint8_t *)aiDoorLockPIN;

    zclClosures_SendDoorLockLockDoor( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, &zclSampleDoorLockController_DstAddr, &cmd, TRUE, Rsp.zclFrameCounter );

    LastCmdStatus = ZInvalidParameter;
    zclSampleDoorLockController_UpdateStatusLine();
}


void zclSampleDoorLockController_UiActionUnlock(const int32_t _itemEntry)
{
    zclDoorLock_t cmd;
    zstack_getZCLFrameCounterRsp_t Rsp;

    Zstackapi_getZCLFrameCounterReq(appServiceTaskId, &Rsp);
    cmd.pPinRfidCode = (uint8_t *)aiDoorLockPIN;

    zclClosures_SendDoorLockUnlockDoor( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, &zclSampleDoorLockController_DstAddr, &cmd, TRUE, Rsp.zclFrameCounter );
    LastCmdStatus = ZInvalidParameter;
    zclSampleDoorLockController_UpdateStatusLine();
}


/****************************************************************************
****************************************************************************/

static void zclSampleDoorLockController_InitializeStatusLine(CUI_clientHandle_t gCuiHandle)
{
    /* Request Async Line for Light application Info */
    CUI_statusLineResourceRequest(gCuiHandle, "   APP Info"CUI_DEBUG_MSG_START"1"CUI_DEBUG_MSG_END, false, &gSampleDoorLockControllerInfoLine1);
    CUI_statusLineResourceRequest(gCuiHandle, "   APP Info"CUI_DEBUG_MSG_START"2"CUI_DEBUG_MSG_END, false, &gSampleDoorLockControllerInfoLine2);

    zclSampleDoorLockController_UpdateStatusLine();
}


static void zclSampleDoorLockController_UpdateStatusLine(void)
{
    uint8_t strLen = 0;
    char lineFormat1[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};
    char lineFormat2[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};

    strcpy(lineFormat1, "["CUI_COLOR_YELLOW"Current PIN"CUI_COLOR_RESET"] ");
    //Length of the string before adding the PIN
    strLen = strlen(lineFormat1);

    //copy the PIN
    memcpy(&lineFormat1[strlen(lineFormat1)],&aiDoorLockPIN[1],aiDoorLockPIN[0]);

    //Add the end of string character after the PIN
    lineFormat1[strLen + aiDoorLockPIN[0]] = '\0';


    strcpy(&lineFormat1[strlen(lineFormat1)], " ["CUI_COLOR_YELLOW"Remote Lock State"CUI_COLOR_RESET"] 0x%04x is ");
    if(remoteDoorIsLocked)
    {
        strcpy(&lineFormat1[strlen(lineFormat1)], CUI_COLOR_RED"Locked"CUI_COLOR_RESET);
    }
    else
    {
        strcpy(&lineFormat1[strlen(lineFormat1)], CUI_COLOR_GREEN"Unlocked"CUI_COLOR_RESET);
    }

    strcpy(&lineFormat2[strlen(lineFormat2)], "["CUI_COLOR_YELLOW"Last Command Status"CUI_COLOR_RESET"] ");
    if(LastCmdStatus == ZInvalidParameter)
    {
        strcpy(&lineFormat2[strlen(lineFormat2)], "UNKNOWN");
    }
    else if(LastCmdStatus == ZSuccess)
    {
        strcpy(&lineFormat2[strlen(lineFormat2)], "SUCCESSFUL");
    }
    else if(LastCmdStatus == ZFailure)
    {
        strcpy(&lineFormat2[strlen(lineFormat2)], "FAILURE");
    }
    //Wrong PIN
    else
    {
        strcpy(&lineFormat2[strlen(lineFormat2)], "WRONG PIN");
    }

    zclSampleDoorLockController_UpdateLedState();

    CUI_statusLinePrintf(gCuiHandle, gSampleDoorLockControllerInfoLine1, lineFormat1, remoteDoorAddr);
    CUI_statusLinePrintf(gCuiHandle, gSampleDoorLockControllerInfoLine2, lineFormat2);
}

void zclSampleDoorLockController_UiActionEnterPin(const char _input, char* _lines[3], CUI_cursorInfo_t * _curInfo)
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
            aiDoorLockPIN[index]++;
            if(aiDoorLockPIN[index] > '9')
            {
                aiDoorLockPIN[index] = '0';
            }
        break;
        case CUI_INPUT_DOWN:
            aiDoorLockPIN[index]--;
            if(aiDoorLockPIN[index] < '0')
            {
                aiDoorLockPIN[index] = '9';
            }
        break;
        case CUI_INPUT_RIGHT:
            if(index < aiDoorLockPIN[0])
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
        {
            zclSampleDoorLockController_UpdateStatusLine();
            // store PIN to NVM
            if(pfnZdlNV && pfnZdlNV->writeItem)
            {
                NVINTF_itemID_t nvId;

                nvId.systemID = NVINTF_SYSID_APP;
                nvId.itemID = (uint16_t)DLSAPP_NV_DOORLOCK_PIN;
                nvId.subID = (uint16_t)0;

                pfnZdlNV->writeItem(nvId, DLSAPP_NV_DOORLOCK_PIN_LEN,
                                      aiDoorLockPIN);
            }
        }
        break;
        default:
        {
            // is input valid?
            if(CUI_IS_INPUT_NUM(_input))
            {
                aiDoorLockPIN[index] = _input;

                if(index < aiDoorLockPIN[0])
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
    strncat(_lines[1],&aiDoorLockPIN[1],aiDoorLockPIN[0]);

    strcpy(_lines[2], "   CHANGE PIN   ");

}


/*********************************************************************
 * @fn      zclSampleDoorLockController_processKey
 *
 * @brief   Key event handler function
 *
 * @param   key - key to handle action for
 *          buttonEvents - event to handle action for
 *
 * @return  none
 */
static void zclSampleDoorLockController_processKey(uint8_t key, Button_EventMask buttonEvents)
{
    if (buttonEvents & Button_EV_CLICKED)
    {
        if(key == CONFIG_BTN_LEFT)
        {
            zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;

            zstack_bdbStartCommissioningReq.commissioning_mode = zclSampleDoorLockController_BdbCommissioningModes;
            Zstackapi_bdbStartCommissioningReq(appServiceTaskId, &zstack_bdbStartCommissioningReq);
        }
        if(key == CONFIG_BTN_RIGHT)
        {
            zclDoorLock_t cmd;
            cmd.pPinRfidCode = (uint8_t *)aiDoorLockPIN;
            zstack_getZCLFrameCounterRsp_t Rsp;

            Zstackapi_getZCLFrameCounterReq(appServiceTaskId, &Rsp);

            if(remoteDoorIsLocked)
            {
              zclClosures_SendDoorLockUnlockDoor( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, &zclSampleDoorLockController_DstAddr, &cmd, TRUE, Rsp.zclFrameCounter );
            }
            else
            {
              zclClosures_SendDoorLockLockDoor( SAMPLEDOORLOCKCONTROLLER_ENDPOINT, &zclSampleDoorLockController_DstAddr, &cmd, TRUE, Rsp.zclFrameCounter );
            }
        }
    }
}
#endif // CUI_DISABLE


