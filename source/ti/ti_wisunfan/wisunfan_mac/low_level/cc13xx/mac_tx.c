/******************************************************************************

 @file  mac_tx.c

 @brief Describe the purpose and contents of the file.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2006-2023, Texas Instruments Incorporated

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

/* hal */
#include "hal_types.h"
#include "hal_defs.h"
#include "hal_mcu.h"

/* high-level */
#include "mac_spec.h"
#include "mac_pib.h"
#include "fh_api.h"
#include "mac_security.h"
#include "mac_timer.h"
#include "mac_common_defines.h"

/* exported low-level */
#include "mac_settings.h"
#include "mac_low_level.h"
#include "mac.h"

/* low-level specific */
#include "mac_tx.h"
#include "mac_symbol_timer.h"
#include "mac_rx.h"
#include "mac_rx_onoff.h"
#include "mac_radio.h"
#include "mac_radio_tx.h"

/* target specific */
#include "mac_radio_defs.h"

/* debug */
#include "mac_assert.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_tx_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

#include "mac_mgmt.h"
#include "mac_data.h"
#include "fh_data.h"
#include "fh_util.h"
/******************************************************************************
  ROM jump table will support
  1. TIMAC ROM image build
  2. TIMAC stack build without ROM image
  3. TIMAC stack with calling ROM image
 *****************************************************************************/
#include "rom_jt_154.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
#define MFR_LEN                   MAC_FCS_FIELD_LEN
#define PREPENDED_BYTE_LEN        1
/* value for promiscuous off, must not conflict with other mode variants from separate include files */
#define PROMISCUOUS_MODE_OFF  0x00

/* This is passed from App */
#define WIFI_RX_ACTIVE            (((macUserCfg_t *)macRadioConfig)->rfWifiRxActive)

#ifdef FEATURE_WISUN_SUPPORT
#define IOT_MIN_BACKOFF_GUARD_DELAY  10 //10 ms min wait into bfio for IOT
#define IOT_MIN_BACKOFF_DWELL_TIME   50 //in ms. Min BC Dwell time to which above rule must be applied
#endif
/* ------------------------------------------------------------------------------------------------
 *                                         Global Constants
 * ------------------------------------------------------------------------------------------------
 */

/**
 * With different symbol rate, the symbol duration changes and
 * the beacon transmission calculation needs to take care of the
 * time changes and allow enough time for the radio to transmit
 * at the start of first slot boundary.
 * By default the beacon prep time is calculated for the
 * MAC_50KBPS_915MHZ_PHY_1 which is at 50K symbol rate.
 */

uint8 macTxSlottedDelayFactor = MAC_MRFSK_50_KSPS_BEACON_DELAY_FACTOR;

/* ------------------------------------------------------------------------------------------------
 *                                         Global Variables
 * ------------------------------------------------------------------------------------------------
 */
volatile uint8 macTxActive;
uint8 macTxType;
uint8 macTxBe;
uint16 macTxCsmaBackoffDelay;
uint8 macTxGpInterframeDelay;
uint8  txAckReq;
MAP_ICall_TimerID macCsmaRtcTimerId = 0;

/* ------------------------------------------------------------------------------------------------
 *                                         Local Variables
 * ------------------------------------------------------------------------------------------------
 */
/*npdb = number of backoffs taken to allow for preamble detection
 * in case Rssi is high and packet is not detected yet */
static uint8 nb, npdb = 0;
uint8 txSeqn;
volatile uint8 txRetransmitFlag;
uint8  txChannel;
#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
MAC_EDFE_DBG macEdfeDbg;
extern macEdfeInfo_t macEdfeInfo;
#endif

MAC_ACK_DBG macAckDbg;

#ifdef FH_BENCHMARK_BUILD_TIMES
uint16 macTx_AckTimeStamp[NUM_MAC_BENCHMARK_CNT];
uint8 ackTxIdx = 0;
uint32 ackStart_ts;
uint32_t dataBuildStart_ts;
#endif

/* ------------------------------------------------------------------------------------------------
 *                                         Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */
static void txCsmaPrep(void);
static void txGo(void);
static void txCsmaGo(void);
static MAC_INTERNAL_API void macTxRssiChannelBusyCallback(bool bForceComplete);

/**************************************************************************************************
 * @fn          macTxInit
 *
 * @brief       Initialize variables for tx module. OSAL must be initialized.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTxInit(void)
{
  macTxActive      = MAC_TX_ACTIVE_NO_ACTIVITY;
  txRetransmitFlag = 0;

  /* Power management state may change. Hence, vote. */
  MAC_PWR_VOTE_NO_SWI(FALSE);
}


/**************************************************************************************************
 * @fn          macTxFrame
 *
 * @brief       Transmit the frame pointed to by pMacDataTx with the specified type.
 *              NOTE! It is not legal to call this function from interrupt context.
 *
 * @param       txType - type of transmit
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTxFrame(uint8 txType)
{
  MAC_ASSERT(!macTxActive);            /* transmit on top of transmit */
  MAC_ASSERT(pMacDataTx != NULL);      /* must have data to transmit */

  /* mark transmit as active */
  macTxActive = MAC_TX_ACTIVE_INITIALIZE;
  /*
   *  The MAC will not enter sleep mode if there is an active transmit.  However, if macSleep() is
   *  ever called from interrupt context, it is possible to enter sleep state after a transmit is
   *  initiated but before macTxActive is set. To recover from this, the transmit must be aborted
   *  and proper notification given to high-level.
   */

  /* save transmit type */
  macTxType = txType;

  /*-------------------------------------------------------------------------------
   *  Prepare for transmit.
   */
  switch (macTxType)
  {
  case MAC_TX_TYPE_SLOTTED_CSMA:
  case MAC_TX_TYPE_UNSLOTTED_CSMA:
    nb = 0;

    macTxBe = (pMacDataTx->internal.txOptions & MAC_TXOPTION_ALT_BE) ? pMacPib->altBe : pMacPib->minBe;

    if ((macTxType == MAC_TX_TYPE_SLOTTED_CSMA) && (pMacPib->battLifeExt))
    {
      macTxBe = MIN(2, macTxBe);
    }
    txCsmaPrep();

    /* In case of MAC_NO_TIME */
    if (pMacDataTx == NULL)
    {
      return;
    }

    break;
  case MAC_TX_TYPE_NO_CSMA:
    nb = 0;
#ifndef FEATURE_WISUN_EDFE_SUPPORT
    macTxUpdateIE();
#else
    macEdfeInfo.ieInfo.txOffset = 0;
    macTxUpdateIE(pMacDataTx);
#endif

    break;

  case MAC_TX_TYPE_FH_CSMA:
    nb = pMacDataTx->internal.nb;
    macTxBe = pMacDataTx->internal.be;
    npdb = 0;
    txCsmaPrep();
    /* In case of MAC_NO_TIME */
    if(pMacDataTx == NULL)
      return;
    break;
  default:
    MAC_ASSERT(0);
    break;
  }

  /*-------------------------------------------------------------------------------
   *  Load transmit FIFO unless this is a retransmit.  No need to write
   *  the FIFO again in that case.
   */
  if (!txRetransmitFlag)
  {
    MAC_ASSERT(pMacDataTx != NULL); /* must have data to transmit */

    /* save needed parameters */
    txAckReq = MAC_ACK_REQUEST(pMacDataTx->msdu.p);
    txSeqn   = MAC_SEQ_NUMBER(pMacDataTx->msdu.p);

    if ( !txAckReq )
    {
      /* ACK is not requested */
      MAC_RADIO_TX_CANCEL_ACK_TIMEOUT_CALLBACK();
    }

    /*
     *  Flush the TX FIFO.  This is necessary in case the previous transmit was never
     *  actually sent (e.g. CSMA failed without strobing TXON).  If bytes are written to
     *  the FIFO but not transmitted, they remain in the FIFO to be transmitted whenever
     *  a strobe of TXON does happen.
     */
    MAC_RADIO_FLUSH_TX_FIFO();
  }

  /*-------------------------------------------------------------------------------
   *  If not receiving, start the transmit.  If receive is active
   *  queue up the transmit.  Transmit slotted type frames regardlessly,
   *  i.e. Beacon frame in beacon mode.
   *
   *  Critical sections around the state change prevents any sort of race condition
   *  with  macTxStartQueuedFrame().  This guarantees function txGo() will only be
   *  called once.
   */
  if((macTxType == MAC_TX_TYPE_NO_CSMA) || (macTxType == MAC_TX_TYPE_SLOTTED))
  {
    halIntState_t  s;

    HAL_ENTER_CRITICAL_SECTION(s);
    if ((macTxType == MAC_TX_TYPE_SLOTTED) || (!macRxActive && !macRxOutgoingAckFlag))
    {
      macTxActive = MAC_TX_ACTIVE_GO;
      HAL_EXIT_CRITICAL_SECTION(s);
      DBG_PRINT1(DBGSYS, "MAC_TX_ACTIVE_GO, macTxType=%i", macTxType);

      /* The txGo() is called in txCsmaPrep() for FH cases */
      txGo();
    }
    else
    {
      macTxActive = MAC_TX_ACTIVE_QUEUED;
      HAL_EXIT_CRITICAL_SECTION(s);
      DBG_PRINT3(DBGSYS, "MAC_TX_ACTIVE_QUEUED, macTxType=%i, macRxActive=0x%X, macRxOutgoingAckFlag=0x%X", macTxType, macRxActive, macRxOutgoingAckFlag);
    }
  }
}


/*=================================================================================================
 * @fn          txCsmaPrep
 *
 * @brief       Prepare/initialize for a CSMA transmit.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void txCsmaPrep(void)
{
  uint32_t txUfsi = 0;
  uint8_t baseBackoff = 0;

  macTxCsmaBackoffDelay = macRadioRandomByte() & ((1 << macTxBe) - 1);

  DBG_PRINT1(DBGSYS, "txCsmaPrep(macTxCsmaBackoffDelay = %i)", macTxCsmaBackoffDelay);

  if(MAC_FH_ENABLED && MAC_FH_GET_PIB_FN )
  {
      /*Rem DT */
      MAC_FH_GET_PIB_FN(FHPIB_CSMA_BASE_BACKOFF, &baseBackoff);

      if(txRetransmitFlag && !nb && baseBackoff)
      {
        DBG_PRINT1(DBGSYS, "macCsmaDelay after dwell adj (delay=%i)", macTxCsmaBackoffDelay);
        if(MAC_FH_GET_REM_DT_FN)
        {
          txUfsi = MAC_FH_GET_REM_DT_FN();
          if(txUfsi)
          {
            macTxCsmaBackoffDelay += (txUfsi/(MAC_A_UNIT_BACKOFF_CCA_PERIOD * macSpecUsecsPerSymbol));
          }
          DBG_PRINT1(DBGSYS, "macCsmaDelay after dwell adj (delay=%i)", macTxCsmaBackoffDelay);
        }
      }

      macTxCsmaBackoffDelay += baseBackoff * nb;

      DBG_PRINT1(DBGSYS, "txCsmaPrep after basebackoff= %i)", macTxCsmaBackoffDelay);
   }
   MAC_RADIO_TX_PREP_CSMA_UNSLOTTED();
}
/*=================================================================================================
 * @fn          txGo
 *
 * @brief       Start a transmit going.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void txGo(void)
{
    txCsmaGo();
}


/*=================================================================================================
 * @fn          txCsmaGo
 *
 * @brief       Start a CSMA transmit going.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void txCsmaGo(void)
{
  switch (macTxType)
  {
  case MAC_TX_TYPE_SLOTTED_CSMA:
  case MAC_TX_TYPE_UNSLOTTED_CSMA:
  case MAC_TX_TYPE_FH_CSMA:
  case MAC_TX_TYPE_NO_CSMA:
    MAC_RADIO_TX_GO(macTxType);
    break;

  default:
    break;
  }
}

/**************************************************************************************************
 * @fn          macTxFrameRetransmit
 *
 * @brief       Retransmit the last frame.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTxFrameRetransmit(void)
{
  txRetransmitFlag = 1;

#if defined ( FEATURE_SYSTEM_STATS )
  /* Update Diagnostics counter */
  macLowLevelDiags(MAC_DIAGS_TX_UCAST_RETRY);
#endif

  macTxFrame(macTxType);
}

/**************************************************************************************************
 * @fn          macTxStartQueuedFrame
 *
 * @brief       See if there is a queued frame waiting to transmit.  If so, initiate
 *              the transmit now.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTxStartQueuedFrame(void)
{
  halIntState_t  s;

  MAC_ASSERT(!macRxActive && !macRxOutgoingAckFlag); /* queued frames should not transmit in middle of a receive */
  DBG_PRINT0(DBGSYS, "macTxStartQueuedFrame()");
  /*
   *  Critical sections around the state change prevents any sort of race condition
   *  with macTxFrame().  This guarantees function txGo() will only be be called once.
   */
  HAL_ENTER_CRITICAL_SECTION(s);
  if (macTxActive == MAC_TX_ACTIVE_QUEUED)
  {
    macTxActive = MAC_TX_ACTIVE_INITIALIZE;
    if((macTxType == MAC_TX_TYPE_SLOTTED) || (macTxType == MAC_TX_TYPE_NO_CSMA))
    {
      macTxActive = MAC_TX_ACTIVE_GO;
    }
    HAL_EXIT_CRITICAL_SECTION(s);

    /* Prepare the CSMA/LBT and TX timing again for queued TX */
    if (macTxType == MAC_TX_TYPE_SLOTTED_CSMA ||
        macTxType == MAC_TX_TYPE_UNSLOTTED_CSMA ||
        macTxType == MAC_TX_TYPE_FH_CSMA)
    {
      txCsmaPrep();
    }
    if((macTxType == MAC_TX_TYPE_SLOTTED) || (macTxType == MAC_TX_TYPE_NO_CSMA))
    {
      txGo();
      DBG_PRINT0(DBGSYS, "Queued TX Sent");
    }
  }
  else
  {
    HAL_EXIT_CRITICAL_SECTION(s);
    DBG_PRINT0(DBGSYS, "Queued TX not Sent");
  }
}


/**************************************************************************************************
 * @fn          macTxChannelBusyCallback
 *
 * @brief       This callback is executed if a CSMA transmit was attempted but the channel
 *              was busy.
 *
 * @param       bForceComplete: whether or not to complete the TX forcefully
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTxChannelBusyCallback(bool bForceComplete)
{
  MAC_ASSERT((macTxType == MAC_TX_TYPE_SLOTTED_CSMA) ||
             (macTxType == MAC_TX_TYPE_UNSLOTTED_CSMA) ||
             (macTxType == MAC_TX_TYPE_FH_CSMA) ||
             (macTxType == MAC_TX_TYPE_NO_CSMA) ||
             (macTxType == MAC_TX_TYPE_LBT));

  macTxActive = MAC_TX_ACTIVE_CHANNEL_BUSY;

  nb++;
  DBG_PRINT1(DBGSYS, "macTxChannelBusyCallback(nb=%i)", nb);

//  if (macTxType != MAC_TX_TYPE_LBT)
  {
    /* clear channel assessment failed, follow through with CSMA algorithm */
    if ( (nb > pMacPib->maxCsmaBackoffs) )
    {
      txComplete(MAC_CHANNEL_ACCESS_FAILURE);
    }
    else
    {
      /* turn off receiver if allowed */
      macRxOffRequest();
      if (macTxType == MAC_TX_TYPE_FH_CSMA)
      {
        /* Notify FH module that CCA is busy */
        if ((MAC_FH_ENABLED) && (MAC_FH_SET_STATE_CB_FN))
        {
          MAC_FH_SET_STATE_CB_FN(LMAC_CCA_BUSY);
        }
      }
      macTxBe = MIN(macTxBe+1, pMacPib->maxBe);

      /* Reverse bit order in each byte after the header and before the FCS */
      macMcuBitReverse(pMacDataTx->msdu.p, pMacDataTx->msdu.len);

#ifdef FEATURE_MAC_SECURITY
      /* Check if security is enabled and FH is enabled */
      if ((*((uint16 *) pMacDataTx->msdu.p) & MAC_FCF_SEC_ENABLED_MASK) &&
          MAC_FH_ENABLED)
      {
        /* Decrypt the frame as it will be re-encrypted by LMAC */
        macTxDecrypt();
      }
#endif
      if (macTxType == MAC_TX_TYPE_NO_CSMA)
      {
        macTxActive = MAC_TX_ACTIVE_GO;
        txCsmaGo();
      }
      else if (macTxType != MAC_TX_TYPE_LBT)
      {
        txCsmaPrep();
      }
    }
  }

}


/**************************************************************************************************
 * @fn          macTxFrameIsr
 *
 * @brief       This callback is executed when transmit completes.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
uint8 numTx = 0;
void macTxFrameIsr(void)
{
    macTxTimestampCallback();
    macTxDoneCallback();
}


/**************************************************************************************************
 * @fn          macTxAbortIsr
 *
 * @brief       This callback is executed when transmit is aborted.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macTxAbortIsr(void)
{
  DBG_PRINT0(DBGSYS, "TX: macTxAbortIsr()");

  if (macTxActive == MAC_TX_ACTIVE_GO)
  {
    if (macTxType == MAC_TX_TYPE_NO_CSMA)
    {
      macTxChannelBusyCallback(FALSE);
    }
    else
    {
      macTxActive = MAC_TX_ACTIVE_DONE;
      txComplete(MAC_TX_ABORTED);
    }
  }
}


/**************************************************************************************************
 * @fn          macTxAckIsr
 *
 * @brief       This callback is executed when ACK transmit completes.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macTxAckIsr(void)
{
  macRxAckTxDoneCallback();
}

#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
/**************************************************************************************************
 * @fn          macTxEDFEIsr
 *
 * @brief       This callback is executed when ACK transmit completes.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macTxEDFEIsr(uint8 frameType)
{
    macRxEDFETxDoneCallback();
}
#endif

/**************************************************************************************************
 * @fn          macTxDoneCallback
 *
 * @brief       This callback is executed when transmit completes.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTxDoneCallback(void)
{
  halIntState_t  s;

  DBG_PRINT0(DBGSYS, "TX: macTxDoneCallback()");

  /* must have a valid data for done call back */
  MAC_ASSERT(pMacDataTx != NULL);
  txAckReq = MAC_ACK_REQUEST(pMacDataTx->msdu.p);
  /*
   *  There is a small chance this function could be called twice for a single transmit.
   *  To prevent logic from executing twice, the state variable macTxActive is used as
   *  a gating mechanism to guarantee single time execution.
   */
  HAL_ENTER_CRITICAL_SECTION(s);
  if (macTxActive == MAC_TX_ACTIVE_GO)
  {
    /* see if ACK was requested */
    if (!txAckReq)
    {
      macTxActive = MAC_TX_ACTIVE_DONE;
      HAL_EXIT_CRITICAL_SECTION(s);

      /* ACK was not requested, transmit is complete */
      txComplete(MAC_SUCCESS);
    }
    else
    {
      /*
       *  ACK was requested - must wait to receive it.  The macRxAckCmd end time
       *  is set to expire after the timeout duration for waiting for an ACK.
       *  If an ACK is received, the function macTxAckReceived() is called.
       *  If an ACK is not received within the timeout period,
       *  the function macTxAckNotReceivedCallback() is called.
       */
      MAC_ASSERT(pMacDataTx->internal.dest.dstAddr.addrMode != MAC_ADDR_MODE_NONE);
      HAL_EXIT_CRITICAL_SECTION(s);
    }
  }
  else
  {
    HAL_EXIT_CRITICAL_SECTION(s);
  }
}

#ifdef FEATURE_WISUN_EDFE_SUPPORT
/**************************************************************************************************
 * @fn          macTxEDFEReplyCallback
 *
 * @brief       This function is called by the receive logic when an ACK is received and
 *              transmit logic is listening for an ACK.
 *
 * @param       seqn        - sequence number of received ACK
 * @param       pendingFlag - set if pending flag of ACK is set, cleared otherwise
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTxEDFEReplyCallback(uint8 txFrameType)
{
  halIntState_t  s;

  DBG_PRINT2(DBGSYS, "macTxAckReceivedCallback(seqn=%i, pendingFlag=0x%X)", seqn, pendingFlag);

  /* only process if listening for an ACK; critical section prevents race condition problems */
  HAL_ENTER_CRITICAL_SECTION(s);
  if (txFrameType == MAC_FRAME_TYPE_EDFE_RDFRM)
  {
  macEdfeDbg.txConfirmed++;

#ifdef FH_BENCHMARK_BUILD_TIMES
    /* save the ACK time */
    macTx_AckTimeStamp[(ackTxIdx ++) % NUM_MAC_BENCHMARK_CNT] = ICall_getTicks() - ackStart_ts;
#endif

//    macTxActive = MAC_TX_ACTIVE_POST_ACK;
    HAL_EXIT_CRITICAL_SECTION(s);

    txComplete(MAC_SUCCESS);
  }
  else
  {
    HAL_EXIT_CRITICAL_SECTION(s);
  }
}
#endif

/**************************************************************************************************
 * @fn          macTxAckReceivedCallback
 *
 * @brief       This function is called by the receive logic when an ACK is received and
 *              transmit logic is listening for an ACK.
 *
 * @param       seqn        - sequence number of received ACK
 * @param       pendingFlag - set if pending flag of ACK is set, cleared otherwise
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTxAckReceivedCallback(uint8 seqn, uint8 pendingFlag)
{
  halIntState_t  s;

  DBG_PRINT2(DBGSYS, "macTxAckReceivedCallback(seqn=%i, pendingFlag=0x%X)", seqn, pendingFlag);

  /* only process if listening for an ACK; critical section prevents race condition problems */
  HAL_ENTER_CRITICAL_SECTION(s);
  if (macTxActive == MAC_TX_ACTIVE_LISTEN_FOR_ACK)
  {
  macAckDbg.txAckCancelTimer++;

#ifdef FH_BENCHMARK_BUILD_TIMES
    /* save the ACK time */
    macTx_AckTimeStamp[(ackTxIdx ++) % NUM_MAC_BENCHMARK_CNT] = ICall_getTicks() - ackStart_ts;
#endif

    macTxActive = MAC_TX_ACTIVE_POST_ACK;
    MAC_RADIO_TX_CANCEL_ACK_TIMEOUT_CALLBACK();
    HAL_EXIT_CRITICAL_SECTION(s);

    /* see if the sequence number of received ACK matches sequence number of packet just sent */
    if (seqn == txSeqn)
    {
      /*
       *  Sequence numbers match so transmit is successful.  Return appropriate
       *  status based on the pending flag of the received ACK.
       */
      if (pendingFlag)
      {
        txComplete(MAC_ACK_PENDING);
      }
      else
      {
        txComplete(MAC_SUCCESS);
      }
    }
    else
    {
      /* sequence number did not match; per spec, transmit failed at this point */
      txComplete(MAC_NO_ACK);
    }
  }
  else
  {
    HAL_EXIT_CRITICAL_SECTION(s);
  }
}


/**************************************************************************************************
 * @fn          macTxAckNotReceivedCallback
 *
 * @brief       This function is called by the receive logic when transmit is listening
 *              for an ACK but something else is received.  It is also called if the
 *              listen-for-ACK timeout is reached.
 *
 * @brief
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTxAckNotReceivedCallback(uint8 param)
{
  halIntState_t  s;

  (void) param;
  DBG_PRINT0(DBGSYS, "macTxAckNotReceivedCallback()");
  macAckDbg.ackTimeOut++;

  /* only process if listening for an ACK; critical section prevents race condition problems */
  HAL_ENTER_CRITICAL_SECTION(s);
  if (macTxActive == MAC_TX_ACTIVE_LISTEN_FOR_ACK)
  {
    macTxActive = MAC_TX_ACTIVE_POST_ACK;
    MAC_RADIO_TX_CANCEL_ACK_TIMEOUT_CALLBACK();
    HAL_EXIT_CRITICAL_SECTION(s);

    /* a non-ACK was received when expecting an ACK, per spec transmit is over at this point */
    txComplete(MAC_NO_ACK);
  }
  else
  {
    HAL_EXIT_CRITICAL_SECTION(s);
  }
}


/*=================================================================================================
 * @fn          txComplete
 *
 * @brief       Transmit has completed.  Perform needed maintenance and return status of
 *              the transmit via callback function.
 *
 * @param       status - status of the transmit that just went out
 *
 * @return      none
 *=================================================================================================
 */
void txComplete(uint8 status)
{
  DBG_PRINT1(DBGSYS, "txComplete(status=0x%X)", status);
  /* reset the retransmit flag */
  txRetransmitFlag = 0;

  /* update tx state; turn off receiver if nothing is keeping it on */
  macTxActive = MAC_TX_ACTIVE_NO_ACTIVITY;

  /* return status of transmit via callback function */
  if ((MAC_FH_ENABLED) && (MAC_FH_COMPLETE_TX_CB_FN ))
  {
    DBG_PRINT1(DBGSYS, "MAC_FH_COMPLETE_TX_CB_FN(status=0x%X)", status);

    if(pMacDataTx->internal.frameType == MAC_FRAME_TYPE_ASYNC)
    {
      /* Reverse bit order in each byte after the header and before the FCS */
      macMcuBitReverse(pMacDataTx->msdu.p, pMacDataTx->msdu.len);

#ifdef FEATURE_MAC_SECURITY
      if((*((uint16 *) pMacDataTx->msdu.p)) & MAC_FCF_SEC_ENABLED_MASK)
      {
        macTxDecrypt();
      }
#endif
    }
    MAC_FH_COMPLETE_TX_CB_FN(status);

    /* Power management state may change. Hence, vote. */
    MAC_PWR_VOTE_NO_SWI(FALSE);
  }

  if (pMacDataTx != NULL)
  {
    if(pMacDataTx->internal.frameType != MAC_FRAME_TYPE_ASYNC)
    {
      /* Turn on RX if allowed */
      macRxOnRequest();
    }
  }
  else
  {
    /* Turn on RX if allowed */
    macRxOnRequest();
  }
}

/**************************************************************************************************
 * @fn          macTxRssiChannelBusyCallback
 *
 * @brief       This callback is executed if a CSMA transmit was attempted but the channel
 *              was busy.
 *
 * @param       bForceComplete: whether or not to complete the TX forcefully
 *
 * @return      none
 **************************************************************************************************
 */
/*preamb detection time of 3ms */
#define PHY_PD_TIME  3
static MAC_INTERNAL_API void macTxRssiChannelBusyCallback(bool bForceComplete)
{
    MAC_ASSERT(macTxType == MAC_TX_TYPE_FH_CSMA);

    macTxActive = MAC_TX_ACTIVE_CHANNEL_BUSY;

    npdb++;

    DBG_PRINT1(DBGSYS, "macFHRssiChannelBusyCallback(npdb=%i)", npdb);

    /* RSSI assement failed, take a CSMA backoff again */

    /* Notify FH module that CCA is busy */
    if (MAC_FH_SET_STATE_CB_FN)
    {
        MAC_FH_SET_STATE_CB_FN(LMAC_RSSI_BUSY);
    }
    macTxCsmaBackoffDelay = PHY_PD_TIME;

    /*Fixed time, Do not adjust PDet backoff */

    MAC_RADIO_TX_PREP_CSMA_UNSLOTTED();

}

/**************************************************************************************************
 * @fn          txCsmaDelay
 *
 * @brief       This function is called after frequency hopping random backoff delay.
 *
 * @param       param - not used
 *
 * @return      none
 **************************************************************************************************
 */
void txCsmaDelay(uint8 param)
{
  halIntState_t  s;

  (void)param;
  MAC_ASSERT(pMacDataTx != NULL); /* must have data to transmit */

  DBG_PRINT0(DBGSYS, "txCsmaDelay()");

#ifdef FH_BENCHMARK_BUILD_TIMES
  dataBuildStart_ts = ICall_getTicks();
#endif

  if (MAC_FH_ENABLED)
  {
    /* Update the timing IEs */
#ifndef FEATURE_WISUN_EDFE_SUPPORT
    macTxUpdateIE();
#else
    macEdfeInfo.ieInfo.txOffset = 0;
    macTxUpdateIE(pMacDataTx);
#endif

    HAL_ENTER_CRITICAL_SECTION(s);
    if (!macRxActive && !macRxOutgoingAckFlag)
    {
      /* Notify FH module that the TX is about to start */
      MAC_FH_SET_STATE_CB_FN(LMAC_TX_START);

      /*ED on current RX channel */
      /* Additional preamble detection backoff is taken
       * if rssi is high to give enough time for preamble to be
       * detected if it is valid packet, else it is assumed as interference.
       * and update to TX channel to transmit
       *
       * It is not necessary to call getRSSI to perform the backoff
       * We will use the RF driver CCA feature. if CCA fails, we got the TX Done callback with
       * error code.
       * */

      if( (npdb >= (FH_MAXPDB - 1) ))
      {
        macTxActive = MAC_TX_ACTIVE_GO;

        macStopCmd(FALSE);

        HAL_EXIT_CRITICAL_SECTION(s);

        /* set channel */
        macPhyChannel = txChannel;
        txGo();
        DBG_PRINT1(DBGSYS, "txCsmaDelay() rssi=%d", rssi);
      }
      else
      {
        HAL_EXIT_CRITICAL_SECTION(s);
        /*RX channel busy */
        /*take a backoff again */
        macTxRssiChannelBusyCallback(0);
      }
    }
    else
    {
      macTxActive = MAC_TX_ACTIVE_QUEUED;
      HAL_EXIT_CRITICAL_SECTION(s);
      MAC_FH_SET_STATE_CB_FN(LMAC_TX_PUSH_TO_QUEUE);
      DBG_PRINT3(DBGSYS, "MAC_TX_ACTIVE_QUEUED, macTxType=%i, macRxActive=0x%X, macRxOutgoingAckFlag=0x%X", macTxType, macRxActive, macRxOutgoingAckFlag);
    }

  }

  /* Power management state may change. Hence, vote. */
  MAC_PWR_VOTE_NO_SWI(FALSE);
}
/**************************************************************************************************
 * @fn          macTxRequestCsmaRTCDelay
 *
 * @brief       This function is used to request a Frequency Hopping CSMA delay.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macTxRequestCsmaRtcDelay( uint32_t backOffDur)
{

  DBG_PRINT0(DBGSYS, "macTxRequestCsmaDelay()");
  uint32_t key;
  key = OsalPort_enterCS();
  MAP_ICall_setTimer((uint_fast32_t)backOffDur/MAP_ICall_getTickPeriod(), (MAP_ICall_TimerCback)macCsmaTimerExpiry,
                 NULL, &macCsmaRtcTimerId);
  OsalPort_leaveCS(key);

}

/**************************************************************************************************
 * @fn          macTxRequestCsmaDelay
 *
 * @brief       This function is used to request a Frequency Hopping CSMA delay.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macTxRequestCsmaDelay( void )
{

  uint32_t backOffDur;
  FHAPI_status status;

  DBG_PRINT0(DBGSYS, "macTxRequestCsmaDelay()");
#if 1 //remove work around
  /*Work around for RAT timer issue for backOffDelay values of 0
  RAT timer to be replaced with CSMA timer next release
  */
  if(!macTxCsmaBackoffDelay)
      macTxCsmaBackoffDelay = 1;
  /*end work around */
#endif
  backOffDur = macTxCsmaBackoffDelay * MAC_A_UNIT_BACKOFF_CCA_PERIOD * macSpecUsecsPerSymbol;

  if ((MAC_FH_ENABLED) && (MAC_FH_GET_TX_PARAMS_FN) && (MAC_FH_COMPLETE_TX_CB_FN))
  {
#ifdef FEATURE_WISUN_SUPPORT
   /* During Certification it was observed that some implementation required more time
    * to switch to Broadcast and hence lost DIO packets sent early in Broadcast slots.
    * Purposefully need to wait at least for 10ms into broadcast slot before sending packet.
    * Check to do so only when macDwellTime > 50 ms
    */
    if(pMacDataTx->internal.frameType == MAC_FRAME_TYPE_BROADCAST)
    {
          uint8_t macBcDwellTime;
          MAP_FHPIB_get(FHPIB_BC_DWELL_INTERVAL, &macBcDwellTime);
          if(macBcDwellTime >= IOT_MIN_BACKOFF_DWELL_TIME)
          {
              backOffDur = backOffDur > (IOT_MIN_BACKOFF_GUARD_DELAY * 1000) ? backOffDur : (IOT_MIN_BACKOFF_GUARD_DELAY * 1000);
          }
    }
#endif
    status = MAC_FH_GET_TX_PARAMS_FN(&backOffDur, &txChannel);

    if(status == FHAPI_STATUS_ERR_OUT_SLOT)
    {
      pMacDataTx->internal.nb = nb;
      pMacDataTx->internal.be = macTxBe;
      macTxActive = MAC_TX_ACTIVE_NO_ACTIVITY;

      txComplete(MAC_NO_TIME);

      return;
    }

    if(status != FHAPI_STATUS_SUCCESS)
    {
        macTxActive = MAC_TX_ACTIVE_NO_ACTIVITY;

        MAP_macTxCompleteCallback(status);

      return;
    }
  }

  /*Start RTC timer */
  macTxRequestCsmaRtcDelay(backOffDur);
}


#ifdef FEATURE_MAC_SECURITY
/**************************************************************************************************
 * @fn          macTxDecrypt
 *
 * @brief       Decrypt frame for re-encryption
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */

uint8 macTxDecrypt(void)
{
  uint8   status;

  /* Replace the unsecured frame with a secured frame only if
   * security is turned on and security level is not 0
   */
  status = MAP_macCcmStarInverseTransform(pMacDataTx->internal.pKeyDesc->key,
                                      pMacDataTx->internal.frameCntr,
                                      pMacDataTx->sec.securityLevel,
                                      pMacDataTx->msdu.p,
                                      pMacDataTx->msdu.len,
                                      pMacDataTx->internal.ptrMData,
                                      pMacDataTx->msdu.len -
                                        (pMacDataTx->internal.ptrMData -
                                         pMacDataTx->msdu.p),
                                      pMacPib->extendedAddress.addr.extAddr);

  return status;
}
#endif /* FEATURE_MAC_SECURITY */

#ifndef FEATURE_WISUN_EDFE_SUPPORT
/**************************************************************************************************
 * @fn          macTxUpdateIE
 *
 * @brief       This function is used to update UTIE/BTIE
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macTxUpdateIE(void)
{
  if ((MAC_FH_ENABLED) && (MAC_FH_GENERATE_IE_FN ))
  {
    if(pMacDataTx->internal.ptrUTIE != NULL)
    {
      MAC_FH_GENERATE_IE_FN(pMacDataTx->internal.ptrUTIE, FH_WISUN_HIE_UT_IE_BITMAP, pMacDataTx, NULL);
    }

    if(pMacDataTx->internal.ptrBTIE != NULL)
    {
      MAC_FH_GENERATE_IE_FN(pMacDataTx->internal.ptrBTIE, FH_WISUN_HIE_BT_IE_BITMAP, pMacDataTx, NULL);
    }
  }
}
#else
/**************************************************************************************************
 * @fn          macTxUpdateIE
 *
 * @brief       This function is used to update UTIE/BTIE
 *
 * @param       pData - data pointer for transmit packet
 *
 * @return      none
 **************************************************************************************************
 */
void macTxUpdateIE(macTx_t *pData)
{
  if ((MAC_FH_ENABLED) && (MAC_FH_GENERATE_IE_FN ))
  {
    if(pData->internal.ptrUTIE != NULL)
    {
      MAC_FH_GENERATE_IE_FN(pData->internal.ptrUTIE, FH_WISUN_HIE_UT_IE_BITMAP, pData, NULL);
    }

    if(pData->internal.ptrBTIE != NULL)
    {
      MAC_FH_GENERATE_IE_FN(pData->internal.ptrBTIE, FH_WISUN_HIE_BT_IE_BITMAP, pData, NULL);
    }

    if(macEdfeInfo.ieInfo.ptrFCIE != NULL)
    {
        uint8 *p = macEdfeInfo.ieInfo.ptrFCIE;
        *(p + 0) = macEdfeInfo.ieInfo.txfc;
        *(p + 1) = macEdfeInfo.ieInfo.rxfc;
    }

    if(macEdfeInfo.ieInfo.ptrRSLIE != NULL)
    {
        uint8 *p = macEdfeInfo.ieInfo.ptrRSLIE;
        *(p + 0) = rxFhRsl;
    }
  }
}
#endif

/**************************************************************************************************
 * @fn          macTxTimestampCallback
 *
 * @brief       This callback function records the timestamp into the receive data structure.
 *              It should be called as soon as possible after there is a valid timestamp.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTxTimestampCallback(void)
{
  MAC_ASSERT(pMacDataTx != NULL); /* transmit structure must be there */

  DBG_PRINTL1(DBGSYS, "TX Time Stamp=0x%X", macTxTimestamp);

  /* The time, in aUnitBackoffPeriod units, at which the frame was transmitted. */
  pMacDataTx->internal.timestamp  = MAC_RADIO_SYMBOL_CAPTURE( macTxTimestamp );

  /* The time, in RAT ticks, at which the frame was transmitted. */
  pMacDataTx->internal.timestamp2 = MAC_RADIO_TIMER_CAPTURE( macTxTimestamp );
}

/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */
#if (MAC_TX_ACTIVE_NO_ACTIVITY != 0x00)
#error "ERROR! Zero is reserved value of macTxActive. Allows boolean operations, e.g !macTxActive."
#endif

/**************************************************************************************************
*/
