/******************************************************************************

 @file  mac_coord.c

 @brief This module implements MAC device management procedures for a
        coordinator.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2005-2023, Texas Instruments Incorporated

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

 ******************************************************************************
 
 
 *****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#ifndef LIBRARY
#include "ti_wisunfan_features.h"
#endif

#include "hal_mcu.h"
#include "timac_api.h"
#include "mac_spec.h"
#include "mac_main.h"
#include "mac_timer.h"
#include "mac_mgmt.h"
#include "mac_data.h"
#include "mac_pib.h"
#include "mac_coord.h"
#include "mac_security.h"
#include "mac_low_level.h"
#include "mac_ie_build.h"

#include "rom_jt_154.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_coord_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE
/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */
void macIndirectMark(macTx_t *pMsg);
void macIndirectTxFrame(macTx_t *pMsg);
void macIndirectRequeueFrame(macTx_t *pMsg);

/**************************************************************************************************
 * @fn          macCoordReset
 *
 * @brief       This function initializes the data structures for module coord.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macCoordReset(void)
{
  /* Coordinator action set 1 */
  macCoordAction1[0] = MAP_macApiStartReq;
  /* Coordinator action set 2 */
  macCoordAction2[0] = MAP_macStartComplete;
  /* Coordinator action set 3 */
  macCoordAction3[0] = MAP_macApiPurgeReq;

  if (macCoord.pStartReq != NULL)
  {
    MAP_osal_mem_free(macCoord.pStartReq);
  }
  macCoord.indirectTimer.pFunc = MAP_macSetEvent;
  macCoord.indirectTimer.parameter = MAC_EXP_INDIRECT_TASK_EVT;
  macCoord.pStartReq = NULL;
  macMgmt.networkStarted = FALSE;
  macCoord.beaconing = FALSE;
  MAP_osal_memset(&macCoord.realignSec, 0, sizeof(ApiMac_sec_t));
  MAP_osal_memset(&macCoord.beaconSec, 0, sizeof(ApiMac_sec_t));
}

/**************************************************************************************************
 * @fn          macBuildAssociateRsp
 *
 * @brief       This function builds and sends an associate response command frame in buffer
 *              pEvent.  The TX options and frame type are set and other parameters in
 *              the macTx_t structure are initialized.  Then macBuildHeader() is called to
 *              build the header with the address information from the API event structure.
 *              If successful the status and the short address are built in the frame and
 *              macDataTxEnqueue() is called to queue the frame for transmission.
 *
 * input parameters
 *
 * @param       pEvent - Pointer to event data.
 *
 * output parameters
 *
 * None.
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macBuildAssociateRsp(macEvent_t *pEvent)
{
  sAddr_t   dstAddr;
  uint8     status;

  /* set parameters */
  pEvent->tx.internal.txOptions = MAC_TXOPTION_ACK | MAC_TXOPTION_INDIRECT;

  pEvent->tx.internal.frameType = MAC_INTERNAL_ASSOC_RSP;

  /* build status */
  pEvent->tx.msdu.p--;
  *pEvent->tx.msdu.p-- = pEvent->apiData.mac.associateRsp.status;

  /* build short address */
  if (pEvent->apiData.mac.associateRsp.status != MAC_SUCCESS)
  {
    pEvent->apiData.mac.associateRsp.assocShortAddress = MAC_SHORT_ADDR_NONE;
  }
  *pEvent->tx.msdu.p-- = HI_UINT16(pEvent->apiData.mac.associateRsp.assocShortAddress);
  *pEvent->tx.msdu.p-- = LO_UINT16(pEvent->apiData.mac.associateRsp.assocShortAddress);

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif
  /* Only the private payload field(Command Contents in case of command frame)
     should be secured. */
  pEvent->tx.internal.ptrMData = pEvent->tx.msdu.p + 1;
#if (defined TIMAC_ROM_IMAGE_BUILD)
    }
#endif

#endif

  /* Command Identifier */
  *pEvent->tx.msdu.p = pEvent->tx.internal.frameType - 1;

  /* set length */
  pEvent->tx.msdu.len += MAC_ASSOC_RSP_PAYLOAD;

  /* set up destination address */
  MAP_sAddrExtCpy(dstAddr.addr.extAddr, pEvent->apiData.mac.associateRsp.deviceAddress);
  dstAddr.addrMode = SADDR_MODE_EXT;

  /* initialize IE pointers */
  pEvent->tx.internal.ptrHeaderIE = NULL;
  pEvent->tx.internal.ptrPayloadIE = NULL;
#ifdef FEATURE_FREQ_HOP_MODE
  /* Frequency hopping enabled */
  if ( MAC_FH_ENABLED )
  {
    pEvent->tx.internal.fhFrameType = MAC_FH_FRAME_DATA;

    pEvent->tx.internal.nb = 0;
    pEvent->tx.internal.be = (pEvent->tx.internal.txOptions & MAC_TXOPTION_ALT_BE) ? pMacPib->altBe : pMacPib->minBe;

    /* Copy the address (needs to be extended) for FH module to use */

    pEvent->tx.internal.dest.dstAddr.addrMode = SADDR_MODE_EXT;
    /*Assoc. Rsp always has ext. addr */
      MAP_sAddrExtCpy(pEvent->tx.internal.dest.destAddr, pEvent->apiData.mac.associateRsp.deviceAddress);
      if(pMacPib->rxOnWhenIdle)
      {
        /* include the necessary Payaload IE's */
        if ( macCmdIncludeFHIe[MAC_INTERNAL_ASSOC_RSP] & MAC_FH_PAYLOAD_IE_MASK )
        {
          pEvent->apiData.internal.payloadIeInfo[0].gId = MAC_PAYLOAD_IE_WISUN_GID;
          pEvent->apiData.internal.payloadIeInfo[0].subId.subIdBMap =
          macCmdIncludeFHIe[MAC_INTERNAL_ASSOC_RSP] & MAC_FH_PAYLOAD_IE_MASK;

          MAP_macBuildPayloadIes(&pEvent->tx, pEvent->apiData.internal.payloadIeInfo, TRUE, FALSE);
        }

        /* include the necessary header IE's */
        if ( macCmdIncludeFHIe[MAC_INTERNAL_ASSOC_RSP] & MAC_FH_HEADER_IE_MASK )
        {
          pEvent->apiData.internal.headerIeInfo[0].elementId = MAC_HEADER_IE_WISUN;
          pEvent->apiData.internal.headerIeInfo[0].subIdBitMap =
          macCmdIncludeFHIe[MAC_INTERNAL_ASSOC_RSP] & MAC_FH_HEADER_IE_MASK;

          MAP_macBuildHeaderIes(&pEvent->tx, pEvent->apiData.internal.headerIeInfo);
        }
      }
  }
#endif
  status = MAP_macBuildHeader(&pEvent->tx, SADDR_MODE_EXT, &dstAddr, pMacPib->panId, 0);
  if (status == MAC_SUCCESS)
  {
    /* queue frame for transmission */
    MAP_macDataTxEnqueue(&pEvent->tx);
  }
  return status;
}

/**************************************************************************************************
 * @fn          macBuildRealign
 *
 * @brief       This function builds and sends a coordinator realignment command frame.
 *              First a buffer is allocated with macAllocTxBuffer().  If successful the
 *              frame type is set and other parameters in the macTx_t structure
 *              are initialized.  Then macBuildHeader() is called to build the header with the
 *              address information from the API event structure.  If successful the PAN ID
 *              and logical channel are built in the frame and macDataTxEnqueue() is called
 *              to queue the frame for transmission.
 *
 * input parameters
 *
 * @param       pMsg - Pointer to message buffer.
 * @param       pDstAddr - Destination address.
 * @param       coordShortAddr - Coordinator short address.
 * @param       newPanId - New PAN ID in realignment message frame.
 * @param       logicalChannel - Logical channel in realignment message frame.
 *
 * output parameters
 *
 * None.
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macBuildRealign(macTx_t *pMsg, sAddr_t *pDstAddr, uint16 dstShortAddr,
                                       uint16 newPanId, uint8 logicalChannel)
{
  uint8   *p;
  uint8   status;

  pMsg->internal.frameType = MAC_INTERNAL_COORD_REALIGN;

  p = pMsg->msdu.p - 1;

  /* Short Addr */
  *p-- = HI_UINT16(dstShortAddr);
  *p-- = LO_UINT16(dstShortAddr);

  /* Logical Channel */
  *p-- = logicalChannel;

  /* Coord Addr */
  *p-- = HI_UINT16(pMacPib->shortAddress);
  *p-- = LO_UINT16(pMacPib->shortAddress);

  /* PAN ID */
  *p-- = HI_UINT16(newPanId);
  *p-- = LO_UINT16(newPanId);

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif
      /* Only the private payload field(Command Contents in case of command frame)
         should be secured. */
      pMsg->internal.ptrMData = p + 1;
#if (defined TIMAC_ROM_IMAGE_BUILD)
    }
#endif
#endif

  /* set command id field */
  *p = pMsg->internal.frameType - 1;

  /* set length */
  pMsg->msdu.len += (uint8) (pMsg->msdu.p - p);
  pMsg->msdu.p = p;

  /* build header */
  if (pDstAddr->addrMode == SADDR_MODE_EXT)
  {
    pMsg->internal.txOptions = MAC_TXOPTION_ACK;
  }
  else
  {
    pMsg->internal.txOptions = 0;
  }

  /* Copy security parameter from start request */
  MAP_osal_memcpy(&pMsg->sec, &macCoord.realignSec, sizeof(ApiMac_sec_t));

  /* initialize IE pointers */
  pMsg->internal.ptrHeaderIE = NULL;
  pMsg->internal.ptrPayloadIE = NULL;

  status = MAP_macBuildHeader(pMsg, SADDR_MODE_EXT, pDstAddr, MAC_PAN_ID_BROADCAST, 0);
  if (status == MAC_SUCCESS)
  {
    /* queue frame for transmission */
    MAP_macDataTxEnqueue(pMsg);
  }
  return status;
}

/**************************************************************************************************
 * @fn          macIndirectMark
 *
 * @brief       This function searches through the data queue for indirect frames matching
 *              the given address.  The last matching frame in the queue is marked.  This
 *              mark is used to optimize building of the pending address list in the
 *              beacon frame.
 *
 * input parameters
 *
 * @param       pMsg - message containing address to match.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void macIndirectMark(macTx_t *pMsg)
{
  macTx_t       *pCurr;
  macTx_t       *pPrev;
  halIntState_t intState;

  HAL_ENTER_CRITICAL_SECTION(intState);

  pPrev = NULL;
  pCurr = MAP_OSAL_MSG_Q_HEAD(&macData.txQueue);
  while (pCurr != NULL)
  {
    /* if indirect and address matches */
    if ((pCurr->internal.txSched & MAC_TX_SCHED_INDIRECT) &&
        MAP_macDestAddrCmp(pCurr->msdu.p, pMsg->msdu.p))
    {
      /* clear mark */
      pCurr->internal.txSched &= ~MAC_TX_SCHED_MARKED;

      /* save match */
      pPrev = pCurr;
    }
    pCurr = MAP_OSAL_MSG_NEXT(pCurr);
  }

  /* if match found */
  if (pPrev != NULL)
  {
    /* mark it */
    pPrev->internal.txSched |= MAC_TX_SCHED_MARKED;
  }

  HAL_EXIT_CRITICAL_SECTION(intState);
}
/**************************************************************************************************
 * @fn          macApiPurgeReq
 *
 * @brief       This action function handles the API purge request event.  It searches the tx
 *              queue for an indirect data frame with a matching handle.  If found, it removes
 *              the frame from the queue.
 *
 * input parameters
 *
 * @param       pEvent - Pointer to event data.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macApiPurgeReq(macEvent_t *pEvent)
{
  macMcpsDataReq_t  *pCurr;
  macMcpsDataReq_t  *pPrev = NULL;
  uint8             status = MAC_INVALID_HANDLE;
  halIntState_t     intState;

  HAL_ENTER_CRITICAL_SECTION(intState);
  pCurr = MAP_OSAL_MSG_Q_HEAD(&macData.txQueue);
  while (pCurr != NULL)
  {
    /* if indirect data frame with matchine handle */
    if ((pCurr->internal.txSched & MAC_TX_SCHED_INDIRECT) &&
        (pCurr->internal.frameType == MAC_INTERNAL_DATA) &&
        (pCurr->mac.msduHandle == pEvent->api.mac.purgeReq.msduHandle))
    {
      /* remove it from data queue */
      MAP_osal_msg_extract(&macData.txQueue, pCurr, pPrev);

      /* decrement indirect frame count */
      macData.indirectCount--;

      /* we're done */
      break;
    }
    pPrev = pCurr;
    pCurr = MAP_OSAL_MSG_NEXT(pCurr);
  }
  HAL_EXIT_CRITICAL_SECTION(intState);

  /* if frame found */
  if (pCurr != NULL)
  {
    /* mark indirect queue for removed frame */
    MAP_macIndirectMark((macTx_t *) pCurr);

    /* send data cnf callback for removed frame */
    MAP_macCbackForEvent((macEvent_t *) pCurr, MAC_TRANSACTION_EXPIRED);

    status = MAC_SUCCESS;
  }

  /* send purge cnf callback */
  MAP_macCbackForEvent(pEvent, status);
}

/**************************************************************************************************
 * @fn          macApiStartReq
 *
 * @brief       This action function handles an API start request event.
 *
 * input parameters
 *
 * @param       pEvent - Pointer to event data.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
#if !defined (TIMAC_ROM_PATCH)
MAC_INTERNAL_API void macApiStartReq(macEvent_t *pEvent)
{
  uint8   status;
  macTx_t *pMsg;
  sAddr_t dstAddr;

  if((pEvent->hdr.status  != MAC_UNAVAILABLE_KEY) &&
       (pEvent->hdr.status  != MAC_UNSUPPORTED_SECURITY) &&
       (pEvent->hdr.status  != MAC_INVALID_PARAMETER) &&
       (pEvent->hdr.status  != MAC_COUNTER_ERROR))
  {
    /* parameter check */
    status = MAP_macStartBegin(pEvent);

    /* handle coordRealignment */
    if (status == MAC_SUCCESS && pEvent->api.mac.startReq.coordRealignment &&
        (!pEvent->api.mac.startReq.startFH) && !MAC_FH_ENABLED)
    {
      /* allocate buffer */
      if ((pMsg = MAP_macAllocTxBuffer(MAC_INTERNAL_COORD_REALIGN, &pEvent->api.mac.startReq.realignSec)) != NULL)
      {
        dstAddr.addrMode = SADDR_MODE_SHORT;
        dstAddr.addr.shortAddr = MAC_SHORT_ADDR_BROADCAST;
        status = MAP_macBuildRealign(pMsg, &dstAddr, MAC_SHORT_ADDR_BROADCAST,
                                 pEvent->api.mac.startReq.panId,
                                 pEvent->api.mac.startReq.logicalChannel);

        if((status  != MAC_UNAVAILABLE_KEY) &&
           (status  != MAC_UNSUPPORTED_SECURITY) &&
           (status  != MAC_INVALID_PARAMETER) &&
           (status  != MAC_COUNTER_ERROR))
        {
          return;
        }
      }
    }

    pEvent->hdr.status = status;
  }

  MAP_macStartContinue(pEvent);
}
#endif
/**************************************************************************************************
 * @fn          macStartBegin
 *
 * @brief       This function handles begins the processing of a start request.
 *              Here the validity of the parameters are checked and the pStartReq
 *              is allocated.
 *
 * input parameters
 *
 * @param       pEvent - Pointer to event data.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macStartBegin(macEvent_t *pEvent)
{
  uint8 status = MAC_SUCCESS;

#ifndef FEATURE_WISUN_SUPPORT
  /* check parameters */
  MAC_PARAM_STATUS((pEvent->api.mac.startReq.channelPage == MAC_CHANNEL_PAGE_9) ||
                   (pEvent->api.mac.startReq.channelPage == MAC_CHANNEL_PAGE_10), status);
#endif
  /* verify phy ID */
  MAC_PARAM_STATUS(((pEvent->api.mac.startReq.phyID >= MAC_MRFSK_STD_PHY_ID_BEGIN) &&
                   (pEvent->api.mac.startReq.phyID <= MAC_MRFSK_STD_PHY_ID_END)) ||
                   ((pEvent->api.mac.startReq.phyID >= MAC_MRFSK_GENERIC_PHY_ID_BEGIN) &&
                   (pEvent->api.mac.startReq.phyID <= MAC_MRFSK_GENERIC_PHY_ID_END)), status);

  if (MAC_SUCCESS == status)
  {
     /* change Phy ID before starting network */
     MAP_MAC_MlmeSetReq(MAC_PHY_CURRENT_DESCRIPTOR_ID, &pEvent->api.mac.startReq.phyID);
     MAP_MAC_MlmeSetReq(MAC_CHANNEL_PAGE, &pEvent->api.mac.startReq.channelPage);

    /* allocate buffer for start parameters */
    if ((macCoord.pStartReq = MAP_osal_mem_alloc(sizeof(ApiMac_mlmeStartReq_t))) != NULL)
    {
      MAP_osal_memcpy(macCoord.pStartReq, &pEvent->api.mac.startReq, sizeof(ApiMac_mlmeStartReq_t));
    }
    else
    {
      status = MAC_NO_RESOURCES;
    }
  }
  return status;
}

/**************************************************************************************************
 * @fn          macStartSetParams
 *
 * @brief       This function sets parameters from an API start request in the MAC.
 *
 * input parameters
 *
 * @param       pStartReq - Pointer to start request data.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macStartSetParams(ApiMac_mlmeStartReq_t *pStartReq)
{
  /* set pan coordinator bit in radio; if pan coordinator set pan id and logical channel */
  macPanCoordinator = pStartReq->panCoordinator;
  MAP_macRadioSetPanCoordinator(macPanCoordinator);

  MAP_MAC_MlmeSetReq(MAC_PHY_CURRENT_DESCRIPTOR_ID, &pStartReq->phyID);
  MAP_MAC_MlmeSetReq(MAC_CHANNEL_PAGE, &pStartReq->channelPage);

  if (macPanCoordinator)
  {
    MAP_MAC_MlmeSetReq(MAC_PAN_ID, &pStartReq->panId);
    MAP_MAC_MlmeSetReq(MAC_LOGICAL_CHANNEL, &pStartReq->logicalChannel);
  }

  DBG_PRINT1(DBGSYS, "macStartSetParams(): Setting PhyId = %i", pStartReq->phyID);
  DBG_PRINT1(DBGSYS, "macStartSetParams(): Setting Logical Ch = %i", pStartReq->logicalChannel);

  macMgmt.networkStarted = TRUE;
  macCoord.beaconing = (pMacPib->beaconOrder != MAC_BO_NON_BEACON);

  /* Save security parameters */
  MAP_osal_memcpy(&macCoord.realignSec, &pStartReq->realignSec, sizeof(ApiMac_sec_t));
  MAP_osal_memcpy(&macCoord.beaconSec, &pStartReq->beaconSec, sizeof(ApiMac_sec_t));
}

/**************************************************************************************************
 * @fn          macStartContinue
 *
 * @brief       This action function continues with the start procedure.
 *
 * input parameters
 *
 * @param       pEvent - Pointer to event data.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macStartContinue(macEvent_t *pEvent)
{
  if ( pEvent->hdr.status == MAC_SUCCESS )
  {
    MAP_macStartSetParams(macCoord.pStartReq);

    /* If start request is made and frequency hopping is enabled
       start the Frequency hopping */
    if (MAC_FH_ENABLED && MAC_FH_START_FN)
    {
      MAC_FH_START_FN();
    }
  }

  /* complete the start */
  pEvent->hdr.event = MAC_INT_START_COMPLETE_EVT;
  MAP_macExecute(pEvent);
}

/**************************************************************************************************
 * @fn          macStartComplete
 *
 * @brief       This action function is called when the start procedure is complete.  The
 *              pMacStartReq buffer is deallocated.  The MAC callback is called with event
 *              MAC_MLME_START_CNF with status from the event.
 *
 * input parameters
 *
 * @param       pEvent - Pointer to event data.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macStartComplete(macEvent_t *pEvent)
{
  if (macCoord.pStartReq != NULL)
  {
    void *tmp = (void *)macCoord.pStartReq;
    macCoord.pStartReq = NULL;
    MAP_osal_mem_free(tmp);
  }
  pEvent->hdr.event = MAC_MLME_START_CNF;
  MAP_MAC_CbackEvent((macCbackEvent_t *) pEvent);
}

/**************************************************************************************************
 * @fn          macRxDataReq
 *
 * @brief       This function processes a data request command frame.  If the frame was ack'ed
 *              with pending, the indirect frame is searched and frame sent to the requester.
 *              If no frame is found, a zero length data frame is sent. The MAC_MLME_POLL_IND
 *              is sent regardless of ack pending status.  The upper layer can use the noRsp
 *              flag to determine whether to respond to the poll indication with  MAC_McpsDataReq()
 *              or not. If noRsp is set to FALSE, the upp layer can respond to the poll
 *              indication with MAC_McpsDataReq().
 *
 *
 * input parameters
 *
 * @param       pEvent - Pointer to event data.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxDataReq(macEvent_t *pEvent)
{
  macMlmePollInd_t pollInd;
  ApiMac_sec_t     sec;
  uint8            noRsp = TRUE;

  /* if frame was ack'ed with pending */
  if (pEvent->rx.internal.flags & MAC_RX_FLAG_ACK_PENDING)
  {
    /* find and send requested frame */
    if (!MAP_macIndirectSend(&pEvent->rx.mac.srcAddr, pEvent->rx.mac.srcPanId))
    {
      if (macCfg.appPendingQueue == TRUE)
      {
        /* Short address matches, set noRsp flag to FALSE so the upper layer
         * can respond to the poll indication with MAC_McpsDataReq().
         */
        noRsp = FALSE;
      }
      else
      {
        /* copy the security settings for zero length data */
        MAP_osal_memcpy(&sec, &pEvent->rx.sec , sizeof(ApiMac_sec_t));

        /* if not found send empty data frame */
        MAP_macBuildCommonReq(MAC_INTERNAL_ZERO_DATA, MAP_macGetMyAddrMode(), &pEvent->rx.mac.srcAddr,
                          pEvent->rx.mac.srcPanId, MAC_TXOPTION_NO_RETRANS | MAC_TXOPTION_NO_CNF, &sec);
      }
    }
  }

  /* Send MAC_MLME_POLL_IND regardless of ack pending status */
  pollInd.hdr.event    = MAC_MLME_POLL_IND;
  MAP_sAddrCpy(&pollInd.srcAddr, &pEvent->rx.mac.srcAddr);
  pollInd.srcPanId     = pEvent->rx.mac.srcPanId;
  pollInd.noRsp        = noRsp;
  MAP_MAC_CbackEvent((macCbackEvent_t *) &pollInd);
}

/**************************************************************************************************
 * @fn          macRxPanConflict
 *
 * @brief       This function handles a PAN ID conflict command frame.  If configured as a
 *              PAN coordinator a sync loss indication is issued.
 *
 * input parameters
 *
 * @param       pEvent - Pointer to event data.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxPanConflict(macEvent_t *pEvent)
{
  (void)pEvent;  // Intentionally unreferenced parameter

  if (macPanCoordinator)
  {
    MAP_macConflictSyncLossInd();
  }
}

/**************************************************************************************************
 * @fn          MAC_InitCoord
 *
 * @brief       This function initializes the MAC as a coordinator.  It loads coordinator
 *              action functions into the state machine action table.  It also configures the
 *              state machine to use state tables required for operation as a device.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void MAC_InitCoord(void)
{
  /* Initialize action set for actions specific to Coordinator node */
  macActionSet[MAC_COORD_ACTION_1] = macCoordAction1;
  macActionSet[MAC_COORD_ACTION_2] = macCoordAction2;
  macActionSet[MAC_COORD_ACTION_3] = macCoordAction3;
#ifndef FEATURE_WISUN_SUPPORT
  macStateTbl[MAC_STARTING_ST] = macStartingSt;
#endif
  macReset[MAC_FEAT_COORD] = MAP_macCoordReset;

#ifndef FEATURE_WISUN_SUPPORT
  macDataTxIndirect = MAP_macIndirectTxFrame;
  macDataRequeueIndirect = MAP_macIndirectRequeueFrame;
#else
  macDataTxIndirect = NULL;
  macDataRequeueIndirect = NULL;
#endif
  /* tx frame set schedule function */
  macDataSetSched = MAP_macSetSched;
  /* tx frame check schedule function */
  macDataCheckSched = MAP_macCheckSched;

  macMain.featureMask |= MAC_FEAT_COORD_MASK;
}
/**************************************************************************************************
 * @fn          MAC_McpsPurgeReq
 *
 * @brief       This function is called by the application to purge a data request from the MAC
 *              data queue.  The function allocates an OSAL message and sends a
 *              MAC_API_PURGE_REQ_EVT to the MAC.
 *
 * input parameters
 *
 * @param       msduHandle - The application-defined handle value
 *
 * output parameters
 *
 * None.
 *
 * @return      MAC_SUCCESS or MAC error code.
 **************************************************************************************************
 */
uint8 MAC_McpsPurgeReq(uint8 msduHandle)
{
  macMcpsPurgeReq_t purgeReq;

  purgeReq.msduHandle = msduHandle;
  return (MAP_macSendMsg(MAC_API_PURGE_REQ_EVT, &purgeReq));
}

/**************************************************************************************************
 * @fn          MAC_MlmeStartReq
 *
 * @brief       This function calls macSendMsg() to send an OSAL message containing a
 *              MAC_API_START_REQ_EVT to the MAC.
 *
 * input parameters
 *
 * @param       pData - Pointer to parameters structure.
 *
 * output parameters
 *
 * None.
 *
 * @return      MAC_SUCCESS or MAC error code.
 **************************************************************************************************
 */
uint8 MAC_MlmeStartReq(ApiMac_mlmeStartReq_t *pData)
{
  return (MAP_macSendMsg(MAC_API_START_REQ_EVT, pData));
}
