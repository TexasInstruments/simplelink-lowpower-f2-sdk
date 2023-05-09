/******************************************************************************

 @file  mac.c

 @brief This file contains the data structures and APIs for CC13xx
        RF Core Firmware Specification for IEEE 802.15.4.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2009-2023, Texas Instruments Incorporated

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

/*******************************************************************************
 * INCLUDES
 */

#ifndef LIBRARY
#include "ti_wisunfan_features.h"
#endif

#include <stdint.h>
#include <rf_mailbox.h>
#include "rf_mac_api.h"
#include "crypto_mac_api.h"
#include "mac_user_config.h"

#include "hw_memmap.h"
#include "hw_aon_rtc.h"
#include "hal_types.h"
#include "hal_defs.h"

#include "saddr.h"

#include "mac_high_level.h"
#include "mac.h"
#include "mac_activity.h"
#include "mac_spec.h"
#include "mac_mem.h"
#include "mac_pib.h"
#include "mac_rx.h"
#include "mac_tx.h"
#include "mac_rx_onoff.h"
#include "mac_settings.h"
#include "mac_radio.h"
#include "mac_radio_defs.h"
#include "mac_symbol_timer.h"

/* High Level MAC */
#include "mac_timer.h"
#include "mac_beacon.h"
#include "mac_main.h"
#include "mac_data.h"
#include "mac_mgmt.h"
#include "macs.h"
#include "mac_low_level.h"
#include "mac_hl_patch.h"

/* Driverlib */
#include <hw_prcm.h>
#include <hw_rfc_pwr.h>

/* Activity Tracking */
#include "mac_activity.h"

/* debug */
#include "mac_assert.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

#include "rom_jt_154.h"
/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

#define MAX_REQACCESS_CNT         1

/*******************************************************************************
 * GLOBAL VARIABLES
 */
uint32_t* lastPostCmd = NULL; /* Track the last RF post cmd */

/*******************************************************************************
 * LOCAL VARIABLES
 */
macSfdDetectCBack_t macSfdDetectCback = NULL;

/* WA to re-initialize RAT compare on the channel in use */
volatile int8_t lastRatChanA = RF_ALLOC_ERROR;
volatile int8_t lastRatChanB = RF_ALLOC_ERROR;

#if defined(RCN_APP_ASSERT)
static APPASRTINTF_assrtHndlerFunc_t assertFP = {NULL};
#endif

/*******************************************************************************
 * GLOBAL VARIABLES
 */
/* Radio command active */
volatile uint8_t macRxCmdActive = FALSE;
volatile uint8_t macTxCmdActive = FALSE;
volatile uint8_t macFsCmdActive = FALSE;

/* MAC TX timestamp */
uint32_t macTxTimestamp;

/* Reveive output buffer */
#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
rfc_propRxOutput_t macRxOutputSubG;
#endif

/* MAC RF front end configuration. Use a pointer to void since
 * the format changes from package to package.
 */
macUserCfg_t *pMacRadioConfig;

/* MAC frame filter */
macFrmFilter_t macFrmFilter;

/* RX command handle */
RF_CmdHandle rxCmdHandle;

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
uint16 macReqAccessCnt = 0;
uint8 macReqAccessTimeCnt = 0;
uint8 reqAccessAlloc = 0;
uint16 macReqAccErrCnt = 0;
#endif

/* Number of pending RF command */
volatile int8_t numRfCmd = 0;

/* Number of pending RX command */
volatile int8_t numRxCmd = 0;

volatile int8_t numRxPostCmd = 0;

#if defined(RFCORE49)
/* Debug count for timedout RX commands */
static uint8 mac_rx_timeoutCnt=0;
#endif

/* RF driver function table */
uint32_t *macRfDrvTblPtr;

/* Tx/Rx backoff Timer Id's */
static MAP_ICall_TimerID macTxBackoffRtcTimerId, macRxBackoffRtcTimerId;

#ifdef DBG_TIMAC_RADIO_CB
uint8_t macRadDbg_idx = 0;
macRadioDbg_t macRadioDbg[MAX_RADIO_DBG_CNT] = {0};
#endif

/*******************************************************************************
 * LOCAL FUNCTIONS
 */
#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
static void macSetCurrChan( void );
#endif

#ifdef FREERTOS_SUPPORT
void runRxCb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);
#endif //FREERTOS_SUPPORT
/*******************************************************************************
 * @fn          macSetupFsCmd
 *
 * @brief       This function sets up the FS Cmd.
 *
 * input parameters
 *
 * @param       channel - logical channel for this frequency band.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void macSetupFsCmd( uint8 channel )
{
  uint32 frequency;
  macMRFSKPHYDesc_t *pPhyDesc;
#ifdef FH_HOP_DEBUG_FIXED
  channel = 11;
#endif
  pPhyDesc = MAP_macMRFSKGetPhyDesc(pMacPib->curPhyID);
  frequency = pPhyDesc->firstChCentrFreq + pPhyDesc->channelSpacing * channel;

  /* Prepare for the FS command per channel */
  RF_cmdFsRx.frequency = (uint16)(frequency / 1000);
  RF_cmdFsRx.fractFreq =
    (uint16)((frequency - 1000 * RF_cmdFsRx.frequency) * 8192 / 125);
  RF_cmdFsTx.frequency = RF_cmdFsRx.frequency;
  RF_cmdFsTx.fractFreq = RF_cmdFsRx.fractFreq;
}


/*******************************************************************************
 * @fn          macSetupReceiveCmd
 *
 * @brief       This routine is used to setup MAC receive command.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void macSetupReceiveCmd( void )
{
  uint8  mrFSKSFD;

  /* Get the sync word format from PIB */
  MAP_MAC_MlmeGetReq(MAC_PHY_MRFSKSFD, &mrFSKSFD);

  /* Set receive queue and output buffer */
  RF_cmdPropRxAdv.pQueue = (dataQueue_t*)&macRxDataEntryQueue;
  RF_cmdPropRxAdv.pOutput = (uint8_t *)&macRxOutputSubG;

  /* For uncoded SFD value on RX and ACK TX */
  RF_cmdPropRxAdv.syncWord0 = RF_cmdPropTxAdv.syncWord =
    (mrFSKSFD == 0)? MAC_2FSK_UNCODED_SFD_0 : MAC_2FSK_UNCODED_SFD_1;
}

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
static void macSetCurrChan( void )
{
  /* Previous FS and RX have failed. Issue a new one. */
  if (MAC_FH_ENABLED)
  {
    MAC_RADIO_SET_CHANNEL(macPhyChannel);
  }
}
#endif

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
/**************************************************************************************************
 * @fn          macRxCbSubG
 *
 * @brief       This callback is called when a RX radio command is executed.
 *
 * @param       h  - RF handle
 *              ch - RF command handle
 *              e  - RF event mask
 *
 * @return      none
 **************************************************************************************************
 */
static void macRxCbSubG(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
  static RF_CmdHandle ch_prev = RF_ALLOC_ERROR;
  static RF_EventMask e_prev = 0;
  static uint16 s_prev = IDLE;

  /* Now we are in swi context */
  bInSwi = TRUE;

  DBG_PRINT1(DBGSYS, "RX: macRxCb(ch=%i)", ch);
  DBG_PRINTL2(DBGSYS, "RX: macRxCb(event=0x%08X%08X)", (e >> 32), e & 0xFFFFFFFF);
  DBG_PRINT1(DBGSYS, "RX: macRxCb(status=0x%X)", RF_cmdPropRxAdv.status);
  /* FS command should have been completed or cancelled */
  macFsCmdActive = FALSE;
#ifdef DBG_TIMAC_RADIO_CB
  macRadioDbg[macRadDbg_idx].fn = MAC_RX_CB_FN,
  macRadioDbg[macRadDbg_idx].rf_cmd_hnd = rxCmdHandle;
  macRadioDbg[macRadDbg_idx].rf_hnd = h;
  macRadioDbg[macRadDbg_idx].ch_var = ch;
  macRadioDbg[macRadDbg_idx].rf_event = e;
  macRadioDbg[macRadDbg_idx].sfd_detect = e & RF_EventMdmSoft;
  macRadioDbg[macRadDbg_idx].clk_ticks = ClockP_getSystemTicks();
  macRadioDbg[macRadDbg_idx].rf_cmd_status = RF_cmdPropRxAdv.status;
  macRadioDbg[macRadDbg_idx].numRxCmd = numRxCmd;
  macRadioDbg[macRadDbg_idx].numRfCmd = numRfCmd;
  macRadDbg_idx = macRadDbg_idx + 1;
  macRadDbg_idx = macRadDbg_idx % MAX_RADIO_DBG_CNT;
#endif

  if (e & RF_EventMdmSoft)
  {
    /* Invalidate saved command handle */
    ch_prev = RF_ALLOC_ERROR;

    if (MAC_FH_ENABLED)
    {
      DBG_PRINT3(DBGSYS, "RX: SFD Detected, macRxActive=%d, macTxACtive=%d, macRxAckFlag=%d",macRxActive, macTxActive, macRxOutgoingAckFlag);

      if((macTxActive != MAC_TX_ACTIVE_GO))
      {
        /* To prevent If no EventRxOK or NOK Cbs, will get stuck at MAC_RX_ACTIVE_STARTED */
        macRxActive = MAC_RX_ACTIVE_STARTED;
      }
      if (macSfdDetectCback != NULL)
      {
        /* Jump to FH module */
        (macSfdDetectCback)(MAC_SFD_DETECTED);
        ch_prev = ch;

      }
    }
  }
  else if (e & (RF_EventCmdCancelled | RF_EventCmdStopped | RF_EventCmdAborted))
  {
      if(numRxCmd > 0)
      {
          numRxCmd--;
      }

    /* Update activity tracking information if necessary */
    if ((activityObject.pSetActivityTrackingRxFn) && (e & RF_EventCmdPreempted))
    {
        activityObject.pSetActivityTrackingRxFn(NULL, false);
    }

    if (e & RF_EventCmdPreempted)
    {
        /* Start the next RX command only if allowed. */
        macRxOnRequest();
    }

    /* If Radio issues RxAborted Event, then MAC issues RF_Cancel cmd. RF Driver does not
     * give a RF_LastCmdDone Event. Hence, we need to issue rxOnRequest here
     */

    if(e_prev & RF_EventRxAborted)
    {
        macRxOnRequest();
        e_prev = e;
    }

    /* Invalidate saved command handle */
    if (ch_prev == ch)
    {
#ifdef FH_HOP_DEBUG
      macSfdDbg.num_sfd_rx_stop++;
#endif
      if ((e_prev == RF_EventRxOk && s_prev == PROP_DONE_OK) ||
          (e_prev == RF_EventRxAborted && s_prev == ACTIVE))
      {
        /* This is not an abort originally intended.
         * Treat this as RF_EventRxNok.
         */
        DBG_PRINT0(DBGSYS, "RX: Treat unexpected RF_EventRxOk/RF_EventLastCmdDone as RF_EventRxNOk");
        macRxNokIsr();

        /* Start the next RX command only if allowed. */
        macRxOnRequest();
      }
      else
      {
        macRxHaltCleanup();
      }
    }

    ch_prev = RF_ALLOC_ERROR;

    if(numRfCmd > 0)
    {
        numRfCmd--;
    }
  }
  else if (e & (RF_EventRxOk | RF_EventRxBufFull | RF_EventRxNOk))
  {
    /* Save this command handle, event and status */
    ch_prev = ch;
    e_prev = e;
    s_prev = RF_cmdPropRxAdv.status;

    /* The last RX might have been done successfully, but we reject this
     * so that it will be handled as an error when CmdStopped or CmdAborted comes.
     * This happens very rare, so is barely going to impact the performance.
     */
    if ((e == RF_EventRxOk) && (RF_cmdPropRxAdv.status == PROP_DONE_OK))
    {
      DBG_PRINT0(DBGSYS, "RX: RF_EventRxOk without RF_EventLastCmdDone");
      RF_cancelCmd(RF_handle, ch, FALSE);
    }
  }
  else if (e & (RF_EventRxIgnored | RF_EventRxEmpty |RF_EventRxAborted |
                RF_EventRxCollisionDetected | RF_EventInternalError))
  {
    /* Save this command handle and event */
    ch_prev = ch;
    e_prev = e;
    s_prev = RF_cmdPropRxAdv.status;

    if ((e == RF_EventRxAborted) && (RF_cmdPropRxAdv.status == ACTIVE || RF_cmdPropRxAdv.status == IDLE))
    {
      /* In FH case, we have to forcefully abort the on-going RX if it was once aborted
       * for some reason and restarted.
       */
      DBG_PRINT0(DBGSYS, "RX: RF_EventRxAborted without Abort request");
      RF_cancelCmd(RF_handle, ch, FALSE);
    }
  }

  /* RF_EventLastCmdDone can come alone or with another event */
  if (e & RF_EventLastCmdDone)
  {
#ifdef RFCORE49
    if((RF_cmdPropRxAdv.status == PROP_DONE_RXTIMEOUT) ||
       (RF_cmdPropRxAdv.status == PROP_DONE_STOPPED) ||
       (RF_cmdFsRx.status == ERROR_SYNTH_PROG))
    {
      mac_rx_timeoutCnt++;
      if(numRxCmd > 0)
      {
          numRxCmd--;
      }
      macRxHaltCleanup();
      macSetCurrChan();
    }
    else
#endif
    {
      /* The status has changed. There might be a pending RX command in the same location.
       * Reset the status to IDLE before CM0 process a possible pending RX command.
       */
      if(numRxCmd > 0)
      {
          numRxCmd--;
      }

      /* If a desired event is followed by RF_EventLastCmdDone, process it */
      if (ch_prev == ch)
      {
        if (e_prev & (RF_EventRxOk | RF_EventRxBufFull))
        {
          /* Packet received okay or RX buffer full. In case of RX buffer full,
           * the macRxFrameIsr() will attempt to allocate another OSAL buffer
           * for the last RX frame which was stuck in the macRxDataEntryQueue.
           */
#ifdef FH_HOP_DEBUG_FIXED
    fhDbg.rxCh = macPhyChannel;
#endif
          macRxFrameIsr();
        }
        else if (e_prev & RF_EventRxNOk)
        {
          /* CRC error - radio will auto flush */
          macRxNokIsr();
        }
        else
        {
#ifdef FH_HOP_DEBUG
          macSfdDbg.num_sfd_rx_abort++;
#endif
          macRxHaltCleanup();
        }
      }
      else /* shouldn't be here */
      {
        if (RF_cmdPropRxAdv.status == PROP_DONE_SYNTH_ERROR)
        {
          /* Previous FS and RX have failed. Issue a new one. */
    	  macSetCurrChan();
        }
        else /* shouldn't be here */
        {
          /**
           *  !!! removing the MAC_ASSERT(0) call, as there are
           *  conditions when the RF driver sends the
           *  RF_EventLastCmdDone without a corresponding error event.
          */
          DBG_PRINT1(DBGSYS, "RX before assert: macRxCb(ch=%i)", ch);
          DBG_PRINT1(DBGSYS, "RX before assert: macRxCb(chPrev=%i)", ch_prev);
          DBG_PRINTL2(DBGSYS, "RX before assert: macRxCb(event=0x%08X%08X)", (e >> 32), e & 0xFFFFFFFF);
        }
      }
    }

    RF_cmdPropRxAdv.status = IDLE;
    /* Invalidate saved command handle */
    ch_prev = RF_ALLOC_ERROR;

    /* Start the next RX command only if allowed. */
    if (MAC_FH_ENABLED)
    {
      if (!macRxOutgoingAckFlag)
      {
        macRxOnRequest();
      }
    }

    if(numRfCmd > 0)
    {
        numRfCmd--;
    }
  }

  if (numRxCmd == 0)
  {
    /* RX command is not active */
    macRxCmdActive = FALSE;
  }

  if (numRfCmd == 0)
  {
    /* If there is no pending RF command, try to power down */
    MAP_macPwrVote(FALSE, FALSE);
  }
  else if (e & (RF_EventCmdCancelled | RF_EventCmdStopped | RF_EventCmdAborted | RF_EventLastCmdDone))
  {
    MAC_ASSERT(numRfCmd > 0);
    DBG_PRINT1(DBGSYS, "RX: %i RF commands are pending. Can't power down.", numRfCmd);
  }

  /* Now we are exiting the swi */
  bInSwi = FALSE;
}
#endif

/**************************************************************************************************
 * @fn          macRxCb
 *
 * @brief       This callback is called when a RX radio command is executed.
 *
 * @param       h  - RF handle
 *              ch - RF command handle
 *              e  - RF event mask
 *
 * @return      none
 **************************************************************************************************
 */
void macRxCb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    SELECT_CALL(macRxCb, h, ch, e);
}

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
/*******************************************************************************
 * @fn          macSendReceiveCmdSubG
 *
 * @brief       This routine is used to send MAC receive command.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      TRUE if successful, FALSE otherwise
 */
bool macSendReceiveCmdSubG( void )
{
#ifndef FEATURE_WISUN_EDFE_SUPPORT
  if ( (lastPostCmd != (uint32_t*)&RF_cmdPropRxAdv)  || (numRxCmd == 0) )
#else
  if ( ((macTxActive != MAC_TX_ACTIVE_LISTEN_FOR_EDFE) && (macTxActive != MAC_TX_ACTIVE_LISTEN_FOR_ACK))
    &&  ((lastPostCmd != (uint32_t*)&RF_cmdPropRxAdv) || (numRxCmd == 0)) )
#endif
  {
    halIntState_t  s;
    RF_EventMask evtMask = RF_EventRxOk | RF_EventRxNOk | RF_EventRxBufFull;
    RF_Op   *rfOpPtr;

    HAL_ENTER_CRITICAL_SECTION(s);

    /* Post the RX cmd */
    RF_cmdPropRxAdv.status = IDLE;

    if (MAC_FH_ENABLED)
    {
      evtMask |= RF_EventMdmSoft | RF_EventRxIgnored | RF_EventRxEmpty | RF_EventRxAborted |
                 RF_EventRxCollisionDetected | RF_EventInternalError | RF_EventCmdPreempted;
    }
    RF_ScheduleCmdParams RF_schRxparams;
    RF_ScheduleCmdParams_init(&RF_schRxparams);

    RF_schRxparams.endTime = 0;
    /* No end time specified, MAC will abort running Rx when necessary */
    RF_schRxparams.endType = RF_EndInfinit;

    /* Calculate command duration, 125 = MAC_MAX_FRAME_SIZE for IEEE mode used as basis
     * for all Rx command durations for simplicity
     */
    if (macRxEnableFlags & MAC_RX_WHEN_IDLE)
    {
        RF_schRxparams.duration = 0;
    }
    else
    {
        RF_schRxparams.duration = macGetRFCmdDuration(125, false);
        RF_schRxparams.endType = RF_EndNotSpecified;
    }

    DBG_PRINT2(DBGSYS, "Rx Command Duration (%x), Frame Type: (%x)", RF_schRxparams.duration, NULL);

    /* Set activity for receive */
    if (activityObject.pSetActivityRxFn)
    {
        activityObject.pSetActivityRxFn(macRxEnableFlags);
    }

    /* Acquire Activity priority */
    if (activityObject.pGetActivityPriorityRxFn)
    {
        RF_schRxparams.activityInfo = activityObject.pGetActivityPriorityRxFn();
    }

#ifdef MAC_ACTIVITY_PROFILING
   sAddr_t dummy = {0};
   if ((activityObject.pPrintActivityInfoFn) && (activityObject.pGetActivityRxFn))
   {
       activityObject.pPrintActivityInfoFn(activityObject.pGetActivityRxFn(), RF_schRxparams.activityInfo, NULL, dummy);
   }
#endif

    /* For non rx-Ack, reset absolute time settings */
    RF_cmdPropRxAdv.startTime = 0x00;
    RF_cmdPropRxAdv.startTrigger.triggerType = TRIG_NOW;
    RF_cmdPropRxAdv.endTime = 1680000000;   //7 minutes

    rfOpPtr = (RF_Op *)&RF_cmdPropRxAdv;

    /*
     * Check last posted command and FS command frequency against MAC frequency.
     * Last post command after a FS update will always be RF_cmdFsRx.
     */
    if ((lastPostCmd == (uint32_t*)&RF_cmdFsRx) ||
        (RF_handle->state.mode_state.cmdFs.fractFreq != RF_cmdFsRx.fractFreq) ||
        (RF_handle->state.mode_state.cmdFs.frequency !=  RF_cmdFsRx.frequency)) {

        macFsCmdActive = TRUE;

        /* Chain FS Command */
        RF_cmdFsRx.status = IDLE;
        RF_cmdFsRx.pNextOp = rfOpPtr;
        RF_cmdFsRx.condition.rule = COND_STOP_ON_FALSE;
        RF_cmdFsRx.startTime = 0x00;
        RF_cmdFsRx.startTrigger.triggerType = TRIG_NOW;

        rfOpPtr = (RF_Op *)&RF_cmdFsRx;
    }


    /* Switch to absolute time for FH and Beacon Rx */
    if (((MAC_FH_ENABLED) || (macRxEnableFlags & MAC_RX_BEACON_DEVICE)) && !(MAC_TX_IS_PHYSICALLY_ACTIVE()))
    {
        /* Update command chain to abs time */
        rfOpPtr->startTime = RF_getCurrentTime();
        rfOpPtr->startTrigger.triggerType = TRIG_ABSTIME;

        RF_schRxparams.startTime = rfOpPtr->startTime;
        RF_schRxparams.startType = RF_StartAbs;
    }

#ifdef FREERTOS_SUPPORT
    rxCmdHandle = RF_scheduleCmd(RF_handle, (RF_Op*) rfOpPtr, &RF_schRxparams, runRxCb, evtMask);
#else
    rxCmdHandle = RF_scheduleCmd(RF_handle, (RF_Op*) rfOpPtr, &RF_schRxparams, macRxCb, evtMask);
#endif

    DBG_PRINT1(DBGSYS, "RX: RF_scheduleCmd(rxCmdHandle=%d)", rxCmdHandle);

    if (rxCmdHandle < 0)
    {
      HAL_EXIT_CRITICAL_SECTION(s);
      DBG_PRINT0(DBGSYS, "Rx: Command Rejected!");

      /* Update activity tracking information if necessary */
      if (activityObject.pSetActivityTrackingRxFn)
      {
          activityObject.pSetActivityTrackingRxFn(NULL, false);
      }

      /* Start backoff for Rx */
      macRequestBackoff(BACKOFF_TYPE_RX, MAC_REJECTED_PKT_BACKOFF);

      return FALSE;
    }

    numRxCmd++;
    numRfCmd++;
    lastPostCmd = (uint32_t*)&RF_cmdPropRxAdv;

    /* Cancel future backoff if successfully scheduled */
    macRequestBackoff(BACKOFF_TYPE_RX, 0);

    /* Configure PanCoordinator */
    MAC_RADIO_SET_PAN_COORDINATOR(macPanCoordinator);

    /* Set local addresses and PANID are send by macSendReceiveCmd() */
    MAC_RADIO_SET_PAN_ID(pMacPib->panId);
    MAC_RADIO_SET_SHORT_ADDR(pMacPib->shortAddress);
    MAC_RADIO_SET_EXT_ADDR(pMacPib->extendedAddress.addr.extAddr);

#ifdef DBG_TIMAC_RADIO_CB
  macRadioDbg[macRadDbg_idx].fn = MAC_RX_ISSUE,
  macRadioDbg[macRadDbg_idx].rf_cmd_hnd = rxCmdHandle;
  macRadioDbg[macRadDbg_idx].rf_hnd = RF_handle;
  macRadioDbg[macRadDbg_idx].ch_var = 0;
  macRadioDbg[macRadDbg_idx].rf_event = 0;
  macRadioDbg[macRadDbg_idx].sfd_detect = 0;
  macRadioDbg[macRadDbg_idx].clk_ticks = ClockP_getSystemTicks();
  macRadioDbg[macRadDbg_idx].rf_cmd_status = 0;
  macRadioDbg[macRadDbg_idx].numRxCmd = numRxCmd;
  macRadioDbg[macRadDbg_idx].numRfCmd = numRfCmd;
  macRadDbg_idx = macRadDbg_idx + 1;
  macRadDbg_idx = macRadDbg_idx % MAX_RADIO_DBG_CNT;
#endif

    HAL_EXIT_CRITICAL_SECTION(s);

    return TRUE;
  }

#ifdef DBG_TIMAC_RADIO_CB
  macRadioDbg[macRadDbg_idx].fn = MAC_RX_ISSUE,
  macRadioDbg[macRadDbg_idx].rf_cmd_hnd = rxCmdHandle;
  macRadioDbg[macRadDbg_idx].rf_hnd = RF_handle;
  macRadioDbg[macRadDbg_idx].ch_var = 0;
  macRadioDbg[macRadDbg_idx].rf_event = 0;
  macRadioDbg[macRadDbg_idx].sfd_detect = 1;
  macRadioDbg[macRadDbg_idx].clk_ticks = ClockP_getSystemTicks();
  macRadioDbg[macRadDbg_idx].rf_cmd_status = 0;
  macRadioDbg[macRadDbg_idx].numRxCmd = numRxCmd;
  macRadioDbg[macRadDbg_idx].numRfCmd = numRfCmd;
  macRadDbg_idx = macRadDbg_idx + 1;
  macRadDbg_idx = macRadDbg_idx % MAX_RADIO_DBG_CNT;
#endif

  return FALSE;
}
#endif

/*******************************************************************************
 * @fn          macSendReceiveCmd
 *
 * @brief       This routine is used to send MAC receive command.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      TRUE if successful, FALSE otherwise
 */
bool macSendReceiveCmd( void )
{
    SELECT_RCALL(macSendReceiveCmd);
}

/*******************************************************************************
 * @fn          macStopCmd
 *
 * @brief       This call is used to gracefully stop all foreground and
 *              background level commands that are running. This will cause
 *              the BG and FG commands to stop as soon as possible after
 *              finishing ongoing operations.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void macStopCmd(bool bGraceful )
{
    halIntState_t  s;

#ifdef DBG_TIMAC_RADIO_CB
  macRadioDbg[macRadDbg_idx].fn = MAC_ABORT_ISSUE,
  macRadioDbg[macRadDbg_idx].rf_cmd_hnd = rxCmdHandle;
  macRadioDbg[macRadDbg_idx].rf_hnd = RF_handle;
  macRadioDbg[macRadDbg_idx].ch_var = 0;
  macRadioDbg[macRadDbg_idx].rf_event = 0;
  macRadioDbg[macRadDbg_idx].sfd_detect = bGraceful;
  macRadioDbg[macRadDbg_idx].clk_ticks = ClockP_getSystemTicks();
  macRadioDbg[macRadDbg_idx].rf_cmd_status = 0;
  macRadioDbg[macRadDbg_idx].numRxCmd = numRxCmd;
  macRadioDbg[macRadDbg_idx].numRfCmd = numRfCmd;
  macRadDbg_idx = macRadDbg_idx + 1;
  macRadDbg_idx = macRadDbg_idx % MAX_RADIO_DBG_CNT;
#endif

    HAL_ENTER_CRITICAL_SECTION(s);

    /* Stop any ongoing Rx backoff */
    macRequestBackoff(BACKOFF_TYPE_RX, 0);

    //RF_getInfo(RF_handle, RF_GET_RADIO_STATE, &info);

    /* Should not flush the command queue while radio is not active.
     * There should not be any pending command to abort/stop
     * if the radio has already gone to power-down state.
     */
    //if (info.bRadioState)
    {
      /* Reconfigure the rule so the CMD_STOP won't trigger a TX nor Rx*/
      RF_cmdPropCs.condition.rule = COND_NEVER;

#ifdef DEBUG_SW_TRACE
      RF_Stat stat = RF_flushCmd(RF_handle, RF_CMDHANDLE_FLUSH_ALL, bGraceful);
      DBG_PRINT1(DBGSYS, "RF_flushCmd(stat=0x%X)", stat);
#else
      DBG_PRINT0(DBGSYS, "Abort Issued!");

      (void) RF_flushCmd(RF_handle, RF_CMDHANDLE_FLUSH_ALL, bGraceful);
#endif /* DEBUG_SW_TRACE */
      HAL_EXIT_CRITICAL_SECTION(s);
    }
}
/*******************************************************************************
 * @fn          macRxQueueFlushCmd
 *
 * @brief       On reception, the radio CPU shall flush the RX queue. Note that
 *              for radio RX overflow, the RX queue is flushed by CM0.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void macRxQueueFlushCmd( void )
{
  rfc_CMD_FLUSH_QUEUE_t rfCmd;

  /* setup an immediate command CMD_FLUSH_QUEUE */
  rfCmd.commandNo   = CMD_FLUSH_QUEUE;
  rfCmd.pQueue   = &macRxDataEntryQueue;
  rfCmd.pFirstEntry   = (uint8 *)macRxDataEntryQueue.pCurrEntry;

  /* issue command */
  (void) RF_runImmediateCmd(RF_handle, (uint32_t*) &rfCmd);

  return;
}


/*******************************************************************************
 * @fn          macFreqSynthCmd
 *
 * @brief       This radio command is used to control Frequency Synthesizer.
 *
 * input parameters
 *
 * @param       channel - logic channel number.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void macFreqSynthCmd( uint8 channel )
{
  /* For test only, if Tx is needed without any underlying Rx operation
   * (i.e., no CSMA-CA), then CMD_FS (synthConf.bTxMode = 1) is necessary,
   * followed by CMD_IEEE_TX and CMD_FS_OFF. This would not be used frequently,
   * except maybe for testing, as Tx without CSMA-CA is only allowed for beacon
   * frames and ACKs.
   */
  (void) channel;
  return;
}

/*******************************************************************************
 * @fn          macSendFsCmd
 *
 * @brief       On reception, the radio CPU shall set the Frequency Synth command.
 *
 * input parameters
 *
 * @param       channel - logical channel for this frequency band.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      TRUE if successful, FALSE otherwise
 */
bool macSendFsCmd( uint8 channel )
{
  MAC_PWR_VOTE_NO_SWI(TRUE);
  macSetupFsCmd(channel);
  lastPostCmd = (uint32_t*)&RF_cmdFsRx;
  return TRUE;
}


/*******************************************************************************
 * @fn          macSetTxPowerVal
 *
 * @brief       On reception, the radio CPU shall set the TX power to
 *              txPowerVal.
 *
 * input parameters
 *
 * @param       txPowerVal - TX power value consisting of
 *                           IB, GC, boost and tempCoeff.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void macSetTxPowerVal( uint32 txPowerVal )
{
  RF_Stat status;
  RF_TxPowerTable_Value newValue;

  newValue = *(RF_TxPowerTable_Value *)&txPowerVal;
  status = RF_setTxPower(RF_handle, newValue);
  MAC_ASSERT(status == RF_StatSuccess);
  return;
}


/*******************************************************************************
 * @fn          macSetupRATChanCompare
 *
 * @brief       This call is used to setup a RAT channel compare that will
 *              generate a HW interrupt.
 *
 * @param       ratCb - RAT Callback.
 * @param       compareTim - RAT Compare Time
 *
 * @return      None.
 */
void macSetupRATChanCompare(RF_RatCallback ratCb, uint32 compareTime)
{
  int8_t chan = RF_ALLOC_ERROR;
  uint32_t powerMgr = 0;
  RF_Stat stat;
  halIntState_t s;

  HAL_ENTER_CRITICAL_SECTION(s);

  /*Disable power management, to avoid power down due to temporary ratDisable*/
  RF_control(RF_handle, RF_CTRL_SET_POWER_MGMT, &powerMgr);

  RF_RatConfigCompare compareConfig;
  RF_RatConfigCompare_init(&compareConfig);
  compareConfig.callback   = ratCb;
  compareConfig.timeout = compareTime;

  /* WA for reinitiating a RAT channel before it fires */
  if (((uint32_t) ratCb & 0xFFFFFFFE) == ((uint32_t) macRatCb_A & 0xFFFFFFFE))
  {
    if (lastRatChanA != RF_ALLOC_ERROR)
    {
      stat = RF_ratDisableChannel(RF_handle, lastRatChanA);
      MAC_ASSERT(stat == RF_StatCmdDoneSuccess);

      DBG_PRINT1(DBGSYS, "RAT: RAT ChanA(%i) disabled", lastRatChanA);
    }
    chan = RF_ratCompare(RF_handle, &compareConfig, NULL);
  }
  else if (((uint32_t) ratCb & 0xFFFFFFFE) == ((uint32_t) macRatCb_B & 0xFFFFFFFE))
  {
    if (lastRatChanB != RF_ALLOC_ERROR)
    {
      stat = RF_ratDisableChannel(RF_handle, lastRatChanB);
      MAC_ASSERT(stat == RF_StatCmdDoneSuccess);

      DBG_PRINT1(DBGSYS, "RAT: RAT ChanB(%i) disabled", lastRatChanB);
    }
    chan = RF_ratCompare(RF_handle, &compareConfig, NULL);
  }
  else
  {
    // Shouldn't be here
    MAC_ASSERT(0);
  }



  DBG_PRINT1(DBGSYS, "RAT: RF_ratCompare(RAT Channel=%i)", chan);

  if (chan < 0)
  {
    /* fatal error */
    MAC_ASSERT(0);
  }

  /* WA for reinitiating a RAT channel before it fires */
  if (((uint32_t) ratCb & 0xFFFFFFFE) == ((uint32_t) macRatCb_A & 0xFFFFFFFE))
  {
    lastRatChanA = chan;
  }
  else if (((uint32_t) ratCb & 0xFFFFFFFE) == ((uint32_t) macRatCb_B & 0xFFFFFFFE))
  {
    lastRatChanB = chan;
  }

  /*Enable power management */
  powerMgr = 1;
  RF_control(RF_handle, RF_CTRL_SET_POWER_MGMT, &powerMgr);

  HAL_EXIT_CRITICAL_SECTION(s);
}


/*******************************************************************************
 * @fn          macRatCb_A Callback
 *
 * @brief       This callback is used to carry out the processing for the RAT
 *              interrupt that corresponds to the RAT channel where the RAT
 *              command was initiated.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       h - radio handle
 * @param       ch - command handle
 * @param       e - radio event (interrupt flag)
 *
 * @return      None.
 */
void macRatCb_A(RF_Handle h, RF_RatHandle ch, RF_EventMask e, uint32_t compareCaptureTime)
{
  /* Now we are in swi context */
  bInSwi = TRUE;
  DBG_PRINT0(DBGSYS, "macRatCb_A() starts");

  /* Clear up channel A */
  lastRatChanA = RF_ALLOC_ERROR;
  DBG_PRINT0(DBGSYS, "macRatCb_A() ends");

  /* Now we are exiting the swi */
  bInSwi = FALSE;
}


/*******************************************************************************
 * @fn          macRatCb_B Callback
 *
 * @brief       This callback is used to carry out the processing for the RAT
 *              interrupt that corresponds to the RAT channel where the RAT
 *              command was initiated.
 *
 * @param       h - radio handle
 * @param       ch - command handle
 * @param       e - radio event (interrupt flag)
 *
 * @return      None.
 */
void macRatCb_B(RF_Handle h, RF_RatHandle ch, RF_EventMask e, uint32_t compareCaptureTime)
{
  /* Clear up channel B */
  lastRatChanB = RF_ALLOC_ERROR;

  if(!macTimerActive)
  {
      /*delayed swi which has already been processed*/
      return;
  }

  /* Now we are in swi context */
  bInSwi = TRUE;
  DBG_PRINT0(DBGSYS, "macRatCb_B() starts");
  DBG_PRINT0(DBGSYS, "macRatCb_B() ends");

  /* Now we are exiting the swi */
  bInSwi = FALSE;
}


/*******************************************************************************
 * @fn          macRegisterSfdDetect
 *
 * @brief       Register callback for SFD detected.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void macRegisterSfdDetect( macSfdDetectCBack_t sfdCback )
{
  macSfdDetectCback = sfdCback;
}


/*******************************************************************************
 * @fn          macSetUserConfig
 *
 * @brief       This call is used to setup user radio configurations.
 *
 * input parameters
 *
 * @param       arg - pointer to user configuration from App.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void macSetUserConfig( macUserCfg_t *pUserCfg )
{
  if ( pUserCfg != NULL )
  {
    /* RF patch, Front End info, and TX Power Table */
    pMacRadioConfig = pUserCfg;

    MAC_ASSERT(pMacRadioConfig->pRfSelectFP != NULL);

    macRfDrvTblPtr = pMacRadioConfig->pRfDrvTblPtr;

#if defined(FEATURE_MAC_SECURITY)
    macCryptoDrvTblPtr = pMacRadioConfig->pCryptoDrvTblPtr;
#endif /* FEATURE_MAC_SECURITY */

#if defined(RCN_APP_ASSERT)
    if (pMacRadioConfig->pAssertFP)
    {
        assertFP.callAssrtHndler = pMacRadioConfig->pAssertFP;
        HAL_APPASRT_register(&assertFP);
    }
#endif

  }
}


/**************************************************************************************************
 * @fn          macRatDisableChannels
 *
 * @brief       Disable RAR channels to prepare for sleep.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRatDisableChannels(void)
{
  if (lastRatChanA != RF_ALLOC_ERROR)
  {
    RF_ratDisableChannel(RF_handle, lastRatChanA);
    DBG_PRINT1(DBGSYS, "RAT Chan A(%d) disabled", lastRatChanA);
  }
  if (lastRatChanB != RF_ALLOC_ERROR)
  {
    RF_ratDisableChannel(RF_handle, lastRatChanB);
    DBG_PRINT1(DBGSYS, "RAT Chan B(%d) disabled", lastRatChanB);
  }
  /* Mark them as not allocated */
  lastRatChanA = lastRatChanB = RF_ALLOC_ERROR;
}
/**************************************************************************************************
 * @fn          macRatDisableChannels
 *
 * @brief       Disable RAR channels to prepare for sleep.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRatDisableChannelB(void)
{
  if (lastRatChanB != RF_ALLOC_ERROR)
  {
    RF_ratDisableChannel(RF_handle, lastRatChanB);
    /* Mark them as not allocated */
    lastRatChanB = RF_ALLOC_ERROR;
  }

}

/**************************************************************************************************
 * @fn          macGetRFCmdDuration
 *
 * @brief       Approximate command duration based on input packet length and whether the radio
 *              must wait or transmit for an ack response/request.
 *
 * @param       uint32_t len Input packet length
 * @param       bool ackRequest Whether the input command must wait or transmit an ack.
 *
 * @return      uint32 Approximate during, in RAT ticks
 **************************************************************************************************
 */
uint32 macGetRFCmdDuration(uint32_t len, bool ackRequest)
{
    uint16 totalSymbols = 0;
    uint32_t psduByteLen = 0;
    uint8_t minBe;

    /* Obtain min backoff exponent */
    MAP_MAC_MlmeGetReq(MAC_MIN_BE, &minBe);

    /* PSDU = MHR, MAC Payload, MFR (FCS) Length in bytes is included for subg in the input length
     * and is included later for IEEE mode
     */
    psduByteLen += len;

    /* 5kbps LRM PHY */
    if (MAP_macCheckPhyRate(pMacPib->curPhyID) == PHY_MODE_SLR_5K)
    {
        /* Convert PHY + PSDU to symbols */
        totalSymbols += ((MAC_PHY_PHR_LEN + psduByteLen) * MAC_SYMBOLS_PER_OCTET_2FSK_LRM);

        /* Number of symbols in frame for Preamble, sync word and termination symbols */
        totalSymbols += MAC_PHY_SHR_LEN_2FSK_LRM + MAC_PHY_TERM_LEN_LRM;

        /* If receiver will be kept on for Rx ACK */
        if (ackRequest)
        {
            /* Add turnaround time before the ack in addition to the preamble, sync word and termination symbols */
            totalSymbols += (MAC_A_TURNAROUND_TIME + MAC_PHY_SHR_LEN_2FSK_LRM  + MAC_PHY_TERM_LEN_LRM);

            /* Add in the bytes for the OTA time for incoming ACK frame
             * MAC_LEN_ACK_FRAME = (MAC_FCF_FIELD_LEN + MAC_SEQ_NUM_FIELD_LEN + MAC_FCS4_FIELD_LEN)
             */
            totalSymbols += ((MAC_PHY_PHR_LEN + (MAC_FCF_FIELD_LEN + MAC_SEQ_NUM_FIELD_LEN + MAC_FCS4_FIELD_LEN)) * MAC_SYMBOLS_PER_OCTET_2FSK_LRM);
        }
    }
    /* Other PHY's */
    else
    {
        /* Convert Preamble, sync word (SHR), PHR, PSDU to symbols */
        totalSymbols += ((MAC_PHY_SHR_LEN_2FSK + MAC_PHY_PHR_LEN + psduByteLen) * MAC_SYMBOLS_PER_OCTET_2FSK);

        /* If receiver will be kept on for Rx ACK */
        if (ackRequest)
        {
            /* Add turnaround time before the ACK */
            totalSymbols += MAC_A_TURNAROUND_TIME;

            /* Add in the bytes in the acknowledgment frame
             * MAC_LEN_ACK_FRAME = (MAC_FCF_FIELD_LEN + MAC_SEQ_NUM_FIELD_LEN + MAC_FCS4_FIELD_LEN)
             */
            totalSymbols += ((MAC_PHY_SHR_LEN_2FSK + MAC_PHY_PHR_LEN + (MAC_FCF_FIELD_LEN + MAC_SEQ_NUM_FIELD_LEN + MAC_FCS4_FIELD_LEN)) * MAC_SYMBOLS_PER_OCTET_2FSK);
        }
    }

    /* Add interframe spacing */
    totalSymbols += (len > MAC_A_MAX_SIFS_FRAME_SIZE) ? MAC_A_MIN_LIFS_PERIOD : MAC_A_MIN_SIFS_PERIOD;

    /* Convert symbols to microseconds, then RAT Ticks */
    return (RF_convertUsToRatTicks(totalSymbols * macSpecUsecsPerSymbol));
}
/*******************************************************************************
 */
/**************************************************************************************************
 * @fn          macRequestBackoff
 *
 * @brief       This function is used to start a backoff for either a Tx or Rx or NOP command.
 *
 * @param       backoffType: BACKOFF_TYPE_TX, BACKOFF_TYPE_RX, BACKOFF_TYPE_NOP
 * @param       backOffDur: Backoff duration in us
 *
 * @return      none
 **************************************************************************************************
 */
void macRequestBackoff(uint8_t backoffType, uint32_t backOffDur)
{
   uint32_t key;
   uint32_t backoffPeriod;

   key = OsalPort_enterCS();

   backoffPeriod = (uint32_t)backOffDur/MAP_ICall_getTickPeriod();

   if (backoffType == BACKOFF_TYPE_TX)
   {
       DBG_PRINT0(DBGSYS, "macRequestBackoff(Tx)");

       /* Stop existing timer if backoff period is 0 */
       if (!backoffPeriod)
       {
           OsalPort_stopTimer((MAP_ICall_TimerID*)macTxBackoffRtcTimerId);
       }
       else
       {
           MAP_ICall_setTimer(backoffPeriod, (MAP_ICall_TimerCback)macTxBackoffTimerExpiry,
               NULL, &macTxBackoffRtcTimerId);
       }
   }
   else if (backoffType == BACKOFF_TYPE_RX)
   {
       DBG_PRINT0(DBGSYS, "macRequestBackoff(Rx)");

       /* Stop existing timer if backoff period is 0 */
       if (!backoffPeriod)
       {
           OsalPort_stopTimer((MAP_ICall_TimerID*)macRxBackoffRtcTimerId);
       }
       else
       {
           MAP_ICall_setTimer(backoffPeriod, (MAP_ICall_TimerCback)macRxBackoffTimerExpiry,
               NULL, &macRxBackoffRtcTimerId);
       }
   }
  OsalPort_leaveCS(key);
}
/**************************************************************************************************
 * @fn          macTxBackoffTimerExpiry
 *
 * @brief       Throw an event for the Tx backoff timer expiry
 *
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTxBackoffTimerExpiry(void *arg)
{
    MAP_macSetEvent16(MAC_TX_BACKOFF_TIM_TASK_EVT);
}

/**************************************************************************************************
 * @fn          macRxBackoffTimerExpiry
 *
 * @brief       Throw an event for the Rx backoff timer expiry
 *
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxBackoffTimerExpiry(void *arg)
{
    MAP_macSetEvent16(MAC_RX_BACKOFF_TIM_TASK_EVT);
}

/**************************************************************************************************
 * @fn          macNopBackoffTimerExpiry
 *
 * @brief       Throw an event for the Nop backoff timer expiry
 *
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macNopBackoffTimerExpiry(void *arg)
{
    /* Attempt to reschedule NOP command */
    DBG_PRINT0(DBGSYS, "MAC Execute Backoff(NOP)");

    /* Execute Dummy NOP Cb and treat as preempted after backoff time */
     numRfCmd++;
     macRadioWakeupCb(NULL, NULL, RF_EventCmdPreempted);
}

#ifdef FREERTOS_SUPPORT
/* POSIX Header files */
#include <pthread.h>
#include <FreeRTOS.h>
#include <semaphore.h>

#define RF_CB_THREADSTACKSIZE 2000

pthread_t rfCdThreadHndl = NULL;

sem_t rfCbSemHandle;

uint32_t rxCount;
bool rxRec;
// rf queue variables
uint8_t macRfQueueSize;
macRfQueue_t macRfQueue[MAC_FREERTOS_RF_SLOTS];
uint8_t wRfPtr;
uint8_t rRfPtr;
uint8_t numRfCmdQueued;

void runRxCb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    //pushing to rf queue
    macRfQueue[wRfPtr].rfCbHndl = h;
    macRfQueue[wRfPtr].rfCbCmdHndl = ch;
    macRfQueue[wRfPtr].rfCbEventMask = e;
    macRfQueue[wRfPtr].rfCb = macRxCb;
    wRfPtr = (wRfPtr + 1) % macRfQueueSize;
    numRfCmdQueued++;
    rxCount++;

    /* Unblock the task by releasing the semaphore. */
    sem_post(&rfCbSemHandle);
}

void *rfCbThread(void *arg0)
{
    while(true)
    {
        sem_wait(&rfCbSemHandle);
        if (numRfCmdQueued)
        {
            macRfQueue[rRfPtr].rfCb(macRfQueue[rRfPtr].rfCbHndl,
                                   macRfQueue[rRfPtr].rfCbCmdHndl,
                                   macRfQueue[rRfPtr].rfCbEventMask);
            portENTER_CRITICAL();
            numRfCmdQueued--;
            rRfPtr = (rRfPtr + 1) % macRfQueueSize;
            portEXIT_CRITICAL();
        }
    }
}

void startRfCbThread(void)
{
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;
    rxCount = 0;
    rxRec = true;
    numRfCmdQueued = 0;

    // initialize the RF Queue
    wRfPtr = 0;
    rRfPtr = 0;
    macRfQueueSize = MAC_FREERTOS_RF_SLOTS;
    for (int i = 0; i < macRfQueueSize; i++)
    {
        memset(&macRfQueue[i], 0, sizeof(macRfQueue_t));
    }

    if(rfCdThreadHndl == NULL)
    {
        /* create semaphores
         */
        retc = sem_init(&rfCbSemHandle, 0, 0);
        if (retc != 0) {
            while (1) {}
        }

        /* Initialize the attributes structure with default values */
        pthread_attr_init(&attrs);

        /* Set priority, detach state, and stack size attributes */
        priParam.sched_priority = (configMAX_PRIORITIES - 1);
        retc = pthread_attr_setschedparam(&attrs, &priParam);
        retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
        retc |= pthread_attr_setstacksize(&attrs, RF_CB_THREADSTACKSIZE);
        if (retc != 0) {
            /* failed to set attributes */
            while (1) {}
        }

        retc = pthread_create(&rfCdThreadHndl, &attrs, rfCbThread, NULL);
        if (retc != 0) {
            /* pthread_create() failed */
            while (1) {}
        }
    }
}

#endif //FREERTOS_SUPPORT
