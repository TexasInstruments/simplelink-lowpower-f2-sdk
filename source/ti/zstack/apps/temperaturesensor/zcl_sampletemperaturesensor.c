/**************************************************************************************************
  Filename:       zcl_sampletemperaturesensor.c
  Revised:        $Date: 2014-10-24 16:04:46 -0700 (Fri, 24 Oct 2014) $
  Revision:       $Revision: 40796 $

  Description:    Zigbee Cluster Library - sample device application.


  Copyright 2013 Texas Instruments Incorporated.

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
  This application implements a ZigBee Temperature Sensor, based on Z-Stack 3.0.

  This application is based on the common sample-application user interface. Please see the main
  comment in zcl_sampleapp_ui.c. The rest of this comment describes only the content specific for
  this sample application.

  Application-specific UI peripherals being used:

  - LEDs:
    LED1 is not used in this application

  Application-specific menu system:
    <SET LOCAL TEMP> Set the temperature of the local temperature sensor
      Up/Down changes the temperature

      The APP Info line will display the following information:
        [Local Temperature]
          XXC - temperature of the local temperature sensor

*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "rom_jt_154.h"
#include "zcomdef.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_ms.h"
#include <string.h>

#include "zcl_sampletemperaturesensor.h"

#include "bdb_interface.h"
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
#define GUI_LOCAL_TEMP    1

#define APP_TITLE "   Temp Sensor  "

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


#ifdef BDB_REPORTING
#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 8
  uint8_t reportableChange[] = {0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 0x2C01 is 300 in int16_t
#endif
#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 4
  uint8_t reportableChange[] = {0x2C, 0x01, 0x00, 0x00}; // 0x2C01 is 300 in int16_t
#endif
#if BDBREPORTING_MAX_ANALOG_ATTR_SIZE == 2
  uint8_t reportableChange[] = {0x2C, 0x01}; // 0x2C01 is 300 in int16_t
#endif
#endif


// Semaphore used to post events to the application thread
static Semaphore_Handle appSemHandle;
static Semaphore_Struct appSem;

/* App service ID used for messaging with stack service task */
static uint8_t  appServiceTaskId;
/* App service task events, set by the stack service task when sending a message */
static uint32_t appServiceTaskEvents;
static endPointDesc_t  zclSampleTemperatureSensorEpDesc = {0};

#if ZG_BUILD_ENDDEVICE_TYPE
static ClockP_Handle EndDeviceRejoinClkHandle;
static ClockP_Struct EndDeviceRejoinClkStruct;
#endif

#ifndef CUI_DISABLE
static uint16_t zclSampleTemperatureSensor_BdbCommissioningModes;
#endif

// Passed in function pointers to the NV driver
static NVINTF_nvFuncts_t *pfnZdlNV = NULL;

#ifndef CUI_DISABLE
CONST char zclSampleTemperatureSensor_appStr[] = APP_TITLE_STR;
CUI_clientHandle_t gCuiHandle;
static uint32_t gSampleTemperatureSensorInfoLine;
#endif
/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void zclSampleTemperatureSensor_Init( void );
static void zclSampleTemperatureSensor_initialization(void);
static void zclSampleTemperatureSensor_process_loop(void);
static void zclSampleTemperatureSensor_initParameters(void);
static void zclSampleTemperatureSensor_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg);

static void zclSampleTemperatureSensor_initializeClocks(void);
static void zclSampleTemperatureSensor_processZStackMsgs(zstackmsg_genericReq_t *pMsg);

static void zclSampleTemperatureSensor_processAfIncomingMsgInd(zstack_afIncomingMsgInd_t *pInMsg);
#if ZG_BUILD_ENDDEVICE_TYPE
static void zclSampleTemperatureSensor_processEndDeviceRejoinTimeoutCallback(UArg a0);
#endif

#ifndef CUI_DISABLE
static void zclSampleTemperatureSensor_processKey(uint8_t key, Button_EventMask buttonEvents);
static void zclSampleTemperatureSensor_RemoveAppNvmData(void);
static void zclSampleTemperatureSensor_InitializeStatusLine(CUI_clientHandle_t gCuiHandle);
void zclSampleTemperatureSensor_UpdateStatusLine(void);
#endif

static void zclSampleTemperatureSensor_BasicResetCB( void );
static void zclSampleTemperatureSensor_ProcessCommissioningStatus(bdbCommissioningModeMsg_t* bdbCommissioningModeMsg);

// Functions to process ZCL Foundation incoming Command/Response messages
static uint8_t zclSampleTemperatureSensor_ProcessIncomingMsg( zclIncoming_t *msg );
#ifdef ZCL_READ
static uint8_t zclSampleTemperatureSensor_ProcessInReadRspCmd( zclIncoming_t *pInMsg );
#endif
#ifdef ZCL_WRITE
static uint8_t zclSampleTemperatureSensor_ProcessInWriteRspCmd( zclIncoming_t *pInMsg );
#endif
static uint8_t zclSampleTemperatureSensor_ProcessInDefaultRspCmd( zclIncoming_t *pInMsg );
#ifdef ZCL_DISCOVER
static uint8_t zclSampleTemperatureSensor_ProcessInDiscCmdsRspCmd( zclIncoming_t *pInMsg );
static uint8_t zclSampleTemperatureSensor_ProcessInDiscAttrsRspCmd( zclIncoming_t *pInMsg );
static uint8_t zclSampleTemperatureSensor_ProcessInDiscAttrsExtRspCmd( zclIncoming_t *pInMsg );
#endif // ZCL_DISCOVER


/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclSampleTemperatureSensor_CmdCallbacks =
{
  zclSampleTemperatureSensor_BasicResetCB,        // Basic Cluster Reset command
  NULL,                                           // Identfiy cmd
  NULL,                                           // Identify Query command
  NULL,                                           // Identify Query Response command
  NULL,                                           // Identify Trigger Effect command
#ifdef ZCL_ON_OFF
  NULL,             				                      // On/Off cluster command
  NULL,                                           // On/Off cluster enhanced command Off with Effect
  NULL,                                           // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                           // On/Off cluster enhanced command On with Timed Off
#endif
#ifdef ZCL_LEVEL_CTRL
  NULL,                                           // Level Control Move to Level command
  NULL,                                           // Level Control Move command
  NULL,                                           // Level Control Step command
  NULL,                                           // Level Control Stop command
  NULL,                                           // Level Control Move to Closest Frequency command
#endif
#ifdef ZCL_GROUPS
  NULL,                                           // Group Response commands
#endif
#ifdef ZCL_SCENES
  NULL,                                           // Scene Store Request command
  NULL,                                           // Scene Recall Request command
  NULL,                                           // Scene Response command
#endif
#ifdef ZCL_ALARMS
  NULL,                                           // Alarm (Response) commands
#endif
#ifdef SE_UK_EXT
  NULL,                                           // Get Event Log command
  NULL,                                           // Publish Event Log command
#endif
  NULL,                                           // RSSI Location command
  NULL                                            // RSSI Location Response command
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
  zclSampleTemperatureSensor_initialization();

  // No return from task process
  zclSampleTemperatureSensor_process_loop();
}



/*******************************************************************************
 * @fn          zclSampleTemperatureSensor_initialization
 *
 * @brief       Initialize the application
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleTemperatureSensor_initialization(void)
{
    /* Initialize user clocks */
    zclSampleTemperatureSensor_initializeClocks();

    /* create semaphores for messages / events
     */
    Semaphore_Params semParam;
    Semaphore_Params_init(&semParam);
    semParam.mode = ti_sysbios_knl_Semaphore_Mode_COUNTING;
    Semaphore_construct(&appSem, 0, &semParam);
    appSemHandle = Semaphore_handle(&appSem);

    appServiceTaskId = OsalPort_registerTask(Task_self(), appSemHandle, &appServiceTaskEvents);


    //Initialize stack
    zclSampleTemperatureSensor_Init();
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
 * @fn          zclSampleTemperatureSensor_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleTemperatureSensor_Init( void )
{
#ifdef BDB_REPORTING
      zstack_bdbRepAddAttrCfgRecordDefaultToListReq_t Req = {0};
#endif

  //Register Endpoint
  zclSampleTemperatureSensorEpDesc.endPoint = SAMPLETEMPERATURESENSOR_ENDPOINT;
  zclSampleTemperatureSensorEpDesc.simpleDesc = &zclSampleTemperatureSensor_SimpleDesc;
  zclport_registerEndpoint(appServiceTaskId, &zclSampleTemperatureSensorEpDesc);

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( SAMPLETEMPERATURESENSOR_ENDPOINT, &zclSampleTemperatureSensor_CmdCallbacks );

  // Register the application's attribute list and reset to default values
  zclSampleTemperatureSensor_ResetAttributesToDefaultValues();
  zcl_registerAttrList( SAMPLETEMPERATURESENSOR_ENDPOINT, zclSampleTemperatureSensor_NumAttributes, zclSampleTemperatureSensor_Attrs );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zclport_registerZclHandleExternal(SAMPLETEMPERATURESENSOR_ENDPOINT, zclSampleTemperatureSensor_ProcessIncomingMsg);

  //Write the bdb initialization parameters
  zclSampleTemperatureSensor_initParameters();

  //Setup ZDO callbacks
  SetupZStackCallbacks();

#if defined ( BDB_TL_INITIATOR )
  zclSampleTemperatureSensor_BdbCommissioningModes |= BDB_COMMISSIONING_MODE_INITIATOR_TL;
#endif

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
  gp_endpointInit(appServiceTaskId);
#endif

#ifdef BDB_REPORTING
  //Adds the default configuration values for the temperature attribute of the ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT cluster, for endpoint SAMPLETEMPERATURESENSOR_ENDPOINT
  //Default maxReportingInterval value is 10 seconds
  //Default minReportingInterval value is 3 seconds
  //Default reportChange value is 300 (3 degrees)
  Req.attrID = ATTRID_TEMPERATURE_MEASUREMENT_MEASURED_VALUE;
  Req.cluster = ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT;
  Req.endpoint = SAMPLETEMPERATURESENSOR_ENDPOINT;
  Req.maxReportInt = 10;
  Req.minReportInt = 3;
  OsalPort_memcpy(Req.reportableChange,reportableChange,BDBREPORTING_MAX_ANALOG_ATTR_SIZE);

  Zstackapi_bdbRepAddAttrCfgRecordDefaultToListReq(appServiceTaskId,&Req);
#endif

#ifndef CUI_DISABLE
  // set up default application BDB commissioning modes based on build type
  if(ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE)
  {
    zclSampleTemperatureSensor_BdbCommissioningModes = BDB_COMMISSIONING_MODE_NWK_FORMATION | BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_FINDING_BINDING;
  }
  else if (ZG_BUILD_JOINING_TYPE && ZG_DEVICE_JOINING_TYPE)
  {
    zclSampleTemperatureSensor_BdbCommissioningModes = BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_FINDING_BINDING;
  }


  gCuiHandle = UI_Init( appServiceTaskId,                     // Application Task ID
            &appServiceTaskEvents,                // The events processed by the sample application
            appSemHandle,                         // Semaphore to post the events in the application thread
            &zclSampleTemperatureSensor_IdentifyTime,
            &zclSampleTemperatureSensor_BdbCommissioningModes,   // A pointer to the application's bdbCommissioningModes
            zclSampleTemperatureSensor_appStr,                   // A pointer to the app-specific name string
            zclSampleTemperatureSensor_processKey,               // A pointer to the app-specific key process function
            zclSampleTemperatureSensor_RemoveAppNvmData          // A pointer to the app-specific NV Item reset function
            );

  //Request the Red LED for App
  LED_Params ledParams;
  LED_Params_init(&ledParams);
  LED_Handle gRedLedHandle = LED_open(CONFIG_LED_RED, &ledParams);
  (void) gRedLedHandle;


  //Initialize the SampleDoorLock UI status line
  zclSampleTemperatureSensor_InitializeStatusLine(gCuiHandle);
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

  // Call BDB initialization. Should be called once from application at startup to restore
  // previous network configuration, if applicable.
  zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;
  zstack_bdbStartCommissioningReq.commissioning_mode = 0;
  Zstackapi_bdbStartCommissioningReq(appServiceTaskId,&zstack_bdbStartCommissioningReq);
}

#ifndef CUI_DISABLE
/*********************************************************************
 * @fn          zclSampleTemperatureSensor_RemoveAppNvmData
 *
 * @brief       Callback when Application performs reset to Factory New Reset.
 *              Application must restore the application to default values
 *
 * @param       none
 *
 * @return      none
 */
static void zclSampleTemperatureSensor_RemoveAppNvmData(void)
{

}
#endif

static void zclSampleTemperatureSensor_initParameters(void)
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
 * @fn      zclSampleTemperatureSensor_initializeClocks
 *
 * @brief   Initialize Clocks
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleTemperatureSensor_initializeClocks(void)
{
#if ZG_BUILD_ENDDEVICE_TYPE
    // Initialize the timers needed for this application
    EndDeviceRejoinClkHandle = UtilTimer_construct(
    &EndDeviceRejoinClkStruct,
    zclSampleTemperatureSensor_processEndDeviceRejoinTimeoutCallback,
    SAMPLEAPP_END_DEVICE_REJOIN_DELAY,
    0, false, 0);
#endif
}

#if ZG_BUILD_ENDDEVICE_TYPE
/*******************************************************************************
 * @fn      zclSampleTemperatureSensor_processEndDeviceRejoinTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleTemperatureSensor_processEndDeviceRejoinTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    appServiceTaskEvents |= SAMPLEAPP_END_DEVICE_REJOIN_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}
#endif


/*******************************************************************************
 * @fn      zclSampleTemperatureSensor_process_loop
 *
 * @brief   Application task processing start.
 *
 * @param   none
 *
 * @return  void
 */
static void zclSampleTemperatureSensor_process_loop(void)
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
                zclSampleTemperatureSensor_processZStackMsgs(pMsg);
                // Free any separately allocated memory
                msgProcessed = Zstackapi_freeIndMsg(pMsg);
            }

            if((msgProcessed == FALSE) && (pMsg != NULL))
            {
                OsalPort_msgDeallocate((uint8_t*)pMsg);
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

#ifndef CUI_DISABLE
            //Process the events that the UI may have
            zclsampleApp_ui_event_loop();
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
  zclSampleTemperatureSensor_UpdateStatusLine();
#endif

        }
    }
}




/*******************************************************************************
 * @fn      zclSampleTemperatureSensor_processZStackMsgs
 *
 * @brief   Process event from Stack
 *
 * @param   pMsg - pointer to incoming ZStack message to process
 *
 * @return  void
 */
static void zclSampleTemperatureSensor_processZStackMsgs(zstackmsg_genericReq_t *pMsg)
{
      switch(pMsg->hdr.event)
      {
          case zstackmsg_CmdIDs_BDB_NOTIFICATION:
              {
                  zstackmsg_bdbNotificationInd_t *pInd;
                  pInd = (zstackmsg_bdbNotificationInd_t*)pMsg;
                  zclSampleTemperatureSensor_ProcessCommissioningStatus(&(pInd->Req));
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
                  zclSampleTemperatureSensor_processAfIncomingMsgInd( &(pInd->req) );
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
 * @fn          zclSampleTemperatureSensor_processAfIncomingMsgInd
 *
 * @brief       Process AF Incoming Message Indication message
 *
 * @param       pInMsg - pointer to incoming message
 *
 * @return      none
 *
 */
static void zclSampleTemperatureSensor_processAfIncomingMsgInd(zstack_afIncomingMsgInd_t *pInMsg)
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
 * @fn      zclSampleTemperatureSensor_ProcessCommissioningStatus
 *
 * @brief   Callback in which the status of the commissioning process are reported
 *
 * @param   bdbCommissioningModeMsg - Context message of the status of a commissioning process
 *
 * @return  none
 */
static void zclSampleTemperatureSensor_ProcessCommissioningStatus(bdbCommissioningModeMsg_t* bdbCommissioningModeMsg)
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
 * @fn      zclSampleTemperatureSensor_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to default values.
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleTemperatureSensor_BasicResetCB( void )
{
  zclSampleTemperatureSensor_ResetAttributesToDefaultValues();
#ifndef CUI_DISABLE
  zclSampleTemperatureSensor_UpdateStatusLine();
#endif
}


/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/


/*********************************************************************
 * @fn      zclSampleTemperatureSensor_ProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  uint8_t - TRUE if got handled
 */
static uint8_t zclSampleTemperatureSensor_ProcessIncomingMsg( zclIncoming_t *pInMsg)
{
  uint8_t handled = FALSE;
  switch ( pInMsg->hdr.commandID )
  {
#ifdef ZCL_READ
    case ZCL_CMD_READ_RSP:
      zclSampleTemperatureSensor_ProcessInReadRspCmd( pInMsg );
      handled = TRUE;
      break;
#endif
#ifdef ZCL_WRITE
    case ZCL_CMD_WRITE_RSP:
      zclSampleTemperatureSensor_ProcessInWriteRspCmd( pInMsg );
      handled = TRUE;
      break;
#endif
#ifdef ZCL_REPORT
    // See ZCL Test Applicaiton (zcl_testapp.c) for sample code on Attribute Reporting
    case ZCL_CMD_CONFIG_REPORT:
      //zclSampleTemperatureSensor_ProcessInConfigReportCmd( pInMsg );
      break;
      case ZCL_CMD_READ_REPORT_CFG:
      //zclSampleTemperatureSensor_ProcessInReadReportCfgCmd( pInMsg );
      break;
    case ZCL_CMD_CONFIG_REPORT_RSP:
      //zclSampleTemperatureSensor_ProcessInConfigReportRspCmd( pInMsg );
      break;
    case ZCL_CMD_READ_REPORT_CFG_RSP:
      //zclSampleTemperatureSensor_ProcessInReadReportCfgRspCmd( pInMsg );
      break;

    case ZCL_CMD_REPORT:
      //zclSampleTemperatureSensor_ProcessInReportCmd( pInMsg );
      break;
#endif
    case ZCL_CMD_DEFAULT_RSP:
      zclSampleTemperatureSensor_ProcessInDefaultRspCmd( pInMsg );
      handled = TRUE;
      break;
#ifdef ZCL_DISCOVER
    case ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP:
      zclSampleTemperatureSensor_ProcessInDiscCmdsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_CMDS_GEN_RSP:
      zclSampleTemperatureSensor_ProcessInDiscCmdsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_ATTRS_RSP:
      zclSampleTemperatureSensor_ProcessInDiscAttrsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_ATTRS_EXT_RSP:
      zclSampleTemperatureSensor_ProcessInDiscAttrsExtRspCmd( pInMsg );
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
 * @fn      zclSampleTemperatureSensor_ProcessInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleTemperatureSensor_ProcessInReadRspCmd( zclIncoming_t *pInMsg )
{
  zclReadRspCmd_t *readRspCmd;
  uint8_t i;

  readRspCmd = (zclReadRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < readRspCmd->numAttr; i++ )
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
 * @fn      zclSampleTemperatureSensor_ProcessInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleTemperatureSensor_ProcessInWriteRspCmd( zclIncoming_t *pInMsg )
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
 * @fn      zclSampleTemperatureSensor_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleTemperatureSensor_ProcessInDefaultRspCmd( zclIncoming_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;

  // Device is notified of the Default Response command.
  (void)pInMsg;

  return ( TRUE );
}

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      zclSampleTemperatureSensor_ProcessInDiscCmdsRspCmd
 *
 * @brief   Process the Discover Commands Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleTemperatureSensor_ProcessInDiscCmdsRspCmd( zclIncoming_t *pInMsg )
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
 * @fn      zclSampleTemperatureSensor_ProcessInDiscAttrsRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleTemperatureSensor_ProcessInDiscAttrsRspCmd( zclIncoming_t *pInMsg )
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
 * @fn      zclSampleTemperatureSensor_ProcessInDiscAttrsExtRspCmd
 *
 * @brief   Process the "Profile" Discover Attributes Extended Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8_t zclSampleTemperatureSensor_ProcessInDiscAttrsExtRspCmd( zclIncoming_t *pInMsg )
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

#ifndef CUI_DISABLE
/*********************************************************************
 * @fn      zclSampleTemperatureSensor_processKey
 *
 * @brief   Key event handler function
 *
 * @param   key - key to handle action for
 *          buttonEvents - event to handle action for
 *
 * @return  none
 */
static void zclSampleTemperatureSensor_processKey(uint8_t key, Button_EventMask buttonEvents)
{
    if (buttonEvents & Button_EV_CLICKED)
    {
        if(key == CONFIG_BTN_LEFT)
        {
            zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;

            zstack_bdbStartCommissioningReq.commissioning_mode = zclSampleTemperatureSensor_BdbCommissioningModes;
            Zstackapi_bdbStartCommissioningReq(appServiceTaskId,&zstack_bdbStartCommissioningReq);
        }
        if(key == CONFIG_BTN_RIGHT)
        {
            //Unused
        }
    }
}



void zclSampleTemperatureSensor_UiActionChangeTemp(const char _input, char* _lines[3], CUI_cursorInfo_t * _curInfo)
{
  char tempStr[3];
#ifdef BDB_REPORTING
  zstack_bdbRepChangedAttrValueReq_t Req;
#endif

  switch (_input)
  {
    case CUI_ITEM_INTERCEPT_START:
      break;
      case CUI_INPUT_UP:
        // increase the temperature
        if ( zclSampleTemperatureSensor_MeasuredValue < zclSampleTemperatureSensor_MaxMeasuredValue )
        {

          zclSampleTemperatureSensor_MeasuredValue = zclSampleTemperatureSensor_MeasuredValue + 100;  // considering using whole number value
#ifdef BDB_REPORTING
      Req.attrID = ATTRID_TEMPERATURE_MEASUREMENT_MEASURED_VALUE;
          Req.cluster = ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT;
          Req.endpoint = SAMPLETEMPERATURESENSOR_ENDPOINT;

          Zstackapi_bdbRepChangedAttrValueReq(appServiceTaskId,&Req);
#endif
        }
        else if ( zclSampleTemperatureSensor_MeasuredValue >= zclSampleTemperatureSensor_MaxMeasuredValue )
        {
          zclSampleTemperatureSensor_MeasuredValue = zclSampleTemperatureSensor_MaxMeasuredValue;
        }
      break;
      case CUI_INPUT_DOWN:
        // decrease the temperature
        if ( zclSampleTemperatureSensor_MeasuredValue > zclSampleTemperatureSensor_MinMeasuredValue )
        {
          zclSampleTemperatureSensor_MeasuredValue = zclSampleTemperatureSensor_MeasuredValue - 100;  // considering using whole number value
#ifdef BDB_REPORTING
      Req.attrID = ATTRID_TEMPERATURE_MEASUREMENT_MEASURED_VALUE;
          Req.cluster = ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT;
          Req.endpoint = SAMPLETEMPERATURESENSOR_ENDPOINT;

          Zstackapi_bdbRepChangedAttrValueReq(appServiceTaskId,&Req);
#endif
        }
        else if ( zclSampleTemperatureSensor_MeasuredValue <= zclSampleTemperatureSensor_MinMeasuredValue )
        {
          zclSampleTemperatureSensor_MeasuredValue = zclSampleTemperatureSensor_MinMeasuredValue;
        }
      break;
  }

  Util_ltoa( ( zclSampleTemperatureSensor_MeasuredValue / 100 ), (void *)(tempStr), 10 );
  tempStr[2] = 0;

  strcpy(_lines[1], "TEMP: ");
  strcat(_lines[1],tempStr);
  strcat(_lines[1], " C");

  if (_input != CUI_ITEM_PREVIEW)
  {
    strcpy(_lines[2], " SET LOCAL TEMP ");
  }
}

static void zclSampleTemperatureSensor_InitializeStatusLine(CUI_clientHandle_t gCuiHandle)
{
    /* Request Async Line for Light application Info */
    CUI_statusLineResourceRequest(gCuiHandle, "   APP Info"CUI_DEBUG_MSG_START"1"CUI_DEBUG_MSG_END, false, &gSampleTemperatureSensorInfoLine);

    zclSampleTemperatureSensor_UpdateStatusLine();
}


void zclSampleTemperatureSensor_UpdateStatusLine(void)
{
  char lineFormat[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};

  strcpy(lineFormat, "["CUI_COLOR_YELLOW"Local Temperature"CUI_COLOR_RESET"] ");

  strcat(lineFormat, "%dC");

  CUI_statusLinePrintf(gCuiHandle, gSampleTemperatureSensorInfoLine, lineFormat, (zclSampleTemperatureSensor_MeasuredValue / 100));
}

#endif

/****************************************************************************
****************************************************************************/




