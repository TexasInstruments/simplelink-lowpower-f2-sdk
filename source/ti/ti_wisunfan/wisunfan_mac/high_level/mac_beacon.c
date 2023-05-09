/******************************************************************************

 @file  mac_beacon.c

 @brief Implements procedures for beacon enabled networks common to both device
        and coordinator.

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

#include "hal_types.h"
#include "hal_mcu.h"
#include "timac_api.h"
#include "mac_spec.h"
#include "mac_low_level.h"
#include "mac_radio_defs.h"
#include "mac_high_level.h"
#include "mac_pib.h"
#include "mac_main.h"
#include "mac_timer.h"
#include "mac_mgmt.h"
#include "mac_data.h"
#include "mac_beacon.h"
#include "mac_radio_tx.h"
#include "mac_tx.h"
// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_beacon_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

#include "rom_jt_154.h"

#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)

/**************************************************************************************************
 * @fn          macBeaconReset
 *
 * @brief       This function initializes the data structures for the beacon module.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macBeaconReset(void)
{
  /* initialize macBeacon struct */
  macBeacon.capStart = 0;
  macBeacon.capEnd = 0;
  macBeacon.sched = MAC_TX_SCHED_INACTIVE;
  macBeacon.state = MAC_BEACON_NONE_ST;
  macBeacon.txSync = FALSE;
  macBeacon.rxWindow = FALSE;
}

/**************************************************************************************************
 * @fn          macBeaconInit
 *
 * @brief       This function initializes the MAC to operate in beacon-enabled networks.  This
 *              function is called by MAC_InitBeaconCoord and MAC_InitBeaconDevice.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macBeaconInit(void)
{
  /* initialize function pointers */
  macDataSetSched = MAP_macBeaconSetSched;
  macDataCheckSched = MAP_macBeaconCheckSched;
  macDataCheckTxTime = MAP_macBeaconCheckTxTime;
  macDataBeaconRequeue = MAP_macBeaconRequeue;

  /* initialize beacon struct */
  MAP_macBeaconReset();
}

/**************************************************************************************************
 * @fn          macBeaconSetSched
 *
 * @brief       This function sets the scheduled transmission period of the given frame to the
 *              incoming or outgoing CAP.
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
MAC_INTERNAL_API void macBeaconSetSched(macTx_t *pMsg)
{
  ApiMac_mlmeSyncReq_t syncReq;
  uint8             *p;

  /* return immediately if nonbeacon network or scan related */
  if ((pMacPib->beaconOrder == MAC_BO_NON_BEACON) ||
      (pMsg->internal.txSched & MAC_TX_SCHED_SCAN_RELATED))
  {
    pMsg->internal.txSched |= MAC_TX_SCHED_OUTGOING_CAP;
    return;
  }

  /* set mode to slotted CSMA transmission */
  pMsg->internal.txMode |= MAC_TX_MODE_SLOTTED;

  /* if we are a device or frame destination is coordinator */
  if (!macMgmt.networkStarted || MAP_macCoordDestAddrCmp(pMsg->msdu.p))
  {
    /* frame is for incoming or outgoing cap, it could be scheduled during an
     * outgoing cap for a beaconing end device.
     */
    pMsg->internal.txSched |= (MAC_TX_SCHED_INCOMING_CAP | MAC_TX_SCHED_OUTGOING_CAP);

    /* if we are not syncing or tracking */
    if (macBeacon.state == MAC_BEACON_NONE_ST)
    {
      /* initiate sync */
      syncReq.logicalChannel = pMacPib->logicalChannel;
      syncReq.channelPage = pMacPib->channelPage;
      syncReq.phyID = pMacPib->curPhyID;
      syncReq.trackBeacon = MAC_TRACK_BEACON_TX_SYNC;
      MAP_MAC_MlmeSyncReq(&syncReq);
    }
  }
  /* else frame is for outgoing cap; check if broadcast frame */
  else
  {
    pMsg->internal.txSched |= MAC_TX_SCHED_OUTGOING_CAP;

    if(pMsg->internal.txOptions & MAC_TXOPTION_INDIRECT)
    {
      p = pMsg->msdu.p;

      if (MAC_DEST_ADDR_MODE(p) == SADDR_MODE_SHORT)
      {
        p += MAC_DEST_ADDR_OFFSET;
        if (BUILD_UINT16(*p, *(p + 1)) == MAC_SHORT_ADDR_BROADCAST)
        {
          /* set mode as broadcast */
          pMsg->internal.txMode |= MAC_TX_MODE_BROADCAST;

          /* clear ready flag to postpone transmission */
          pMsg->internal.txSched &= ~MAC_TX_SCHED_READY;
        }
      }
    }
  }
}

/**************************************************************************************************
 * @fn          macBeaconCheckSched
 *
 * @brief       This function checks the current superframe period and determines whether it
 *              is currently in the incoming, outgoing, or inactive period of the superframe.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      MAC_SCHED_INACTIVE, MAC_SCHED_INCOMING, or MAC_SCHED_OUTGOING
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macBeaconCheckSched(void)
{
  if (MAP_macStateScanning())
  {
    return MAC_TX_SCHED_SCAN_RELATED;
  }
  else if (pMacPib->beaconOrder == MAC_BO_NON_BEACON)
  {
    return MAC_TX_SCHED_OUTGOING_CAP;
  }
  else
  {
    return macBeacon.sched;
  }
}

/**************************************************************************************************
 * @fn          macBeaconRequeue
 *
 * @brief       This function is called from macDataTxCallback() to requeue a frame that could
 *              not be transmitted by low level in time for the current superframe.
 *
 * input parameters
 *
 * @param       pMsg - pointer to TX message.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macBeaconRequeue(macTx_t *pMsg)
{
  halIntState_t intState;

  /* if direct frame */
  if (!(pMsg->internal.txOptions & MAC_TXOPTION_INDIRECT))
  {
    /* push frame on to the tx queue */
    MAP_osal_msg_push(&macData.txQueue, pMsg);

    /* increment direct count */
    HAL_ENTER_CRITICAL_SECTION(intState);
    macData.directCount++;
    HAL_EXIT_CRITICAL_SECTION(intState);
  }
  /* else if indirect frame, clear ready flag and push
   * frame back on to the tx queue
   */
  else
  {
    (*macDataRequeueIndirect)(pMacDataTx);
  }
}

#if !defined (TIMAC_ROM_PATCH)
/**************************************************************************************************
 * @fn          macBeaconCheckTxTime
 *
 * @brief       This function is called from low level MAC when a frame is being transmitted
 *              with slotted CSMA or LBT.  It calculates the number of symbols remaining for
 *              a transmission to begin.
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
MAC_INTERNAL_API uint16 macBeaconCheckTxTime(void)
{
  int32 timeLeft;

  /* If non beacon return 0 */
  if (pMacPib->beaconOrder == MAC_BO_NON_BEACON)
  {
    return 0;
  }

  /* if inactive then no time left */
  if (macBeacon.sched == MAC_TX_SCHED_INACTIVE)
  {
    return 0;
  }

  if(pMacPib->rfFreq == MAC_RF_FREQ_2_4G)
  {
      /* transmission must begin and end before end of CAP */
      timeLeft = ((int32) macBeacon.capEnd - (int32) MAP_macTimerGetTime() - macData.duration -
                 16*macBeaconMargin[pMacPib->superframeOrder]);
  }
  else
  {
  /* if incoming superframe with batt life ext */
  if (macBeacon.battLifeExt && macBeacon.sched == MAC_TX_SCHED_INCOMING_CAP)
  {
    /* transmission must begin before end of batt life ext period */
    timeLeft = (int32) MAP_macTimerGetTime() - (int32) macBeacon.capStart -
               (pMacPib->battLifeExtPeriods * MAC_A_UNIT_BACKOFF_CCA_PERIOD);
  }
  else
  {
    /* transmission must begin and end before end of CAP */
    timeLeft = ((int32) macBeacon.capEnd - (int32) MAP_macTimerGetTime() - macData.duration -
               macBeaconMargin[pMacPib->superframeOrder]);
    timeLeft -= MAP_macRadioTxBOBoundary();
  }
  }
  DBG_PRINTL1(DBGSYS, "macBeaconCheckTxTime(timeLeft=%li)", timeLeft);
  if (timeLeft < 0)
  {
    return 0;
  }
  else
  {
    return (uint16) MIN(timeLeft, 65535);
  }
}
#endif

/**************************************************************************************************
 * @fn          macBeaconSetupCap
 *
 * @brief       This function sets the CAP start and end time of the current superframe.
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
MAC_INTERNAL_API void macBeaconSetupCap(uint8 sched, uint8 superframeOrder, uint16 beaconLen)
{
  /* CAP starts after beacon frame transmission rounded up to next symbol period */
  macBeacon.capStart = MAP_macFrameDuration((uint16)beaconLen, 0);

  /* CAP ends at final CAP slot */
  if (superframeOrder == MAC_SO_NONE)
  {
    macBeacon.capEnd = 0;
  }
  else if (superframeOrder == pMacPib->beaconOrder)
  {
    macBeacon.capEnd = ((((uint32) (macBeacon.finalCapSlot + 1))  << superframeOrder) *
                     MAC_A_BASE_SLOT_DURATION) - 160;
  }
  else
  {
    macBeacon.capEnd = ((((uint32) (macBeacon.finalCapSlot + 1))  << superframeOrder) *
                     MAC_A_BASE_SLOT_DURATION) - 16;
  }

  macBeacon.sched = sched;
  if (sched == MAC_TX_SCHED_OUTGOING_CAP)
  {
     macBeacon.capStart += pMacPib->beaconTxTime;
     macBeacon.capEnd += pMacPib->beaconTxTime;
  }

  DBG_PRINTL2(DBGSYS, "macBeaconSetupCap(capStart=%i, capEnd=%i)", macBeacon.capStart, macBeacon.capEnd);
}

#endif
