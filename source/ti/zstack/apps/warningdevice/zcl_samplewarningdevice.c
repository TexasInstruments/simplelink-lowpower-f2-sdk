/**************************************************************************************************
  Filename:       zcl_samplewarningdevice.c
  Revised:        $Date: 2015-08-19 17:11:00 -0700 (Wed, 19 Aug 2015) $
  Revision:       $Revision: 44460 $

  Description:    Zigbee Cluster Library - warning device application.


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
  This application implements a Zigbee IAS Warning Device, based on Z-Stack 3.0.

  This application is based on the common sample-application user interface. Please see the main
  comment in zcl_sampleapp_ui.c. The rest of this comment describes only the content specific for
  this sample application.

  Application-specific UI peripherals being used:

  - LEDs:
    LED1:
      - Off: Indicates no alarm.
      - On: Received warning message from another device.
      - Blinking: Alarmed in local device.
      - Single Blink: Received Squawk from CIE.

  Application-specific menu system:
    <DISCOVER> Sets Warning Device into Identify mode. Once the discovery process is done,
      this triggers the commissioning method on CIE to write IEEE address into this device.

    <ENROLLMENT MODE> Sets the enrollment mode to enroll in zone.
      TRIP-TO-PAIR: Zone should perform manual Zone Enroll Request via user action (e.g. through UI)
        Enabled by default, use <SEND ENROLL REQ> to perform this action.
      AUTO ENROLL REQUEST: Zone should automatically perform Zone Enroll Request after CIE IEEE Addr
        attribute is written by the CIE.

    <SEND ENROLL REQ> Manually sends Zone Enroll Request to CIE, if CIE IEEE Addr is written to
      local device. Used for Trip-to-Pair enrollment method.

    <TOGGLE ALARM> Changes alarm state of the local device, updating the Zone Alarmed status line
      and sends a Zone Change notification to the CIE.

    The APP Info line will display the following information:
      [CIE Address]
        XXXXXXXXXXXXXXXX - CIE IEEE Addr attribute populated by the CIE
      [Zone Id]
        0xXX - Zone ID
      [Zone Enrolled]
        Yes/No - Indicates whether device was successfully enrolled
      [Zone Type]
        Fire detector - Indicates the type of Zone (currently will always show as Fire detector)
      [Zone Alarmed]
        Yes/No - Indicates whether Zone is currently Alarmed or not
      [Strobe Alarmed]
        Yes/No - Indicates whether warning message is received from CIE or not

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

#include "zcl_samplewarningdevice.h"

#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>

#include "ti_drivers_config.h"

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
#include "addr_mgr.h"
#include <string.h>


#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
#include "gp_common.h"
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
// Semaphore used to post events to the application thread
static Semaphore_Handle appSemHandle;
static Semaphore_Struct appSem;

/* App service ID used for messaging with stack service task */
static uint8_t  appServiceTaskId;
/* App service task events, set by the stack service task when sending a message */
static uint32_t appServiceTaskEvents;
static endPointDesc_t  zclSampleWarningDeviceEpDesc = {0};

#if ZG_BUILD_ENDDEVICE_TYPE
static ClockP_Handle EndDeviceRejoinClkHandle;
static ClockP_Struct EndDeviceRejoinClkStruct;
#endif

static ClockP_Handle AutoEnrollReqClkHandle;
static ClockP_Struct AutoEnrollReqClkStruct;
static ClockP_Struct SquawkClkStruct;
static ClockP_Handle WarningDurationClkHandle;
static ClockP_Struct WarningDurationClkStruct;
#ifndef CUI_DISABLE
static ClockP_Handle SquawkClkHandle;
#endif

// Passed in function pointers to the NV driver
static NVINTF_nvFuncts_t *pfnZdlNV = NULL;

#ifndef CUI_DISABLE
static uint16_t zclSampleWarningDevice_BdbCommissioningModes;
#endif

afAddrType_t zclSampleWarningDevice_DstAddr;

//Temporal variable to store the CIE endpoint
static uint8_t tempCIEsrcEndpoint = 0;

#ifdef ZCL_WD
uint8_t tempWarningInProgress = 0;
bool gSquawkReq = FALSE;
#endif

static LED_Handle gRedLedHandle;

#ifndef CUI_DISABLE
CONST char zclSampleWarningDevice_appStr[] = APP_TITLE_STR;
CUI_clientHandle_t gCuiHandle;
static uint32_t gSampleWarningDeviceInfoLine1;
static uint32_t gSampleWarningDeviceInfoLine2;
static uint32_t gSampleWarningDeviceInfoLine3;
#endif

// Modify this to select the default Zone Enrollment Mode.
// Choices are (per spec):
//    TRIP_TO_PAIR
//    AUTO_ENROLL_REQUEST
static IAS_Zone_EnrollmentModes_t zclSampleWarningDevice_enrollmentMode = TRIP_TO_PAIR;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void zclSampleWarningDevice_initialization(void);
static void zclSampleWarningDevice_process_loop(void);
static void zclSampleWarningDevice_initParameters(void);
static void zclSampleWarningDevice_processZStackMsgs(zstackmsg_genericReq_t *pMsg);
static void SetupZStackCallbacks(void);
static void zclSampleWarningDevice_processAfIncomingMsgInd(zstack_afIncomingMsgInd_t *pInMsg);
static void zclSampleWarningDevice_initializeClocks(void);
#if ZG_BUILD_ENDDEVICE_TYPE
static void zclSampleWarningDevice_processEndDeviceRejoinTimeoutCallback(UArg a0);
#endif
static void zclSampleWarningDevice_processWarningDurationTimeoutCallback(UArg a0);
static void zclSampleWarningDevice_processAutoEnrollReqTimeoutCallback(UArg a0);
#ifndef CUI_DISABLE
static void zclSampleWarningDevice_processSquawkClkTimeoutCallback(UArg a0);

static void zclSampleWarningDevice_processKey(uint8_t key, Button_EventMask buttonEvents);
static void zclSampleWarningDevice_RemoveAppNvmData(void);
static void zclSampleWarningDevice_InitializeStatusLine(CUI_clientHandle_t gCuiHandle);
static void zclSampleWarningDevice_UpdateStatusLine(void);
#endif
static void zclSampleWarningDevice_Init( void );

static void zclSampleWarningDevice_BasicResetCB( void );
static void zclSampleWarningDevice_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg);



// Functions to process ZCL Foundation incoming Command/Response messages
static uint8_t zclSampleWarningDevice_ProcessIncomingMsg( zclIncoming_t *msg );
#ifdef ZCL_READ
static uint8_t zclSampleWarningDevice_ProcessInReadRspCmd( zclIncoming_t *pInMsg );
#endif
#ifdef ZCL_WRITE
static uint8_t zclSampleWarningDevice_ProcessInWriteRspCmd( zclIncoming_t *pInMsg );
#endif
static uint8_t zclSampleWarningDevice_ProcessInDefaultRspCmd( zclIncoming_t *pInMsg );
#ifdef ZCL_DISCOVER
static uint8_t zclSampleWarningDevice_ProcessInDiscCmdsRspCmd( zclIncoming_t *pInMsg );
static uint8_t zclSampleWarningDevice_ProcessInDiscAttrsRspCmd( zclIncoming_t *pInMsg );
static uint8_t zclSampleWarningDevice_ProcessInDiscAttrsExtRspCmd( zclIncoming_t *pInMsg );
#endif

#ifdef ZCL_WD
//WD Cluster
static ZStatus_t zclSampleWarningDevice_StartWarningCB(zclWDStartWarning_t *pCmd);
static ZStatus_t zclSampleWarningDevice_SquawkCB(zclWDSquawk_t *squawk);
#endif

#ifdef ZCL_ZONE
static void zclSampleWarningDevice_SendEnrollReqToCIE(void);
static ZStatus_t zclSampleWarningDevice_EnrollResponseCB(zclZoneEnrollRsp_t *rsp);
static uint8_t zclSampleWarningDevice_AuthenticateCIE(afAddrType_t *srcAddr, zclAttrRec_t *pAttr, uint8_t oper);
#ifndef CUI_DISABLE
static void zclSampleWarningDevice_SendChangeNotification(void);
static void ArrayToString (uint8_t * buf, char * str, uint8_t num_of_digists, bool big_endian);
#endif
#endif // ZCL_ZONE


/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * REFERENCED EXTERNALS
 */

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclSampleWarningDevice_CmdCallbacks =
{
  zclSampleWarningDevice_BasicResetCB,    // Basic Cluster Reset command
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


/*********************************************************************
* ZCL SS Profile Callback table
*/
static zclSS_AppCallbacks_t zclSampleWarningDevice_SSCmdCallbacks =
{
  NULL,                                         // Change Notification command
  NULL,                                         // Enroll Request command
  zclSampleWarningDevice_EnrollResponseCB,      // Enroll Reponse command
  NULL,                                         // Initiate Normal Operation Mode command
  NULL,                                         // Initiate Test Mode command
  NULL,                                         // Arm command
  NULL,                                         // Bypass command
  NULL,                                         // Emergency command
  NULL,                                         // Fire command
  NULL,                                         // Panic command
  NULL,                                         // Get Zone ID Map command
  NULL,                                         // Get Zone Information Command
  NULL,                                         // Get Panel Status Command
  NULL,                                         // Get Bypassed Zone List Command
  NULL,                                         // Get Zone Status Command
  NULL,                                         // ArmResponse command
  NULL,                                         // Get Zone ID Map Response command
  NULL,                                         // Get Zone Information Response command
  NULL,                                         // Zone Status Changed command
  NULL,                                         // Panel Status Changed command
  NULL,                                         // Get Panel Status Response command
  NULL,                                         // Set Bypassed Zone List command
  NULL,                                         // Bypass Response command
  NULL,                                         // Get Zone Status Response command
  zclSampleWarningDevice_StartWarningCB,        // Start Warning command
  zclSampleWarningDevice_SquawkCB               // Squawk command
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
  zclSampleWarningDevice_initialization();

  // No return from task process
  zclSampleWarningDevice_process_loop();
}




/*******************************************************************************
 * @fn          zclSampleWarningDevice_initialization
 *
 * @brief       Initialize the application
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleWarningDevice_initialization(void)
{
    /* Initialize user clocks */
    zclSampleWarningDevice_initializeClocks();

    /* create semaphores for messages / events
     */
    Semaphore_Params semParam;
    Semaphore_Params_init(&semParam);
    semParam.mode = ti_sysbios_knl_Semaphore_Mode_COUNTING;
    Semaphore_construct(&appSem, 0, &semParam);
    appSemHandle = Semaphore_handle(&appSem);

    appServiceTaskId = OsalPort_registerTask(Task_self(), appSemHandle, &appServiceTaskEvents);


    //Initialize stack
    zclSampleWarningDevice_Init();
}



/*********************************************************************
 * @fn          zclSampleWarningDevice_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleWarningDevice_Init( void )
{

  // Set destination address to indirect
  zclSampleWarningDevice_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
  zclSampleWarningDevice_DstAddr.endPoint = 0;
  zclSampleWarningDevice_DstAddr.addr.shortAddr = 0;

  //Register Endpoint
  zclSampleWarningDeviceEpDesc.endPoint = SAMPLEWARNINGDEVICE_ENDPOINT;
  zclSampleWarningDeviceEpDesc.simpleDesc = &zclSampleWarningDevice_SimpleDesc;
  zclport_registerEndpoint(appServiceTaskId, &zclSampleWarningDeviceEpDesc);

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
  gp_endpointInit(appServiceTaskId);
#endif

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( SAMPLEWARNINGDEVICE_ENDPOINT, &zclSampleWarningDevice_CmdCallbacks );

  // Register the ZCL Security and Safety Cluster Library callback functions
  zclSS_RegisterCmdCallbacks( SAMPLEWARNINGDEVICE_ENDPOINT, &zclSampleWarningDevice_SSCmdCallbacks );

  // Register the application's attribute list and reset to default values
  zclSampleWarningDevice_ResetAttributesToDefaultValues();
  zcl_registerAttrList( SAMPLEWARNINGDEVICE_ENDPOINT, zclSampleWarningDevice_NumAttributes, zclSampleWarningDevice_Attrs );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zclport_registerZclHandleExternal(SAMPLEWARNINGDEVICE_ENDPOINT, zclSampleWarningDevice_ProcessIncomingMsg);

  zcl_registerReadWriteCB( SAMPLEWARNINGDEVICE_ENDPOINT, NULL, zclSampleWarningDevice_AuthenticateCIE );

  //Write the bdb initialization parameters
  zclSampleWarningDevice_initParameters();

  //Setup ZDO callbacks
  SetupZStackCallbacks();


#if defined ( BDB_TL_INITIATOR )
  zclSampleWarningDevice_BdbCommissioningModes |= BDB_COMMISSIONING_MODE_INITIATOR_TL;
#endif

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
  app_Green_Power_Init(appServiceTaskId, &appServiceTaskEvents, appSemHandle, SAMPLEAPP_PROCESS_GP_DATA_SEND_EVT,
                       SAMPLEAPP_PROCESS_GP_EXPIRE_DUPLICATE_EVT, SAMPLEAPP_PROCESS_GP_TEMP_MASTER_EVT);
#endif

#ifndef CUI_DISABLE
  // set up default application BDB commissioning modes based on build type
  if(ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE)
  {
    zclSampleWarningDevice_BdbCommissioningModes = BDB_COMMISSIONING_MODE_NWK_FORMATION | BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_FINDING_BINDING;
  }
  else if (ZG_BUILD_JOINING_TYPE && ZG_DEVICE_JOINING_TYPE)
  {
    zclSampleWarningDevice_BdbCommissioningModes = BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_FINDING_BINDING;
  }


  gCuiHandle = UI_Init( appServiceTaskId,                     // Application Task ID
           &appServiceTaskEvents,                // The events processed by the sample application
           appSemHandle,                         // Semaphore to post the events in the application thread
           &zclSampleWarningDevice_IdentifyTime,
           &zclSampleWarningDevice_BdbCommissioningModes,   // A pointer to the application's bdbCommissioningModes
           zclSampleWarningDevice_appStr,                   // A pointer to the app-specific name string
           zclSampleWarningDevice_processKey,               // A pointer to the app-specific key process function
           zclSampleWarningDevice_RemoveAppNvmData          // A pointer to the app-specific NV Item reset function
           );

  //Request the Red LED for App
  LED_Params ledParams;
  LED_Params_init(&ledParams);
  gRedLedHandle = LED_open(CONFIG_LED_RED, &ledParams);

  //Initialize the SampleWarningDevice UI status line
  zclSampleWarningDevice_InitializeStatusLine(gCuiHandle);
#endif


  /* Initialize IAS ZONE Nv Items */
  if(SUCCESS == zclport_initializeNVItem(IAS_ZONE_NV_ID, IAS_ZONE_ZONE_STATE_NV_SUBID, sizeof(zclSampleWarningDevice_ZoneState),&zclSampleWarningDevice_ZoneState))
  {
      //retrieve the Zone State, either enrolled or not
      zclport_readNV(IAS_ZONE_NV_ID, IAS_ZONE_ZONE_STATE_NV_SUBID,0, sizeof(zclSampleWarningDevice_ZoneState),&zclSampleWarningDevice_ZoneState);
  }
  if(SUCCESS == zclport_initializeNVItem(IAS_ZONE_NV_ID, IAS_ZONE_CIE_ADDRESS_NV_SUBID, sizeof(zclSampleWarningDevice_CIE_Address),zclSampleWarningDevice_CIE_Address))
  {
      //retrieve the CIE Address
      zclport_readNV(IAS_ZONE_NV_ID, IAS_ZONE_CIE_ADDRESS_NV_SUBID,0, sizeof(zclSampleWarningDevice_CIE_Address),zclSampleWarningDevice_CIE_Address);
  }
  if(SUCCESS == zclport_initializeNVItem(IAS_ZONE_NV_ID, IAS_ZONE_ZONE_ID_NV_SUBID, sizeof(zclSampleWarningDevice_ZoneId),&zclSampleWarningDevice_ZoneId))
  {
      //retrieve the Zone ID
      zclport_readNV(IAS_ZONE_NV_ID, IAS_ZONE_ZONE_ID_NV_SUBID, 0, sizeof(zclSampleWarningDevice_ZoneId),&zclSampleWarningDevice_ZoneId);
  }

  // Call BDB initialization. Should be called once from application at startup to restore
  // previous network configuration, if applicable.
  zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;
  zstack_bdbStartCommissioningReq.commissioning_mode = 0;
  Zstackapi_bdbStartCommissioningReq(appServiceTaskId,&zstack_bdbStartCommissioningReq);
}

#ifndef CUI_DISABLE
/*********************************************************************
 * @fn          zclSampleWarningDevice_RemoveAppNvmData
 *
 * @brief       Callback when Application performs reset to Factory New Reset.
 *              Application must restore the application to default values
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleWarningDevice_RemoveAppNvmData(void)
{
    zclSampleWarningDevice_BasicResetCB();
}
#endif


static void zclSampleWarningDevice_initParameters(void)
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
 * @fn      zclSampleWarningDevice_initializeClocks
 *
 * @brief   Initialize Clocks
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleWarningDevice_initializeClocks(void)
{
#if ZG_BUILD_ENDDEVICE_TYPE
    // Initialize the timers needed for this application
    EndDeviceRejoinClkHandle = UtilTimer_construct(
    &EndDeviceRejoinClkStruct,
    zclSampleWarningDevice_processEndDeviceRejoinTimeoutCallback,
    SAMPLEAPP_END_DEVICE_REJOIN_DELAY,
    0, false, 0);
#endif

    WarningDurationClkHandle = UtilTimer_construct(
    &WarningDurationClkStruct,
    zclSampleWarningDevice_processWarningDurationTimeoutCallback,
    zclSampleWarningDevice_MaxDuration*1000,
    0, false, 0);


    AutoEnrollReqClkHandle = UtilTimer_construct(
    &AutoEnrollReqClkStruct,
    zclSampleWarningDevice_processAutoEnrollReqTimeoutCallback,
    SAMPLEAPP_AUTO_ENROLL_REQ_DELAY,
    0, false, 0);

#ifndef CUI_DISABLE
    SquawkClkHandle = UtilTimer_construct(
    &SquawkClkStruct,
    zclSampleWarningDevice_processSquawkClkTimeoutCallback,
    SAMPLEAPP_AUTO_ENROLL_REQ_DELAY,
    0, false, 0);
#endif

}

#if ZG_BUILD_ENDDEVICE_TYPE
/*******************************************************************************
 * @fn      zclSampleWarningDevice_processEndDeviceRejoinTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleWarningDevice_processEndDeviceRejoinTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    appServiceTaskEvents |= SAMPLEAPP_END_DEVICE_REJOIN_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}
#endif

/*******************************************************************************
 * @fn      zclSampleWarningDevice_processWarningDurationTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleWarningDevice_processWarningDurationTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    appServiceTaskEvents |= SAMPLEAPP_WARNING_DURATION_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}

#ifndef CUI_DISABLE
/*******************************************************************************
 * @fn      zclSampleWarningDevice_processEndDeviceRejoinTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleWarningDevice_processSquawkClkTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    appServiceTaskEvents |= SAMPLEAPP_SQUAWK_UI_TIMEOUT_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}
#endif

/*******************************************************************************
 * @fn      zclSampleWarningDevice_processEndDeviceRejoinTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleWarningDevice_processAutoEnrollReqTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    appServiceTaskEvents |= SAMPLEAPP_AUTO_ENROLL_REQ_EVT;

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

    (void)Zstackapi_DevZDOCBReq(appServiceTaskId, &zdoCBReq);
}



/*******************************************************************************
 * @fn      zclSampleWarningDevice_process_loop
 *
 * @brief   Application task processing start.
 *
 * @param   none
 *
 * @return  void
 */
static void zclSampleWarningDevice_process_loop(void)
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
                zclSampleWarningDevice_processZStackMsgs(pMsg);
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

            if(appServiceTaskEvents & SAMPLEAPP_WRITE_TO_CIE_DONE_EVT)
            {
                zstack_zdoSetBindUnbindAuthAddr_t zstack_zdoSetBindUnbindAuthAddr;

                //Set that the only remote device that can modify binds to the ZONE cluster is the CIE
                OsalPort_memcpy(zstack_zdoSetBindUnbindAuthAddr.AuthAddress, zclSampleWarningDevice_CIE_Address, Z_EXTADDR_LEN);
                zstack_zdoSetBindUnbindAuthAddr.Endpoint = SAMPLEWARNINGDEVICE_ENDPOINT;
                zstack_zdoSetBindUnbindAuthAddr.ClusterId = ZCL_CLUSTER_ID_SS_IAS_ZONE;
                Zstackapi_ZdoSetBindUnbindAuthAddrReq(appServiceTaskId, &zstack_zdoSetBindUnbindAuthAddr);

#ifndef CUI_DISABLE
                zclSampleWarningDevice_UpdateStatusLine();
#endif

                //save in nv the CIE Address
                zclport_writeNV(IAS_ZONE_NV_ID, IAS_ZONE_CIE_ADDRESS_NV_SUBID, sizeof(zclSampleWarningDevice_CIE_Address),zclSampleWarningDevice_CIE_Address);

                appServiceTaskEvents &= ~SAMPLEAPP_WRITE_TO_CIE_DONE_EVT;
            }


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

            if( appServiceTaskEvents & SAMPLEAPP_AUTO_ENROLL_REQ_EVT )
            {
                zclSampleWarningDevice_SendEnrollReqToCIE();

                appServiceTaskEvents &= ~SAMPLEAPP_AUTO_ENROLL_REQ_EVT;
            }
            if( appServiceTaskEvents & SAMPLEAPP_WARNING_DURATION_EVT )
            {
                tempWarningInProgress = FALSE;
                //Only if there is no fire in the local device
                if(zclSampleWarningDevice_ZoneStatus == 0)
                {
                  LED_stopBlinking(gRedLedHandle);
                  LED_setOff(gRedLedHandle);
                }
#ifndef CUI_DISABLE
                zclSampleWarningDevice_UpdateStatusLine();
#endif
                appServiceTaskEvents &= ~SAMPLEAPP_WARNING_DURATION_EVT;
            }

            if( appServiceTaskEvents & SAMPLEAPP_SQUAWK_UI_TIMEOUT_EVT)
            {
              gSquawkReq = FALSE;
#ifndef CUI_DISABLE
              zclSampleWarningDevice_UpdateStatusLine();
#endif
              appServiceTaskEvents &= ~SAMPLEAPP_SQUAWK_UI_TIMEOUT_EVT;
            }

        }
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
static void zclSampleWarningDevice_processZStackMsgs(zstackmsg_genericReq_t *pMsg)
{
      switch(pMsg->hdr.event)
      {
          case zstackmsg_CmdIDs_BDB_NOTIFICATION:
              {
                  zstackmsg_bdbNotificationInd_t *pInd;
                  pInd = (zstackmsg_bdbNotificationInd_t*)pMsg;
                  zclSampleWarningDevice_ProcessCommissioningStatus(&(pInd->Req));
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
                  zclSampleWarningDevice_processAfIncomingMsgInd( &(pInd->req) );
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
          case zstackmsg_CmdIDs_BDB_TC_LINK_KEY_EXCHANGE_NOTIFICATION_IND:
          case zstackmsg_CmdIDs_AF_DATA_CONFIRM_IND:
          case zstackmsg_CmdIDs_ZDO_DEVICE_ANNOUNCE:
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
 * @fn          zclSampleWarningDevice_processAfIncomingMsgInd
 *
 * @brief       Process AF Incoming Message Indication message
 *
 * @param       pInMsg - pointer to incoming message
 *
 * @return      none
 *
 */
static void zclSampleWarningDevice_processAfIncomingMsgInd(zstack_afIncomingMsgInd_t *pInMsg)
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




#ifndef CUI_DISABLE
/*********************************************************************
 * @fn      zclSampleWarningDevice_processKey
 *
 * @brief   Key event handler function
 *
 * @param   key - key to handle action for
 *          buttonEvents - event to handle action for
 *
 * @return  none
 */
static void zclSampleWarningDevice_processKey(uint8_t key, Button_EventMask buttonEvents)
{
    if (buttonEvents & Button_EV_CLICKED)
    {
        if(key == CONFIG_BTN_LEFT)
        {
            zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;

            zstack_bdbStartCommissioningReq.commissioning_mode = zclSampleWarningDevice_BdbCommissioningModes;
            Zstackapi_bdbStartCommissioningReq(appServiceTaskId,&zstack_bdbStartCommissioningReq);
        }
        if(key == CONFIG_BTN_RIGHT)
        {
            if(zclSampleWarningDevice_ZoneState == SS_IAS_ZONE_STATE_ENROLLED)
            {
                zclSampleWarningDevice_SendChangeNotification();
            }
            else
            {
                //Try Trip-To-Pair enrollment
                zclSampleWarningDevice_SendEnrollReqToCIE();
            }
        }
    }
}
#endif



/*********************************************************************
 * @fn      zclSampleWarningDevice_ProcessCommissioningStatus
 *
 * @brief   Callback in which the status of the commissioning process are reported
 *
 * @param   bdbCommissioningModeMsg - Context message of the status of a commissioning process
 *
 * @return  none
 */
static void zclSampleWarningDevice_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg)
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
#if ZG_BUILD_ENDDEVICE_TYPE
        // speed up polling until zone enrollment has occured
        zstack_sysConfigWriteReq_t writeReq = { 0 };
        writeReq.has_pollRate = true;
        writeReq.pollRate = 500;
        writeReq.pollRateType = POLL_RATE_TYPE_APP_1;
        Zstackapi_sysConfigWriteReq(appServiceTaskId, &writeReq);
#endif
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
 * @fn      zclSampleWarningDevice_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to  default values.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleWarningDevice_BasicResetCB( void )
{
  zclSampleWarningDevice_ResetAttributesToDefaultValues();

  //Update the attributes in Nv
  zclport_writeNV(IAS_ZONE_NV_ID, IAS_ZONE_CIE_ADDRESS_NV_SUBID, sizeof(zclSampleWarningDevice_CIE_Address),zclSampleWarningDevice_CIE_Address);
  zclport_writeNV(IAS_ZONE_NV_ID, IAS_ZONE_ZONE_ID_NV_SUBID, sizeof(zclSampleWarningDevice_ZoneId),&zclSampleWarningDevice_ZoneId);
  zclport_writeNV(IAS_ZONE_NV_ID, IAS_ZONE_ZONE_STATE_NV_SUBID, sizeof(zclSampleWarningDevice_ZoneState),&zclSampleWarningDevice_ZoneState);

  // update the display
#ifndef CUI_DISABLE
  zclSampleWarningDevice_UpdateStatusLine( );
#endif
}


#ifndef CUI_DISABLE
static void zclSampleWarningDevice_InitializeStatusLine(CUI_clientHandle_t gCuiHandle)
{
    /* Request Async Line for Light application Info */
    CUI_statusLineResourceRequest(gCuiHandle, "   APP Info"CUI_DEBUG_MSG_START"1"CUI_DEBUG_MSG_END, false, &gSampleWarningDeviceInfoLine1);
    CUI_statusLineResourceRequest(gCuiHandle, "   APP Info"CUI_DEBUG_MSG_START"2"CUI_DEBUG_MSG_END, false, &gSampleWarningDeviceInfoLine2);
    CUI_statusLineResourceRequest(gCuiHandle, "   APP Info"CUI_DEBUG_MSG_START"3"CUI_DEBUG_MSG_END, false, &gSampleWarningDeviceInfoLine3);

    zclSampleWarningDevice_UpdateStatusLine();
}


static void zclSampleWarningDevice_UpdateStatusLine(void)
{
    char lineFormat1[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};
    char lineFormat2[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};
    char lineFormat3[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};
    char StrCIEAddr[17];
    char StrzoneId[3];

    strcpy(lineFormat1, "["CUI_COLOR_YELLOW"CIE Address"CUI_COLOR_RESET"] ");

    //Copy the CIE address
    ArrayToString(zclSampleWarningDevice_CIE_Address, StrCIEAddr,16, FALSE);
    StrCIEAddr[16] = 0;
    strcat(lineFormat1,StrCIEAddr);


    //Indicate the Zone ID
    strcat(lineFormat1, " ["CUI_COLOR_YELLOW"Zone Id"CUI_COLOR_RESET"] 0x");
    ArrayToString(&zclSampleWarningDevice_ZoneId,StrzoneId,2,FALSE);
    StrzoneId[2] = 0;
    strcat(lineFormat1,StrzoneId);

    //Indicate Zone enrollment status
    strcat(lineFormat2, "["CUI_COLOR_YELLOW"Zone Enrolled"CUI_COLOR_RESET"] ");
    if(zclSampleWarningDevice_ZoneState == SS_IAS_ZONE_STATE_ENROLLED)
    {
        strcat(lineFormat2, "Yes");
    }
    else
    {
        strcat(lineFormat2, "No");
    }

    strcat(lineFormat2, " ["CUI_COLOR_YELLOW"Zone Type"CUI_COLOR_RESET"] Fire detector");

    //Indicate if Zone is alarmed
    strcat(lineFormat2, " ["CUI_COLOR_YELLOW"Zone Alarmed"CUI_COLOR_RESET"] ");
    if(zclSampleWarningDevice_ZoneStatus & SS_IAS_ZONE_STATUS_ALARM1_ALARMED)
    {
        strcat(lineFormat2, "Yes");
    }
    else
    {
        strcat(lineFormat2, "No");
    }

    //Indicate if Strobe is alarmed
    strcat(lineFormat3, "["CUI_COLOR_YELLOW"Strobe Alarmed"CUI_COLOR_RESET"] ");
    if(tempWarningInProgress)
    {
        strcat(lineFormat3, "Yes");
    }
    else
    {
        strcat(lineFormat3, "No");
    }

    if(gSquawkReq)
    {
        strcat(lineFormat3, " "CUI_COLOR_MAGENTA"SQUAWK!!!"CUI_COLOR_RESET);
        UtilTimer_stop( &SquawkClkStruct );
        UtilTimer_setTimeout( SquawkClkHandle, SAMPLEAPP_SQUAWK_UI_TIMEOUT );
        UtilTimer_start( &SquawkClkStruct );
    }

    CUI_statusLinePrintf(gCuiHandle, gSampleWarningDeviceInfoLine1, lineFormat1);
    CUI_statusLinePrintf(gCuiHandle, gSampleWarningDeviceInfoLine2, lineFormat2);
    CUI_statusLinePrintf(gCuiHandle, gSampleWarningDeviceInfoLine3, lineFormat3);
}

#endif



/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/

/*********************************************************************
 * @fn      zclSampleWarningDevice_ProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  uint8_t - TRUE if got handled
 */
static uint8_t zclSampleWarningDevice_ProcessIncomingMsg( zclIncoming_t *pInMsg )
{
  uint8_t handled = FALSE;

  switch ( pInMsg->hdr.commandID )
  {
#ifdef ZCL_READ
    case ZCL_CMD_READ_RSP:
      zclSampleWarningDevice_ProcessInReadRspCmd( pInMsg );
      handled = TRUE;
      break;
#endif
#ifdef ZCL_WRITE
    case ZCL_CMD_WRITE_RSP:
      zclSampleWarningDevice_ProcessInWriteRspCmd( pInMsg );
      handled = TRUE;
      break;
#endif
#ifdef ZCL_REPORT
    // See ZCL Test Applicaiton (zcl_testapp.c) for sample code on Attribute Reporting
    case ZCL_CMD_CONFIG_REPORT:
      //zclSampleWarningDevice_ProcessInConfigReportCmd( pInMsg );
      break;

    case ZCL_CMD_CONFIG_REPORT_RSP:
      //zclSampleWarningDevice_ProcessInConfigReportRspCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG:
      //zclSampleWarningDevice_ProcessInReadReportCfgCmd( pInMsg );
      break;

    case ZCL_CMD_READ_REPORT_CFG_RSP:
      //zclSampleWarningDevice_ProcessInReadReportCfgRspCmd( pInMsg );
      break;

    case ZCL_CMD_REPORT:
      //zclSampleWarningDevice_ProcessInReportCmd( pInMsg );
      break;
#endif
    case ZCL_CMD_DEFAULT_RSP:
      zclSampleWarningDevice_ProcessInDefaultRspCmd( pInMsg );
      handled = TRUE;
      break;
#ifdef ZCL_DISCOVER
    case ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP:
      zclSampleWarningDevice_ProcessInDiscCmdsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_CMDS_GEN_RSP:
      zclSampleWarningDevice_ProcessInDiscCmdsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_ATTRS_RSP:
      zclSampleWarningDevice_ProcessInDiscAttrsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_ATTRS_EXT_RSP:
      zclSampleWarningDevice_ProcessInDiscAttrsExtRspCmd( pInMsg );
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
 * @fn      zclSampleWarningDevice_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleWarningDevice_ProcessInReadRspCmd( zclIncoming_t *pInMsg )
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
 * @fn      zclSampleWarningDevice_ProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleWarningDevice_ProcessInWriteRspCmd( zclIncoming_t *pInMsg )
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

/*********************************************************************
 * @fn      zclSampleWarningDevice_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleWarningDevice_ProcessInDefaultRspCmd( zclIncoming_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;
  // Device is notified of the Default Response command.
  (void)pInMsg;
  return TRUE;
}

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      zclSampleWarningDevice_ProcessInDiscCmdsRspCmd
 *
 * @brief   Process the Discover Commands Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleWarningDevice_ProcessInDiscCmdsRspCmd( zclIncoming_t *pInMsg )
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
 * @fn      zclSampleWarningDevice_ProcessInDiscAttrsRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleWarningDevice_ProcessInDiscAttrsRspCmd( zclIncoming_t *pInMsg )
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
 * @fn      zclSampleWarningDevice_ProcessInDiscAttrsExtRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Extended Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleWarningDevice_ProcessInDiscAttrsExtRspCmd( zclIncoming_t *pInMsg )
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

#ifdef ZCL_WD
/*********************************************************************
* @fn      zclSampleWarningDevice_StartWarningCB
*
* @brief   Process in the received StartWarning Command.
*
* @param   pCmd - pointer to the incoming command
*
*/
static ZStatus_t zclSampleWarningDevice_StartWarningCB(zclWDStartWarning_t *pCmd)
{
  //Only blinks if the alarm is set
  if(((zclWDStartWarning_t *)pCmd->warningmessage.warningbits.warnMode != \
    SS_IAS_START_WARNING_WARNING_MODE_STOP)
     ||((zclWDStartWarning_t *)pCmd->warningmessage.warningbits.warnStrobe != \
       SS_IAS_START_WARNING_STROBE_NO_STROBE_WARNING))
  {
    //Only if there is no fire in the local device
    if(zclSampleWarningDevice_ZoneStatus == 0)
    {
        uint32_t timeout;
        if(zclSampleWarningDevice_MaxDuration < pCmd->warningDuration)
        {
            timeout = zclSampleWarningDevice_MaxDuration * 1000;
        }
        else
        {
            timeout = pCmd->warningDuration * 1000;
        }

        //Start the timer to process the warning duration
        UtilTimer_setTimeout( WarningDurationClkHandle, timeout );
        UtilTimer_start(&WarningDurationClkStruct);

        tempWarningInProgress = TRUE;
        LED_stopBlinking(gRedLedHandle);
        LED_setOn(gRedLedHandle, LED_BRIGHTNESS_MAX);
    }
  }
  else //if the alarm is not present
  {
    tempWarningInProgress = FALSE;

    //Only if there is no fire in the local device
    if(zclSampleWarningDevice_ZoneStatus == 0)
    {
      LED_stopBlinking(gRedLedHandle);
      LED_setOff(gRedLedHandle);
    }
  }

#ifndef CUI_DISABLE
  zclSampleWarningDevice_UpdateStatusLine();
#endif
  return (ZStatus_t)(0)  ;

}

/*********************************************************************
* @fn      zclSampleWarningDevice_SquawkCB
*
* @brief   Process in the received Squawk Command.
*
* @param   pCmd - pointer to the incoming command
*
*/
ZStatus_t zclSampleWarningDevice_SquawkCB(zclWDSquawk_t* squawk)
{
  zclWDSquawk_t squawks = *squawk;
  //Squawk is only valid if there is no warning in progress
  if(tempWarningInProgress == 0)
  {
    if((squawks.squawkbits.squawkMode == SS_IAS_SQUAWK_SQUAWK_MODE_SYSTEM_ALARMED_NOTIFICATION_SOUND)
       || (squawks.squawkbits.strobe == SS_IAS_SQUAWK_STROBE_USE_STROBE_BLINK_IN_PARALLEL_TO_SQUAWK))
    {
        //Only if there is no fire in the local device
        if(zclSampleWarningDevice_ZoneStatus == 0)
        {
          LED_startBlinking(gRedLedHandle, 500, 1);
          gSquawkReq = TRUE;
        }
    }
  }

#ifndef CUI_DISABLE
  zclSampleWarningDevice_UpdateStatusLine();
#endif

   return (ZStatus_t)(0)  ;
}
#endif // ZCL_WD



#ifdef ZCL_ZONE
/*******************************************************************************
* @fn      static void zclSampleWarningDevice_EnrollResponseCB
*
* @brief   Process incoming Enroll Response Command
*
* @param   Zone Enroll Response command
*
* @return  none
*/
static ZStatus_t zclSampleWarningDevice_EnrollResponseCB(zclZoneEnrollRsp_t *rsp)
{
    AddrMgrEntry_t entry;
    uint8_t InvalidCIEAddr = true;
    uint8_t index;

    for(index=0; index < Z_EXTADDR_LEN; index++)
    {
        if(zclSampleWarningDevice_CIE_Address[index] != 0xFF)
        {
            InvalidCIEAddr = false;
            index = Z_EXTADDR_LEN;
        }
    }

    //if we have a valid CIE Address, check that it comes from CIE
    if(!InvalidCIEAddr)
    {
        entry.user = ADDRMGR_USER_DEFAULT;
        entry.nwkAddr = INVALID_NODE_ADDR;
        //Get the short address of the CIE if exist
        OsalPort_memcpy(entry.extAddr, zclSampleWarningDevice_CIE_Address, Z_EXTADDR_LEN);
        AddrMgrEntryLookupExt(&entry);    //AddrMgr stack interface

        //if the frame does not comes from CIE, then do not process it
        if(entry.nwkAddr != rsp->srcAddr)
        {
            zstack_zdoNwkAddrReq_t zstack_zdoNwkAddrReq;

            OsalPort_memcpy(zstack_zdoNwkAddrReq.ieeeAddr, zclSampleWarningDevice_CIE_Address, Z_EXTADDR_LEN);
            zstack_zdoNwkAddrReq.startIndex = 0;
            zstack_zdoNwkAddrReq.type = zstack_NwkAddrReqType_SINGLE_DEVICE;

            Zstackapi_ZdoNwkAddrReq(appServiceTaskId, &zstack_zdoNwkAddrReq);

            //Maybe CIE changed address due to address conflict?, try to update its address will not hurt.
            return ZCL_STATUS_NOT_AUTHORIZED;
        }
    }

    switch(rsp->responseCode)
    {
      case SS_IAS_ZONE_STATUS_ENROLL_RESPONSE_CODE_SUCCESS:
      {
        //Must set the Zone State attribute to Enrolled if succesful
        zclSampleWarningDevice_ZoneState = SS_IAS_ZONE_STATE_ENROLLED;
        zclSampleWarningDevice_ZoneId = rsp->zoneID;

        //Save the Zone ID and the enrollment status in Nv
        zclport_writeNV(IAS_ZONE_NV_ID, IAS_ZONE_ZONE_ID_NV_SUBID, sizeof(zclSampleWarningDevice_ZoneId),&zclSampleWarningDevice_ZoneId);
        zclport_writeNV(IAS_ZONE_NV_ID, IAS_ZONE_ZONE_STATE_NV_SUBID, sizeof(zclSampleWarningDevice_ZoneState),&zclSampleWarningDevice_ZoneState);
#if ZG_BUILD_ENDDEVICE_TYPE
        // resume stack poll rate since we have finished zone enrollment
        zstack_sysConfigWriteReq_t writeReq = { 0 };
        writeReq.has_pollRate = true;
        writeReq.pollRate = POLL_RATE_MAX;
        writeReq.pollRateType = POLL_RATE_TYPE_APP_1;
        Zstackapi_sysConfigWriteReq(appServiceTaskId, &writeReq);
#endif
      }
    break;

        //The rest of the cases indicate that the enrollment was not succesful
        //dealt by printing error message
      default:
        break;
    }

#ifndef CUI_DISABLE
    zclSampleWarningDevice_UpdateStatusLine();
#endif

    return (ZStatus_t)(0)  ;
}

/*******************************************************************************
* @fn      zclSampleWarningDevice_SendEnrollReqToCIE
*
* @brief   Send Enroll Request to CIE if the IEEE address of CIE has been set and local device is not enrolled
*
* @param   none
*
* @return  none
*/
void zclSampleWarningDevice_SendEnrollReqToCIE(void)
{

  uint8_t temp[Z_EXTADDR_LEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  zstack_zdoBindReq_t Req;
  zstack_sysNwkInfoReadRsp_t  Rsp;


  Zstackapi_sysNwkInfoReadReq(appServiceTaskId, &Rsp);

  //fill in bind request to self
  Req.nwkAddr = Rsp.nwkAddr;
  OsalPort_memcpy(Req.bindInfo.srcAddr, Rsp.ieeeAddr, Z_EXTADDR_LEN);
  Req.bindInfo.dstAddr.addrMode = zstack_AFAddrMode_EXT;
  OsalPort_memcpy(Req.bindInfo.dstAddr.addr.extAddr, zclSampleWarningDevice_CIE_Address, Z_EXTADDR_LEN);
  Req.bindInfo.dstAddr.endpoint = tempCIEsrcEndpoint;
  Req.bindInfo.clusterID = ZCL_CLUSTER_ID_SS_IAS_ZONE;
  Req.bindInfo.srcEndpoint = SAMPLEWARNINGDEVICE_ENDPOINT;

  //create the bind to that device
  Zstackapi_ZdoBindReq(appServiceTaskId, &Req);

  //If we have the IEEE address of the CIE device and is not enrolled, then send enrollment request, otherwise we can't do it
  if( memcmp(temp, zclSampleWarningDevice_CIE_Address, Z_EXTADDR_LEN) && (zclSampleWarningDevice_ZoneState == SS_IAS_ZONE_STATE_NOT_ENROLLED))
  {
      AddrMgrEntry_t entry;

      entry.user = ADDRMGR_USER_DEFAULT;
      OsalPort_memcpy(entry.extAddr, zclSampleWarningDevice_CIE_Address, Z_EXTADDR_LEN);
      AddrMgrEntryLookupExt(&entry);

      if(entry.nwkAddr != INVALID_NODE_ADDR)
      {
          uint16_t manufacturerCode;
          uint8_t disableDefaultRsp = TRUE;
          zstack_getZCLFrameCounterRsp_t Rsp;

          manufacturerCode = BUILD_UINT16(ZDO_Config_Node_Descriptor.ManufacturerCode[0],ZDO_Config_Node_Descriptor.ManufacturerCode[1]);

          //get the appropriate sequence number
          Zstackapi_getZCLFrameCounterReq(appServiceTaskId, &Rsp);

          //Send enroll request
          zclSS_IAS_Send_ZoneStatusEnrollRequestCmd(SAMPLEWARNINGDEVICE_ENDPOINT, &zclSampleWarningDevice_DstAddr,
                                                    zclSampleWarningDevice_ZoneType, manufacturerCode,
                                                    disableDefaultRsp, Rsp.zclFrameCounter);
      }
  }

}

/*********************************************************************
 * @fn      zclSampleWarningDevice_AuthenticateCIE
 *
 * @brief   Authenticate write attribute operation to IAS Zone cluster
 *
 * @param   afAddrType_t *srcAddr - Address of the remote device requesting the operation
 * @param   zclAttrRec_t *pAttr   - Attribute record on which the operation will be performed
 * @param   uint8_t oper            - type of operation (write, read)
 * @return  none
 */
static uint8_t zclSampleWarningDevice_AuthenticateCIE( afAddrType_t *srcAddr, zclAttrRec_t *pAttr, uint8_t oper )
{
    //If it is not write, do not interfere with other operations.
    if(oper != ZCL_OPER_WRITE)
    {
        return  ZCL_STATUS_SUCCESS;
    }

    if(pAttr->clusterID == ZCL_CLUSTER_ID_SS_IAS_ZONE)
    {
        AddrMgrEntry_t entry;
        uint8_t InvalidCIEAddr = true;
        uint8_t index;

        for(index=0; index < Z_EXTADDR_LEN; index++)
        {
            if(zclSampleWarningDevice_CIE_Address[index] != 0xFF)
            {
                InvalidCIEAddr = false;
                index = Z_EXTADDR_LEN;
            }
        }

        //if we have a valid CIE Address, check that it comes from CIE
        if(!InvalidCIEAddr)
        {
            entry.user = ADDRMGR_USER_DEFAULT;
            entry.nwkAddr = INVALID_NODE_ADDR;
            //Get the short address of the CIE if exist
            OsalPort_memcpy(entry.extAddr, zclSampleWarningDevice_CIE_Address, Z_EXTADDR_LEN);
            AddrMgrEntryLookupExt(&entry);    //AddrMgr stack interface

            //if the frame does not comes from CIE, then do not process it
            if(entry.nwkAddr != srcAddr->addr.shortAddr)
            {
                zstack_zdoNwkAddrReq_t zstack_zdoNwkAddrReq;

                OsalPort_memcpy(zstack_zdoNwkAddrReq.ieeeAddr, zclSampleWarningDevice_CIE_Address, Z_EXTADDR_LEN);
                zstack_zdoNwkAddrReq.startIndex = 0;
                zstack_zdoNwkAddrReq.type = zstack_NwkAddrReqType_SINGLE_DEVICE;

                //Maybe CIE changed address due to address conflict?, try to update its address will not hurt.
                Zstackapi_ZdoNwkAddrReq(appServiceTaskId, &zstack_zdoNwkAddrReq);

                return ZCL_STATUS_NOT_AUTHORIZED;
            }
        }
        //Perform Auto-Enroll-Request (if enabled via UI)
        else if(pAttr->attr.attrId == ATTRID_SS_IAS_CIE_ADDRESS)
        {
            tempCIEsrcEndpoint = srcAddr->endPoint;

            if(zclSampleWarningDevice_enrollmentMode == AUTO_ENROLL_REQUEST)
            {
              //Start the timer to send auto enroll request
              UtilTimer_setTimeout( AutoEnrollReqClkHandle, SAMPLEAPP_AUTO_ENROLL_REQ_DELAY );
              UtilTimer_start( &AutoEnrollReqClkStruct );
            }

            //Set and event which will be process after write operation takes place.
            appServiceTaskEvents |= SAMPLEAPP_WRITE_TO_CIE_DONE_EVT;
            // Wake up the application thread when it waits for clock event
            Semaphore_post(appSemHandle);
        }
    }

   return ZCL_STATUS_SUCCESS;
}

#ifndef CUI_DISABLE
/*******************************************************************************
* @fn      zclSampleWarningDevice_SendChangeNotification
*
* @brief
*
* @param
*
* @return  none
*/
static void zclSampleWarningDevice_SendChangeNotification(void)
{
  uint8_t extendedStatus = 0;
  uint16_t delay = 0;

  zstack_getZCLFrameCounterRsp_t Rsp;
  uint8_t disableDefaultRsp = TRUE;

  // Using this as the "Alarm Switch"
  //Toggle the zone status attrib with button press
  zclSampleWarningDevice_ZoneStatus ^= SS_IAS_ZONE_STATUS_ALARM1_ALARMED;

  //get the appropiate sequence number
  Zstackapi_getZCLFrameCounterReq(appServiceTaskId, &Rsp);

  //generates a Zone Status Change Notification Command
  zclSS_IAS_Send_ZoneStatusChangeNotificationCmd(SAMPLEWARNINGDEVICE_ENDPOINT,
                                                 &zclSampleWarningDevice_DstAddr,
                                                 zclSampleWarningDevice_ZoneStatus, extendedStatus, zclSampleWarningDevice_ZoneId,delay,
                                                 disableDefaultRsp, Rsp.zclFrameCounter );


  if(zclSampleWarningDevice_ZoneStatus == SS_IAS_ZONE_STATUS_ALARM1_ALARMED)
  {
    LED_startBlinking(gRedLedHandle, 500, LED_BLINK_FOREVER);
  }
  else
  {
    LED_stopBlinking(gRedLedHandle);
    LED_setOff(gRedLedHandle);
  }
}

void zclSampleWarningDevice_UiActionSendEnroll(const int32_t _itemEntry)
{
    zclSampleWarningDevice_SendEnrollReqToCIE();
    zclSampleWarningDevice_UpdateStatusLine();
}

void zclSampleWarningDevice_UiActionToggleAlarm(const int32_t _itemEntry)
{
    zclSampleWarningDevice_SendChangeNotification();

    if(zclSampleWarningDevice_ZoneStatus == SS_IAS_ZONE_STATUS_ALARM1_ALARMED)
    {
      LED_startBlinking(gRedLedHandle, 500, LED_BLINK_FOREVER);
    }
    else
    {
      LED_stopBlinking(gRedLedHandle);
      LED_setOff(gRedLedHandle);
    }

    zclSampleWarningDevice_UpdateStatusLine();
}

/*********************************************************************
 * @fn          zclSampleWarningDevice_UiActionChangeEnrollmentMode
 *
 * @brief       Process UART input for device enrollment mode
 *
 * @param       _input - uart key or uart notification
 *              _pLines - pointers to the buffers to be displayed
 *              _pCurInfo - pointer to the cursor position. The position is given
 *                          by the length of _pLines and the number of lines.
 *                          If -1,-1 is provided, no pointer is displayed
 *
 * @return      none
 */
void zclSampleWarningDevice_UiActionChangeEnrollmentMode(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    // loop through enrollment modes
    if (_input == CUI_INPUT_UP)
    {
      zclSampleWarningDevice_enrollmentMode =(IAS_Zone_EnrollmentModes_t) \
      ((uint8_t)(zclSampleWarningDevice_enrollmentMode + 1) %
      (uint8_t)ENROLLMENT_MODE_ENUM_LENGTH);
    }
    else if(_input == CUI_INPUT_DOWN)
    {
      zclSampleWarningDevice_enrollmentMode = (IAS_Zone_EnrollmentModes_t) \
      ((uint8_t)(zclSampleWarningDevice_enrollmentMode - 1) %
      (uint8_t)ENROLLMENT_MODE_ENUM_LENGTH);
    }

    switch(zclSampleWarningDevice_enrollmentMode)
    {
      case TRIP_TO_PAIR:
      {
        strncpy(_pLines[1], "TRIP-TO-PAIR", MAX_MENU_LINE_LEN);
        break;
      }
      case AUTO_ENROLL_REQUEST:
      {
        strncpy(_pLines[1], "AUTO ENROLL REQ", MAX_MENU_LINE_LEN);
        break;
      }
      case ENROLLMENT_MODE_ENUM_LENGTH:
      default:
          break;
    }

    if (_input != CUI_ITEM_PREVIEW) {
        strncpy(_pLines[0], "Up/Down to select:", MAX_MENU_LINE_LEN);
        strncpy(_pLines[2], " ENROLLMENT MODE ", MAX_MENU_LINE_LEN);
    }
}

/*********************************************************************
 * @fn          zclSampleWarningDevice_UiActionDiscoverCIE
 *
 * @brief       Process UART input for manual CIE Discovery
 *
 * @param       _input - uart key or uart notification
 *              _pLines - pointers to the buffers to be displayed
 *              _pCurInfo - pointer to the cursor position. The position is given
 *                          by the length of _pLines and the number of lines.
 *                          If -1,-1 is provided, no pointer is displayed
 *
 * @return      none
 */
void zclSampleWarningDevice_UiActionDiscoverCIE(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
  static uint8_t discoveryEnabled = FALSE;

  if (_input == CUI_INPUT_UP || _input == CUI_INPUT_DOWN)
  {
    discoveryEnabled = !discoveryEnabled;
  }

  zstack_sysNwkInfoReadRsp_t  Rsp;
  zstack_getZCLFrameCounterRsp_t zclCounterRsp;
  afAddrType_t dstAddr;

  Zstackapi_getZCLFrameCounterReq(appServiceTaskId, &zclCounterRsp);

  //Get our short address
  Zstackapi_sysNwkInfoReadReq(appServiceTaskId, &Rsp);

  dstAddr.endPoint = SAMPLEWARNINGDEVICE_ENDPOINT;
  dstAddr.addrMode = afAddr16Bit;
  dstAddr.addr.shortAddr = Rsp.nwkAddr;

  switch(discoveryEnabled)
  {
    case TRUE:
    {
      // 0xFFFF = identify forever, until disabled
      zclGeneral_SendIdentify(SAMPLEWARNINGDEVICE_ENDPOINT, &dstAddr, 0xFFFF, TRUE, zclCounterRsp.zclFrameCounter);

      strncpy(_pLines[1], "ENABLED", MAX_MENU_LINE_LEN);
    }
    break;
    case FALSE:
    {
      // 0 = stop identifying
      zclGeneral_SendIdentify(SAMPLEWARNINGDEVICE_ENDPOINT, &dstAddr, 0, TRUE, zclCounterRsp.zclFrameCounter);

      strncpy(_pLines[1], "DISABLED", MAX_MENU_LINE_LEN);
    }
    break;
  }

  if (_input != CUI_ITEM_PREVIEW) {
      strncpy(_pLines[0], "Up/Down to select:", MAX_MENU_LINE_LEN);
      strncpy(_pLines[2], "  DISCOVER ZONE  ", MAX_MENU_LINE_LEN);
  }
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

#endif //ZCL_ZONE




