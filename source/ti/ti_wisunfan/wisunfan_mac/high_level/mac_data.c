/******************************************************************************

 @file  mac_data.c

 @brief This module implements high level procedures for receiving and
        transmitting data.

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
#include "mac_mcu.h"
#include "timac_api.h"
#include "mac_main.h"
#include "mac_pib.h"
#include "mac_timer.h"
#include "mac_mgmt.h"
#include "mac_spec.h"
#include "mac_data.h"
#include "mac_coord.h"
#include "mac_low_level.h"
#include "mac_tx.h"
#include "mac_security.h"
#include "mac_rf4ce.h"
#include "mac_radio_defs.h"
#include "mac_ie_build.h"
#include "mac_ie_parse.h"
#include "mac_assert.h"
#include "mac_hl_patch.h"

#include "rom_jt_154.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_data_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */
#ifdef DBG_OSAL
extern struct osal_debug osalDbg;
#endif
/* tx timer callback for Green Power delayed transmission */
void macDataTxDelayCallback(uint8 param);

/**************************************************************************************************
 * @fn          macDataReset
 *
 * @brief       This function initializes the data structures for the data module.
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
MAC_INTERNAL_API void macDataReset(void)
{
  /* clear tx queue */
  while (!MAP_OSAL_MSG_Q_EMPTY(&macData.txQueue))
  {
    uint8 *ptr = MAP_osal_msg_dequeue(&macData.txQueue);
    MAP_mac_msg_deallocate(&ptr);
  }

  /* clear rx queue */
  while (!MAP_OSAL_MSG_Q_EMPTY(&macData.rxQueue))
  {
    uint8 *ptr = MAP_osal_msg_dequeue(&macData.rxQueue);
    MAP_mac_msg_deallocate(&ptr);
  }

  /* initialize variables */
  MAP_OSAL_MSG_Q_INIT(&macData.txQueue);
  MAP_OSAL_MSG_Q_INIT(&macData.rxQueue);
  macData.indirectCount = 0;
  macData.directCount = 0;
  macData.rxCount = 0;
}

#if !defined(TIMAC_ROM_PATCH)
/**************************************************************************************************
 * @fn          macAllocTxBuffer
 *
 * @brief       This function allocates an OSAL message buffer to build a transmission frame.
 *
 * input parameters
 *
 * @param       cmd - Command frame identifier.
 * @param       *sec - pointer to MAC security parameters
 *
 * output parameters
 *
 * None.
 *
 * @return      Pointer to allocated buffer or NULL if allocation failed.
 **************************************************************************************************
 */

MAC_INTERNAL_API macTx_t *macAllocTxBuffer(uint8 cmd, ApiMac_sec_t *sec)
{
  macTx_t       *pMsg;
  uint8         len;
  uint8         micLen;

  /* we have table of cmd frame size and sizeof(msg) indexed by cmd */
  len = macCmdBufSize[cmd];
#ifdef FEATURE_FREQ_HOP_MODE
  if ( MAC_FH_ENABLED && MAC_FH_GET_IE_LEN_FN )
  {
    uint16 ieLen = 0;
    /* buffer space for FH header IE's if any for this command */
    ieLen = MAC_FH_GET_IE_LEN_FN( macCmdIncludeFHIe[cmd] & MAC_FH_HEADER_IE_MASK);
    /* buffer space for FH Payload IE's if any for this command */
    ieLen += MAC_FH_GET_IE_LEN_FN( macCmdIncludeFHIe[cmd] & MAC_FH_PAYLOAD_IE_MASK);

    if ( ieLen )
    {
      len += ieLen + MAC_HEADER_IE_HEADER_LEN + MAC_PAYLOAD_IE_HEADER_LEN;
    }
  }
#endif

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif
      if (pMacPib->securityEnabled == TRUE && sec->securityLevel > MAC_SEC_LEVEL_NONE)
      {
        /* Save local copy */
        micLen = macAuthTagLen[sec->securityLevel];

        /* Add security header length as data offset.
         * Do not add MIC length as it is not part of security header.
         */
        len += (MAC_SEC_CONTROL_FIELD_LEN + MAC_FRAME_COUNTER_LEN + macKeySourceLen[sec->keyIdMode]);
      }
      else
      {
        micLen = 0;
      }
#if (defined TIMAC_ROM_IMAGE_BUILD)
    }
    else
    {
        micLen = 0;
    }
#endif
#else
  (void)sec;
  micLen = 0;
#endif /* FEATURE_MAC_SECURITY */

  /* allocation includes micLen including MAC_PHY_PHR_LEN.
   * Low level MAC will fill in PHR.
   */
#ifndef ENHACK_MALLOC
  if(cmd == MAC_INTERNAL_ENH_ACK)
  {
    if((MAC_PHY_PHR_LEN + len + micLen) > 250)
    {
      MAC_ASSERT(0);
    }
    else
    {
      pMsg = (macTx_t *)enhancedAckBuf;
    }
  }
  else
  {
    pMsg = (macTx_t *) MAP_osal_msg_allocate(MAC_PHY_PHR_LEN + len + micLen);
  }
#else
    pMsg = (macTx_t *) MAP_osal_msg_allocate(MAC_PHY_PHR_LEN + len + micLen);
#endif
  if (pMsg)
  {
    MAP_osal_memset( pMsg, 0x0, MAC_PHY_PHR_LEN + len + micLen);

    /* Initialize to zero the internal data structure */
    MAP_osal_memset(&(pMsg->internal), 0x0, sizeof(macTxIntData_t));

    /* Initialize the headerIeInfo and PayloadIeInfo */
    MAP_osal_memset( &(pMsg->internal.headerIeInfo), MAC_HEADER_IE_TERM,
                 sizeof(headerIeInfo_t) * MAC_HEADER_IE_MAX );
    MAP_osal_memset( &(pMsg->internal.payloadIeInfo), MAC_PAYLOAD_IE_TERM_GID,
                 sizeof(payloadIeInfo_t) * MAC_PAYLOAD_IE_MAX );

    /* set up data pointer to end of message (exclude MIC); to be built in reverse */
    pMsg->msdu.p = (uint8 *) pMsg + len;
    pMsg->msdu.len = micLen;

    /* set command identifier */
    if ((cmd > MAC_INTERNAL_DATA) && (cmd < MAC_INTERNAL_COEXIST_EB_REQ))
    {
      *(--pMsg->msdu.p) = cmd - 1;
      pMsg->msdu.len++;
      len--;
    }

    pMsg->internal.totalLen = len;

    return pMsg;
  }
  /* alloc failed; try to alloc a smaller buffer for failure message */
  else
  {
    pMsg = (macTx_t *) MAP_osal_msg_allocate(sizeof(macEventHdr_t));
    if (pMsg)
    {
      /* treat as tx failure; send osal msg to MAC */
      pMsg->hdr.event = macTxFrameFailed[cmd];
      pMsg->hdr.status = MAC_NO_RESOURCES;
      MAP_osal_msg_send(macTaskId, (uint8 *) pMsg);
    }
    return NULL;
  }
}
#endif

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
/**************************************************************************************************
 * @fn          macFrameDurationSubG
 *
 * @brief       This function calculates the duration of a transmitted data frame in symbols.
 *              The duration includes the acknowledgement frame (if ack required) and the
 *              interframe spacing.
 *
 * input parameters
 *
 * @param       len - frame length
 * @param       txOptions - TX options
 *
 * output parameters
 *
 * None.
 *
 * @return      Duration of a transmitted data frame in symbols.
 **************************************************************************************************
 */
uint16 macFrameDurationSubG(uint16 len, uint16 txOptions)
{
    uint16 totalSymbols;
    uint16 totalByteCount;
    bool fcsType, fcsLen;

    MAP_MAC_MlmeGetReq(MAC_FCS_TYPE, &fcsType);
    fcsLen = MAC_FCS2_FIELD_LEN;
    if(!fcsType)
    {
      fcsLen = MAC_FCS4_FIELD_LEN;
    }
    if (MAP_macCheckPhyRate(pMacPib->curPhyID) == PHY_MODE_SLR_5K)
    {
      /* number of bytes in frame including preamble, SFD, length and FCS */
      totalSymbols = MAC_PHY_SHR_LEN_2FSK_LRM +
                     (MAC_PHY_PHR_LEN + len + fcsLen) * MAC_SYMBOLS_PER_OCTET_2FSK_LRM +
                     MAC_PHY_TERM_LEN_LRM;

      /* if acknowledge requested */
      if (txOptions & MAC_TXOPTION_ACK)
      {
        /* Add turnaround time before the ack */
        totalSymbols += MAC_A_TURNAROUND_TIME;

        /* add in the bytes in the acknowledgement frame */
        totalSymbols += MAC_PHY_SHR_LEN_2FSK_LRM +
                        (MAC_PHY_PHR_LEN + MAC_LEN_ACK_FRAME) * MAC_SYMBOLS_PER_OCTET_2FSK_LRM +
                        MAC_PHY_TERM_LEN_LRM;
      }

      /* add interframe spacing */
      totalSymbols += (len > MAC_A_MAX_SIFS_FRAME_SIZE) ?
                      (MAC_A_MIN_LIFS_PERIOD) : (MAC_A_MIN_SIFS_PERIOD);

      /* convert bytes to symbols */
      return (totalSymbols);
    }
    else
    {
      /* number of bytes in frame including preamble, SFD, length and FCS */
      totalByteCount = MAC_PHY_SHR_LEN_2FSK + MAC_PHY_PHR_LEN + len + fcsLen ;

      /* if acknowledge requested */
      if (txOptions & MAC_TXOPTION_ACK)
      {
        /* Add turnaround time before the ack */
        totalByteCount += (MAC_A_TURNAROUND_TIME / MAC_SYMBOLS_PER_OCTET_2FSK);

        /* add in the bytes in the acknowledgment frame */
        totalByteCount += MAC_PHY_SHR_LEN_2FSK + MAC_PHY_PHR_LEN + MAC_LEN_ACK_FRAME;
      }
      /* add interframe spacing */
      totalByteCount += (len > MAC_A_MAX_SIFS_FRAME_SIZE) ?
                        ((MAC_A_MIN_LIFS_PERIOD + (MAC_A_MIN_LIFS_PERIOD % MAC_SYMBOLS_PER_OCTET_2FSK)) / MAC_SYMBOLS_PER_OCTET_2FSK) :
                        ((MAC_A_MIN_SIFS_PERIOD + (MAC_A_MIN_SIFS_PERIOD % MAC_SYMBOLS_PER_OCTET_2FSK)) / MAC_SYMBOLS_PER_OCTET_2FSK);

      /* convert bytes to symbols */
      return (totalByteCount * MAC_SYMBOLS_PER_OCTET_2FSK);
    }
}
#endif

/**************************************************************************************************
 * @fn          macFrameDuration
 *
 * @brief       This function calculates the duration of a transmitted data frame in symbols.
 *              The duration includes the acknowledgement frame (if ack required) and the
 *              interframe spacing.
 *
 * input parameters
 *
 * @param       len - frame length
 * @param       txOptions - TX options
 *
 * output parameters
 *
 * None.
 *
 * @return      Duration of a transmitted data frame in symbols.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint16 macFrameDuration(uint16 len, uint16 txOptions)
{
  SELECT_RCALL(MAP_macFrameDuration, len, txOptions);
}
/**************************************************************************************************
 * @fn          macDataRxMemAlloc
 *
 * @brief       This function is called by the low level MAC to allocate a received frame.
 *
 * input parameters
 *
 * @param       len - Length of received message.
 *              dataFrame - Indicates message is for a data frame.
 *
 * output parameters
 *
 * None.
 *
 * @return      ZSUCCESS, INVALID_MSG_POINTER.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 *macDataRxMemAlloc(uint16 len)
{
  if (macData.rxCount < macCfg.rxMax)
  {
    return MAP_osal_msg_allocate(len);
  }
#ifdef DBG_OSAL
  else
  {
      osalDbg.pk_drop_no_rx_buffer++;
  }
#endif
  return NULL;
}

/**************************************************************************************************
 * @fn          macDataRxMemFree
 *
 * @brief       This function is called by the low level MAC to deallocate a received frame
 *              that is not passed to macRxCompleteCallback.
 *
 * input parameters
 *
 * @param       pMsg - Pointer-pointer to the received message.
 *
 * output parameters
 *
 * @param       pMsg - Pointer is set to NULL after memory is freed.
 *
 * @return      MAC_SUCCESS
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macDataRxMemFree(uint8 **pMsg)
{
  MAP_mac_msg_deallocate(pMsg);
  return MAC_SUCCESS;
}

/**************************************************************************************************
 * @fn          macRxCompleteCallback
 *
 * @brief       This function is called by the low level MAC to process a received frame.  Except
 *              for a few special cases, this function looks at the frame type, sets the
 *              corresponding MAC event in hdr.event and sends an OSAL message to the MAC.  If
 *              a beacon frame is received and macStateScanning() returns FALSE,
 *              macRxBeaconCritical() is called.  If a data frame is received and
 *              macStateIdle() returns TRUE, the MAC callback function is executed directly
 *              with event MAC_MCPS_DATA_IND.
 *
 * input parameters
 *
 * @param       pMsg - Pointer received message.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxCompleteCallback(macRx_t *pMsg)
{
  halIntState_t intState;
  uint8_t frameType = pMsg->internal.fcf & MAC_FCF_FRAME_TYPE_MASK;

#ifdef FEATURE_FREQ_HOP_MODE
  if(MAC_FH_ENABLED)
  {
    /* All data frames and command frames with Id < MAC_DATA_REQ_FRAME are only
       allowed in FH mode */
    if((pMsg->internal.fhFrameType == MAC_FH_FRAME_INVALID) ||
       ((frameType != MAC_FRAME_TYPE_COMMAND) &&
        (frameType != MAC_FRAME_TYPE_DATA)) ||
       ((frameType == MAC_FRAME_TYPE_COMMAND) &&
        ((*pMsg->msdu.p) > MAC_DATA_REQ_FRAME)))
    {
      /* Dropping All unsupported frame types in FH mode */
      MAP_mac_msg_deallocate((uint8 **)&pMsg);
      return;
    }
  }
#endif

  switch (frameType)
  {
    case MAC_FRAME_TYPE_DATA:
      pMsg->hdr.event = MAC_RX_DATA_IND_EVT;
      break;
    default:
      /* discard invalid command frame */
      MAP_mac_msg_deallocate((uint8 **)&pMsg);
      return;
      /* no break required here */
  }

  /* enqueue frame in rx queue and send event to MAC */
  MAP_osal_msg_enqueue(&macData.rxQueue, pMsg);
  HAL_ENTER_CRITICAL_SECTION(intState);
  macData.rxCount++;
  HAL_EXIT_CRITICAL_SECTION(intState);
  MAP_macSetEvent(MAC_RX_QUEUE_TASK_EVT);
}

/**************************************************************************************************
 * @fn          macDataTxTimeAvailable
 *
 * @brief       This function is called by low level to determine the number of symbols
 *              remaining for a transmission to begin.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      The number of symbols remaining for a transmission to begin.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint16 macDataTxTimeAvailable(void)
{
  if (macDataCheckTxTime)
  {
    return (*macDataCheckTxTime)();
  }
  else
  {
    return 0;
  }
}

#if !defined(TIMAC_ROM_PATCH)
/**************************************************************************************************
 * @fn          macBuildDataFrame
 *
 * @brief       This function builds and sends a data frame in buffer pEvent.
 *
 * input parameters
 *
 * @param       pEvent - Pointer to event data.
 *
 * output parameters
 *
 * None.
 *
 * @return      status.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macBuildDataFrame(macEvent_t *pEvent)
{
  uint8 status = MAC_SUCCESS;

  if (MAC_FH_ENABLED)
  {
    /* set parameters */

    if (pEvent->dataReq.mac.dstAddr.addrMode == SADDR_MODE_NONE)
    {
      pEvent->tx.internal.frameType  = MAC_INTERNAL_DATA_BROADCAST;
      pEvent->tx.internal.be = (pMacDataTx->internal.txOptions & MAC_TXOPTION_ALT_BE) ? pMacPib->altBe : pMacPib->minBe;
      pEvent->tx.internal.be += 2;
    }
    else
    {
      pEvent->tx.internal.frameType  = MAC_INTERNAL_DATA_UNICAST;
      pEvent->tx.internal.be = (pMacDataTx->internal.txOptions & MAC_TXOPTION_ALT_BE) ? pMacPib->altBe : pMacPib->minBe;
    }

    pEvent->tx.internal.fhFrameType  = MAC_FH_FRAME_DATA;
    pEvent->tx.internal.nb = 0;
    MAP_osal_memcpy(&pEvent->tx.internal.dest.dstAddr, &pEvent->dataReq.mac.dstAddr, sizeof(sAddr_t));
    //MAP_osal_memcpy(pEvent->tx.internal.dest.destAddr, pEvent->dataReq.mac.dstAddr.addr.extAddr, SADDR_EXT_LEN);
  }
  else
  {
    pEvent->tx.internal.frameType  = MAC_INTERNAL_DATA;
  }

  pEvent->tx.internal.txOptions  = pEvent->dataReq.mac.txOptions;
  pEvent->tx.internal.channel    = pEvent->dataReq.mac.channel;
  pEvent->tx.internal.power      = pEvent->dataReq.mac.power;

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif
      /* If there is no IE, only the legacy payload will be secured. */
      pEvent->tx.internal.ptrMData = pEvent->tx.msdu.p;
#if (defined TIMAC_ROM_IMAGE_BUILD)
    }
#endif
#endif

  if ( MAC_PAYLOAD_IE_TERM_GID != pEvent->tx.internal.payloadIeInfo[0].gId )
  {
    bool includeTermIe = (0 == pEvent->dataReq.mac.payloadIELen)?TRUE:FALSE;
    /* Builds MAC specific payload IE's if any and also adds the payload termination
       IE if requested */
    status = MAP_macBuildPayloadIes(&pEvent->tx, pEvent->tx.internal.payloadIeInfo,
                                includeTermIe, FALSE);
  }

  if (status != MAC_SUCCESS)
  {
    return status;
  }

  if ( (MAC_HEADER_IE_TERM != pEvent->tx.internal.headerIeInfo[0].elementId) ||
       (NULL != pEvent->tx.internal.ptrPayloadIE) )
  {
    /* Builds the MAC specific header IE's if any and also adds the header
       termination IE */
    status = MAP_macBuildHeaderIes(&pEvent->tx, pEvent->tx.internal.headerIeInfo);
  }

  if (status != MAC_SUCCESS)
  {
    return status;
  }

  /* build header */
  status = MAP_macBuildHeader(&pEvent->tx, pEvent->dataReq.mac.srcAddrMode,
                          &pEvent->dataReq.mac.dstAddr, pEvent->dataReq.mac.dstPanId, 0);

  if (status == MAC_SUCCESS)
  {
    /* check for the maximum length
     * if the msdu.len is greater than MAX length return error
     *
     */
    if (pEvent->tx.msdu.len > macCfg.macMaxFrameSize)
    { /* TX data packet is too big, just drop */
      macTx_t *pMsg;

      pMsg = &pEvent->tx;
      pMsg->hdr.status = MAC_FRAME_TOO_LONG;
      MAP_macDataTxComplete(pMsg);
    }
    else
    {
      DBG_PRINT0(DBGSYS, "Data Enqueued");

      /* queue frame for transmission */
      MAP_macDataTxEnqueue(&pEvent->tx);
    }
  }
  return status;
}
#endif //#if !defined(TIMAC_ROM_PATCH)

/**************************************************************************************************
 * @fn          macCheckSched
 *
 * @brief       This function checks current frame scheduling for non-beacon networks.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      Curent frame schedule
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macCheckSched(void)
{
  /* if scanning only send scan related frames */
  if (MAP_macStateScanning())
  {
    return MAC_TX_SCHED_SCAN_RELATED;
  }
  else
  {
    return MAC_TX_SCHED_OUTGOING_CAP;
  }
}

/**************************************************************************************************
 * @fn          macSetSched
 *
 * @brief       This function sets the scheduled transmission period of the given frame to the
 *              outgoing CAP.
 *
 * input parameters
 *
 * @param       pMsg - pointer to TX message.
 *
 * output parameters
 *
 * @param       pMsg->internal.txSched may be set by this function.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macSetSched(macTx_t *pMsg)
{
  pMsg->internal.txSched |= MAC_TX_SCHED_OUTGOING_CAP;
}

#if !defined(TIMAC_ROM_PATCH)
/**************************************************************************************************
 * @fn          macDataTxComplete
 *
 * @brief       This function is called when a data transmission attempt is complete.  If this
 *              is for a non-internal data frame the MAC callback is executed directly.
 *              Otherwise the appropriate TX event is sent as an OSAL message to the MAC.
 *
 * input parameters
 *
 * @param       pMsg - Pointer message that was transmitted.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macDataTxComplete(macTx_t *pMsg)
{
  /* if data frame call callback directly; this must be done so that the application
   * can free the buffer
   */
  if (((pMsg->internal.frameType == MAC_INTERNAL_DATA) ||
       (pMsg->internal.frameType == MAC_INTERNAL_DATA_UNICAST) ||
       (pMsg->internal.frameType == MAC_INTERNAL_DATA_BROADCAST) ||
       (pMsg->internal.frameType == MAC_INTERNAL_ASYNC) ) &&
      !(pMsg->internal.txMode & MAC_TX_MODE_INTERNAL))
  {
    if(pMacPib->enDataAckPending)
    {
      if ( pMsg->hdr.status == MAC_ACK_PENDING)
      {
        /* turn off receiver */
        MAP_macRxSoftEnable(MAC_RX_POLL);
        /*set frame response timer to wait for the pending data */
        MAP_macSetEvent16(MAC_TX_DATA_REQ_TASK_EVT);
      }
      pMsg->hdr.status = MAC_SUCCESS;
    }
    MAP_macCbackForEvent((macEvent_t *) pMsg, pMsg->hdr.status);
    return;
  }
  /* check for beacon frame tx in beacon network */
  else if (pMsg->internal.txMode & MAC_TX_MODE_BEACON)
  {
      if (macDataTxBeaconComplete)
      {
          (*macDataTxBeaconComplete)(pMsg->hdr.status);
      }
    /* return immediately; don't send event to mac */
    return;
  }
  /* no action for internal data frames */
  else if (((pMsg->internal.frameType == MAC_INTERNAL_DATA) ||
      (pMsg->internal.frameType == MAC_INTERNAL_DATA_UNICAST)) &&
          (pMsg->internal.txMode & MAC_TX_MODE_INTERNAL))
  {
    pMsg->hdr.event = MAC_NO_ACTION_EVT;
  }
  /* deal with ack w. pending for data req */
  else if (pMsg->hdr.status == MAC_ACK_PENDING)
  {
    pMsg->hdr.event = MAC_TX_DATA_REQ_PEND_EVT;
  }
  else if (pMsg->hdr.status == MAC_SUCCESS)
  {
    pMsg->hdr.event = macTxFrameSuccess[pMsg->internal.frameType];
  }
  else
  {
    pMsg->hdr.event = macTxFrameFailed[pMsg->internal.frameType];
  }

  /* send event to mac */
  MAP_osal_msg_send(macTaskId, (uint8 *) pMsg);
}
#endif

/**************************************************************************************************
 * @fn          macTxCompleteCallback
 *
 * @brief       This function is called by low level when a data transmission is complete.
 *
 * input parameters
 *
 * @param       status - The MAC status for the transmission.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTxCompleteCallback(uint8 status)
{
  uint8 txType, fhReTxCond = 0, baseBackoff;
  macPHYDesc_t *pPhyDesc;

  if(MAC_FH_ENABLED && MAC_FH_GET_PIB_FN && MAC_FH_GET_TX_CH_HOP_FN)
  {
    MAC_FH_GET_PIB_FN(FHPIB_CSMA_BASE_BACKOFF, &baseBackoff);
    fhReTxCond = (MAC_FH_GET_TX_CH_HOP_FN()
           && baseBackoff && (status == MAC_CHANNEL_ACCESS_FAILURE));
  }
  /* Get current PHY Descriptor*/
  pPhyDesc = MAP_macMRFSKGetPhyDesc(pMacPib->curPhyID);

  txType = (pPhyDesc->ccaType != MAC_CCA_TYPE_LBT) ?
           MAC_TX_TYPE_UNSLOTTED_CSMA : MAC_TX_TYPE_LBT;

  if (pMacDataTx != NULL)
  {
    /* status is success, frame is complete */
    if (status == MAC_SUCCESS)
    {
      pMacDataTx->hdr.status = MAC_SUCCESS;
      MAP_macDataTxComplete(pMacDataTx);
    }
    /* ack w. pending for non data req frame same as success */
    else if ((status == MAC_ACK_PENDING) &&
             (pMacDataTx->internal.frameType != MAC_INTERNAL_DATA_REQ) &&
             (!pMacPib->enDataAckPending))
    {
            pMacDataTx->hdr.status = MAC_SUCCESS;
            MAP_macDataTxComplete(pMacDataTx);
    }
    /* if no ack handle possible retransmission */
    else if ((status == MAC_NO_ACK) || (fhReTxCond))
    {
      /* Increment diagnostic TX failure counter */
      MAP_macLowLevelDiags( MAC_DIAGS_TX_UCAST_FAIL );

      if (pMacDataTx->internal.retries > 0)
      {
        /* Reverse bit order in each byte after the header and before the FCS */
        MAP_macMcuBitReverse(pMacDataTx->msdu.p, pMacDataTx->msdu.len);

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
        if (macPib.securityEnabled == TRUE)
        {
#endif
          /* Check if security is enabled and FH is enabled */
          if ((*((uint16 *) pMacDataTx->msdu.p) & MAC_FCF_SEC_ENABLED_MASK) &&
              MAC_FH_ENABLED)
          {
            /* Decrypt the frame as it will be re-encrypted by LMAC */
            MAP_macTxDecrypt();
          }

#if (defined TIMAC_ROM_IMAGE_BUILD)
        }
#endif
#endif

        /* if direct frame, transmit immediately if retries remain */
        if (!(pMacDataTx->internal.txOptions & MAC_TXOPTION_INDIRECT))
        {
          /* only direct frames use retries */
          pMacDataTx->internal.retries--;

          MAP_macTxFrameRetransmit();

          /* return immediately */
          return;
        }
        /* else if indirect frame, clear requested flag and push
        * frame back on to the tx queue
        */
        else
        {
          pMacDataTx->internal.txMode &= ~MAC_TX_MODE_REQUESTED;
          if (pMacDataTx->internal.frameType == MAC_INTERNAL_ASSOC_RSP)
          {
            /* if ACK is not received for association response,
             * mark NO ACK flag then push frame back on to the tx queue
             */
            pMacDataTx->internal.txSched |= MAC_TX_SCHED_INDIRECT_NO_ACK;
          }

          /* Increment diagnostic retry counter */
          MAP_macLowLevelDiags( MAC_DIAGS_TX_UCAST_RETRY );

          (*macDataRequeueIndirect)(pMacDataTx);
        }
      }
      else /* else tx is complete */
      {
        if (MAP_MAC_CbackQueryRetransmit() &&
            !(pMacDataTx->internal.txOptions & MAC_TXOPTION_INDIRECT))
        {
          /* RF4CE feature to retransmit till directed to stop.
           * MAC_CbackQuery might have changed channel as well.
           */
          pMacDataTx->internal.retries = pMacPib->maxFrameRetries;
          MAC_SEQ_NUMBER(pMacDataTx->msdu.p) = pMacPib->dsn++;
          MAP_macTxFrame(txType);
          return;
        }

        pMacDataTx->hdr.status = status;
        MAP_macDataTxComplete(pMacDataTx);
      }
    }

    /* For RF4CE - channel access failure should also query retransmit */
    else if ( (status == MAC_CHANNEL_ACCESS_FAILURE) &&
               MAP_MAC_CbackQueryRetransmit()            &&
               !(pMacDataTx->internal.txOptions & MAC_TXOPTION_INDIRECT)
            )
    {
      /* retransmit on another channel requested */
      pMacDataTx->internal.retries = pMacPib->maxFrameRetries;
      MAC_SEQ_NUMBER(pMacDataTx->msdu.p) = pMacPib->dsn++;
      DBG_PRINT0(DBGSYS, "MAP_macTxFrame(2)");
      MAP_macTxFrame(txType);
      return;
    }
    /* else tx is complete */
    else
    {
      pMacDataTx->hdr.status = status;
      MAP_macDataTxComplete(pMacDataTx);
    }

    /* Proprietary - swiching back to the default power and channel */
    if (pMacDataTx->internal.txOptions & MAC_TXOPTION_PWR_CHAN)
    {
      MAP_macRadioSetTxPower(pMacPib->phyTransmitPower);
    }

    /* clear current frame */
    pMacDataTx = NULL;

    /* set up next data transmission */
    MAP_macSetEvent(MAC_TX_COMPLETE_TASK_EVT);
  }
}

/**************************************************************************************************
 * @fn          macDataTxSend
 *
 * @brief       This function services the TX data queue and sends data.  First it checks if
 *              pMacDataTx is NULL.  If it is not it returns as a transmission is in
 *              progress.  Next it determines the current period and searches the queue for
 *              the next frame for the current period.  If found, it calls macTxFrame()
 *              to transmit the frame.
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
MAC_INTERNAL_API void macDataTxSend(void)
{
  macTx_t           *pCurr;
  macTx_t           *pPrev=NULL;
  uint8             schedMask = MAC_TX_SCHED_READY;
  halIntState_t     intState;
  macPHYDesc_t *pPhyDesc;

  HAL_ENTER_CRITICAL_SECTION(intState);

  /* if transmission in progress return */
  if ((pMacDataTx != NULL))
  {
    HAL_EXIT_CRITICAL_SECTION(intState);
    return;
  }

  /* determine current schedule */
  schedMask |= (*macDataCheckSched)();

  /* find next frame to transmit */
  pCurr = MAP_OSAL_MSG_Q_HEAD(&macData.txQueue);
  while (pCurr != NULL)
  {
    if ((pCurr->internal.txSched & schedMask) == schedMask)
    {
      /* take frame out of queue */
      MAP_osal_msg_extract(&macData.txQueue, pCurr, pPrev);
      break;
    }
    pPrev = pCurr;
    pCurr = MAP_OSAL_MSG_NEXT(pCurr);
  }

  if (pCurr == NULL)
  {
    HAL_EXIT_CRITICAL_SECTION(intState);
    return;
  }

  /* initialize pointer for tx */
  pMacDataTx = pCurr;

  /* decrement frame count */
  if (pMacDataTx->internal.txSched & MAC_TX_SCHED_INDIRECT)
  {
    macData.indirectCount--;
  }
  else
  {
    macData.directCount--;
  }

  HAL_EXIT_CRITICAL_SECTION(intState);

  macData.duration = MAP_macFrameDuration(pMacDataTx->msdu.len, pMacDataTx->internal.txOptions);

  /* Get current PHY Descriptor*/
  pPhyDesc = MAP_macMRFSKGetPhyDesc(pMacPib->curPhyID);

  /* Proprietary - switching power and channel per data request */
  if (pMacDataTx->internal.txOptions & MAC_TXOPTION_PWR_CHAN)
  {
    MAP_macRadioSetChannel(pMacDataTx->internal.channel);
    MAP_macRadioSetTxPower(pMacDataTx->internal.power);
  }
  if (pPhyDesc->ccaType == MAC_CCA_TYPE_CSMA_CA)
  {
    /* initiate low level transmission for 915MHz band */
    MAP_macTxFrame((pMacDataTx->internal.txMode & MAC_TX_MODE_SLOTTED) ?
               MAC_TX_TYPE_SLOTTED_CSMA : MAC_TX_TYPE_UNSLOTTED_CSMA);
  }
  else if (pPhyDesc->ccaType == MAC_CCA_TYPE_LBT)
  {
    /* initiate low level transmission for 863MHz band */
    MAP_macTxFrame(MAC_TX_TYPE_LBT);
  }
}

/**************************************************************************************************
 * @fn          macDataTxEnqueue
 *
 * @brief       This function queues a frame in the TX data queue and then services the queue to
 *              send data.  If the frame is a data frame and the queue length is greater than the
 *              data frame queue maximum, the frame status is set to MAC_TRANSACTION_OVERFLOW and
 *              the transmission completes with a failure.  Otherwise the frame is queued in
 *              the TX data queue.  If the frame is an indirect frame the indirect frame count is
 *              incremented.  If the frame is direct macDataSetSched() is called to set the
 *              frame's scheduled transmission period and macDataTxSend() is called to service the
 *              queue and send data.
 *
 * input parameters
 *
 * @param       pMsg - Pointer message to transmit.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macDataTxEnqueue(macTx_t *pMsg)
{
  uint8         max;
  halIntState_t intState;

  /* mark buffer as reserved */
  MAP_macMainReserve((uint8 *)pMsg);

  /* if frame is a data frame and not internal */
  if (!(pMsg->internal.txMode & MAC_TX_MODE_INTERNAL) &&
      (pMsg->internal.frameType == MAC_INTERNAL_DATA))
  {
    max = macCfg.txDataMax;
  }
  /* else treat as command frame */
  else
  {
    max = macCfg.txMax;
  }

  /* if data can be queued */
  if (MAP_osal_msg_enqueue_max(&macData.txQueue, pMsg, max))
  {
    /* schedule frame */
    (*macDataSetSched)(pMsg);

    if (!(pMsg->internal.txOptions & MAC_TXOPTION_INDIRECT))
    {
      /* service data queue and send data */
      HAL_ENTER_CRITICAL_SECTION(intState);
      macData.directCount++;
      HAL_EXIT_CRITICAL_SECTION(intState);

      if ((MAC_FH_ENABLED) && (MAC_FH_SEND_DATA_FN))
      {
        MAC_FH_SEND_DATA_FN();
      }
    }
    else
    {
      /* handle indirect data */
      (*macDataTxIndirect)(pMsg);
    }
  }
  /* else queue is at max, transaction overflow */
  else
  {
    pMsg->hdr.status = MAC_TRANSACTION_OVERFLOW;
    MAP_macDataTxComplete(pMsg);
  }
}

/**************************************************************************************************
 * @fn          macDataSend
 *
 * @brief       This action function is called to service the tx data queue and send any
 *              queued data which is ready to be sent.
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
MAC_INTERNAL_API void macDataSend(macEvent_t *pEvent)
{
  (void)pEvent;  // Intentionally unreferenced parameter

  if ((MAC_FH_ENABLED) && (MAC_FH_SEND_DATA_FN))
  {
    MAC_FH_SEND_DATA_FN();
  }
}

#if !defined(TIMAC_ROM_PATCH)
/**************************************************************************************************
 * @fn          macApiDataReq
 *
 * @brief       This function handles an API data request event.  First the parameters are checked.
 *              If any parameters are invalid it sets hdr.status to MAC_INVALID_PARAMETER
 *              and calls the MAC callback function directly.  If ok, it calls
 *              macBuildDataFrame() to build and send a data frame.
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
MAC_INTERNAL_API void macApiDataReq(macEvent_t *pEvent)
{
  uint8   status = MAC_SUCCESS;

  /* check parameters */
  MAC_PARAM_STATUS((macMain.featureMask & MAC_FEAT_COORD_MASK) ||
                   ((pEvent->dataReq.mac.txOptions & MAC_TXOPTION_INDIRECT) == 0), status);
  MAC_PARAM_STATUS((macMain.featureMask & MAC_FEAT_GTS_MASK) ||
                   ((pEvent->dataReq.mac.txOptions & MAC_TXOPTION_GTS) == 0), status);
  MAC_PARAM_STATUS(pEvent->dataReq.msdu.len <= macCfg.macMaxFrameSize, status);

  if ((pEvent->dataReq.mac.srcAddrMode == SADDR_MODE_NONE) &&
      (pEvent->dataReq.mac.dstAddr.addrMode == SADDR_MODE_NONE))
  {
    status = MAC_INVALID_ADDRESS;
  }

  if (status == MAC_SUCCESS)
  {
    if (pEvent->dataReq.mac.includeFhIEs && (!MAC_FH_ENABLED) )
    {
      status = MAC_UNSUPPORTED;
    }
    else if ( MAC_FH_ENABLED )
    {

      if((pEvent->dataReq.mac.dstAddr.addrMode == SADDR_MODE_NONE) ||
         (pEvent->dataReq.mac.dstAddr.addrMode == SADDR_MODE_EXT))

      {
        /* FH UT IE + FH BT IE*/
        pEvent->dataReq.mac.includeFhIEs |= (MAC_FH_UT_IE | MAC_FH_BT_IE);
      }

      if ( pEvent->dataReq.mac.includeFhIEs & MAC_FH_HEADER_IE_MASK )
      {
        pEvent->dataReq.internal.headerIeInfo[0].elementId = MAC_HEADER_IE_WISUN;
        pEvent->dataReq.internal.headerIeInfo[0].subIdBitMap =
          pEvent->dataReq.mac.includeFhIEs & MAC_FH_HEADER_IE_MASK;
      }

      if ((pEvent->dataReq.mac.includeFhIEs & (~MAC_FH_MPX_IE)) & MAC_FH_PAYLOAD_IE_MASK )
      {
        pEvent->dataReq.internal.payloadIeInfo[0].gId = MAC_PAYLOAD_IE_WISUN_GID;
        pEvent->dataReq.internal.payloadIeInfo[0].subId.subIdBMap =
          pEvent->dataReq.mac.includeFhIEs & MAC_FH_PAYLOAD_IE_MASK;
      }
    }

    if (status == MAC_SUCCESS)
    {
      status = MAP_macBuildDataFrame(pEvent);
    }
  }

  if (status != MAC_SUCCESS)
  {
    DBG_PRINT1(DBGSYS, "Failed Status= 0x%X", status);
    MAP_macCbackForEvent(pEvent, status);
  }
}

/**************************************************************************************************
 * @fn          macDataRxInd
 *
 * @brief       This action function is called to handle a received data indication.  It
 *              executes the MAC callback function with event MAC_MCPS_DATA_IND.
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
MAC_INTERNAL_API void macDataRxInd(macEvent_t *pEvent)
{
  DBG_PRINT0(DBGSYS, "macDataRxInd");

  if(pMacPib->enDataAckPending)
  {
      if(pEvent->rx.internal.flags & MAC_RX_FLAG_ACK_PENDING)
      {
          MAP_macIndirectSend(&pEvent->rx.mac.srcAddr, pEvent->rx.mac.srcPanId);
      }
  }
  if (pEvent->dataInd.internal.fhFrameType <= MAC_FH_FRAME_PAN_CONFIG_SOLICIT )
  {
    pEvent->hdr.event = MAC_MLME_WS_ASYNC_IND;
  }
  else
  {
    pEvent->hdr.event = MAC_MCPS_DATA_IND;
  }

  MAP_MAC_CbackEvent((macCbackEvent_t *) pEvent);
}

/**************************************************************************************************
 * @fn          MAC_McpsDataReq
 *
 * @brief       This function sends application data to the MAC for transmission in a MAC data
 *              frame.  The function sets the event to MAC_API_DATA_REQ_EVT and sends the
 *              parameters as an OSAL message to the MAC.  The message buffer containing the
 *              parameters has already been allocated by the application.
 *
 * input parameters
 *
 * @param       pData - Pointer to parameters structure.
 *
 * output parameters
 *
 * None.
 *
 * @return      MAC_SUCCESS if successful otherwise error status.
 *              MAC_NO_RESOURCES if unable to allocate memory.
 *              MAC_TRANSACTION_OVERFLOW if the tx queue has reached the limit.
 **************************************************************************************************
 */
extern uint16_t convertTxOptions(ApiMac_txOptions_t txOptions);
extern void copyApiMacAddrToMacAddr(sAddr_t *pDst, ApiMac_sAddr_t *pSrc);
uint8 MAC_McpsDataReq(ApiMac_mcpsDataReq_t* pData)
{
  macMcpsDataReq_t *pMsg;

  /* check if correct power value is selected */
  if (pData->txOptions.usePowerAndChannel == true)
  {
      if (MAP_macGetRadioTxPowerReg(pData->power) == MAC_RADIO_TX_POWER_INVALID)
      {
        return MAC_INVALID_PARAMETER;
      }
  }

  if (((FALSE == pData->txOptions.indirect) &&
       (macCfg.txDataMax > macData.directCount)) ||
      ((TRUE == pData->txOptions.indirect) &&
       (macCfg.txDataMax > macData.indirectCount)))
  {
    pMsg = MAP_MAC_McpsDataAlloc(pData->msdu.len, pData->sec.securityLevel,
                             pData->sec.keyIdMode,
                             pData->includeFhIEs,
                             pData->payloadIELen);
    if(pMsg != NULL)
    {
      /* Fill in the message content */
      pMsg->hdr.event = MAC_API_DATA_REQ_EVT;
      pMsg->hdr.status = MAC_SUCCESS;

      //pMsg->msdu.len = pData->msdu.len;
      MAP_osal_memcpy(pMsg->msdu.p, pData->msdu.p, pData->msdu.len);
      MAP_osal_memcpy(&(pMsg->sec), &(pData->sec), sizeof( ApiMac_sec_t));

      MAP_copyApiMacAddrToMacAddr(&(pMsg->mac.dstAddr), &(pData->dstAddr));
      pMsg->mac.dstPanId = pData->dstPanId;
      pMsg->mac.srcAddrMode = pData->srcAddrMode;
      pMsg->mac.msduHandle = pData->msduHandle;
      pMsg->mac.txOptions = MAP_convertTxOptions(pData->txOptions);
      pMsg->mac.channel = pData->channel;
      pMsg->mac.power = pData->power;
      pMsg->internal.gpDuration = pData->gpDuration;
      pMsg->mac.payloadIELen = pData->payloadIELen;

      if(pMsg->mac.payloadIELen)
      {
        MAP_osal_memcpy(pMsg->mac.pIEList, pData->pIEList, pMsg->mac.payloadIELen);
      }

      pMsg->mac.fhProtoDispatch = pData->fhProtoDispatch;
      pMsg->mac.includeFhIEs = pData->includeFhIEs;

      if ( pMsg->mac.payloadIELen && (NULL != pMsg->mac.pIEList) )
      {
        /**
         * Fill the Payload termination IE.
         */
        *(--pMsg->msdu.p) = MAC_PAYLOAD_IE_TERM_SECOND_BYTE_PAYIE;
        *(--pMsg->msdu.p) = MAC_PAYLOAD_IE_TERM_FIRST_BYTE;

        /* application should have copied the payload ie's into buffer,
           move the msdu.p pointer appropriately and update the length field */
        pMsg->msdu.p -= pMsg->mac.payloadIELen;
        pMsg->msdu.len += MAC_PAYLOAD_IE_HEADER_LEN + pMsg->mac.payloadIELen;

        /* update the payload ie ptr */
        pMsg->internal.ptrPayloadIE = pMsg->msdu.p;
      }

      if (pMsg->mac.txOptions & MAC_TXOPTION_GREEN_PWR)
      {
      /* The GP offset must be greater than ~1.6ms. There can be no ACK requested and
       * must be a direct message.
       */
        if ((pData->gpOffset >= MAC_MIN_GREEN_PWR_DELAY) &&
          ((pMsg->mac.txOptions & MAC_TXOPTION_INDIRECT) == 0) &&
          ((pMsg->mac.txOptions & MAC_TXOPTION_ACK) == 0))
        {
          MAP_macTimerCancel(&macDataTxDelay);
          MAP_macTimer(&macDataTxDelay, pData->gpOffset);
        }
        else
        {
          return MAC_INVALID_PARAMETER;
        }
      }
      MAP_osal_msg_send(macTaskId, (uint8 *) pMsg);

      return MAC_SUCCESS;
    }
    else
    {
      return MAC_NO_RESOURCES;
    }
  }
  else
  {
    return MAC_TRANSACTION_OVERFLOW;
  }
}



/**
 * @brief       This direct-execute function simplifies the allocation and
 *              preparation of the data buffer MAC_McpsDataReq().  The
 *              function allocates a buffer and prepares the data pointer.
 *
 * @param       len - Length of the application data.
 * @param       securityLevel - security suire used for this frame.
 * @param       keyIdMode - Key Id mode used for this frame.
 *                  Possible values are:
 *                  MAC_KEY_ID_MODE_NONE        0x00
 *                  MAC_KEY_ID_MODE_IMPLICIT    0x00
 *                  MAC_KEY_ID_MODE_1           0x01
 *                  MAC_KEY_ID_MODE_4           0x02
 *                  MAC_KEY_ID_MODE_8           0x03
 *                  Set to MAC_KEY_ID_MODE_NONE if security is
 *                  not used.
 * @param       includeFhIEs - bitmap indicating which FH IE's
 *                           need to be included.
 * @param       payloadIeLen - length of the application payload IE's if any.
 *                             This function will allocate the buffer for the
 *                             payload IE's and set the data element pIEList
 *                             of the macDataReq_t in macMcpsDataReq_t
 *                             appropriately.
 *
 * @return      Returns a pointer to the allocated buffer. If
 *              the function fails for any reason it returns
 *              NULL.
 */
macMcpsDataReq_t *MAC_McpsDataAlloc(uint16 len, uint8 securityLevel, uint8 keyIdMode,
                                    uint32 includeFhIEs, uint16 payloadIeLen)
{
  macMcpsDataReq_t *pData;
  uint16 offset;
  uint8 micLen = 0;

  /* MAC_DATA_OFFSET is assuming the worst case */
  offset = sizeof(macMcpsDataReq_t) + MAC_DATA_OFFSET;

  if (includeFhIEs)
  {
    if (MAC_FH_ENABLED && MAC_FH_GET_IE_LEN_FN)
    {
      int16 ieLen = 0;

      /* allocate space for the freq hopping ie's */
      /* space for FH header ie's if any */
      ieLen = MAC_FH_GET_IE_LEN_FN( includeFhIEs & MAC_FH_HEADER_IE_MASK );

      /* space for FH payload ie's if any */
      ieLen += MAC_FH_GET_IE_LEN_FN( includeFhIEs & MAC_FH_PAYLOAD_IE_MASK );

      /* allocate length for termination header and Payload IE */
      ieLen += MAC_HEADER_IE_HEADER_LEN + MAC_PAYLOAD_IE_HEADER_LEN;

      offset += ieLen;
    }
    else
    {
      return NULL;
    }
  }

  /* if payload ies will be included allocate buffer for the termination ie's */
  if ( payloadIeLen )
  {
      payloadIeLen += 4;
  }

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif
      if (pMacPib->securityEnabled == TRUE && securityLevel > MAC_SEC_LEVEL_NONE)
      {
        /* Save local copy */
        micLen = macAuthTagLen[securityLevel];

        /* Add security header length as data offset.
         * Do not add MIC length as it is not part of security header.
         */
        offset += (MAC_SEC_CONTROL_FIELD_LEN + MAC_FRAME_COUNTER_LEN + macKeySourceLen[keyIdMode]);
      }

#if (defined TIMAC_ROM_IMAGE_BUILD)
    }
#endif
#else
  (void)keyIdMode;
  (void)securityLevel;
#endif /* FEATURE_MAC_SECURITY */

  /* Allocation including security header length, MIC length, and MAC_PHY_PHR_LEN.
   * Low level MAC will fill in PHR.
   */
  pData = (macMcpsDataReq_t *)
  MAP_osal_msg_allocate(MAC_PHY_PHR_LEN + len + payloadIeLen + offset + micLen);

  if (pData)
  {
    pData->msdu.len = len + micLen;
    pData->msdu.p = (uint8 *) pData + offset + payloadIeLen + MAC_PHY_PHR_LEN;

    if ( payloadIeLen )
    {
      pData->mac.pIEList = (uint8 *)pData + offset + MAC_PHY_PHR_LEN;
    }
    else
    {
      pData->mac.pIEList = NULL;
    }

    /* Initialize to zero the internal data structure */
    MAP_osal_memset(&(pData->internal), 0x0, sizeof(macTxIntData_t));

    pData->internal.totalLen = len + offset + payloadIeLen + MAC_PHY_PHR_LEN + micLen;

    /* Initialize the headerIeInfo and PayloadIeInfo */
    MAP_osal_memset( &(pData->internal.headerIeInfo), MAC_HEADER_IE_TERM,
                 sizeof(headerIeInfo_t) * MAC_HEADER_IE_MAX );
    MAP_osal_memset( &(pData->internal.payloadIeInfo), MAC_PAYLOAD_IE_TERM_GID,
                 sizeof(payloadIeInfo_t) * MAC_PAYLOAD_IE_MAX );

    return pData;
  }
  else
  {
    return NULL;
  }
}

#endif

