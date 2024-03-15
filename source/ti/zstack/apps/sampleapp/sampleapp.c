/**************************************************************************************************
  Filename:       sampleapp.c
  Revised:        $Date: 2019-08-05 15:52:00 -0700 (Mon, 5 Aug 2019) $
  Revision:       $Revision: Unknown $

  Description:    Zigbee Application Builder - Sample Device Application.

  Copyright 2006-2019 Texas Instruments Incorporated. All rights reserved.

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
  This application is a template to get started writing an application
  using the SysConfig Zigbee Application Builder.

  Look for the sections marked with "TODO" to add application
  specific code.
*********************************************************************/

/*********************************************************************
 * INCLUDES
 *********************************************************************/

#include "zcomdef.h"

#include "bdb_interface.h"
#include "zcl_port.h"

#include "nvintf.h"
#include <string.h>

#include "zstackmsg.h"
#include "zstackapi.h"
#include "cui.h"

#include "sampleapp.h"
#include "zcl_sample_app_def.h"
#include "zcl_sampleapps_ui.h"
#include "zcl_port.h"
#include "util_timer.h"
#include <ti/drivers/apps/Button.h>
#include "ti_drivers_config.h"

#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#include "zcl_config.h"

#if defined ( BDB_TL_INITIATOR )
#include "touchlink_initiator_app.h"
#elif defined ( BDB_TL_TARGET )
#include "touchlink_target_app.h"
#endif

/*********************************************************************
 * MACROS
 *********************************************************************/


/*********************************************************************
 * TYPEDEFS
 *********************************************************************/


/*********************************************************************
 * GLOBAL VARIABLES
 *********************************************************************/
byte sampleApp_TaskID;


/*********************************************************************
 * LOCAL VARIABLES
 *********************************************************************/
// Semaphore used to post events to the application thread
static Semaphore_Handle appSemHandle;
static Semaphore_Struct appSem;

/* App service ID used for messaging with stack service task */
uint8_t  sampleApp_serviceTaskId;
/* App service task events, set by the stack service task when sending a message */
static uint32_t appServiceTaskEvents;

#if ZG_BUILD_ENDDEVICE_TYPE
static ClockP_Handle EndDeviceRejoinClkHandle;
static ClockP_Struct EndDeviceRejoinClkStruct;
#endif

// Passed in function pointers to the NV driver
static NVINTF_nvFuncts_t *pfnZdlNV = NULL;

afAddrType_t sampleApp_DstAddr;

#ifndef CUI_DISABLE
CONST char zclSampleApp_appStr[] = "SampleApp";
CUI_clientHandle_t gCuiHandle;
#endif

#if defined(USE_DMM) && defined(BLE_START) || !defined(CUI_DISABLE)
static uint16_t zclSampleApp_BdbCommissioningModes;
#endif // defined(USE_DMM) && defined(BLE_START) || !defined(CUI_DISABLE)

/*********************************************************************
 * LOCAL FUNCTIONS
 *********************************************************************/
// Functions for sample application setup
void sampleApp_task(NVINTF_nvFuncts_t *pfnNV);
static void sampleApp_initialization(void);
static void sampleApp_process_loop(void);
static void sampleApp_initParameters(void);
static void sampleApp_processZStackMsgs(zstackmsg_genericReq_t *pMsg);
static void SetupZStackCallbacks(void);
static void sampleApp_processAfIncomingMsgInd(zstack_afIncomingMsgInd_t *pInMsg);
static void sampleApp_initializeClocks(void);
#if ZG_BUILD_ENDDEVICE_TYPE
static void sampleApp_processEndDeviceRejoinTimeoutCallback(UArg a0);
#endif
static void sampleApp_Init( void );
static void sampleApp_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg);

// Functions to process ZCL Foundation incoming Command/Response messages
static uint8_t sampleApp_ProcessIncomingMsg( zclIncoming_t *pInMsg );
#ifdef ZCL_READ
static uint8_t sampleApp_ProcessInReadRspCmd( zclIncoming_t *pInMsg );
#endif
#ifdef ZCL_WRITE
static uint8_t sampleApp_ProcessInWriteRspCmd( zclIncoming_t *pInMsg );
#endif
static uint8_t sampleApp_ProcessInDefaultRspCmd( zclIncoming_t *pInMsg );
#ifdef ZCL_DISCOVER
static uint8_t sampleApp_ProcessInDiscCmdsRspCmd( zclIncoming_t *pInMsg );
static uint8_t sampleApp_ProcessInDiscAttrsRspCmd( zclIncoming_t *pInMsg );
static uint8_t sampleApp_ProcessInDiscAttrsExtRspCmd( zclIncoming_t *pInMsg );
#endif

#ifndef CUI_DISABLE
static void zclSampleApp_processKey(uint8_t key, Button_EventMask buttonEvents);
#endif

#if defined (BDB_TL_TARGET) || defined (BDB_TL_INITIATOR)
// Touchlink BDB Finding and Binding callback function
static void tl_BDBFindingBindingCb(void);

tl_BDBFindingBindingCb_t tl_FindingBindingCb =
{
  tl_BDBFindingBindingCb
};
#endif // defined ( BDB_TL_TARGET ) || defined (BDB_TL_INITIATOR)

/******************************************************************************
 *
 *  Functions for sample application setup
 *
 *****************************************************************************/

/*******************************************************************************
 * @fn          sampleApp_task
 * @brief       Application task entry point for the Z-Stack
 *              Sample Application
 * @param       pfnNV - pointer to the NV functions
 *******************************************************************************/
void sampleApp_task(NVINTF_nvFuncts_t *pfnNV)
{
  // Save and register the function pointers to the NV drivers
  pfnZdlNV = pfnNV;
  zclport_registerNV(pfnZdlNV, ZCL_PORT_SCENE_TABLE_NV_ID);

  // Initialize application
  sampleApp_initialization();

  // No return from task process
  sampleApp_process_loop();
}

/*******************************************************************************
 * @fn          sampleApp_initialization
 * @brief       Initialize the application
 *******************************************************************************/
static void sampleApp_initialization(void)
{
    /* Initialize user clocks */
    sampleApp_initializeClocks();

    /* create semaphores for messages / events */
    Semaphore_Params semParam;
    Semaphore_Params_init(&semParam);
    semParam.mode = ti_sysbios_knl_Semaphore_Mode_COUNTING;
    Semaphore_construct(&appSem, 0, &semParam);
    appSemHandle = Semaphore_handle(&appSem);

    sampleApp_serviceTaskId = OsalPort_registerTask(Task_self(), appSemHandle, &appServiceTaskEvents);

    //Initialize stack
    sampleApp_Init();
}


/*********************************************************************
 * @fn          sampleApp_process_loop
 * @brief       Event Loop Processor.
 *********************************************************************/
static void sampleApp_process_loop( void )
{
    /* Forever loop */
    for(;;)
    {
        zstackmsg_genericReq_t *pMsg = NULL;

        /* Wait for response message */
        if(Semaphore_pend(appSemHandle, BIOS_WAIT_FOREVER ))
        {
            /* Retrieve the response message */
            if( (pMsg = (zstackmsg_genericReq_t*) OsalPort_msgReceive( sampleApp_serviceTaskId )) != NULL)
            {
                /* Process the message from the stack */
                sampleApp_processZStackMsgs(pMsg);

                // Free any separately allocated memory
                Zstackapi_freeIndMsg(pMsg);
            }

            if(pMsg != NULL)
            {
                OsalPort_msgDeallocate((uint8_t*)pMsg);
            }
            // TODO: Add Functionality to Event Loop Processor


#if ZG_BUILD_ENDDEVICE_TYPE
        if ( appServiceTaskEvents & SAMPLEAPP_END_DEVICE_REJOIN_EVT )
        {
          zstack_bdbRecoverNwkRsp_t zstack_bdbRecoverNwkRsp;

          Zstackapi_bdbRecoverNwkReq(sampleApp_serviceTaskId,&zstack_bdbRecoverNwkRsp);

          appServiceTaskEvents &= ~SAMPLEAPP_END_DEVICE_REJOIN_EVT;
        }
#endif

#ifndef CUI_DISABLE
            zclsampleApp_ui_event_loop();
#endif
#if defined (BDB_TL_TARGET) || defined (BDB_TL_INITIATOR)
        if(appServiceTaskEvents & TL_BDB_FB_EVT)
        {
            zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;
            zstack_bdbStartCommissioningReq.commissioning_mode = BDB_COMMISSIONING_MODE_FINDING_BINDING;
            Zstackapi_bdbStartCommissioningReq(sampleApp_serviceTaskId, &zstack_bdbStartCommissioningReq);
            appServiceTaskEvents &= ~TL_BDB_FB_EVT;
        }
#endif // defined ( BDB_TL_TARGET ) || defined (BDB_TL_INITIATOR)
      }
  }
}


/*********************************************************************
 * @fn          sampleApp_initParameters
 * @brief       Initialization function for the bdb attribute set
 *********************************************************************/
static void sampleApp_initParameters(void)
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

    Zstackapi_bdbSetAttributesReq(sampleApp_serviceTaskId, &zstack_bdbSetAttrReq);
}

#if defined (BDB_TL_TARGET) || defined (BDB_TL_INITIATOR)
static void tl_BDBFindingBindingCb(void)
{
  OsalPortTimers_startTimer(sampleApp_serviceTaskId, TL_BDB_FB_EVT, TL_BDB_FB_DELAY);
}
#endif // defined ( BDB_TL_TARGET ) || defined (BDB_TL_INITIATOR)

/*******************************************************************************
 * @fn      sampleApp_processZStackMsgs
 * @brief   Process event from Stack
 * @param   pMsg - pointer to incoming ZStack message to process
 * @return  void
 *******************************************************************************/
static void sampleApp_processZStackMsgs(zstackmsg_genericReq_t *pMsg)
{
    switch(pMsg->hdr.event)
    {
        case zstackmsg_CmdIDs_BDB_NOTIFICATION:
            {
                zstackmsg_bdbNotificationInd_t *pInd;
                pInd = (zstackmsg_bdbNotificationInd_t*)pMsg;
                sampleApp_ProcessCommissioningStatus(&(pInd->Req));
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
        case zstackmsg_CmdIDs_DEV_STATE_CHANGE_IND:
            {
            // The ZStack Thread is indicating a State change
#if defined(USE_DMM) && defined(BLE_START) || !defined(CUI_DISABLE)
                zstackmsg_devStateChangeInd_t *pInd = (zstackmsg_devStateChangeInd_t *)pMsg;
#endif // defined(USE_DMM) && defined(BLE_START) || !defined(CUI_DISABLE)

#ifndef CUI_DISABLE
                UI_DeviceStateUpdated(&(pInd->req));
                UI_UpdateBdbStatusLine(NULL);
#endif
            }
            break;

        case zstackmsg_CmdIDs_AF_INCOMING_MSG_IND:
            {
                // Process incoming data messages
                zstackmsg_afIncomingMsgInd_t *pInd;
                pInd = (zstackmsg_afIncomingMsgInd_t *)pMsg;
                sampleApp_processAfIncomingMsgInd( &(pInd->req) );
            }
            break;

        case zstackmsg_CmdIDs_DEV_PERMIT_JOIN_IND:
            {
              //  zstackmsg_devPermitJoinInd_t *pInd;
              //  pInd = (zstackmsg_devPermitJoinInd_t*)pMsg;
              //  uiProcessPermitJoin(&(pInd->Req));
            }
            break;


#if (ZG_BUILD_JOINING_TYPE)
        case zstackmsg_CmdIDs_BDB_CBKE_TC_LINK_KEY_EXCHANGE_IND:
        {
          zstack_bdbCBKETCLinkKeyExchangeAttemptReq_t zstack_bdbCBKETCLinkKeyExchangeAttemptReq;
          /* Z3.0 has not defined CBKE yet, so lets attempt default TC Link Key exchange procedure
           * by reporting CBKE failure. */

          zstack_bdbCBKETCLinkKeyExchangeAttemptReq.didSuccess = FALSE;

          Zstackapi_bdbCBKETCLinkKeyExchangeAttemptReq(sampleApp_serviceTaskId,
                                                       &zstack_bdbCBKETCLinkKeyExchangeAttemptReq);
        }
        break;

        case zstackmsg_CmdIDs_BDB_FILTER_NWK_DESCRIPTOR_IND:

         /*   User logic to remove networks that do not want to join
          *   Networks to be removed can be released with Zstackapi_bdbNwkDescFreeReq */

          Zstackapi_bdbFilterNwkDescComplete(sampleApp_serviceTaskId);
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

        /*
         * These are messages/indications from ZStack that this
         * application doesn't process.  These message can be
         * processed by your application, remove from this list and
         * process them here in this switch statement.
         */

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
 * @fn      SetupZStackCallbacks
 * @brief   Setup the Zstack Callbacks wanted
 *******************************************************************************/
static void SetupZStackCallbacks(void)
{
    zstack_devZDOCBReq_t zdoCBReq = {0};

    // Register for Callbacks, turn on:
    //  Device State Change,
    //  ZDO Match Descriptor Response,
    zdoCBReq.has_devStateChange = true;
    zdoCBReq.devStateChange = true;

    (void)Zstackapi_DevZDOCBReq(sampleApp_serviceTaskId, &zdoCBReq);
}


/*******************************************************************************
 * @fn          sampleApp_processAfIncomingMsgInd
 * @brief       Process AF Incoming Message Indication message
 * @param       pInMsg - pointer to incoming message
 *******************************************************************************/
static void sampleApp_processAfIncomingMsgInd(zstack_afIncomingMsgInd_t *pInMsg)
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
        OsalPort_memcpy(afMsg.srcAddr.addr.extAddr, &(pInMsg->srcAddr.addr.extAddr), Z_EXTADDR_LEN);
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


/*******************************************************************************
 * @fn      sampleApp_initializeClocks
 * @brief   Initialize Clocks
 *******************************************************************************/
static void sampleApp_initializeClocks(void)
{
#if ZG_BUILD_ENDDEVICE_TYPE
    // Initialize the timers needed for this application
    EndDeviceRejoinClkHandle = UtilTimer_construct(
    &EndDeviceRejoinClkStruct,
    sampleApp_processEndDeviceRejoinTimeoutCallback,
    SAMPLEAPP_END_DEVICE_REJOIN_DELAY,
    0, false, 0);
#endif

}


#if ZG_BUILD_ENDDEVICE_TYPE
/*******************************************************************************
 * @fn      sampleApp_processEndDeviceRejoinTimeoutCallback
 * @brief   Timeout handler function
 * @param   a0 - ignored
 *******************************************************************************/
static void sampleApp_processEndDeviceRejoinTimeoutCallback(UArg a0)
{
    (void)a0; // Parameter is not used

    appServiceTaskEvents |= SAMPLEAPP_END_DEVICE_REJOIN_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
}
#endif


/*********************************************************************
 * @fn          sampleApp_Init
 * @brief       Initialization function for the zclGeneral layer.
 *********************************************************************/
static void sampleApp_Init( void )
{

  // Set destination address to indirect
  sampleApp_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
  sampleApp_DstAddr.endPoint = 0;
  sampleApp_DstAddr.addr.shortAddr = 0;

  zclConfigInit();

  // Register the Application to receive the unprocessed Foundation command/response messages
  // User TODO: change SAMPLEAPP_ENDPOINT if default application name is modified in sysconfig
  // this will be defined in the generated file, zcl_config.h, in the "Simple Descriptor" section
  zclport_registerZclHandleExternal(SAMPLEAPP_ENDPOINT, sampleApp_ProcessIncomingMsg);

  //Write the bdb initialization parameters
  sampleApp_initParameters();

  //Setup ZDO callbacks
  SetupZStackCallbacks();

#if defined (BDB_TL_TARGET) || defined (BDB_TL_INITIATOR)
  touchLinkApp_registerFindingBindingCb(tl_FindingBindingCb);
#endif // defined ( BDB_TL_TARGET ) || (BDB_TL_INITIATOR)

  // Start BDB commissioning initialization
  zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;
  zstack_bdbStartCommissioningReq.commissioning_mode = BDB_COMMISSIONING_MODE_IDDLE;
  Zstackapi_bdbStartCommissioningReq(sampleApp_serviceTaskId, &zstack_bdbStartCommissioningReq);

#if !defined(CUI_DISABLE) || defined(USE_DMM) && defined(BLE_START)
  zclSampleApp_BdbCommissioningModes = DEFAULT_COMISSIONING_MODE;
#ifdef BDB_TL_INITIATOR
  zclSampleApp_BdbCommissioningModes |= BDB_COMMISSIONING_MODE_INITIATOR_TL;
#endif // BDB_TL_INITIATOR
#endif // defined(CUI_DISABLE) || defined(USE_DMM) && defined(BLE_START)

#ifndef CUI_DISABLE
  gCuiHandle = UI_Init( sampleApp_serviceTaskId,                     // Application Task ID
           &appServiceTaskEvents,                // The events processed by the sample application
           appSemHandle,                         // Semaphore to post the events in the application thread
           NULL,
           &zclSampleApp_BdbCommissioningModes,   // A pointer to the application's bdbCommissioningModes
           zclSampleApp_appStr,                   // A pointer to the app-specific name string
           zclSampleApp_processKey,               // A pointer to the app-specific key process function
           NULL          // A pointer to the app-specific NV Item reset function
           );
    //zclSampleApp_InitializeStatusLine(gCuiHandle);
#endif

#if defined ( BDB_TL_INITIATOR )
    touchLinkInitiatorApp_Init(sampleApp_serviceTaskId);
#elif defined ( BDB_TL_TARGET )
    touchLinkTargetApp_Init(sampleApp_serviceTaskId);
#endif
}


/*********************************************************************
 * @fn      sampleApp_ProcessCommissioningStatus
 * @brief   Callback in which the status of the commissioning process are reported
 * @param   bdbCommissioningModeMsg - Context message of the status of a commissioning process
 *******************************************************************************/
static void sampleApp_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg)
{
  switch(bdbCommissioningModeMsg->bdbCommissioningMode)
  {
    case BDB_COMMISSIONING_FORMATION:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;

        //After formation, perform nwk steering again plus the remaining commissioning modes that has not been process yet
        zstack_bdbStartCommissioningReq.commissioning_mode = BDB_COMMISSIONING_MODE_NWK_STEERING | bdbCommissioningModeMsg->bdbRemainingCommissioningModes;
        Zstackapi_bdbStartCommissioningReq(sampleApp_serviceTaskId,&zstack_bdbStartCommissioningReq);
      }
      break;

#if ZG_BUILD_ENDDEVICE_TYPE
    case BDB_COMMISSIONING_PARENT_LOST:
      if (!(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_NETWORK_RESTORED))
      {
        //Parent not found, attempt to rejoin again after a fixed delay
        UtilTimer_setTimeout( EndDeviceRejoinClkHandle, SAMPLEAPP_END_DEVICE_REJOIN_DELAY );
        UtilTimer_start(&EndDeviceRejoinClkStruct);
      }
      break;
#endif

    case BDB_COMMISSIONING_NWK_STEERING:
    case BDB_COMMISSIONING_FINDING_BINDING:
    case BDB_COMMISSIONING_INITIALIZATION:
      break;
  }

#ifndef CUI_DISABLE
  UI_UpdateBdbStatusLine(bdbCommissioningModeMsg);
#endif
}


/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/

/*********************************************************************
 * @fn      sampleApp_ProcessIncomingMsg
 * @brief   Process ZCL Foundation incoming message
 * @param   pInMsg - pointer to the received message
 *********************************************************************/
static uint8_t sampleApp_ProcessIncomingMsg( zclIncoming_t *pInMsg )
{
  uint8_t handled = FALSE;

  switch ( pInMsg->hdr.commandID )
  {
#ifdef ZCL_READ
    case ZCL_CMD_READ_RSP:
      sampleApp_ProcessInReadRspCmd( pInMsg );
      handled = TRUE;
      break;
#endif
#ifdef ZCL_WRITE
    case ZCL_CMD_WRITE_RSP:
      sampleApp_ProcessInWriteRspCmd( pInMsg );
      handled = TRUE;
      break;
#endif
    case ZCL_CMD_CONFIG_REPORT:
    case ZCL_CMD_CONFIG_REPORT_RSP:
    case ZCL_CMD_READ_REPORT_CFG:
    case ZCL_CMD_READ_REPORT_CFG_RSP:
    case ZCL_CMD_REPORT:
      break;

    case ZCL_CMD_DEFAULT_RSP:
      sampleApp_ProcessInDefaultRspCmd( pInMsg );
      handled = TRUE;
      break;
#ifdef ZCL_DISCOVER
    case ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP:
      sampleApp_ProcessInDiscCmdsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_CMDS_GEN_RSP:
      sampleApp_ProcessInDiscCmdsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_ATTRS_RSP:
      sampleApp_ProcessInDiscAttrsRspCmd( pInMsg );
      handled = TRUE;
      break;

    case ZCL_CMD_DISCOVER_ATTRS_EXT_RSP:
      sampleApp_ProcessInDiscAttrsExtRspCmd( pInMsg );
      handled = TRUE;
      break;
#endif
    default:
      break;
  }

  if ( pInMsg->attrCmd )
    MAP_osal_mem_free( pInMsg->attrCmd );

  return handled;
}


#ifdef ZCL_READ
/*********************************************************************
 * @fn      sampleApp_ProcessInReadRspCmd
 * @brief   Process the "Profile" Read Response Command
 * @param   pInMsg - incoming message to process
 *********************************************************************/
static uint8_t sampleApp_ProcessInReadRspCmd( zclIncoming_t *pInMsg )
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
 * @fn      sampleApp_ProcessInWriteRspCmd
 * @brief   Process the "Profile" Write Response Command
 * @param   pInMsg - incoming message to process
 *********************************************************************/
static uint8_t sampleApp_ProcessInWriteRspCmd( zclIncoming_t *pInMsg )
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
 * @fn      sampleApp_ProcessInDefaultRspCmd
 * @brief   Process the "Profile" Default Response Command
 * @param   pInMsg - incoming message to process
 *********************************************************************/
static uint8_t sampleApp_ProcessInDefaultRspCmd( zclIncoming_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;

  // Device is notified of the Default Response command.
  (void)pInMsg;

  return ( TRUE );
}


#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      sampleApp_ProcessInDiscCmdsRspCmd
 * @brief   Process the Discover Commands Response Command
 * @param   pInMsg - incoming message to process
 *********************************************************************/
static uint8_t sampleApp_ProcessInDiscCmdsRspCmd( zclIncoming_t *pInMsg )
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
 * @fn      sampleApp_ProcessInDiscAttrsRspCmd
 * @brief   Process the "Profile" Discover Attributes Response Command
 * @param   pInMsg - incoming message to process
 *********************************************************************/
static uint8_t sampleApp_ProcessInDiscAttrsRspCmd( zclIncoming_t *pInMsg )
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
 * @fn      sampleApp_ProcessInDiscAttrsExtRspCmd
 * @brief   Process the "Profile" Discover Attributes Extended Response Command
 * @param   pInMsg - incoming message to process
 *********************************************************************/
static uint8_t sampleApp_ProcessInDiscAttrsExtRspCmd( zclIncoming_t *pInMsg )
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
 * @fn      zclSampleApp_processKey
 *
 * @brief   Key event handler function
 *
 * @param   key - key to handle action for
 *          buttonEvents - event to handle action for
 *
 * @return  none
 */
static void zclSampleApp_processKey(uint8_t key, Button_EventMask buttonEvents)
{
    if (buttonEvents & Button_EV_CLICKED)
    {
        if(key == CONFIG_BTN_LEFT)
        {
            zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;

            zstack_bdbStartCommissioningReq.commissioning_mode = zclSampleApp_BdbCommissioningModes;
            Zstackapi_bdbStartCommissioningReq(sampleApp_serviceTaskId,&zstack_bdbStartCommissioningReq);
        }
        if(key == CONFIG_BTN_RIGHT)
        {

        }
    }
}
#endif // CUI_DISABLE
/****************************************************************************
****************************************************************************/

/****************************************************************************
 *
 * TODO: Add Additional Local Functions
 *
****************************************************************************/

