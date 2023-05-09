/******************************************************************************

 @file  mac_radio_tx.c

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
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */

#ifndef LIBRARY
#include "ti_wisunfan_features.h"
#endif

/* hal */
#include "hal_types.h"
#include "hal_mcu.h"

/* high-level */
#include "mac_spec.h"
#include "mac_pib.h"
#include "mac_timer.h"
#include "mac_beacon.h"
#include "mac_security.h"
#include "mac_mgmt.h"
#include "mac_data.h"
#include "mac_hl_patch.h"

/* low-level specific */
#include "mac_radio_tx.h"
#include "mac_tx.h"
#include "mac_rx.h"
#include "mac_rx_onoff.h"
#include "mac_radio.h"
#include "mac_low_level.h"

/* target specific */
#include "mac_radio_defs.h"
#include "mac_symbol_timer.h"

/* debug */
#include "mac_assert.h"

/* CM0 related */
#include "mac_settings.h"

/* Activity Tracking */
#include "mac_activity.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_radio_tx_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

/******************************************************************************
  ROM jump table will support
  1. TIMAC ROM image build
  2. TIMAC stack build without ROM image
  3. TIMAC stack with calling ROM image
 *****************************************************************************/
#include "rom_jt_154.h"

/* ------------------------------------------------------------------------------------------------
 *                                         Defines
 * ------------------------------------------------------------------------------------------------
 */
#define MAC_RADIO_TX_TIMESTAMP_OFFSET       384
#if defined(DeviceFamily_CC13X2) || defined(DeviceFamily_CC13X2X7) || defined(DeviceFamily_CC13X4)
#define MAC_RADIO_CMD_START_TIME_ADDR       0x21000128
#endif
#define MAC_SLOT_INTERVAL(x)                (((uint32) MAC_A_BASE_SLOT_DURATION) << (x))

/* ------------------------------------------------------------------------------------------------
 *                                       Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */
static void macRadioTxAck(uint32 txTime);
static void macRadioTxAckCb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);
#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
static void macRadioTxEDFEEfrm(uint8 frameType, uint32 txTime);
static void macRadioTxRequestTxEDFEEfrm(uint8 frameType);
static void macRadioTxEDFECb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);
#endif
#ifdef FREERTOS_SUPPORT
void runTxCb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);
#endif //FREERTOS_SUPPORT

/* ------------------------------------------------------------------------------------------------
 *                                     Local Variables
 * ------------------------------------------------------------------------------------------------
 */
RF_CmdHandle  txCmdHandle;
macTimer_t macAIFSTimer;
macTx_t *pMacEnhancedAck = NULL;

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
/* For 4-byte CRC, whitening enabled */
uint8 macRadioAckFrame[] = { 0x07, 0x08, 0x02, 0x00, 0x00 };
#endif

#ifdef FH_BENCHMARK_BUILD_TIMES
extern uint32 ackStart_ts;
extern uint32_t dataBuildStart_ts;
uint16_t macDataBuildTime[NUM_MAC_BENCHMARK_CNT];
uint8_t buildDataIdx = 0;
uint16 macBuildAckTime[NUM_MAC_BENCHMARK_CNT];
uint8 macBuildAckIdx =0;
#endif

extern uint32 macRxSymbol;

#ifdef FH_HOP_DEBUG_FIXED
fhDbg_t fhDbg;
#endif

#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
extern macEdfeInfo_t macEdfeInfo;
#endif

#ifdef FREERTOS_SUPPORT
RF_Callback  txCbRf;
#endif //FREERTOS_SUPPORT
/* ------------------------------------------------------------------------------------------------
 *                                          Macros
 * ------------------------------------------------------------------------------------------------
 */

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
/**************************************************************************************************
 * @fn          macRadioTxPrepCsmaUnslottedSubG
 *
 * @brief       Prepare CM0 for "Unslotted CSMA" transmit.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioTxPrepCsmaUnslottedSubG(void)
{
  /* Setup MAC transmit command with FH CSMA/CA */
  RF_cmdPropCs.startTrigger.triggerType = TRIG_NOW;

  /* Start the CS command immediately for FH CSMA/CA */
  RF_cmdPropCs.startTime = 0;

  /* The beacon mode may have changed the TX trigger. Restore it here. */
  RF_cmdPropTxAdv.startTrigger.triggerType = TRIG_NOW;

  RF_cmdPropCs.csConf.busyOp = 1;
  RF_cmdPropCs.csConf.idleOp = 1;
  RF_cmdPropCs.csEndTime = 8000; // 2 ms

  /* For CSMA, start a backoff timer */
  macTxRequestCsmaDelay();

}
#endif

/**************************************************************************************************
 * @fn          macRadioTxPrepCsmaUnslotted
 *
 * @brief       Prepare CM0 for "Unslotted CSMA" transmit.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioTxPrepCsmaUnslotted(void)
{
  SELECT_CALL(macRadioTxPrepCsmaUnslotted);
}

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
/**************************************************************************************************
 * @fn          macRadioTxPrepCsmaSlottedSubG
 *
 * @brief       Prepare CM0 for "Slotted CSMA" transmit.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioTxPrepCsmaSlottedSubG(void)
{
}
#endif

/**************************************************************************************************
 * @fn          macRadioTxPrepCsmaSlotted
 *
 * @brief       Prepare CM0 for "Slotted CSMA" transmit.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioTxPrepCsmaSlotted(void)
{
  SELECT_CALL(macRadioTxPrepCsmaSlotted);
}

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
/**************************************************************************************************
 * @fn          macRadioTxHandleAck
 *
 * @brief       This function handles routing the callback to correctly handle rx acks.
 *
 *
 * @param       h - radio handle
 *              ch - radio command handle
 *              e - radio event
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioTxHandleAck(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
  if (RF_cmdPropRxAdv.status == PROP_DONE_RXTIMEOUT)
  {
      macRxNokIsr();
  }
  else
  {
     /* Incrementing the number of RF commands because the total count was
      * reduced once the last command was reached, but macRxCb needs to
      * know that there is still a command to do.
      */
    numRfCmd++;
     /* Incrementing the number of RX commands because the receiver does
      * not listen for the rx ack unless there is more than 0 rx commands
      * Note: The numRxCmd variable is normally increased when issuing an Rx Command.
      */
    numRxCmd++;
    macRxCb(h,ch,e);
  }
  /* wrong packet, didn't receive ACk */
  if (macTxActive == MAC_TX_ACTIVE_LISTEN_FOR_ACK)
  {
      macRxNokIsr();
  }
}

/**************************************************************************************************
 * @fn          macRadioCsmaTxCbSubG
 *
 * @brief       This callback is called when the CSMA is busy or TX is complete or stopped.
 *
 * @param       h - radio handle
 *              ch - radio command handle
 *              e - radio event
 *
 * @return      none
 **************************************************************************************************
 */
static void macRadioCsmaTxCbSubG(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
  RF_Op *pRfOp;
  /* Now we are in swi context */
  bInSwi = TRUE;

  DBG_PRINT1(DBGSYS, "TX: macRadioCsmaTxCb(ch=%i)", ch);
  DBG_PRINTL2(DBGSYS, "TX: macRadioCsmaTxCb(event=0x%08X%08X)", e >> 32, e & 0xFFFFFFFF);

  /* FS command should have been completed */
  macFsCmdActive = FALSE;

#ifdef DBG_TIMAC_RADIO_CB
  macRadioDbg[macRadDbg_idx].fn = MAC_TX_CB_FN,
  macRadioDbg[macRadDbg_idx].rf_cmd_hnd = txCmdHandle;
  macRadioDbg[macRadDbg_idx].rf_hnd = h;
  macRadioDbg[macRadDbg_idx].ch_var = ch;
  macRadioDbg[macRadDbg_idx].rf_event = e;
  macRadioDbg[macRadDbg_idx].sfd_detect = e & RF_EventMdmSoft;
  macRadioDbg[macRadDbg_idx].clk_ticks = ClockP_getSystemTicks();
  macRadioDbg[macRadDbg_idx].rf_cmd_status = RF_cmdPropTxAdv.status;
  macRadioDbg[macRadDbg_idx].numRxCmd = numRxCmd;
  macRadioDbg[macRadDbg_idx].numRfCmd = numRfCmd;
  macRadDbg_idx = macRadDbg_idx + 1;
  macRadDbg_idx = macRadDbg_idx % MAX_RADIO_DBG_CNT;
#endif

  MAC_ASSERT(pMacDataTx != NULL); /* transmit buffer must be present */
//  pRfOp = RF_getCmdOp(h, ch);


  /* Parse transmission status and update tracking data */
  if (activityObject.pSetActivityTrackingTxFn)
  {
      activityObject.pSetActivityTrackingTxFn(&pMacDataTx->internal, RF_cmdPropTxAdv.status, e);
  }

  if ((e & RF_EventLastCmdDone))
  {
    numRfCmd--;
    macTxCmdActive = FALSE;

    if (RF_cmdPropCsSlotted.status == PROP_DONE_BUSY ||
        RF_cmdPropCsSlotted.status == PROP_DONE_BUSYTIMEOUT ||
        RF_cmdPropCs.status == PROP_DONE_BUSY ||
        RF_cmdPropCs.status == PROP_DONE_BUSYTIMEOUT)
    {
        DBG_PRINT0(DBGSYS, "CSMA Backoff");

      /* Make sure that the callback is not for TX command
       * since the callback for CSMA and TX is shared.
       */
//      if (pRfOp && (pRfOp->commandNo != CMD_PROP_TX_ADV))
      {
        /* CCA busy */
        macTxChannelBusyCallback(FALSE);

        if (numRfCmd == 0)
        {
          /* Power management state may change. Hence, vote. */
          MAP_macPwrVote(FALSE, FALSE);
        }
        else
        {
          MAC_ASSERT(numRfCmd > 0);
          DBG_PRINT1(DBGSYS, "TX: %i RF commands are pending. Can't power down.", numRfCmd);
        }

        /* Now we are exiting the swi */
        bInSwi = FALSE;

        return;
      }
    }

    if((RF_cmdPropTxAdv.status == PROP_DONE_SYNTH_ERROR) ||
       (RF_cmdPropCs.status == PROP_DONE_SYNTH_ERROR) ||
       (RF_cmdPropCsSlotted.status == PROP_DONE_SYNTH_ERROR) ||
       (RF_cmdFsTx.status == ERROR_SYNTH_PROG))
    {
        DBG_PRINT0(DBGSYS, "Synth error");

      /*In case of synth error, skip if CSMA is not needed for packet */
        /* get the band info from PHY ID */
      {
        if (( MAC_TX_TYPE_SLOTTED == macTxType ) || (macTxType == MAC_TX_TYPE_NO_CSMA))
        {
          /**
           * MAC received a synth error while trying to tx periodic beacon
           * (beacon enabled network). There is no way to retry beacon as it
           * is time-critical. For the system to not error out, we have to
           * fake a successful beacon transmission, so that that system can
           * maintain the time synchronization and prepare to send the next
           * beacon at the right interval.
           */
          txComplete(MAC_SUCCESS);
        }
        else
        {
          /* Safer to recover gracefully using Channel Busy */
          macTxChannelBusyCallback(FALSE);

          if (numRfCmd == 0)
          {
            /* Power management state may change. Hence, vote. */
            MAP_macPwrVote(FALSE, FALSE);
          }
          else
          {
            MAC_ASSERT(numRfCmd > 0);
            DBG_PRINT1(DBGSYS, "TX: %i RF commands are pending. Can't power down.", numRfCmd);
          }

        }
        /* Now we are exiting the swi */
        bInSwi = FALSE;

        return;
      }
    }

    /* Normal cases */
    if (RF_cmdPropTxAdv.status == PROP_DONE_OK)
    {
      /* Must add FW timestamp offset due to lack of direct HW support.
       * This will get us very close to the TX starting time.
       */
      macTxTimestamp = HWREG(MAC_RADIO_CMD_START_TIME_ADDR) + MAC_RADIO_TX_TIMESTAMP_OFFSET;
      if (macTxActive == MAC_TX_ACTIVE_LISTEN_FOR_ACK)
      {
        DBG_PRINT0(DBGSYS, "Waiting for ACK");

        macRadioTxHandleAck(h,ch,e);
      }
      else
      {
         DBG_PRINT0(DBGSYS, "Tx success processing");

         macTxFrameIsr();
      }
    }
    else if (RF_cmdPropTxAdv.status & 0x0800)
    {
      /* It is an error if bit 11 is set */
      DBG_PRINT1(DBGSYS, "!!! TX Command Status Error !!! status = 0x%X", RF_cmdPropTxAdv.status);
      txComplete(MAC_TX_ABORTED);
    }

    /* Turn on RX if needed */
    macRxOnRequest();
  }
  else if (e & (RF_EventCmdStopped | RF_EventCmdAborted | RF_EventCmdCancelled | RF_EventCmdPreempted))
  {
      numRfCmd--;
      DBG_PRINT0(DBGSYS, "Tx command aborted");

    if (macTxActive == MAC_TX_ACTIVE_LISTEN_FOR_ACK)
    {
        /* Complete Tx with MAC_NO_ACK */
        macRxNokIsr();
    }
    else {
        /* Complete Tx with MAC_TX_ABORTED */
        macTxAbortIsr();
    }
  }
  else if (e & (RF_EventRxOk | RF_EventRxBufFull | RF_EventRxNOk | RF_EventMdmSoft))
  {
      DBG_PRINT0(DBGSYS, "Processing ACK");

      macRxCb(h,ch,e);
  }
//  else if (pRfOp && (pRfOp->commandNo != CMD_FS))
  else
  {
    DBG_PRINT0(DBGSYS, "!!! TX Error !!! Unhandled exception!");
    txComplete(MAC_TX_ABORTED);
  }


  if (numRfCmd == 0)
  {
    /* Power management state may change. Hence, vote. */
    MAP_macPwrVote(FALSE, FALSE);
  }
  else
  {
    MAC_ASSERT(numRfCmd > 0);
    DBG_PRINT1(DBGSYS, "TX: %i RF commands are pending. Can't power down.", numRfCmd);
  }

  /* Now we are exiting the swi */
  bInSwi = FALSE;
}
#endif

/**************************************************************************************************
 * @fn          macRadioCsmaTxCb
 *
 * @brief       This callback is called when the CSMA is busy or TX is complete or stopped.
 *
 * @param       h - radio handle
 *              ch - radio command handle
 *              e - radio event
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioCsmaTxCb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
  SELECT_CALL(macRadioCsmaTxCb, h, ch, e);
}
/**************************************************************************************************
 * @fn          macRadioFHEncryptFrame
 *
 * @brief       Run previously radio command for CSMA transmit.  Handles unslotted CSMA transmits.
 *              When transmission has finished, this ISR will call macTxDoneCallback().
 *
 * @param       txType - Slotted, unslotted, FH or LBT.
 *
 * @return      none
 **************************************************************************************************
 */
#ifdef FEATURE_MAC_SECURITY
MAC_INTERNAL_API uint8 macRadioFHEncryptFrame(uint8 *p, uint16 len)
{
  uint8 *pFrameCnt;
  uint8   status = MAC_SUCCESS;
/*
 * update the frame count in the mac frame
 * increment the frame cnt
 */
/* retriev the frame count location */
  pFrameCnt = pMacDataTx->internal.ptrFrameCnt;

/* check if the frame cnt is reaching the MAX */
  if ( (pMacDataTx->internal.pKeyDesc->frameCounter == MAC_MAX_FRAME_COUNTER) ||
     (pFrameCnt == NULL) )
  {
    status = MAC_COUNTER_ERROR;
  }
  else
  {
    pMacDataTx->internal.frameCntr = pMacDataTx->internal.pKeyDesc->frameCounter++;

  /* build frame counter */
    *pFrameCnt-- = BREAK_UINT32( pMacDataTx->internal.frameCntr, 3 );
    *pFrameCnt-- = BREAK_UINT32( pMacDataTx->internal.frameCntr, 2 );
    *pFrameCnt-- = BREAK_UINT32( pMacDataTx->internal.frameCntr, 1 );
    *pFrameCnt-- = BREAK_UINT32( pMacDataTx->internal.frameCntr, 0 );

    status = MAP_macCcmStarTransform(pMacDataTx->internal.pKeyDesc->key,
                               pMacDataTx->internal.frameCntr,
                               pMacDataTx->sec.securityLevel,
                               p,
                               len,
                               pMacDataTx->internal.ptrMData,
                               len - (pMacDataTx->internal.ptrMData - p));
  }

  if (status != MAC_SUCCESS)
  {
    if (MAC_FH_COMPLETE_TX_CB_FN )
    {
      macTxActive = MAC_TX_ACTIVE_NO_ACTIVITY;
      MAC_FH_COMPLETE_TX_CB_FN(MAC_SECURITY_ERROR);
    }
    else
    {
      txComplete(MAC_SECURITY_ERROR);
    }
  }

  return status;
}
#endif

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
/**************************************************************************************************
 * @fn          macRadioTxAckTimeout
 *
 * @brief       
 *
 * @param       None
 *
 * @return      uint32_t
 **************************************************************************************************
 */
static uint32_t macRadioTxGetAckTimeoutVal()
{
    uint16 totalSymbols = 0;
    if (MAP_macCheckPhyRate(pMacPib->curPhyID) == PHY_MODE_SLR_5K)
    {
        /* Add turnaround time before the ack in addition to the preamble, sync word and termination symbols */
        totalSymbols += (MAC_A_TURNAROUND_TIME + MAC_PHY_SHR_LEN_2FSK_LRM  + MAC_PHY_TERM_LEN_LRM);

        /* Add in the bytes for the OTA time for incoming ACK frame */
        totalSymbols += ((MAC_PHY_PHR_LEN + MAC_LEN_ACK_FRAME_FH) * MAC_SYMBOLS_PER_OCTET_2FSK_LRM);

    }
    else /* 50k and 200k */
    {
        /* Add turnaround time before the ACK */
        totalSymbols += MAC_A_TURNAROUND_TIME;

        /* Add in the bytes in the acknowledgment frame */
        totalSymbols += ((MAC_PHY_SHR_LEN_2FSK + MAC_PHY_PHR_LEN + MAC_LEN_ACK_FRAME_FH) * MAC_SYMBOLS_PER_OCTET_2FSK);
    }

    /* Convert symbols to microseconds, then RAT Ticks */
    return (RF_convertUsToRatTicks((totalSymbols * macSpecUsecsPerSymbol) + MAC_ACK_AIFS_PROCESS_DELAY));
}
/**************************************************************************************************
 * @fn          macRadioTxGoSubG
 *
 * @brief       Run previously radio command for CSMA transmit.  Handles unslotted CSMA transmits.
 *
 *              When transmission has finished, this ISR will call macTxDoneCallback().
 *
 * @param       txType - Slotted, unslotted, FH or LBT.
 *
 * @return      none
 **************************************************************************************************
 */
static void macRadioTxGoSubG(uint8 txType)
{
  uint8   mrFSKSFD;
  uint8   *p, ackReq;
#ifdef FEATURE_WISUN_EDFE_SUPPORT
  uint8   rfrmReq, frfrm;
#endif
  uint16  lenMhrMsdu;
  halIntState_t  s;
  RF_Op   *rfOpPtr;
  RF_Callback pCb;
  macMRFSKPHYDesc_t *pPhyDesc;
  RF_EventMask evts = RF_EventLastCmdDone;

  RF_ScheduleCmdParams RF_schTxparams;
  RF_ScheduleCmdParams_init(&RF_schTxparams);

  MAC_ASSERT(pMacDataTx != NULL); /* must have data to transmit */

  /* Power vote */
  macTxCmdActive = TRUE;

  ackReq = MAC_ACK_REQUEST(pMacDataTx->msdu.p);

#ifdef FEATURE_WISUN_EDFE_SUPPORT
  rfrmReq = 0;
  frfrm = 0;
  if(pMacDataTx->internal.frameType == MAC_FRAME_TYPE_EDFE)
  {
    p = macEdfeInfo.ieInfo.ptrFCIE;
    if(!ackReq && p)
    {
        rfrmReq = 1;
        if(*(p+0) == 0 && *(p+1) == 0)
        {
            frfrm = 1;
        }
    }
  }
#endif

  MAC_PWR_VOTE_NO_SWI(TRUE);

  /* set length of frame (note: use of term msdu is a misnomer, here it's actually mhr + msdu) */
  lenMhrMsdu = pMacDataTx->msdu.len;

  /* Load the local pointer */
  p = pMacDataTx->msdu.p;

  /* Stop RX command in progress */
  macRxOff();

#ifdef FEATURE_MAC_SECURITY
  /* Encrypt & authenticate the frame at every backoff or retransmission
     in case FH is enabled so that timestamps can be updated.
     In other cases, the frame has already been encrypted and authenticated
     by HMAC.
   */
  if ((*((uint16 *) p) & MAC_FCF_SEC_ENABLED_MASK) && MAC_FH_ENABLED)
  {
    uint8 status = macRadioFHEncryptFrame(p, lenMhrMsdu);
    if (status != MAC_SUCCESS)
    {
        return;
    }
  }
#endif

  /* Reverse bit order in each byte after the header and before the FCS */
  macMcuBitReverse(p, lenMhrMsdu);

  /* Get the sync word format from PIB */
  MAP_MAC_MlmeGetReq(MAC_PHY_MRFSKSFD, &mrFSKSFD);

  /* Set the channel again as the CMD_FS might be canceled */
  if (MAC_FH_ENABLED)
  {
    if (TRUE != MAC_RADIO_SET_CHANNEL(macPhyChannel))
      {
        txComplete(MAC_NO_RESOURCES);
        return;
      }
  }


#ifdef FH_BENCHMARK_BUILD_TIMES
  /* save the Data Build times */
  macDataBuildTime[(buildDataIdx ++) % NUM_MAC_BENCHMARK_CNT] = ICall_getTicks() - dataBuildStart_ts ;
#endif
  /* Set TX payload length including PHR header length */
  RF_cmdPropTxAdv.pktLen = lenMhrMsdu + MAC_PHY_PHR_LEN;

  /*-------------------------------------------------------------------------------
   *  Set PHR here. MAC_PHY_PHR_LEN spaces are reserved by high level MAC.
   */

  /* The length field in PHR should exclude the header but include CRC length.
   * Whitening is enabled by default to conform to WiSUN.
   * 4-byte CRC is enabled by default to conform to WiSUN.
   * TODO: For 2-byte CRC, need to "or" in MAC_PHY_PHR_FCS_TYPE_MASK.
   */
  bool fcsType;
  MAP_MAC_MlmeGetReq(MAC_FCS_TYPE, &fcsType);
#ifdef MAC_PROTOCOL_TEST
  *(--p) = (HI_UINT16(lenMhrMsdu) & MAC_PHY_PHR_LEN_MSB_MASK) | MAC_PHY_PHR_DW_MASK;
#else
  if(!fcsType)
  {
    lenMhrMsdu += MAC_FCS4_FIELD_LEN;
    *(--p) = (HI_UINT16(lenMhrMsdu) & MAC_PHY_PHR_LEN_MSB_MASK) | MAC_PHY_PHR_DW_MASK;
  }
  else
  {
    lenMhrMsdu += MAC_FCS2_FIELD_LEN;
    *(--p) = (HI_UINT16(lenMhrMsdu) & MAC_PHY_PHR_LEN_MSB_MASK) | MAC_PHY_PHR_DW_MASK | MAC_PHY_PHR_FCS_TYPE_MASK;
  }
#endif
  *(--p) =  LO_UINT16(lenMhrMsdu);

  /* Calculate command duration */
  RF_schTxparams.duration = macGetRFCmdDuration(lenMhrMsdu, MAC_ACK_REQUEST(p));
  DBG_PRINT2(DBGSYS, "Tx Command Duration (%x), Frame Type: (%x)", RF_schTxparams.duration, pMacDataTx->internal.frameType);

  /* Align payload pointer */
  RF_cmdPropTxAdv.pPkt = p;

  /* Reset TX status to IDLE */
  RF_cmdPropTxAdv.status = IDLE;

  /* Print out some debug info */
  DBG_PRINT4(DBGSYS, "DATA[0..3]=0x%X, 0x%X, 0x%X, 0x%X", RF_cmdPropTxAdv.pPkt[0], RF_cmdPropTxAdv.pPkt[1], RF_cmdPropTxAdv.pPkt[2], RF_cmdPropTxAdv.pPkt[3]);
  DBG_PRINT4(DBGSYS, "DATA[4..7]=0x%X, 0x%X, 0x%X, 0x%X", RF_cmdPropTxAdv.pPkt[4], RF_cmdPropTxAdv.pPkt[5], RF_cmdPropTxAdv.pPkt[6], RF_cmdPropTxAdv.pPkt[7]);
  DBG_PRINT4(DBGSYS, "DATA[8..11]=0x%X, 0x%X, 0x%X, 0x%X", RF_cmdPropTxAdv.pPkt[8], RF_cmdPropTxAdv.pPkt[9], RF_cmdPropTxAdv.pPkt[10], RF_cmdPropTxAdv.pPkt[11]);

  /* For uncoded SFD value on TX and RX */
  RF_cmdPropTxAdv.syncWord = RF_cmdPropRxAdv.syncWord0 =
    (mrFSKSFD == 0)? MAC_2FSK_UNCODED_SFD_0 : MAC_2FSK_UNCODED_SFD_1;

  RF_schTxparams.endTime = 0;

  /* Update the current Activity based on the frame type */
  if (activityObject.pSetActivityTxFn)
  {
      activityObject.pSetActivityTxFn(&pMacDataTx->internal);
  }

  /* Calculate activity information/RF Priority */
  if (activityObject.pGetActivityPriorityTxFn)
  {
      RF_schTxparams.activityInfo = activityObject.pGetActivityPriorityTxFn();
  }

#ifdef MAC_ACTIVITY_PROFILING
  if ((activityObject.pPrintActivityInfoFn) && (activityObject.pGetActivityTxFn))
  {
      activityObject.pPrintActivityInfoFn(activityObject.pGetActivityTxFn(), RF_schTxparams.activityInfo, pMacDataTx->internal.frameType, pMacDataTx->internal.dest.dstAddr);
  }
#endif

  RF_cmdPropCs.status = IDLE;
  RF_cmdPropCsSlotted.status = IDLE;

  /* Reconfigure CS parameters as they may be changed by CMD_STOP or LBT setup*/
  RF_cmdPropCs.condition.rule = COND_STOP_ON_TRUE;

  rfOpPtr = (RF_Op *)&RF_cmdPropCs;
  pCb = macRadioCsmaTxCb;

  HAL_ENTER_CRITICAL_SECTION(s);
  if ((txType == MAC_TX_TYPE_UNSLOTTED_CSMA) || (txType == MAC_TX_TYPE_FH_CSMA))
  {
    rfOpPtr = (RF_Op *)&RF_cmdPropCs;
  }
  else if (txType == MAC_TX_TYPE_SLOTTED_CSMA)
  {
    /* Post slotted CSMA/TX command */

    rfOpPtr = (RF_Op *)&RF_cmdPropCsSlotted;

#ifdef DEBUG_SW_TRACE
    {
        volatile uint32_t curvalue;
        curvalue = RF_getCurrentTime();
        DBG_PRINTL2(DBGSYS, "TX: RF_postCmd(startTime=0x%X, RATTime=0x%X)", RF_cmdPropCsSlotted.startTime, curvalue);
    }
#endif /* DEBUG_SW_TRACE */
  }
  else if(txType == MAC_TX_TYPE_NO_CSMA)
  {
      /* Get current PHY Descriptor*/
      pPhyDesc = MAP_macMRFSKGetPhyDesc(pMacPib->curPhyID);
      if (pPhyDesc->ccaType != MAC_CCA_TYPE_LBT)
      {
          rfOpPtr = (RF_Op *)&RF_cmdPropTxAdv;
      }
      /*rfOptPtr by default pointing to CS command for LBT CCA+TX */
  }
  else /*slotted*/
  {
      rfOpPtr = (RF_Op *)&RF_cmdPropTxAdv;
  }

  /* setup the receive ack command to follow the tx command */
  RF_cmdPropTxAdv.pNextOp = NULL;
  RF_cmdPropTxAdv.condition.rule = COND_NEVER;
  if(ackReq)
  {
      RF_cmdPropTxAdv.pNextOp = (rfc_radioOp_t *) &RF_cmdPropRxAdv;
      RF_cmdPropTxAdv.condition.rule = COND_STOP_ON_FALSE;
      RF_cmdPropRxAdv.status = IDLE;
      RF_cmdPropRxAdv.startTrigger.triggerType = TRIG_REL_PREVEND;
      RF_cmdPropRxAdv.startTime = (MAC_AIFS_PERIOD - MAC_AIFS_RX_OFFSET) * MAC_SYMBOL_TO_RAT_RATIO; // start after AIFS time
      RF_cmdPropRxAdv.endTime = macRadioTxGetAckTimeoutVal();
      macTxActive = MAC_TX_ACTIVE_LISTEN_FOR_ACK;
      evts |= (RF_EventRxOk | RF_EventRxBufFull | RF_EventRxNOk | RF_EventMdmSoft);
  }
#ifdef FEATURE_WISUN_EDFE_SUPPORT
  else if(rfrmReq)
  {
      uint32 elapsedSymbol = macSymbolTimerCount() - macRxSymbol;
      pCb = macRadioTxEDFECb;
      RF_cmdPropTxAdv.startTime = (MAC_EDFE_RD_PERIOD - MAC_EDFE_TX_OFFSET - elapsedSymbol) * MAC_SYMBOL_TO_RAT_RATIO;
      RF_cmdPropTxAdv.startTrigger.triggerType = TRIG_REL_SUBMIT;
      RF_cmdPropTxAdv.startTrigger.pastTrig = 1;
      if(frfrm)
      {
          macEdfeDbg.txFFRM++;
          macTxActive = MAC_TX_ACTIVE_POST_EDFE;
          evts |= (RF_EventRxOk|RF_EventMdmSoft);
    //      evts |= (RF_EventRxOk | RF_EventRxBufFull | RF_EventRxNOk | RF_EventMdmSoft);
      }
      else
      {
          macEdfeDbg.txDFRM++;
          RF_cmdPropTxAdv.pNextOp = (rfc_radioOp_t *) &RF_cmdPropRxAdv;
          RF_cmdPropTxAdv.condition.rule = COND_STOP_ON_FALSE;
          RF_cmdPropRxAdv.status = IDLE;
          RF_cmdPropRxAdv.startTrigger.triggerType = TRIG_REL_PREVEND;
          RF_cmdPropRxAdv.startTime = (MAC_EDFE_RD_PERIOD - MAC_EDFE_RX_OFFSET) * MAC_SYMBOL_TO_RAT_RATIO; // start after AIFS time
          RF_cmdPropRxAdv.endTime = 4000 * 10; // 10msec
          macTxActive = MAC_TX_ACTIVE_LISTEN_FOR_EDFE;
          macEdfeInfo.txFrameType = MAC_FRAME_TYPE_EDFE_RDFRM;
          evts |= (RF_EventRxOk|RF_EventMdmSoft);
    //      evts |= (RF_EventRxOk | RF_EventRxBufFull | RF_EventRxNOk | RF_EventMdmSoft);
      }
  }
#endif


  /*
   * Check last posted command and FS command frequency against MAC frequency.
   * Last post command after a FS update will always be RF_cmdFsRx.
   */
  if ((lastPostCmd == (uint32_t*)&RF_cmdFsRx) ||
      (RF_handle->state.mode_state.cmdFs.fractFreq != RF_cmdFsTx.fractFreq) ||
      (RF_handle->state.mode_state.cmdFs.frequency !=  RF_cmdFsTx.frequency)) {

      macFsCmdActive = TRUE;

      /* Chain FS Command */
      RF_cmdFsTx.status = IDLE;
      RF_cmdFsTx.pNextOp = rfOpPtr;
      RF_cmdFsTx.condition.rule = COND_STOP_ON_FALSE;

      RF_cmdFsTx.startTime = 0;
      RF_cmdFsTx.startTrigger.triggerType = TRIG_NOW;

      rfOpPtr = (RF_Op *)&RF_cmdFsTx;
  }

  rfOpPtr->startTime = 0x00;
  rfOpPtr->startTrigger.triggerType = TRIG_NOW;

  /* Update command chain to abs time */
  if ((MAC_FH_ENABLED) && (pMacDataTx && pMacDataTx->internal.frameType != MAC_INTERNAL_ASYNC))
  {
      rfOpPtr->startTime = RF_getCurrentTime();
      rfOpPtr->startTrigger.triggerType = TRIG_ABSTIME;

      RF_schTxparams.startTime = rfOpPtr->startTime;
      RF_schTxparams.startType = RF_StartAbs;
  }

#ifdef FH_HOP_DEBUG_FIXED
    fhDbg.txCh = macPhyChannel;
#endif

  /* Schedule CSMA/TX command */
#ifdef FREERTOS_SUPPORT
  txCbRf = pCb;
  txCmdHandle = RF_scheduleCmd(RF_handle, rfOpPtr,
                               &RF_schTxparams, runTxCb,
                               evts);
#else
  txCmdHandle = RF_scheduleCmd(RF_handle, rfOpPtr,
                               &RF_schTxparams, pCb,
                               evts);
#endif //FREERTOS_SUPPORT
#ifdef DBG_TIMAC_RADIO_CB
  macRadioDbg[macRadDbg_idx].fn = MAC_TX_ISSUE,
  macRadioDbg[macRadDbg_idx].rf_cmd_hnd = txCmdHandle;
  macRadioDbg[macRadDbg_idx].rf_hnd = 0;
  macRadioDbg[macRadDbg_idx].ch_var = 0;
  macRadioDbg[macRadDbg_idx].rf_event = 0;
  macRadioDbg[macRadDbg_idx].sfd_detect = 0;
  macRadioDbg[macRadDbg_idx].clk_ticks = Clock_getTicks();
  macRadioDbg[macRadDbg_idx].rf_cmd_status = 0;
  macRadioDbg[macRadDbg_idx].numRxCmd = numRxCmd;
  macRadioDbg[macRadDbg_idx].numRfCmd = numRfCmd;
  macRadDbg_idx = macRadDbg_idx + 1;
  macRadDbg_idx = macRadDbg_idx % MAX_RADIO_DBG_CNT;
#endif

  if (txCmdHandle < 0)
  {
    DBG_PRINT1(DBGSYS, "TX: RF_ScheduleCmd Rejected(txCmdHandle=%d)", txCmdHandle);

    /* Parse transmission status and update tracking data */
    if (activityObject.pSetActivityTrackingTxFn)
    {
      activityObject.pSetActivityTrackingTxFn(NULL, NULL , RF_EventCmdPreempted);
    }

    HAL_EXIT_CRITICAL_SECTION(s);

    /* For Async frames do not perform a backoff and complete the current Tx */
    if ((MAC_FH_ENABLED) && (pMacDataTx && (pMacDataTx->internal.frameType == MAC_INTERNAL_ASYNC)))
    {
      txComplete(MAC_NO_RESOURCES);
    }

    return;
  }
  else
  {
    DBG_PRINT1(DBGSYS, "TX: RF_ScheduleCmd Complete(txCmdHandle=%d)", txCmdHandle);

    numRfCmd++;
    /* Increment the count in case a RX ACK command is appended */

    /* This might not be the right cmd pointer, but the goal is to
       identify whether the last cmd was RX. */
    lastPostCmd = (uint32_t*)&RF_cmdPropTxAdv;
  }

  HAL_EXIT_CRITICAL_SECTION(s);
}
#endif

/**************************************************************************************************
 * @fn          macRadioTxGo
 *
 * @brief       Run previously radio command for CSMA transmit.  Handles unslotted CSMA transmits.
 *              When transmission has finished, this ISR will call macTxDoneCallback().
 *
 * @param       txType - Slotted, unslotted, FH or LBT.
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioTxGo(uint8 txType)
{
  SELECT_CALL(macRadioTxGo, txType);
}

/**************************************************************************************************
 * @fn          macRadioTxAckCb
 *
 * @brief       This callback is called when TX ACK is complete or stopped.
 *
 * @param       h - radio handle
 *              ch - radio command handle
 *              e - radio event
 *
 * @return      none
 **************************************************************************************************
 */
static void macRadioTxAckCb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
  /* Now we are in swi context */
  bInSwi = TRUE;
 uint32_t event;

#ifdef DBG_TIMAC_RADIO_CB
  macRadioDbg[macRadDbg_idx].fn = MAC_TX_ACK_CB_FN,
  macRadioDbg[macRadDbg_idx].rf_cmd_hnd = txCmdHandle;
  macRadioDbg[macRadDbg_idx].rf_hnd = h;
  macRadioDbg[macRadDbg_idx].ch_var = ch;
  macRadioDbg[macRadDbg_idx].rf_event = e;
  macRadioDbg[macRadDbg_idx].sfd_detect = e & RF_EventMdmSoft;
  macRadioDbg[macRadDbg_idx].clk_ticks = ClockP_getSystemTicks();
  macRadioDbg[macRadDbg_idx].rf_cmd_status = RF_cmdPropTxAdv.status;
  macRadioDbg[macRadDbg_idx].numRxCmd = numRxCmd;
  macRadioDbg[macRadDbg_idx].numRfCmd = numRfCmd;
  macRadDbg_idx = macRadDbg_idx + 1;
  macRadDbg_idx = macRadDbg_idx % MAX_RADIO_DBG_CNT;
#endif

  DBG_PRINT1(DBGSYS, "TX ACK: macRadioTxAckCb(ch=%i)", ch);
  DBG_PRINTL2(DBGSYS, "TX ACK: macRadioTxAckCb(event=0x%08X%08X)", e >> 32, e & 0xFFFFFFFF);

  /* Parse transmission status and update tracking data */
  if (activityObject.pSetActivityTrackingTxFn)
  {
      activityObject.pSetActivityTrackingTxFn(NULL, NULL, e);
  }

  event = RF_EventLastCmdDone;

  if ((e & event) == event)
  {
    /* Normal cases */
    numRfCmd--;
    macTxAckIsr();
    /* Turn on RX if needed */
    macRxOnRequest();
  }
  else if (e & (RF_EventCmdStopped | RF_EventCmdAborted | RF_EventCmdCancelled | RF_EventCmdPreempted))
  {
    numRfCmd--;
    DBG_PRINT0(DBGSYS, "TX ACK: Cleanup after RF_EventCmdStopped!");
    macTxAckIsr();

    /* Turn on RX if needed */
    macRxOnRequest();
  }
  else
  {
    /* Should never get here */
    DBG_PRINT0(DBGSYS, "!!! TX Enhanced ACK Error !!!");
    macAckDbg.ackCbErr++;
    macTxAckIsr();
    macRxOnRequest();
  }

#ifdef ENHACK_MALLOC
  if (pMacEnhancedAck != NULL)
  {
    osal_msg_deallocate( (uint8*)pMacEnhancedAck );
    pMacEnhancedAck  = NULL;

  }
#endif
  macTxCmdActive = FALSE;

  if (numRfCmd == 0)
  {
    /* Power management state may change. Hence, vote. */
    MAP_macPwrVote(FALSE, FALSE);
  }
  else
  {
    MAC_ASSERT(numRfCmd > 0);
    DBG_PRINT1(DBGSYS, "TX ACK: %i RF commands are pending. Can't power down.", numRfCmd);
  }

  /* Now we are exiting the swi */
  bInSwi = FALSE;
}

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
/**************************************************************************************************
 * @fn          macRadioSendImmAckSubG
 *
 * @brief       This function is to prepare for the outgoing Immediate ACK and start a
 *              MAC timer so that the ACK can be sent out at the correct time.
 *
 * @param       sequence number
 * @param       pending bit internal mask
 *
 * @return      none
 **************************************************************************************************
 */
static void macRadioSendImmAckSubG(uint8 seq, uint8 pending)
{
  bool fcsType;
  MAP_MAC_MlmeGetReq(MAC_FCS_TYPE, &fcsType);
  if(fcsType)
  {
    macRadioAckFrame[0] = 0x05;
    /*Enable whitening and 2-byte FCS */
    macRadioAckFrame[1] = MAC_PHY_PHR_FCS_TYPE_MASK | MAC_PHY_PHR_DW_MASK;
  }
  else
  {
    macRadioAckFrame[0] = 0x07;
    /*Enable whitening and 4-byte FCS */
    macRadioAckFrame[1] = 0x08;
  }
  /* Re-initialize the ack frame */
  macRadioAckFrame[2] = 0x02;
  macRadioAckFrame[3] = 0x00;

  /* MAC_FRAME_PENDING */
  if (pending)
  {
    macRadioAckFrame[2] |= 0x10;
  }

  /* Write sequence number to the ACK frame */
  macRadioAckFrame[4] = seq;

  /* Populate part of the command for immediate ACK */
  RF_cmdPropTxAdv.pktLen = sizeof(macRadioAckFrame);
  RF_cmdPropTxAdv.pPkt = &macRadioAckFrame[0];

  /* Reverse bit order in each byte after the header and before the FCS */
  macMcuBitReverse(&macRadioAckFrame[2], sizeof(macRadioAckFrame) - 2);

  /* start Tx Ack */
  MAC_RADIO_REQUEST_TX_ACK();
}
#endif
/**************************************************************************************************
 * @fn          macRadioSendImmAck
 *
 * @brief       This function is to prepare for the outgoing Immediate ACK and start a
 *              MAC timer so that the ACK can be sent out at the correct time.
 *
 * @param       sequence number
 * @param       pending bit internal mask
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioSendImmAck(uint8 seq, uint8 pending)
{
  SELECT_CALL(macRadioSendImmAck, seq, pending);
}

#ifdef FEATURE_ENHANCED_ACK
/**************************************************************************************************
 * @fn          macRadioSendEnhAck
 *
 * @brief       This function is to prepare for the outgoing Enhanced ACK and start a
 *              MAC timer so that the ACK can be sent out at the correct time.
 *
 * @param       pending bit internal mask
 * @param       pointer to dest address
 * @param       pointer to security settings
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API
void macRadioSendEnhAck(uint8 pending, sAddr_t *pDstAddr, ApiMac_sec_t *sec)
{
  uint8_t  status;
  uint16_t txOptions = 0;
#ifdef FH_BENCHMARK_BUILD_TIMES
  uint32 startTs;
#endif

  if (pending)
  {
    txOptions |= MAC_TXOPTION_PEND_BIT;
  }

  /* Prepare the enhanced ACK */
  if (MAC_FH_ENABLED)
  {
#ifdef FH_BENCHMARK_BUILD_TIMES
  startTs = ICall_getTicks();
#endif

  /* the Enhanced ACK pointer will be filled after it is built */
    pMacEnhancedAck = NULL;
    status = MAP_macBuildCommonReq(MAC_INTERNAL_ENH_ACK,
                               pDstAddr->addrMode, 
                               pDstAddr,
                               pMacPib->panId,
                               txOptions,
                               sec);
#ifdef FH_BENCHMARK_BUILD_TIMES
    macBuildAckTime[(macBuildAckIdx++) % NUM_MAC_BENCHMARK_CNT] = ICall_getTicks() -startTs;
#endif

    /* After the enhanced ACK frame is built,
     * macRadioTxAckPrepCb() will be called from high level MAC.
     */
    if (status == MAC_SUCCESS)
    {
      /* prepare and start ACK timer callback */
      MAC_RADIO_REQUEST_TX_ACK();
    }
    else
    {
      macTxAckIsr();
      DBG_PRINT1(DBGSYS, "Enhanced ACK: failure status=0x%X", status);
    }
  }
}

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
/**************************************************************************************************
 * @fn          macRadioTxEnhAckCb
 *
 * @brief       This function populates the Enhanced ACK contents to TX buffer.
 *
 * @param       pMsg pointer to TX buffer
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioTxEnhAckCb(macTx_t *pMsg)
{
  uint8   mrFSKSFD;
  uint8   *p;
  uint16  lenMhrMsdu;

  /* Store the enhanced ACK pointer so it can be freed after TX ACK done */
  pMacEnhancedAck = pMsg;

  /* set length of frame (note: use of term msdu is a misnomer, here it's actually mhr + msdu) */
  lenMhrMsdu = pMsg->msdu.len;

  /* Load the local pointer */
  p = pMsg->msdu.p;

  /* Get the sync word format from PIB */
  MAP_MAC_MlmeGetReq(MAC_PHY_MRFSKSFD, &mrFSKSFD);

  /* Set TX payload length including PHR header length */
  RF_cmdPropTxAdv.pktLen = lenMhrMsdu + MAC_PHY_PHR_LEN;

  /*-------------------------------------------------------------------------------
   *  Set PHR here. MAC_PHY_PHR_LEN spaces are reserved by high level MAC.
   */

  /* The length field in PHR should exclude the header but include CRC length.
   * Whitening is enabled by default to conform to WiSUN.
   * 4-byte CRC is enabled by default to conform to WiSUN.
   * TODO: For 2-byte CRC, need to "or" in MAC_PHY_PHR_FCS_TYPE_MASK.
   */
  bool fcsType;
  uint16 fcsLen;

  MAP_MAC_MlmeGetReq(MAC_FCS_TYPE, &fcsType);
  if(!fcsType)
  {
    lenMhrMsdu += MAC_FCS4_FIELD_LEN;
    fcsLen = MAC_FCS4_FIELD_LEN;
    *(--p) = (HI_UINT16(lenMhrMsdu) & MAC_PHY_PHR_LEN_MSB_MASK) | MAC_PHY_PHR_DW_MASK;
  }
  else
  {
    lenMhrMsdu += MAC_FCS2_FIELD_LEN;
    fcsLen = MAC_FCS2_FIELD_LEN;
    *(--p) = (HI_UINT16(lenMhrMsdu) & MAC_PHY_PHR_LEN_MSB_MASK) | MAC_PHY_PHR_DW_MASK | MAC_PHY_PHR_FCS_TYPE_MASK;
  }

  *(--p) =  LO_UINT16(lenMhrMsdu);

  /* Assign the payload pointer */
  RF_cmdPropTxAdv.pPkt = p;

  /* Reverse bit order in each byte after the header and before the FCS */
  macMcuBitReverse(RF_cmdPropTxAdv.pPkt + MAC_PHY_PHR_LEN, lenMhrMsdu -  fcsLen);

  /* Print out some debug info */
  DBG_PRINT4(DBGSYS, "ACK[0..3]=0x%X, 0x%X, 0x%X, 0x%X", RF_cmdPropTxAdv.pPkt[0], RF_cmdPropTxAdv.pPkt[1], RF_cmdPropTxAdv.pPkt[2], RF_cmdPropTxAdv.pPkt[3]);

  /* For uncoded SFD value on TX and RX */
  RF_cmdPropTxAdv.syncWord = RF_cmdPropRxAdv.syncWord0 =
    (mrFSKSFD == 0)? MAC_2FSK_UNCODED_SFD_0 : MAC_2FSK_UNCODED_SFD_1;
}
#endif
#endif /* FEATURE_ENHANCED_ACK */

/**************************************************************************************************
 * @fn          macRadioTxAck
 *
 * @brief       This function actually sends out the ACK packet. This is a callback from
 *              ACK timer.
 *
 * @param       param - not used
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioTxAck(uint32 txTime)
{
  RF_ScheduleCmdParams txParam;
  RF_ScheduleCmdParams_init(&txParam);

  /* Set activity */
  if (activityObject.pSetActivityTxFn)
  {
      activityObject.pSetActivityTxFn(NULL);
  }

  /* Acquire Activity priority */
  if (activityObject.pGetActivityPriorityTxFn)
  {
      txParam.activityInfo = activityObject.pGetActivityPriorityTxFn();
  }
#ifdef MAC_ACTIVITY_PROFILING
  if ((activityObject.pPrintActivityInfoFn) && (activityObject.pGetActivityTxFn))
  {
      activityObject.pPrintActivityInfoFn(activityObject.pGetActivityTxFn(), txParam.activityInfo, NULL, pMacDataTx->internal.dest.dstAddr);
  }
#endif

  /* If ACK is not canceled during turn around time */
  if (macRxOutgoingAckFlag && !MAC_TX_IS_PHYSICALLY_ACTIVE())
  {
    halIntState_t  s;

    macAckDbg.txAck++;
    /* Send ACK with ACK length and pointer.
     * The SFD should not have changed for ACK.
     */
    RF_cmdPropTxAdv.startTime = txTime;
    RF_cmdPropTxAdv.startTrigger.triggerType = TRIG_REL_SUBMIT;
    RF_cmdPropTxAdv.startTrigger.pastTrig = 1;
    RF_cmdPropTxAdv.pNextOp = NULL;
    RF_cmdPropTxAdv.condition.rule = COND_NEVER;
    macTxCmdActive = TRUE;
    MAC_PWR_VOTE_NO_SWI(TRUE);

    /* RX may be in progress, abort it */
    macRxOff();

    /* Useful to track ICall delay */
    DBG_PRINT0(DBGSYS, "TX ACK: TX ACK command posting");

    HAL_ENTER_CRITICAL_SECTION(s);
    /* Post TX command */
      RF_ScheduleCmdParams RF_schTxparams;
      RF_ScheduleCmdParams_init(&RF_schTxparams);

      /* Acquire Activity priority */
      if (activityObject.pGetActivityPriorityTxFn)
      {
          RF_schTxparams.activityInfo = activityObject.pGetActivityPriorityTxFn();
      }

      RF_schTxparams.endTime = 0;

      /* Calculate command duration */
      RF_schTxparams.duration = macGetRFCmdDuration(RF_cmdPropTxAdv.pktLen, false);
      DBG_PRINT2(DBGSYS, "Tx Command Duration (%x), Frame Type: (%x)", RF_schTxparams.duration, NULL);

#ifdef FREERTOS_SUPPORT
      txCbRf = macRadioTxAckCb;
      txCmdHandle = RF_scheduleCmd(RF_handle, (RF_Op*)&RF_cmdPropTxAdv,
                                   &RF_schTxparams, runTxCb,
                                   RF_EventLastCmdDone);
#else
      txCmdHandle = RF_scheduleCmd(RF_handle, (RF_Op*)&RF_cmdPropTxAdv,
                                   &RF_schTxparams, macRadioTxAckCb,
                                   RF_EventLastCmdDone);
#endif

    if (txCmdHandle >= 0)
    {
        DBG_PRINT0(DBGSYS, "TX ACK: ACK Scheduled!");

      numRfCmd++;
      lastPostCmd = (uint32_t*)&RF_cmdPropTxAdv;
      HAL_EXIT_CRITICAL_SECTION(s);

    }
    else
    {
        macAckDbg.txAckCancel++;
    #ifdef ENHACK_MALLOC
        osal_msg_deallocate( (uint8*)pMacEnhancedAck );
        pMacEnhancedAck = NULL;
    #endif
        macRxOutgoingAckFlag = 0;
        HAL_EXIT_CRITICAL_SECTION(s);

        macTxAckIsr();

        DBG_PRINT0(DBGSYS, "TX ACK: ACK canceled!");
    }
    DBG_PRINT1(DBGSYS, "TX ACK: RF_postCmd(txAckCmdHandle=%i)", txCmdHandle);
  }
  else
  {
    macAckDbg.txAckCancel++;
#ifdef ENHACK_MALLOC
    osal_msg_deallocate( (uint8*)pMacEnhancedAck );
    pMacEnhancedAck = NULL;
#endif
    macRxOutgoingAckFlag = 0;

    macTxAckIsr();

    DBG_PRINT0(DBGSYS, "TX ACK: ACK canceled!");
  }
}


/**************************************************************************************************
 * @fn          macRadioTxRequestTxAck
 *
 * @brief       This function is used to request a ACK transmission after AIFS.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioTxRequestTxAck( void )
{
  uint32 elapsedSymbol;

  DBG_PRINT0(DBGSYS, "macRadioTxRequestTxAck()");

  elapsedSymbol = macSymbolTimerCount() - macRxSymbol;
  // When symbol time is around rollover, this should happen.
  if(elapsedSymbol > MAC_AIFS_PERIOD - MAC_AIFS_RX_OFFSET)
  {
      macAckDbg.ackTimeRollover++;
      // In order not to stuck at rollover, added this routine.
      elapsedSymbol = (MAC_AIFS_PERIOD - MAC_AIFS_RX_OFFSET) >> 2;
  }
  macRadioTxAck((MAC_AIFS_PERIOD - MAC_AIFS_RX_OFFSET - elapsedSymbol) * MAC_SYMBOL_TO_RAT_RATIO);
}


/**************************************************************************************************
 * @fn          macRadioTxCancelTxAck
 *
 * @brief       This function is used to cancel the ACK request. This could mean that the
 *              current received frame is being filtered out.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioTxCancelTxAck( void )
{
/*
 * Since moved TxAck to the end of header and security processing, no need to cancel the ack.
 */
}

#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined (FEATURE_WISUN_MIN_EDFE)
/**************************************************************************************************
 * @fn          macRadioSendEDFEEFRM
 *
 * @brief       This function is to prepare for the outgoing Enhanced ACK and start a
 *              MAC timer so that the ACK can be sent out at the correct time.
 *
 * @param       pending bit internal mask
 * @param       pointer to dest address
 * @param       pointer to security settings
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API
void macRadioSendEDFEEfrm(sAddr_t *pDstAddr, ApiMac_sec_t *sec, uint8 frameType)
{
  uint8_t  status;
  uint16_t txOptions = 0;
#ifdef FH_BENCHMARK_BUILD_TIMES
  uint32 startTs;
#endif

  /* Prepare the EDFE frame */
  if (MAC_FH_ENABLED)
  {
#ifdef FH_BENCHMARK_BUILD_TIMES
  startTs = ICall_getTicks();
#endif

  /* build the EDFE frame */
    status = MAP_macBuildCommonReq(frameType,
                               pDstAddr->addrMode,
                               pDstAddr,
                               pMacPib->panId,
                               txOptions,
                               sec);
#ifdef FH_BENCHMARK_BUILD_TIMES
    macBuildAckTime[(macBuildAckIdx++) % NUM_MAC_BENCHMARK_CNT] = ICall_getTicks() -startTs;
#endif

    /* After the EDFE frame is built,
     * macRadioTxRequestTxEDFEEfrm() will be called from high level MAC.
     */
    if (status == MAC_SUCCESS)
    {
      /* prepare and start EDFE timer callback */
        macRadioTxRequestTxEDFEEfrm(frameType);
    }
    else
    {
      macTxEDFEIsr(frameType);
      DBG_PRINT1(DBGSYS, "Enhanced ACK: failure status=0x%X", status);
    }
  }
}

/**************************************************************************************************
 * @fn          macRadioTxEDFEEfrmCb
 *
 * @brief       This function populates the Enhanced ACK contents to TX buffer.
 *
 * @param       pMsg pointer to TX buffer
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRadioTxEDFEEfrmCb(macTx_t *pMsg)
{
  uint8   mrFSKSFD;
  uint8   *p;
  uint16  lenMhrMsdu;

  /* Store the enhanced ACK pointer so it can be freed after TX ACK done */

  /* set length of frame (note: use of term msdu is a misnomer, here it's actually mhr + msdu) */
  lenMhrMsdu = pMsg->msdu.len;

  /* Load the local pointer */
  p = pMsg->msdu.p;

  /* Get the sync word format from PIB */
  MAP_MAC_MlmeGetReq(MAC_PHY_MRFSKSFD, &mrFSKSFD);

  /* Set TX payload length including PHR header length */
  RF_cmdPropTxAdv.pktLen = lenMhrMsdu + MAC_PHY_PHR_LEN;

  /*-------------------------------------------------------------------------------
   *  Set PHR here. MAC_PHY_PHR_LEN spaces are reserved by high level MAC.
   */

  /* The length field in PHR should exclude the header but include CRC length.
   * Whitening is enabled by default to conform to WiSUN.
   * 4-byte CRC is enabled by default to conform to WiSUN.
   * TODO: For 2-byte CRC, need to "or" in MAC_PHY_PHR_FCS_TYPE_MASK.
   */
  bool fcsType;
  uint16 fcsLen;

  MAP_MAC_MlmeGetReq(MAC_FCS_TYPE, &fcsType);
  if(!fcsType)
  {
    lenMhrMsdu += MAC_FCS4_FIELD_LEN;
    fcsLen = MAC_FCS4_FIELD_LEN;
    *(--p) = (HI_UINT16(lenMhrMsdu) & MAC_PHY_PHR_LEN_MSB_MASK) | MAC_PHY_PHR_DW_MASK;
  }
  else
  {
    lenMhrMsdu += MAC_FCS2_FIELD_LEN;
    fcsLen = MAC_FCS2_FIELD_LEN;
    *(--p) = (HI_UINT16(lenMhrMsdu) & MAC_PHY_PHR_LEN_MSB_MASK) | MAC_PHY_PHR_DW_MASK | MAC_PHY_PHR_FCS_TYPE_MASK;
  }

  *(--p) =  LO_UINT16(lenMhrMsdu);

  /* Assign the payload pointer */
  RF_cmdPropTxAdv.pPkt = p;

  /* Reverse bit order in each byte after the header and before the FCS */
  macMcuBitReverse(RF_cmdPropTxAdv.pPkt + MAC_PHY_PHR_LEN, lenMhrMsdu -  fcsLen);

  /* Print out some debug info */
  DBG_PRINT4(DBGSYS, "ACK[0..3]=0x%X, 0x%X, 0x%X, 0x%X", RF_cmdPropTxAdv.pPkt[0], RF_cmdPropTxAdv.pPkt[1], RF_cmdPropTxAdv.pPkt[2], RF_cmdPropTxAdv.pPkt[3]);

  /* For uncoded SFD value on TX and RX */
  RF_cmdPropTxAdv.syncWord = RF_cmdPropRxAdv.syncWord0 =
    (mrFSKSFD == 0)? MAC_2FSK_UNCODED_SFD_0 : MAC_2FSK_UNCODED_SFD_1;
}

/**************************************************************************************************
 * @fn          macRadioTxEDFECb
 *
 * @brief       This callback is called when TX ACK is complete or stopped.
 *
 * @param       h - radio handle
 *              ch - radio command handle
 *              e - radio event
 *
 * @return      none
 **************************************************************************************************
 */
static void macRadioTxEDFECb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
  /* Now we are in swi context */
  bInSwi = TRUE;
 static RF_EventMask eCsRx = 0;

  DBG_PRINT1(DBGSYS, "TX EDFE: macRadioTxEDFECb(ch=%i)", ch);
  DBG_PRINTL2(DBGSYS, "TX EDFE: macRadioTxEDFECb(event=0x%08X%08X)", e >> 32, e & 0xFFFFFFFF);

  if(e & RF_EventMdmSoft)
  {
      macSfdDetectCback(MAC_SFD_DETECTED);
  }

  if(!(e & RF_EventLastCmdDone))
  {
      eCsRx |= e;
      return;
  }

  /* Parse transmission status and update tracking data */
  if (activityObject.pSetActivityTrackingTxFn)
  {
      activityObject.pSetActivityTrackingTxFn(NULL, NULL, e);
  }

  if (e & RF_EventLastCmdDone)
  {
    /* Normal cases */
    numRfCmd--;
#if !defined(FEATURE_WISUN_MIN_EDFE)
    e |= eCsRx;
    macEdfeDbg.opDone++;
    if(macTxActive == MAC_TX_ACTIVE_LISTEN_FOR_EDFE)
    {
        macTxActive = MAC_TX_ACTIVE_NO_ACTIVITY;
        if((macEdfeInfo.txFrameType == MAC_FRAME_TYPE_EDFE_IFRM) && (RF_cmdPropCs.status != PROP_DONE_IDLE))
        {
            macEdfeDbg.txCCAFail++;
            MAP_FHDATA_compEDFEReq(FHAPI_STATUS_ERR_EDFE_CCA_FAIL);
        }
        else if(e & RF_EventRxOk)
        {
            macRxFrameIsr();
        }
        else
        {
            macEdfeDbg.txNoRFRM++;
            MAP_FHDATA_compEDFEReq(FHAPI_STATUS_ERR_EDFE_NO_RFRM);
        }
    }
    else if(macTxActive == MAC_TX_ACTIVE_POST_EDFE)
    {
        macTxActive = MAC_TX_ACTIVE_NO_ACTIVITY;
        MAP_FHDATA_compEDFEReq(FHAPI_STATUS_SUCCESS);
    }
    else
    {
        macTxActive = MAC_TX_ACTIVE_NO_ACTIVITY;
    }
#else
    macTxActive = MAC_TX_ACTIVE_NO_ACTIVITY;
    //macRxFrameIsr(); // mv: why is this needed; i feel it is not needed and seeing only 1 edfe frame sent when this is commented out vs the case where we see some 20+ attempts
    // check with KV in code review and does it make sense to call macTxEDFEIsr() here or is it called only for falied cases?
#endif
    /* Turn on RX if needed */
    macRxOnRequest();
  }
  else if (e & (RF_EventCmdStopped | RF_EventCmdAborted | RF_EventCmdCancelled | RF_EventCmdPreempted))
  {
    numRfCmd--;
    DBG_PRINT0(DBGSYS, "TX ACK: Cleanup after RF_EventCmdStopped!");
    macEdfeDbg.opStopped++;
    macTxEDFEIsr(0);

    /* Turn on RX if needed */
    macRxOnRequest();
  }
  else
  {
    /* Should never get here */
    DBG_PRINT0(DBGSYS, "!!! TX Enhanced ACK Error !!!");
    macEdfeDbg.opError++;
    macTxEDFEIsr(0);
    macRxOnRequest();
  }

#ifdef ENHACK_MALLOC
  if (pMacEnhancedAck != NULL)
  {
    osal_msg_deallocate( (uint8*)pMacEnhancedAck );
    pMacEnhancedAck  = NULL;

  }
#endif
  macTxCmdActive = FALSE;
  eCsRx = 0;
  if (numRfCmd == 0)
  {
    /* Power management state may change. Hence, vote. */
    MAP_macPwrVote(FALSE, FALSE);
  }
  else
  {
    MAC_ASSERT(numRfCmd > 0);
    DBG_PRINT1(DBGSYS, "TX ACK: %i RF commands are pending. Can't power down.", numRfCmd);
  }

  /* Now we are exiting the swi */
  bInSwi = FALSE;
}
/**************************************************************************************************
 * @fn          macRadioTxRequestTxEDFEEfrm
 *
 * @brief       This function is used to request a ACK transmission after AIFS.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
static void macRadioTxRequestTxEDFEEfrm( uint8 frameType)
{
  uint32 elapsedSymbol;

  DBG_PRINT0(DBGSYS, "macRadioTxRequestTxAck()");

  if(frameType == MAC_FRAME_TYPE_EDFE_IFRM)
  {
      macRxOff();
      if(MAC_RADIO_SET_CHANNEL(txChannel) == TRUE)
      {
          macPhyChannel = txChannel;
          macRadioTxEDFEEfrm(frameType, macEdfeInfo.txStartTime * MAC_RAT_MHZ);
      }
      else
      {
          MAC_ASSERT(0);
      }
  }
  else
  {
      elapsedSymbol = macSymbolTimerCount() - macRxSymbol;
      // When symbol time is around rollover, this should happen.
      if(elapsedSymbol > MAC_EDFE_RD_PERIOD - MAC_EDFE_TX_OFFSET)
      {
          macEdfeDbg.timeRollover++;
          // In order not to stuck at rollover, added this routine.
          elapsedSymbol = (MAC_EDFE_RD_PERIOD - MAC_EDFE_TX_OFFSET) >> 2;
      }
      macRadioTxEDFEEfrm(frameType, (MAC_EDFE_RD_PERIOD - MAC_EDFE_TX_OFFSET - elapsedSymbol) * MAC_SYMBOL_TO_RAT_RATIO);
  }
}

/**************************************************************************************************
 * @fn          macRadioTxEDFEEfrm
 *
 * @brief       This function actually sends out the ACK packet. This is a callback from
 *              ACK timer.
 *
 * @param       param - not used
 *
 * @return      none
 **************************************************************************************************
 */
static void macRadioTxEDFEEfrm(uint8 frameType, uint32 txTime)
{
    RF_Op   *rfOpPtr;
  RF_ScheduleCmdParams txParam;
  RF_ScheduleCmdParams_init(&txParam);

  /* Set activity */
  if (activityObject.pSetActivityTxFn)
  {
      activityObject.pSetActivityTxFn(NULL);
  }

  /* Acquire Activity priority */
  if (activityObject.pGetActivityPriorityTxFn)
  {
      txParam.activityInfo = activityObject.pGetActivityPriorityTxFn();
  }
#ifdef MAC_ACTIVITY_PROFILING
  if ((activityObject.pPrintActivityInfoFn) && (activityObject.pGetActivityTxFn))
  {
      activityObject.pPrintActivityInfoFn(activityObject.pGetActivityTxFn(), txParam.activityInfo, NULL, pMacDataTx->internal.dest.dstAddr);
  }
#endif

  if (/*(macEdfeInfo.rxFrameType || macEdfeInfo.txFrameType) &&*/ !MAC_TX_IS_PHYSICALLY_ACTIVE())
  {
      halIntState_t  s;
      MAC_PWR_VOTE_NO_SWI(TRUE);
      /* RX may be in progress, abort it */
      macRxOff();

      /* Send ACK with ACK length and pointer.
       * The SFD should not have changed for ACK.
       */
      if (frameType == MAC_FRAME_TYPE_EDFE_IFRM)
      {
          macEdfeDbg.txIFRM++;
          rfOpPtr = (RF_Op *)&RF_cmdPropCs;
          RF_cmdPropCs.status = IDLE;
          RF_cmdPropCs.startTime = (txTime > FH_PHY_CCA_TIME) ? (txTime - FH_PHY_CCA_TIME) : 1;
          RF_cmdPropCs.startTrigger.triggerType = TRIG_REL_SUBMIT;
          RF_cmdPropCs.startTrigger.pastTrig = 1;
          RF_cmdPropCs.pNextOp = (rfc_radioOp_t *) &RF_cmdPropTxAdv;
          RF_cmdPropCs.condition.rule = COND_STOP_ON_TRUE;
          RF_cmdPropCs.csConf.bEnaRssi = 1;     //CCA mode 1
          RF_cmdPropCs.csConf.bEnaCorr = 0;
          RF_cmdPropCs.csConf.operation = 0;    // busy if rssi exceeds threshold
          RF_cmdPropCs.csConf.busyOp = 1;       // return when busy detected
          RF_cmdPropCs.csConf.idleOp = 1;       // return when idle detected
          RF_cmdPropCs.csEndTrigger.triggerType = TRIG_REL_START;
          RF_cmdPropCs.csEndTime = 4000 * 2;   // 2 msec
          RF_cmdPropTxAdv.status = IDLE;
          RF_cmdPropTxAdv.startTime = 0;
          RF_cmdPropTxAdv.startTrigger.triggerType = TRIG_NOW;
          RF_cmdPropTxAdv.startTrigger.pastTrig = 1;
          RF_cmdPropTxAdv.pNextOp = (rfc_radioOp_t *) &RF_cmdPropRxAdv;
          RF_cmdPropTxAdv.condition.rule = COND_STOP_ON_FALSE;
          RF_cmdPropRxAdv.status = IDLE;
          RF_cmdPropRxAdv.startTrigger.triggerType = TRIG_REL_PREVEND;
          RF_cmdPropRxAdv.startTime = (MAC_EDFE_RD_PERIOD - MAC_EDFE_RX_OFFSET) * MAC_SYMBOL_TO_RAT_RATIO; // start after AIFS time
          RF_cmdPropRxAdv.endTrigger.triggerType = TRIG_REL_START;
          RF_cmdPropRxAdv.endTime = 4000 * 10; // 10msec
          macTxActive = MAC_TX_ACTIVE_LISTEN_FOR_EDFE;
      }
      else if (frameType == MAC_FRAME_TYPE_EDFE_FFRM)
      {
          macEdfeDbg.txFFRM++;
          rfOpPtr = (RF_Op *)&RF_cmdPropTxAdv;
          RF_cmdPropTxAdv.startTime = txTime;
          RF_cmdPropTxAdv.startTrigger.triggerType = TRIG_REL_SUBMIT;
          RF_cmdPropTxAdv.startTrigger.pastTrig = 1;
          RF_cmdPropTxAdv.pNextOp = (rfc_radioOp_t *) NULL;
          RF_cmdPropTxAdv.condition.rule = COND_NEVER;
          macTxActive = MAC_TX_ACTIVE_POST_EDFE;
      }
      else
      {
          macEdfeDbg.txRFRM++;
          rfOpPtr = (RF_Op *)&RF_cmdPropTxAdv;
          RF_cmdPropTxAdv.startTime = txTime;
          RF_cmdPropTxAdv.startTrigger.triggerType = TRIG_REL_SUBMIT;
          RF_cmdPropTxAdv.startTrigger.pastTrig = 1;
          RF_cmdPropTxAdv.pNextOp = (rfc_radioOp_t *) &RF_cmdPropRxAdv;
          RF_cmdPropTxAdv.condition.rule = COND_STOP_ON_FALSE;
          RF_cmdPropRxAdv.status = IDLE;
          RF_cmdPropRxAdv.startTrigger.triggerType = TRIG_REL_PREVEND;
          RF_cmdPropRxAdv.startTime = (MAC_EDFE_RD_PERIOD - MAC_EDFE_RX_OFFSET) * MAC_SYMBOL_TO_RAT_RATIO; // start after AIFS time
          RF_cmdPropRxAdv.endTrigger.triggerType = TRIG_REL_START;
          RF_cmdPropRxAdv.endTime = 4000 * 10; // 10msec
          macTxActive = MAC_TX_ACTIVE_LISTEN_FOR_EDFE;
      }

      macEdfeInfo.txFrameType = frameType;
      macTxCmdActive = TRUE;

      /* Useful to track ICall delay */
      DBG_PRINT0(DBGSYS, "TX EDFE: TX EDFE command posting");

      HAL_ENTER_CRITICAL_SECTION(s);

      /* Post TX command */

      RF_ScheduleCmdParams RF_schTxparams;
      RF_ScheduleCmdParams_init(&RF_schTxparams);

      /* Acquire Activity priority */
      if (activityObject.pGetActivityPriorityTxFn)
      {
          RF_schTxparams.activityInfo = activityObject.pGetActivityPriorityTxFn();
      }

      RF_schTxparams.endTime = 0;

      /* Calculate command duration */
      RF_schTxparams.duration = macGetRFCmdDuration(RF_cmdPropTxAdv.pktLen, false);
      DBG_PRINT2(DBGSYS, "Tx Command Duration (%x), Frame Type: (%x)", RF_schTxparams.duration, NULL);

#ifdef FREERTOS_SUPPORT
      txCbRf = macRadioTxEDFECb;
      txCmdHandle = RF_scheduleCmd(RF_handle, (RF_Op*)rfOpPtr,
                                  &RF_schTxparams, runTxCb,
                                   RF_EventLastCmdDone|RF_EventRxOk|RF_EventMdmSoft);
#else
      txCmdHandle = RF_scheduleCmd(RF_handle, (RF_Op*)rfOpPtr,
                                  &RF_schTxparams, macRadioTxEDFECb,
                                   RF_EventLastCmdDone|RF_EventRxOk|RF_EventMdmSoft);
#endif //FREERTOS_SUPPORT


      if (txCmdHandle >= 0)
      {
          numRfCmd++;
          lastPostCmd = (uint32_t*)&RF_cmdPropTxAdv;
      }
      DBG_PRINT1(DBGSYS, "TX EDFE: RF_scheduleCmd(txCmdHandle=%i)", txCmdHandle);
      HAL_EXIT_CRITICAL_SECTION(s);
  }
  else
  {
      macEdfeDbg.txFail++;
#ifdef ENHACK_MALLOC
      osal_msg_deallocate( (uint8*)pMacEnhancedAck );
      pMacEnhancedAck = NULL;
#endif
      macEdfeInfo.rxFrameType = 0;
      macTxEDFEIsr(0);
      DBG_PRINT0(DBGSYS, "TX EDFE: EDFE failed to send!");
  }
}
#endif

#ifdef FREERTOS_SUPPORT
/* POSIX Header files */
#include <semaphore.h>

uint32_t txCount = 0;
void runTxCb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    //pushing to rf queue
    macRfQueue[wRfPtr].rfCbHndl = h;
    macRfQueue[wRfPtr].rfCbCmdHndl = ch;
    macRfQueue[wRfPtr].rfCbEventMask = e;
    macRfQueue[wRfPtr].rfCb = txCbRf;
    wRfPtr = (wRfPtr + 1) % macRfQueueSize;
    numRfCmdQueued++;
    txCount++;

    /* Unblock the task by releasing the semaphore. */
    sem_post(&rfCbSemHandle);
}
#endif //FREERTOS_SUPPORT
/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */

#if (MAC_TX_TYPE_SLOTTED_CSMA != 0)
#error "WARNING!  This define value changed.  It was selected for optimum performance."
#endif

#if (HAL_MAC_TX_SLOTTED_DELAY > MAC_A_BASE_SLOT_DURATION)
#error "ERROR!  HAL_MAC_TX_SLOTTED_DELAY too big.  May cause the slotted frame to be sent one slot early."
#endif

/**************************************************************************************************
*/
