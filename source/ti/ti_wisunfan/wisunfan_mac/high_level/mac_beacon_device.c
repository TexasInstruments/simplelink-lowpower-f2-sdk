/******************************************************************************

 @file  mac_beacon_device.c

 @brief Implements device only procedures for beacon enabled networks.

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

#include "hal_mcu.h"
#include "timac_api.h"
#include "mac_main.h"
#include "mac_pib.h"
#include "mac_timer.h"
#include "mac_mgmt.h"
#include "mac_low_level.h"
#include "mac_radio.h"
#include "mac_data.h"
#include "mac_device.h"
#include "mac_beacon.h"
#include "mac_spec.h"
#include "mac_beacon_device.h"
#include "mac_symbol_timer.h"
#include "mac_hl_patch.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_beacon_device_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

#include "rom_jt_154.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */
#define MAC_BEACON_TRACK_TIME(X) (MAC_BEACON_INTERVAL(pMacPib->beaconOrder) + \
                                  MAC_A_BASE_SUPERFRAME_DURATION + (X))

#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)
/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */

void macBroadcastPendCallback(uint8 param);
void macBeaconStopTrack(void);
void macIncomingNonSlottedTx(void);
void macBeaconSyncLoss(void);
void macSyncTimeoutCallback(uint8 param);

#if !defined(TIMAC_ROM_PATCH)
void macTrackTimeoutCallback(uint8 param);
void macTrackStartCallback(uint8 param);
#endif

void macTrackPeriodCallback(uint8 param);
void macRxBeaconCritical(macRx_t *pBuf);
uint8 macCheckPendAddr(uint8 pendAddrSpec, uint8 *pAddrList);

/**************************************************************************************************
 * @fn          macBroadcastPendCallback
 *
 * @brief       This function is called when the broadcast pending timer expires.  It turns off
 *              the receiver.
 *
 * input parameters
 *
 * @param       param - not used.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void macBroadcastPendCallback(uint8 param)
{
  (void)param;  // Intentionally unreferenced parameter

  MAP_macRxDisable(MAC_RX_BROADCAST_PEND);
}

/**************************************************************************************************
 * @fn          macBeaconStopTrack
 *
 * @brief       This function stops beacon tracking.
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
void macBeaconStopTrack(void)
{
  DBG_PRINT0(DBGSYS, "Beacon Device: macBeaconStopTrack()");

  /* stop beacon tracking */
  if (pMacPib->rxOnWhenIdle)
  {
    MAP_macRxSoftEnable(MAC_RX_WHEN_IDLE);
  }
  MAP_macRxDisable(MAC_RX_BEACON_DEVICE);
  MAP_macTimerCancel(&macBeaconDevice.timeoutTimer);
  MAP_macTimerCancel(&macBeaconDevice.trackTimer);

  MAP_macSymbolTimerInit();

}
/**************************************************************************************************
 * @fn          macIncomingNonSlottedTx
 *
 * @brief       Set up all frames scheduled for the incoming superframe for nonslotted tx.
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
void macIncomingNonSlottedTx(void)
{
  macTx_t       *pCurr;
  halIntState_t intState;

  DBG_PRINT0(DBGSYS, "Beacon Device: macIncomingNonSlottedTx()");

  HAL_ENTER_CRITICAL_SECTION(intState);

  /* iterate over tx queue */
  pCurr = MAP_OSAL_MSG_Q_HEAD(&macData.txQueue);
  while (pCurr != NULL)
  {
    /* set frame to be *both* incoming and outgoing CAP; this will allow transmission in
     * outgoing CAP when sync is lost or in incoming CAP if we sync again before the
     * frame is transmitted
     */
    if (pCurr->internal.txSched & MAC_TX_SCHED_INCOMING_CAP)
    {
      pCurr->internal.txSched |= MAC_TX_SCHED_OUTGOING_CAP;
      pCurr->internal.txMode &= ~MAC_TX_MODE_SLOTTED;
    }
    pCurr = MAP_OSAL_MSG_NEXT(pCurr);
  }

  HAL_EXIT_CRITICAL_SECTION(intState);

  MAP_macSetEvent(MAC_TX_COMPLETE_TASK_EVT);
}

/**************************************************************************************************
 * @fn          macBeaconSyncLoss
 *
 * @brief       Handle sync loss.
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
void macBeaconSyncLoss(void)
{
  macMlmeSyncLossInd_t syncLossInd;

  DBG_PRINT0(DBGSYS, "Beacon Device: macBeaconSyncLoss()");

  /* if cluster tree network is no longer started */
  macMgmt.networkStarted = FALSE;

  /* initialize beacon state and schedule */
  macBeacon.state = MAC_BEACON_NONE_ST;
  macBeacon.sched = MAC_TX_SCHED_OUTGOING_CAP;
  macBeacon.rxWindow = FALSE;

  /* set up any queued data for non-slotted tx */
  MAP_macIncomingNonSlottedTx();

  /* if cluster tree need to start timer for indirect data expiration */
  MAP_macSetEvent(MAC_EXP_INDIRECT_TASK_EVT);

  /* if interally generated sync */
  if (macBeacon.txSync)
  {
    macBeacon.txSync = FALSE;
  }
  /* else send sync loss ind */
  else
  {
    syncLossInd.hdr.event = MAC_MLME_SYNC_LOSS_IND;
    syncLossInd.hdr.status = MAC_BEACON_LOSS;
    syncLossInd.panId = pMacPib->panId;
    syncLossInd.logicalChannel = pMacPib->logicalChannel;
    syncLossInd.phyID = pMacPib->curPhyID;
    syncLossInd.channelPage = pMacPib->channelPage;
    syncLossInd.sec.securityLevel = MAC_SEC_LEVEL_NONE;
    MAP_MAC_CbackEvent((macCbackEvent_t *) &syncLossInd);
  }
}

/**************************************************************************************************
 * @fn          macSyncTimeoutCallback
 *
 * @brief       This function is called if the beacon is not found in the alloted window
 *              during a beacon search initiated by a sync request.
 *
 * input parameters
 *
 * @param       param - not used.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void macSyncTimeoutCallback(uint8 param)
{
#ifdef COMBO_MAC
    uint8 rfFreq = pMacPib->rfFreq;
#endif

  (void)param;  // Intentionally unreferenced parameter

  DBG_PRINT1(DBGSYS, "Beacon Device: macSyncTimeoutCallback(rxWindow=%i)", macBeacon.rxWindow);

  /* if rxWindow is FALSE then sync was postponed because a tx was in progress; if tx is no
   * longer in progress turn on the receiver and let sync proceed
   */
  if (macBeacon.rxWindow == FALSE)
  {
    if (pMacDataTx == NULL)
    {
      macBeacon.rxWindow = TRUE;
      MAP_macRxEnable(MAC_RX_BEACON_DEVICE);
    }
#ifdef COMBO_MAC
   if (rfFreq == MAC_RF_FREQ_SUBG)
   {
       MAP_macTimer(&macBeaconDevice.timeoutTimer, MAC_BEACON_TRACK_TIME(MAC_PROCESS_DELAY));
   }
   else
   {
       MAP_macTimer(&macBeaconDevice.timeoutTimer, MAC_BEACON_TRACK_TIME(MAC_IEEE_PROCESS_DELAY));
   }
#else
#ifndef FREQ_2_4G
    MAP_macTimer(&macBeaconDevice.timeoutTimer, MAC_BEACON_TRACK_TIME(MAC_PROCESS_DELAY));
#else
    MAP_macTimer(&macBeaconDevice.timeoutTimer, MAC_BEACON_TRACK_TIME(MAC_IEEE_PROCESS_DELAY));
#endif
#endif
  }
  else
  {
    macBeaconDevice.lostCount++;

    if (macBeaconDevice.lostCount == MAC_A_MAX_LOST_BEACONS)
    {
      MAP_macRxDisable(MAC_RX_BEACON_DEVICE);
      MAP_macBeaconSyncLoss();
    }
    else
    {
#ifdef COMBO_MAC
      if (rfFreq == MAC_RF_FREQ_SUBG)
      {
          MAP_macTimer(&macBeaconDevice.timeoutTimer, MAC_BEACON_TRACK_TIME(MAC_PROCESS_DELAY));
      }
      else
      {
          MAP_macTimer(&macBeaconDevice.timeoutTimer, MAC_BEACON_TRACK_TIME(MAC_IEEE_PROCESS_DELAY));
      }
#else
#ifndef FREQ_2_4G
      MAP_macTimer(&macBeaconDevice.timeoutTimer, MAC_BEACON_TRACK_TIME(MAC_PROCESS_DELAY));
#else
      MAP_macTimer(&macBeaconDevice.timeoutTimer, MAC_BEACON_TRACK_TIME(MAC_IEEE_PROCESS_DELAY));
#endif
#endif
    }
  }
}


#if !defined(TIMAC_ROM_PATCH)
/**************************************************************************************************
 * @fn          macTrackTimeoutCallback
 *
 * @brief       This function is called if the beacon is not found in the alloted window
 *              during beacon tracking.
 *
 * input parameters
 *
 * @param       param - not used.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void macTrackTimeoutCallback(uint8 param)
{
  uint8 marginBeaconOrder = pMacPib->beaconOrder;
  (void)param;  // Intentionally unreferenced parameter

  macBeacon.rxWindow = FALSE;
  macBeaconDevice.lostCount++;

  DBG_PRINT1(DBGSYS, "Beacon Device: macTrackTimeoutCallback(lostCount=%i)", macBeaconDevice.lostCount);

  if (macBeaconDevice.lostCount == MAC_A_MAX_LOST_BEACONS)
  {
    MAP_macBeaconStopTrack();
    MAP_macBeaconSyncLoss();
  }
  else
  {
    MAP_macRxDisable(MAC_RX_BEACON_DEVICE);

    /* added to allow end-nodes, that are synced to larger than
     * network beacon-orders, to sync to the next beacon (after one is missed)
     * if the device beacon order is in effect.
     */
    if ((pMacPib->deviceBeaconOrder != MAC_BO_NON_BEACON) &&
        (macBeacon.coordBeaconOrder < pMacPib->beaconOrder))
    {
      /* use the larger margin window value of the device beacon order */
      marginBeaconOrder = pMacPib->deviceBeaconOrder;

      /* set the beacon order back to the coordinator beacon order */
      pMacPib->beaconOrder = macBeacon.coordBeaconOrder;

      /* the MAC timer rollover must be set too */
      MAP_macTimerSetRollover(pMacPib->beaconOrder);
    }

    /* increase the beacon tracking window by the beacon margin */
    MAP_macTimerAligned(&macBeaconDevice.trackTimer,
                    MAC_BEACON_INTERVAL(pMacPib->beaconOrder) -
                    (macBeaconMargin[marginBeaconOrder] * (macBeaconDevice.lostCount + 1)));
  }
}

/**************************************************************************************************
 * @fn          macTrackStartCallback
 *
 * @brief       This callback is executed when beacon tracking begins for the next beacon
 *              interval.
 *
 * input parameters
 *
 * @param       param - not used.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void macTrackStartCallback(uint8 param)
{
#ifdef COMBO_MAC
    uint8 rfFreq = pMacPib->rfFreq;
#endif

  (void)param;  // Intentionally unreferenced parameter

  DBG_PRINT0(DBGSYS, "Beacon Device: macTrackStartCallback()");

  macBeacon.rxWindow = TRUE;

  if (pMacPib->beaconOrder == MAC_BO_NON_BEACON)
  {
    /* stop beacon tracking */
    MAP_macBeaconStopTrack();
  }
  else
  {
    /* Netwrok is started after beacon tracking */
    macMgmt.networkStarted = TRUE;

    MAP_macRxEnable(MAC_RX_BEACON_DEVICE);
    macBeaconDevice.timeoutTimer.pFunc = macTrackTimeoutCallback;
#ifdef COMBO_MAC
    if (rfFreq == MAC_RF_FREQ_SUBG)
    {
        MAP_macTimerAligned(&macBeaconDevice.timeoutTimer, MAC_BEACON_TO_MULTIPLE *
                        macBeaconMargin[pMacPib->beaconOrder] * (macBeaconDevice.lostCount + 1) +
                        MAC_PROCESS_DELAY);
    }
    else
    {
        MAP_macTimerAligned(&macBeaconDevice.timeoutTimer, MAC_BEACON_TO_MULTIPLE *
                        macBeaconMargin[pMacPib->beaconOrder] * (macBeaconDevice.lostCount + 1) +
                        MAC_IEEE_PROCESS_DELAY);
    }
#else
#ifndef FREQ_2_4G
    MAP_macTimerAligned(&macBeaconDevice.timeoutTimer, MAC_BEACON_TO_MULTIPLE *
                    macBeaconMargin[pMacPib->beaconOrder] * (macBeaconDevice.lostCount + 1) +
                    MAC_PROCESS_DELAY);
#else
    MAP_macTimerAligned(&macBeaconDevice.timeoutTimer, MAC_BEACON_TO_MULTIPLE *
                    macBeaconMargin[pMacPib->beaconOrder] * (macBeaconDevice.lostCount + 1) +
                    MAC_IEEE_PROCESS_DELAY);
#endif
#endif
  }
}
#endif

/**************************************************************************************************
 * @fn          macTrackPeriodCallback
 *
 * @brief       This callback is executed at end of the CAP.
 *
 * input parameters
 *
 * @param       param - not used.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
#if !defined (TIMAC_ROM_PATCH)
void macTrackPeriodCallback(uint8 param)
{
  ApiMac_mlmeSyncReq_t syncReq;

  (void)param;  // Intentionally unreferenced parameter

  DBG_PRINT0(DBGSYS, "Beacon Device: macTrackPeriodCallback()");

  macBeacon.sched = MAC_TX_SCHED_INACTIVE;

  /* if only did a sync for one beacon */
  if (macBeacon.state == MAC_BEACON_SYNC_ST)
  {
    /* end of superframe */
    macBeacon.state = MAC_BEACON_NONE_ST;

    /* if data is queued initiate internal sync req */
    if (macData.directCount != 0)
    {
      syncReq.logicalChannel = pMacPib->logicalChannel;
      syncReq.channelPage = pMacPib->channelPage;
      syncReq.phyID = pMacPib->curPhyID;
      syncReq.trackBeacon = MAC_TRACK_BEACON_TX_SYNC;
      MAP_MAC_MlmeSyncReq(&syncReq);
    }
  }
  /* else disable rx if there is an inactive period */
  else if (pMacPib->superframeOrder != pMacPib->beaconOrder)
  {
    MAP_macRxHardDisable();
    /* Power down the radio */
    macSymbolTimerPowerDown();
  }
}

/**************************************************************************************************

 * @fn          macRxBeaconCritical
 *
 * @brief       Performs timing-critical procedures for the received beacons.
 *
 * input parameters
 *
 * @param       pBuf - received beacon.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void macRxBeaconCritical(macRx_t *pBuf)
{
  uint16 sfs;

  DBG_PRINT1(DBGSYS, "Beacon Device: macRxBeaconCritical(rxWindow=%i)", macBeacon.rxWindow);

  if (macBeacon.rxWindow && (pBuf->mac.srcPanId == pMacPib->panId) &&
      MAP_macCoordAddrCmp(&pBuf->mac.srcAddr))
  {
    macBeacon.rxWindow = FALSE;

    /* disable receiver for tracking or polling, but keep it on if rxOnWhenIdle */
    if (pMacPib->rxOnWhenIdle)
    {
      MAP_macRxSoftEnable(MAC_RX_WHEN_IDLE);
    }
    if (macDevice.dataReqMask != 0)
    {
      MAP_macRxSoftEnable(MAC_RX_POLL);
    }

    /* handle pending broadcast from coordinator */
    if (pBuf->internal.flags & MAC_RX_FLAG_PENDING)
    {
      MAP_macRxSoftEnable(MAC_RX_BROADCAST_PEND);
      MAP_macSetEvent(MAC_BROADCAST_PEND_TASK_EVT);
    }

    MAP_macRxDisable(MAC_RX_BEACON_DEVICE);

    /* cancel the timeout timer */
    MAP_macTimerCancel(&macBeaconDevice.timeoutTimer);

    /* this beacon is from our coordinator, record the beacon info from superframe spec */
    sfs = BUILD_UINT16(pBuf->msdu.p[MAC_SFS_OFFSET], pBuf->msdu.p[MAC_SFS_OFFSET+1]);
    macBeacon.finalCapSlot = MAC_SFS_FINAL_CAP_SLOT(sfs);
    macBeacon.battLifeExt = MAC_SFS_BLE(sfs);
    macBeacon.coordBeaconOrder = MAC_SFS_BEACON_ORDER(sfs);
    pMacPib->superframeOrder = MAC_SFS_SUPERFRAME_ORDER(sfs);

    /* if device beacon order is being used and the received
     * coordinator beacon order is less than the device beacon order and
     * there is no pending data
     *
     * byte pBuf->msdu.p[MAC_PENDING_ADDR_OFFSET] contains the pending address field of
     * the beacon frame (If there are no pending frames it will equal zero).
     * note: check of pending data assumes GTS is not in use
     */
    if ((pMacPib->deviceBeaconOrder != MAC_BO_NON_BEACON) &&
        (macBeacon.coordBeaconOrder < pMacPib->deviceBeaconOrder) &&
        (pBuf->msdu.p[MAC_PENDING_ADDR_OFFSET] == 0))
    {
      /* set the beacon order to the device beacon order */
      pMacPib->beaconOrder = pMacPib->deviceBeaconOrder;
    }
    else
    {
      /* set the beacon order to the received beacon order */
      pMacPib->beaconOrder = macBeacon.coordBeaconOrder;
    }

    /* Clear the lost count */
    macBeaconDevice.lostCount = 0;

    if (pMacPib->beaconOrder != MAC_BO_NON_BEACON)
    {
      /* setup for next beacon track */
      MAP_macTimerRealign(pBuf, pMacPib->beaconOrder);

      if (macBeacon.state == MAC_BEACON_TRACKING_ST)
      {
        MAP_macTimerCancel(&macBeaconDevice.trackTimer);
        MAP_macTimerAligned(&macBeaconDevice.trackTimer,
          MAC_BEACON_INTERVAL(pMacPib->beaconOrder) - macBeaconMargin[pMacPib->beaconOrder]);
      }

      MAP_macBeaconSetupCap(MAC_TX_SCHED_INCOMING_CAP, pMacPib->superframeOrder, pBuf->msdu.len);

      MAP_macTimerCancel(&macBeacon.periodTimer);
      macBeacon.periodTimer.pFunc = MAP_macTrackPeriodCallback;
      MAP_macTimerAligned(&macBeacon.periodTimer, macBeacon.capEnd);

      MAP_macSetEvent(MAC_TX_COMPLETE_TASK_EVT);
    }
    else
    {
      if (macBeacon.state == MAC_BEACON_TRACKING_ST)
      {
        /* stop tracking */
        MAP_macBeaconStopTrack();
        MAP_macBeaconSyncLoss();
      }
    }
  }
}
#endif
/**************************************************************************************************
 * @fn          MAC_InitBeaconDevice
 *
 * @brief       Initializes the MAC to operate as a device in a beacon enabled network.
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
void MAC_InitBeaconDevice(void)
{
  /* Initialize beacon device action tables */
  macBeaconDeviceAction[0] = MAP_macApiSyncReq;
  macBeaconDeviceAction[1] = MAP_macAutoPoll;
  macBeaconDeviceAction[2] = MAP_macBeaconStartFrameResponseTimer;
  macBeaconDeviceAction[3] = MAP_macStartBroadcastPendTimer;

  /* setup timer callbacks */
  macBeaconDevice.trackTimer.pFunc = MAP_macTrackStartCallback;
  macBeaconDevice.timeoutTimer.pFunc = MAP_macTrackTimeoutCallback;
  macBeaconDevice.broacastPendTimer.pFunc = MAP_macBroadcastPendCallback;

  /* setup beacon pointer to functions */
  macDataRxBeaconCritical = MAP_macRxBeaconCritical;

  /* set up action table */
  macActionSet[MAC_BEACON_DEVICE_ACTION] = macBeaconDeviceAction;

  MAP_macBeaconInit();

  macMain.featureMask |= MAC_FEAT_BEACON_DEVICE_MASK;
}

/**************************************************************************************************
 * @fn          macCheckPendAddr
 *
 * @brief       Checks the pending address field of the received beacon to see if it matches
 *              the device's address.
 *
 * input parameters
 *
 * @param       pEvent - Pointer to event data.
 *
 * output parameters
 *
 * None.
 *
 * @return      The address mode if match found, otherwise SADDR_MODE_NONE.
 **************************************************************************************************
 */
uint8 macCheckPendAddr(uint8 pendAddrSpec, uint8 *pAddrList)
{
  uint8 numShort = MAC_PEND_NUM_SHORT(pendAddrSpec);
  uint8 numExt = MAC_PEND_NUM_EXT(pendAddrSpec);

  /* check if a short addr in the list matches ours  */
  while (numShort-- > 0)
  {
    if (pMacPib->shortAddress == BUILD_UINT16(pAddrList[0], pAddrList[1]))
    {
      return SADDR_MODE_SHORT;
    }
    pAddrList += MAC_SHORT_ADDR_FIELD_LEN;
  }

  /* check if an extended addr in the list matches ours  */
  while (numExt-- > 0)
  {
    if (MAP_sAddrExtCmp(pMacPib->extendedAddress.addr.extAddr, pAddrList))
    {
      return SADDR_MODE_EXT;
    }
    pAddrList += MAC_EXT_ADDR_FIELD_LEN;
  }

  return SADDR_MODE_NONE;
}

#if !defined (TIMAC_ROM_PATCH)
/**************************************************************************************************
 * @fn          macAutoPoll
 *
 * @brief       This action function is executed to send an automatic poll request.  Variable
 *              dataReqMask is set to indicate that the poll request is generated internally.
 *              Then it builds and sends a data request command frame.  In addition, some handling
 *              of the end-of-CAP period timer is done here.
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
MAC_INTERNAL_API void macAutoPoll(macEvent_t *pEvent)
{
  uint8         srcAddrMode = SADDR_MODE_NONE;
  sAddr_t       coordAddr;
  halIntState_t intState;

  if ((macDevice.dataReqMask == 0) && pMacPib->autoRequest)
  {
    srcAddrMode = MAP_macCheckPendAddr(pEvent->beaconNotify.info.beaconData.pendAddrSpec,
                                   pEvent->beaconNotify.info.beaconData.pAddrList);
    if (srcAddrMode != SADDR_MODE_NONE)
    {
      macDevice.dataReqMask |= MAC_DATA_REQ_AUTO;

      /* our address is in the beacon, setup for data poll */
      MAP_macGetCoordAddress(&coordAddr);

      MAP_macRxSoftEnable(MAC_RX_POLL);

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif
      /* prepare auto poll secutiry settings */
      MAP_MAC_MlmeGetSecurityReq(MAC_AUTO_REQUEST_KEY_SOURCE, &macDevice.sec.keySource);
      MAP_MAC_MlmeGetSecurityReq(MAC_AUTO_REQUEST_SECURITY_LEVEL, &macDevice.sec.securityLevel);
      MAP_MAC_MlmeGetSecurityReq(MAC_AUTO_REQUEST_KEY_ID_MODE, &macDevice.sec.keyIdMode);
      MAP_MAC_MlmeGetSecurityReq(MAC_AUTO_REQUEST_KEY_INDEX, &macDevice.sec.keyIndex);
#if (defined TIMAC_ROM_IMAGE_BUILD)
    }
#endif
#endif /* FEATURE_MAC_SECURITY */

      MAP_macBuildCommonReq(MAC_INTERNAL_DATA_REQ, srcAddrMode,
                        &coordAddr, pMacPib->panId, MAC_TXOPTION_ACK, &macDevice.sec);
    }
  }

  /* determine whether the period timer should be cancelled; first check variables
   * that do not require a critical section
   */
  if ((srcAddrMode == SADDR_MODE_NONE) &&                 /* not doing an auto-poll right now */
      (pMacPib->beaconOrder != MAC_BO_NON_BEACON) &&        /* configured as beacon-enabled */
      (pMacPib->beaconOrder != pMacPib->superframeOrder) &&   /* there is an inactive period */
      !pMacPib->rxOnWhenIdle &&                             /* receiver not always on */
      MAP_macStateIdle())                                 /* state machine in idle state */
  {
    /* Make sure RAT is active before calling macTimerCancel() within the critical section */
      /*REDUNDANT - To be removed */
    MAC_PWR_VOTE_NO_SWI(TRUE);

    /* check variables which require a critical section */
    HAL_ENTER_CRITICAL_SECTION(intState);
    if ((macBeacon.state == MAC_BEACON_TRACKING_ST) &&    /* beacon is being tracked */
        (macBeacon.sched != MAC_TX_SCHED_INACTIVE) &&     /* not already in inactive period */
        (pMacDataTx == NULL) &&                           /* no transmission in progress */
        MAP_OSAL_MSG_Q_EMPTY(&macData.txQueue))               /* no queued tx data */
    {
      HAL_EXIT_CRITICAL_SECTION(intState);
      /* MAC is not active, so force the end of CAP early; this way the device can enter
       * low power mode without the period timer unnecessarily waking it up.
       * Keep interrupts disabled to protect against timer expiring before it is cancelled.
       */
      MAP_macTimerCancel(&macBeacon.periodTimer);
      MAP_macTrackPeriodCallback(0);
    }
    else
    {
      HAL_EXIT_CRITICAL_SECTION(intState);
    }
  }
}
#endif
/**************************************************************************************************
 * @fn          macApiSyncReq
 *
 * @brief       This function handles the API for sync request.
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
MAC_INTERNAL_API void macApiSyncReq(macEvent_t *pEvent)
{
  bool          txSync = FALSE;
  uint8         state;
  halIntState_t intState;
  uint8         status = MAC_SUCCESS;
#ifdef COMBO_MAC
  uint8 rfFreq = pMacPib->rfFreq;
#endif

#if (MAC_CHAN_LOWEST == 0)
  /* No need to check MAC_CHAN_LOWEST if it is zero */
  MAC_PARAM_STATUS((pEvent->api.mac.syncReq.logicalChannel <= MAC_CHAN_HIGHEST), status);
#else
  MAC_PARAM_STATUS((pEvent->api.mac.syncReq.logicalChannel >= MAC_CHAN_LOWEST) &&
                   (pEvent->api.mac.syncReq.logicalChannel <= MAC_CHAN_HIGHEST), status);
#endif

#ifdef COMBO_MAC
  if (rfFreq == MAC_RF_FREQ_SUBG)
  {
      /* verify channel page */
      MAC_PARAM_STATUS((pEvent->api.mac.syncReq.channelPage >= MAC_CHANNEL_PAGE_9) &&
                       (pEvent->api.mac.syncReq.channelPage <= MAC_CHANNEL_PAGE_10), status);

      /* verify phy ID */
      MAC_PARAM_STATUS(((pEvent->api.mac.syncReq.phyID >= MAC_MRFSK_STD_PHY_ID_BEGIN) &&
                       (pEvent->api.mac.syncReq.phyID <= MAC_MRFSK_STD_PHY_ID_END)) ||
                       ((pEvent->api.mac.syncReq.phyID >= MAC_MRFSK_GENERIC_PHY_ID_BEGIN) &&
                       (pEvent->api.mac.syncReq.phyID <= MAC_MRFSK_GENERIC_PHY_ID_END)), status);
  }
#else
#ifndef FREQ_2_4G
  /* verify channel page */
  MAC_PARAM_STATUS((pEvent->api.mac.syncReq.channelPage >= MAC_CHANNEL_PAGE_9) &&
                   (pEvent->api.mac.syncReq.channelPage <= MAC_CHANNEL_PAGE_10), status);

  /* verify phy ID */
  MAC_PARAM_STATUS(((pEvent->api.mac.syncReq.phyID >= MAC_MRFSK_STD_PHY_ID_BEGIN) &&
                   (pEvent->api.mac.syncReq.phyID <= MAC_MRFSK_STD_PHY_ID_END)) ||
                   ((pEvent->api.mac.syncReq.phyID >= MAC_MRFSK_GENERIC_PHY_ID_BEGIN) &&
                   (pEvent->api.mac.syncReq.phyID <= MAC_MRFSK_GENERIC_PHY_ID_END)), status);
#endif
#endif

  if ((status == MAC_SUCCESS) &&
      (pMacPib->beaconOrder != MAC_BO_NON_BEACON) &&
      (pMacPib->panId != MAC_PAN_ID_BROADCAST) &&
      (pMacPib->coordShortAddress != MAC_SHORT_ADDR_NONE))
  {
    /* if application enabled beacon tracking */
    if (pEvent->api.mac.syncReq.trackBeacon == TRUE)
    {
      state = MAC_BEACON_TRACKING_ST;
    }
    /* if application disabled beacon tracking */
    else if (pEvent->api.mac.syncReq.trackBeacon == FALSE)
    {
      state = MAC_BEACON_SYNC_ST;
    }
    /* if this is an internally generated sync (note trackBeacon has non-boolean value) */
    else if ((pEvent->api.mac.syncReq.trackBeacon == MAC_TRACK_BEACON_TX_SYNC) &&
             (macBeacon.state == MAC_BEACON_NONE_ST))
    {
      state = MAC_BEACON_SYNC_ST;
      txSync = TRUE;
    }
    else
    {
      return;
    }

    /* stop beacon tracking and superframe timers */
    MAP_macTimerCancel(&macBeaconDevice.timeoutTimer);
    MAP_macTimerCancel(&macBeaconDevice.trackTimer);
    MAP_macTimerCancel(&macBeacon.periodTimer);

    /* initialize global variables */
    HAL_ENTER_CRITICAL_SECTION(intState);
    macBeacon.state = state;
    macBeacon.capEnd = 0;
    macBeacon.sched = MAC_TX_SCHED_INACTIVE;

    /* it is possible to for a sync to occur when a transmit is in progress; if this
     * happens we hold off sync by setting rxWindow to FALSE and keep the receiver off,
     * then enable things later from macSyncTimeoutCallback
     */
    if (pMacDataTx == NULL)
    {
      macBeacon.rxWindow = TRUE;
    }
    else
    {
      macBeacon.rxWindow = FALSE;
      DBG_PRINT0(DBGSYS, "!!! Sync occurred while transmit in progress.");
    }

    macBeacon.txSync = txSync;
    if (txSync)
    {
      macBeaconDevice.lostCount = MAC_A_MAX_LOST_BEACONS - 1;
    }
    else
    {
      macBeaconDevice.lostCount = 0;
    }
    HAL_EXIT_CRITICAL_SECTION(intState);

    /* initiate beacon search */
    MAP_MAC_MlmeSetReq(MAC_PHY_CURRENT_DESCRIPTOR_ID, &pEvent->api.mac.syncReq.phyID);
    MAP_MAC_MlmeSetReq(MAC_CHANNEL_PAGE, &pEvent->api.mac.syncReq.channelPage);
    MAP_MAC_MlmeSetReq(MAC_LOGICAL_CHANNEL, &pEvent->api.mac.syncReq.logicalChannel);

    /* if rxWindow is FALSE we're holding off sync because a tx is in progress */
    if (macBeacon.rxWindow == TRUE)
    {
      MAP_macRxEnable(MAC_RX_BEACON_DEVICE);
    }

    macBeaconDevice.timeoutTimer.pFunc = MAP_macSyncTimeoutCallback;
#ifdef COMBO_MAC
    if (rfFreq == MAC_RF_FREQ_SUBG)
    {
        MAP_macTimer(&macBeaconDevice.timeoutTimer, MAC_BEACON_TRACK_TIME(MAC_PROCESS_DELAY));
    }
    else
    {
        MAP_macTimer(&macBeaconDevice.timeoutTimer, MAC_BEACON_TRACK_TIME(MAC_IEEE_PROCESS_DELAY));
    }
#else
#ifndef FREQ_2_4G
    MAP_macTimer(&macBeaconDevice.timeoutTimer, MAC_BEACON_TRACK_TIME(MAC_PROCESS_DELAY));
#else
    MAP_macTimer(&macBeaconDevice.timeoutTimer, MAC_BEACON_TRACK_TIME(MAC_IEEE_PROCESS_DELAY));
#endif
#endif
  }
}

/**************************************************************************************************
 * @fn          macBeaconStartFrameResponseTimer
 *
 * @brief       This function starts the frame response timer.
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
MAC_INTERNAL_API void macBeaconStartFrameResponseTimer(macEvent_t *pEvent)
{
  uint32 timeout = pMacPib->maxFrameTotalWaitTime;

  (void)pEvent;  // Intentionally unreferenced parameter

  if (pMacPib->beaconOrder != MAC_BO_NON_BEACON)
  {
    /* If not enough time for frame response before CAP end, set timer to expire
     * in the next beacon interval.
     */
    if ((MAP_macTimerGetTime() + timeout) >= macBeacon.capEnd)
    {
      timeout = timeout + MAC_BEACON_INTERVAL(pMacPib->beaconOrder) +
                MAC_A_BASE_SUPERFRAME_DURATION - macBeacon.capEnd;
    }
  }

  DBG_PRINT1(DBGSYS, "Beacon Device: macBeaconStartFrameResponseTimer(timeout=%i)", timeout);

  MAP_macTimerCancel(&macDevice.frameTimer);
  MAP_macTimer(&macDevice.frameTimer, timeout);
}

/**************************************************************************************************
 * @fn          macStartBroadcastPendTimer
 *
 * @brief       This function starts the broadcast pending timer.
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
MAC_INTERNAL_API void macStartBroadcastPendTimer(macEvent_t *pEvent)
{
  (void)pEvent;  // Intentionally unreferenced parameter

  DBG_PRINT0(DBGSYS, "Beacon Device: macStartBroadcastPendTimer()");

  if (pMacPib->beaconOrder != MAC_BO_NON_BEACON)
  {
    /* if superframe order is zero, don't set timer; let end of superframe turn off rx */
    if (pMacPib->superframeOrder > 0)
    {
      MAP_macTimerCancel(&macBeaconDevice.broacastPendTimer);
      MAP_macTimer(&macBeaconDevice.broacastPendTimer,
               (uint32) pMacPib->maxFrameTotalWaitTime);
    }
  }
  /* beacon order is non-beacon; don't need to receive broadcast, just turn off rx */
  else
  {
    MAP_macRxDisable(MAC_RX_BROADCAST_PEND);
  }
}

#endif

