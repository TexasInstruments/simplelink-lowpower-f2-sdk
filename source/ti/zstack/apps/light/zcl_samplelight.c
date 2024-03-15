/**************************************************************************************************
  Filename:       zcl_sampleLight.c
  Revised:        $Date: 2014-10-24 16:04:46 -0700 (Fri, 24 Oct 2014) $
  Revision:       $Revision: 40796 $


  Description:    Zigbee Cluster Library - sample light application.


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

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
  This application implements a ZigBee Light, based on Z-Stack 3.0. It can be configured as an
  On/Off light or as a dimmable light, by undefining or defining ZCL_LEVEL_CTRL, respectively.

  This application is based on the common sample-application user interface. Please see the main
  comment in zcl_sampleapp_ui.c. The rest of this comment describes only the content specific for
  this sample application.

  Application-specific UI peripherals being used:

  - LEDs:
    LED1 reflect the current light state (On / Off accordingly).

  Application-specific menu system:

    <TOGGLE LIGHT> Toggle the local light and display its status and level
      Press ENTER to toggle the local light on and off.
      Note when ZCL_LEVEL_CTRL is enabled:
        - If the light state is ON and the light level is X, and then the light receives the OFF or TOGGLE
          commands: The level will decrease gradually until it reaches 1, and only then the light state will
          be changed to OFF. The level then will be restored to X, with the state staying OFF. At this stage
          the light is not lighting, and the level represent the target level for the next ON or TOGGLE
          commands.
        - If the light state is OFF and the light level is X, and then the light receives the ON or TOGGLE
          commands; The level will be set to 1, the light state will be set to ON, and then the level will
          increase gradually until it reaches level X.
        - Any level-setting command will affect the level directly, and may also affect the on/off state,
          depending on the command's arguments.

    <DISCOVER> Sends Identify Query to start discovery mechanism for creating bind to a Switch
      Press ENTER to make the Light send an Identify Query request.
      When Switch devices respond to the query, the Light shall discover any missing information
        from the responding device to form the bind, and afterwards will begin sending reports
        (when BDB_REPORTING is enabled) every 3-10 seconds (by default). This is modifiable by
        setting the minReportInt and maxReportInt.

    The APP Info line will display the following information:
      [Light State]
        On/Off - current state of the local light
      [Level] - Only populated if ZCL_LEVEL_CTRL is defined. Shows current level of light
        if light state is ON, or the target level to be set to when switched from off to on
        using the on or toggle command.

*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "rom_jt_154.h"
#include "zcomdef.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"

#include "zcl_samplelight.h"
#include <string.h>
#include "bdb_interface.h"

#include "ti_drivers_config.h"
#include "nvintf.h"
#include "zstackmsg.h"
#include "zcl_port.h"

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>
#include "zstackapi.h"
#include "util_timer.h"

#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>

#ifndef CUI_DISABLE
#include "zcl_sampleapps_ui.h"
#include "zcl_sample_app_def.h"
#endif

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
#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
#include "gp_sink.h"
#endif


#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
#if ZG_BUILD_ENDDEVICE_TYPE
#error: End devices cannot have Green Power Combo enabled
#endif
#endif
/*********************************************************************
 * MACROS
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

#ifdef BDB_REPORTING
#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 8
  uint8_t reportableChange[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif
#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 4
  uint8_t reportableChange[] = {0x01, 0x00, 0x00, 0x00};
#endif
#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 2
  uint8_t reportableChange[] = {0x01, 0x00};
#endif
#endif

// Semaphore used to post events to the application thread
static Semaphore_Handle appSemHandle;
static Semaphore_Struct appSem;

/* App service ID used for messaging with stack service task */
uint8_t  appServiceTaskId;
/* App service task events, set by the stack service task when sending a message */
static uint32_t appServiceTaskEvents;

static endPointDesc_t  zclSampleLightEpDesc = {0};

#if ZG_BUILD_ENDDEVICE_TYPE
static ClockP_Handle EndDeviceRejoinClkHandle;
static ClockP_Struct EndDeviceRejoinClkStruct;
#endif
#ifdef ZCL_LEVEL_CTRL
static ClockP_Handle LevelControlClkHandle;
static ClockP_Struct LevelControlClkStruct;
#endif

static ClockP_Handle DiscoveryClkHandle;
static ClockP_Struct DiscoveryClkStruct;

#if defined(USE_DMM) && defined(BLE_START)
static ClockP_Struct SyncAttrClkStruct;
#endif // defined(USE_DMM) && defined(BLE_START)

// Passed in function pointers to the NV driver
static NVINTF_nvFuncts_t *pfnZdlNV = NULL;

#if defined(USE_DMM) && defined(BLE_START) || !defined(CUI_DISABLE)
static uint16_t zclSampleLight_BdbCommissioningModes;
#endif // defined(USE_DMM) && defined(BLE_START) || !defined(CUI_DISABLE)

afAddrType_t zclSampleLight_DstAddr;

#ifdef ZCL_LEVEL_CTRL
uint8_t zclSampleLight_WithOnOff;       // set to TRUE if state machine should set light on/off
uint8_t zclSampleLight_NewLevel;        // new level when done moving
uint8_t zclSampleLight_LevelChangeCmd; // current level change was triggered by an on/off command
bool  zclSampleLight_NewLevelUp;      // is direction to new level up or down?
int32_t zclSampleLight_CurrentLevel32;  // current level, fixed point (e.g. 192.456)
int32_t zclSampleLight_Rate32;          // rate in units, fixed point (e.g. 16.123)
uint8_t zclSampleLight_LevelLastLevel;  // to save the Current Level before the light was turned OFF
#endif

#if defined (Z_POWER_TEST)
#ifndef Z_POWER_TEST_DATA_TX_INTERVAL
#define Z_POWER_TEST_DATA_TX_INTERVAL 5000
#endif
#if defined (POWER_TEST_POLL_DATA)
static uint16_t powerTestZEDAddr = 0xFFFE;
#endif
#endif // Z_POWER_TEST

#ifndef CUI_DISABLE
CONST char zclSampleLight_appStr[] = APP_TITLE_STR;
CUI_clientHandle_t gCuiHandle;
static LED_Handle gRedLedHandle;
static uint32_t gSampleLightInfoLine;
#endif

#if defined (BDB_TL_TARGET) || defined (BDB_TL_INITIATOR)
// Touchlink BDB Finding and Binding callback function
static void tl_BDBFindingBindingCb(void);
#endif // defined ( BDB_TL_TARGET ) || defined (BDB_TL_INITIATOR)

static uint8_t endPointDiscovered = 0x00;

//Discovery in progress state. This last 3 seconds to get the responses.
static uint8_t discoveryInprogress = 0x00;
#define DISCOVERY_IN_PROGRESS_TIMEOUT   3000


/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void zclSampleLight_initialization(void);
static void zclSampleLight_process_loop(void);
static void zclSampleLight_initParameters(void);
static void zclSampleLight_processZStackMsgs(zstackmsg_genericReq_t *pMsg);
static void SetupZStackCallbacks(void);
static void zclSampleLight_processAfIncomingMsgInd(zstack_afIncomingMsgInd_t *pInMsg);
static void zclSampleLight_initializeClocks(void);
#if ZG_BUILD_ENDDEVICE_TYPE
static void zclSampleLight_processEndDeviceRejoinTimeoutCallback(UArg a0);
#endif
static void zclSampleLight_processDiscoveryTimeoutCallback(UArg a0);

static void zclSampleLight_Init( void );

static void zclSampleLight_BasicResetCB( void );
static void zclSampleLight_IdentifyQueryRspCB(zclIdentifyQueryRsp_t *pRsp);
#ifndef CUI_DISABLE
static void zclSampleLight_processKey(uint8_t key, Button_EventMask buttonEvents);
static void zclSampleLight_RemoveAppNvmData(void);
static void zclSampleLight_InitializeStatusLine(CUI_clientHandle_t gCuiHandle);
static void zclSampleLight_UpdateStatusLine(void);
#endif
static uint8_t zclSampleLight_SceneStoreCB(zclSceneReq_t *pReq);
static void  zclSampleLight_SceneRecallCB(zclSceneReq_t *pReq);
static void  zclSampleLight_OnOffCB( uint8_t cmd );
ZStatus_t zclSampleLight_ReadWriteAttrCB( uint16_t clusterId, uint16_t attrId, uint8_t oper,
                                          uint8_t *pValue, uint16_t *pLen );


static void zclSampleLight_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg);


#ifdef ZCL_LEVEL_CTRL
static void zclSampleLight_LevelControlMoveToLevelCB( zclLCMoveToLevel_t *pCmd );
static void zclSampleLight_LevelControlMoveCB( zclLCMove_t *pCmd );
static void zclSampleLight_LevelControlStepCB( zclLCStep_t *pCmd );
static void zclSampleLight_LevelControlStopCB( zclLCStop_t *pCmd );
static void zclSampleLight_LevelControlMoveToClosestFrequencyCB( zclLCMoveFreq_t *pCmd );
static void zclSampleLight_DefaultMove( uint8_t OnOff );
static uint32_t zclSampleLight_TimeRateHelper( uint8_t newLevel );
static uint16_t zclSampleLight_GetTime ( uint8_t level, uint16_t time );
static void zclSampleLight_MoveBasedOnRate( uint8_t newLevel, uint32_t rate );
static void zclSampleLight_MoveBasedOnTime( uint8_t newLevel, uint16_t time );
static void zclSampleLight_AdjustLightLevel( void );
#endif

// Functions to process ZCL Foundation incoming Command/Response messages
static uint8_t zclSampleLight_ProcessIncomingMsg( zclIncoming_t *msg );
#ifdef ZCL_READ
static uint8_t zclSampleLight_ProcessInReadRspCmd( zclIncoming_t *pInMsg );
#endif
#ifdef ZCL_WRITE
static uint8_t zclSampleLight_ProcessInWriteRspCmd( zclIncoming_t *pInMsg );
#endif
static uint8_t zclSampleLight_ProcessInDefaultRspCmd( zclIncoming_t *pInMsg );
#ifdef ZCL_DISCOVER
static uint8_t zclSampleLight_ProcessInDiscCmdsRspCmd( zclIncoming_t *pInMsg );
static uint8_t zclSampleLight_ProcessInDiscAttrsRspCmd( zclIncoming_t *pInMsg );
static uint8_t zclSampleLight_ProcessInDiscAttrsExtRspCmd( zclIncoming_t *pInMsg );
#endif


static void zclSampleLight_UpdateLedState(void);


#ifdef ZCL_LEVEL_CTRL
static void zclSampleLight_processLevelControlTimeoutCallback(UArg a0);
#endif

#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
static void zclSampleLight_GPSink_Toggle(zclGpNotification_t *zclGpNotification);
static void zclSampleLight_GPSink_On(zclGpNotification_t *zclGpNotification);
static void zclSampleLight_GPSink_Off(zclGpNotification_t *zclGpNotification);
static void zclSampleLight_GPSink_Identify(zclGpNotification_t *zclGpNotification);
#endif

#if defined (BDB_TL_TARGET) || defined (BDB_TL_INITIATOR)
tl_BDBFindingBindingCb_t tl_FindingBindingCb =
{
  tl_BDBFindingBindingCb
};
#endif // defined ( BDB_TL_TARGET ) || defined (BDB_TL_INITIATOR)

#if defined(USE_DMM) && defined(BLE_START)
// Clock callback functions
static void zclSampleLight_processSyncAttrTimeoutCallback(UArg a0);

// Remote display callback functions
static void setLightAttrCb(RemoteDisplayLightAttr_t lightAttr, void *const value, uint8_t len);
static void getLightAttrCb(RemoteDisplayLightAttr_t lightAttr, void *value, uint8_t len);

// Provisioning callback functions
static void provisionConnectCb(void);
static void provisionDisconnectCb(void);
static void setProvisioningCb(RemoteDisplay_ProvisionAttr_t provisioningAttr, void *const value, uint8_t len);
static void getProvisioningCb(RemoteDisplay_ProvisionAttr_t provisioningAttr, void *value, uint8_t len);
#endif // defined(USE_DMM) && defined(BLE_START)

#ifdef DMM_OAD
static void zclSampleLight_dmmPausePolicyCb(uint16_t _pause);
/*********************************************************************
 * DMM Policy Callbacks
 */
static DMMPolicy_AppCbs_t dmmPolicyAppCBs =
{
     zclSampleLight_dmmPausePolicyCb
};
#endif

/*********************************************************************
 * CONSTANTS
 */





#define LEVEL_CHANGED_BY_LEVEL_CMD  0
#define LEVEL_CHANGED_BY_ON_CMD     1
#define LEVEL_CHANGED_BY_OFF_CMD    2



/*********************************************************************
 * REFERENCED EXTERNALS
 */
extern int16_t zdpExternalStateTaskID;

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclSampleLight_CmdCallbacks =
{
  zclSampleLight_BasicResetCB,            // Basic Cluster Reset command
  NULL,                                   // Identfiy cmd
  NULL,                                   // Identify Query command
  zclSampleLight_IdentifyQueryRspCB,      // Identify Query Response command
  NULL,                                   // Identify Trigger Effect command
#ifdef ZCL_ON_OFF
  zclSampleLight_OnOffCB,                 // On/Off cluster commands
  NULL,                                   // On/Off cluster enhanced command Off with Effect
  NULL,                                   // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                   // On/Off cluster enhanced command On with Timed Off
#endif
#ifdef ZCL_LEVEL_CTRL
  zclSampleLight_LevelControlMoveToLevelCB,             // Level Control Move to Level command
  zclSampleLight_LevelControlMoveCB,                    // Level Control Move command
  zclSampleLight_LevelControlStepCB,                    // Level Control Step command
  zclSampleLight_LevelControlStopCB,                    // Level Control Stop command
  zclSampleLight_LevelControlMoveToClosestFrequencyCB,  // Level Control Stop command
#endif
#ifdef ZCL_GROUPS
  NULL,                                   // Group Response commands
#endif
#ifdef ZCL_SCENES
  zclSampleLight_SceneStoreCB,           // Scene Store Request command
  zclSampleLight_SceneRecallCB,          // Scene Recall Request command
  NULL,                                  // Scene Response command
#endif
#ifdef ZCL_ALARMS
  NULL,                                  // Alarm (Response) commands
#endif
#ifdef SE_UK_EXT
  NULL,                                  // Get Event Log command
  NULL,                                  // Publish Event Log command
#endif
  NULL,                                  // RSSI Location command
  NULL                                   // RSSI Location Response command
};


#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
GpSink_AppCallbacks_t zclSampleLight_GpSink_AppCallbacks =
{
#ifdef ZCL_IDENTIFY
    zclSampleLight_GPSink_Identify,     //IdentifyCmd;
#endif
#ifdef ZCL_SCENES
    NULL,                               //RecallSceneCmd;
    NULL,                               //StoreSceneCmd;
#endif
#ifdef ZCL_ON_OFF
    zclSampleLight_GPSink_Off,          //OffCmd;
    zclSampleLight_GPSink_On,           //OnCmd;
    zclSampleLight_GPSink_Toggle,       //ToggleCmd;
#endif
#ifdef ZCL_LEVEL_CTRL
    NULL,                               //LevelControlStopCmd;
    NULL,                               //MoveUpCmd;
    NULL,                               //MoveDownCmd;
    NULL,                               //StepUpCmd;
    NULL,                               //StepDownCmd;
    NULL,                               //MoveUpWithOnOffCmd;
    NULL,                               //MoveDownWithOnOffCmd;
    NULL,                               //StepUpWithOnOffCmd;
    NULL,                               //StepDownWithOnOffCmd;
#endif
    NULL,                               //MoveHueStopCmd;
    NULL,                               //MoveHueUpCmd;
    NULL,                               //MoveHueDownCmd;
    NULL,                               //StepHueUpCmd;
    NULL,                               //StepHueDownCmd;
    NULL,                               //MoveSaturationStopCmd;
    NULL,                               //MoveSaturationUpCmd;
    NULL,                               //MoveSaturationDownCmd;
    NULL,                               //StepSaturationUpCmd;
    NULL,                               //StepSaturationDownCmd;
    NULL,                               //MoveColorCmd;
    NULL,                               //StepColorCmd;
#ifdef ZCL_DOORLOCK
    NULL,                               //LockDoorCmd;
    NULL,                               //UnlockDoorCmd;
#endif
    NULL,                               //AttributeReportingCmd;
    NULL,                               //MfrSpecificReportingCmd;
    NULL,                               //MultiClusterReportingCmd;
    NULL,                               //MfrSpecificMultiReportingCmd;
    NULL,                               //RequestAttributesCmd;
    NULL,                               //ReadAttributeRspCmd;
    NULL,                               //zclTunnelingCmd;
};
#endif

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

zstack_DevState provState = zstack_DevState_HOLD;
uint16_t provPanId = ZDAPP_CONFIG_PAN_ID;
uint32_t provChanMask = DEFAULT_CHANLIST;

DMMPolicy_StackRole DMMPolicy_StackRole_Zigbee =
#if ZG_BUILD_ENDDEVICE_TYPE
    DMMPolicy_StackRole_ZigbeeEndDevice;
#elif ZG_BUILD_RTRONLY_TYPE
    DMMPolicy_StackRole_ZigbeeRouter;
#elif ZG_BUILD_COORDINATOR_TYPE
    DMMPolicy_StackRole_ZigbeeCoordinator;
#endif

#endif // defined(USE_DMM) && defined(BLE_START)

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
  zclSampleLight_initialization();

  // No return from task process
  zclSampleLight_process_loop();
}



/*******************************************************************************
 * @fn          zclSampleLight_initialization
 *
 * @brief       Initialize the application
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleLight_initialization(void)
{
    /* Initialize user clocks */
    zclSampleLight_initializeClocks();

    /* create semaphores for messages / events
     */
    Semaphore_Params semParam;
    Semaphore_Params_init(&semParam);
    semParam.mode = ti_sysbios_knl_Semaphore_Mode_COUNTING;
    Semaphore_construct(&appSem, 0, &semParam);
    appSemHandle = Semaphore_handle(&appSem);

    appServiceTaskId = OsalPort_registerTask(Task_self(), appSemHandle, &appServiceTaskEvents);

    //Initialize stack
    zclSampleLight_Init();

#if defined (Z_POWER_TEST)
    zstack_sysSetTxPowerReq_t txPowerReq;
    zstack_sysSetTxPowerRsp_t txPowerRsp;
    txPowerReq.requestedTxPower = POWER_TEST_TX_PWR;
    Zstackapi_sysSetTxPowerReq(appServiceTaskId, &txPowerReq, &txPowerRsp);
    OsalPort_setEvent(appServiceTaskId, SAMPLEAPP_POWER_TEST_START_EVT);
#endif

#ifdef DMM_OAD
    // register the app callbacks
    DMMPolicy_registerAppCbs(dmmPolicyAppCBs, DMMPolicy_StackRole_Zigbee);
#endif
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
#if defined Z_POWER_TEST
#if defined (POWER_TEST_POLL_DATA)
    zdoCBReq.has_deviceAnnounce = true;
    zdoCBReq.deviceAnnounce = true;
#endif
#endif // Z_POWER_TEST

    (void)Zstackapi_DevZDOCBReq(appServiceTaskId, &zdoCBReq);
}



/*********************************************************************
 * @fn          zclSampleLight_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleLight_Init( void )
{
#ifdef BDB_REPORTING
      zstack_bdbRepAddAttrCfgRecordDefaultToListReq_t Req = {0};
#endif

  // Set destination address to indirect
  zclSampleLight_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
  zclSampleLight_DstAddr.endPoint = 0;
  zclSampleLight_DstAddr.addr.shortAddr = 0;

  //Register Endpoint
  zclSampleLightEpDesc.endPoint = SAMPLELIGHT_ENDPOINT;
  zclSampleLightEpDesc.simpleDesc = &zclSampleLight_SimpleDesc;
  zclport_registerEndpoint(appServiceTaskId, &zclSampleLightEpDesc);

#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
  zclGp_RegisterCBForGPDCommand(&zclSampleLight_GpSink_AppCallbacks);
#endif


  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( SAMPLELIGHT_ENDPOINT, &zclSampleLight_CmdCallbacks );

  // Register the application's attribute list and reset to default values
  zclSampleLight_ResetAttributesToDefaultValues();
  zcl_registerAttrList( SAMPLELIGHT_ENDPOINT, zclSampleLight_NumAttributes, zclSampleLight_Attrs );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zclport_registerZclHandleExternal(SAMPLELIGHT_ENDPOINT, zclSampleLight_ProcessIncomingMsg);

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
  gp_endpointInit(appServiceTaskId);
#endif

  //Write the bdb initialization parameters
  zclSampleLight_initParameters();

  //Setup ZDO callbacks
  SetupZStackCallbacks();

#if defined ( BDB_TL_INITIATOR )
  zclSampleLight_BdbCommissioningModes |= BDB_COMMISSIONING_MODE_INITIATOR_TL;
#endif

#ifdef ZCL_LEVEL_CTRL
  zclSampleLight_LevelLastLevel = zclSampleLight_getCurrentLevelAttribute();
#endif

#ifdef BDB_REPORTING
  //Adds the default configuration values for the temperature attribute of the ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT cluster, for endpoint SAMPLETEMPERATURESENSOR_ENDPOINT
  //Default maxReportingInterval value is 10 seconds
  //Default minReportingInterval value is 3 seconds
  //Default reportChange value is 300 (3 degrees)
  Req.attrID = ATTRID_ON_OFF_ON_OFF;
  Req.cluster = ZCL_CLUSTER_ID_GENERAL_ON_OFF;
  Req.endpoint = SAMPLELIGHT_ENDPOINT;
  Req.maxReportInt = 10;
  Req.minReportInt = 0;
  OsalPort_memcpy(Req.reportableChange,reportableChange,BDBREPORTING_MAX_ANALOG_ATTR_SIZE);

  Zstackapi_bdbRepAddAttrCfgRecordDefaultToListReq(appServiceTaskId,&Req);

#ifdef ZCL_LEVEL_CTRL
  Req.attrID = ATTRID_LEVEL_CURRENT_LEVEL;
  Req.cluster = ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL;
  Req.endpoint = SAMPLELIGHT_ENDPOINT;
  Req.maxReportInt = 10;
  Req.minReportInt = 0;
  OsalPort_memcpy(Req.reportableChange,reportableChange,BDBREPORTING_MAX_ANALOG_ATTR_SIZE);

  Zstackapi_bdbRepAddAttrCfgRecordDefaultToListReq(appServiceTaskId,&Req);
#endif

#endif


#ifdef ZCL_DISCOVER
  // Register the application's command list
  zcl_registerCmdList( SAMPLELIGHT_ENDPOINT, zclCmdsArraySize, zclSampleLight_Cmds );
#endif

  zcl_registerReadWriteCB(SAMPLELIGHT_ENDPOINT,zclSampleLight_ReadWriteAttrCB,NULL);

#if defined (BDB_TL_TARGET) || defined (BDB_TL_INITIATOR)
  touchLinkApp_registerFindingBindingCb(tl_FindingBindingCb);
#endif // defined ( BDB_TL_TARGET ) || (BDB_TL_INITIATOR)

#if defined(USE_DMM) && defined(BLE_START)
  RemoteDisplay_registerClientProvCbs(zclSwitch_ProvissioningCbs);
  RemoteDisplay_registerLightCbs(zclSwitch_LightCbs);
#endif // defined(USE_DMM) && defined(BLE_START)

#if !defined(CUI_DISABLE) || defined(USE_DMM) && defined(BLE_START)
  // set up default application BDB commissioning modes based on build type
  if(ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE)
  {
    zclSampleLight_BdbCommissioningModes = BDB_COMMISSIONING_MODE_NWK_FORMATION | BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_FINDING_BINDING;
  }
  else if (ZG_BUILD_JOINING_TYPE && ZG_DEVICE_JOINING_TYPE)
  {
    zclSampleLight_BdbCommissioningModes = BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_FINDING_BINDING;
  }
#endif // !defined(CUI_DISABLE) || defined(USE_DMM) && defined(BLE_START)

#ifndef CUI_DISABLE
  gCuiHandle = UI_Init( appServiceTaskId,                     // Application Task ID
           &appServiceTaskEvents,                // The events processed by the sample application
           appSemHandle,                         // Semaphore to post the events in the application thread
           &zclSampleLight_IdentifyTime,
           &zclSampleLight_BdbCommissioningModes,   // a pointer to the applications bdbCommissioningModes
           zclSampleLight_appStr,
           zclSampleLight_processKey,
           zclSampleLight_RemoveAppNvmData         // A pointer to the app-specific NV Item reset function
           );

  //Request the Red LED for App
  LED_Params ledParams;
  LED_Params_init(&ledParams);
  gRedLedHandle = LED_open(CONFIG_LED_RED, &ledParams);


  //Initialize the sampleLight UI status line
  zclSampleLight_InitializeStatusLine(gCuiHandle);
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

#ifdef PER_TEST
#ifndef CUI_DISABLE
  PERTest_init( appSemHandle, appServiceTaskId, gCuiHandle );
#else
  PERTest_init( appSemHandle, appServiceTaskId, NULL );
#endif
#endif

  // Call BDB initialization. Should be called once from application at startup to restore
  // previous network configuration, if applicable.
  zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;
  zstack_bdbStartCommissioningReq.commissioning_mode = 0;
  Zstackapi_bdbStartCommissioningReq(appServiceTaskId,&zstack_bdbStartCommissioningReq);
}

#ifndef CUI_DISABLE
/*********************************************************************
 * @fn          zclSampleLight_RemoveAppNvmData
 *
 * @brief       Callback when Application performs reset to Factory New Reset.
 *              Application must restore the application to default values
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleLight_RemoveAppNvmData(void)
{
#ifdef ZCL_GROUPS
    uint8_t numGroups;
    uint16_t groupList[APS_MAX_GROUPS];
    uint8_t i;

    if ( 0 != ( numGroups = aps_FindAllGroupsForEndpoint( SAMPLELIGHT_ENDPOINT, groupList ) ) )
    {
      for ( i = 0; i < numGroups; i++ )
      {
#if defined ( ZCL_SCENES )
        zclGeneral_RemoveAllScenes( SAMPLELIGHT_ENDPOINT, groupList[i] );
#endif
      }
      aps_RemoveAllGroup( SAMPLELIGHT_ENDPOINT );
    }
#endif
}
#endif

static void zclSampleLight_initParameters(void)
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
 * @fn      zclSampleLight_initializeClocks
 *
 * @brief   Initialize Clocks
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleLight_initializeClocks(void)
{
#if ZG_BUILD_ENDDEVICE_TYPE
    // Initialize the timers needed for this application
    EndDeviceRejoinClkHandle = UtilTimer_construct(
    &EndDeviceRejoinClkStruct,
    zclSampleLight_processEndDeviceRejoinTimeoutCallback,
    SAMPLEAPP_END_DEVICE_REJOIN_DELAY,
    0, false, 0);
#endif
#ifdef ZCL_LEVEL_CTRL
    LevelControlClkHandle = UtilTimer_construct(
    &LevelControlClkStruct,
    zclSampleLight_processLevelControlTimeoutCallback,
    100,
    0, false, 0);
#endif
#if defined(USE_DMM) && defined(BLE_START)
    // Clock for synchronizing application configuration parameters for BLE
    UtilTimer_construct(
    &SyncAttrClkStruct,
    zclSampleLight_processSyncAttrTimeoutCallback,
    SAMPLEAPP_CONFIG_SYNC_TIMEOUT,
    SAMPLEAPP_CONFIG_SYNC_TIMEOUT, true, 0);
#endif // defined(USE_DMM) && defined(BLE_START)

    // Initialize the timers needed for this application
    DiscoveryClkHandle = UtilTimer_construct(
    &DiscoveryClkStruct,
    zclSampleLight_processDiscoveryTimeoutCallback,
    DISCOVERY_IN_PROGRESS_TIMEOUT,
    0, false, 0);

}

#if ZG_BUILD_ENDDEVICE_TYPE
/*******************************************************************************
 * @fn      zclSampleLight_processEndDeviceRejoinTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleLight_processEndDeviceRejoinTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    appServiceTaskEvents |= SAMPLEAPP_END_DEVICE_REJOIN_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}
#endif


/*******************************************************************************
 * @fn      zclSampleLight_processDiscoveryTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleLight_processDiscoveryTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    appServiceTaskEvents |= SAMPLEAPP_DISCOVERY_TIMEOUT_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}


/*******************************************************************************
 * @fn      zclSampleLight_process_loop
 *
 * @brief   Application task processing start.
 *
 * @param   none
 *
 * @return  void
 */
static void zclSampleLight_process_loop(void)
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
            if((pMsg = (zstackmsg_genericReq_t*) OsalPort_msgReceive( appServiceTaskId )) != NULL)
            {

                zclSampleLight_processZStackMsgs(pMsg);
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

#ifndef CUI_DISABLE
            //Process the events that the UI may have
            zclsampleApp_ui_event_loop();
#endif

            if ( appServiceTaskEvents & SAMPLEAPP_DISCOVERY_TIMEOUT_EVT )
            {
              discoveryInprogress = FALSE;

              appServiceTaskEvents &= ~SAMPLEAPP_DISCOVERY_TIMEOUT_EVT;
            }

#if defined (BDB_TL_TARGET) || defined (BDB_TL_INITIATOR)
            if(appServiceTaskEvents & TL_BDB_FB_EVT)
            {
                zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;
                zstack_bdbStartCommissioningReq.commissioning_mode = BDB_COMMISSIONING_MODE_FINDING_BINDING;
                Zstackapi_bdbStartCommissioningReq(appServiceTaskId, &zstack_bdbStartCommissioningReq);
                appServiceTaskEvents &= ~TL_BDB_FB_EVT;
            }
#endif // defined ( BDB_TL_TARGET ) || (BDB_TL_INITIATOR)

#if defined(USE_DMM) && defined(BLE_START)
            if(appServiceTaskEvents & SAMPLEAPP_PROV_CONNECT_EVT)
            {
                zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;
                zstack_bdbStartCommissioningReq.commissioning_mode = zclSampleLight_BdbCommissioningModes;
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
                else if( (((provState > zstack_DevState_INIT) && (provState < zstack_DevState_DEV_ROUTER)) ||
                          (provState > zstack_DevState_DEV_ROUTER) ) &&
                         (stackState != DMMPOLICY_ZB_PROVISIONING) )
                {
                    DMMPolicy_updateApplicationState(DMMPolicy_StackRole_Zigbee, DMMPOLICY_ZB_PROVISIONING);
                }
                // If connected
                else if( (provState == zstack_DevState_DEV_ROUTER) &&
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
#endif // defined(USE_DMM) && defined(BLE_START)

#ifdef ZCL_LEVEL_CTRL
            if(appServiceTaskEvents & SAMPLELIGHT_LEVEL_CTRL_EVT)
            {
                zclSampleLight_AdjustLightLevel();
                appServiceTaskEvents &= ~SAMPLELIGHT_LEVEL_CTRL_EVT;
            }
#endif // ZCL_LEVEL_CTRL

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
              // assuming we are ZC in this test, we must create + open the network
              zstack_bdbStartCommissioningReq.commissioning_mode = BDB_COMMISSIONING_MODE_NWK_FORMATION | BDB_COMMISSIONING_MODE_NWK_STEERING;
              Zstackapi_bdbStartCommissioningReq(appServiceTaskId,&zstack_bdbStartCommissioningReq);
#elif defined (POWER_TEST_DATA_ACK)
              // assuming we are ZC in this test, we must create + open the network + start finding&binding
              zstack_bdbStartCommissioningReq.commissioning_mode = BDB_COMMISSIONING_MODE_NWK_FORMATION | BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_FINDING_BINDING;
              Zstackapi_bdbStartCommissioningReq(appServiceTaskId,&zstack_bdbStartCommissioningReq);
#endif
              appServiceTaskEvents &= ~SAMPLEAPP_POWER_TEST_START_EVT;
            }
#if defined (POWER_TEST_POLL_DATA)
            if ( appServiceTaskEvents & SAMPLEAPP_POWER_TEST_ZCL_DATA_EVT )
            {
              static uint8_t theMessageData[] = {"\0\0\30POWER TEST!!!!!"};
              static uint8_t transID = 0;
              extern uint8_t zcl_TransID;

              zstack_getZCLFrameCounterRsp_t pRsp;
              Zstackapi_getZCLFrameCounterReq(appServiceTaskId, &pRsp);

              theMessageData[1] = transID++;

              zstack_afDataReq_t pReq;
              pReq.dstAddr.addrMode = zstack_AFAddrMode_SHORT;
              pReq.dstAddr.addr.shortAddr = powerTestZEDAddr;
              pReq.dstAddr.endpoint = 0xAF;
              pReq.pRelayList = NULL;
              pReq.n_relayList = 0;
              pReq.srcEndpoint = SAMPLELIGHT_ENDPOINT;
              pReq.clusterID = 0xBEEF;
              pReq.transID = &zcl_TransID;
              pReq.options.ackRequest = FALSE;
              pReq.options.apsSecurity = FALSE;
              pReq.options.limitConcentrator = FALSE;
              pReq.options.skipRouting = FALSE;
              pReq.options.suppressRouteDisc = FALSE;
              pReq.options.wildcardProfileID = FALSE;
              pReq.radius = AF_DEFAULT_RADIUS;
              pReq.n_payload = sizeof(theMessageData);
              pReq.pPayload = theMessageData;

              Zstackapi_AfDataReq(appServiceTaskId, &pReq);

              appServiceTaskEvents &= ~SAMPLEAPP_POWER_TEST_ZCL_DATA_EVT;
            }
#endif
#endif // Z_POWER_TEST
        }
    }
}

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
 *  @param  value  pointer to data from remote display application
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
        case ProvisionAttr_ExtPanId:
            //Not supported for ZigBee
        case ProvisionAttr_Freq:
            //Not supported for ZigBee
        case ProvisionAttr_FFDAddr:
            //Not supported for ZigBee
        case ProvisionAttr_NtwkKey:
            //Not supported for ZigBee
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
        case ProvisionAttr_SensorChannelMask:
        {
            ((uint8_t *)value)[0] = Util_breakUint32(provChanMask, 3);
            ((uint8_t *)value)[1] = Util_breakUint32(provChanMask, 2);
            ((uint8_t *)value)[2] = Util_breakUint32(provChanMask, 1);
            ((uint8_t *)value)[3] = Util_breakUint32(provChanMask, 0);
            break;
        }
        case ProvisionAttr_Freq:
            //Not supported for ZigBee
        case ProvisionAttr_ExtPanId:
            //Not supported for ZigBee
        case ProvisionAttr_FFDAddr:
            //Not supported for ZigBee
        case ProvisionAttr_NtwkKey:
            //Not supported for ZigBee
        default:
            // Attribute not found
            break;
    }
}

/** @brief  Set remote display callback functions
 *
 *  @param  lightAttr  Remote display attribute value to set
 *  @param  value  pointer to data from remote display application
 *  @param  len  length of data from remote display application
 */
static void setLightAttrCb(RemoteDisplayLightAttr_t lightAttr,
    void *const value, uint8_t len)
{
    switch(lightAttr)
    {
        case LightAttr_Light_OnOff:
        {
#ifndef Z_POWER_TEST
            if(*((uint8_t*)value) == 0)
            {
                zclSampleLight_OnOffCB(COMMAND_ON_OFF_OFF);
            }
            else
            {
                zclSampleLight_OnOffCB(COMMAND_ON_OFF_ON);
            }
#endif
            break;
        }
        case LightAttr_Target_Addr_Type:
        {
            /* Not supported for sample light */
            break;
        }
        case LightAttr_Target_Addr:
        {
            /* Not supported for sample light */
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
#ifdef ZCL_LEVEL_CTRL
            ((uint8_t *)value)[0] = (zclSampleLight_LevelChangeCmd == LEVEL_CHANGED_BY_ON_CMD) ?  LIGHT_ON : LIGHT_OFF;
#else
            ((uint8_t *)value)[0] = zclSampleLight_getOnOffAttribute() ? LIGHT_ON : LIGHT_OFF;
#endif
            break;
        }
        case LightAttr_Target_Addr_Type:
        {
            /* Not supported for sample light */
            break;
        }
        case LightAttr_Target_Addr:
        {
            /* Not supported for sample light */
            break;
        }
        default:
            // Attribute not found
            break;
        }
}

/*******************************************************************************
 * @fn      zclSampleLight_processSyncAttrTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleLight_processSyncAttrTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    appServiceTaskEvents |= SAMPLEAPP_SYNC_ATTR_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}

#endif //defined(USE_DMM) && defined(BLE_START)

/*********************************************************************
 * @fn      zclSampleLight_IdentifyQueryRspCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to process a Indentify Query Rsp.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleLight_IdentifyQueryRspCB(zclIdentifyQueryRsp_t *pRsp)
{
    zstack_zdoMatchDescReq_t Req;
    uint16_t  OnOffCluster = ZCL_CLUSTER_ID_GENERAL_ON_OFF;

    if(discoveryInprogress)
    {
        Req.dstAddr = pRsp->srcAddr->addr.shortAddr;
        Req.nwkAddrOfInterest = pRsp->srcAddr->addr.shortAddr;
        Req.profileID = ZCL_HA_PROFILE_ID;

        endPointDiscovered = pRsp->srcAddr->endPoint;

        Req.n_inputClusters = 0;
        Req.n_outputClusters = 1;

        Req.pOutputClusters = &OnOffCluster;

        Zstackapi_ZdoMatchDescReq(appServiceTaskId, &Req);
    }
}


/*******************************************************************************
 * @fn      zclSampleLight_processZStackMsgs
 *
 * @brief   Process event from Stack
 *
 * @param   pMsg - pointer to incoming ZStack message to process
 *
 * @return  void
 */
static void zclSampleLight_processZStackMsgs(zstackmsg_genericReq_t *pMsg)
{
    switch(pMsg->hdr.event)
    {
        case zstackmsg_CmdIDs_BDB_NOTIFICATION:
            {
                zstackmsg_bdbNotificationInd_t *pInd;
                pInd = (zstackmsg_bdbNotificationInd_t*)pMsg;
                zclSampleLight_ProcessCommissioningStatus(&(pInd->Req));
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
                zclSampleLight_processAfIncomingMsgInd( &(pInd->req) );
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
#if !defined(CUI_DISABLE) || defined(USE_DMM) && defined(BLE_START)
            // The ZStack Thread is indicating a State change
            zstackmsg_devStateChangeInd_t *pInd =
                (zstackmsg_devStateChangeInd_t *)pMsg;
#endif // !defined(CUI_DISABLE) || defined(USE_DMM) && defined(BLE_START)

#ifndef CUI_DISABLE
            UI_DeviceStateUpdated(&(pInd->req));
#endif

#if defined(USE_DMM) && defined(BLE_START)
            provState = pInd->req.state;

            appServiceTaskEvents |= SAMPLEAPP_POLICY_UPDATE_EVT;

            // Wake up the application thread when it waits for clock event
            Semaphore_post(appSemHandle);
#endif // defined(USE_DMM) && defined(BLE_START)
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
          UI_UpdateBdbStatusLine(NULL);
        }
        break;
#endif

        case zstackmsg_CmdIDs_ZDO_DEVICE_ANNOUNCE:
#if defined (Z_POWER_TEST)
#if defined (POWER_TEST_POLL_DATA)
        {
          zstackmsg_zdoDeviceAnnounceInd_t *pInd;
          pInd = (zstackmsg_zdoDeviceAnnounceInd_t*)pMsg;

          // save the short address of the ZED to send ZCL test data
          powerTestZEDAddr = pInd->req.srcAddr;

          // start periodic timer for sending ZCL data to zed
          OsalPortTimers_startReloadTimer(appServiceTaskId, SAMPLEAPP_POWER_TEST_ZCL_DATA_EVT, Z_POWER_TEST_DATA_TX_INTERVAL);
        }
#endif
#else
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
                Req.bindInfo.clusterID = ZCL_CLUSTER_ID_GENERAL_ON_OFF;
                Req.bindInfo.srcEndpoint = SAMPLELIGHT_ENDPOINT;

                //create the bind to that device
                Zstackapi_ZdoBindReq(appServiceTaskId, &Req);
            }

        }
        break;


        case zstackmsg_CmdIDs_DEV_PERMIT_JOIN_IND:
        case zstackmsg_CmdIDs_BDB_TC_LINK_KEY_EXCHANGE_NOTIFICATION_IND:
        case zstackmsg_CmdIDs_AF_DATA_CONFIRM_IND:
        case zstackmsg_CmdIDs_ZDO_NWK_ADDR_RSP:
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
 * @fn          zclSampleLight_processAfIncomingMsgInd
 *
 * @brief       Process AF Incoming Message Indication message
 *
 * @param       pInMsg - pointer to incoming message
 *
 * @return      none
 *
 */
static void zclSampleLight_processAfIncomingMsgInd(zstack_afIncomingMsgInd_t *pInMsg)
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
 * @fn      zclSampleLight_ProcessCommissioningStatus
 *
 * @brief   Callback in which the status of the commissioning process are reported
 *
 * @param   bdbCommissioningModeMsg - Context message of the status of a commissioning process
 *
 * @return  none
 */
static void zclSampleLight_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg)
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
 * @fn      zclSampleLight_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to default values.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleLight_BasicResetCB( void )
{
  //Reset every attribute in all supported cluster to their default value.

  zclSampleLight_ResetAttributesToDefaultValues();

  zclSampleLight_UpdateLedState();

#ifndef CUI_DISABLE
  zclSampleLight_UpdateStatusLine();
#endif
}

/*********************************************************************
 * @fn      zclSampleLight_OnOffCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an On/Off Command for this application.
 *
 * @param   cmd - COMMAND_ON_OFF_ON, COMMAND_ON_OFF_OFF or COMMAND_ON_OFF_TOGGLE
 *
 * @return  none
 */
static void zclSampleLight_OnOffCB( uint8_t cmd )
{
  afIncomingMSGPacket_t *pPtr = zcl_getRawAFMsg();

  uint8_t OnOff = 0xFE; // initialize to invalid

  zclSampleLight_DstAddr.addr.shortAddr = pPtr->srcAddr.addr.shortAddr;

  // Turn on the light
  if ( cmd == COMMAND_ON_OFF_ON )
  {
    OnOff = LIGHT_ON;
  }
  // Turn off the light
  else if ( cmd == COMMAND_ON_OFF_OFF )
  {
    OnOff = LIGHT_OFF;
  }
  // Toggle the light
  else if ( cmd == COMMAND_ON_OFF_TOGGLE )
  {
#ifdef ZCL_LEVEL_CTRL
    if (zclSampleLight_LevelRemainingTime > 0)
    {
      if (zclSampleLight_NewLevelUp)
      {
        OnOff = LIGHT_OFF;
      }
      else
      {
        OnOff = LIGHT_ON;
      }
    }
    else
#endif
    {
      if (zclSampleLight_getOnOffAttribute() == LIGHT_ON)
      {
        OnOff = LIGHT_OFF;
      }
      else
      {
        OnOff = LIGHT_ON;
      }
    }
  }

  if( ((zclSampleLight_getOnOffAttribute() == LIGHT_ON) && (OnOff == LIGHT_ON)) ||
      ((zclSampleLight_getOnOffAttribute() == LIGHT_OFF) && (OnOff == LIGHT_OFF)) )
  {
    // if light is on and received an on command, ignore it.
    // if light is off and received an off command, ignore it.
    return;
  }

#ifdef ZCL_LEVEL_CTRL
  zclSampleLight_LevelChangeCmd = (OnOff == LIGHT_ON ? LEVEL_CHANGED_BY_ON_CMD : LEVEL_CHANGED_BY_OFF_CMD);
  zclSampleLight_DefaultMove(OnOff);
#else
   zclSampleLight_updateOnOffAttribute(OnOff);
#endif

  zclSampleLight_UpdateLedState();

#if defined(USE_DMM) && defined(BLE_START)
  // update BLE application
  RemoteDisplay_updateLightProfData();
#endif // defined(USE_DMM) && defined(BLE_START)

#ifndef CUI_DISABLE
  //Update status line
  zclSampleLight_UpdateStatusLine();
#endif
}

#ifdef ZCL_LEVEL_CTRL

/*******************************************************************************
 * @fn      zclSampleLight_processLevelControlTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleLight_processLevelControlTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    appServiceTaskEvents |= SAMPLELIGHT_LEVEL_CTRL_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}


/*********************************************************************
 * @fn      zclSampleLight_TimeRateHelper
 *
 * @brief   Calculate time based on rate, and startup level state machine
 *
 * @param   newLevel - new level for current level
 *
 * @return  diff (directly), zclSampleLight_CurrentLevel32 and zclSampleLight_NewLevel, zclSampleLight_NewLevelUp
 */
static uint32_t zclSampleLight_TimeRateHelper( uint8_t newLevel )
{
  uint32_t diff;
  uint32_t newLevel32;

  // remember current and new level
  zclSampleLight_NewLevel = newLevel;
  zclSampleLight_CurrentLevel32 = (uint32_t)1000 * zclSampleLight_getCurrentLevelAttribute();

  // calculate diff
  newLevel32 = (uint32_t)1000 * newLevel;
  if ( zclSampleLight_getCurrentLevelAttribute() > newLevel )
  {
    diff = zclSampleLight_CurrentLevel32 - newLevel32;
    zclSampleLight_NewLevelUp = FALSE;  // moving down
  }
  else
  {
    diff = newLevel32 - zclSampleLight_CurrentLevel32;
    zclSampleLight_NewLevelUp = TRUE;   // moving up
  }

  return ( diff );
}

/*********************************************************************
 * @fn      zclSampleLight_MoveBasedOnRate
 *
 * @brief   Calculate time based on rate, and startup level state machine
 *
 * @param   newLevel - new level for current level
 * @param   rate16   - fixed point rate (e.g. 16.123)
 *
 * @return  none
 */
static void zclSampleLight_MoveBasedOnRate( uint8_t newLevel, uint32_t rate )
{
  uint32_t diff;

  // determine how much time (in 10ths of seconds) based on the difference and rate
  zclSampleLight_Rate32 = rate;
  diff = zclSampleLight_TimeRateHelper( newLevel );
  zclSampleLight_LevelRemainingTime = diff / rate;
  if ( !zclSampleLight_LevelRemainingTime )
  {
    zclSampleLight_LevelRemainingTime = 1;
  }

  UtilTimer_setTimeout( LevelControlClkHandle, 100 );
  UtilTimer_start(&LevelControlClkStruct);

}

/*********************************************************************
 * @fn      zclSampleLight_MoveBasedOnTime
 *
 * @brief   Calculate rate based on time, and startup level state machine
 *
 * @param   newLevel  - new level for current level
 * @param   time      - in 10ths of seconds
 *
 * @return  none
 */
static void zclSampleLight_MoveBasedOnTime( uint8_t newLevel, uint16_t time )
{
  uint32_t diff;

  // determine rate (in units) based on difference and time
  diff = zclSampleLight_TimeRateHelper( newLevel );
  zclSampleLight_LevelRemainingTime = zclSampleLight_GetTime( newLevel, time );
  zclSampleLight_Rate32 = diff / time;

  UtilTimer_setTimeout( LevelControlClkHandle, 100 );
  UtilTimer_start(&LevelControlClkStruct);
}

/*********************************************************************
 * @fn      zclSampleLight_GetTime
 *
 * @brief   Determine amount of time that MoveXXX will take to complete.
 *
 * @param   level = new level to move to
 *          time  = 0xffff=default, or 0x0000-n amount of time in tenths of seconds.
 *
 * @return  none
 */
static uint16_t zclSampleLight_GetTime( uint8_t newLevel, uint16_t time )
{
  // there is a hierarchy of the amount of time to use for transitioning
  // check each one in turn. If none of defaults are set, then use fastest
  // time possible.
  if ( time == 0xFFFF )
  {
    // use On or Off Transition Time if set (not 0xffff)
    if ( zclSampleLight_getCurrentLevelAttribute() > newLevel )
    {
      time = zclSampleLight_LevelOffTransitionTime;
    }
    else
    {
      time = zclSampleLight_LevelOnTransitionTime;
    }

    // else use OnOffTransitionTime if set (not 0xffff)
    if ( time == 0xFFFF )
    {
      time = zclSampleLight_LevelOnOffTransitionTime;
    }

    // else as fast as possible
    if ( time == 0xFFFF )
    {
      time = 1;
    }
  }

  if ( time == 0 )
  {
    time = 1; // as fast as possible
  }

  return ( time );
}

/*********************************************************************
 * @fn      zclSampleLight_DefaultMove
 *
 * @brief   We were turned on/off. Use default time to move to on or off.
 *
 * @param   zclSampleLight_OnOff - must be set prior to calling this function.
 *
 * @return  none
 */
static void zclSampleLight_DefaultMove( uint8_t OnOff )
{
  uint8_t  newLevel;
  uint32_t rate;      // fixed point decimal (3 places, eg. 16.345)
  uint16_t time;

  // if moving to on position, move to on level
  if ( OnOff )
  {
    if (zclSampleLight_getOnOffAttribute() == LIGHT_OFF)
    {
        zclSampleLight_updateCurrentLevelAttribute(ATTR_LEVEL_MIN_LEVEL);
    }

    if ( zclSampleLight_LevelOnLevel == ATTR_LEVEL_ON_LEVEL_NO_EFFECT )
    {
      // The last Level (before going OFF) should be used)
      newLevel = zclSampleLight_LevelLastLevel;
    }
    else
    {
      newLevel = zclSampleLight_LevelOnLevel;
    }

    time = zclSampleLight_LevelOnTransitionTime;

  }
  else
  {
    newLevel = ATTR_LEVEL_MIN_LEVEL;

    time = zclSampleLight_LevelOffTransitionTime;
  }

  // else use OnOffTransitionTime if set (not 0xffff)
  if ( time == 0xFFFF )
  {
    time = zclSampleLight_LevelOnOffTransitionTime;
  }

  // else as fast as possible
  if ( time == 0xFFFF )
  {
    time = 1;
  }

  // calculate rate based on time (int 10ths) for full transition (1-254)
  rate = 255000 / time;    // units per tick, fixed point, 3 decimal places (e.g. 8500 = 8.5 units per tick)

  // start up state machine.
  zclSampleLight_WithOnOff = TRUE;
  zclSampleLight_MoveBasedOnRate( newLevel, rate );
}

/*********************************************************************
 * @fn      zclSampleLight_AdjustLightLevel
 *
 * @brief   Called each 10th of a second while state machine running
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleLight_AdjustLightLevel( void )
{
  // one tick (10th of a second) less
  if ( zclSampleLight_LevelRemainingTime )
  {
    --zclSampleLight_LevelRemainingTime;
  }

  // no time left, done
  if ( zclSampleLight_LevelRemainingTime == 0)
  {
      zclSampleLight_updateCurrentLevelAttribute(zclSampleLight_NewLevel);
  }

  // still time left, keep increment/decrementing
  else
  {
    if ( zclSampleLight_NewLevelUp )
    {
      zclSampleLight_CurrentLevel32 += zclSampleLight_Rate32;
    }
    else
    {
      zclSampleLight_CurrentLevel32 -= zclSampleLight_Rate32;
    }

    zclSampleLight_updateCurrentLevelAttribute( zclSampleLight_CurrentLevel32 / 1000 );

  }

  if (( zclSampleLight_LevelChangeCmd == LEVEL_CHANGED_BY_LEVEL_CMD ) && ( zclSampleLight_LevelOnLevel == ATTR_LEVEL_ON_LEVEL_NO_EFFECT ))
  {
    zclSampleLight_LevelLastLevel = zclSampleLight_getCurrentLevelAttribute();
  }

  // also affect on/off
  if ( zclSampleLight_WithOnOff )
  {
    uint8_t OnOffTempState;
    if ( zclSampleLight_getCurrentLevelAttribute() > ATTR_LEVEL_MIN_LEVEL )
    {
      OnOffTempState = LIGHT_ON;
    }
    else
    {
      if (zclSampleLight_LevelChangeCmd != LEVEL_CHANGED_BY_ON_CMD)
      {
        OnOffTempState = LIGHT_OFF;
      }
      else
      {
        OnOffTempState = LIGHT_ON;
      }

      if (( zclSampleLight_LevelChangeCmd != LEVEL_CHANGED_BY_LEVEL_CMD ) && ( zclSampleLight_LevelOnLevel == ATTR_LEVEL_ON_LEVEL_NO_EFFECT ))
      {
          zclSampleLight_updateCurrentLevelAttribute(zclSampleLight_LevelLastLevel);
      }
    }
    zclSampleLight_updateOnOffAttribute(OnOffTempState);
  }


  zclSampleLight_UpdateLedState();
#ifndef CUI_DISABLE
  zclSampleLight_UpdateStatusLine();
#endif

  // keep ticking away
  if ( zclSampleLight_LevelRemainingTime )
  {
      UtilTimer_setTimeout( LevelControlClkHandle, 100 );
      UtilTimer_start(&LevelControlClkStruct);
  }
}

/*********************************************************************
 * @fn      zclSampleLight_LevelControlMoveToLevelCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received a LevelControlMoveToLevel Command for this application.
 *
 * @param   pCmd - ZigBee command parameters
 *
 * @return  none
 */
static void zclSampleLight_LevelControlMoveToLevelCB( zclLCMoveToLevel_t *pCmd )
{
  zclSampleLight_LevelChangeCmd = LEVEL_CHANGED_BY_LEVEL_CMD;

  zclSampleLight_WithOnOff = pCmd->withOnOff;
  zclSampleLight_MoveBasedOnTime( pCmd->level, pCmd->transitionTime );
}

/*********************************************************************
 * @fn      zclSampleLight_LevelControlMoveCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received a LevelControlMove Command for this application.
 *
 * @param   pCmd - ZigBee command parameters
 *
 * @return  none
 */
static void zclSampleLight_LevelControlMoveCB( zclLCMove_t *pCmd )
{
  uint8_t newLevel;
  uint32_t rate;

  // convert rate from units per second to units per tick (10ths of seconds)
  // and move at that right up or down
  zclSampleLight_WithOnOff = pCmd->withOnOff;

  if ( pCmd->moveMode == LEVEL_MOVE_UP )
  {
    newLevel = ATTR_LEVEL_MAX_LEVEL;  // fully on
  }
  else
  {
    newLevel = ATTR_LEVEL_MIN_LEVEL; // fully off
  }

  zclSampleLight_LevelChangeCmd = LEVEL_CHANGED_BY_LEVEL_CMD;

  rate = (uint32_t)100 * pCmd->rate;
  zclSampleLight_MoveBasedOnRate( newLevel, rate );
}

/*********************************************************************
 * @fn      zclSampleLight_LevelControlStepCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an On/Off Command for this application.
 *
 * @param   pCmd - ZigBee command parameters
 *
 * @return  none
 */
static void zclSampleLight_LevelControlStepCB( zclLCStep_t *pCmd )
{
  uint8_t newLevel;

  // determine new level, but don't exceed boundaries
  if ( pCmd->stepMode == LEVEL_MOVE_UP )
  {
    if ( (uint16_t)zclSampleLight_getCurrentLevelAttribute() + pCmd->amount > ATTR_LEVEL_MAX_LEVEL )
    {
      newLevel = ATTR_LEVEL_MAX_LEVEL;
    }
    else
    {
      newLevel = zclSampleLight_getCurrentLevelAttribute() + pCmd->amount;
    }
  }
  else
  {
    if ( pCmd->amount >= zclSampleLight_getCurrentLevelAttribute() )
    {
      newLevel = ATTR_LEVEL_MIN_LEVEL;
    }
    else
    {
      newLevel = zclSampleLight_getCurrentLevelAttribute() - pCmd->amount;
    }
  }

  zclSampleLight_LevelChangeCmd = LEVEL_CHANGED_BY_LEVEL_CMD;

  // move to the new level
  zclSampleLight_WithOnOff = pCmd->withOnOff;
  zclSampleLight_MoveBasedOnTime( newLevel, pCmd->transitionTime );
}

/*********************************************************************
 * @fn      zclSampleLight_LevelControlStopCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Level Control Stop Command for this application.
 *
 * @param   pCmd - ZigBee command parameters
 *
 * @return  none
 */
static void zclSampleLight_LevelControlStopCB( zclLCStop_t *pCmd )
{
  // TODO: process pCmd

  // stop immediately
  if(UtilTimer_isActive(&LevelControlClkStruct) == true)
  {
      UtilTimer_stop(&LevelControlClkStruct);
  }

  zclSampleLight_LevelRemainingTime = 0;
}

/*********************************************************************
 * @fn      zclSampleLight_LevelControlMoveToClosestFrequencyCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Level Control Move To
 *          Closest Frequency Command for this application.
 *
 * @param   pCmd - ZigBee command parameters
 *
 * @return  none
 */
static void zclSampleLight_LevelControlMoveToClosestFrequencyCB( zclLCMoveFreq_t *pCmd )
{
  // TODO
}
#endif

/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/

/*********************************************************************
 * @fn      zclSampleLight_SceneRecallCB
 *
 * @brief   Callback from the ZCL Scenes Cluster Library
 *          to recall a set of attributes from a stored scene.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleLight_SceneRecallCB( zclSceneReq_t *pReq )
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

         //If On/Off then retrieve the attribute
         if(extField.ClusterID == ZCL_CLUSTER_ID_GENERAL_ON_OFF)
         {
             uint8_t tempState = *extField.AttrBuf;
             zclSampleLight_updateOnOffAttribute(tempState);
         }
#ifdef ZCL_LEVEL_CTRL
         //If Level Control then retrieve the attribute
         else if(extField.ClusterID == ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL)
         {
             uint8_t tempState = *extField.AttrBuf;
             zclSampleLight_updateCurrentLevelAttribute(tempState);
         }
#endif
         //Move to the next extension field (increase pointer by clusterId, Attribute len field, and attribute)
         pBuf += sizeof(uint16_t) + sizeof(uint8_t) + extField.AttrLen;
         extLen += sizeof(uint16_t) + sizeof(uint8_t) + extField.AttrLen;  //increase ExtField
     }

    //Update scene attributes
    zclSampleLight_ScenesValid = TRUE;
    zclSampleLight_ScenesCurrentGroup = pReq->scene->groupID;
    zclSampleLight_ScenesCurrentScene = pReq->scene->ID;

    zclSampleLight_UpdateLedState();

}


/*********************************************************************
 * @fn      zclSampleLight_SceneStoreCB
 *
 * @brief   Callback from the ZCL Scenes Cluster Library
 *          to store a set of attributes to a specific scene.
 *
 * @param   none
 *
 * @return  none
 */
static uint8_t zclSampleLight_SceneStoreCB( zclSceneReq_t *pReq )
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

        //If On/Off then store attribute
        if(extField.ClusterID == ZCL_CLUSTER_ID_GENERAL_ON_OFF)
        {
            uint8_t tempState = zclSampleLight_getOnOffAttribute();
            if(*extField.AttrBuf != tempState )
            {
                sceneChanged = TRUE;
            }
            *extField.AttrBuf = tempState;
        }
#ifdef ZCL_LEVEL_CTRL
        //If Level Control then store attribute
        else if(extField.ClusterID == ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL)
        {
            uint8_t tempState = zclSampleLight_getCurrentLevelAttribute();
            if(*extField.AttrBuf != tempState )
            {
                sceneChanged = TRUE;
            }
            *extField.AttrBuf = tempState;
        }
#endif
        //Move to the next extension field (increase pointer by clusterId, Attribute len field, and attribute)
        pBuf += sizeof(uint16_t) + sizeof(uint8_t) + extField.AttrLen;
        extLen += sizeof(uint16_t) + sizeof(uint8_t) + extField.AttrLen;  //increase ExtField
    }

    //Update scene attributes
    zclSampleLight_ScenesValid = TRUE;
    zclSampleLight_ScenesCurrentGroup = pReq->scene->groupID;
    zclSampleLight_ScenesCurrentScene = pReq->scene->ID;

    return sceneChanged;
}




/*********************************************************************
 * @fn      zclSampleLight_ReadWriteAttrCB
 *
 * @brief   Handle ATTRID_SCENES_SCENE_COUNT, ATTRID_ON_OFF_ON_OFF and ATTRID_LEVEL_CURRENT_LEVEL.
 *          Only to be called if any of the attributes change.
 *
 * @param   clusterId - cluster that attribute belongs to
 * @param   attrId - attribute to be read or written
 * @param   oper - ZCL_OPER_LEN, ZCL_OPER_READ, or ZCL_OPER_WRITE
 * @param   pValue - pointer to attribute value, OTA endian
 * @param   pLen - length of attribute value read, native endian
 *
 * @return  status
 */
ZStatus_t zclSampleLight_ReadWriteAttrCB( uint16_t clusterId, uint16_t attrId, uint8_t oper,
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
 * @fn      zclSampleLight_ProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  uint8_t - TRUE if got handled
 */
static uint8_t zclSampleLight_ProcessIncomingMsg( zclIncoming_t *pInMsg )
{
  uint8_t handled = FALSE;

  switch ( pInMsg->hdr.commandID )
  {
#ifdef ZCL_READ
    case ZCL_CMD_READ_RSP:
      zclSampleLight_ProcessInReadRspCmd( pInMsg );
      handled = TRUE;
      break;
#endif
#ifdef ZCL_WRITE
    case ZCL_CMD_WRITE_RSP:
      zclSampleLight_ProcessInWriteRspCmd( pInMsg );
      handled = TRUE;
      break;
#endif
    case ZCL_CMD_CONFIG_REPORT:
    case ZCL_CMD_CONFIG_REPORT_RSP:
    case ZCL_CMD_READ_REPORT_CFG:
    case ZCL_CMD_READ_REPORT_CFG_RSP:
    case ZCL_CMD_REPORT:
      //bdb_ProcessIncomingReportingMsg( pInMsg );
      break;

    case ZCL_CMD_DEFAULT_RSP:
      zclSampleLight_ProcessInDefaultRspCmd( pInMsg );
      handled = TRUE;
      break;
#ifdef ZCL_DISCOVER
    case ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP:
      zclSampleLight_ProcessInDiscCmdsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_CMDS_GEN_RSP:
      zclSampleLight_ProcessInDiscCmdsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_ATTRS_RSP:
      zclSampleLight_ProcessInDiscAttrsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_ATTRS_EXT_RSP:
      zclSampleLight_ProcessInDiscAttrsExtRspCmd( pInMsg );
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
 * @fn      zclSampleLight_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleLight_ProcessInReadRspCmd( zclIncoming_t *pInMsg )
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

  return ( TRUE );
}
#endif // ZCL_READ

#ifdef ZCL_WRITE
/*********************************************************************
 * @fn      zclSampleLight_ProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleLight_ProcessInWriteRspCmd( zclIncoming_t *pInMsg )
{
  zclWriteRspCmd_t *writeRspCmd;
  uint8_t i;

  writeRspCmd = (zclWriteRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < writeRspCmd->numAttr; i++ )
  {
    // Notify the device of the results of the its original write attributes
    // command.
  }

  return ( TRUE );
}
#endif // ZCL_WRITE

/*********************************************************************
 * @fn      zclSampleLight_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleLight_ProcessInDefaultRspCmd( zclIncoming_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;

  // Device is notified of the Default Response command.
  (void)pInMsg;

  return ( TRUE );
}

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      zclSampleLight_ProcessInDiscCmdsRspCmd
 *
 * @brief   Process the Discover Commands Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleLight_ProcessInDiscCmdsRspCmd( zclIncoming_t *pInMsg )
{
  zclDiscoverCmdsCmdRsp_t *discoverRspCmd;
  uint8_t i;

  discoverRspCmd = (zclDiscoverCmdsCmdRsp_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numCmd; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return ( TRUE );
}

/*********************************************************************
 * @fn      zclSampleLight_ProcessInDiscAttrsRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleLight_ProcessInDiscAttrsRspCmd( zclIncoming_t *pInMsg )
{
  zclDiscoverAttrsRspCmd_t *discoverRspCmd;
  uint8_t i;

  discoverRspCmd = (zclDiscoverAttrsRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return ( TRUE );
}

/*********************************************************************
 * @fn      zclSampleLight_ProcessInDiscAttrsExtRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Extended Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleLight_ProcessInDiscAttrsExtRspCmd( zclIncoming_t *pInMsg )
{
  zclDiscoverAttrsExtRsp_t *discoverRspCmd;
  uint8_t i;

  discoverRspCmd = (zclDiscoverAttrsExtRsp_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return ( TRUE );
}
#endif // ZCL_DISCOVER


void zclSampleLight_UiActionToggleLight(const int32_t _itemEntry)
{
  zclSampleLight_OnOffCB(COMMAND_ON_OFF_TOGGLE);
}




void zclSampleLight_UiActionSwitchDiscovery(const int32_t _itemEntry)
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

    zclGeneral_SendIdentifyQuery(SAMPLELIGHT_ENDPOINT, &destAddr, FALSE, Rsp.zclFrameCounter);

}


static void zclSampleLight_UpdateLedState(void)
{

#ifndef CUI_DISABLE
  // set the LED1 based on light (on or off)
  if ( LIGHT_ON == zclSampleLight_getOnOffAttribute())
  {
#ifdef ZCL_LEVEL_CTRL
    uint8_t lightLevel = zclSampleLight_getCurrentLevelAttribute();

    // lightLevel is a value from 0-255. We must map this to a percentage (0-100%)
    LED_stopBlinking(gRedLedHandle);
    LED_setOn(gRedLedHandle, (lightLevel * 100) / 255);
#else
    LED_stopBlinking(gRedLedHandle);
    LED_setOn(gRedLedHandle, LED_BRIGHTNESS_MAX);
    #endif
  }
  else
  {
    LED_stopBlinking(gRedLedHandle);
    LED_setOff(gRedLedHandle);
  }
#endif

}


/****************************************************************************
****************************************************************************/


#ifndef CUI_DISABLE
/*********************************************************************
 * @fn      zclSampleLight_processKey
 *
 * @brief   Key event handler function
 *
 * @param   key - key to handle action for
 *          buttonEvents - event to handle action for
 *
 * @return  none
 */
static void zclSampleLight_processKey(uint8_t key, Button_EventMask buttonEvents)
{
    if (buttonEvents & Button_EV_CLICKED)
    {
        if(key == CONFIG_BTN_LEFT)
        {
            zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;

            zstack_bdbStartCommissioningReq.commissioning_mode = zclSampleLight_BdbCommissioningModes;
            Zstackapi_bdbStartCommissioningReq(appServiceTaskId, &zstack_bdbStartCommissioningReq);
        }
        if(key == CONFIG_BTN_RIGHT)
        {

        }
    }
}

#ifdef DMM_OAD
/*********************************************************************
 * @fn      zclSampleLight_dmmPausePolicyCb
 *
 * @brief   DMM Policy callback to pause the stack
 */
static void zclSampleLight_dmmPausePolicyCb(uint16_t pause)
{
    zstack_pauseResumeDeviceReq_t zstack_pauseResumeDeviceReq;
    zstack_pauseResumeDeviceReq.pause = pause;
    Zstackapi_pauseResumeDeviceReq(appServiceTaskId, &zstack_pauseResumeDeviceReq);
}
#endif


static void zclSampleLight_InitializeStatusLine(CUI_clientHandle_t gCuiHandle)
{
    /* Request Async Line for Light application Info */
    CUI_statusLineResourceRequest(gCuiHandle, "   APP Info"CUI_DEBUG_MSG_START"1"CUI_DEBUG_MSG_END, false, &gSampleLightInfoLine);

    zclSampleLight_UpdateStatusLine();
}


static void zclSampleLight_UpdateStatusLine(void)
{

    char lineFormat[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};


    strcpy(lineFormat, "["CUI_COLOR_YELLOW"Light State"CUI_COLOR_RESET"]");
    // set the LED1 based on light (on or off)
    if ( LIGHT_ON == zclSampleLight_getOnOffAttribute())
    {
        strcat(lineFormat, CUI_COLOR_GREEN" On "CUI_COLOR_RESET);
    }
    else
    {
        strcat(lineFormat, CUI_COLOR_RED" Off"CUI_COLOR_RESET);
    }
#ifdef ZCL_LEVEL_CTRL
    strcat(lineFormat, " ["CUI_COLOR_YELLOW"Level"CUI_COLOR_RESET"] %03d");
    CUI_statusLinePrintf(gCuiHandle, gSampleLightInfoLine, lineFormat, zclSampleLight_getCurrentLevelAttribute());
#else
    CUI_statusLinePrintf(gCuiHandle, gSampleLightInfoLine, lineFormat);
#endif
}




#if defined (ENABLE_GREENPOWER_COMBO_BASIC)

void zclSampleLight_setGPSinkCommissioningMode(const int32_t _itemEntry)
{
  bool zclSampleLight_SetSinkCommissioningMode = 0;

  //Toggle current commissioning flag value and set that value again
  zclSampleLight_SetSinkCommissioningMode = !gp_GetSinkCommissioningMode();
  gp_SetSinkCommissioningMode(zclSampleLight_SetSinkCommissioningMode);

  UI_UpdateGpStatusLine();

}
#endif
#endif

#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
/*********************************************************************
 * @fn      zclSampleLight_GPSink_Identify
 *
 * @brief   Callback to process Identify command from a GPD
 *
 * @param   zclGpNotification
 *
 * @return  none
 */
static void zclSampleLight_GPSink_Identify(zclGpNotification_t *zclGpNotification)
{
  afAddrType_t  dstAddr;

  dstAddr.endPoint = SAMPLELIGHT_ENDPOINT;
  dstAddr.panId = 0;
  dstAddr.addrMode = afAddr16Bit;
  dstAddr.addr.shortAddr = _NIB.nwkDevAddress;

  //Identify is a payloadless command which triggers a 60 seconds identify in the device (doc 14-0563-16 GP spec Table 49)
  zclGeneral_SendIdentify(SAMPLELIGHT_ENDPOINT,&dstAddr,60,TRUE, 1);
}

/*********************************************************************
 * @fn      zclSampleLight_GPSink_Off
 *
 * @brief   Callback to process Off command from a GPD
 *
 * @param   zclGpNotification
 *
 * @return  none
 */
static void zclSampleLight_GPSink_Off(zclGpNotification_t *zclGpNotification)
{
  zclSampleLight_OnOffCB(COMMAND_ON_OFF_OFF);
}

/*********************************************************************
 * @fn      zclSampleLight_GPSink_On
 *
 * @brief   Callback to process On command from a GPD
 *
 * @param   zclGpNotification
 *
 * @return  none
 */
static void zclSampleLight_GPSink_On(zclGpNotification_t *zclGpNotification)
{
  zclSampleLight_OnOffCB(COMMAND_ON_OFF_ON);
}

/*********************************************************************
 * @fn      zclSampleLight_GPSink_Toggle
 *
 * @brief   Callback to process Toggle command from a GPD
 *
 * @param   zclGpNotification
 *
 * @return  none
 */
static void zclSampleLight_GPSink_Toggle(zclGpNotification_t *zclGpNotification)
{
  zclSampleLight_OnOffCB(COMMAND_ON_OFF_TOGGLE);
}
#endif









