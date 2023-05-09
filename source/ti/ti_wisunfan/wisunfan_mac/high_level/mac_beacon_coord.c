/******************************************************************************

 @file  mac_beacon_coord.c

 @brief Implements coordinator only procedures for beacon enabled networks.

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

#ifdef FEATURE_BEACON_MODE
#include "hal_mcu.h"
#include "timac_api.h"
#include "mac_main.h"
#include "mac_timer.h"
#include "mac_mgmt.h"
#include "mac_spec.h"
#include "mac_data.h"
#include "mac_pib.h"
#include "mac_coord.h"
#include "mac_low_level.h"
#include "mac_high_level.h"
#include "mac_beacon.h"
#include "mac_beacon_coord.h"
#include "mac_assert.h"
#include "mac_security.h"
#include "mac_ie_build.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_beacon_coord_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

#include "rom_jt_154.h"


/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */
void macBeaconSetPrepareTime(void);
uint8 macBeaconCheckStartTime(macEvent_t *pEvent);
void macBeaconPeriodCallback(uint8 param);
void macBeaconTxCallback(uint8 param);
void macStartBeaconPrepareCallback(uint8 param);
void macBeaconBattLifeCallback(uint8 param);
#ifdef FEATURE_ENHANCED_BEACON
void macEBeaconPrepareCallback(uint8 param);
void macEBeaconTxCallback(uint8 param);
#endif /* FEATURE_ENHANCED_BEACON */
MAC_INTERNAL_API void macBeaconCoordReset(void);

/**************************************************************************************************
 * @fn          macBeaconCoordReset
 *
 * @brief       This function initializes the data structures for module beacon coord.
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
MAC_INTERNAL_API void macBeaconCoordReset(void)
{
  macBeaconCoordAction1[0] = MAP_macApiBeaconStartReq;
  macBeaconCoordAction1[1] = MAP_macBeaconStartContinue;

  macActionSet[MAC_COORD_ACTION_1] = macBeaconCoordAction1;
}

/**************************************************************************************************
 * @fn          MAC_InitBeaconCoord
 *
 * @brief       Initializes the MAC to operate as a coordinator in a beacon-enabled network.
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
void MAC_InitBeaconCoord(void)
{
  macBeaconCoordAction1[0] = MAP_macApiBeaconStartReq;
  macBeaconCoordAction1[1] = MAP_macBeaconStartContinue;

  macDataTxBeaconComplete = MAP_macTxBeaconCompleteCallback;
  macActionSet[MAC_COORD_ACTION_1] = macBeaconCoordAction1;

  macReset[MAC_FEAT_BEACON_COORD] = MAP_macBeaconCoordReset;

  MAP_macBeaconInit();
  macMain.featureMask |= MAC_FEAT_BEACON_COORD_MASK;
}

/**************************************************************************************************
 * @fn          macBeaconSetPrepareTime
 *
 * @brief       This function calculates the beacon prepare time.
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
void macBeaconSetPrepareTime(void)
{
  if (pMacPib->beaconTxTime)
  {
    macBeaconCoord.prepareTime = pMacPib->beaconTxTime -
                                 MAC_BEACON_PREPARE_OFFSET -
                                 (macTxSlottedDelay);
  }
  else
  {
    macBeaconCoord.prepareTime = MAC_BEACON_INTERVAL(pMacPib->beaconOrder) -
                                 MAC_BEACON_PREPARE_OFFSET -
                                 (macTxSlottedDelay);
  }
}

/**************************************************************************************************
 * @fn          macBeaconCheckStartTime
 *
 * @brief       Verifies that the parameters to a Start Request are in range.
 *
 * input parameters
 *
 * @param       pEvent - Pointer to event data.
 *
 * output parameters
 *
 * None.
 *
 * @return      status of the operation.
 **************************************************************************************************
 */
uint8 macBeaconCheckStartTime(macEvent_t *pEvent)
{
  uint8 beaconOrder = pEvent->api.mac.startReq.beaconOrder;
  uint16 margin = macBeaconMargin[beaconOrder] * 2;
  uint32 startTime = pEvent->api.mac.startReq.startTime;

  /* success if non-beaconed */
  if ((beaconOrder == MAC_BO_NON_BEACON) || (macBeacon.state != MAC_BEACON_TRACKING_ST))
  {
    return MAC_SUCCESS;
  }

  /* if not coordinator, verify start time is valid */
  if (!pEvent->api.mac.startReq.panCoordinator)
  {
    if ((startTime < (uint32)(MAC_SF_INTERVAL(pMacPib->superframeOrder) + margin)) ||
        (startTime > (uint32)(MAC_BEACON_INTERVAL(pMacPib->beaconOrder) - margin)))

    {
      return MAC_SUPERFRAME_OVERLAP;
    }

    /* check to see if the outgoing superframe duration is not greater than the inactive period
     * of the incoming superframe.
     */
    if (MAC_SF_INTERVAL(pEvent->api.mac.startReq.superframeOrder) >
        ((uint32)(MAC_BEACON_INTERVAL(pMacPib->beaconOrder)) - (uint32)(MAC_SF_INTERVAL(pMacPib->superframeOrder))))
    {
      return MAC_SUPERFRAME_OVERLAP;
    }

    pMacPib->beaconTxTime = startTime;
  }
  else
  {
    /* ignore start time in PAN coordinator */
    pMacPib->beaconTxTime = 0;
  }

  return MAC_SUCCESS;
}

/**************************************************************************************************
 * @fn          macApiBeaconStartReq
 *
 * @brief       This action function handles an API start request event for
 *              beacon enabled networks.
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
MAC_INTERNAL_API void macApiBeaconStartReq(macEvent_t *pEvent)
{
  uint8   status;
  macTx_t *pMsg;
  sAddr_t dstAddr;

  /* parameter check */
  status = MAP_macStartBegin(pEvent);

  if (status == MAC_SUCCESS)
  {
    /* Check start timer */
    status = MAP_macBeaconCheckStartTime(pEvent);

    /* handle coordRealignment */
    if (status == MAC_SUCCESS && pEvent->api.mac.startReq.coordRealignment &&
        (!pEvent->api.mac.startReq.startFH) && !MAC_FH_ENABLED)
    {
      /* allocate buffer */
      if ((pMsg = MAP_macAllocTxBuffer(MAC_INTERNAL_COORD_REALIGN, &pEvent->api.mac.startReq.realignSec)) != NULL)
      {
        dstAddr.addrMode = SADDR_MODE_SHORT;
        dstAddr.addr.shortAddr = MAC_SHORT_ADDR_BROADCAST;
        MAP_macBuildRealign(pMsg, &dstAddr, MAC_SHORT_ADDR_BROADCAST,
                        pEvent->api.mac.startReq.panId,
                        pEvent->api.mac.startReq.logicalChannel);
        return;
      }
    }
  }

  pEvent->hdr.status = status;
  MAP_macBeaconStartContinue(pEvent);
}
#endif
/**************************************************************************************************
 * @fn          macBeaconPeriodCallback
 *
 * @brief       This timer callback is executed at the start of the next period.
 *
 * input parameters
 *
 * @param       param - length of period.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void macBeaconPeriodCallback(uint8 param)
{
  (void)param;  // Intentionally unreferenced parameter

  DBG_PRINT0(DBGSYS, "macBeaconPeriodCallback()");

  macBeacon.sched = MAC_TX_SCHED_INACTIVE;

  /* clear ready flag on any queued indirect frames */
  MAP_macBeaconClearIndirect();

  /* if there is an inactive period */
  if (pMacPib->superframeOrder != pMacPib->beaconOrder)
  {
    /* disable rx */
    MAP_macRxHardDisable();
  }

  /* Send message to process indirect data */
  MAP_macSetEvent(MAC_EXP_INDIRECT_TASK_EVT);
}

/**************************************************************************************************
 * @fn          macBeaconTxCallback
 *
 * @brief       This timer callback transmits a beacon that has been previously prepared.
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
void macBeaconTxCallback(uint8 param)
{
  (void)param;  // Intentionally unreferenced parameter

  if (macBeaconCoord.pBeacon != NULL)
  {
    if (pMacDataTx == NULL)
    {
      pMacDataTx = macBeaconCoord.pBeacon;
      macBeaconCoord.pBeacon = NULL;

      DBG_PRINT0(DBGSYS, "macBeaconTxCallback, TX BEACON ");

      /* transmit the beacon frame */
      MAP_macTxFrame(MAC_TX_TYPE_SLOTTED);

      /* For ourgoing CAP, keep receiver on after beacon tx */
      MAP_macRxSoftEnable(MAC_RX_WHEN_IDLE);
    }
    else
    {
      /* if pMacDataTx is set, free the beacon buffer; this should never happen,
       * but recover gracefully rather than assert.
       */
      MAP_mac_msg_deallocate((uint8 **)&macBeaconCoord.pBeacon);
    }
  }

  DBG_PRINTL1(DBGSYS, "macBeaconTxCallback, SymCnt1 = %d", macTimerGetTime());

  /* restart the prepare timer for the next beacon */
  if (macMgmt.networkStarted && pMacPib->beaconOrder != MAC_BO_NON_BEACON)
  {
    macBeaconCoord.prepareTimer.pFunc = MAP_macBeaconPrepareCallback;
    MAP_macTimerAligned(&macBeaconCoord.prepareTimer, macBeaconCoord.prepareTime);
  }
}

#ifdef FEATURE_ENHANCED_BEACON
/**************************************************************************************************
 * @fn          macEBeaconTxCallback
 *
 * @brief       This timer callback transmits enhanced beacon that has been previously prepared.
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
void macEBeaconTxCallback(uint8 param)
{
  (void)param;  // Intentionally unreferenced parameter

  DBG_PRINTL1(DBGSYS, "macEBeaconTxCallback, SymCnt = %lu", macTimerGetTime());

  if (macBeaconCoord.pBeacon != NULL)
  {
    if (pMacDataTx == NULL)
    {
      pMacDataTx = macBeaconCoord.pBeacon;
      macBeaconCoord.pBeacon = NULL;

      if ( macCoord.beaconing )
      {
        /* transmit the beacon frame */
        MAP_macTxFrame(MAC_TX_TYPE_SLOTTED);

        /* For ourgoing CAP, keep receiver on after beacon tx */
        MAP_macRxSoftEnable(MAC_RX_WHEN_IDLE);
      }
      else
      {
        /* transmit the beacon frame */
        MAP_macTxFrame(MAC_TX_TYPE_UNSLOTTED_CSMA);
        /* set the next ebeacon beacon prepare timer */
        MAP_macTimer(&macBeaconCoord.eBeaconPrepareTimer, macBeaconCoord.eBeaconPrepareTime);
      }

      macBeaconCoord.eBeaconStatus = MAC_EBEACON_QUEUED;
    }
    else
    {
      /* if pMacDataTx is set, free the beacon buffer; this should never happen,
       * but recover gracefully rather than assert.
       */
      MAP_mac_msg_deallocate((uint8 **)&macBeaconCoord.pBeacon);
      macBeaconCoord.pBeacon = NULL;

      if ( macCoord.beaconing )
      {
        macBeacon.sched = macBeaconCoord.origSched;
      }
      else
      {
        /* For non beacon mode, set the next ebeacon beacon prepare timer */
        MAP_macTimer(&macBeaconCoord.eBeaconPrepareTimer, macBeaconCoord.eBeaconPrepareTime);
      }
    }
  }
}

/**************************************************************************************************
 * @fn          macEBeaconPrepareCallback
 *
 * @brief       This timer callback is called to prepare for the transmission of an enhanced beacon.
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
void macEBeaconPrepareCallback(uint8 param)
{
  (void)param;  // Intentionally unreferenced parameter

  /* set a timer for next enhanced beacon beacon */
  if (macMgmt.networkStarted)
  {
    uint8 phyId;
    DBG_PRINTL1(DBGSYS, "HMAC: macEBeaconPrepareCallback, SymCnt1 = %d", macTimerGetTime());

    /* start tx timer for beacon transmission */
    macBeaconCoord.eBeaconTxTimer.pFunc = macEBeaconTxCallback;

    if ( macCoord.beaconing )
    {
      /* forced the schedule to inactive as we will be
         switching the PHY  */
      macBeaconCoord.origSched = macBeacon.sched;
      macBeacon.sched = MAC_TX_SCHED_INACTIVE;

      MAP_macTimerAligned(&macBeaconCoord.eBeaconTxTimer,
                      macBeaconCoord.eBeaconPrepareTime + MAC_EBEACON_PREPARE_OFFSET);
    }
    else
    {
      DBG_PRINT0(DBGSYS, "HMAC: macEBeaconPrepareCallback, setting macTxCallback timer ");
      MAP_macTimer(&macBeaconCoord.eBeaconTxTimer, MAC_EBEACON_PREPARE_OFFSET);
    }

    /* build the beacon */
    if ((macBeaconCoord.pBeacon = macBuildEnhancedBeacon(pMacPib->beaconOrder,
                                  pMacPib->superframeOrder, pMacPib->battLifeExt, 1)) != NULL)
    {
      /* set frame as beacon mode */
      macBeaconCoord.pBeacon->internal.txMode |= MAC_TX_MODE_BEACON;
    }

    /* save the phy ID */
    if ( MAC_EBEACON_NOT_STARTED == macBeaconCoord.eBeaconStatus )
    {
      macBeaconCoord.origPhyId = pMacPib->curPhyID;
    }

    macBeaconCoord.eBeaconStatus = MAC_EBEACON_PREPARED;

    /* Get the corresponding CSM Phy Id */
    phyId = MAP_macGetCSMPhy( pMacPib->curPhyID );

    if ( phyId != pMacPib->curPhyID )
    {
      MAP_macUpdateCsmPhyParam(TRUE);
      /* Set the requested/csm Phy ID */
      MAP_MAC_MlmeSetReq(MAC_PHY_CURRENT_DESCRIPTOR_ID, &phyId);
    }
  }
}
#endif /* FEATURE_ENHANCED_BEACON */

/**************************************************************************************************
 * @fn          macBeaconPrepareCallback
 *
 * @brief       This timer callback is called to prepare for the transmition of a beacon.
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
MAC_INTERNAL_API void macBeaconPrepareCallback(uint8 param)
{
  (void)param;  // Intentionally unreferenced parameter

  /* set a timer for next beacon */
  if (macMgmt.networkStarted)
  {
    /* start tx timer for beacon transmission */
    macBeaconCoord.txTimer.pFunc = MAP_macBeaconTxCallback;

    MAP_macTimerAligned(&macBeaconCoord.txTimer,
                    macBeaconCoord.prepareTime + MAC_BEACON_PREPARE_OFFSET);

    DBG_PRINTL1(DBGSYS, "HMAC: macBeaconPrepareCallback, SymCnt = %lu", macTimerGetTime());

    /* build the beacon */
    if (MAP_macStateScanning() == FALSE)
    {
      if ((macBeaconCoord.pBeacon = MAP_macBuildBeacon(pMacPib->beaconOrder,
                                    pMacPib->superframeOrder, pMacPib->battLifeExt)) != NULL)
      {
        /* set frame as beacon mode */
        macBeaconCoord.pBeacon->internal.txMode |= MAC_TX_MODE_BEACON;
      }
    }
    else
    {
      (pMacPib->bsn)++;
      macBeaconCoord.pBeacon = NULL;
    }
  }
}

/**************************************************************************************************
 * @fn          macStartBeaconPrepareCallback
 *
 * @brief       This timer callback is called to prepare for the transmition of a beacon
 *              when an existing beacon configuration has changed.
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
void macStartBeaconPrepareCallback(uint8 param)
{
  /* Record the start request parameters in the pib */
  MAP_macStartSetParams(macCoord.pStartReq);

  /* complete the start request */
  MAP_macSetEvent(MAC_START_COMPLETE_TASK_EVT);

  /* prepare the beacon */
  MAP_macBeaconPrepareCallback(param);

  /* Tell the timer to sync to a new beacon and change prep time */
  MAP_macBeaconSetPrepareTime();
  if (macPanCoordinator)
  {
    MAP_macTimerSyncRollover(pMacPib->beaconOrder);
  }
}

/**************************************************************************************************
 * @fn          macBeaconStartContinue
 *
 * @brief       This action function continues an API start request event for
 *              beacon enabled networks.
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
MAC_INTERNAL_API void macBeaconStartContinue(macEvent_t *pEvent)
{
  halIntState_t intState;
  uint8 initialBeaconOrder = pMacPib->beaconOrder;

  if (pEvent->hdr.status == MAC_SUCCESS)
  {
    if (macMgmt.networkStarted && initialBeaconOrder != MAC_BO_NON_BEACON)
    {
      /* the beacon interval is changing, prepare timer is already running;
       * just switch the callback function
       */
      HAL_ENTER_CRITICAL_SECTION(intState);
      macBeaconCoord.prepareTimer.pFunc = MAP_macStartBeaconPrepareCallback;
      HAL_EXIT_CRITICAL_SECTION(intState);
      return;
    }
    else if (macCoord.pStartReq->beaconOrder != MAC_BO_NON_BEACON)
    {
      MAP_macStartSetParams(macCoord.pStartReq);
      MAP_macBeaconSetPrepareTime();

      if (initialBeaconOrder == MAC_BO_NON_BEACON)
      {
        /* initialize timer alignment to begin beacon transmssion */
        MAP_macTimerStart(macBeaconCoord.prepareTime - MAC_BEACON_START_OFFSET,
                      macCoord.pStartReq->beaconOrder);
      }

      /*  start the prepare timer */
      macBeaconCoord.prepareTimer.pFunc = MAP_macBeaconPrepareCallback;
      MAP_macTimerAligned(&macBeaconCoord.prepareTimer, macBeaconCoord.prepareTime);

      if ( MAC_BO_NON_BEACON != macCoord.pStartReq->mpmParams.eBeaconOrder )
      {
        macBeaconCoord.eBeaconStatus = MAC_EBEACON_NOT_STARTED;
      }
    }
    else
    {
      MAP_macStartSetParams(macCoord.pStartReq);

      /* If start request is made and frequency hopping is enabled
         start the Frequency hopping */
      if (macCoord.pStartReq->startFH && MAC_FH_ENABLED && MAC_FH_START_FN)
      {
        MAC_FH_START_FN();
      }
#ifdef FEATURE_ENHANCED_BEACON
      else if ( MAC_EBEACON_ORDER_NBPAN_MAX != macCoord.pStartReq->mpmParams.NBPANEBeaconOrder )
      {
        /* start periodic enhanced beacon in a non beacon enabled PAN */
        macBeaconCoord.eBeaconPrepareTime = MAC_A_BASE_SLOT_DURATION *
          pMacPib->eBeaconOrderNBPAN - MAC_EBEACON_PREPARE_OFFSET;
        macBeaconCoord.eBeaconPrepareTimer.pFunc = macEBeaconPrepareCallback;
        MAP_macTimer( &macBeaconCoord.eBeaconPrepareTimer, macBeaconCoord.eBeaconPrepareTime );
        macBeaconCoord.eBeaconStatus = MAC_EBEACON_NOT_STARTED;
      }
#endif /* FEATURE_ENHANCED_BEACON */
    }
  }

  /* complete the start */
  pEvent->hdr.event = MAC_INT_START_COMPLETE_EVT;
  MAP_macExecute(pEvent);
}

/**************************************************************************************************
 * @fn          macBeaconBattLifeCallback
 *
 * @brief       This callback expires at the end of the battery life period.
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
void macBeaconBattLifeCallback(uint8 param)
{
  (void)param;  // Intentionally unreferenced parameter

  MAP_macRxDisable(MAC_RX_WHEN_IDLE);
}

/**************************************************************************************************
 * @fn          macTxBeaconCompleteCallback
 *
 * @brief       Is callback is called by the lower level when transmition of a beacon has
 *              completed.
 *
 * input parameters
 *
 * @param       status - status of the transmition.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTxBeaconCompleteCallback(uint8 status)
{
#ifdef COMBO_MAC
  uint8 rfFreq = pMacPib->rfFreq;
#endif

  DBG_PRINT1(DBGSYS, "macTxBeaconCompleteCallback status=0x%X", status);

  /* if beacon transmitted successfully and we're in a beacon network */
  if ((status == MAC_SUCCESS) && (pMacPib->beaconOrder != MAC_BO_NON_BEACON))
  {
    macBeacon.finalCapSlot = MAC_A_NUM_SUPERFRAME_SLOTS - 1;

    DBG_PRINTL1(DBGSYS, "HMAC: macTxBeaconCompleteCallback, SymCnt1 = %d", macTimerGetTime());

    /* set up the cap */
    MAP_macBeaconSetupCap(MAC_TX_SCHED_OUTGOING_CAP,
                      pMacPib->superframeOrder,
                      pMacDataTx->msdu.len);

    /* set battery life timer */
    if (pMacPib->battLifeExt && pMacPib->rxOnWhenIdle)
    {
      macBeaconCoord.battLifeTimer.pFunc = MAP_macBeaconBattLifeCallback;
      MAP_macTimerAligned(&macBeaconCoord.battLifeTimer,
                      macBeacon.capStart + MAC_BEACON_BATT_LIFE_OFFSET +
                      pMacPib->battLifeExtPeriods);
    }

    /* set the period timer */
    if (pMacPib->superframeOrder == MAC_SO_NONE)
    {
      MAP_macRxHardDisable();
    }
    else
    {
      DBG_PRINTL1(DBGSYS, "HMAC: macTxBeaconCompleteCallback, SymCnt2 = %d", macTimerGetTime());

#ifdef FEATURE_ENHANCED_BEACON
      if ( macPanCoordinator &&
           (pMacPib->eBeaconOrder != MAC_ENHANCED_BEACON_ORDER_MAX))
      {

        /* Check whether it is enhanced beacon */
        if( pMacDataTx->internal.ptrHeaderIE)
        {
          /* set enhanced beacon prepare time equal to EBI */
          macBeaconCoord.eBeaconPrepareTime +=
            MAC_BEACON_INTERVAL(pMacPib->eBeaconOrder);

          macBeaconCoord.eBeaconStatus = MAC_EBEACON_TX;

          /* switch back to the original Phy */
          if ( macBeaconCoord.origPhyId != pMacPib->curPhyID )
          {
            /* Set the original Phy ID */
            MAP_MAC_MlmeSetReq(MAC_PHY_CURRENT_DESCRIPTOR_ID, &macBeaconCoord.origPhyId);
          }

          /* set the original schedule */
          macBeacon.sched = macBeaconCoord.origSched;
        }
        else
        {
          /* normal beacon, set the enhanced beacon prepare time to the offset value */
          macBeaconCoord.eBeaconPrepareTime =
            macBeacon.capStart + (pMacPib->offsetTimeSlot * MAC_A_BASE_SLOT_DURATION)
            - MAC_EBEACON_PREPARE_OFFSET - macTxSlottedDelay;

          /* set the cap end period callback timer */
          macBeacon.periodTimer.pFunc = MAP_macBeaconPeriodCallback;
          MAP_macTimerAligned(&macBeacon.periodTimer, macBeacon.capEnd);
        }

        /* Check to make sure that the next EBeacon will fall within the CAP period */
        if ( macBeaconCoord.eBeaconPrepareTime < macBeacon.capEnd)
        {
          /* set the enhanced beacon period callback */
          macBeaconCoord.eBeaconPrepareTimer.pFunc = macEBeaconPrepareCallback;

          MAP_macTimerAligned(&macBeaconCoord.eBeaconPrepareTimer,
                          macBeaconCoord.eBeaconPrepareTime);
        }

      }
      else
#endif /* FEATURE_ENHANCED_BEACON */
      {
        macBeacon.periodTimer.pFunc = MAP_macBeaconPeriodCallback;
        MAP_macTimerAligned(&macBeacon.periodTimer, macBeacon.capEnd);
      }
    }

    /* If we have a broadcast pending set it up.
       NOTE: The data is reversed before sending it out (as hw cannot do it),
       and hence we need to use the macro which can intepret data in reversed fashion.
     */
#ifdef COMBO_MAC
    if (rfFreq == MAC_RF_FREQ_2_4G)
    {
        if (MAC_FRAME_PENDING_BIT(pMacDataTx->msdu.p))
        {
          MAP_macBeaconSetupBroadcast();
        }
    }
    else
    {
        if (MAC_FRAME_PENDING_BIT_REVERSED(pMacDataTx->msdu.p))
        {
          MAP_macBeaconSetupBroadcast();
        }
    }
#else
#ifdef FREQ_2_4G
    if (MAC_FRAME_PENDING_BIT(pMacDataTx->msdu.p))
    {
      MAP_macBeaconSetupBroadcast();
    }
#else
    if (MAC_FRAME_PENDING_BIT_REVERSED(pMacDataTx->msdu.p))
    {
      MAP_macBeaconSetupBroadcast();
    }
#endif
#endif
/**
 * Added compile flag (and is not defined) around this function
 * which sets indirect requested frames (from the previous
 * superframe) to be ready to be tx in this new superframe.
 * We are turning off this feature as it is causing the
 * association response frame/data frames to be sent in the new
 * superframe without the new data request pull from the device.
 * This change has the side effect of device needing to send the
 * data request in every new superframe if its pending address
 * (even when it received frame pending in the ack of the data
 * request in the prior superframe) is set in the beacon.
 */
#ifdef MAC_SET_READY_INDIRECT_REQUESTED
    /* mark any requested indirect frames as ready */
    MAP_macBeaconSchedRequested();
#endif
  }
  /* if beacon tx is complete and beacon order is non-beacon, we switched from
   * beacon-enabled to non-beacon
   */
  else if (pMacPib->beaconOrder == MAC_BO_NON_BEACON)
  {
    /* Check whether it is enhanced beacon */
    if( pMacDataTx->internal.ptrHeaderIE)
    {
      macBeaconCoord.eBeaconStatus = MAC_EBEACON_TX;
      /* switch back to the original Phy */
      if ( macBeaconCoord.origPhyId != pMacPib->curPhyID )
      {
        /* Set the original Phy ID */
        MAP_MAC_MlmeSetReq(MAC_PHY_CURRENT_DESCRIPTOR_ID, &macBeaconCoord.origPhyId);
      }
    }

    /* set receiver to rxOnWhenIdle state */
    if (pMacPib->rxOnWhenIdle)
    {
      MAP_macRxEnable(MAC_RX_WHEN_IDLE);
    }

    /* set up any queued frames as ready to transmit now */
    MAP_macOutgoingNonSlottedTx();

    /* trigger expiration of indirect data */
    MAP_macSetEvent(MAC_EXP_INDIRECT_TASK_EVT);
  }

  MAP_mac_msg_deallocate((uint8**)&pMacDataTx);
}

/**************************************************************************************************
 * @fn          macBeaconSetupBroadcast
 *
 * @brief       This function sets up a pending broadcast frame to be transmitted at the
 *              very beginning of the outgoing superframe.
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
MAC_INTERNAL_API void macBeaconSetupBroadcast(void)
{
  macTx_t       *pCurr;
  macTx_t       *pPrev=NULL;
  halIntState_t intState;

  HAL_ENTER_CRITICAL_SECTION(intState);

  /* find first broadcast message in tx queue */
  pCurr = MAP_OSAL_MSG_Q_HEAD(&macData.txQueue);
  while (pCurr != NULL)
  {
    if (pCurr->internal.txMode & MAC_TX_MODE_BROADCAST)
    {
      /* take message out of queue */
      MAP_osal_msg_extract(&macData.txQueue, pCurr, pPrev);

      break;
    }
    pPrev = pCurr;
    pCurr = MAP_OSAL_MSG_NEXT(pCurr);
  }
  HAL_EXIT_CRITICAL_SECTION(intState);

  /* if broadcast message found */
  if (pCurr != NULL)
  {
    /* mark as ready */
    pCurr->internal.txSched |= MAC_TX_SCHED_READY;

    /* push it to front of queue */
    MAP_osal_msg_push(&macData.txQueue, pCurr);
  }
}

/**************************************************************************************************
 * @fn          macOutgoingNonSlottedTx
 *
 * @brief       This function sets up all frames for non-slotted TX.  It
 *              also marks any queued broadcast frames as ready for transmission.
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
MAC_INTERNAL_API void macOutgoingNonSlottedTx(void)
{
  macTx_t       *pCurr;
  halIntState_t intState;

  HAL_ENTER_CRITICAL_SECTION(intState);

  /* iterate over tx queue */
  pCurr = MAP_OSAL_MSG_Q_HEAD(&macData.txQueue);
  while (pCurr != NULL)
  {
    /* set as non-slotted */
    pCurr->internal.txMode &= ~MAC_TX_MODE_SLOTTED;

    /* if broadcast frame mark as ready */
    if (pCurr->internal.txMode & MAC_TX_MODE_BROADCAST)
    {
      pCurr->internal.txSched |= MAC_TX_SCHED_READY;
    }
    pCurr = MAP_OSAL_MSG_NEXT(pCurr);
  }

  HAL_EXIT_CRITICAL_SECTION(intState);
}

/**************************************************************************************************
 * @fn          macBeaconSchedRequested
 *
 * @brief       This function marks any queued requested indirect frames as ready for transmission.
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
MAC_INTERNAL_API void macBeaconSchedRequested(void)
{
  macTx_t       *pCurr;
  halIntState_t intState;

  HAL_ENTER_CRITICAL_SECTION(intState);

  /* iterate over tx queue */
  pCurr = MAP_OSAL_MSG_Q_HEAD(&macData.txQueue);
  while (pCurr != NULL)
  {
    /* if broadcast frame mark as ready */
    if (pCurr->internal.txMode & MAC_TX_MODE_REQUESTED)
    {
      pCurr->internal.txMode &= ~MAC_TX_MODE_REQUESTED;
      pCurr->internal.txSched |= MAC_TX_SCHED_READY;
    }
    pCurr = MAP_OSAL_MSG_NEXT(pCurr);
  }

  HAL_EXIT_CRITICAL_SECTION(intState);
}

/**************************************************************************************************
 * @fn          macBeaconClearIndirect
 *
 * @brief       This function clears the ready flag in any queued indirect frames.
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
MAC_INTERNAL_API void macBeaconClearIndirect(void)
{
  macTx_t       *pCurr;
  halIntState_t intState;

  HAL_ENTER_CRITICAL_SECTION(intState);

  /* iterate over tx queue */
  pCurr = MAP_OSAL_MSG_Q_HEAD(&macData.txQueue);
  while (pCurr != NULL)
  {
    /* if indirect frame mark as ready */
    if (pCurr->internal.txSched & MAC_TX_SCHED_INDIRECT)
    {
      pCurr->internal.txSched &= ~MAC_TX_SCHED_READY;
    }
    pCurr = MAP_OSAL_MSG_NEXT(pCurr);
  }

  HAL_EXIT_CRITICAL_SECTION(intState);
}
#endif /* (FEATURE_BEACON_MODE) */

