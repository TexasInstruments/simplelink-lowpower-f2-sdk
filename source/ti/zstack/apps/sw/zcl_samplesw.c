/**************************************************************************************************
  Filename:       zcl_samplesw.c
  Revised:        $Date: 2015-08-19 17:11:00 -0700 (Wed, 19 Aug 2015) $
  Revision:       $Revision: 44460 $

  Description:    Zigbee Cluster Library - sample switch application.


  Copyright 2006-2013 Texas Instruments Incorporated.

  All rights reserved not granted herein.
  Limited License.

  Texas Instruments Incorporated grants a world-wide, royalty-free,
  non-exclusive license under copyrights and patents it now or hereafter
  owns or controls to make, have made, use, import, offer to sell and sell
  ("Utilize") this software subject to the terms herein. With respect to the
  foregoing patent license, such license is granted solely to the extent that
  any such patent is necessary to Utilize the software alone. The patent
  license shall not apply to any combinations which include this software,
  other than combinations with devices manufactured by or for TI ("TI
  Devices"). No hardware patent is licensed hereunder.

  Redistributions must preserve existing copyright notices and reproduce
  this license (including the above copyright notice and the disclaimer and
  (if applicable) source code license limitations below) in the documentation
  and/or other materials provided with the distribution.

  Redistribution and use in binary form, without modification, are permitted
  provided that the following conditions are met:

    * No reverse engineering, decompilation, or disassembly of this software
      is permitted with respect to any software provided in binary form.
    * Any redistribution and use are licensed by TI for use only with TI Devices.
    * Nothing shall obligate TI to provide you with source code for the software
      licensed and provided to you in object code.

  If software source code is provided to you, modification and redistribution
  of the source code are permitted provided that the following conditions are
  met:

    * Any redistribution and use of the source code, including any resulting
      derivative works, are licensed by TI for use only with TI Devices.
    * Any redistribution and use of any object code compiled from the source
      code and any resulting derivative works, are licensed by TI for use
      only with TI Devices.

  Neither the name of Texas Instruments Incorporated nor the names of its
  suppliers may be used to endorse or promote products derived from this
  software without specific prior written permission.

  DISCLAIMER.

  THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************/

/*********************************************************************
  This application implements a ZigBee On/Off Switch, based on Z-Stack 3.0.

  This application is based on the common sample-application user interface. Please see the main
  comment in zcl_sampleapp_ui.c. The rest of this comment describes only the content specific for
  this sample application.

  Application-specific UI peripherals being used:

  - LEDs:
    LED1 is not used in this application

  Application-specific menu system:
    <TOGGLE LIGHT> Send a toggle command targeting appropriate devices from the binding table.

    <DISCOVER> Sets Switch device into Identify mode.

    The APP Info line will display the following information:
      [Remote Light]
        0xXXXX is On/Off/Unknown- Remote light short address and state of the remote light

*********************************************************************/

#if ! defined ZCL_ON_OFF
#error ZCL_ON_OFF must be defined for this project.
#endif

/*********************************************************************
 * INCLUDES
 */
#include "rom_jt_154.h"
#include "zcomdef.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"

#include "zcl_samplesw.h"
#include "zcl_diagnostic.h"
#include <string.h>

#include "zcl_sampleapps_ui.h"
#include "zcl_sample_app_def.h"
#include "bdb_interface.h"
#include "nwk_util.h"

#if defined (OTA_CLIENT_INTEGRATED)
#include "zcl_ota.h"
#include "ota_client.h"
#endif

#include "ti_drivers_config.h"
#include "nvintf.h"
#include "zstackmsg.h"
#include "zcl_port.h"

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>
#include "zstackapi.h"
#ifndef CUI_DISABLE
#include "cui.h"
#endif
#include <ti/drivers/apps/Button.h>
#include "util_timer.h"

#include "ti_zstack_config.h"
#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
#include "gp_common.h"
#endif

#if defined ( BDB_TL_INITIATOR )
#include "touchlink_initiator_app.h"
#elif defined ( BDB_TL_TARGET )
#include "touchlink_target_app.h"
#endif


#if defined(USE_DMM) && defined(BLE_START)
#include "ti_dmm_application_policy.h"
#include "remote_display.h"
#include "mac_util.h"
#endif // defined(USE_DMM) && defined(BLE_START)

#ifdef PER_TEST
#include "per_test.h"
#endif

#if defined(DMM_ZCSWITCH) && defined(NWK_TOPOLOGY_DISCOVERY)
#include "nwk_discovery.h"
#endif

#if defined (Z_POWER_TEST)
#ifndef Z_POWER_TEST_DATA_TX_INTERVAL
#define Z_POWER_TEST_DATA_TX_INTERVAL 5000
#endif
#endif // Z_POWER_TEST

/*********************************************************************
 * MACROS
 */

//UI definitions



/*********************************************************************
 * TYPEDEFS
 */




/*********************************************************************
 * GLOBAL VARIABLES
 */


uint8_t zclSampleSwSeqNum;

uint8_t zclSampleSw_OnOffSwitchType = ON_OFF_SWITCH_CONFIGURATION_SWITCH_TYPE_MOMENTARY;

uint8_t zclSampleSw_OnOffSwitchActions;

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

static endPointDesc_t  zclSampleSwEpDesc = {0};

#if ZG_BUILD_ENDDEVICE_TYPE
static ClockP_Handle EndDeviceRejoinClkHandle;
static ClockP_Struct EndDeviceRejoinClkStruct;
#endif

#if defined(USE_DMM) && defined(BLE_START)
static ClockP_Struct SyncAttrClkStruct;
#endif // defined(USE_DMM) && defined(BLE_START)

// Passed in function pointers to the NV driver
static NVINTF_nvFuncts_t *pfnZdlNV = NULL;

afAddrType_t zclSampleSw_DstAddr;

#define SAMPLESW_TOGGLE_TEST_EVT   0x1000

#if !defined(CUI_DISABLE)
CONST char zclSampleSw_appStr[] = APP_TITLE_STR;
CUI_clientHandle_t gCuiHandle;
static uint32_t gSampleSwInfoLine;
#endif

#if defined(USE_DMM) && defined(BLE_START) || !defined(CUI_DISABLE)
static uint16_t zclSampleSw_BdbCommissioningModes;
#endif // defined(USE_DMM) && defined(BLE_START) || !defined(CUI_DISABLE)

#ifndef CUI_DISABLE
static uint8_t  remoteLightIsOn = LIGHT_UNKNOWN;
static uint16_t remoteLightAddr = 0xFFFF;
#endif

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void zclSampleSw_initialization(void);
static void zclSampleSw_process_loop(void);
static void zclSampleSw_initParameters(void);
static void zclSampleSw_processZStackMsgs(zstackmsg_genericReq_t *pMsg);
static void SetupZStackCallbacks(void);
static void zclSampleSw_processAfIncomingMsgInd(zstack_afIncomingMsgInd_t *pInMsg);
static void zclSampleSw_initializeClocks(void);
#if ZG_BUILD_ENDDEVICE_TYPE
static void zclSampleSw_processEndDeviceRejoinTimeoutCallback(UArg a0);
#endif
static void zclSampleSw_Init( void );

static void zclSampleSw_BasicResetCB( void );
static void zclSampleSw_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg);

// Functions to process ZCL Foundation incoming Command/Response messages
static uint8_t zclSampleSw_ProcessIncomingMsg( zclIncoming_t *msg );
#ifdef ZCL_READ
static uint8_t zclSampleSw_ProcessInReadRspCmd( zclIncoming_t *pInMsg );
#endif
#ifdef ZCL_WRITE
static uint8_t zclSampleSw_ProcessInWriteRspCmd( zclIncoming_t *pInMsg );
#endif
static uint8_t zclSampleSw_ProcessInDefaultRspCmd( zclIncoming_t *pInMsg );
#ifdef ZCL_DISCOVER
static uint8_t zclSampleSw_ProcessInDiscCmdsRspCmd( zclIncoming_t *pInMsg );
static uint8_t zclSampleSw_ProcessInDiscAttrsRspCmd( zclIncoming_t *pInMsg );
static uint8_t zclSampleSw_ProcessInDiscAttrsExtRspCmd( zclIncoming_t *pInMsg );
#endif

#ifdef ZCL_REPORT_DESTINATION_DEVICE
static void zclSampleSw_ProcessInReportCmd( zclIncoming_t *pInMsg );
#endif

#ifndef CUI_DISABLE
static void zclSampleSw_InitializeStatusLine(CUI_clientHandle_t gCuiHandle);
static void zclSampleSw_UpdateStatusLine(void);
static void zclSampleSw_processKey(uint8_t key, Button_EventMask buttonEvents);
static void zclSampleSw_RemoveAppNvmData(void);
#endif

#if defined (BDB_TL_TARGET) || defined (BDB_TL_INITIATOR)
// Touchlink BDB Finding and Binding callback function
static void tl_BDBFindingBindingCb(void);
#endif // defined ( BDB_TL_TARGET ) || defined (BDB_TL_INITIATOR)

#if defined(USE_DMM) && defined(BLE_START)
// Clock callback functions
static void zclSampleSw_processSyncAttrTimeoutCallback(UArg a0);

// Remote display callback functions
static void setLightAttrCb(RemoteDisplayLightAttr_t lightAttr, void *const value, uint8_t len);
static void getLightAttrCb(RemoteDisplayLightAttr_t lightAttr, void *value, uint8_t len);

// Provisioning callback functions
static void provisionConnectCb(void);
static void provisionDisconnectCb(void);
static void setProvisioningCb(RemoteDisplay_ProvisionAttr_t provisioningAttr, void *const value, uint8_t len);
static void getProvisioningCb(RemoteDisplay_ProvisionAttr_t provisioningAttr, void *value, uint8_t len);

#if defined(DMM_ZCSWITCH) && defined(NWK_TOPOLOGY_DISCOVERY)
static void networkDeviceCb(uint16_t devAddr, union RemoteDisplay_DeviceInfo_t* nDdevInfo);
#endif
#endif // defined(USE_DMM) && defined(BLE_START)

#if defined(DMM_ZCSWITCH) && defined(NWK_TOPOLOGY_DISCOVERY)
static void zclSampleSw_deviceDiscoveryCb(NwkDiscovery_device_t* newDevice);
static void zclSampleSw_postNwkDiscoveryEvent();
#endif

#ifdef DMM_OAD
static void zclSampleSw_dmmPausePolicyCb(uint16_t _pause);
/*********************************************************************
 * DMM Policy Callbacks
 */
static DMMPolicy_AppCbs_t dmmPolicyAppCBs =
{
     zclSampleSw_dmmPausePolicyCb
};
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
static zclGeneral_AppCallbacks_t zclSampleSw_CmdCallbacks =
{
  zclSampleSw_BasicResetCB,               // Basic Cluster Reset command
  NULL,                                   // Identfiy cmd
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

#if defined (BDB_TL_TARGET) || defined (BDB_TL_INITIATOR)
tl_BDBFindingBindingCb_t tl_FindingBindingCb =
{
  tl_BDBFindingBindingCb
};
#endif // defined ( BDB_TL_TARGET ) || defined (BDB_TL_INITIATOR)

#if defined(USE_DMM) && defined(BLE_START)
RemoteDisplay_clientProvisioningtCbs_t zclSwitch_ProvissioningCbs =
{
    setProvisioningCb,
    getProvisioningCb,
    provisionConnectCb,
    provisionDisconnectCb
};

RemoteDisplay_LightCbs_t zclSwitch_LightCbs =
{
    setLightAttrCb,
    getLightAttrCb
};

#if defined(DMM_ZCSWITCH) && defined(NWK_TOPOLOGY_DISCOVERY)
RemoteDisplay_networkDeviceCb_t znetworkDeviceCb =
{
    networkDeviceCb
};
#endif

zstack_DevState provState = zstack_DevState_HOLD;
uint16_t provPanId = ZDAPP_CONFIG_PAN_ID;
uint32_t provChanMask = DEFAULT_CHANLIST;
zstack_sysNwkInfoReadRsp_t *nwkInfo;

DMMPolicy_StackRole DMMPolicy_StackRole_Zigbee =
#if ZG_BUILD_ENDDEVICE_TYPE
    DMMPolicy_StackRole_ZigbeeEndDevice;
#elif ZG_BUILD_RTRONLY_TYPE
    DMMPolicy_StackRole_ZigbeeRouter;
#elif ZG_BUILD_COORDINATOR_TYPE
    DMMPolicy_StackRole_ZigbeeCoordinator;
#endif

#endif // defined(USE_DMM) && defined(BLE_START)

#if defined(DMM_ZCSWITCH) && defined(NWK_TOPOLOGY_DISCOVERY)
NwkDiscovery_clientFnxs zclSampleSw_nwkDiscoveryCbs =
{
    zclSampleSw_deviceDiscoveryCb,
    zclSampleSw_postNwkDiscoveryEvent,
};
#endif

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
  zclSampleSw_initialization();

  // No return from task process
  zclSampleSw_process_loop();
}

/*******************************************************************************
 * @fn          zclSampleSw_initialization
 *
 * @brief       Initialize the application
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleSw_initialization(void)
{
    /* Initialize user clocks */
    zclSampleSw_initializeClocks();

    /* create semaphores for messages / events
     */
    Semaphore_Params semParam;
    Semaphore_Params_init(&semParam);
    semParam.mode = ti_sysbios_knl_Semaphore_Mode_COUNTING;
    Semaphore_construct(&appSem, 0, &semParam);
    appSemHandle = Semaphore_handle(&appSem);

    appServiceTaskId = OsalPort_registerTask(Task_self(), appSemHandle, &appServiceTaskEvents);

    //Initialize stack
    zclSampleSw_Init();

#if defined (OTA_CLIENT_INTEGRATED)
    otaClient_SetEndpoint(SAMPLESW_ENDPOINT);
    otaClient_setAttributes(zclSampleSw_Attrs, zclSampleSw_NumAttributes);
    zclOTA_setAttributes(zclSampleSw_Attrs, zclSampleSw_NumAttributes);
#ifndef CUI_DISABLE
    otaClient_Init ( appSemHandle, appServiceTaskId, gCuiHandle );
#else
    otaClient_Init ( appSemHandle, appServiceTaskId, NULL );
#endif
#endif // OTA_CLIENT_INTEGRATED

#if defined (Z_POWER_TEST)
    zstack_sysSetTxPowerReq_t txPowerReq;
    zstack_sysSetTxPowerRsp_t txPowerRsp;
    txPowerReq.requestedTxPower = POWER_TEST_TX_PWR;
    Zstackapi_sysSetTxPowerReq(appServiceTaskId, &txPowerReq, &txPowerRsp);
    OsalPortTimers_startTimer(appServiceTaskId, SAMPLEAPP_POWER_TEST_START_EVT, 1000);
#endif

#ifdef DMM_OAD
    // register the app callbacks
    DMMPolicy_registerAppCbs(dmmPolicyAppCBs, DMMPolicy_StackRole_Zigbee);
#endif
}



/*********************************************************************
 * @fn          zclSampleSw_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleSw_Init( void )
{
  // Set destination address to indirect
  zclSampleSw_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
  zclSampleSw_DstAddr.endPoint = 0;
  zclSampleSw_DstAddr.addr.shortAddr = 0;

  //Register Endpoint
  zclSampleSwEpDesc.endPoint = SAMPLESW_ENDPOINT;
  zclSampleSwEpDesc.simpleDesc = &zclSampleSw_SimpleDesc;
  zclport_registerEndpoint(appServiceTaskId, &zclSampleSwEpDesc);

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( SAMPLESW_ENDPOINT, &zclSampleSw_CmdCallbacks );

  // Register the application's attribute list and reset to default values
  zclSampleSw_ResetAttributesToDefaultValues();
  zcl_registerAttrList( SAMPLESW_ENDPOINT, zclSampleSw_NumAttributes, zclSampleSw_Attrs );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zclport_registerZclHandleExternal(SAMPLESW_ENDPOINT, zclSampleSw_ProcessIncomingMsg);

  //Write the bdb initialization parameters
  zclSampleSw_initParameters();

  //Setup ZDO callbacks
  SetupZStackCallbacks();

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
  gp_endpointInit(appServiceTaskId);
#endif

#ifdef ZCL_DIAGNOSTIC
  // Register the application's callback function to read/write attribute data.
  // This is only required when the attribute data format is unknown to ZCL.
  zcl_registerReadWriteCB( SAMPLESW_ENDPOINT, zclDiagnostic_ReadWriteAttrCB, NULL );

  if ( zclDiagnostic_InitStats() == ZSuccess )
  {
    // Here the user could start the timer to save Diagnostics to NV
  }
#endif


#ifdef ZCL_DIAGNOSTIC
  // Register the application's callback function to read/write attribute data.
  // This is only required when the attribute data format is unknown to ZCL.
  zcl_registerReadWriteCB( SAMPLESW_ENDPOINT, zclDiagnostic_ReadWriteAttrCB, NULL );

  if ( zclDiagnostic_InitStats() == ZSuccess )
  {
    // Here the user could start the timer to save Diagnostics to NV
  }
#endif

#if defined (BDB_TL_TARGET) || defined (BDB_TL_INITIATOR)
  touchLinkApp_registerFindingBindingCb(tl_FindingBindingCb);
#endif // defined ( BDB_TL_TARGET ) || defined (BDB_TL_INITIATOR)

#if defined(USE_DMM) && defined(BLE_START)
  RemoteDisplay_registerClientProvCbs(zclSwitch_ProvissioningCbs);
  RemoteDisplay_registerLightCbs(zclSwitch_LightCbs);
#if defined(DMM_ZCSWITCH) && defined(NWK_TOPOLOGY_DISCOVERY)
  RemoteDisplay_registerNetworkDeviceCb(znetworkDeviceCb);
#endif
#endif // defined(USE_DMM) && defined(BLE_START)

#if !defined(CUI_DISABLE) || defined(USE_DMM) && defined(BLE_START)
  zclSampleSw_BdbCommissioningModes = DEFAULT_COMISSIONING_MODE;
#endif // !defined(CUI_DISABLE) || defined(USE_DMM) && defined(BLE_START)

#ifndef CUI_DISABLE
#ifdef BDB_TL_INITIATOR
  zclSampleSw_BdbCommissioningModes |= BDB_COMMISSIONING_MODE_INITIATOR_TL;
#endif

  gCuiHandle = UI_Init( appServiceTaskId,                     // Application Task ID
           &appServiceTaskEvents,                // The events processed by the sample application
           appSemHandle,                         // Semaphore to post the events in the application thread
           &zclSampleSw_IdentifyTime,
           &zclSampleSw_BdbCommissioningModes,   // A pointer to the application's bdbCommissioningModes
           zclSampleSw_appStr,                   // A pointer to the app-specific name string
           zclSampleSw_processKey,               // A pointer to the app-specific key process function
           zclSampleSw_RemoveAppNvmData          // A pointer to the app-specific NV Item reset function
           );

  //Initialize the sampleLight UI status line
  zclSampleSw_InitializeStatusLine(gCuiHandle);

#endif // CUI_DISABLE

#if defined ( BDB_TL_INITIATOR )
    touchLinkInitiatorApp_Init(appServiceTaskId);
#elif defined ( BDB_TL_TARGET )
    touchLinkTargetApp_Init(appServiceTaskId);
#endif

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
  app_Green_Power_Init(appServiceTaskId, &appServiceTaskEvents, appSemHandle, SAMPLEAPP_PROCESS_GP_DATA_SEND_EVT,
                       SAMPLEAPP_PROCESS_GP_EXPIRE_DUPLICATE_EVT, SAMPLEAPP_PROCESS_GP_TEMP_MASTER_EVT);
#endif

#ifdef PER_TEST
#ifndef CUI_DISABLE
  PERTest_init( appSemHandle, appServiceTaskId, gCuiHandle );
#else
  PERTest_init( appSemHandle, appServiceTaskId, 0 );
#endif
#endif // PER_TEST

#if defined(DMM_ZCSWITCH) && defined(NWK_TOPOLOGY_DISCOVERY)
  {
      /* Set Default parameters structure
       */
      NwkDiscovery_Params_t nwkDiscovery_params;

      NwkDiscovery_init();

      /* Initialize and open the Network Topology Discovery Module
       */
      NwkDiscovery_Params_init(&nwkDiscovery_params);

      /* Set ServiceTaskId
       */
      nwkDiscovery_params.appServiceTaskId = appServiceTaskId;

      nwkDiscovery_params.nwkDiscoveryPeriod = 300; //re-discover network every 5 minutes
      nwkDiscovery_params.deviceDiscoveryPeriod = 1000; //leave 1s between Lqi Discovery messages

      NwkDiscovery_open(&nwkDiscovery_params);

      NwkDiscovery_registerClientFxns(&zclSampleSw_nwkDiscoveryCbs);
  }
#endif

  // Call BDB initialization. Should be called once from application at startup to restore
  // previous network configuration, if applicable.
  zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;
  zstack_bdbStartCommissioningReq.commissioning_mode = 0;
  Zstackapi_bdbStartCommissioningReq(appServiceTaskId,&zstack_bdbStartCommissioningReq);
}

#ifndef CUI_DISABLE
/*********************************************************************
 * @fn          zclSampleSw_RemoveAppNvmData
 *
 * @brief       Callback when Application performs reset to Factory New Reset.
 *              Application must restore the application to default values
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleSw_RemoveAppNvmData(void)
{

}
#endif

static void zclSampleSw_initParameters(void)
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
 * @fn      zclSampleSw_initializeClocks
 *
 * @brief   Initialize Clocks
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleSw_initializeClocks(void)
{
#if ZG_BUILD_ENDDEVICE_TYPE
    // Initialize the timers needed for this application
    EndDeviceRejoinClkHandle = UtilTimer_construct(
    &EndDeviceRejoinClkStruct,
    zclSampleSw_processEndDeviceRejoinTimeoutCallback,
    SAMPLEAPP_END_DEVICE_REJOIN_DELAY,
    0, false, 0);
#endif
#if defined(USE_DMM) && defined(BLE_START) && !defined(Z_POWER_TEST)
    // Clock for synchronizing application configuration parameters for BLE
    UtilTimer_construct(
    &SyncAttrClkStruct,
    zclSampleSw_processSyncAttrTimeoutCallback,
    SAMPLEAPP_CONFIG_SYNC_TIMEOUT,
    SAMPLEAPP_CONFIG_SYNC_TIMEOUT, true, 0);
#endif // defined(USE_DMM) && defined(BLE_START) && !defined(Z_POWER_TEST)
}

#if ZG_BUILD_ENDDEVICE_TYPE
/*******************************************************************************
 * @fn      zclSampleSw_processEndDeviceRejoinTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleSw_processEndDeviceRejoinTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    appServiceTaskEvents |= SAMPLEAPP_END_DEVICE_REJOIN_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}
#endif

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
#if defined(USE_DMM) && defined(BLE_START)
    zdoCBReq.has_activeEndpointRsp = true;
    zdoCBReq.activeEndpointRsp = true;
    zdoCBReq.has_simpleDescRsp = true;
    zdoCBReq.simpleDescRsp = true;
#endif // defined(USE_DMM) && defined(BLE_START)
#if defined(OTA_CLIENT_INTEGRATED)
    zdoCBReq.has_matchDescRsp = true;
    zdoCBReq.matchDescRsp = true;
    zdoCBReq.has_ieeeAddrRsp = true;
    zdoCBReq.ieeeAddrRsp = true;
#endif
#if defined(DMM_ZCSWITCH) && defined(NWK_TOPOLOGY_DISCOVERY)
    zdoCBReq.has_mgmtLqiRsp = true;
    zdoCBReq.mgmtLqiRsp = true;
    zdoCBReq.has_deviceAnnounce = true;
    zdoCBReq.deviceAnnounce = true;
    zdoCBReq.has_matchDescRsp = true;
    zdoCBReq.matchDescRsp = true;
#endif
    (void)Zstackapi_DevZDOCBReq(appServiceTaskId, &zdoCBReq);
}



/*******************************************************************************
 * @fn      zclSampleSw_process_loop
 *
 * @brief   Application task processing start.
 *
 * @param   none
 *
 * @return  void
 */
static void zclSampleSw_process_loop(void)
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
                zclSampleSw_processZStackMsgs(pMsg);
#ifdef PER_TEST
                PERTest_processZStackMsg(pMsg);
#endif
                // Free any separately allocated memory
                msgProcessed = Zstackapi_freeIndMsg(pMsg);
            }

            if((msgProcessed == FALSE) && (pMsg != NULL))
            {
                OsalPort_msgDeallocate((uint8_t*)pMsg);
            }

#ifdef PER_TEST
            PERTest_process();
#endif
#if defined (OTA_CLIENT_INTEGRATED)
            otaClient_event_loop();
#endif

#ifndef CUI_DISABLE
            zclsampleApp_ui_event_loop();
#endif

#if defined (BDB_TL_TARGET) || defined (BDB_TL_INITIATOR)
            if(appServiceTaskEvents & TL_BDB_FB_EVT)
            {
                zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;
                zstack_bdbStartCommissioningReq.commissioning_mode = BDB_COMMISSIONING_MODE_FINDING_BINDING;
                Zstackapi_bdbStartCommissioningReq(appServiceTaskId, &zstack_bdbStartCommissioningReq);
                appServiceTaskEvents &= ~TL_BDB_FB_EVT;
            }
#endif // defined ( BDB_TL_TARGET ) || defined (BDB_TL_INITIATOR)

#if defined(USE_DMM) && defined(BLE_START)
            if(appServiceTaskEvents & SAMPLEAPP_PROV_CONNECT_EVT)
            {
                zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;
                zstack_bdbStartCommissioningReq.commissioning_mode = zclSampleSw_BdbCommissioningModes;
                Zstackapi_bdbStartCommissioningReq(appServiceTaskId, &zstack_bdbStartCommissioningReq);
                appServiceTaskEvents &= ~SAMPLEAPP_PROV_CONNECT_EVT;
            }

            if(appServiceTaskEvents & SAMPLEAPP_PROV_DISCONNECT_EVT)
            {
                Zstackapi_bdbResetLocalActionReq(appServiceTaskId);

                appServiceTaskEvents &= ~SAMPLEAPP_PROV_DISCONNECT_EVT;
            }

            if(appServiceTaskEvents & SAMPLEAPP_POLICY_UPDATE_EVT)
            {
               static uint32_t stackState = DMMPOLICY_ZB_UNINIT;

               RemoteDisplay_updateJoinState((RemoteDisplay_DevState)provState);

               // If uninitialized
               if( (provState < zstack_DevState_INIT) && (stackState != DMMPOLICY_ZB_UNINIT) )
               {
                   DMMPolicy_updateApplicationState(DMMPolicy_StackRole_Zigbee, DMMPOLICY_ZB_UNINIT);
               }
               // If provisioning
               else if( (((provState > zstack_DevState_INIT) && (provState < zstack_DevState_DEV_ZB_COORD)) ||
                         (provState > zstack_DevState_DEV_ZB_COORD) ) &&
                        (stackState != DMMPOLICY_ZB_PROVISIONING) )
               {
                   DMMPolicy_updateApplicationState(DMMPolicy_StackRole_Zigbee, DMMPOLICY_ZB_PROVISIONING);
               }
               // If connected
               else if( (provState == zstack_DevState_DEV_ZB_COORD) &&
                        (stackState != DMMPOLICY_ZB_CONNECTED) )
               {
                   DMMPolicy_updateApplicationState(DMMPolicy_StackRole_Zigbee, DMMPOLICY_ZB_CONNECTED);
               }

               appServiceTaskEvents &= ~SAMPLEAPP_POLICY_UPDATE_EVT;
            }

            // Read most recent application attributes
            if(appServiceTaskEvents & SAMPLEAPP_SYNC_ATTR_EVT)
            {
                zstack_sysConfigReadReq_t readReq = {0};
                zstack_sysConfigReadRsp_t pRsp = {0};

                readReq.panID = true;
                readReq.chanList = true;

                if (Zstackapi_sysConfigReadReq(appServiceTaskId, &readReq, &pRsp) == zstack_ZStatusValues_ZSuccess) {

                    if (pRsp.has_panID == true) {
                        provPanId = pRsp.panID;
                    }
                    if (pRsp.has_chanList == true) {
                        provChanMask = pRsp.chanList;
                    }
                    // Synchronize new data with BLE application
                    RemoteDisplay_updateProvProfData();
                }
                appServiceTaskEvents &= ~SAMPLEAPP_SYNC_ATTR_EVT;
            }

            if(appServiceTaskEvents & SAMPLEAPP_GET_NWK_INFO_EVT)
            {
                // Obtain active endpoints for the target address
                zstack_zdoActiveEndpointReq_t pReq;
                // Destination address
                pReq.dstAddr = zclSampleSw_DstAddr.addr.shortAddr;
                // Network address is the destination for endpoint discovery
                pReq.nwkAddrOfInterest = zclSampleSw_DstAddr.addr.shortAddr;

                Zstackapi_ZdoActiveEndpointReq(appServiceTaskId, &pReq);

                appServiceTaskEvents &= ~SAMPLEAPP_GET_NWK_INFO_EVT;
            }
#endif //  defined(USE_DMM) && defined(BLE_START)

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
#if defined (Z_POWER_TEST)
            if ( appServiceTaskEvents & SAMPLEAPP_POWER_TEST_START_EVT )
            {
              zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;
#if defined (POWER_TEST_POLL_ACK) || defined (POWER_TEST_POLL_DATA)
              // assuming we are ZED in this test, we must search for + join a network
              zstack_bdbStartCommissioningReq.commissioning_mode = BDB_COMMISSIONING_MODE_NWK_STEERING;
              Zstackapi_bdbStartCommissioningReq(appServiceTaskId,&zstack_bdbStartCommissioningReq);
#elif defined (POWER_TEST_DATA_ACK)
              // assuming we are ZC in this test, we must search for + join a network + start finding&binding
              zstack_bdbStartCommissioningReq.commissioning_mode = BDB_COMMISSIONING_MODE_NWK_STEERING;
              Zstackapi_bdbStartCommissioningReq(appServiceTaskId,&zstack_bdbStartCommissioningReq);
#endif
              appServiceTaskEvents &= ~SAMPLEAPP_POWER_TEST_START_EVT;
            }
#if defined (POWER_TEST_DATA_ACK)
            if ( appServiceTaskEvents & SAMPLEAPP_POWER_TEST_TOGGLE_EVT )
            {
              zstack_getZCLFrameCounterRsp_t Rsp;

              Zstackapi_getZCLFrameCounterReq(appServiceTaskId, &Rsp);
              zclGeneral_SendOnOff_CmdToggle( SAMPLESW_ENDPOINT, &zclSampleSw_DstAddr, TRUE, Rsp.zclFrameCounter );

              appServiceTaskEvents &= ~SAMPLEAPP_POWER_TEST_TOGGLE_EVT;
            }
#endif
#endif // Z_POWER_TEST
#if defined(DMM_ZCSWITCH) && defined(NWK_TOPOLOGY_DISCOVERY)
            if ( appServiceTaskEvents & SAMPLEAPP_NWK_DISC_EVT )
            {
                NwkDiscovery_processEvents();
                appServiceTaskEvents &= ~SAMPLEAPP_NWK_DISC_EVT;
            }
#endif
        }
    }
}

#if defined(DMM_ZCSWITCH) && defined(NWK_TOPOLOGY_DISCOVERY)
static void zclSampleSw_deviceDiscoveryCb(NwkDiscovery_device_t* newDevice)
{
#if defined(USE_DMM) && defined(BLE_START)
    RemoteDisplay_deviceUpdate(newDevice->nwkAddr);
#endif // defined(USE_DMM) && defined(BLE_START)
}

static void zclSampleSw_postNwkDiscoveryEvent()
{
 appServiceTaskEvents |= SAMPLEAPP_NWK_DISC_EVT;
 // Wake up the application thread when it waits for clock event
 Semaphore_post(appSemHandle);
}
#endif

#if defined (BDB_TL_TARGET) || defined (BDB_TL_INITIATOR)
static void tl_BDBFindingBindingCb(void)
{
  OsalPortTimers_startTimer(appServiceTaskId, TL_BDB_FB_EVT, TL_BDB_FB_DELAY);
}
#endif // defined ( BDB_TL_TARGET ) || defined (BDB_TL_INITIATOR)

#if defined(USE_DMM) && defined(BLE_START)
static void provisionConnectCb(void)
{
    appServiceTaskEvents |= SAMPLEAPP_PROV_CONNECT_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}

static void provisionDisconnectCb(void)
{
    appServiceTaskEvents |= SAMPLEAPP_PROV_DISCONNECT_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}

/** @brief  Set provisioning callback functions
 *
 *  @param  ProvisionAttr_t  Remote display attribute value to set
 *  @param  value  pointer to data from remote dispaly application
 *  @param  len  length of data from remote display application
 */
static void setProvisioningCb(RemoteDisplay_ProvisionAttr_t provisioningAttr,
    void *const value, uint8_t len)
{
    uint8_t *byteArr = (uint8_t *)value;

    switch(provisioningAttr)
    {
        case ProvisionAttr_PanId:
        {
            provPanId = Util_buildUint16(byteArr[1], byteArr[0]);

            zstack_sysConfigWriteReq_t writeReq = {0};
            writeReq.has_panID = true;
            writeReq.panID = provPanId;
            Zstackapi_sysConfigWriteReq(appServiceTaskId, &writeReq);
            break;
        }
        case ProvisionAttr_SensorChannelMask:
        {
            provChanMask = Util_buildUint32(byteArr[3], byteArr[2],
                                              byteArr[1], byteArr[0]);

            zstack_bdbSetAttributesReq_t req = {0};
            req.bdbPrimaryChannelSet = provChanMask;
            req.has_bdbPrimaryChannelSet = true;
            Zstackapi_bdbSetAttributesReq(appServiceTaskId, &req);
            break;
        }
        default:
            // Attribute not found
            break;
        }
}

/** @brief  Get provisioning callback functions
 *
 *  @param  ProvisionAttr_t  Remote display attribute value to set
 *
 *  @return  uint8_t  Current value of data present in 15.4 application
 */
static void getProvisioningCb(RemoteDisplay_ProvisionAttr_t provisioningAttr, void *value, uint8_t len)
{
    switch(provisioningAttr)
    {
        case ProvisionAttr_ProvState:
        {
            *(uint8_t *)value = provState;
            break;
        }
        // The PAN ID and Channel mask are reversed in byte order below
        // to allow the Light Blue BLE phone application to parse this data properly.
        case ProvisionAttr_PanId:
        {
            ((uint8_t *)value)[0] = Util_hiUint16(provPanId);
            ((uint8_t *)value)[1] = Util_loUint16(provPanId);
            break;
        }
        case ProvisionAttr_ExtPanId:
        {
            zstack_sysConfigReadReq_t readReq = {0};
            zstack_sysConfigReadRsp_t pRsp = {0};
            readReq.extendedPANID = true;
            Zstackapi_sysConfigReadReq(appServiceTaskId, &readReq, &pRsp);
            osal_memcpy(value, pRsp.extendedPANID, Z_EXTADDR_LEN);
            break;
        }
        case ProvisionAttr_SensorChannelMask:
        {
            ((uint8_t *)value)[0] = Util_breakUint32(provChanMask, 3);
            ((uint8_t *)value)[1] = Util_breakUint32(provChanMask, 2);
            ((uint8_t *)value)[2] = Util_breakUint32(provChanMask, 1);
            ((uint8_t *)value)[3] = Util_breakUint32(provChanMask, 0);
            break;
        }
        default:
            // Attribute not found
            break;
    }
}

/** @brief  Set remote display callback functions
 *
 *  @param  lightAttr  Remote display attribute value to set
 *  @param  value  pointer to data from remote dispaly application
 *  @param  len  length of data from remote display application
 */
static void setLightAttrCb(RemoteDisplayLightAttr_t lightAttr,
    void *const value, uint8_t len)
{
    switch(lightAttr)
    {
        case LightAttr_Light_OnOff:
        {
            zstack_getZCLFrameCounterRsp_t Rsp;
            Zstackapi_getZCLFrameCounterReq(appServiceTaskId, &Rsp);

            if(*((uint8_t*)value) == 0)
            {
                zclGeneral_SendOnOff_CmdOff( SAMPLESW_ENDPOINT, &zclSampleSw_DstAddr, FALSE, Rsp.zclFrameCounter );
            }
            else
            {
                zclGeneral_SendOnOff_CmdOn( SAMPLESW_ENDPOINT, &zclSampleSw_DstAddr, FALSE, Rsp.zclFrameCounter );
            }

            break;
        }
        case LightAttr_Target_Addr_Type:
        {
            afAddrMode_t newAddrMode = (afAddrMode_t) *((uint8_t*)value);

            // Only Binding table or 16b(direct addressing) address mode is supported
            if( (newAddrMode == afAddrNotPresent) || //Binding table
                (newAddrMode == afAddr16Bit) )
            {
                zclSampleSw_DstAddr.addrMode = newAddrMode;
            }

            break;
        }
        case LightAttr_Target_Addr:
        {
            uint16_t newAddr = Util_buildUint16(((uint8_t *)value)[1], ((uint8_t *)value)[0]);

            zclSampleSw_DstAddr.addr.shortAddr = newAddr;

            appServiceTaskEvents |= SAMPLEAPP_GET_NWK_INFO_EVT;

            // Wake up the application thread when it waits for clock event
            Semaphore_post(appSemHandle);
            break;
        }
        case LightAttr_Target_Endpoint:
        {
            uint8_t newEndpoint = *((uint8_t*)value);
            zclSampleSw_DstAddr.endPoint = newEndpoint;

            break;
        }
        default:
            return;
    }
}

/** @brief  Get remote display callback functions
 *
 *  @param  lightAttr  Remote display attribute value to set
 *
 *  @return  void *  Current value of data present in 15.4 application
 */
static void getLightAttrCb(RemoteDisplayLightAttr_t lightAttr, void *value, uint8_t len)
{
    switch(lightAttr)
    {
        case LightAttr_Light_OnOff:
        {
            /* Getting Lights On/Off value not supported as ZCL read would need to block
             * the Char read
             */
            break;
        }
        case LightAttr_Target_Addr_Type:
        {
            *(uint8_t *)value = zclSampleSw_DstAddr.addrMode;
            break;
        }
        case LightAttr_Target_Addr:
        {
            ((uint8_t *)value)[0] = Util_loUint16(zclSampleSw_DstAddr.addr.shortAddr);
            ((uint8_t *)value)[1] = Util_hiUint16(zclSampleSw_DstAddr.addr.shortAddr);
            break;
        }
        case LightAttr_Target_Endpoint:
        {
            *(uint8_t *)value = zclSampleSw_DstAddr.endPoint;
            break;
        }
        default:
            // Attribute not found
            break;
        }
}

/*******************************************************************************
 * @fn      zclSampleSw_processSyncAttrTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleSw_processSyncAttrTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    appServiceTaskEvents |= SAMPLEAPP_SYNC_ATTR_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}

#if defined(DMM_ZCSWITCH) && defined(NWK_TOPOLOGY_DISCOVERY)
/** @brief  Send 154 network data callback functions
 *
 *  @param  uint16_t devAddr
 *  @param  RemoteDisplay_DeviceInfo_t  Device information
 *
 *  @return  void
 */
static void networkDeviceCb(uint16_t devAddr, union RemoteDisplay_DeviceInfo_t* nDdevInfo)
{
    NwkDeviceListEntry_t* devEntry;
    // 0xFFFF is a special input to indicate that discovery should start again
    if (devAddr == 0xFFFF)
    {
        NwkDiscovery_start();
        // Ensure that this query does not produce a characteristic update
        nDdevInfo->zigbeeDeviceInfo.devAddr = 0xFFFE;
    }
    else
    {
        devEntry = NwkDiscovery_deviceGet(devAddr);

        if (devEntry != NULL)
        {
            nDdevInfo->zigbeeDeviceInfo.devAddr = devEntry->discoveredDevice.nwkAddr;
            nDdevInfo->zigbeeDeviceInfo.lightEndPoint = devEntry->discoveredDevice.lightEndPoint;
            nDdevInfo->zigbeeDeviceInfo.parentAddr = devEntry->discoveredDevice.parentAddress;
            nDdevInfo->zigbeeDeviceInfo.rssi = devEntry->discoveredDevice.rxLqi;
        }
        else
        {
            // Send back an invalid zigbee address
            nDdevInfo->zigbeeDeviceInfo.devAddr = 0xFFFE;
        }
    }
}
#endif

#endif // defined(USE_DMM) && defined(BLE_START)

/*******************************************************************************
 * @fn      zclSampleSw_processZStackMsgs
 *
 * @brief   Process event from Stack
 *
 * @param   pMsg - pointer to incoming ZStack message to process
 *
 * @return  void
 */
static void zclSampleSw_processZStackMsgs(zstackmsg_genericReq_t *pMsg)
{
      switch(pMsg->hdr.event)
      {
          case zstackmsg_CmdIDs_BDB_NOTIFICATION:
              {
                  zstackmsg_bdbNotificationInd_t *pInd;
                  pInd = (zstackmsg_bdbNotificationInd_t*)pMsg;
                  zclSampleSw_ProcessCommissioningStatus(&(pInd->Req));
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
                  zclSampleSw_processAfIncomingMsgInd( &(pInd->req) );
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
              // The ZStack Thread is indicating a State change
#if defined(USE_DMM) && defined(BLE_START) || !defined(CUI_DISABLE)
              zstackmsg_devStateChangeInd_t *pInd = (zstackmsg_devStateChangeInd_t *)pMsg;
#endif // defined(USE_DMM) && defined(BLE_START) || !defined(CUI_DISABLE)

#if !defined(CUI_DISABLE)
              UI_DeviceStateUpdated(&(pInd->req));
              UI_UpdateBdbStatusLine(NULL);
#endif

#if defined(USE_DMM) && defined(BLE_START)
              provState = pInd->req.state;

              if ((provState == zstack_DevState_DEV_END_DEVICE) ||
                 (provState == zstack_DevState_DEV_ROUTER)) {
                  /* Obtain parent address */
                  nwkInfo = zclport_getDeviceInfo(appServiceTaskId);
              }

#if defined(DMM_ZEDSWITCH)
              RemoteDisplay_updateJoinState((RemoteDisplay_DevState)provState);
#elif defined(DMM_ZCSWITCH)
              appServiceTaskEvents |= SAMPLEAPP_POLICY_UPDATE_EVT;

              // Wake up the application thread when it waits for clock event
              Semaphore_post(appSemHandle);
#endif // defined(USE_DMM) && defined(BLE_START)

#endif
#if defined(DMM_ZCSWITCH) && defined(NWK_TOPOLOGY_DISCOVERY)
              if(pInd->req.state == zstack_DevState_DEV_ZB_COORD)
              {
                  NwkDiscovery_start();
              }
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
#ifndef CUI_DISABLE
              UI_UpdateBdbStatusLine(NULL);
#endif
          }
          break;
#endif
          case zstackmsg_CmdIDs_ZDO_MATCH_DESC_RSP:
#if defined (OTA_CLIENT_INTEGRATED)
          {
            zstackmsg_zdoMatchDescRspInd_t  *pInd =
               (zstackmsg_zdoMatchDescRspInd_t *) pMsg;

            ZDO_MatchDescRsp_t              *pRsp;

            /*
             * Parse the Match Descriptor Response and give it
             * to the ZCL EZMode module t+o process
             */
            pRsp = (ZDO_MatchDescRsp_t *) OsalPort_msgAllocate(
                   sizeof(ZDO_MatchDescRsp_t) + pInd->rsp.n_matchList);
            if(pRsp)
            {
              pRsp->status = pInd->rsp.status;
              pRsp->nwkAddr = pInd->rsp.nwkAddrOfInterest;
              pRsp->cnt = pInd->rsp.n_matchList;
              OsalPort_memcpy(pRsp->epList, pInd->rsp.pMatchList, pInd->rsp.n_matchList);

              otaClient_ProcessMatchDescRsp ( pRsp );
              OsalPort_msgDeallocate((uint8_t *)pRsp);
            }
          }
          break;
#elif defined(DMM_ZCSWITCH) && defined(NWK_TOPOLOGY_DISCOVERY)
          {
            zstackmsg_zdoMatchDescRspInd_t  *pInd =
               (zstackmsg_zdoMatchDescRspInd_t *) pMsg;
            NwkDiscovery_processMatchDescRspInd(pInd);
          }
          break;
#else
          {

          }
          break;
#endif
          case zstackmsg_CmdIDs_ZDO_SIMPLE_DESC_RSP:
#if defined(USE_DMM) && defined(BLE_START)
          {
              uint8_t i;
              zstackmsg_zdoSimpleDescRspInd_t *pInd;
              pInd = (zstackmsg_zdoSimpleDescRspInd_t*)pMsg;
              // Set endpoint if not set by user
              if (zclSampleSw_DstAddr.endPoint == 0x00) {
                  for (i = 0; i < pInd->rsp.simpleDesc.n_inputClusters; i++) {
                      if (pInd->rsp.simpleDesc.pInputClusters[i] == ZCL_CLUSTER_ID_GENERAL_ON_OFF)
                      {
                          zclSampleSw_DstAddr.endPoint = pInd->rsp.simpleDesc.endpoint;
                          // Notify BLE application of change
                          RemoteDisplay_updateLightProfData();
                      }
                  }
              }
          }
          break;
#else
          {

          }
          break;
#endif // defined(USE_DMM) && defined(BLE_START)

          case zstackmsg_CmdIDs_ZDO_ACTIVE_EP_RSP:
#if defined(USE_DMM) && defined(BLE_START)
          {
              uint8_t i;
              zstackmsg_zdoActiveEndpointsRspInd_t *pInd;
              pInd = (zstackmsg_zdoActiveEndpointsRspInd_t*)pMsg;

              zstack_zdoSimpleDescReq_t pReq;
              pReq.dstAddr = pInd->rsp.srcAddr;
              pReq.nwkAddrOfInterest = pInd->rsp.nwkAddrOfInterest;
              // Send a simple descriptor request for each endpoint found
              for(i = 0; i < pInd->rsp.n_activeEPList; i++) {
                  pReq.endpoint = pInd->rsp.pActiveEPList[i];
                  Zstackapi_ZdoSimpleDescReq(appServiceTaskId, &pReq);
              }
          }
          break;
#else
          {

          }
          break;
#endif // defined(USE_DMM) && defined(BLE_START)
          case zstackmsg_CmdIDs_ZDO_MGMT_LQI_RSP:
#if defined(DMM_ZCSWITCH) && defined(NWK_TOPOLOGY_DISCOVERY)
          {
              zstackmsg_zdoMgmtLqiRspInd_t *pInd;
              pInd = (zstackmsg_zdoMgmtLqiRspInd_t*)pMsg;
              NwkDiscovery_processMgmtLqiRspInd(pInd);
          }
          break;
#else
          {

          }
          break;
#endif

          case zstackmsg_CmdIDs_ZDO_DEVICE_ANNOUNCE:
#if defined(DMM_ZCSWITCH) && defined(NWK_TOPOLOGY_DISCOVERY)
          {
              NwkDiscovery_start();
          }
          break;
    #else
          {

          }
          break;
    #endif
          case zstackmsg_CmdIDs_DEV_PERMIT_JOIN_IND:
          case zstackmsg_CmdIDs_BDB_TC_LINK_KEY_EXCHANGE_NOTIFICATION_IND:
          case zstackmsg_CmdIDs_AF_DATA_CONFIRM_IND:
          case zstackmsg_CmdIDs_ZDO_NWK_ADDR_RSP:
          case zstackmsg_CmdIDs_ZDO_IEEE_ADDR_RSP:
          case zstackmsg_CmdIDs_ZDO_NODE_DESC_RSP:
          case zstackmsg_CmdIDs_ZDO_POWER_DESC_RSP:
          case zstackmsg_CmdIDs_ZDO_COMPLEX_DESC_RSP:
          case zstackmsg_CmdIDs_ZDO_USER_DESC_RSP:
          case zstackmsg_CmdIDs_ZDO_USER_DESC_SET_RSP:
          case zstackmsg_CmdIDs_ZDO_SERVER_DISC_RSP:
          case zstackmsg_CmdIDs_ZDO_END_DEVICE_BIND_RSP:
          case zstackmsg_CmdIDs_ZDO_BIND_RSP:
          case zstackmsg_CmdIDs_ZDO_UNBIND_RSP:
          case zstackmsg_CmdIDs_ZDO_MGMT_NWK_DISC_RSP:
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
 * @fn          zclSampleSw_processAfIncomingMsgInd
 *
 * @brief       Process AF Incoming Message Indication message
 *
 * @param       pInMsg - pointer to incoming message
 *
 * @return      none
 *
 */
static void zclSampleSw_processAfIncomingMsgInd(zstack_afIncomingMsgInd_t *pInMsg)
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
 * @fn      zclSampleSw_ProcessCommissioningStatus
 *
 * @brief   Callback in which the status of the commissioning process are reported
 *
 * @param   bdbCommissioningModeMsg - Context message of the status of a commissioning process
 *
 * @return  none
 */
static void zclSampleSw_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg)
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
#if defined (Z_POWER_TEST)
#if defined (POWER_TEST_POLL_ACK) || defined (POWER_TEST_POLL_DATA)
        // set poll rate to POLL_RATE after joining
        zstack_sysConfigWriteReq_t writeReq = { 0 };
        // Set the new poll rates
        writeReq.has_pollRate = true;
        writeReq.pollRate = POLL_RATE;
        writeReq.pollRateType = POLL_RATE_TYPE_DEFAULT;
        Zstackapi_sysConfigWriteReq(appServiceTaskId, &writeReq);
        // disable response and queued poll rates
        writeReq.pollRate = POLL_RATE_MAX;
        writeReq.pollRateType = POLL_RATE_TYPE_QUEUED;
        Zstackapi_sysConfigWriteReq(appServiceTaskId, &writeReq);
        writeReq.pollRateType = POLL_RATE_TYPE_RESPONSE;
        Zstackapi_sysConfigWriteReq(appServiceTaskId, &writeReq);
#endif
#if defined (POWER_TEST_DATA_ACK)
        // we have created a bind w/ the light now, so we can disable polling
        zstack_sysConfigWriteReq_t writeReq = { 0 };
        writeReq.has_disablePollRate = true;
        writeReq.disablePollRate = true;

        Zstackapi_sysConfigWriteReq(appServiceTaskId, &writeReq);

        // instead of using binds, directly address ZC
        zclSampleSw_DstAddr.endPoint = 0x08; // hard-code to default samplelight endpoint
        zclSampleSw_DstAddr.addrMode = afAddr16Bit;
        zclSampleSw_DstAddr.addr.shortAddr = 0x0000; // hard-code to ZC address

        // start 5 second periodic timer for sending zcl on/off toggle to ZC (0x0000)
        OsalPortTimers_startReloadTimer(appServiceTaskId, SAMPLEAPP_POWER_TEST_TOGGLE_EVT, Z_POWER_TEST_DATA_TX_INTERVAL);
#endif
#endif // Z_POWER_TEST

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
 * @fn      zclSampleSw_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to  default values.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleSw_BasicResetCB( void )
{
  zclSampleSw_ResetAttributesToDefaultValues();
}


/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/

/*********************************************************************
 * @fn      zclSampleSw_ProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  uint8_t - TRUE if got handled
 */
static uint8_t zclSampleSw_ProcessIncomingMsg( zclIncoming_t *pInMsg )
{
  uint8_t handled = FALSE;

  switch ( pInMsg->hdr.commandID )
  {
#ifdef ZCL_READ
    case ZCL_CMD_READ_RSP:
      zclSampleSw_ProcessInReadRspCmd( pInMsg );
      handled = TRUE;
      break;
#endif
#ifdef ZCL_WRITE
    case ZCL_CMD_WRITE_RSP:
      zclSampleSw_ProcessInWriteRspCmd( pInMsg );
      handled = TRUE;
      break;
#endif
#ifdef ZCL_REPORT_DESTINATION_DEVICE
    // See ZCL Test Applicaiton (zcl_testapp.c) for sample code on Attribute Reporting
    case ZCL_CMD_CONFIG_REPORT:
      //zclSampleSw_ProcessInConfigReportCmd( pInMsg );
      break;

    case ZCL_CMD_CONFIG_REPORT_RSP:
      //zclSampleSw_ProcessInConfigReportRspCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG:
      //zclSampleSw_ProcessInReadReportCfgCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG_RSP:
      //zclSampleSw_ProcessInReadReportCfgRspCmd( pInMsg );
      break;

    case ZCL_CMD_REPORT:
      zclSampleSw_ProcessInReportCmd( pInMsg );
      handled = TRUE;
      break;
#endif
    case ZCL_CMD_DEFAULT_RSP:
      zclSampleSw_ProcessInDefaultRspCmd( pInMsg );
      handled = TRUE;
      break;
#ifdef ZCL_DISCOVER
    case ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP:
      zclSampleSw_ProcessInDiscCmdsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_CMDS_GEN_RSP:
      zclSampleSw_ProcessInDiscCmdsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_ATTRS_RSP:
      zclSampleSw_ProcessInDiscAttrsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_ATTRS_EXT_RSP:
      zclSampleSw_ProcessInDiscAttrsExtRspCmd( pInMsg );
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
 * @fn      zclSampleSw_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleSw_ProcessInReadRspCmd( zclIncoming_t *pInMsg )
{
  zclReadRspCmd_t *readRspCmd;
  uint8_t i;

  readRspCmd = (zclReadRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < readRspCmd->numAttr; i++)
  {
    // Notify the originator of the results of the original read attributes
    // attempt and, for each successful request, the value of the requested
    // attribute
  }

  return TRUE;
}
#endif // ZCL_READ

#ifdef ZCL_WRITE
/*********************************************************************
 * @fn      zclSampleSw_ProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleSw_ProcessInWriteRspCmd( zclIncoming_t *pInMsg )
{
  zclWriteRspCmd_t *writeRspCmd;
  uint8_t i;

  writeRspCmd = (zclWriteRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < writeRspCmd->numAttr; i++)
  {
    // Notify the device of the results of the its original write attributes
    // command.
  }

  return TRUE;
}
#endif // ZCL_WRITE

#ifdef ZCL_REPORT_DESTINATION_DEVICE
/*********************************************************************
 * @fn      zclSampleSw_ProcessInReportCmd
 *
 * @brief   Process the "Profile" Report Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static void zclSampleSw_ProcessInReportCmd( zclIncoming_t *pInMsg )
{
  zclReportCmd_t *pInSwReport;

  pInSwReport = (zclReportCmd_t *)pInMsg->attrCmd;

  if ( pInSwReport->attrList[0].attrID != ATTRID_ON_OFF_ON_OFF )
  {
    return;
  }

#ifndef CUI_DISABLE
  // read the Light state and display the information
  if ( pInSwReport->attrList[0].attrData[0] == LIGHT_ON )
  {
    // On
      remoteLightIsOn = TRUE;
  }
  else if ( pInSwReport->attrList[0].attrData[0] == LIGHT_OFF )
  {
    // Off
      remoteLightIsOn = FALSE;
  }
  remoteLightAddr = pInMsg->msg->srcAddr.addr.shortAddr;

  //Update status line
  zclSampleSw_UpdateStatusLine();
#endif
}
#endif

/*********************************************************************
 * @fn      zclSampleSw_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleSw_ProcessInDefaultRspCmd( zclIncoming_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;
  // Device is notified of the Default Response command.
  (void)pInMsg;
  return TRUE;
}

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      zclSampleSw_ProcessInDiscCmdsRspCmd
 *
 * @brief   Process the Discover Commands Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleSw_ProcessInDiscCmdsRspCmd( zclIncoming_t *pInMsg )
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
 * @fn      zclSampleSw_ProcessInDiscAttrsRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleSw_ProcessInDiscAttrsRspCmd( zclIncoming_t *pInMsg )
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
 * @fn      zclSampleSw_ProcessInDiscAttrsExtRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Extended Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleSw_ProcessInDiscAttrsExtRspCmd( zclIncoming_t *pInMsg )
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



/****************************************************************************
****************************************************************************/

void zclSampleSw_actionToggleLight(const int32_t _itemEntry)
{
    zstack_getZCLFrameCounterRsp_t Rsp;

    Zstackapi_getZCLFrameCounterReq(appServiceTaskId, &Rsp);

    zclGeneral_SendOnOff_CmdToggle( SAMPLESW_ENDPOINT, &zclSampleSw_DstAddr, TRUE, Rsp.zclFrameCounter );
}


void zclSampleSw_UiActionSwDiscoverable(const int32_t _itemEntry)
{

    zstack_sysNwkInfoReadRsp_t  Rsp;

    zstack_getZCLFrameCounterRsp_t zclCounterRsp;
    afAddrType_t dstAddr;

    Zstackapi_getZCLFrameCounterReq(appServiceTaskId, &zclCounterRsp);

    //Get our short address
    Zstackapi_sysNwkInfoReadReq(appServiceTaskId, &Rsp);

    dstAddr.endPoint = SAMPLESW_ENDPOINT;
    dstAddr.addrMode = afAddr16Bit;
    dstAddr.addr.shortAddr = Rsp.nwkAddr;

    zclGeneral_SendIdentify(SAMPLESW_ENDPOINT, &dstAddr,60, TRUE, zclCounterRsp.zclFrameCounter);
}

#ifndef CUI_DISABLE
/*********************************************************************
 * @fn      zclSampleSw_processKey
 *
 * @brief   Key event handler function
 *
 * @param   key - key to handle action for
 *          buttonEvents - event to handle action for
 *
 * @return  none
 */
static void zclSampleSw_processKey(uint8_t key, Button_EventMask buttonEvents)
{
    if (buttonEvents & Button_EV_CLICKED)
    {
        if(key == CONFIG_BTN_LEFT)
        {
            zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;

            zstack_bdbStartCommissioningReq.commissioning_mode = zclSampleSw_BdbCommissioningModes;
            Zstackapi_bdbStartCommissioningReq(appServiceTaskId,&zstack_bdbStartCommissioningReq);
        }
        if(key == CONFIG_BTN_RIGHT)
        {
            zstack_getZCLFrameCounterRsp_t rsp;

            Zstackapi_getZCLFrameCounterReq(appServiceTaskId, &rsp);
            zclGeneral_SendOnOff_CmdToggle( SAMPLESW_ENDPOINT, &zclSampleSw_DstAddr, FALSE, rsp.zclFrameCounter );
        }
    }
}

static void zclSampleSw_InitializeStatusLine(CUI_clientHandle_t cuiHandle)
{
    /* Request Async Line for Light application Info */
    CUI_statusLineResourceRequest(cuiHandle, "   APP Info"CUI_DEBUG_MSG_START"1"CUI_DEBUG_MSG_END, false, &gSampleSwInfoLine);

    zclSampleSw_UpdateStatusLine();
}



static void zclSampleSw_UpdateStatusLine(void)
{
    char lineFormat[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};

    strcpy(lineFormat, "["CUI_COLOR_YELLOW"Remote Light"CUI_COLOR_RESET"] 0x%04x ");
    if(remoteLightIsOn == LIGHT_UNKNOWN)
    {
        strcat(lineFormat, "state is Unknown");
    }
    else if(remoteLightIsOn == LIGHT_ON)
    {
        strcat(lineFormat, "is "CUI_COLOR_GREEN"On"CUI_COLOR_RESET);
    }
    else
    {
        strcat(lineFormat, "is "CUI_COLOR_RED"Off"CUI_COLOR_RESET);
    }

    CUI_statusLinePrintf(gCuiHandle, gSampleSwInfoLine, lineFormat, remoteLightAddr);
}

#endif // CUI_DISABLE

#ifdef DMM_OAD
/*********************************************************************
 * @fn      zclSampleSw_dmmPausePolicyCb
 *
 * @brief   DMM Policy callback to pause the stack
 */
static void zclSampleSw_dmmPausePolicyCb(uint16_t pause)
{
    zstack_pauseResumeDeviceReq_t zstack_pauseResumeDeviceReq;
    zstack_pauseResumeDeviceReq.pause = pause;
    Zstackapi_pauseResumeDeviceReq(appServiceTaskId, &zstack_pauseResumeDeviceReq);
}
#endif
