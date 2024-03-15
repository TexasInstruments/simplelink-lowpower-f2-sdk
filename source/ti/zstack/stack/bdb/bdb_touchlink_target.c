/**************************************************************************************************
  Filename:       bdb_touchlink_target.c
  Revised:        $Date: 2013-11-26 15:12:49 -0800 (Tue, 26 Nov 2013) $
  Revision:       $Revision: 36298 $

  Description:    Zigbee Cluster Library - Light Link Target.


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
#include "rom_jt_154.h"
#include "ti_zstack_config.h"
#include "osal_nv.h"
#include "af.h"
#include "zd_app.h"
#include "nwk_util.h"
#include "addr_mgr.h"
#include "zd_sec_mgr.h"

#include "stub_aps.h"

#include "zcl.h"
#include "zcl_general.h"
#include "bdb.h"
#include "bdb_tl_commissioning.h"
#include "bdb_touchlink.h"

#include "bdb_touchlink_target.h"

#if defined ( BDB_TL_TARGET )

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8_t touchLinkTarget_TaskID;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
#if (ZSTACK_ROUTER_BUILD)
static afAddrType_t initiatorDstAddr;
static uint8_t rxSeqNum;
#endif

static uint8_t initiatorRxChannel;
static bdbTLReq_t rxReq; // network start or join request
static bool targetJoinedNwk;
static bool touchlinkAllowStealing = BDB_ALLOW_TL_STEALING;
static tlGCB_TargetEnable_t pfnTargetEnableChangeCB = NULL;
static bool touchlink_target_perpetual_operation = TARGET_PERPETUAL_OPERATION_ENABLED;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t touchLinkTarget_InitDevice( void );
static void targetSelectNwkParams( void );
static ZStatus_t targetVerifyNwkParams( uint16_t PANID, uint8_t *pExtendedPANID );
static ZStatus_t touchLinkTarget_PermitJoin( uint8_t duration );


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      touchLinkTarget_Init
 *
 * @brief   Initialization function for the TouchLink Target task.
 *
 * @param   task_id - TouchLink Target task id
 *
 * @return  none
 */
void touchLinkTarget_Init( uint8_t task_id )
{
  touchLinkTaskId = task_id;
  touchLinkTarget_TaskID = touchLinkTaskId;

  // Initialize TouchLink common variables
  touchLink_InitVariables( FALSE );

  // Register for Initiator to receive Leave Confirm
  ZDO_RegisterForZdoCB( ZDO_LEAVE_CNF_CBID, targetZdoLeaveCnfCB );

  // Register to process ZDO messages
  ZDO_RegisterForZDOMsg( touchLinkTarget_TaskID, Mgmt_Permit_Join_req );
  ZDO_RegisterForZDOMsg( touchLinkTarget_TaskID, Device_annce );
}


/*********************************************************************
 * @fn      touchLinkTarget_event_loop
 *
 * @brief   Event Loop Processor for TouchLink Target.
 *
 * @param   task_id - task id
 * @param   events - event bitmap
 *
 * @return  unprocessed events
 */
uint32_t touchLinkTarget_event_loop( uint8_t task_id, uint32_t events )
{

  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    OsalPort_EventHdr *pMsg;

    if ( (pMsg = (OsalPort_EventHdr *)OsalPort_msgReceive( touchLinkTarget_TaskID )) != NULL )
    {
      switch (pMsg->event )
      {
        case ZDO_CB_MSG:
          // ZDO sends the message that we registered for
          touchLink_RouterProcessZDOMsg( (zdoIncomingMsg_t *)pMsg );
          break;

        default:
          break;
      }

      // Release the OSAL message
      VOID OsalPort_msgDeallocate( (uint8_t *)pMsg );
    }

    // return unprocessed events
    return ( events ^ SYS_EVENT_MSG );
  }

  if ( events & TOUCHLINK_NWK_START_EVT )
  {
    bdbTLNwkStartReq_t *pReq = &(rxReq.nwkStartReq);

    // If the PAN Id, Extended PAN Id or Logical Channel are zero then
    // determine each of these parameters
    if ( !nwk_ExtPANIDValid( pReq->nwkParams.extendedPANID ) )
    {
      touchLink_GenerateRandNum( pReq->nwkParams.extendedPANID, Z_EXTADDR_LEN );
    }

    if ( pReq->nwkParams.panId == 0 )
    {
      pReq->nwkParams.panId = OsalPort_rand();
    }

    if ( pReq->nwkParams.logicalChannel == 0 )
    {
      pReq->nwkParams.logicalChannel = touchLink_GetRandPrimaryChannel();
    }

    if ( pReq->nwkParams.nwkAddr == 0 )
    {
      pReq->nwkParams.nwkAddr = OsalPort_rand();
    }

    // Perform Network Discovery to verify our new network parameters uniqeness
    touchLink_PerformNetworkDisc( (uint32_t)1 << pReq->nwkParams.logicalChannel );

    initiatorRxChannel = _NIB.nwkLogicalChannel;

    // return unprocessed events
    return ( events ^ TOUCHLINK_NWK_START_EVT );
  }

  if ( events & TOUCHLINK_NWK_FORMATION_SUCCESS_EVT )
  {
#if (ZSTACK_ROUTER_BUILD)
    bdbTLNwkStartReq_t *pReq = &(rxReq.nwkStartReq);
    bdbTLNwkParams_t *pParams = &(pReq->nwkParams);

    if( bdbCommissioningProcedureState.bdbCommissioningState == BDB_COMMISSIONING_STATE_TL )
    {
      if ( targetJoinedNwk == FALSE )
      {
        // Tune to the channel that the Scan Response was heard on
        touchLink_SetChannel( initiatorRxChannel );

        touchLinkTarget_PermitJoin(APLC_MAX_PERMIT_JOIN_DURATION);

        // Send a response back
        targetSendNwkStartRsp( &initiatorDstAddr, pReq->transID, TOUCHLINK_NETWORK_START_RSP_STATUS_SUCCESS, pParams, _NIB.nwkUpdateId, rxSeqNum );
      }

      zTouchLinkNwkStartRtr = FALSE;

      OsalPortTimers_startTimer( touchLinkTarget_TaskID, TOUCHLINK_NWK_RESTORE_NWK_PARAMETERS_EVT, 500 );
    }
#else
    (void)targetJoinedNwk;
    (void)initiatorRxChannel;
#endif
    return ( events ^ TOUCHLINK_NWK_FORMATION_SUCCESS_EVT );
  }

  if ( events & TOUCHLINK_NWK_RESTORE_NWK_PARAMETERS_EVT )
  {
#if (ZSTACK_ROUTER_BUILD)
    bdbTLNwkStartReq_t *pReq = &(rxReq.nwkStartReq);
    bdbTLNwkParams_t *pParams = &(pReq->nwkParams);

    if ( OsalPortTimers_getTimerTimeout( touchLinkTarget_TaskID, TOUCHLINK_NWK_FORMATION_SUCCESS_EVT ) )
    {
      OsalPortTimers_stopTimer( touchLinkTarget_TaskID, TOUCHLINK_NWK_FORMATION_SUCCESS_EVT );
    }

    if( bdbCommissioningProcedureState.bdbCommissioningState == BDB_COMMISSIONING_STATE_TL )
    {
      // Tune back to our channel
      touchLink_SetChannel( pParams->logicalChannel );
      bdb_reportCommissioningState( BDB_COMMISSIONING_STATE_TL, TRUE );
    }
#endif
    OsalPortTimers_startTimer( touchLinkTarget_TaskID, TOUCHLINK_NWK_ANNOUNCE_EVT, 250 );

    return ( events ^ TOUCHLINK_NWK_RESTORE_NWK_PARAMETERS_EVT );
  }

  if ( events & TOUCHLINK_NWK_DISC_CNF_EVT )
  {
    bdbTLNwkStartReq_t *pReq = &(rxReq.nwkStartReq);
    bdbTLNwkParams_t *pParams = &(pReq->nwkParams);
    uint8_t status;

    // Verify the received Network Parameters
    if ( targetVerifyNwkParams( pParams->panId, pParams->extendedPANID ) == ZSuccess )
    {
      status = TOUCHLINK_NETWORK_START_RSP_STATUS_SUCCESS;
    }
    else
    {
      status = TOUCHLINK_NETWORK_START_RSP_STATUS_FAILURE;
    }


    if ( status == TOUCHLINK_NETWORK_START_RSP_STATUS_SUCCESS )
    {
      touchLink_FreeNwkParamList();
      // If not factory new, perform a Leave on our old network
      if ( ( bdbAttributes.bdbNodeIsOnANetwork == TRUE ) && ( touchLink_SendLeaveReq() == ZSuccess ) )
      {
        // Wait for Leave confirmation before joining the new network
        touchLinkLeaveInitiated = TOUCHLINK_LEAVE_TO_START_NWK;
      }
      else
      {
        // Notify our task to start the network
        OsalPort_setEvent( touchLinkTarget_TaskID, TOUCHLINK_START_NWK_EVT );
      }
    }
    else
    {
      // Join to the chosen network
      OsalPort_setEvent( touchLinkTarget_TaskID, TOUCHLINK_JOIN_ATTEMPT_EVT );
    }

    // return unprocessed events
    return ( events ^ TOUCHLINK_NWK_DISC_CNF_EVT );
  }

  if ( events & TOUCHLINK_NWK_JOIN_IND_EVT )
  {
    bdbTLNwkJoinReq_t *pReq = &(rxReq.nwkJoinReq);

    initiatorRxChannel = pReq->nwkParams.logicalChannel;

    // If not factory new, perform a Leave on our old network
    if ( ( bdbAttributes.bdbNodeIsOnANetwork == TRUE ) && ( touchLink_SendLeaveReq() == ZSuccess ) )
    {
      // Wait for Leave confirmation before joining the new network
      touchLinkLeaveInitiated = TOUCHLINK_LEAVE_TO_JOIN_NWK;
    }
    else
    {
      // Notify our task to join this network
      // Perform Network Discovery to verify our new network parameters uniqeness
      touchLink_PerformNetworkDisc( (uint32_t)1 << initiatorRxChannel );
    }

    // return unprocessed events
    return ( events ^ TOUCHLINK_NWK_JOIN_IND_EVT );
  }

  if ( events & TOUCHLINK_START_NWK_EVT )
  {
    bdbTLNwkStartReq_t *pReq = &(rxReq.nwkStartReq);
    bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_TL;

    // Start operating on the new network
    touchLinkStartRtr( &(pReq->nwkParams), pReq->transID );

    // Perform a ZigBee Direct Join in order to allow direct communication
    // via the ZigBee network between the Initiator and the Target (i.e.,
    // create an entry in the neighbor table with the IEEE address and the
    // network address of the Initiator).
    NLME_DirectJoinRequestWithAddr( pReq->initiatorIeeeAddr, pReq->initiatorNwkAddr,
                                    CAPINFO_DEVICETYPE_RFD );

    // return unprocessed events
    return ( events ^ TOUCHLINK_START_NWK_EVT );
  }

  if ( events & TOUCHLINK_JOIN_ATTEMPT_EVT )
  {
    bdbTLNwkJoinReq_t *pReq = &(rxReq.nwkJoinReq);
    initiatorRxChannel = pReq->nwkParams.logicalChannel;

#if ( ZSTACK_ROUTER_BUILD )
    bdbCommissioningProcedureState.bdbCommissioningState = BDB_COMMISSIONING_STATE_TL;

    // Start operating on the new network
    touchLinkStartRtr( &(pReq->nwkParams), pReq->transID );
#else
    bdbTLNwkRejoin_t rejoinInf;

    // Copy the new network parameters to NIB
    touchLink_SetNIB( NWK_REJOINING,
                pReq->nwkParams.nwkAddr, pReq->nwkParams.extendedPANID,
                pReq->nwkParams.logicalChannel, pReq->nwkParams.panId, pReq->nwkUpdateId );

    bdb_setNodeIsOnANetwork(TRUE);

    // Apply the received network key
    touchLink_DecryptNwkKey( pReq->nwkParams.nwkKey, pReq->nwkParams.keyIndex, pReq->transID, touchLinkResponseID );

    // This is not a usual Trust Center protected network
    ZDSecMgrUpdateTCAddress( NULL );

    rejoinInf.panId = pReq->nwkParams.panId;
    rejoinInf.logicalChannel = pReq->nwkParams.logicalChannel;
    OsalPort_memcpy( rejoinInf.extendedPANID, pReq->nwkParams.extendedPANID, Z_EXTADDR_LEN);
    rejoinInf.nwkAddr = discoveredTouchlinkNwk.chosenRouter;
    rejoinInf.nwkUpdateId = pReq->nwkUpdateId;

    touchLink_DevRejoin( &rejoinInf );
#endif
    touchLink_FreeNwkParamList();

    // return unprocessed events
    return ( events ^ TOUCHLINK_JOIN_ATTEMPT_EVT );
  }

  if ( events & TOUCHLINK_RESET_TO_FN_EVT )
  {
    bdb_resetLocalAction();

    // return unprocessed events
    return ( events ^ TOUCHLINK_RESET_TO_FN_EVT );
  }

  if ( events & TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT )
  {
    touchLinkTransID = 0;
    // return unprocessed events
    return ( events ^ TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT );
  }

  if(events & TOUCHLINK_TARGET_ENABLE_TIMEOUT)
  {
    touchLinkTargetEnabled = FALSE;
    touchlink_target_perpetual_operation = FALSE;
    pfnTargetEnableChangeCB( touchLinkTargetEnabled );
    (void)OsalPort_pwrmgrTaskState(touchLinkTarget_TaskID, OsalPort_PWR_CONSERVE);

    bdb_ClearNetworkParams();

#if (ZG_BUILD_ENDDEVICE_TYPE)
    if(ZG_DEVICE_ENDDEVICE_TYPE)
    {
      // Allow the poll manager to resume its current operation
      nwk_SetCurrentPollRateType(POLL_RATE_RX_ON_TRUE,FALSE);
    }
#endif
    return (events ^ TOUCHLINK_TARGET_ENABLE_TIMEOUT);
  }

  if ( events & TOUCHLINK_NWK_ANNOUNCE_EVT )
  {
    ZDP_DeviceAnnce( NLME_GetShortAddr(), NLME_GetExtAddr(),
                     ZDO_Config_Node_Descriptor.CapabilityFlags, 0 );
  }
  // If reach here, the events are unknown
  // Discard or make more handlers
  return 0;
}

/*********************************************************************
 * @fn      touchLink_TargetSendScanRsp
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
ZStatus_t touchLink_TargetSendScanRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint32_t transID, uint8_t seqNum )
{
  ZStatus_t status = ZSuccess;

  if ( touchLinkTargetEnabled == FALSE )
  {
    return ZFailure;
  }

  // Make sure we respond only once during a Device Discovery
  if ( touchLinkLastAcceptedTransID != transID )
  {
    bdbTLScanRsp_t *pRsp;

    pRsp = (bdbTLScanRsp_t *)OsalPort_malloc( sizeof( bdbTLScanRsp_t ) );
    if ( pRsp )
    {
      memset( pRsp, 0, sizeof( bdbTLScanRsp_t ) );

      // Save transaction id
      touchLinkLastAcceptedTransID = transID;
      OsalPortTimers_startTimer( touchLinkTarget_TaskID, TOUCHLINK_TRANS_LIFETIME_EXPIRED_EVT,
                          BDBCTL_INTER_PAN_TRANS_ID_LIFETIME );

      pRsp->transID = transID;
      pRsp->rssiCorrection = TOUCHLINK_RSSI_CORRECTION;
      pRsp->zLogicalType = zgDeviceLogicalType;
      pRsp->touchLinkAddressAssignment = touchLink_IsValidSplitFreeRanges( 0 );
      pRsp->touchLinkInitiator = FALSE;
      pRsp->touchLinkProfileInterop = TRUE;

      if ( ZDO_Config_Node_Descriptor.CapabilityFlags & CAPINFO_RCVR_ON_IDLE )
      {
        pRsp->zRxOnWhenIdle = TRUE;
      }

      pRsp->touchLinklinkPriority = FALSE;
      pRsp->keyBitmask = touchLink_GetNwkKeyBitmask();

      // Generate a new Response ID
      touchLinkResponseID = ( ((uint32_t)OsalPort_rand()) << 16 ) + OsalPort_rand();
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
      osal_cpyExtAddr( pRsp->extendedPANID, _NIB.extendedPANID );

      pRsp->numSubDevices = touchLink_GetNumSubDevices( 0 );
      if ( pRsp->numSubDevices == 1 )
      {
        touchLink_GetSubDeviceInfo( 0, &(pRsp->deviceInfo) );
      }

      pRsp->totalGrpIDs = touchLink_GetNumGrpIDs();

      // Send a response back
      status = bdbTL_Send_ScanRsp( srcEP, dstAddr, pRsp, seqNum );

      OsalPort_free( pRsp );
    }
    else
    {
      status = ZMemError;
    }
  }

  return ( status );
}

#if (ZSTACK_ROUTER_BUILD)
/*********************************************************************
 * @fn      targetSendNwkStartRsp
 *
 * @brief   Send out a Network Start Response command.
 *
 * @param   dstAddr - destination's address
 * @param   transID - touch link transaction identifier
 * @param   status - Network Start Response command status field
 * @param   pNwkParams - network parameters
 * @param   nwkUpdateId - network update identifier
 * @param   seqNum
 *
 * @return  none
 */
void targetSendNwkStartRsp( afAddrType_t *dstAddr, uint32_t transID, uint8_t status,
                                   bdbTLNwkParams_t *pNwkParams, uint8_t nwkUpdateId, uint8_t seqNum )
{
  bdbTLNwkStartRsp_t *pRsp;

  // Send out a response
  pRsp = (bdbTLNwkStartRsp_t *)OsalPort_malloc( sizeof( bdbTLNwkStartRsp_t ) );
  if ( pRsp )
  {
    pRsp->transID = transID;
    pRsp->status = status;

    if ( pNwkParams != NULL )
    {
      osal_cpyExtAddr( pRsp->extendedPANID, pNwkParams->extendedPANID );
      pRsp->logicalChannel = pNwkParams->logicalChannel;
      pRsp->panId = pNwkParams->panId;
    }
    else
    {
      memset( pRsp->extendedPANID, 0, Z_EXTADDR_LEN );
      pRsp->logicalChannel = 0;
      pRsp->panId = 0;
    }

    pRsp->nwkUpdateId = nwkUpdateId;

    bdbTL_Send_NwkStartRsp( TOUCHLINK_INTERNAL_ENDPOINT, dstAddr, pRsp, seqNum );

    OsalPort_free( pRsp );
  }
}

#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
/*********************************************************************
 * @fn      targetProcessNwkStartEvt
 *
 * @brief   Send network start event to touchlink event loop
 *
 * @param   bdbTLNwkStartReq_t - pointer to start request
 * @param   seqNum - sequence number
 *
 * @return  none
 */
void targetProcessNwkStartEvt(afAddrType_t *pAddr, bdbTLNwkStartReq_t *pNwkStartReq, uint8_t seqNum, bool allowStealing)
{
    targetJoinedNwk = FALSE;
    if ( ( allowStealing == TRUE ) || ( bdbAttributes.bdbNodeIsOnANetwork == FALSE ) )
    {
      // Save the request for later
      OsalPort_memcpy(&initiatorDstAddr, pAddr, sizeof(afAddrType_t));
      OsalPort_memcpy(&(rxReq.nwkStartReq), pNwkStartReq, sizeof(bdbTLNwkStartReq_t));
      rxSeqNum = seqNum;
      OsalPort_setEvent( touchLinkTarget_TaskID, TOUCHLINK_NWK_START_EVT );
    }
    else
    {
      targetSendNwkStartRsp( pAddr, pNwkStartReq->transID, TOUCHLINK_NETWORK_START_RSP_STATUS_FAILURE,
                             NULL, 0, seqNum );
    }
}
#endif
#endif  // ZSTACK_ROUTER_BUILD

/*********************************************************************
 * @fn      targetProcessNwkJoinEvt
 *
 * @brief   Send network join event to touchlink event loop
 *
 * @param   bdbTLNwkStartReq_t - pointer to join request
 * @param   seqNum - sequence number
 *
 * @return  none
 */
void targetProcessNwkJoinEvt(afAddrType_t *pAddr, bdbTLNwkJoinReq_t *pNwkJoinReq, uint8_t seqNum, bool allowStealing)
{
    bdbTLNwkJoinRsp_t rsp;

    targetJoinedNwk = TRUE;
    if ( ( allowStealing == TRUE ) || ( bdbAttributes.bdbNodeIsOnANetwork == FALSE ) )
    {
      // Save the request for later
      rxReq.nwkJoinReq = *pNwkJoinReq;
      touchlinkPanId = rxReq.nwkJoinReq.nwkParams.panId;

      // Wait at least BDBCTL_MIN_STARTUP_DELAY_TIME seconds to allow the
      // initiator to start the network correctly. Join the initiator afterwards.
      OsalPortTimers_startTimer( touchLinkTarget_TaskID, TOUCHLINK_NWK_JOIN_IND_EVT, BDBCTL_MIN_STARTUP_DELAY_TIME);

      rsp.status = TOUCHLINK_NETWORK_JOIN_RSP_STATUS_SUCCESS;
    }
    else
    {
      rsp.status = TOUCHLINK_NETWORK_JOIN_RSP_STATUS_FAILURE;
    }

    rsp.transID = pNwkJoinReq->transID;

    // Send a response back
  #if ( ZSTACK_ROUTER_BUILD )
    bdbTL_Send_NwkJoinRtrRsp( TOUCHLINK_INTERNAL_ENDPOINT, pAddr, &rsp, seqNum );
  #else
    bdbTL_Send_NwkJoinEDRsp( TOUCHLINK_INTERNAL_ENDPOINT, pAddr, &rsp, seqNum );
  #endif  //ZSTACK_ROUTER_BUILD
}

/*********************************************************************
 * @fn      targetProcessResetToFNEvt
 *
 * @brief   Send factory new reset to event loop.
 *
 * @return  none
 */
void targetProcessResetToFNEvt( void )
{
  OsalPort_setEvent( touchLinkTarget_TaskID, TOUCHLINK_RESET_TO_FN_EVT );
}

/*********************************************************************
 * @fn      targetProcessNwkUpdateEvt
 *
 * @brief   Send network Update event to touchlink event loop
 *
 * @param   bdbTLNwkUpdateReq_t - pointer to update request
 * @param   seqNum - sequence number
 *
 * @return  none
 */
void targetProcessNwkUpdateEvt(bdbTLNwkUpdateReq_t *pNwkUpdateReq)
{
    // Discard the request if the Extended PAN ID and PAN ID are not
    // identical with our corresponding stored values
    if ( TOUCHLINK_SAME_NWK( pNwkUpdateReq->PANID, pNwkUpdateReq->extendedPANID ) )
    {
      uint8_t newUpdateId = touchLink_NewNwkUpdateId( pNwkUpdateReq->nwkUpdateId, _NIB.nwkUpdateId);
      if ( _NIB.nwkUpdateId != newUpdateId )
      {
        // Update the network update id and logical channel
        touchLink_ProcessNwkUpdate( newUpdateId, pNwkUpdateReq->logicalChannel );
      }
    }
}

/*********************************************************************
 * @fn      touchLinkTarget_PermitJoin
 *
 * @brief   Set the router permit join flag, to allow or deny classical
 *          commissioning by other ZigBee devices.
 *
 * @param   duration - enable up to aplcMaxPermitJoinDuration seconds,
 *                     0 to disable
 *
 * @return  status
 */
ZStatus_t touchLinkTarget_PermitJoin( uint8_t duration )
{
  return touchLink_PermitJoin( duration );
}

/*********************************************************************
 * @fn      touchLinkTarget_EnableCommissioning
 *
 * @brief   Enable the reception of TL Commissioning commands. Refer to
 *          bdb_RegisterTouchlinkTargetEnableCB to get enable/disable notifications
 *
 * @param   timeoutTime - Enable timeout in ms
 *
 * @return  status
 */
void touchLinkTarget_EnableCommissioning( uint32_t timeoutTime )
{
    if( touchlink_target_perpetual_operation == FALSE)
    {
        touchLinkTargetEnabled = TRUE;

        touchLinkTarget_InitDevice( );

        // if time == 0xFFFF set target active forever, otherwise disable it in
        // the timeout given by timeoutTime in ms
        if ( timeoutTime < TOUCHLINK_TARGET_PERPETUAL )
        {
            OsalPortTimers_startTimer(touchLinkTarget_TaskID, TOUCHLINK_TARGET_ENABLE_TIMEOUT, timeoutTime);
            touchlink_target_perpetual_operation = FALSE;
        }
        else
        {
            touchlink_target_perpetual_operation = TRUE;
        }
        pfnTargetEnableChangeCB( touchLinkTargetEnabled );
        (void)OsalPort_pwrmgrTaskState(touchLinkTarget_TaskID, OsalPort_PWR_HOLD);
    }
}

/*********************************************************************
 * @fn      bdb_RegisterTouchlinkTargetEnableCB
 *
 * @brief   Register an Application's Enable/Disable callback function.
 *          Refer to touchLinkTarget_EnableCommissioning to enable/disable TL as target
 *
 * @param   pfnIdentify - application callback
 *
 * @return  none
 */
void bdb_RegisterTouchlinkTargetEnableCB( tlGCB_TargetEnable_t pfnTargetEnableChange )
{
  pfnTargetEnableChangeCB = pfnTargetEnableChange;
}

/*********************************************************************
 * @fn      touchLinkTarget_DisableCommissioning
 *
 * @brief   Disable TouchLink on a target device
 *
 * @param   none
 *
 * @return  none
 */
void touchLinkTarget_DisableCommissioning( void )
{
  OsalPortTimers_stopTimer(touchLinkTarget_TaskID, TOUCHLINK_TARGET_ENABLE_TIMEOUT);
  OsalPort_setEvent(touchLinkTarget_TaskID, TOUCHLINK_TARGET_ENABLE_TIMEOUT);
}

/*********************************************************************
 * @fn      touchLinkTarget_GetTimer
 *
 * @brief   Get remaining touchlink duration on a target device
 *
 * @param   none
 *
 * @return  Remaining touchlink duration in milliseconds
 */
uint32_t touchLinkTarget_GetTimer( void )
{
  if (!touchLinkTargetEnabled)
  {
    return 0;
  }
  else if (touchlink_target_perpetual_operation)
  {
    return TOUCHLINK_TARGET_PERPETUAL;
  }

  return OsalPortTimers_getTimerTimeout(touchLinkTarget_TaskID, TOUCHLINK_TARGET_ENABLE_TIMEOUT);
}

/******************************************************************************
 * @fn      targetZdoLeaveCnfCB
 *
 * @brief   This callback is called to process a Leave Confirmation message.
 *
 *          Note: this callback function returns a pointer if it has handled
 *                the confirmation message and no further action should be
 *                taken with it. It returns NULL if it has not handled the
 *                confirmation message and normal processing should take place.
 *
 * @param       pParam - received message
 *
 * @return      Pointer if message processed. NULL, otherwise.
 */
void* targetZdoLeaveCnfCB( void *pParam )
{
  // Did we initiate the leave?
  if ( touchLinkLeaveInitiated == FALSE )
  {
    return ( NULL );
  }

  if ( touchLinkLeaveInitiated == TOUCHLINK_LEAVE_TO_START_NWK )
  {
    // Notify our task to start the network
    OsalPort_setEvent( touchLinkTarget_TaskID, TOUCHLINK_START_NWK_EVT );
  }
  else // TOUCHLINK_LEAVE_TO_JOIN_NWK
  {
    AssocReset();
    nwkNeighborInitTable();
    AddrMgrSetDefaultNV();
    // Immediately store empty tables in NV
    OsalPort_setEvent( ZDAppTaskID, ZDO_NWK_UPDATE_NV );
    // Notify our task to start the network
    OsalPortTimers_startTimer( touchLinkTarget_TaskID, TOUCHLINK_NWK_START_EVT, 100 );
  }

  return ( (void *)&touchLinkLeaveInitiated );
}

/*********************************************************************
 * @fn      bdb_TouchlinkSetAllowStealing
 *
 * @brief   General function to allow stealing when performing TL as target
 *
 * @param   allow - allow stealling if TRUE, deny if FALSE
 *
 * @return  none
 */
void bdb_TouchlinkSetAllowStealing( bool allow )
{
  touchlinkAllowStealing = allow;
}

/*********************************************************************
 * @fn      bdb_TouchlinkGetAllowStealing
 *
 * @brief   General function to get the allow stealing value
 *
 * @param
 *
 * @return  return TRUE if allowed, FALSE if not allowed
 */
bool bdb_TouchlinkGetAllowStealing( void )
{
  return touchlinkAllowStealing;
}

/*********************************************************************
 * @fn      targetProcessStateChange
 *
 * @brief   Process ZDO device state change
 *
 * @param   devState - The device's network state
 *
 * @return  none
 */
void targetProcessStateChange( devStates_t devState )
{
  if ( ( ( devState == DEV_ROUTER ) || ( devState == DEV_END_DEVICE ) ) && ( touchlinkDistNwk == TRUE ) )
  {
    // Set touchlink flag to false after joining is complete
    touchlinkDistNwk = FALSE;

    if ( !_NIB.CapabilityFlags )
    {
      _NIB.CapabilityFlags = ZDO_Config_Node_Descriptor.CapabilityFlags;
    }
    // Initialize the security for type of device
    ZDApp_SecInit( ZDO_INITDEV_RESTORED_NETWORK_STATE );

    bdb_setNodeIsOnANetwork(TRUE);

    OsalPortTimers_startTimer( touchLinkTarget_TaskID, TOUCHLINK_NWK_FORMATION_SUCCESS_EVT, 500);

  }
}
/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      touchLinkTarget_InitDevice
 *
 * @brief   Start the TouchLink Target device in the network if it's not
 *          factory new. Otherwise, determine the network parameters
 *          and wait for a touchlink command.
 *
 * @param   none
 *
 * @return  status
 */
static ZStatus_t touchLinkTarget_InitDevice( void )
{
  ZDO_Config_Node_Descriptor.LogicalType = zgDeviceLogicalType;

  uint8_t x = TRUE;

  // Enable our receiver
  ZMacSetReq( ZMacRxOnIdle, &x );

#if (ZG_BUILD_ENDDEVICE_TYPE)
  if(ZG_DEVICE_ENDDEVICE_TYPE)
  {
    // Turn on the radio for touch-link as target procedure
    nwk_SetCurrentPollRateType(POLL_RATE_RX_ON_TRUE,TRUE);
  }
#endif
  if( bdbAttributes.bdbNodeIsOnANetwork == FALSE )
  {
    targetSelectNwkParams( );
  }

  // Wait for a touchlink command
  touchLinkTarget_PermitJoin(0);

  return ( ZSuccess );
}

/*********************************************************************
 * @fn      targetSelectNwkParams
 *
 * @brief   Select a unique PAN ID and Extended PAN ID when compared to
 *          the PAN IDs and Extended PAN IDs of the networks detected
 *          on the TouchLink channels. The selected Extended PAN ID must be
 *          a random number (and not equal to our IEEE address).
 *
 * @param   void
 *
 * @return  void
 */
static void targetSelectNwkParams( void )
{
  uint8_t status = ZFailure;

  while ( status == ZFailure )
  {
    // Select a random Extended PAN ID
    touchLink_GenerateRandNum( _NIB.extendedPANID, Z_EXTADDR_LEN );

    // Select a random PAN ID
    _NIB.nwkPanId = OsalPort_rand( );

    // Make sure they're unique
    status = targetVerifyNwkParams( _NIB.nwkPanId, _NIB.extendedPANID );
  }

  if ( _NIB.nwkLogicalChannel == 0 )
  {
    // Select randomly one of the TouchLink channels as our logical channel
    _NIB.nwkLogicalChannel = touchLink_GetRandPrimaryChannel( );
  }

  _NIB.nwkDevAddress = OsalPort_rand( );

  // Configure MAC with our network parameters
  touchLink_SetMacNwkParams( _NIB.nwkDevAddress, _NIB.nwkPanId, _NIB.nwkLogicalChannel );
}

/*********************************************************************
 * @fn      targetVerifyNwkParams
 *
 * @brief   Verify that the PAN ID and Extended PAN ID are unique.
 *
 * @param   PANID - PAN Identifier
 * @param   pExtendedPANID - extended PAN Identifier
 *
 * @return  status
 */
static ZStatus_t targetVerifyNwkParams( uint16_t PANID, uint8_t *pExtendedPANID )
{
  // Add for our network parameters in the Network Parameter
  if ( ( discoveredTouchlinkNwk.PANID == PANID ) &&
       ( osal_ExtAddrEqual( discoveredTouchlinkNwk.extendedPANID, pExtendedPANID ) ) )
  {
    return ( ZFailure );
  }

  return ( ZSuccess );
}
#endif
