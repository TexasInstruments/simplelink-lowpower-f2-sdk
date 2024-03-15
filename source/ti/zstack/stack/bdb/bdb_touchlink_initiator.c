/**************************************************************************************************
  Filename:       bdb_touchlink_initiator.c
  Revised:        $Date: 2013-11-22 16:17:23 -0800 (Fri, 22 Nov 2013) $
  Revision:       $Revision: 36220 $

  Description:    Zigbee Cluster Library - Light Link Initiator.


  Copyright 2011-2013 Texas Instruments Incorporated.

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
 * INCLUDES
 */
#include "zcomdef.h"
#include "ti_zstack_config.h"
#include "rom_jt_154.h"
#include "osal_nv.h"
#include "af.h"
#include "zd_app.h"
#include "zd_sec_mgr.h"
#include "zd_object.h"
#include "nwk_util.h"
#include "zglobals.h"
#include "addr_mgr.h"

#include "stub_aps.h"

#include "zcl.h"
#include "zcl_general.h"
#include "bdb.h"
#include "bdb_tl_commissioning.h"
#include "bdb_touchlink.h"
#include "bdb_touchlink_initiator.h"

#if defined ( BDB_TL_INITIATOR )

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define TOUCHLINK_INITIATOR_REJOIN_TIMEOUT             2500 // 2.5 sec

#define DEV_INFO_INVALID_EP                            0xFE

#define TOUCHLINK_INITIATOR_NUM_SCAN_REQ_PRIMARY       8  // 5 times on 1st channel, plus once for each remianing primary channel
#define TOUCHLINK_INITIATOR_NUM_SCAN_REQ_EXTENDED      20 // (TOUCHLINK_NUM_SCAN_REQ_PRIMARY + sizeof(TOUCHLINK_SECONDARY_CHANNELS_SET))

/*********************************************************************
 * TYPEDEFS
 */
typedef union
{
  bdbTLNwkStartRsp_t nwkStartRsp;
  bdbTLNwkJoinRsp_t nwkJoinRsp;
} bdbTLRsp_t;

typedef struct
{
  bdbTLScanRsp_t scanRsp;
  afAddrType_t srcAddr;
  uint16_t newNwkAddr;
  uint8_t rxChannel;  // channel scan response was heard on
  int8_t lastRssi;    // receieved RSSI
} targetCandidate_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8_t touchLinkInitiator_TaskID;
uint16_t selectedTargetNwkAddr;
ZLongAddr_t selectedTargetIEEEAddr;

/*********************************************************************
 * EXTERNAL VARIABLES
 */
extern devStartModes_t devStartMode;
extern uint8_t _tmpRejoinState;

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
extern void bdb_setNodeIsOnANetwork(bool isOnANetwork);

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8_t initiatorSeqNum;

// Touch Link channel tracking
static uint8_t numScanReqSent;
static int8_t  savedTXPower;
static uint8_t scanReqChannels;

// Network key sent to the target to start the network with
static uint8_t keyIndexSent;
static uint8_t encKeySent[SEC_KEY_LEN];
static uint32_t responseIDSent;

// Info related to the received request
#if ZSTACK_END_DEVICE_BUILD
static bdbTLNwkJoinReq_t joinReq;
#endif

// Info related to the received response
static targetCandidate_t selectedTarget;
static bdbTLRsp_t rxRsp; // network start or join response

static bdbTLNwkParams_t initiatorNwkParams = {0};

// Addresses used for sending/receiving messages
static afAddrType_t bcastAddr;

static uint8_t savedRxOnIdle;

// Application callback
static touchLink_NotifyAppTLCB_t pfnNotifyAppCB = NULL;
static touchLink_SelectDiscDevCB_t pfnSelectDiscDevCB = NULL;

static uint8_t initiatorRegisteredMsgAppTaskID = OsalPort_TASK_NO_TASK;

/*********************************************************************
 * GLOBAL FUNCTIONS
 */
extern void touchLink_ItemInit( uint16_t id, uint16_t len, void *pBuf );

/*********************************************************************
 * LOCAL FUNCTIONS
 */
#if ( ZSTACK_ROUTER_BUILD )
static void initiatorJoinNwk( void );
static void initiatorReJoinNwk( devStartModes_t startMode );
#endif
static void initiatorSendScanReq( bool freshScan );
#if (ZSTACK_END_DEVICE_BUILD)
static ZStatus_t initiatorSendNwkStartReq( bdbTLScanRsp_t *pRsp );
#endif
static ZStatus_t initiatorSendNwkJoinReq( bdbTLScanRsp_t *pRsp );
static ZStatus_t initiatorSendNwkUpdateReq( bdbTLScanRsp_t *pRsp );

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      touchLinkInitiator_StartDevDisc
 *
 * @brief   Start device discovery, scanning for other devices in the vicinity
 *          of the originator (initiating first part of the Touch-Link process).
 *          Device discovery shall only be initiated by address assignment capable
 *          devices. To perform device discovery, the initiator shall broadcast
 *          inter-PAN Scan Requests, spaced at an interval of
 *          BDBCTL_SCAN_TIME_BASE_DURATION seconds.
 *
 * @param   none
 *
 * @return  status
 */
ZStatus_t touchLinkInitiator_StartDevDisc( void )
{
  OsalPort_clearEvent( ZDAppTaskID, ZDO_NETWORK_INIT ); // in case orphaned rejoin was called

  //abort any touchlink in progress and start the new dev discovery.
  touchLinkInitiator_AbortTL();

  // To perform device discovery, switch to channel 11 and broadcast five
  // consecutive inter-PAN Scan Requests. Then switch to each remaining
  // TOUCHLINK channels in turn (i.e., 15, 20, and 25) and broadcast a single
  // inter-PAN Scan Request on each channel.
  if ( !OsalPortTimers_getTimerTimeout( touchLinkInitiator_TaskID, TOUCHLINK_TL_SCAN_BASE_EVT ) )
  {
    uint8_t rxOnIdle = TRUE;

    // Generate a new Transaction Id
    touchLinkTransID = ( ( (uint32_t)OsalPort_rand() ) << 16 ) + OsalPort_rand();
    OsalPortTimers_startTimer( touchLinkInitiator_TaskID, TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT,
                        BDBCTL_INTER_PAN_TRANS_ID_LIFETIME );

#if (ZG_BUILD_ENDDEVICE_TYPE)
    if(ZG_DEVICE_ENDDEVICE_TYPE)
    {
      // Remember current rx state
      ZMacGetReq( ZMacRxOnIdle, &savedRxOnIdle );

      // Turn off polling during touch-link procedure
      nwk_SetCurrentPollRateType(POLL_RATE_DISABLED,TRUE);
    }
#endif

    // MAC receiver should be on during touch-link procedure
    ZMacSetReq( ZMacRxOnIdle, &rxOnIdle );
    zgAllowRadioRxOff = FALSE; // prevent radio to be turned off

    scanReqChannels = TOUCHLINK_SCAN_PRIMARY_CHANNELS;
    numScanReqSent = 0;

    // Send out the first Scan Request
    initiatorSendScanReq( TRUE );

    return ( ZSuccess );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn      bdbTL_Send_IndentifyReq
 *
 * @brief   Call to send out a scan request for factory new procedure
 *
 * @param
 *
 * @return
 */
void touchLinkInitiator_ResetToFNProcedure( void )
{
  if((bdbCommissioningProcedureState.bdbCommissioningState == 0) ||
     (OsalPortTimers_getTimerTimeout( bdb_TaskID,BDB_CHANGE_COMMISSIONING_STATE) == 0))
  {
    touchlinkFNReset = TRUE;
    touchLinkInitiator_StartDevDisc( );
  }
}

/*********************************************************************
 * @fn      touchLinkInitiator_AbortTL
 *
 * @brief   Abort Touch-link device discovery.
 *          Successful execution could be done before Network Start/Join
 *          commands are sent. Until then, since no device parameters
 *          such as network settings are altered, the Touch-Link is
 *          still reversible.
 *
 * @param   none
 *
 * @return  status
 */
ZStatus_t touchLinkInitiator_AbortTL( void )
{
  if((OsalPortTimers_stopTimer(touchLinkInitiator_TaskID, TOUCHLINK_TL_SCAN_BASE_EVT) == SUCCESS)     ||
     (OsalPortTimers_stopTimer(touchLinkInitiator_TaskID, TOUCHLINK_CFG_TARGET_EVT) == SUCCESS)       ||
     (OsalPortTimers_stopTimer(touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_START_RSP_EVT) == SUCCESS) ||
     (OsalPortTimers_stopTimer(touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_JOIN_RSP_EVT) == SUCCESS))
  {
    initiatorSetNwkToInitState();
    touchLinkTransID = 0;
    numScanReqSent = 0;
    initiatorClearSelectedTarget();
    selectedTargetNwkAddr = 0;

    if( savedTXPower != 0 )
    {
      // restore previous TX power prior to scan requests
      ZMacSetTransmitPower( (ZMacTransmitPower_t) savedTXPower);
    }

    return ( ZSuccess );
  }

  return ( ZFailure );
}

/*********************************************************************
 * @fn          touchLinkInitiator_Init
 *
 * @brief       Initialization function for the TOUCHLINK Initiator task.
 *
 * @param       task_id - TOUCHLINK Initiator task id
 *
 * @return      none
 */
void touchLinkInitiator_Init( uint8_t task_id )
{
  // Save our own Task ID
  touchLinkTaskId = task_id;
  touchLinkInitiator_TaskID = touchLinkTaskId;

  // Build a broadcast address for the Scan Request
  bcastAddr.addrMode = afAddrBroadcast;
  bcastAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVALL;
  bcastAddr.panId = 0xFFFF;
  bcastAddr.endPoint = STUBAPS_INTER_PAN_EP;

  // Initialize TOUCHLINK common variables
  touchLink_InitVariables( TRUE );

  numScanReqSent = 0;
  initiatorClearSelectedTarget();
  scanReqChannels = TOUCHLINK_SCAN_PRIMARY_CHANNELS;

  initiatorSeqNum = 0;

#if (ZSTACK_ROUTER_BUILD)
  // Register to process ZDO messages
  ZDO_RegisterForZDOMsg( touchLinkInitiator_TaskID, Mgmt_Permit_Join_req );
  ZDO_RegisterForZDOMsg( touchLinkInitiator_TaskID, Device_annce );
#endif

}

/*********************************************************************
 * @fn      initiatorSelectNwkParams
 *
 * @brief   Select a unique PAN ID and Extended PAN ID when compared to
 *          the PAN IDs and Extended PAN IDs of the networks detected
 *          on the TOUCHLINK channels. The selected Extended PAN ID must be
 *          a random number (and not equal to our IEEE address).
 *
 * @param   void
 *
 * @return  void
 */
void initiatorSelectNwkParams( void )
{
  // Set our group ID range
  touchLink_PopGrpIDRange( touchLink_GetNumGrpIDs(), &touchLinkGrpIDsBegin, &touchLinkGrpIDsEnd );

  // Select a random Extended PAN ID
  touchLink_GenerateRandNum( _NIB.extendedPANID, Z_EXTADDR_LEN );

  // Select a random PAN ID
  _NIB.nwkPanId = OsalPort_rand();

  if ( _NIB.nwkLogicalChannel == 0 )
  {
    // Select randomly one of the TouchLink channels as our logical channel
    _NIB.nwkLogicalChannel = touchLink_GetRandPrimaryChannel( );
  }

  selectedTargetNwkAddr = 0;

  if ( devState != DEV_INIT )
  {
    // Let's assume we're the first initiator
    _NIB.nwkDevAddress = touchLink_PopNwkAddress();
  }

  // Configure MAC with our network parameters
  touchLink_SetMacNwkParams( _NIB.nwkDevAddress, _NIB.nwkPanId, _NIB.nwkLogicalChannel );
}

/*********************************************************************
 * @fn      touchLinkInitiator_RegisterForMsg
 *
 * @brief   Register application task to receive unprocessed messages
 *          received by the initiator endpoint.
 *
 * @param   taskId - task Id of the Application where commands will be sent to
 *
 * @return  ZSuccess if task registration successful
 *********************************************************************/
ZStatus_t touchLinkInitiator_RegisterForMsg( uint8_t taskId )
{
  if(initiatorRegisteredMsgAppTaskID == OsalPort_TASK_NO_TASK)
  {
    initiatorRegisteredMsgAppTaskID = taskId;
    return ( ZSuccess );
  }
  return ( ZFailure );
}

/*********************************************************************
 * @fn          touchLinkInitiator_event_loop
 *
 * @brief       Event Loop Processor for TOUCHLINK Initiator.
 *
 * @param       task_id - task id
 * @param       events - event bitmap
 *
 * @return      unprocessed events
 */
uint32_t touchLinkInitiator_event_loop( uint8_t task_id, uint32_t events )
{
  (void)task_id;  // Intentionally unreferenced parameter

  if(events & SYS_EVENT_MSG)
  {
    OsalPort_EventHdr *pMsg;
    ZStatus_t stat = ZFailure;

    if((pMsg = (OsalPort_EventHdr *)OsalPort_msgReceive(touchLinkInitiator_TaskID)) != NULL)
    {
      switch(pMsg->event)
      {
#if (ZSTACK_ROUTER_BUILD)
        case ZDO_CB_MSG:
          // ZDO sends the message that we registered for
          touchLink_RouterProcessZDOMsg((zdoIncomingMsg_t *)pMsg);
          stat = ZSuccess;
          break;
#endif

        default:
          break;
      }

      if(stat == ZSuccess)
      {
        // Release the OSAL message
        VOID OsalPort_msgDeallocate((uint8_t*)pMsg);
      }
      else
      {
        // forward to the application
        OsalPort_msgSend(initiatorRegisteredMsgAppTaskID, (uint8_t*)pMsg);
      }
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if(events & TOUCHLINK_TL_SCAN_BASE_EVT)
  {
    if(((scanReqChannels == TOUCHLINK_SCAN_PRIMARY_CHANNELS) && (numScanReqSent < TOUCHLINK_INITIATOR_NUM_SCAN_REQ_PRIMARY)) ||
       ((scanReqChannels == TOUCHLINK_SCAN_SECONDARY_CHANNELS) && (numScanReqSent < TOUCHLINK_INITIATOR_NUM_SCAN_REQ_EXTENDED)))
    {
      // Send another Scan Request on the next channel
      initiatorSendScanReq(FALSE);
    }
    else // Channels scan is complete
    {
      if(scanReqChannels == TOUCHLINK_SCAN_PRIMARY_CHANNELS)
      {
        // Extended scan is required, lets scan secondary channels
        scanReqChannels = TOUCHLINK_SCAN_SECONDARY_CHANNELS;

        // Send another Scan Request on the next channel
        initiatorSendScanReq( FALSE );
      }
      // See if we've received any Scan Responses back
      else if((selectedTarget.lastRssi != TOUCHLINK_WORST_RSSI) &&
              (selectedTarget.scanRsp.deviceInfo.endpoint != DEV_INFO_INVALID_EP))
      {
        // restore previous TX power prior to scan requests
        ZMacSetTransmitPower( (ZMacTransmitPower_t) savedTXPower);

        // Make sure the responder is not a factory new initiator if this device is also
        // factory new
        if ((selectedTarget.scanRsp.touchLinkInitiator == FALSE) ||
             (bdbAttributes.bdbNodeIsOnANetwork == TRUE))
        {
          bdbTLIdentifyReq_t req;

          // Tune to the channel that the Scan Response was heard on
          touchLink_SetChannel(selectedTarget.rxChannel);

          req.transID = selectedTarget.scanRsp.transID;
          req.IdDuration = BDB_TL_IDENTIFY_TIME;

          if(touchlinkFNReset == TRUE)
          {
            OsalPort_setEvent(touchLinkInitiator_TaskID, TOUCHLINK_CFG_TARGET_EVT);
            return (events ^ TOUCHLINK_TL_SCAN_BASE_EVT);
          }

          bdbTL_Send_IndentifyReq(TOUCHLINK_INTERNAL_ENDPOINT, &(selectedTarget.srcAddr), &req, initiatorSeqNum++);

#if ZSTACK_ROUTER_BUILD
          uint8_t i = 0;
          while (!GET_BIT(&(selectedTarget.scanRsp.keyBitmask), i))
          {
            i++;
          }

          initiatorNwkParams.keyIndex = i;
          zTouchLinkNwkStartRtr = TRUE;
          // Disabe other TouchLink events
          OsalPort_setEvent(touchLinkInitiator_TaskID, TOUCHLINK_DISABLE_RX_EVT);
#endif
          OsalPortTimers_startTimer(touchLinkInitiator_TaskID, TOUCHLINK_CFG_TARGET_EVT, TOUCHLINK_INITIATOR_IDENTIFY_INTERVAL);
        }
        // else wait for touch-link commands from the other initiator
      }
      else
      {
        // restore previous TX power prior to scan requests
        ZMacSetTransmitPower( (ZMacTransmitPower_t) savedTXPower);

        // We did not manage to select any target
        // Let's just go back to our initial configuration
        OsalPort_setEvent(touchLinkInitiator_TaskID, TOUCHLINK_DISABLE_RX_EVT);
        bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_TL_NO_SCAN_RESPONSE;
        bdb_reportCommissioningState( BDB_COMMISSIONING_STATE_TL, FALSE );
      }
    }

    // return unprocessed events
    return(events ^ TOUCHLINK_TL_SCAN_BASE_EVT);
  }

  if(events & TOUCHLINK_CFG_TARGET_EVT)
  {
    ZStatus_t status = ZFailure;

    bdbTLIdentifyReq_t req;

    req.transID = selectedTarget.scanRsp.transID;
    req.IdDuration = 0x00;
    uint8_t tcExtAddr[Z_EXTADDR_LEN];
    uint8_t DistributedSecurityNwkAddress[Z_EXTADDR_LEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    if(zTouchLinkNwkStartRtr == FALSE)
    {
      // Send an Identify stop Request
      bdbTL_Send_IndentifyReq( TOUCHLINK_INTERNAL_ENDPOINT, &(selectedTarget.srcAddr), &req, initiatorSeqNum++ );
    }

   // See if the target is part of our network
    if(!TOUCHLINK_SAME_NWK( selectedTarget.scanRsp.PANID, selectedTarget.scanRsp.extendedPANID))
    {
      // If the local device is not the trust center, always return TRUE
      status = APSME_GetRequest( apsTrustCenterAddress, 0, tcExtAddr );

      // Notify BDB state machine
      if(bdbAttributes.bdbNodeIsOnANetwork == TRUE)
      {
        if((status == ZSuccess) && (!osal_ExtAddrEqual(DistributedSecurityNwkAddress, tcExtAddr)))
        {
          bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_TL_NOT_PERMITTED;
          bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_TL, FALSE);
          return (events ^ TOUCHLINK_CFG_TARGET_EVT);
        }
      }

      // verify address ranges split possible if required
      if(!touchLink_IsValidSplitFreeRanges(selectedTarget.scanRsp.totalGrpIDs))
      {
        bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_TL_NOT_AA_CAPABLE;
        bdb_reportCommissioningState( BDB_COMMISSIONING_STATE_TL, FALSE);
        return(events ^ TOUCHLINK_CFG_TARGET_EVT);
      }

      if(touchlinkFNReset == TRUE)
      {
        touchlinkFNReset = FALSE;
        touchLinkInitiator_ResetToFNSelectedTarget();
        OsalPort_setEvent(touchLinkInitiator_TaskID, TOUCHLINK_DISABLE_RX_EVT);

        return(events ^ TOUCHLINK_CFG_TARGET_EVT);
      }

      if(bdbAttributes.bdbNodeIsOnANetwork == TRUE)
      {
        // Ask the target to join our network
        OsalPortTimers_startTimer( touchLinkInitiator_TaskID, TOUCHLINK_NWK_FORMATION_SUCCESS_EVT, 50);
        zTouchLinkNwkStartRtr = FALSE;
        return (events ^ TOUCHLINK_CFG_TARGET_EVT);
      }
#if ( ZSTACK_ROUTER_BUILD )
      // Tune to the channel that the Scan Response was heard on
      touchLink_SetChannel(selectedTarget.rxChannel);

      // Try to form a new network
      OsalPort_setEvent( touchLinkInitiator_TaskID, TOUCHLINK_NWK_RTR_START_EVT);
      return (events ^ TOUCHLINK_CFG_TARGET_EVT);
#elif ( ZSTACK_END_DEVICE_BUILD )
      if(selectedTarget.scanRsp.zLogicalType == ZG_DEVICETYPE_ROUTER)
      {
        if(bdbAttributes.bdbNodeIsOnANetwork == FALSE)
        {
          _NIB.nwkDevAddress = APL_FREE_NWK_ADDR_RANGE_BEGIN;
       }

        // Must be the first light; ask the light to start the network
        status = initiatorSendNwkStartReq( &selectedTarget.scanRsp);
      }
      else
      {
        // Notify the BDB state machine
        bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_NO_NETWORK;
        bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_TL, FALSE);
        return (events ^ TOUCHLINK_CFG_TARGET_EVT);
      }
#endif
    }
    else if(_NIB.nwkUpdateId != selectedTarget.scanRsp.nwkUpdateId)
    {
      // Set NWK task to run
      nwk_setStateIdle(TRUE);

      // Target is already part of our network
      if(selectedTarget.scanRsp.nwkUpdateId > _NIB.nwkUpdateId)
      {
        // Update our network update id and logical channel
        touchLink_ProcessNwkUpdate(selectedTarget.scanRsp.nwkUpdateId, selectedTarget.scanRsp.logicalChannel);

        // We're done here
        status = ZSuccess;
      }
      else if(selectedTarget.scanRsp.nwkUpdateId < _NIB.nwkUpdateId)
      {
        // Inform the target to update its network update id and logical channel
        initiatorSendNwkUpdateReq(&selectedTarget.scanRsp); // there's no corresponding response!

        // Notify the application about this device
        OsalPort_setEvent(touchLinkInitiator_TaskID, TOUCHLINK_NOTIFY_APP_EVT);
      }

      bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_SUCCESS;
      bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_TL, TRUE);
      return (events ^ TOUCHLINK_CFG_TARGET_EVT);
    }

    //we are touchlinking to a light in our network, just send application the device info
    else if(selectedTarget.scanRsp.touchLinkInitiator == FALSE)
    {
      epInfoRec_t rec;
      rec.nwkAddr = selectedTarget.scanRsp.nwkAddr;
      rec.endpoint = selectedTarget.scanRsp.deviceInfo.endpoint;
      rec.profileID = selectedTarget.scanRsp.deviceInfo.profileID;
      rec.deviceID = selectedTarget.scanRsp.deviceInfo.deviceID;
      rec.version = selectedTarget.scanRsp.deviceInfo.version;
      // Notify the application
      if(pfnNotifyAppCB != NULL)
      {
        (*pfnNotifyAppCB)( &rec );
      }
      // Clean selected target data
      initiatorClearSelectedTarget();
      pRespondentCurr = NULL;
      pRespondentNext = NULL;
      bdb_zclRespondentListClean(&pRespondentHead);
      bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_SUCCESS;
      bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_TL, TRUE);
    }

    // return unprocessed events
    return (events ^ TOUCHLINK_CFG_TARGET_EVT);
  }

  if (events & TOUCHLINK_W4_NWK_START_RSP_EVT)
  {
    bdbTLNwkStartRsp_t *pRsp = &(rxRsp.nwkStartRsp);

    // Look if we have a valid response
    if((pRsp->status == TOUCHLINK_NETWORK_START_RSP_STATUS_SUCCESS) &&
      (nwk_ExtPANIDValid(pRsp->extendedPANID) && touchLink_IsValidTransID(pRsp->transID)))
    {
      // Copy the new network parameters to NIB
      touchLink_SetNIB( ( ZSTACK_ROUTER_BUILD ? NWK_ROUTER : NWK_REJOINING ),
                  _NIB.nwkDevAddress, pRsp->extendedPANID,
                  pRsp->logicalChannel, pRsp->panId, pRsp->nwkUpdateId );

      // Apply the received network key
      touchLink_DecryptNwkKey(encKeySent, keyIndexSent, pRsp->transID, responseIDSent);

      // This is not a usual Trust Center protected network
      ZDSecMgrUpdateTCAddress(NULL);

      // Notify the application about this device
      OsalPort_setEvent(touchLinkInitiator_TaskID, TOUCHLINK_NOTIFY_APP_EVT);

      // Wait at least BDBCTL_MIN_STARTUP_DELAY_TIME seconds to allow the
      // target to start the network correctly. Join the target afterwards.
      OsalPortTimers_startTimer(touchLinkInitiator_TaskID, TOUCHLINK_START_NWK_EVT, BDBCTL_MIN_STARTUP_DELAY_TIME);
    }
    else
    {
      // Notify the BDB state machine
      bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_NO_NETWORK;
      bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_TL, FALSE);
    }

    // return unprocessed events
    return (events ^ TOUCHLINK_W4_NWK_START_RSP_EVT);
  }

  if (events & TOUCHLINK_START_NWK_EVT)
  {
    // Rejoins without NWK scan

    bdbTLNwkStartRsp_t *pRsp = &(rxRsp.nwkStartRsp);
    bdbTLNwkRejoin_t rejoinInf;

    rejoinInf.panId = pRsp->panId;
    rejoinInf.logicalChannel = pRsp->logicalChannel;
    OsalPort_memcpy(rejoinInf.extendedPANID, pRsp->extendedPANID, Z_EXTADDR_LEN);
    rejoinInf.nwkAddr = selectedTargetNwkAddr;
    rejoinInf.nwkUpdateId = pRsp->nwkUpdateId;

    touchLink_DevRejoin( &rejoinInf );

    // return unprocessed events
    return (events ^ TOUCHLINK_START_NWK_EVT);
  }

  if(events & TOUCHLINK_W4_NWK_JOIN_RSP_EVT)
  {
    bdbTLNwkJoinRsp_t *pRsp = &(rxRsp.nwkJoinRsp);

    if(pRsp->status == TOUCHLINK_NETWORK_JOIN_RSP_STATUS_SUCCESS)
    {
      // Wait at least BDBCTL_MIN_STARTUP_DELAY_TIME seconds to allow the
      // target to start operating on the network correctly. Notify the
      // application afterwards.
      OsalPortTimers_startTimer( touchLinkInitiator_TaskID, TOUCHLINK_NOTIFY_APP_EVT,
                          BDBCTL_MIN_STARTUP_DELAY_TIME );

      savedRxOnIdle = TRUE;
      // We're done with touch-link procedure here
      initiatorSetNwkToInitState();

      touchLink_UpdateNV( TOUCHLINK_UPDATE_NV_RANGES );

      if((POLL_RATE == 0) && (selectedTarget.scanRsp.zLogicalType == ZG_DEVICETYPE_ENDDEVICE))
      {
        //For RxOnIdle Devices, set the poll rate to 1sec
        nwk_SetCurrentPollRateType(POLL_RATE_TYPE_GENERIC_1_SEC, TRUE);
      }
      if(ZSTACK_ROUTER_BUILD)
      {
        linkInfo_t *linkInfo;

        // check if entry exists
        linkInfo = nwkNeighborGetLinkInfo( selectedTargetNwkAddr, _NIB.nwkPanId );

        // if not, look for a vacant entry to add this node...
        if ( linkInfo == NULL )
        {
          nwkNeighborAdd( selectedTargetNwkAddr, _NIB.nwkPanId, DEF_LQI );
          linkInfo = nwkNeighborGetLinkInfo( selectedTargetNwkAddr, _NIB.nwkPanId );
          linkInfo->txCost = DEF_LINK_COST;
          linkInfo->rxLqi = MIN_LQI_COST_1;
        }
      }
    }
    else
    {
      touchLink_SendLeaveReq();
      bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_TL_TARGET_FAILURE;
      bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_TL, FALSE);
    }

    // return unprocessed events
    return (events ^ TOUCHLINK_W4_NWK_JOIN_RSP_EVT);
  }

  if (events & TOUCHLINK_NWK_JOIN_IND_EVT)
  {
    // If not factory new, perform a Leave on our old network
    if((bdbAttributes.bdbNodeIsOnANetwork == TRUE) &&
       (touchLink_SendLeaveReq() == ZSuccess))
    {
      // Wait for Leave confirmation before joining the new network
      touchLinkLeaveInitiated = TOUCHLINK_LEAVE_TO_JOIN_NWK;
    }
    else
    {
#if ( ZSTACK_ROUTER_BUILD )
      // Notify our task to join this network
      OsalPort_setEvent( touchLinkInitiator_TaskID, TOUCHLINK_JOIN_NWK_ATTEMPT_EVT );
#else
      bdbTLNwkParams_t *pParams = &(joinReq.nwkParams);

      // Notify our task to join this network
      // Perform Network Discovery to verify our new network parameters uniqeness
      touchLink_PerformNetworkDisc((uint32_t)1 << pParams->logicalChannel);
#endif
    }

    // return unprocessed events
    return (events ^ TOUCHLINK_NWK_JOIN_IND_EVT);
  }

  if(events & TOUCHLINK_JOIN_NWK_ATTEMPT_EVT)
  {
    // Join the network
#if ( ZSTACK_ROUTER_BUILD )
    initiatorJoinNwk();
#else
    bdbTLNwkParams_t *pParams = &(joinReq.nwkParams);

    bdbTLNwkRejoin_t rejoinInf;

    rejoinInf.panId = pParams->panId;
    rejoinInf.logicalChannel = pParams->logicalChannel;
    OsalPort_memcpy( rejoinInf.extendedPANID, pParams->extendedPANID, Z_EXTADDR_LEN);
    rejoinInf.nwkAddr = discoveredTouchlinkNwk.chosenRouter;
    rejoinInf.nwkUpdateId = joinReq.nwkUpdateId;

    touchLink_DevRejoin(&rejoinInf);
#endif
    touchLink_FreeNwkParamList();

    // return unprocessed events
    return (events ^ TOUCHLINK_JOIN_NWK_ATTEMPT_EVT);
  }

  if (events & TOUCHLINK_DISABLE_RX_EVT)
  {
    // We're not asked to join a network
    initiatorSetNwkToInitState();

    scanReqChannels = TOUCHLINK_SCAN_PRIMARY_CHANNELS;
    numScanReqSent = 0;
    // Reset selected target
    if (zTouchLinkNwkStartRtr == FALSE)
    {
      initiatorClearSelectedTarget();
    }

    // return unprocessed events
    return (events ^ TOUCHLINK_DISABLE_RX_EVT);
  }

  if(events & TOUCHLINK_W4_REJOIN_EVT)
  {

    // return unprocessed events
    return(events ^ TOUCHLINK_W4_REJOIN_EVT);
  }

  if(events & TOUCHLINK_NOTIFY_APP_EVT)
  {
    ZDP_DeviceAnnce(NLME_GetShortAddr(), NLME_GetExtAddr(),
                    ZDO_Config_Node_Descriptor.CapabilityFlags, 0);

    if (selectedTarget.lastRssi > TOUCHLINK_WORST_RSSI)
    {
      epInfoRec_t rec;
      rec.nwkAddr = selectedTarget.newNwkAddr; // newly assigned network address
      rec.endpoint = selectedTarget.scanRsp.deviceInfo.endpoint;
      rec.profileID = selectedTarget.scanRsp.deviceInfo.profileID;
      rec.deviceID = selectedTarget.scanRsp.deviceInfo.deviceID;
      rec.version = selectedTarget.scanRsp.deviceInfo.version;
      // Notify the application
      if(pfnNotifyAppCB)
      {
        (*pfnNotifyAppCB)( &rec );
      }
    }
    // return unprocessed events
    return(events ^ TOUCHLINK_NOTIFY_APP_EVT);
  }

  if(events & TOUCHLINK_NWK_RTR_START_EVT)
  {
    _NIB.nwkDevAddress = INVALID_NODE_ADDR;
    nwkIB_t _NIBCpy;

    osal_nv_read(ZCD_NV_NIB, 0, sizeof(nwkIB_t), &_NIBCpy);
    _NIBCpy.nwkDevAddress = _NIB.nwkDevAddress;
    osal_nv_write(ZCD_NV_NIB, sizeof(nwkIB_t), &_NIBCpy);

    // If the PAN Id, Extended PAN Id or Logical Channel are zero then
    // determine each of these parameters
    if(!nwk_ExtPANIDValid( initiatorNwkParams.extendedPANID))
    {
      touchLink_GenerateRandNum(initiatorNwkParams.extendedPANID, Z_EXTADDR_LEN);
    }

    if(initiatorNwkParams.panId == 0)
    {
      initiatorNwkParams.panId = OsalPort_rand();
    }

    if(initiatorNwkParams.logicalChannel == 0)
    {
      // Select randomly one of the TouchLink channels as our logical channel
      initiatorNwkParams.logicalChannel = touchLink_GetRandPrimaryChannel();
    }

    if(selectedTarget.scanRsp.touchLinkAddressAssignment)
    {
      touchLink_GerFreeRanges(&initiatorNwkParams);
    }

    // Perform Network Discovery to verify our new network parameters uniqeness
    touchLink_PerformNetworkDisc((uint32_t)1 << initiatorNwkParams.logicalChannel);

    // return unprocessed events
    return (events ^ TOUCHLINK_NWK_RTR_START_EVT);
  }

  if (events & TOUCHLINK_NWK_FORMATION_SUCCESS_EVT)
  {
    if(bdbCommissioningProcedureState.bdbCommissioningState == BDB_COMMISSIONING_STATE_TL)
    {
      // Tune to the channel that the Scan Response was heard on
      touchLink_SetChannel(selectedTarget.rxChannel);

      if(_NIB.nwkUpdateId <= selectedTarget.scanRsp.nwkUpdateId)
      {
        NLME_SetUpdateID(selectedTarget.scanRsp.nwkUpdateId);
      }
      // Ask the target to join our network
      bdb_Initiator_SendNwkJoinReq( );
      zTouchLinkNwkStartRtr = FALSE;
      OsalPortTimers_startTimer(touchLinkInitiator_TaskID, TOUCHLINK_NWK_ANNOUNCE_EVT, 250);
    }
    return (events ^ TOUCHLINK_NWK_FORMATION_SUCCESS_EVT);
  }

  if(events & TOUCHLINK_NWK_DISC_CNF_EVT)
  {
#if ZSTACK_ROUTER_BUILD
    if(bdbAttributes.bdbNodeIsOnANetwork == FALSE)
    {
      initiatorNwkParams.nwkAddr = APL_FREE_NWK_ADDR_RANGE_BEGIN;
    }
    // Copy in the encrypted network key
    touchLink_EncryptNwkKey(initiatorNwkParams.nwkKey, initiatorNwkParams.keyIndex, touchLinkTransID, touchLinkResponseID);
    // Start operating on the new network
    touchLinkStartRtr(&initiatorNwkParams, touchLinkTransID);
#else
    // Join to the chosen network
    OsalPort_setEvent(touchLinkInitiator_TaskID, TOUCHLINK_JOIN_NWK_ATTEMPT_EVT);
#endif
    // return unprocessed events
    return(events ^ TOUCHLINK_NWK_DISC_CNF_EVT);
  }

  if(events & TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT)
  {
    touchLinkTransID = 0;
    initiatorClearSelectedTarget();
    initiatorSetNwkToInitState();

    // return unprocessed events
    return (events ^ TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT);
  }

  if(events & TOUCHLINK_NWK_ANNOUNCE_EVT)
  {
    ZDP_DeviceAnnce( NLME_GetShortAddr(), NLME_GetExtAddr(),
                     ZDO_Config_Node_Descriptor.CapabilityFlags, 0);
    bdb_reportCommissioningState(BDB_COMMISSIONING_STATE_TL, TRUE);
  }

  // If reach here, the events are unknown
  // Discard or make more handlers
  return 0;
}

/*********************************************************************
 * @fn      touchLinkInitiator_ChannelChange
 *
 * @brief   Change channel to supprot Frequency agility.
 *
 * @param   targetChannel - channel to
 *
 * @return  status
 */
ZStatus_t touchLinkInitiator_ChannelChange( uint8_t targetChannel )
{
    uint32_t channelMask;
    zAddrType_t dstAddr = {0};
    if(targetChannel < 11  || targetChannel > 26)
    {
      if(TOUCHLINK_PRIMARY_CHANNEL(_NIB.nwkLogicalChannel))
      {
        switch(_NIB.nwkLogicalChannel)
        {
        case TOUCHLINK_FIRST_CHANNEL:
          targetChannel = TOUCHLINK_SECOND_CHANNEL;
          break;
        case TOUCHLINK_SECOND_CHANNEL:
          targetChannel = TOUCHLINK_THIRD_CHANNEL;
          break;
        case TOUCHLINK_THIRD_CHANNEL:
          targetChannel = TOUCHLINK_FOURTH_CHANNEL;
          break;
        case TOUCHLINK_FOURTH_CHANNEL:
          targetChannel = TOUCHLINK_FIRST_CHANNEL;
        }
      }
      else
      {
        targetChannel = _NIB.nwkLogicalChannel + 1;
        if ( _NIB.nwkLogicalChannel > 26 )
          targetChannel = 11;
      }
    }

    dstAddr.addrMode = AddrBroadcast;
    dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVRXON;
    channelMask = (uint32_t)1 << targetChannel;

    // Increment the nwkUpdateId parameter and set the updateID in the beacon
    NLME_SetUpdateID(_NIB.nwkUpdateId + 1);
    ZDP_MgmtNwkUpdateReq(&dstAddr, channelMask, 0xfe, 0, _NIB.nwkUpdateId, 0);
    return ZSuccess;
}

/*********************************************************************
 * @fn      touchLinkSampleRemote_SendEPInfo
 *
 * @brief   Send Endpoint info command.
 *
 * @param   srcEP - source endpoint
 * @param   dstAddr - destination address
 * @param   seqNum - transaction sequnece number
 *
 * @return  ZStatus_t
 */
ZStatus_t touchLinkInitiator_SendEPInfo( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t seqNum)
{
    bdbTLEndpointInfo_t bdbTLEndpointInfoCmd;
    bdbTLDeviceInfo_t  bdbTLDeviceInfo;
      //send Epinfo cmd
    touchLink_GetSubDeviceInfo(0, &bdbTLDeviceInfo);
    bdbTLEndpointInfoCmd.endpoint = bdbTLDeviceInfo.endpoint;
    bdbTLEndpointInfoCmd.profileID = bdbTLDeviceInfo.profileID;
    bdbTLEndpointInfoCmd.deviceID = bdbTLDeviceInfo.deviceID;
    bdbTLEndpointInfoCmd.version = bdbTLDeviceInfo.version;

    osal_cpyExtAddr(bdbTLEndpointInfoCmd.ieeeAddr, NLME_GetExtAddr());
    bdbTLEndpointInfoCmd.nwkAddr = NLME_GetShortAddr();

    dstAddr->panId = _NIB.nwkPanId;
    return bdbTL_Send_EndpointInfo( srcEP, dstAddr,
                                    &bdbTLEndpointInfoCmd,
                                    0, seqNum );
}

/*********************************************************************
 * @fn      touchLinkInitiator_ResetToFNSelectedTarget
 *
 * @brief   Send Reset to Factory New Request command to the selected
 *          target of the current Touch-Link transaction.
 *          Note - this function should be called within no later than
 *          BDBCTL_INTER_PAN_TRANS_ID_LIFETIME ms from the Scan Request.
 *
 * @param   none
 *
 * @return  status - failure is returned due to invalid selected target or
 *          expired Touch-Link transaction.
 */
ZStatus_t touchLinkInitiator_ResetToFNSelectedTarget( void )
{
  bdbTLResetToFNReq_t req;
  req.transID = touchLinkTransID;

  // Cancel further touch-link commissioning (if called during identify interval)
  OsalPortTimers_stopTimer( touchLinkInitiator_TaskID, TOUCHLINK_CFG_TARGET_EVT );

  touchLink_SetChannel( selectedTarget.rxChannel );
  return bdbTL_Send_ResetToFNReq(TOUCHLINK_INTERNAL_ENDPOINT, &(selectedTarget.srcAddr), &req, initiatorSeqNum++);
}

/******************************************************************************
 * @fn      initiatorZdoLeaveCnfCB
 *
 * @brief   This callback is called to process a Leave Confirmation message.
 *
 *          Note: this callback function returns a pointer if it has handled
 *                the confirmation message and no further action should be
 *                taken with it. It returns NULL if it has not handled the
 *                confirmation message and normal processing should take place.
 *
 * @param   pParam - received message
 *
 * @return  Pointer if message processed. NULL, otherwise.
 */
void *initiatorZdoLeaveCnfCB( void *pParam )
{

  (void)pParam;

  // Did we initiate the leave?
  if(touchLinkLeaveInitiated == FALSE)
  {
    return(NULL);
  }

  if(touchLinkLeaveInitiated == TOUCHLINK_LEAVE_TO_JOIN_NWK)
  {
    // Notify our task to join the new network
    OsalPort_setEvent(touchLinkInitiator_TaskID, TOUCHLINK_JOIN_NWK_ATTEMPT_EVT);
  }

  return ( (void *)&touchLinkLeaveInitiated );
}

/*********************************************************************
 * @fn      touchLink_InitiatorSendScanRsp
 *
 * @brief   Send out a Scan Response command.
 *
 * @param   srcEP - sender's endpoint
 * @param   dstAddr - pointer to destination address struct
 * @param   transID - received transaction id
 * @param   seqNum - received sequence number
 *
 * @return  ZStatus_t
 */
ZStatus_t touchLink_InitiatorSendScanRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint32_t transID, uint8_t seqNum )
{
  ZStatus_t status = ZSuccess;

  // Make sure we respond only once during a Device Discovery
  if ( touchLinkLastAcceptedTransID != transID )
  {
    bdbTLScanRsp_t *pRsp;

    pRsp = (bdbTLScanRsp_t*)OsalPort_malloc(sizeof(bdbTLScanRsp_t));
    if ( pRsp )
    {
      memset(pRsp, 0, sizeof(bdbTLScanRsp_t));

      // Save transaction id
      touchLinkLastAcceptedTransID = transID;
      OsalPortTimers_startTimer( touchLinkInitiator_TaskID,
                              TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT,
                              BDBCTL_INTER_PAN_TRANS_ID_LIFETIME );

      pRsp->transID = transID;
      pRsp->rssiCorrection = TOUCHLINK_RSSI_CORRECTION;
      pRsp->zLogicalType = zgDeviceLogicalType;
      pRsp->touchLinkAddressAssignment = touchLink_IsValidSplitFreeRanges(0);
      pRsp->touchLinkInitiator = TRUE;
      pRsp->touchLinkProfileInterop = TRUE;

      if(ZDO_Config_Node_Descriptor.CapabilityFlags & CAPINFO_RCVR_ON_IDLE)
      {
        pRsp->zRxOnWhenIdle = TRUE;
      }

      pRsp->touchLinklinkPriority = FALSE;
      pRsp->keyBitmask = touchLink_GetNwkKeyBitmask();

      // Generate a new Response ID
      touchLinkResponseID = (((uint32_t)OsalPort_rand()) << 16) + OsalPort_rand();
      pRsp->responseID = touchLinkResponseID;

      pRsp->touchLinkFactoryNew = !bdbAttributes.bdbNodeIsOnANetwork;
      if ( pRsp->touchLinkFactoryNew )
      {
        pRsp->nwkAddr = 0xFFFF;
        pRsp->nwkUpdateId = 0;
      }
      else
      {
        pRsp->nwkAddr = _NIB.nwkDevAddress;
        pRsp->nwkUpdateId = _NIB.nwkUpdateId;
      }
      pRsp->PANID = _NIB.nwkPanId;
      pRsp->logicalChannel = _NIB.nwkLogicalChannel;
      osal_cpyExtAddr(pRsp->extendedPANID, _NIB.extendedPANID);

      pRsp->numSubDevices = touchLink_GetNumSubDevices(0);
      if(pRsp->numSubDevices == 1)
      {
        touchLink_GetSubDeviceInfo( 0, &(pRsp->deviceInfo));
      }

      pRsp->totalGrpIDs = touchLink_GetNumGrpIDs();

      // Send a response back
      status = bdbTL_Send_ScanRsp(srcEP, dstAddr, pRsp, seqNum);

      // If we're a factory new initiator and are in the middle of a Device
      // Discovery, stop the procedure and wait for subsequent configuration
      // information from the non-factory new initiator that we just responded to.
      if((status == ZSuccess) && (bdbAttributes.bdbNodeIsOnANetwork == FALSE))
      {
        OsalPortTimers_stopTimer(touchLinkInitiator_TaskID, TOUCHLINK_TL_SCAN_BASE_EVT);
      }

      OsalPort_free(pRsp);
    }
    else
    {
      status = ZMemError;
    }
  }

  return(status);
}

/*********************************************************************
 * @fn      bdb_Initiator_SendNwkJoinReq
 *
 * @brief   Send out a Network Join Router or End Device Request command.
 *          using the selected Target.
 *
 * @param   -
 *
 * @return  ZStatus_t
 */
ZStatus_t bdb_Initiator_SendNwkJoinReq( void )
{
  // Set NWK task to idle
  nwk_setStateIdle(TRUE);
  return initiatorSendNwkJoinReq(&(selectedTarget.scanRsp));
}

#if (ZSTACK_ROUTER_BUILD)
/*********************************************************************
 * @fn      touchLinkInitiator_PermitJoin
 *
 * @brief   Set the router permit join flag, to allow or deny classical
 *          commissioning by other ZigBee devices.
 *
 * @param   duration - enable up to aplcMaxPermitJoinDuration seconds,
 *                     0 to disable
 *
 * @return  status
 */
ZStatus_t touchLinkInitiator_PermitJoin( uint8_t duration )
{
  return touchLink_PermitJoin(duration);
}
#endif //(ZSTACK_ROUTER_BUILD)

/*********************************************************************
 * @fn      initiatorProcessGetScanBaseTime
 *
 * @brief   Get scan base time
 *
 * @param   stop - stops the timer if true
 *
 * @return  remaining time
 */
uint16_t initiatorProcessGetScanBaseTime(bool stop)
{
    uint16_t time;

    time = OsalPortTimers_getTimerTimeout(touchLinkInitiator_TaskID, TOUCHLINK_TL_SCAN_BASE_EVT);

    if(stop == TRUE)
    {
      OsalPortTimers_stopTimer(touchLinkInitiator_TaskID, TOUCHLINK_TL_SCAN_BASE_EVT);
      time = 0;
    }
    return time;
}

/*********************************************************************
 * @fn      initiatorProcessNwkJoinEvt
 *
 * @brief   Send network join event to touchlink event loop
 *
 * @param   bdbTLNwkStartReq_t - pointer to join request
 * @param   seqNum - sequence number
 *
 * @return  none
 */
void initiatorProcessNwkJoinEvt(afAddrType_t *pAddr, bdbTLNwkJoinReq_t *pNwkJoinReq, uint8_t seqNum)
{
    bdbTLNwkJoinRsp_t rsp;
    afAddrType_t dstAddr;
    nwk_states_t nwkState;

    rsp.transID = pNwkJoinReq->transID;

    if(nwk_ExtPANIDValid( pNwkJoinReq->nwkParams.extendedPANID))
      //NOTE: additional nwk params verification may be added here, e.g. ranges.
    {
 #if ( ZSTACK_ROUTER_BUILD )
      nwkState = NWK_ROUTER;
 #else
      // Save the request for later
      joinReq = *pNwkJoinReq;
      nwkState = NWK_ENDDEVICE;
 #endif

      // Notify our task to join the new network
      OsalPortTimers_startTimer(touchLinkInitiator_TaskID, TOUCHLINK_NWK_JOIN_IND_EVT, BDBCTL_MIN_STARTUP_DELAY_TIME);

      OsalPortTimers_stopTimer(touchLinkInitiator_TaskID, TOUCHLINK_DISABLE_RX_EVT);
      OsalPortTimers_stopTimer(touchLinkInitiator_TaskID, TOUCHLINK_CFG_TARGET_EVT);

      rsp.status = TOUCHLINK_NETWORK_JOIN_RSP_STATUS_SUCCESS;

      bdb_setNodeIsOnANetwork(TRUE);

      // Apply the received network key
      touchLink_DecryptNwkKey(pNwkJoinReq->nwkParams.nwkKey, pNwkJoinReq->nwkParams.keyIndex, pNwkJoinReq->transID, touchLinkResponseID);

      // This is not a usual Trust Center protected network
      ZDSecMgrUpdateTCAddress(NULL);

      // Configure MAC with our network parameters
      NLME_InitNV();
      touchLink_SetNIB( nwkState, pNwkJoinReq->nwkParams.nwkAddr, pNwkJoinReq->nwkParams.extendedPANID,
                   _NIB.nwkLogicalChannel, pNwkJoinReq->nwkParams.panId, pNwkJoinReq->nwkParams.panId );

      touchLink_SetMacNwkParams(_NIB.nwkDevAddress, _NIB.nwkPanId, _NIB.nwkLogicalChannel);
    }
    else
    {
      rsp.status = TOUCHLINK_NETWORK_JOIN_RSP_STATUS_FAILURE;
    }

    dstAddr = *pAddr;
    dstAddr.panId = 0xFFFF;

    // Send a response back
  #if ( ZSTACK_ROUTER_BUILD )
    bdbTL_Send_NwkJoinRtrRsp(TOUCHLINK_INTERNAL_ENDPOINT, &dstAddr, &rsp, seqNum);
  #else
    bdbTL_Send_NwkJoinEDRsp(TOUCHLINK_INTERNAL_ENDPOINT, &dstAddr, &rsp, seqNum);
  #endif
}

/*********************************************************************
 * @fn      initiatorProcessNwkUpdateEvt
 *
 * @brief   Send network update event to touchlink event loop
 *
 * @param   pNwkUpdateReq - pointer to update request
 *
 * @return  none
 */
void initiatorProcessNwkUpdateEvt(bdbTLNwkUpdateReq_t *pNwkUpdateReq )
{
  // Discard the request if the Extended PAN ID and PAN ID are not
  // identical with our corresponding stored values
  if(TOUCHLINK_SAME_NWK( pNwkUpdateReq->PANID, pNwkUpdateReq->extendedPANID))
  {
    uint8_t newUpdateId = touchLink_NewNwkUpdateId( pNwkUpdateReq->nwkUpdateId, _NIB.nwkUpdateId);
    if(_NIB.nwkUpdateId != newUpdateId)
    {
      // Update the network update id and logical channel
      touchLink_ProcessNwkUpdate(newUpdateId, pNwkUpdateReq->logicalChannel);
    }
  }
}

/*********************************************************************
 * @fn      initiatorProcessScanRsp
 *
 * @brief   Send scan response event to touchlink event loop
 *
 * @param   pDstAddr - pointer to address
 * @param   pRsp - pointer to scan response indication
 *
 * @return  none
 */
void initiatorProcessScanRsp(afAddrType_t *pDstAddr, bdbTLScanRsp_t *pRsp)
{
    if(OsalPortTimers_getTimerTimeout( touchLinkInitiator_TaskID, TOUCHLINK_TL_SCAN_BASE_EVT) == 0)
    {
        return;
    }

    bdbFindingBindingRespondent_t *pCurr;

    uint8_t selectThisTarget = FALSE;
    int8_t rssi = touchLink_GetMsgRssi();
    if ( pfnSelectDiscDevCB != NULL )
    {
        selectThisTarget = pfnSelectDiscDevCB( pRsp, rssi );
    }
    // Default selection - according to RSSI
    else if(rssi > TOUCHLINK_WORST_RSSI)
    {
        if((rssi + pRsp->rssiCorrection ) > selectedTarget.lastRssi)
        {
            // Better RSSI, select this target
            selectThisTarget = TRUE;
        }
    }

    if(selectThisTarget == TRUE)
    {
        selectedTarget.scanRsp = *pRsp;
        selectedTarget.lastRssi = rssi;
        selectedTarget.srcAddr = *pDstAddr;
        selectedTarget.srcAddr.panId = 0xFFFF;
        touchLinkResponseID = pRsp->responseID;
        touchLinkTransID = pRsp->transID;

        // Remember channel we heard back this scan response on
        ZMacGetReq(ZMacChannel, &(selectedTarget.rxChannel));

        if(pRsp->numSubDevices > 1)
        {
            selectedTarget.scanRsp.deviceInfo.endpoint = DEV_INFO_INVALID_EP;

            bdbTLDeviceInfoReq_t devInfoReq;
            devInfoReq.transID = pRsp->transID;
            devInfoReq.startIndex = 0;

            bdbTL_Send_DeviceInfoReq( TOUCHLINK_INTERNAL_ENDPOINT, pDstAddr,
                                             &devInfoReq, initiatorSeqNum++ );
            return;
        }

        // add new node to the list
        pCurr = bdb_AddRespondentNode(&pRespondentHead, NULL);
        if(pCurr != NULL)
        {
            pCurr->data.endPoint = pRsp->deviceInfo.endpoint;
            pCurr->data.panId = pDstAddr->panId;
        }
    }
}

/*********************************************************************
 * @fn      initiatorProcessDevInfoRsp
 *
 * @brief   Send device information response to touchlink event loop
 *
 * @param   pDstAddr - pointer to address
 * @param   pRsp - pointer to device information response indication
 *
 * @return  none
 */
void initiatorProcessDevInfoRsp(afAddrType_t *pDstAddr, bdbTLDeviceInfoRsp_t *pRsp)
{
    bdbFindingBindingRespondent_t *pCurr;

    if(OsalPort_memcmp(pDstAddr->addr.extAddr, selectedTarget.srcAddr.addr.extAddr, Z_EXTADDR_LEN))
    {
      uint8_t i;
      uint8_t selectedIdx = pRsp->cnt;
      bdbTLScanRsp_t tmpInfo = selectedTarget.scanRsp;

      for (i = 0; i < pRsp->cnt; ++i)
      {
        if (pfnSelectDiscDevCB != NULL)
        {
          tmpInfo.deviceInfo = pRsp->devInfoRec[i].deviceInfo;
          if(pfnSelectDiscDevCB( &tmpInfo, selectedTarget.lastRssi))
          {
            selectedIdx = i;
            // no break here to allow cycling through all sub-devices
          }
        }
        else
        {
          if((pRsp->devInfoRec[i].deviceInfo.profileID == TOUCHLINK_PROFILE_ID) ||
             (pRsp->devInfoRec[i].deviceInfo.profileID == Z3_PROFILE_ID))
          {
            selectedIdx = i;
            break; // first match
          }
        }
      }
      if(selectedIdx < pRsp->cnt)
      {
        // NOTE - the original scan response device info is overwritten with the
        // selected sub-device info, to complete the data required for the application.
        selectedTarget.scanRsp.deviceInfo = pRsp->devInfoRec[selectedIdx].deviceInfo;

        for(i = 0; i < pRsp->cnt; ++i)
        {
          if((pRsp->devInfoRec[i].deviceInfo.profileID == TOUCHLINK_PROFILE_ID) ||
             (pRsp->devInfoRec[i].deviceInfo.profileID == Z3_PROFILE_ID))
          {
            // add new node to the list
            pCurr = bdb_AddRespondentNode(&pRespondentHead, NULL);

            if ( pCurr != NULL )
            {
              memset(pCurr, 0x00, sizeof(bdbFindingBindingRespondent_t));
              pCurr->data.endPoint = pRsp->devInfoRec[i].deviceInfo.endpoint;
              pCurr->data.panId = pDstAddr->panId;
              osal_cpyExtAddr(selectedTargetIEEEAddr, pRsp->devInfoRec[i].ieeeAddr);
            }
          }
        }
      }
      else
      {
        // no sub-device of the currently selected device was selected.
        // clear selection
        initiatorClearSelectedTarget();
        bdb_zclRespondentListClean(&pRespondentHead);
      }
    }
}

/*********************************************************************
 * @fn      initiatorProcessNwkStartRsp
 *
 * @brief   Send network start response to touchlink event loop
 *
 * @param   pRsp - pointer to network start response indication
 *
 * @return  none
 */
void initiatorProcessNwkStartRsp(bdbTLNwkStartRsp_t *pRsp)
{
  // Make sure we didn't timeout waiting for this response
  if(OsalPortTimers_getTimerTimeout( touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_START_RSP_EVT))
  {
    // Save the Network Start Response for later
    rxRsp.nwkStartRsp = *pRsp;

    // No need to wait longer
    OsalPortTimers_stopTimer(touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_START_RSP_EVT);
    OsalPort_setEvent(touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_START_RSP_EVT);
  }
}

/*********************************************************************
 * @fn      initiatorProcessNwkJoinRsp
 *
 * @brief   Send network join response to touchlink event loop
 *
 * @param   pRsp - pointer to network join response indication
 *
 * @return  none
 */
void initiatorProcessNwkJointRsp(afAddrType_t *pSrcAddr, bdbTLNwkJoinRsp_t *pRsp)
{
  if(OsalPort_memcmp( selectedTarget.srcAddr.addr.extAddr, pSrcAddr->addr.extAddr, Z_EXTADDR_LEN))
  {
    // Make sure we didn't timeout waiting for this response
    if(OsalPortTimers_getTimerTimeout( touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_JOIN_RSP_EVT))
    {
      // Save the Network Join Response for later
      rxRsp.nwkJoinRsp = *pRsp;

      // No need to wait longer
      OsalPortTimers_stopTimer( touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_JOIN_RSP_EVT );
      OsalPort_setEvent( touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_JOIN_RSP_EVT );
    }
  }
}

/*********************************************************************
 * @fn      initiatorProcessStateChange
 *
 * @brief   Process ZDO device state change
 *
 * @param   state - The device's network state
 *
 * @return  none
 */
void initiatorProcessStateChange( devStates_t devState )
{
  if(((devState == DEV_ROUTER) || (devState == DEV_END_DEVICE)) && (touchlinkDistNwk == TRUE))
  {
    // Set touchlink flag to false after joining is complete
    touchlinkDistNwk = FALSE;

    // Save the latest NIB to update our parent's address
    touchLink_UpdateNV( TOUCHLINK_UPDATE_NV_NIB );

    if ( !_NIB.CapabilityFlags )
    {
      _NIB.CapabilityFlags = ZDO_Config_Node_Descriptor.CapabilityFlags;
    }

    if(ZSTACK_ROUTER_BUILD)
    {
      bdb_setNodeIsOnANetwork(TRUE);
      OsalPortTimers_startTimer(touchLinkInitiator_TaskID, TOUCHLINK_NWK_FORMATION_SUCCESS_EVT, 500);
    }

    // We found our parent
    OsalPortTimers_stopTimer( touchLinkInitiator_TaskID, TOUCHLINK_W4_REJOIN_EVT );
  }

  else if((devState == DEV_NWK_ORPHAN) || (devState == DEV_NWK_DISC))
  {
    // Device has lost information about its parent; give it some time to rejoin
    if(!OsalPortTimers_getTimerTimeout( touchLinkInitiator_TaskID, TOUCHLINK_W4_REJOIN_EVT))
    {
      OsalPortTimers_startTimer(touchLinkInitiator_TaskID, TOUCHLINK_W4_REJOIN_EVT,
                             (NUM_DISC_ATTEMPTS + 1) * TOUCHLINK_INITIATOR_REJOIN_TIMEOUT);
    }
  }
}

/*********************************************************************
 * @fn      initiatorSetNwkToInitState
 *
 * @brief   Set our network state to its original state.
 *
 * @param   void
 *
 * @return  void
 */
void initiatorSetNwkToInitState()
{

  // Turn MAC receiver back to its old state
  ZMacSetReq( ZMacRxOnIdle, &savedRxOnIdle );

  // Tune back to our channel
  touchLink_SetChannel( _NIB.nwkLogicalChannel );

  // Set NWK task to run
  nwk_setStateIdle( FALSE );

#if (ZG_BUILD_ENDDEVICE_TYPE)
    if(ZG_DEVICE_ENDDEVICE_TYPE)
    {
      if(bdbAttributes.bdbNodeIsOnANetwork == TRUE)
      {
        nwk_SetCurrentPollRateType(POLL_RATE_DISABLED,FALSE);
        nwk_SetCurrentPollRateType(POLL_RATE_TYPE_JOIN_REJOIN,FALSE);
        nwk_SetCurrentPollRateType(POLL_RATE_TYPE_RESPONSE,FALSE);
      }
      else
      {
        nwk_SetCurrentPollRateType(POLL_RATE_DISABLED,TRUE);
      }
    }
#endif
}

/*********************************************************************
 * @fn      initiatorClearSelectedTarget
 *
 * @brief   clear selected target variable.
 *
 * @param   none
 *
 * @return  none
 */
void initiatorClearSelectedTarget( void )
{
  memset( &selectedTarget, 0x00, sizeof(targetCandidate_t) );
  selectedTarget.lastRssi = TOUCHLINK_WORST_RSSI;
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

#if ( ZSTACK_ROUTER_BUILD )
/*********************************************************************
 * @fn      initiatorJoinNwk
 *
 * @brief   Initiate a network join request.
 *
 * @param   void
 *
 * @return  void
 */
static void initiatorJoinNwk( void )
{
  // Save free ranges
  touchLink_UpdateNV( TOUCHLINK_UPDATE_NV_RANGES );

  // In case we're here after a leave
  touchLinkLeaveInitiated = FALSE;

  // Clear leave control logic
  ZDApp_LeaveCtrlReset();

  if(POLL_RATE == 0)
  {
    //For RxOnIdle Devices, set the poll rate to 1sec
    nwk_SetCurrentPollRateType(POLL_RATE_TYPE_GENERIC_1_SEC, TRUE);
  }

  touchLinkInitiator_PermitJoin( 0 );

  ZDOInitDevice( 0 );
  // Perform a network rejoin request
  _NIB.nwkState = NWK_REJOINING;
  initiatorReJoinNwk( MODE_REJOIN );
}

/*********************************************************************
 * @fn      initiatorReJoinNwk
 *
 * @brief   Send out an Rejoin Request.
 *
 * @param   startMode - MODE_REJOIN or MODE_RESUME
 *
 * @return  none
 */
static void initiatorReJoinNwk( devStartModes_t startMode )
{
  // Set NWK task to run
  nwk_setStateIdle( FALSE );

  // Configure MAC with our network parameters
  touchLink_SetMacNwkParams( _NIB.nwkDevAddress, _NIB.nwkPanId, _NIB.nwkLogicalChannel );

  // Use the new network paramters
  zgConfigPANID = _NIB.nwkPanId;
  zgDefaultChannelList = _NIB.channelList;
  osal_cpyExtAddr( ZDO_UseExtendedPANID, _NIB.extendedPANID );

  devStartMode = startMode;

  _tmpRejoinState = TRUE;

  // Start the network joining process
  OsalPort_setEvent( ZDAppTaskID, ZDO_NETWORK_INIT );
}
#endif

/*********************************************************************
 * @fn      initiatorSendScanReq
 *
 * @brief   Send out an Scan Request command on one of the TOUCHLINK channels.
 *
 * @param   freshScan - TRUE to start fresh scan, FALSE to resume existing process.
 *
 * @return  void
 */
static void initiatorSendScanReq( bool freshScan )
{
  bdbTLScanReq_t req;
  uint8_t newChannel;
  uint8_t secondaryChList[] = TOUCHLINK_SECONDARY_CHANNELS_SET;
  static uint8_t channelIndex = 0;

  // Set the device as initiator of touchlink commissioning
  touchLink_DeviceIsInitiator( TRUE );

  if ( freshScan )
  {
    channelIndex = 0;
  }

  // First figure out the channel
  if ( scanReqChannels == TOUCHLINK_SCAN_PRIMARY_CHANNELS )
  {
    if ( numScanReqSent < 5 )
    {
      // First five consecutive requests are sent on channel 11
      newChannel = TOUCHLINK_FIRST_CHANNEL;
    }
    else if ( numScanReqSent == 5 )
    {
      // Sixth request is sent on channel 15
      newChannel = TOUCHLINK_SECOND_CHANNEL;
    }
    else if ( numScanReqSent == 6 )
    {
      // Seventh request is sent on channel 20
      newChannel = TOUCHLINK_THIRD_CHANNEL;
    }
    else
    {
      // Last request is sent on channel 25
      newChannel = TOUCHLINK_FOURTH_CHANNEL;
    }
  }
  else
  {
    // scan secondary channel list
    if ( channelIndex < sizeof(secondaryChList) )
    {
       newChannel = secondaryChList[channelIndex++];
    }
    else
    {
      // set it to initial value for next discovery process
      channelIndex = 0;
      return;
    }
  }

  if ( touchLinkTransID != 0 )
  {
    // Build the request
    req.transID = touchLinkTransID;
    touchLinkLastAcceptedTransID = touchLinkTransID;

    req.zInfo.zInfoByte = 0;
    req.zLogicalType = zgDeviceLogicalType;
    if ( ZDO_Config_Node_Descriptor.CapabilityFlags & CAPINFO_RCVR_ON_IDLE )
    {
      req.zRxOnWhenIdle = TRUE;
    }

    req.touchLinkInfo.touchLinkInfoByte = 0;
    req.touchLinkFactoryNew = !bdbAttributes.bdbNodeIsOnANetwork;
    req.touchLinkAddressAssignment = TRUE;
    req.touchLinkInitiator = TRUE;
    req.touchLinkProfileInterop = TRUE;

    // First switch to the right channel
    touchLink_SetChannel( newChannel );

    // only save the current TX power if we are starting a new network discovery
    if( freshScan )
    {
      // read + save current TX power
      ZMacGetReq(ZMacPhyTransmitPowerSigned, (byte *) &savedTXPower);

      // set TX power to 0 per BDB v1.0 sec 8.7 (3)
      // TX power is restored after scan rsp is received
      ZMacSetTransmitPower(TX_PWR_ZERO);
    }

    // Broadcast the request
    bdbTL_Send_ScanReq( TOUCHLINK_INTERNAL_ENDPOINT, &bcastAddr, &req, initiatorSeqNum++ );

    numScanReqSent++;

    // After each transmission, wait BDBCTL_SCAN_TIME_BASE_DURATION seconds
    // to receive any responses.
    OsalPortTimers_startTimer( touchLinkInitiator_TaskID, TOUCHLINK_TL_SCAN_BASE_EVT, BDBCTL_SCAN_TIME_BASE_DURATION );
  }
  else
  {
    touchLinkInitiator_AbortTL();
  }
}

#if (ZSTACK_END_DEVICE_BUILD)
/*********************************************************************
 * @fn      initiatorSendNwkStartReq
 *
 * @brief   Send out a Network Start Request command.
 *
 * @param   pRsp - received Scan Response
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorSendNwkStartReq( bdbTLScanRsp_t *pRsp )
{
  bdbTLNwkStartReq_t *pReq;
  ZStatus_t status;

  pReq = (bdbTLNwkStartReq_t*)OsalPort_malloc(sizeof(bdbTLNwkStartReq_t));
  if ( pReq != NULL )
  {
    uint16_t i;
    bdbTLNwkParams_t *pParams = &(pReq->nwkParams);

    memset(pReq, 0, sizeof(bdbTLNwkStartReq_t));

    // Build the request
    pReq->transID = selectedTarget.scanRsp.transID;

    // Find out key index (prefer highest)
    for (i = 15; i > 0; i--)
    {
      if(((uint16_t)1 << i) &pRsp->keyBitmask)
      {
        break;
      }
    }
    pParams->keyIndex = i;

    // Copy in the encrypted network key
    touchLink_EncryptNwkKey(pParams->nwkKey, i, pRsp->transID, pRsp->responseID);

    pParams->nwkAddr = touchLink_PopNwkAddress();
    if(pParams->nwkAddr == 0)
    {
      pParams->nwkAddr = OsalPort_rand();
    }
    // update address for app notification
    selectedTarget.newNwkAddr = pParams->nwkAddr;
    selectedTargetNwkAddr = pParams->nwkAddr;

    // Set group ID range
    if(pRsp->totalGrpIDs > 0)
    {
      touchLink_PopGrpIDRange(pRsp->totalGrpIDs, &pParams->grpIDsBegin, &pParams->grpIDsEnd);
    }

    if(pRsp->touchLinkAddressAssignment)
    {
      touchLink_SplitFreeRanges(&pParams->freeNwkAddrBegin, &pParams->freeNwkAddrEnd,
                                &pParams->freeGrpIDBegin, &pParams->freeGrpIDEnd);
    }

#ifdef TOUCHLINK_INITIATOR_SET_NEW_NWK_PARAMS
    pParams->logicalChannel = _NIB.nwkLogicalChannel;
    pParams->panId = _NIB.nwkPanId;
    OsalPort_memcpy( pParams->extendedPANID, _NIB.extendedPANID ,Z_EXTADDR_LEN);
#endif

    osal_cpyExtAddr(pReq->initiatorIeeeAddr, NLME_GetExtAddr());
    pReq->initiatorNwkAddr = _NIB.nwkDevAddress;

    status = bdbTL_Send_NwkStartReq(TOUCHLINK_INTERNAL_ENDPOINT, &selectedTarget.srcAddr, pReq, initiatorSeqNum++);
    if ( status == ZSuccess )
    {
      // Keep a copy of the encryted network key sent to the target
      keyIndexSent = i;
      OsalPort_memcpy( encKeySent, pParams->nwkKey, SEC_KEY_LEN );
      responseIDSent = pRsp->responseID;

      // After the transmission, wait BDBCTL_RX_WINDOW_DURATION seconds to
      // receive a response.
      OsalPortTimers_startTimer( touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_START_RSP_EVT, BDBCTL_RX_WINDOW_DURATION );
    }

    OsalPort_free( pReq );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}
#endif

/*********************************************************************
 * @fn      initiatorSendNwkJoinReq
 *
 * @brief   Send out a Network Join Router or End Device Request command.
 *
 * @param   pRsp - received Scan Response
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorSendNwkJoinReq( bdbTLScanRsp_t *pRsp )
{
  bdbTLNwkJoinReq_t *pReq;
  ZStatus_t status;

  pReq = (bdbTLNwkJoinReq_t*)OsalPort_malloc(sizeof( bdbTLNwkJoinReq_t));
  if ( pReq != NULL )
  {
    uint16_t i;
    bdbTLNwkParams_t *pParams = &(pReq->nwkParams);

    memset(pReq, 0, sizeof(bdbTLNwkJoinReq_t));

    // Build the request
    pReq->transID = selectedTarget.scanRsp.transID;

    // Find out key index (prefer highest)
    for(i = 15; i > 0; i--)
    {
      if(((uint16_t)1 << i) &pRsp->keyBitmask)
      {
        break;
      }
    }
    pParams->keyIndex = i;

    // Copy in the encrypted network key
    touchLink_EncryptNwkKey(pParams->nwkKey, i, pRsp->transID, pRsp->responseID);

    pParams->nwkAddr = touchLink_PopNwkAddress();
    if(pParams->nwkAddr == 0)
    {
      pParams->nwkAddr = OsalPort_rand();
    }
    // update address for app notification
    selectedTarget.newNwkAddr = pParams->nwkAddr;
    selectedTargetNwkAddr = pParams->nwkAddr;

    // Set group ID range
    if(pRsp->totalGrpIDs > 0)
    {
      touchLink_PopGrpIDRange(pRsp->totalGrpIDs, &pParams->grpIDsBegin, &pParams->grpIDsEnd);
    }

    if(pRsp->touchLinkAddressAssignment)
    {
      touchLink_SplitFreeRanges(&pParams->freeNwkAddrBegin, &pParams->freeNwkAddrEnd,
                                &pParams->freeGrpIDBegin, &pParams->freeGrpIDEnd);
    }

    pParams->logicalChannel = _NIB.nwkLogicalChannel;
    pParams->panId = _NIB.nwkPanId;
    osal_cpyExtAddr(pParams->extendedPANID, _NIB.extendedPANID);
    pReq->nwkUpdateId = _NIB.nwkUpdateId;

    // Let PAN ID, Extended PAN ID and Logical Channel to be determined by the target
    if(pRsp->zLogicalType == ZG_DEVICETYPE_ROUTER)
    {
      // It's a light
      status = bdbTL_Send_NwkJoinRtrReq(TOUCHLINK_INTERNAL_ENDPOINT, &selectedTarget.srcAddr, pReq, initiatorSeqNum++);
    }
    else // another controller
    {
      status = bdbTL_Send_NwkJoinEDReq(TOUCHLINK_INTERNAL_ENDPOINT, &selectedTarget.srcAddr, pReq, initiatorSeqNum++);
    }

    if(status == ZSuccess)
    {
      // After the transmission, wait BDBCTL_RX_WINDOW_DURATION seconds to
      // receive a response.
      OsalPortTimers_startTimer(touchLinkInitiator_TaskID, TOUCHLINK_W4_NWK_JOIN_RSP_EVT, BDBCTL_RX_WINDOW_DURATION);
    }

    OsalPort_free( pReq );
  }
  else
  {
    status = ZMemError;
  }

  return(status);
}

/*********************************************************************
 * @fn      initiatorSendNwkUpdateReq
 *
 * @brief   Send out a Network Update Request command.
 *
 * @param   pRsp - received Scan Response
 *
 * @return  ZStatus_t
 */
static ZStatus_t initiatorSendNwkUpdateReq( bdbTLScanRsp_t *pRsp )
{
  bdbTLNwkUpdateReq_t *pReq;
  ZStatus_t status;

  pReq = (bdbTLNwkUpdateReq_t*)OsalPort_malloc(sizeof( bdbTLNwkUpdateReq_t));
  if ( pReq!= NULL )
  {
    // Build the request
    pReq->transID = pRsp->transID;
    osal_cpyExtAddr( pReq->extendedPANID, _NIB.extendedPANID );
    pReq->nwkUpdateId = _NIB.nwkUpdateId;
    pReq->logicalChannel = _NIB.nwkLogicalChannel;
    pReq->PANID = _NIB.nwkPanId;
    pReq->nwkAddr = pRsp->nwkAddr;

    status = bdbTL_Send_NwkUpdateReq(TOUCHLINK_INTERNAL_ENDPOINT, &selectedTarget.srcAddr, pReq, initiatorSeqNum++);

    OsalPort_free( pReq );
  }
  else
  {
    status = ZMemError;
  }

  return ( status );
}

#endif //BDB_TL_INITIATOR

/*********************************************************************
*********************************************************************/
