/**************************************************************************************************
  Filename:       zcl_samplecie.c
  Revised:        $Date: 2015-08-19 17:11:00 -0700 (Wed, 19 Aug 2015) $
  Revision:       $Revision: 44460 $

  Description:    Zigbee Cluster Library - sample cie application.


  Copyright 2006-2013 Texas Instruments Incorporated. All rights reserved.

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
  This application implements a Zigbee IAS CIE, based on Z-Stack 3.0.

  This application is based on the common sample-application user interface. Please see the main
  comment in zcl_sampleapp_ui.c. The rest of this comment describes only the content specific for
  this sample application.

  Application-specific UI peripherals being used:
  - LEDs:
    LED1:
      - Off: Indicates No Fire from change notification.
      - On: Indicates Fire from change notification.

  Application-specific menu system:
    <DISCOVER> Sends Identify Query to start discovery mechanism for creating bind to a Zone.

    <SEND SQUAWK> Sends squawk to all binded devices which has Warning cluster.
      This will cause the LED1 of remote devices which support Warning cluster to blink once
        if there is no alarm, or warning in progress.

    <CONFIG DISCOVERY> Enables/disables automatic Service Discovery of newly joined devices.

    The APP Info line will display the following information:
      [Last Event]
        NO EVENT YET - No events have yet been received from remote devices
        Zone added device 0xXXXX - Zone device added a new remote device
        No fire in device 0xXXXX - Received change notification indicating 'No Fire'
        ALARM! in device: 0xXXXX - Received change notification indicating 'Fire'

*********************************************************************/


/*********************************************************************
 * INCLUDES
 */
#include "rom_jt_154.h"
#include "zcomdef.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_ss.h"

#include "zcl_samplecie.h"

#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>

#ifndef CUI_DISABLE
#include "zcl_sampleapps_ui.h"
#include "zcl_sample_app_def.h"
#endif
#include "bdb_interface.h"

#include "nvintf.h"
#include "zstackmsg.h"
#include "zcl_port.h"

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>
#include "zstackapi.h"
#include "util_timer.h"

#include "ti_drivers_config.h"


#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
#include "gp_common.h"
#endif

/*********************************************************************
 * MACROS
 */

#define NO_EVENT               0
#define ZONE_ADDED             1
#define NOTIFICATION_ALARMED   2
#define NOTIFICATION_NO_FIRE   3

#define NUM_IAS_ZONE_SERVERS       10
#define NUM_DISCOVERABLE_DEVICES   10

/*********************************************************************
 * TYPEDEFS
 */

typedef struct _zoneDevice_t
{
  uint16_t nwkAddr;
  uint8_t  zoneID;
  uint16_t zoneType;
} zoneDevice_t;

typedef struct _discoveredDevice_t
{
  uint16_t nwkAddr;
  uint8_t  CIE_IEEE_Addr_Written;
} discoveredDevice_t;

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
static endPointDesc_t  zclSampleCIEEpDesc = {0};

#if ZG_BUILD_ENDDEVICE_TYPE
static ClockP_Handle EndDeviceRejoinClkHandle;
static ClockP_Struct EndDeviceRejoinClkStruct;
#endif

static ClockP_Struct DiscoverDeviceClkStruct;

//Store the short addresses of the device to be discovered.
static discoveredDevice_t serviceDiscoveryList[NUM_DISCOVERABLE_DEVICES];
static zoneDevice_t authenticatedZoneDevices[NUM_IAS_ZONE_SERVERS];

// Passed in function pointers to the NV driver
static NVINTF_nvFuncts_t *pfnZdlNV = NULL;

static afAddrType_t zclSampleCIE_DstAddr;

//App current state to display in UI
#ifdef ZCL_ZONE
uint8_t lastEvent = 0;
uint16_t lastEventAddr = 0xFFFF;
#endif

static LED_Handle gRedLedHandle;

#ifndef CUI_DISABLE
static uint16_t zclSampleCIE_BdbCommissioningModes;
CONST char zclSampleCIE_appStr[] = APP_TITLE_STR;
CUI_clientHandle_t gCuiHandle;
static uint32_t gSampleCIEInfoLine;
#endif

static uint8_t serviceDiscoveryEnabled = TRUE;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void zclSampleCIE_initialization(void);
static void zclSampleCIE_process_loop(void);
static void zclSampleCIE_initParameters(void);
static void zclSampleCIE_processZStackMsgs(zstackmsg_genericReq_t *pMsg);
static void SetupZStackCallbacks(void);
static void zclSampleCIE_processAfIncomingMsgInd(zstack_afIncomingMsgInd_t *pInMsg);
static void zclSampleCIE_initializeClocks(void);
#if ZG_BUILD_ENDDEVICE_TYPE
static void zclSampleCIE_processEndDeviceRejoinTimeoutCallback(UArg a0);
#endif
static void zclSampleCIE_processDiscoverDeviceTimeoutCallback(UArg a0);

static void zclSampleCIE_Init( void );

static void zclSampleCIE_BasicResetCB( void );
#ifndef CUI_DISABLE
static void zclSampleCIE_processKey(uint8_t key, Button_EventMask buttonEvents);
static void zclSampleCIE_RemoveAppNvmData(void);
static void zclSampleCIE_InitializeStatusLine(CUI_clientHandle_t gCuiHandle);
static void zclSampleCIE_UpdateStatusLine(void);
static void ArrayToString (uint8_t * buf, char * str, uint8_t num_of_digists, bool big_endian);
#endif
static void zclSampleCIE_IdentifyQueryRspCB(zclIdentifyQueryRsp_t *pRsp);
static void zclSampleCIE_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg);
static void zclSampleCIE_performServiceDiscovery(uint16_t discoveryAddr);

// Functions to process ZCL Foundation incoming Command/Response messages
static uint8_t zclSampleCIE_ProcessIncomingMsg( zclIncoming_t *msg );
#ifdef ZCL_READ
static uint8_t zclSampleCIE_ProcessInReadRspCmd( zclIncoming_t *pInMsg );
#endif
#ifdef ZCL_WRITE
static uint8_t zclSampleCIE_ProcessInWriteRspCmd( zclIncoming_t *pInMsg );
#endif
static uint8_t zclSampleCIE_ProcessInDefaultRspCmd( zclIncoming_t *pInMsg );
#ifdef ZCL_DISCOVER
static uint8_t zclSampleCIE_ProcessInDiscCmdsRspCmd( zclIncoming_t *pInMsg );
static uint8_t zclSampleCIE_ProcessInDiscAttrsRspCmd( zclIncoming_t *pInMsg );
static uint8_t zclSampleCIE_ProcessInDiscAttrsExtRspCmd( zclIncoming_t *pInMsg );
#endif

#ifdef ZCL_ZONE
static void zclSampleCIE_WriteIAS_CIE_Address(afAddrType_t *dstAddr);
//SS cluster callback functions
//Zone Cluster
static ZStatus_t zclSampleCIE_ChangeNotificationCB(zclZoneChangeNotif_t *pCmd,afAddrType_t *srcAddr );
static ZStatus_t zclSampleCIE_EnrollRequestCB(zclZoneEnrollReq_t *pReq, uint8_t endpoint );
#endif

#ifdef ZCL_WD
static void zclSendWarningToAllWD(zclWDStartWarning_t *alarm);
#ifndef CUI_DISABLE
static void zclSendSquawkToAllWD( zclWDSquawk_t *squawk );
#endif
#endif

#ifdef ZCL_ACE
static ZStatus_t zclSampleCIE_GetZoneInformationCB( zclIncoming_t *pInMsg );
static ZStatus_t zclSampleCIE_GetPanelStatusCB( zclIncoming_t *pInMsg );
static ZStatus_t zclSampleCIE_GetBypassedZoneListCB( zclIncoming_t *pInMsg );
static ZStatus_t zclSampleCIE_GetZoneStatusCB( zclIncoming_t *pInMsg );

static uint8_t zclSampleCIE_ArmCB(zclACEArm_t* armMode);
static ZStatus_t zclSampleCIE_BypassCB(zclACEBypass_t *pCmd);
static ZStatus_t zclSampleCIE_EmergencyCB(void);
static ZStatus_t zclSampleCIE_FireCB(void);
static ZStatus_t zclSampleCIE_PanicCB(void);
#endif

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * REFERENCED EXTERNALS
 */
extern int16_t zdpExternalStateTaskID;

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclSampleCIE_CmdCallbacks =
{
  zclSampleCIE_BasicResetCB,              // Basic Cluster Reset command
  NULL,                                   // Identfiy cmd
  NULL,                                   // Identify Query command
  zclSampleCIE_IdentifyQueryRspCB,        // Identify Query Response command
  NULL,                                   // Identify Trigger Effect command
#ifdef ZCL_ON_OFF
  NULL,                                   // On/Off cluster commands
  NULL,                                   // On/Off cluster enhanced command Off with Effect
  NULL,                                   // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                   // On/Off cluster enhanced command On with Timed Off
#endif
#ifdef ZCL_LEVEL_CTRL
  NULL,                                   // Level Control Move to Level command
  NULL,                                   // Level Control Move command
  NULL,                                   // Level Control Step command
  NULL,                                   // Level Control Stop command
  NULL,                                   // Level Control Move to Closest Frequency command
#endif
#ifdef ZCL_GROUPS
  NULL,                                   // Group Response commands
#endif
#ifdef ZCL_SCENES
  NULL,                                   // Scene Store Request command
  NULL,                                   // Scene Recall Request command
  NULL,                                   // Scene Response command
#endif
#ifdef ZCL_ALARMS
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
* ZCL SS Profile Callback table
*/


static zclSS_AppCallbacks_t zclSampleCIE_SSCmdCallbacks =
{
  zclSampleCIE_ChangeNotificationCB,      // Change Notification command
  zclSampleCIE_EnrollRequestCB,           // Enroll Request command
  NULL,                                   // Enroll Reponse command
  NULL,                                   // Initiate Normal Operation Mode command
  NULL,                                   // Initiate Test Mode command
  zclSampleCIE_ArmCB,                     // Arm command
  zclSampleCIE_BypassCB,                  // Bypass command
  zclSampleCIE_EmergencyCB,               // Emergency command
  zclSampleCIE_FireCB,                    // Fire command
  zclSampleCIE_PanicCB,                   // Panic command
  NULL,                                   // Get Zone ID Map command
  zclSampleCIE_GetZoneInformationCB,      // Get Zone Information Command
  zclSampleCIE_GetPanelStatusCB,          // Get Panel Status Command
  zclSampleCIE_GetBypassedZoneListCB,     // Get Bypassed Zone List Command
  zclSampleCIE_GetZoneStatusCB,           // Get Zone Status Command
  NULL,                                   // ArmResponse command
  NULL,                                   // Get Zone ID Map Response command
  NULL,                                   // Get Zone Information Response command
  NULL,                                   // Zone Status Changed command
  NULL,                                   // Panel Status Changed command
  NULL,                                   // Get Panel Status Response command
  NULL,                                   // Set Bypassed Zone List command
  NULL,                                   // Bypass Response command
  NULL,                                   // Get Zone Status Response command
  NULL,                                   // Start Warning command
  NULL                                    // Squawk command
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
  zclSampleCIE_initialization();

  // No return from task process
  zclSampleCIE_process_loop();
}




/*******************************************************************************
 * @fn          zclSampleCIE_initialization
 *
 * @brief       Initialize the application
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleCIE_initialization(void)
{
    /* Initialize user clocks */
    zclSampleCIE_initializeClocks();

    /* create semaphores for messages / events
     */
    Semaphore_Params semParam;
    Semaphore_Params_init(&semParam);
    semParam.mode = ti_sysbios_knl_Semaphore_Mode_COUNTING;
    Semaphore_construct(&appSem, 0, &semParam);
    appSemHandle = Semaphore_handle(&appSem);

    appServiceTaskId = OsalPort_registerTask(Task_self(), appSemHandle, &appServiceTaskEvents);

    //Initialize stack
    zclSampleCIE_Init();
}



/*********************************************************************
 * @fn          zclSampleCIE_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleCIE_Init( void )
{

  // Set destination address to indirect
  zclSampleCIE_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
  zclSampleCIE_DstAddr.endPoint = 0;
  zclSampleCIE_DstAddr.addr.shortAddr = 0;

  //Register Endpoint
  zclSampleCIEEpDesc.endPoint = SAMPLECIE_ENDPOINT;
  zclSampleCIEEpDesc.simpleDesc = &zclSampleCIE_SimpleDesc;
  zclport_registerEndpoint(appServiceTaskId, &zclSampleCIEEpDesc);

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
  gp_endpointInit(appServiceTaskId);
#endif

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( SAMPLECIE_ENDPOINT, &zclSampleCIE_CmdCallbacks );

  // Register the ZCL Security and Safety Cluster Library callback functions
  zclSS_RegisterCmdCallbacks( SAMPLECIE_ENDPOINT, &zclSampleCIE_SSCmdCallbacks );

  // Register the application's attribute list and reset to default values
  zclSampleCIE_ResetAttributesToDefaultValues();
  zcl_registerAttrList( SAMPLECIE_ENDPOINT, zclSampleCIE_NumAttributes, zclSampleCIE_Attrs );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zclport_registerZclHandleExternal(SAMPLECIE_ENDPOINT, zclSampleCIE_ProcessIncomingMsg);

  //Write the bdb initialization parameters
  zclSampleCIE_initParameters();

  //Setup ZDO callbacks
  SetupZStackCallbacks();

  // initialize device list entries to invalid addr
  uint8_t i;
  for(i = 0; i < NUM_IAS_ZONE_SERVERS; i++)
  {
    authenticatedZoneDevices[i].nwkAddr = INVALID_NODE_ADDR;
  }

  for(i = 0; i < NUM_DISCOVERABLE_DEVICES; i++)
  {
    serviceDiscoveryList[i].nwkAddr = INVALID_NODE_ADDR;
    serviceDiscoveryList[i].CIE_IEEE_Addr_Written = FALSE;
  }


#ifndef CUI_DISABLE
#if defined ( BDB_TL_INITIATOR )
  zclSampleCIE_BdbCommissioningModes |= BDB_COMMISSIONING_MODE_INITIATOR_TL;
#endif
   // set up default application BDB commissioning modes based on build type
   if(ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE)
   {
     zclSampleCIE_BdbCommissioningModes = BDB_COMMISSIONING_MODE_NWK_FORMATION | BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_FINDING_BINDING;
   }
   else if (ZG_BUILD_JOINING_TYPE && ZG_DEVICE_JOINING_TYPE)
   {
     zclSampleCIE_BdbCommissioningModes = BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_FINDING_BINDING;
   }


   gCuiHandle = UI_Init( appServiceTaskId,                     // Application Task ID
            &appServiceTaskEvents,                // The events processed by the sample application
            appSemHandle,                         // Semaphore to post the events in the application thread
            &zclSampleCIE_IdentifyTime,
            &zclSampleCIE_BdbCommissioningModes,   // A pointer to the application's bdbCommissioningModes
            zclSampleCIE_appStr,                   // A pointer to the app-specific name string
            zclSampleCIE_processKey,               // A pointer to the app-specific key process function
            zclSampleCIE_RemoveAppNvmData          // A pointer to the app-specific NV Item reset function
            );


   //Request the Red LED for App
   LED_Params ledParams;
   LED_Params_init(&ledParams);
   gRedLedHandle = LED_open(CONFIG_LED_RED, &ledParams);


   //Initialize the SampleCIE UI status line
   zclSampleCIE_InitializeStatusLine(gCuiHandle);
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
 * @fn          zclSampleCIE_RemoveAppNvmData
 *
 * @brief       Callback when Application performs reset to Factory New Reset.
 *              Application must restore the application to default values
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleCIE_RemoveAppNvmData(void)
{

}
#endif

static void zclSampleCIE_initParameters(void)
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
 * @fn      zclSampleCIE_initializeClocks
 *
 * @brief   Initialize Clocks
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleCIE_initializeClocks(void)
{
#if ZG_BUILD_ENDDEVICE_TYPE
    // Initialize the timers needed for this application
    EndDeviceRejoinClkHandle = UtilTimer_construct(
    &EndDeviceRejoinClkStruct,
    zclSampleCIE_processEndDeviceRejoinTimeoutCallback,
    SAMPLEAPP_END_DEVICE_REJOIN_DELAY,
    0, false, 0);
#endif

    // Initialize the timers needed for this application
    UtilTimer_construct(
    &DiscoverDeviceClkStruct,
    zclSampleCIE_processDiscoverDeviceTimeoutCallback,
    SAMPLECIE_SERVICE_DISCOVERY_RETRY_PERIOD,
    0, false, 0);
}

#if ZG_BUILD_ENDDEVICE_TYPE
/*******************************************************************************
 * @fn      zclSampleCIE_processEndDeviceRejoinTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleCIE_processEndDeviceRejoinTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    appServiceTaskEvents |= SAMPLEAPP_END_DEVICE_REJOIN_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}
#endif


/*******************************************************************************
 * @fn      zclSampleCIE_processDiscoverDeviceTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleCIE_processDiscoverDeviceTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    appServiceTaskEvents |= SAMPLEAPP_DISCOVER_DEVICE_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
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
    zdoCBReq.has_deviceAnnounce = true;
    zdoCBReq.deviceAnnounce = true;

    (void)Zstackapi_DevZDOCBReq(appServiceTaskId, &zdoCBReq);
}



/*******************************************************************************
 * @fn      zclSampleCIE_process_loop
 *
 * @brief   Application task processing start.
 *
 * @param   none
 *
 * @return  void
 */
static void zclSampleCIE_process_loop(void)
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
                zclSampleCIE_processZStackMsgs(pMsg);
                // Free any separately allocated memory
                msgProcessed = Zstackapi_freeIndMsg(pMsg);
            }

            if((msgProcessed == FALSE) && (pMsg != NULL))
            {
                OsalPort_msgDeallocate((uint8_t*)pMsg);
            }

            if(appServiceTaskEvents & SAMPLEAPP_DISCOVER_DEVICE_EVT)
            {
              uint8_t i;
              uint8_t restartTimer = FALSE;
              // retry service discovery to any addresses
              // that have not been cleared
              // address is cleared in write attribute rsp processing
              for ( i = 0; i < NUM_DISCOVERABLE_DEVICES; i++ )
              {
                if ( serviceDiscoveryList[i].nwkAddr != INVALID_NODE_ADDR &&
                     serviceDiscoveryList[i].CIE_IEEE_Addr_Written == FALSE )
                {
                  zclSampleCIE_performServiceDiscovery(serviceDiscoveryList[i].nwkAddr);
                  // if there are any devices still in the list,
                  // restart the timer again
                  restartTimer = TRUE;
                }
              }

              if ( restartTimer && serviceDiscoveryEnabled )
              {
                UtilTimer_start(&DiscoverDeviceClkStruct);
              }

              appServiceTaskEvents &= ~SAMPLEAPP_DISCOVER_DEVICE_EVT;
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

#ifndef CUI_DISABLE
  //Update status line
  zclSampleCIE_UpdateStatusLine();
#endif

        }
    }
}




/*******************************************************************************
 * @fn      zclSampleCIE_processZStackMsgs
 *
 * @brief   Process event from Stack
 *
 * @param   pMsg - pointer to incoming ZStack message to process
 *
 * @return  void
 */
static void zclSampleCIE_processZStackMsgs(zstackmsg_genericReq_t *pMsg)
{
      switch(pMsg->hdr.event)
      {
          case zstackmsg_CmdIDs_BDB_NOTIFICATION:
              {
                  zstackmsg_bdbNotificationInd_t *pInd;
                  pInd = (zstackmsg_bdbNotificationInd_t*)pMsg;
                  zclSampleCIE_ProcessCommissioningStatus(&(pInd->Req));
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
                  zclSampleCIE_processAfIncomingMsgInd( &(pInd->req) );
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

            uiProcessTouchlinkTargetEnable(&(pInd->req));
          }
          break;
#endif

          case zstackmsg_CmdIDs_ZDO_MATCH_DESC_RSP:
          {
              zstackmsg_zdoMatchDescRspInd_t *pInd =
                      (zstackmsg_zdoMatchDescRspInd_t *)pMsg;

              if((pInd->rsp.status == zstack_ZdpStatus_SUCCESS) && (pInd->rsp.n_matchList))
              {
                  afAddrType_t dstAddr;

                  dstAddr.addr.shortAddr  = pInd->rsp.nwkAddrOfInterest;
                  dstAddr.endPoint = *pInd->rsp.pMatchList;  //send it to the first endpoint.
                  dstAddr.addrMode = afAddr16Bit;
                  dstAddr.panId = 0;

                  zclSampleCIE_WriteIAS_CIE_Address(&dstAddr);
              }
          }
          break;

          case zstackmsg_CmdIDs_ZDO_DEVICE_ANNOUNCE:
          {
              zstackmsg_zdoDeviceAnnounceInd_t *pInd =
                      (zstackmsg_zdoDeviceAnnounceInd_t *)pMsg;

               zclSampleCIE_performServiceDiscovery(pInd->req.devAddr);
          }
          break;

          case zstackmsg_CmdIDs_BDB_TC_LINK_KEY_EXCHANGE_NOTIFICATION_IND:
          case zstackmsg_CmdIDs_AF_DATA_CONFIRM_IND:
          case zstackmsg_CmdIDs_ZDO_NWK_ADDR_RSP:
          case zstackmsg_CmdIDs_DEV_PERMIT_JOIN_IND:
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
 * @fn          zclSampleCIE_processAfIncomingMsgInd
 *
 * @brief       Process AF Incoming Message Indication message
 *
 * @param       pInMsg - pointer to incoming message
 *
 * @return      none
 *
 */
static void zclSampleCIE_processAfIncomingMsgInd(zstack_afIncomingMsgInd_t *pInMsg)
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
 * @fn      zclSampleCIE_ProcessCommissioningStatus
 *
 * @brief   Callback in which the status of the commissioning process are reported
 *
 * @param   bdbCommissioningModeMsg - Context message of the status of a commissioning process
 *
 * @return  none
 */
static void zclSampleCIE_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg)
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
 * @fn      zclSampleCIE_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to  default values.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleCIE_BasicResetCB( void )
{
  zclSampleCIE_ResetAttributesToDefaultValues();

#ifndef CUI_DISABLE
  // update the display
  zclSampleCIE_UpdateStatusLine( );
#endif
}

/*********************************************************************
 * @fn      zclSampleCIE_IdentifyQueryRspCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to process a Indentify Query Rsp.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleCIE_IdentifyQueryRspCB(zclIdentifyQueryRsp_t *pRsp)
{
  zclSampleCIE_performServiceDiscovery(pRsp->srcAddr->addr.shortAddr);
}

/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/

/*********************************************************************
 * @fn      zclSampleCIE_ProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  uint8_t - TRUE if got handled
 */
static uint8_t zclSampleCIE_ProcessIncomingMsg( zclIncoming_t *pInMsg )
{
  uint8_t handled = FALSE;

  switch ( pInMsg->hdr.commandID )
  {
#ifdef ZCL_READ
    case ZCL_CMD_READ_RSP:
      zclSampleCIE_ProcessInReadRspCmd( pInMsg );
      handled = TRUE;
      break;
#endif
#ifdef ZCL_WRITE
    case ZCL_CMD_WRITE_RSP:
      zclSampleCIE_ProcessInWriteRspCmd( pInMsg );
      handled = TRUE;
      break;
#endif
#ifdef ZCL_REPORT
    // See ZCL Test Applicaiton (zcl_testapp.c) for sample code on Attribute Reporting
    case ZCL_CMD_CONFIG_REPORT:
      //zclSampleCIE_ProcessInConfigReportCmd( pInMsg );
      break;

    case ZCL_CMD_CONFIG_REPORT_RSP:
      //zclSampleCIE_ProcessInConfigReportRspCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG:
      //zclSampleCIE_ProcessInReadReportCfgCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG_RSP:
      //zclSampleCIE_ProcessInReadReportCfgRspCmd( pInMsg );
      break;

    case ZCL_CMD_REPORT:
      //zclSampleCIE_ProcessInReportCmd( pInMsg );
      break;
#endif
    case ZCL_CMD_DEFAULT_RSP:
      zclSampleCIE_ProcessInDefaultRspCmd( pInMsg );
      handled = TRUE;
      break;
#ifdef ZCL_DISCOVER
    case ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP:
      zclSampleCIE_ProcessInDiscCmdsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_CMDS_GEN_RSP:
      zclSampleCIE_ProcessInDiscCmdsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_ATTRS_RSP:
      zclSampleCIE_ProcessInDiscAttrsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_ATTRS_EXT_RSP:
      zclSampleCIE_ProcessInDiscAttrsExtRspCmd( pInMsg );
      handled = TRUE;
      break;
#endif
    default:
      break;
  }

  return handled;
}

#ifdef ZCL_READ
/*********************************************************************
 * @fn      zclSampleCIE_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleCIE_ProcessInReadRspCmd( zclIncoming_t *pInMsg )
{
  zclReadRspCmd_t *readRspCmd;
  uint8_t i;

  readRspCmd = (zclReadRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < readRspCmd->numAttr; i++)
  {
    // Notify the originator of the results of the original read attributes
    // attempt and, for each successfull request, the value of the requested
    // attribute
  }

  return TRUE;
}
#endif // ZCL_READ

#ifdef ZCL_WRITE
/*********************************************************************
 * @fn      zclSampleCIE_ProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleCIE_ProcessInWriteRspCmd( zclIncoming_t *pInMsg )
{
  zclWriteRspCmd_t *writeRspCmd;
  uint8_t i;
  uint16_t srcAddr = pInMsg->msg->srcAddr.addr.shortAddr;

  writeRspCmd = (zclWriteRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < writeRspCmd->numAttr; i++)
  {
    // If Write CIE Address was successful on remote device
    if(writeRspCmd->attrList[i].status == ZCL_STATUS_SUCCESS)
    {
      uint8_t j;
      // find the device that sent the response and update the
      // table entry to show that the CIE IEEE Addr has been written
      for ( j = 0; j < NUM_DISCOVERABLE_DEVICES; j++ )
      {
        if (serviceDiscoveryList[j].nwkAddr == srcAddr)
        {
          serviceDiscoveryList[j].CIE_IEEE_Addr_Written = TRUE;
          break;
        }
      }
      break;
    }
  }

  return TRUE;
}
#endif // ZCL_WRITE

/*********************************************************************
 * @fn      zclSampleCIE_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleCIE_ProcessInDefaultRspCmd( zclIncoming_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;
  // Device is notified of the Default Response command.
  (void)pInMsg;
  return TRUE;
}

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      zclSampleCIE_ProcessInDiscCmdsRspCmd
 *
 * @brief   Process the Discover Commands Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleCIE_ProcessInDiscCmdsRspCmd( zclIncoming_t *pInMsg )
{
  zclDiscoverCmdsCmdRsp_t *discoverRspCmd;
  uint8_t i;

  discoverRspCmd = (zclDiscoverCmdsCmdRsp_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numCmd; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return TRUE;
}

/*********************************************************************
 * @fn      zclSampleCIE_ProcessInDiscAttrsRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleCIE_ProcessInDiscAttrsRspCmd( zclIncoming_t *pInMsg )
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

/*********************************************************************
 * @fn      zclSampleCIE_ProcessInDiscAttrsExtRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Extended Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleCIE_ProcessInDiscAttrsExtRspCmd( zclIncoming_t *pInMsg )
{
  zclDiscoverAttrsExtRsp_t *discoverRspCmd;
  uint8_t i;

  discoverRspCmd = (zclDiscoverAttrsExtRsp_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return TRUE;
}
#endif // ZCL_DISCOVER

#ifdef ZCL_ZONE
/*******************************************************************************
* @fn      static void zclSampleCIE_ChangeNotificationCB
*
* @brief   Process incoming Change Notification Command
*
* @param   Zone Status Change Notification command
*
* @return  none
*/
static ZStatus_t zclSampleCIE_ChangeNotificationCB(zclZoneChangeNotif_t *pCmd, afAddrType_t *srcAddr )
{

  //building the warning msg
  zclWDStartWarning_t alarm;
  alarm.warningDuration = SS_IAS_WD_ALARM_DEFAULT_DURATION;
  alarm.strobeDutyCycle = SS_IAS_WD_STROBE_DEFAULT_DUTY_CYCLE;
  alarm.strobeLevel = SS_IAS_STROBE_LEVEL_MEDIUM_LEVEL_STROBE;
  alarm.warningmessage.warningbits.warnSirenLevel = SS_IAS_SIREN_LEVEL_MEDIUM_LEVEL_SOUND;

  //displaying messages
  switch(pCmd->zoneStatus)
  {
    //Here only implementing fire alarm (1st bit), other bits not checked
    case 0://since zoneStatus is a bit mask
    {
      lastEvent = NOTIFICATION_NO_FIRE;
      lastEventAddr = (srcAddr->addr.shortAddr);

      //setting params for the Warning Message
      LED_stopBlinking(gRedLedHandle);
      LED_setOff(gRedLedHandle);
      alarm.warningmessage.warningbits.warnMode = SS_IAS_START_WARNING_WARNING_MODE_STOP;
      alarm.warningmessage.warningbits.warnStrobe = SS_IAS_START_WARNING_STROBE_NO_STROBE_WARNING;
#ifndef CUI_DISABLE
      zclSampleCIE_UpdateStatusLine();
#endif
    }
    break;

    case SS_IAS_ZONE_STATUS_ALARM1_ALARMED:
    {
      lastEvent = NOTIFICATION_ALARMED;
      lastEventAddr = (srcAddr->addr.shortAddr);

      //setting params for the Warning Message
      LED_stopBlinking(gRedLedHandle);
      LED_setOn(gRedLedHandle, LED_BRIGHTNESS_MAX);
      alarm.warningmessage.warningbits.warnMode = SS_IAS_START_WARNING_WARNING_MODE_FIRE;
      alarm.warningmessage.warningbits.warnStrobe = SS_IAS_START_WARNING_STROBE_USE_STPOBE_IN_PARALLEL_TO_WARNING;
#ifndef CUI_DISABLE
      zclSampleCIE_UpdateStatusLine();
#endif
    }
    break;

  default:
    break;
  }
#ifndef CUI_DISABLE
  //Update status line
  zclSampleCIE_UpdateStatusLine();
#endif

  //sending the warning to all WD in table
  zclSendWarningToAllWD(&alarm);

  return ZSuccess ;
}



/*******************************************************************************
* @fn      static void zclSampleCIE_EnrollRequestCB
*
* @brief   Process incoming Zone Enroll Request Command
*
* @param   Zone Enroll Request command
*
* @return  none
*/
static ZStatus_t zclSampleCIE_EnrollRequestCB(zclZoneEnrollReq_t *pReq, uint8_t endpoint )
{
  //create temporary placeholder for IEEE address
  uint8_t extAddr[Z_EXTADDR_LEN];

  //copy the IEEE Address to the temp var
  if(APSME_LookupExtAddr(pReq->srcAddr->addr.shortAddr, extAddr))
  {
    //fill in the enrolling Device IEEE Address in the zone record
    zclSS_UpdateZoneAddress( pReq->srcAddr->endPoint, pReq->zoneID, extAddr );
  }

  lastEvent = ZONE_ADDED;
  lastEventAddr = pReq->srcAddr->addr.shortAddr;
#ifndef CUI_DISABLE
  //Update status line
  zclSampleCIE_UpdateStatusLine();
#endif

  uint8_t i;

  // service discovery and zone enrollment have been completed for this node,
  // so we can remove it from this table
  for ( i = 0; i < NUM_DISCOVERABLE_DEVICES; i++ )
  {
    if (serviceDiscoveryList[i].nwkAddr == pReq->srcAddr->addr.shortAddr)
    {
      serviceDiscoveryList[i].nwkAddr = INVALID_NODE_ADDR;
      serviceDiscoveryList[i].CIE_IEEE_Addr_Written = FALSE;
    }
  }

  // save the newly enrolled Zone device info locally
  for ( i = 0; i < NUM_IAS_ZONE_SERVERS; i++ )
  {
    if (authenticatedZoneDevices[i].nwkAddr == INVALID_NODE_ADDR)
    {
      authenticatedZoneDevices[i].nwkAddr = pReq->srcAddr->addr.shortAddr;
      authenticatedZoneDevices[i].zoneID = pReq->zoneID;
      authenticatedZoneDevices[i].zoneType = pReq->zoneType;
      break;
    }
  }

  return ZSuccess;

}
/*******************************************************************************
* @fn      static void zclSampleCIE_WriteIAS_CIE_Address
*
* @brief   Write to the IAS_CIE_Addr attribute on the Zone server
*
* @param   none
*
* @return  none
*/

static void zclSampleCIE_WriteIAS_CIE_Address(afAddrType_t *dstAddr)
{
  //only one attrib to write
  uint8_t numAttr = 1;
  uint8_t hdrLen;
  uint8_t dataLen = zclGetDataTypeLength(ZCL_DATATYPE_IEEE_ADDR);

  // calculate the length of the cmd
  hdrLen = sizeof( zclWriteCmd_t ) + ( numAttr * sizeof( zclWriteRec_t ) );
  zclWriteCmd_t *writeCmd = (zclWriteCmd_t *)OsalPort_malloc( hdrLen + dataLen );

  //Building the command and sending it
  if ( writeCmd != NULL )
  {
    zstack_sysNwkInfoReadRsp_t  Rsp;
    Zstackapi_sysNwkInfoReadReq(appServiceTaskId, &Rsp);

    zstack_getZCLFrameCounterRsp_t zclCounterRsp;
    Zstackapi_getZCLFrameCounterReq(appServiceTaskId, &zclCounterRsp);

    writeCmd->numAttr = numAttr;
    zclWriteRec_t *attrRec = &(writeCmd->attrList[0]);
    attrRec->attrID = ATTRID_SS_IAS_CIE_ADDRESS;
    attrRec->dataType = ZCL_DATATYPE_IEEE_ADDR;
    attrRec->attrData = Rsp.ieeeAddr;

    zcl_SendWrite(SAMPLECIE_ENDPOINT, dstAddr,ZCL_CLUSTER_ID_SS_IAS_ZONE,
                  writeCmd,ZCL_FRAME_CLIENT_SERVER_DIR,0,zclCounterRsp.zclFrameCounter );

    OsalPort_free( writeCmd );
  }
}

#endif //ZCL_ZONE



#ifdef ZCL_WD
#ifndef CUI_DISABLE
/*******************************************************************************

* @fn      static void zclSendSquawkToAllWD
*
* @brief   Send Squawk Signal to all Warning Devices in the table
*
* @param   none
*
* @return  none
*/
static void zclSendSquawkToAllWD( zclWDSquawk_t *squawk )
{
    zclSS_Send_IAS_WD_SquawkCmd(SAMPLECIE_ENDPOINT, &zclSampleCIE_DstAddr, squawk, 0, 1);
}
#endif

/*******************************************************************************

* @fn      static void zclSendWarningToAllWD
*
* @brief   sends warning message to all WD in WD table
*
* @param   uint16_t network addr of alarm origininator
*
* @return  none
*/
static void zclSendWarningToAllWD(zclWDStartWarning_t *alarm)
{
  zclSS_Send_IAS_WD_StartWarningCmd(SAMPLECIE_ENDPOINT, &zclSampleCIE_DstAddr, alarm, 1, 1);
}

#endif //ZCL_WD

#ifndef CUI_DISABLE
/*********************************************************************
 * @fn      zclSampleCIE_processKey
 *
 * @brief   Key event handler function
 *
 * @param   key - key to handle action for
 *          buttonEvents - event to handle action for
 *
 * @return  none
 */
static void zclSampleCIE_processKey(uint8_t key, Button_EventMask buttonEvents)
{
    if (buttonEvents & Button_EV_CLICKED)
    {
        if(key == CONFIG_BTN_LEFT)
        {
            zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;

            zstack_bdbStartCommissioningReq.commissioning_mode = zclSampleCIE_BdbCommissioningModes;
            Zstackapi_bdbStartCommissioningReq(appServiceTaskId,&zstack_bdbStartCommissioningReq);
        }
        if(key == CONFIG_BTN_RIGHT)
        {
            //unused
        }
    }
}

static void zclSampleCIE_UpdateStatusLine(void)
{
    char lineFormat[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};
    bool reportFromNode = FALSE;


    strcpy(lineFormat,"["CUI_COLOR_YELLOW"Last Event"CUI_COLOR_RESET"] ");
    switch(lastEvent)
    {
        case NO_EVENT:
            strcat(lineFormat,"NO EVENT YET  ");
        break;
        case ZONE_ADDED:
            strcat(lineFormat,"Zone added device 0x");
           reportFromNode = TRUE;
        break;
        case NOTIFICATION_NO_FIRE:
            strcat(lineFormat,"No fire in device 0x");
            reportFromNode = TRUE;
        break;
        case NOTIFICATION_ALARMED:
            strcat(lineFormat,"ALARM! in device: 0x");
            reportFromNode = TRUE;
        break;
    }
    if(reportFromNode)
    {
        char tempBuf[5];
        uint8_t shortAddBuf[2];
        shortAddBuf[0] = LO_UINT16(lastEventAddr);
        shortAddBuf[1] = HI_UINT16(lastEventAddr);
        ArrayToString(shortAddBuf,tempBuf,4,FALSE);
        tempBuf[4] = 0;
        strcat(lineFormat, tempBuf);
    }

    CUI_statusLinePrintf(gCuiHandle, gSampleCIEInfoLine, lineFormat);
}

static void zclSampleCIE_InitializeStatusLine(CUI_clientHandle_t gCuiHandle)
{
    /* Request Async Line for Light application Info */
    CUI_statusLineResourceRequest(gCuiHandle, "   APP Info"CUI_DEBUG_MSG_START"1"CUI_DEBUG_MSG_END, false, &gSampleCIEInfoLine);

    zclSampleCIE_UpdateStatusLine();
}

/*********************************************************************
 * @fn          ArrayToString
 *
 * @brief       format a memory buffer into a string buffer in hex representation.
 *
 * @param       buf - pointer to a bufer to be formatted
 *              str - pointer to a buffer to store the formatted string
 *              num_of_digists - number of digits to include in the formatted string
 *              big_endian - whether the memory content should be represented as big or little endian
 *
 * @return      none
 */
static void ArrayToString (uint8_t * buf, char * str, uint8_t num_of_digists, bool big_endian)
{
  int i;
  uint8_t stringIndex;
  uint8_t value;

  for (i = 0; i < num_of_digists; i++)
  {
    stringIndex = (big_endian ? i : num_of_digists - 1 - i);
    if(big_endian)
    {
      value = (buf[i / 2] >> (4 * (!(i % 2)))) & 0x0F;
    }
    else
    {
      value = (buf[i / 2] >> (4 * (i % 2))) & 0x0F;
    }
    str[stringIndex] = '0' + value;
    if (str[stringIndex] > '9')
    {
      str[stringIndex] += 'A' - '0' - 10;
    }
  }
}
#endif // CUI_DISABLE

static void zclSampleCIE_performServiceDiscovery(uint16_t discoveryAddr)
{
  uint8_t i;

  for ( i = 0; i < NUM_IAS_ZONE_SERVERS; i++ )
  {
    if (authenticatedZoneDevices[i].nwkAddr == discoveryAddr)
    {
      // we have already authenticated this Zone device, so no need to continue
      return;
    }
  }

  uint8_t newDevice = TRUE;

  // check if the device is already in the service discovery table
  for ( i = 0; i < NUM_DISCOVERABLE_DEVICES; i++ )
  {
    if (serviceDiscoveryList[i].nwkAddr == discoveryAddr)
    {
      // if the CIE IEEE Addr has already been written on this device,
      // we don't need to continue with service discovery
      if(serviceDiscoveryList[i].CIE_IEEE_Addr_Written == TRUE)
      {
        return;
      }
      // otherwise, we are retrying service discovery on an existing device
      else
      {
        newDevice = FALSE;
      }
      break;
    }
  }

  // a new device has joined the network, query it to see if it is an
  // IAS Zone Server device.
  zstack_zdoMatchDescReq_t matchDescReq;
  uint16_t  zoneCluster = ZCL_CLUSTER_ID_SS_IAS_ZONE;

  matchDescReq.dstAddr = discoveryAddr;
  matchDescReq.nwkAddrOfInterest = discoveryAddr;
  matchDescReq.profileID = ZCL_HA_PROFILE_ID;

  matchDescReq.n_inputClusters = 1;
  matchDescReq.n_outputClusters = 0;

  matchDescReq.pInputClusters = &zoneCluster;

  Zstackapi_ZdoMatchDescReq(appServiceTaskId, &matchDescReq);

  // if this is a new device (not a retry), add the device
  // to the service discovery table
  if(newDevice)
  {
    // find an empty slot
    for ( i = 0; i < NUM_DISCOVERABLE_DEVICES; i++ )
    {
      if (serviceDiscoveryList[i].nwkAddr == INVALID_NODE_ADDR)
      {
        serviceDiscoveryList[i].nwkAddr = discoveryAddr;
        break;
      }
    }
    if (i == NUM_DISCOVERABLE_DEVICES)
    {
      // no space in discovery list,
      // increase NUM_DISCOVERABLE_DEVICES
    }
  }

  UtilTimer_stop(&DiscoverDeviceClkStruct);
  if( serviceDiscoveryEnabled )
  {
    UtilTimer_start(&DiscoverDeviceClkStruct);
  }
}

#ifndef CUI_DISABLE
void zclSampleCIE_UiActionSendSquawk(const int32_t _itemEntry)
{
    zclWDSquawk_t squawk;
    squawk.squawkbits.squawkMode = SS_IAS_SQUAWK_SQUAWK_MODE_SYSTEM_ALARMED_NOTIFICATION_SOUND;
    squawk.squawkbits.squawkLevel = 0;
    squawk.squawkbits.strobe = 0;

    zclSendSquawkToAllWD(&squawk);
}

void zclSampleCIE_UiActionDiscoverZone(const int32_t _itemEntry)
{
  afAddrType_t destAddr;
  zstack_getZCLFrameCounterRsp_t Rsp;

  destAddr.endPoint = 0xFF;
  destAddr.addrMode = afAddr16Bit;
  destAddr.addr.shortAddr = 0xFFFF;

  Zstackapi_getZCLFrameCounterReq(appServiceTaskId, &Rsp);

  zclGeneral_SendIdentifyQuery(SAMPLECIE_ENDPOINT, &destAddr, FALSE, Rsp.zclFrameCounter);
}

void zclSampleCIE_UiActionConfigureServiceDiscovery(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
  if (_input == CUI_INPUT_UP || _input == CUI_INPUT_DOWN)
  {
    serviceDiscoveryEnabled = !serviceDiscoveryEnabled;
  }

  switch(serviceDiscoveryEnabled)
  {
    case TRUE:
    {
      UtilTimer_start(&DiscoverDeviceClkStruct);
      strncpy(_pLines[1], "ENABLED", MAX_MENU_LINE_LEN);
    }
    break;
    case FALSE:
    {
      UtilTimer_stop(&DiscoverDeviceClkStruct);
      strncpy(_pLines[1], "DISABLED", MAX_MENU_LINE_LEN);
    }
    break;
  }

  if (_input != CUI_ITEM_PREVIEW) {
      strncpy(_pLines[0], "Up/Down to select:", MAX_MENU_LINE_LEN);
      strncpy(_pLines[2], " CONFIG DISCOVERY ", MAX_MENU_LINE_LEN);
  }
}
#endif // CUI_DISABLE

#ifdef ZCL_ACE

/*********************************************************************
* @fn      zclSampleCIE_ArmCB
*
* @brief   Process in the received Arm Command.
*
* @param   armMode
*
*/
static uint8_t zclSampleCIE_ArmCB( zclACEArm_t* payload )
{
  //Process the ACE Arm command
  switch(payload->armMode)
  {
  case SS_IAS_ACE_ARM_DISARM:
    //set the CIE to disarmed
    break;
  case SS_IAS_ACE_ARM_DAY_HOME_ZONES_ONLY:

    break;
  case SS_IAS_ACE_ARM_NIGHT_SLEEP_ZONES_ONLY:

    break;
  case SS_IAS_ACE_ARM_ALL_ZONES:

    break;
  }

  return (ZSuccess);
}

/*********************************************************************
* @fn      zclSampleCIE_BypassCB
*
* @brief   Process in the received Bypass Command.
*
* @param   zclACEBypass_t *pCmd
*
*/
static ZStatus_t zclSampleCIE_BypassCB(zclACEBypass_t *pCmd)
{
  //Process the ACE Bypass cmd
  return ZSuccess ;
}


/*******************************************************************************
* @fn      zclSampleCIE_FireCB
*
* @brief   Process incoming Fire Command
*
* @param   none
*
* @return  none
*/
static ZStatus_t zclSampleCIE_FireCB( void)
{
  //Make the UI display the information about a fire being detected
  return ZSuccess ;

}

/*******************************************************************************
* @fn      zclSampleCIE_EmergencyCB
*
* @brief   Process incoming Emergency Command
*
* @param   none
*
* @return  none
*/
static ZStatus_t zclSampleCIE_EmergencyCB(void)
{
  //display emergency state on UI
  return ZSuccess ;
}

/*******************************************************************************
* @fn      zclSampleCIE_PanicCB
*
* @brief   Process incoming Panic Command
*
* @param   none
*
* @return  none
*/
static ZStatus_t zclSampleCIE_PanicCB(void)
{
  //display Panic state on UI
  return ZSuccess ;
}


/*******************************************************************************
* @fn      zclSampleCIE_GetPanelStatusCB
*
* @brief   Process incoming Get Panel Status Command
*
* @param   none
*
* @return  none
*/
static ZStatus_t zclSampleCIE_GetPanelStatusCB(zclIncoming_t *pInMsg )
{

    /* Your Job: Fill in the response with the panel status parameters */
    zclACEGetPanelStatusRsp_t Cmd;

    Cmd.alarmStatus = SS_IAS_ACE_ALARM_STATUS_NO_ALARM;
    Cmd.audibleNotification = SS_IAS_ACE_AUDIBLE_NOTIFICATION_MUTE;
    Cmd.panelStatus = SS_IAS_ACE_PANEL_STATUS_ALL_ZONES_DISARMED;
    Cmd.secondsRemaining = 0;

    return zclSS_Send_IAS_ACE_GetPanelStatusResponseCmd(SAMPLECIE_ENDPOINT,
                                                 &pInMsg->msg->srcAddr,
                                                 &Cmd, TRUE,
                                                 pInMsg->hdr.transSeqNum);

}


/*******************************************************************************
* @fn      zclSampleCIE_GetBypassedZoneListCB
*
* @brief   Process incoming Get Panel Status Command
*
* @param   none
*
* @return  none
*/
static ZStatus_t zclSampleCIE_GetBypassedZoneListCB(zclIncoming_t *pInMsg )
{

    /* Your Job: Fill in the response with the list of Zones bypassed */
    zclACESetBypassedZoneList_t Cmd;

    //Get the number of Zones bypassed
    Cmd.numberOfZones = 0;
    Cmd.zoneID = NULL;



   return zclSS_Send_IAS_ACE_SetBypassedZoneListCmd(SAMPLECIE_ENDPOINT,
                                              &pInMsg->msg->srcAddr,
                                              &Cmd, TRUE,
                                              pInMsg->hdr.transSeqNum);
}

/*******************************************************************************
* @fn      zclSampleCIE_GetZoneStatusCB
*
* @brief   Process incoming Get Zone Status Command
*
* @param   none
*
* @return  none
*/
static ZStatus_t zclSampleCIE_GetZoneStatusCB(zclIncoming_t *pInMsg )
{
    zclACEGetZoneStatus_t InCmd;
    zclACEGetZoneStatusRsp_t Cmd;

    //Parse the request
    InCmd.startingZoneID = pInMsg->pData[0];
    InCmd.maxNumZoneIDs = pInMsg->pData[1];
    InCmd.zoneStatusMaskFlag = pInMsg->pData[2];
    InCmd.zoneStatusMask = BUILD_UINT16( pInMsg->pData[3], pInMsg->pData[4] );
    //InCmd intentionally not used
    (void) InCmd;

    //Your Job: Look for the zones based on the requested parameters in InCmd
    //and put them into the response Cmd

    Cmd.numberOfZones = 0;
    Cmd.zoneInfo = NULL;
    Cmd.zoneStatusComplete = TRUE;

    return zclSS_Send_IAS_ACE_GetZoneStatusResponseCmd(SAMPLECIE_ENDPOINT,
                                                &pInMsg->msg->srcAddr,
                                                &Cmd, TRUE,
                                                pInMsg->hdr.transSeqNum);


}



/*******************************************************************************
* @fn      zclSampleCIE_GetZoneInformationCB
*
* @brief   Process incoming Get Zone Information Command
*
* @param   none
*
* @return  none
*/
static ZStatus_t zclSampleCIE_GetZoneInformationCB(zclIncoming_t *pInMsg )
{
  IAS_ACE_ZoneTable_t *IAS_ACE_ZoneTable;
  zclACEGetZoneInfoRsp_t Cmd;
  uint8_t dummyIEEE[Z_EXTADDR_LEN];
  uint8_t zoneId;

  //Get the zone Id
  zoneId = pInMsg->pData[0];

  //Get the entry from the zone table
  IAS_ACE_ZoneTable = zclSS_FindZone(SAMPLECIE_ENDPOINT, zoneId);

  //Populate rsp if entry exist, otherwise set IEEE and Zone type to 0xFF's to
  //indicate that zone id does not exist
  if(IAS_ACE_ZoneTable != NULL)
  {
      Cmd.ieeeAddr = IAS_ACE_ZoneTable->zoneAddress;
      Cmd.zoneID = zoneId;
      Cmd.zoneType = IAS_ACE_ZoneTable->zoneType;
  }
  else
  {
      memset(dummyIEEE,0xFF,Z_EXTADDR_LEN);
      Cmd.ieeeAddr = dummyIEEE;
      Cmd.zoneType = 0xFFFF;
  }
  Cmd.zoneLabel.strLen = 0;
  Cmd.zoneLabel.pStr = NULL;

  return zclSS_Send_IAS_ACE_GetZoneInformationResponseCmd(SAMPLECIE_ENDPOINT,
                                                   &pInMsg->msg->srcAddr,
                                                   &Cmd, TRUE,
                                                   pInMsg->hdr.transSeqNum );
}

#endif //ZCL_ACE
