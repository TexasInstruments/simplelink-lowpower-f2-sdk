/******************************************************************************

 @file  mac_scan.c

 @brief This module implements procedures required for scan.

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
#ifndef LIBRARY
#include "ti_wisunfan_features.h"
#endif

#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)
/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "timac_api.h"
#include "mac_spec.h"
#include "mac_main.h"
#include "mac_timer.h"
#include "mac_mgmt.h"
#include "mac_pib.h"
#include "mac_security_pib.h"
#include "mac_data.h"
#include "mac_scan.h"
#include "mac_low_level.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_scan_c
#endif
#include "dbgid_sys_mst.h"
//#include "dbgid_sys_slv.h"
#endif // DEBUG_SW_TRACE

/******************************************************************************
  ROM jump table will support
  1. TIMAC ROM image build
  2. TIMAC stack build without ROM image
  3. TIMAC stack with calling ROM image
 *****************************************************************************/
#include "rom_jt_154.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

/* Maximum scan duration */
#define MAC_SCAN_DURATION_MAX           14

/* Value to determine last channel for end of scan */
#define MAC_SCAN_CHAN_END               (MAC_CHAN_HIGHEST + 1)

/* Value indicating scanning has not started */
#define MAC_SCAN_CHAN_INIT              (MAC_SCAN_CHAN_END + 1)

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */

void macScanCnfInit(macMlmeScanCnf_t *pScanCnf, macEvent_t *pEvent);

/**************************************************************************************************
 * @fn          macScanCnfInit
 *
 * @brief       This function initializes a scan confirm data structure from data in the api
 *              scan request event.
 *
 * input parameters
 *
 * @param       pScanCnf - Pointer to scan confirm data.
 * @param       pEvent - Pointer to event data.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void macScanCnfInit(macMlmeScanCnf_t *pScanCnf, macEvent_t *pEvent)
{
  pScanCnf->hdr.event = MAC_MLME_SCAN_CNF;
  pScanCnf->scanType = pEvent->api.mac.scanReq.scanType;
  pScanCnf->channelPage = pEvent->api.mac.scanReq.channelPage;
  pScanCnf->phyID = pEvent->api.mac.scanReq.phyID;
  MAP_osal_memcpy(pScanCnf->unscannedChannels, pEvent->api.mac.scanReq.scanChannels, MAC_154G_CHANNEL_BITMAP_SIZ);
  if (pScanCnf->scanType == MAC_SCAN_ED)
  {
    /* For energy scan, set the result list size to the allocated size of the list*/
    pScanCnf->resultListSize = APIMAC_154G_MAX_NUM_CHANNEL;
  }
  else
  {
    pScanCnf->resultListSize = 0;
  }
  pScanCnf->result.pEnergyDetect = pEvent->api.mac.scanReq.result.pEnergyDetect;
}

/**************************************************************************************************
 * @fn          macApiScanReq
 *
 * @brief       This function handles an API scan request event.  It first copies the API
 *              parameters to the macScan structure.  Then it verifies the parameters.  If
 *              the parameters are out of range it sets macScan.scanCnf.hdr.status to
 *              MAC_INVALID_PARAMETER and calls macExecute() to send a MAC_INT_SCAN_COMPLETE
 *              to the MAC.  If parameters are ok it then calls macRadioStartScan() to initialize
 *              the receiver for scanning.  Then it calls macExecute() to send a MAC_INT_TIM_SCAN_EVT
 *              to the MAC.  This sets the first channel and starts the scan.
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
MAC_INTERNAL_API void macApiScanReq(macEvent_t *pEvent)
{
#ifdef COMBO_MAC
    uint8 rfFreq = pMacPib->rfFreq;
#endif
  macScan.scanCnf.hdr.status = MAC_SUCCESS;

  /* check api parameters */
  MAC_PARAM_STATUS(pEvent->api.mac.scanReq.scanType <= MAC_SCAN_ACTIVE_ENHANCED,
                   macScan.scanCnf.hdr.status);
  MAC_PARAM_STATUS((pEvent->api.mac.scanReq.scanDuration <= MAC_SCAN_DURATION_MAX) ||
                   (pEvent->api.mac.scanReq.scanType == MAC_SCAN_ORPHAN),
                   macScan.scanCnf.hdr.status);
  MAC_PARAM_STATUS((pEvent->api.mac.scanReq.scanType != MAC_SCAN_ORPHAN) ||
                   (macMain.featureMask & MAC_FEAT_DEVICE_MASK),
                   macScan.scanCnf.hdr.status);
#ifdef FEATURE_ENHANCED_BEACON
  MAC_PARAM_STATUS( (FALSE == pEvent->api.mac.scanReq.MPMScan) ||
                    ((TRUE == pEvent->api.mac.scanReq.MPMScan) &&
                     ((MAC_MPM_SCAN_BPAN == pEvent->api.mac.scanReq.MPMScanType) ||
                      (MAC_MPM_SCAN_NBPAN == pEvent->api.mac.scanReq.MPMScanType))),
                    macScan.scanCnf.hdr.status );

  MAC_PARAM_STATUS( (FALSE == pEvent->api.mac.scanReq.MPMScan) ||
                    ((TRUE == pEvent->api.mac.scanReq.MPMScan) &&
                     ((MAC_MPM_SCAN_BPAN == pEvent->api.mac.scanReq.MPMScanType) &&
                      (pEvent->api.mac.scanReq.MPMScanDuration <= 14))||
                      ((MAC_MPM_SCAN_NBPAN == pEvent->api.mac.scanReq.MPMScanType) &&
                       (pEvent->api.mac.scanReq.MPMScanDuration > 0) &&
                       (pEvent->api.mac.scanReq.MPMScanDuration <= 16383))),
                    macScan.scanCnf.hdr.status );

  MAC_PARAM_STATUS( (MAC_MPM_SCAN_BPAN != pEvent->api.mac.scanReq.MPMScanType) ||
                    (MAC_MPM_SCAN_BPAN == pEvent->api.mac.scanReq.MPMScanType) &&
                    (pEvent->api.mac.scanReq.scanType == MAC_SCAN_PASSIVE) &&
                    (TRUE == pEvent->api.mac.scanReq.MPMScan),
                    macScan.scanCnf.hdr.status);
#else
 MAC_PARAM_STATUS( (FALSE == pEvent->api.mac.scanReq.MPMScan),
                   macScan.scanCnf.hdr.status);
#endif /* FEATURE_ENHANCED_BEACON */

  /* verify channel page */
#ifdef COMBO_MAC
  if (rfFreq == MAC_RF_FREQ_SUBG)
  {
      MAC_PARAM_STATUS((pEvent->api.mac.scanReq.channelPage >= MAC_CHANNEL_PAGE_9) &&
                       (pEvent->api.mac.scanReq.channelPage <= MAC_CHANNEL_PAGE_10),
                       macScan.scanCnf.hdr.status);

      /* verify phy ID */
      MAC_PARAM_STATUS(((pEvent->api.mac.scanReq.phyID >= MAC_MRFSK_STD_PHY_ID_BEGIN) &&
                       (pEvent->api.mac.scanReq.phyID <= MAC_MRFSK_STD_PHY_ID_END)) ||
                       ((pEvent->api.mac.scanReq.phyID >= MAC_MRFSK_GENERIC_PHY_ID_BEGIN) &&
                       (pEvent->api.mac.scanReq.phyID <= MAC_MRFSK_GENERIC_PHY_ID_END)),
                       macScan.scanCnf.hdr.status);
  }
#else
#ifndef FREQ_2_4G
  MAC_PARAM_STATUS((pEvent->api.mac.scanReq.channelPage >= MAC_CHANNEL_PAGE_9) &&
                   (pEvent->api.mac.scanReq.channelPage <= MAC_CHANNEL_PAGE_10),
                   macScan.scanCnf.hdr.status);

  /* verify phy ID */
  MAC_PARAM_STATUS(((pEvent->api.mac.scanReq.phyID >= MAC_MRFSK_STD_PHY_ID_BEGIN) &&
                   (pEvent->api.mac.scanReq.phyID <= MAC_MRFSK_STD_PHY_ID_END)) ||
                   ((pEvent->api.mac.scanReq.phyID >= MAC_MRFSK_GENERIC_PHY_ID_BEGIN) &&
                   (pEvent->api.mac.scanReq.phyID <= MAC_MRFSK_GENERIC_PHY_ID_END)),
                   macScan.scanCnf.hdr.status);
#endif
#endif

  /* store and initialize scan parameters */
  MAP_macScanCnfInit(&macScan.scanCnf, pEvent);
  MAP_osal_memcpy(macScan.scanChannels, pEvent->api.mac.scanReq.scanChannels,
         MAC_154G_CHANNEL_BITMAP_SIZ );

  /* if it is MPM Scan use the value for scan duration from MPMScanDuration */
  if (pEvent->api.mac.scanReq.MPMScan)
  {
    macScan.MPMScanType = pEvent->api.mac.scanReq.MPMScanType;
    macScan.scanDuration = pEvent->api.mac.scanReq.MPMScanDuration;
  }
  else
  {
    /* Initialize MPM Scan type to 0, indicating not a MPM scan */
    macScan.MPMScanType = 0;
    macScan.scanDuration = pEvent->api.mac.scanReq.scanDuration;
  }

  macScan.maxResults = pEvent->api.mac.scanReq.maxResults;
  macScan.permitJoining = pEvent->api.mac.scanReq.permitJoining;
  macScan.linkQuality = pEvent->api.mac.scanReq.linkQuality;
  macScan.percentFilter = pEvent->api.mac.scanReq.percentFilter;
  macScan.currentChannel = MAC_SCAN_CHAN_INIT;
  macScan.origChannel = pMacPib->logicalChannel;
  macScan.origChPage = pMacPib->channelPage;
  macScan.origPhyId = pMacPib->curPhyID;
  macScan.scanTimer.pFunc = MAP_macSetEvent;
  macScan.scanTimer.parameter = MAC_SCAN_TASK_EVT;

  /* copy scan request security settings */
  MAP_osal_memcpy(&macScan.sec, &pEvent->api.mac.scanReq.sec, sizeof(ApiMac_sec_t));

  if ((macScan.scanCnf.hdr.status == MAC_SUCCESS) && (pEvent->hdr.status == MAC_SUCCESS))
  {
    /* initialize the phy ID to the requested phy ID */
    uint8 phyId = pEvent->api.mac.scanReq.phyID;

    /* initialize status */
    if (macScan.scanCnf.scanType != MAC_SCAN_ED)
    {
      macScan.scanCnf.hdr.status = MAC_NO_BEACON;
    }

    /* Set the requested channel Page */
    MAP_MAC_MlmeSetReq(MAC_CHANNEL_PAGE, &(pEvent->api.mac.scanReq.channelPage));

    /* For MPM Scan, set the PHY corresponding to the CSM mode */
    if ( macScan.MPMScanType )
    {
      phyId = MAP_macGetCSMPhy(pEvent->api.mac.scanReq.phyID);
    }

    if ( phyId != pMacPib->curPhyID )
    {
      /* Set the requested/csm Phy ID */
      MAP_MAC_MlmeSetReq(MAC_PHY_CURRENT_DESCRIPTOR_ID, &phyId);
    }

    /* initialize radio for scanning */
    MAP_macRadioStartScan(macScan.scanCnf.scanType);

    pEvent->hdr.event = MAC_TIM_SCAN_EVT;

    /* indicates RX needs to be kept on */
    MAP_macRxSoftEnable(MAC_RX_SCAN);
  }
  else
  {
    pEvent->hdr.event = MAC_INT_SCAN_COMPLETE_EVT;
  }

  MAP_macExecute(pEvent);
}

#if !defined(TIMAC_ROM_PATCH)
/**************************************************************************************************
 * @fn          macScanNextChan
 *
 * @brief       This function sets up for the next channel to be scanned.  It determines the next
 *              channel based on the current channel and the requested scan channels.  If this is
 *              an energy detect or passive scan, it calls macScanStartTimer() to start the scan
 *              timer immediately.  If this is an active or orphan scan, it calls
 *              macBuildCommonReq() to build and send a beacon request command frame or orphan
 *              notification command frame. If there are no more channels to be scanned (or) if
 *              there are any security failures update the scanCnf status, then the function
 *              calls macExecute() to send a MAC_INT_SCAN_COMPLETE_EVT to the MAC.
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
MAC_INTERNAL_API void macScanNextChan(macEvent_t *pEvent)
{
  uint8   cmd;
  sAddr_t dstAddr;
  uint8   srcAddrMode;
#ifdef FEATURE_ENHANCED_BEACON
  macTx_t *pMsg;
  uint8   requestField = 0;
#endif /* FEATURE_ENHANCED_BEACON */
  uint8   status;
  uint8   tempScanMap;

  DBG_PRINT0(DBGSYS, "macScaNextChan");

  /* if energy detect scan store value from previous channel scanned */
  if ((macScan.scanCnf.scanType == MAC_SCAN_ED) &&
      (macScan.currentChannel != MAC_SCAN_CHAN_INIT))
  {
    /* we used maxResults element to temporarily store value */
    macScan.scanCnf.result.pEnergyDetect[macScan.currentChannel - MAC_CHAN_LOWEST]
      = MAP_macRadioEnergyDetectStop();
  }

  /* initialize current channel */
  if (macScan.currentChannel == MAC_SCAN_CHAN_INIT)
  {
    macScan.currentChannel = MAC_CHAN_LOWEST;
  }
  else
  {
    macScan.currentChannel++;
  }

  /* get next channel */
  do
  {
    tempScanMap = macScan.scanChannels[(macScan.currentChannel - MAC_CHAN_LOWEST) / 8];
    macScan.scanChannels[(macScan.currentChannel - MAC_CHAN_LOWEST) / 8] >>= 1;
    if ((tempScanMap & 0x01) == 0x01)
    {
      break;
    }
  } while (++(macScan.currentChannel) < MAC_SCAN_CHAN_END);

  DBG_PRINT1(DBGSYS, "Scan CurrCh= 0x%X", macScan.currentChannel);

  /* if not done */
  if (macScan.currentChannel < MAC_SCAN_CHAN_END)
  {
    /* set channel */
    MAP_MAC_MlmeSetReq(MAC_LOGICAL_CHANNEL, &macScan.currentChannel);

    /* destAddr for orphan, active scan, and enhanced active scan */
    dstAddr.addrMode = SADDR_MODE_SHORT;
    dstAddr.addr.shortAddr = MAC_SHORT_ADDR_BROADCAST;
    if ((macScan.scanCnf.scanType == MAC_SCAN_ED) ||
        (macScan.scanCnf.scanType == MAC_SCAN_PASSIVE))
    {
      /* if energy detect or passive scan start the scan immediately */
      MAP_macScanStartTimer(pEvent);
    }
#ifdef FEATURE_ENHANCED_BEACON
    else if ( (MAC_SCAN_ACTIVE == macScan.scanCnf.scanType ) && (MAC_MPM_SCAN_NBPAN == macScan.MPMScanType) )
    {
      /* allocate buffer */
      if ( (pMsg = MAP_macAllocTxBuffer(MAC_INTERNAL_COEXIST_EB_REQ, &macScan.sec)) != NULL )
      {
        /* build and enqueue Coexist enhanced beacon request */
        status = macBuildCoexistEBeaconReq(pMsg, &dstAddr, &macScan.sec);
        /* If the outgoing frame has security errors, then pass it up
         * to the upperlayer by first calling macScanComplete, which
         * in turn calls MAC_MLME_SCAN_CNF
         */
        if((status == MAC_COUNTER_ERROR) ||
           (status == MAC_FRAME_TOO_LONG) ||
           (status == MAC_UNAVAILABLE_KEY) ||
           (status == MAC_UNSUPPORTED_SECURITY) ||
           (status == MAC_INVALID_PARAMETER))
        {
          macScan.scanCnf.hdr.status = status;
          pEvent->hdr.event = MAC_INT_SCAN_COMPLETE_EVT;
          MAP_macExecute(pEvent);
        }
      }
    }
    else if (macScan.scanCnf.scanType == MAC_SCAN_ACTIVE_ENHANCED)
    {
      /* allocate buffer */
      if ((pMsg = MAP_macAllocTxBuffer(MAC_INTERNAL_ENHANCED_BEACON_REQ, &macScan.sec)) != NULL)
      {
        requestField  = ((macScan.permitJoining == TRUE) << 0);
        requestField |= ((macScan.linkQuality   > 0)     << 1);
        requestField |= ((macScan.percentFilter > 0)     << 2);
        status = macBuildEnhanceBeaconReq(pMsg, &dstAddr, &macScan.sec, requestField,
                                              macScan.linkQuality, macScan.percentFilter);

        /* If the outgoing frame has security errors, then pass it up
         * to the upperlayer by first calling macScanComplete, which
         * in turn calls MAC_MLME_SCAN_CNF
         */
        if((status == MAC_COUNTER_ERROR) ||
           (status == MAC_FRAME_TOO_LONG) ||
           (status == MAC_UNAVAILABLE_KEY) ||
           (status == MAC_UNSUPPORTED_SECURITY) ||
           (status == MAC_INVALID_PARAMETER))
        {
          macScan.scanCnf.hdr.status = status;
          pEvent->hdr.event = MAC_INT_SCAN_COMPLETE_EVT;
          MAP_macExecute(pEvent);
        }
      }
    }
#endif /* FEATURE_ENHANCED_BEACON */
    else
    {
      /* if orphan or active scan build an orphan notif. or beacon req */
      if (macScan.scanCnf.scanType == MAC_SCAN_ACTIVE)
      {
        cmd = MAC_INTERNAL_BEACON_REQ;
        srcAddrMode = SADDR_MODE_NONE | MAC_SRC_PAN_ID_BROADCAST;
      }
      else
      {
        cmd = MAC_INTERNAL_ORPHAN_NOTIF;
        srcAddrMode = SADDR_MODE_EXT | MAC_SRC_PAN_ID_BROADCAST;
      }

      status = MAP_macBuildCommonReq(cmd, srcAddrMode, &dstAddr, MAC_PAN_ID_BROADCAST, 0, &macScan.sec);
      /* If the outgoing frame has security errors, then pass it up to the
       * upperlayer by first calling macScanComplete, which in turn calls
       * MAC_MLME_SCAN_CNF
       */
      if((status == MAC_COUNTER_ERROR) ||
         (status == MAC_FRAME_TOO_LONG) ||
         (status == MAC_UNAVAILABLE_KEY) ||
         (status == MAC_UNSUPPORTED_SECURITY) ||
         (status == MAC_INVALID_PARAMETER))
      {
          macScan.scanCnf.hdr.status = status;
          pEvent->hdr.event = MAC_INT_SCAN_COMPLETE_EVT;
          MAP_macExecute(pEvent);
      }
    }
  }
  else
  {
    /* reached last channel, scan is complete */
    pEvent->hdr.event = MAC_INT_SCAN_COMPLETE_EVT;
    MAP_macExecute(pEvent);
  }
}
#endif

/**************************************************************************************************
 * @fn          macScanStartTimer
 *
 * @brief       This function marks the current channel as scanned and sets up the scan timer
 *              callback.
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
MAC_INTERNAL_API void macScanStartTimer(macEvent_t *pEvent)
{
  uint32  timeout;

  (void)pEvent;  // Intentionally unreferenced parameter

  /* mark current channel as scanned */
  macScan.scanCnf.unscannedChannels[(macScan.currentChannel - MAC_CHAN_LOWEST) / 8]
    &= ~((uint8) 1 << ((macScan.currentChannel - MAC_CHAN_LOWEST) % 8));

  /* set timeout; different for orphan and other scans */
  if (macScan.scanCnf.scanType == MAC_SCAN_ORPHAN)
  {
    timeout = (uint32) pMacPib->responseWaitTime * MAC_A_BASE_SUPERFRAME_DURATION;
  }
  else
  {
    if ( MAC_MPM_SCAN_BPAN == macScan.MPMScanType )
    {
      timeout = ((uint32)1 << macScan.scanDuration) * MAC_A_BASE_SUPERFRAME_DURATION;
    }
    else if ( MAC_MPM_SCAN_NBPAN == macScan.MPMScanType )
    {
      timeout = macScan.scanDuration * MAC_A_BASE_SLOT_DURATION;
    }
    else
    {
      timeout = (((uint32) 1 << macScan.scanDuration) + 1) * MAC_A_BASE_SUPERFRAME_DURATION;
    }
  }

  MAP_macTimer(&macScan.scanTimer, timeout);
}

/**************************************************************************************************
 * @fn          macScanRxBeacon
 *
 * @brief       This function performs beacon processing during an active or passive scan
 *              procedure.  If macScan.maxResults is nonzero and there is room in the result
 *              list to store the result, store the PAN descriptor in the scan result list.  If
 *              no more results will fit in the result list, macExecute() is called to send a
 *              MAC_INT_SCAN_COMPLETE to the MAC.  If pMacPib->autoRequest is FALSE or if there is
 *              a beacon payload, function macBeaconNotifyInd() is called to send
 *              MAC_BEACON_NOTIFY_IND to the application.
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
MAC_INTERNAL_API void macScanRxBeacon(macEvent_t *pEvent)
{
  macMlmeBeaconNotifyInd_t  notifyInd;
  macPanDesc_t              panDesc;
  macPanDesc_t              *pPanDesc;
  uint8                     i;

  DBG_PRINT0(DBGSYS, "Inside macScanRxBeacon");

  if ((macScan.scanCnf.scanType == MAC_SCAN_PASSIVE) ||
      (macScan.scanCnf.scanType == MAC_SCAN_ACTIVE)  ||
      (macScan.scanCnf.scanType == MAC_SCAN_ACTIVE_ENHANCED))
  {
#ifdef FEATURE_ENHANCED_BEACON
    /* Enhanced beacons are of frame version 2 */
    if ( macScan.MPMScanType &&
         (((pEvent->rx.internal.fcf & MAC_FCF_FRAME_VERSION_MASK) >> MAC_FCF_FRAME_VERSION_POS) == MAC_FRAME_VERSION_2) )
    {
      if ( pEvent->rx.payloadIePresent )
      {
        /* initialize */
        MAP_osal_memset( &notifyInd, 0x0, sizeof(macMlmeBeaconNotifyInd_t) );
        notifyInd.beaconType = MAC_BEACON_ENHANCED;

        MAP_osal_memset( &panDesc, 0x0, sizeof(macPanDesc_t) );
        notifyInd.info.eBeaconData.pPanDesc = &panDesc;

        /* build a beacon notify indication from beacon frame */
        if ( MAC_SUCCESS == macBuildEBeaconNotifyInd(&notifyInd, pEvent))
        {
          /**
           * send beacon notify only if we are able to
           * build one with coexistence IE data.
           */
          macScan.scanCnf.hdr.status = MAC_SUCCESS;

          DBG_PRINT0(DBGSYS, "Sending ebeacon notify");
          notifyInd.hdr.event = MAC_MLME_BEACON_NOTIFY_IND;
          MAP_MAC_CbackEvent((macCbackEvent_t *) &notifyInd);
        }
      }
    }
    else
#endif /* FEATURE_ENHANCED_BEACON */
    if ( !macScan.MPMScanType ) /* send normal beacon notify only for normal scan */
    {
      /* since we got a beacon, so far success */
      macScan.scanCnf.hdr.status = MAC_SUCCESS;

      /* initialize the data */
      MAP_osal_memset( &notifyInd, 0x0, sizeof(macMlmeBeaconNotifyInd_t) );

      /* build a beacon notify indication from beacon frame */
      notifyInd.info.beaconData.pPanDesc = &panDesc;
      MAP_macBuildBeaconNotifyInd(&notifyInd, pEvent);

      /* if we are storing results and there is room */
      if (macScan.scanCnf.resultListSize < macScan.maxResults)
      {
        /* Assume unique PAN */
        bool uniquePan = TRUE;

        /* Check if it is unique against the list. If not, set to FALSE and break */
        pPanDesc = macScan.scanCnf.result.pPanDescriptor;
        for (i = macScan.scanCnf.resultListSize; i > 0; i--, pPanDesc++)
        {
          if ((pPanDesc->coordPanId == panDesc.coordPanId) &&
              MAP_sAddrCmp(&pPanDesc->coordAddress, &panDesc.coordAddress))
          {
            uniquePan = FALSE;
            break;
          }
        }

        /* if result is unique, add it */
        if (uniquePan)
        {
          /* store pan descriptor in scan results list */
          MAP_osal_memcpy(&macScan.scanCnf.result.pPanDescriptor[macScan.scanCnf.resultListSize],
                      &panDesc, sizeof(macPanDesc_t));
          macScan.scanCnf.resultListSize++;
        }
      }

      /* send beacon notify ind */
      if (pMacPib->autoRequest == FALSE || notifyInd.info.beaconData.sduLength > 0 || macScan.maxResults == 0)
      {
        MAP_MAC_CbackEvent((macCbackEvent_t *) &notifyInd);
      }
        /* if results list is full scan is complete */
      if ((macScan.maxResults != 0) &&
          (macScan.scanCnf.resultListSize == macScan.maxResults))
      {
        pEvent->hdr.event = MAC_INT_SCAN_COMPLETE_EVT;
        MAP_macExecute(pEvent);
      }
    }
  }
  else
  {
    /* Valid values for ScanType are passive, active or enhanced. If
     * scantype is invalid, then set status to MAC_INVALID_PARAMETER
     */
    macScan.scanCnf.hdr.status = MAC_INVALID_PARAMETER;
    pEvent->hdr.event = MAC_INT_SCAN_COMPLETE_EVT;
    MAP_macExecute(pEvent);
  }
}
#if !defined(TIMAC_ROM_PATCH)
/**************************************************************************************************
 * @fn          macScanRxCoordRealign
 *
 * @brief       This function handles a coordinator realignment received during an orphan scan.
 *              If the addressing mode of the frame is correct, it calls macProcessCoordRealign()
 *              to finish processing of the coordinator realignment.  The status value in
 *              macScan is set to MAC_SUCCESS, then macExecute() is called to send a
 *              MAC_INT_SCAN_COMPLETE to the MAC.
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
MAC_INTERNAL_API void macScanRxCoordRealign(macEvent_t *pEvent)
{
  uint16  dat;
  uint8   *p;


  /* verify addressing mode is correct for orphan scan */
  if ((pEvent->rx.mac.srcAddr.addrMode != SADDR_MODE_EXT) ||
      (pEvent->rx.mac.dstAddr.addrMode != SADDR_MODE_EXT))
  {
    /* Addressing mode needs to be correct for orphan.
     * If not, set the status to MAC_INVALID_PARAM
     */
     macScan.scanCnf.hdr.status = MAC_INVALID_PARAMETER;
  }
  /* verify that stored extended address
   * from previous parent matches coordinator alignment attempt
   */
  else if (memcmp(pEvent->rx.mac.srcAddr.addr.extAddr,
             macPib.coordExtendedAddress.addr.extAddr, SADDR_EXT_LEN) != 0)
  {
      macScan.scanCnf.hdr.status = MAC_NO_BEACON;
  }
  /* addressing mode is correct for orphan scan and stored extended address
   * from previous parent matches coordinator alignment attempt
   */
  else {
    /* set coordinator extended address */
    MAP_MAC_MlmeSetReq(MAC_COORD_EXTENDED_ADDRESS, &pEvent->rx.mac.srcAddr);

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif

    /* set security PIB */
    MAP_MAC_MlmeSetSecurityReq(MAC_PAN_COORD_EXTENDED_ADDRESS, &pEvent->rx.mac.srcAddr);
#if (defined TIMAC_ROM_IMAGE_BUILD)
    }
#endif

#endif

    p = pEvent->rx.msdu.p;

    /* build and set the PAN ID */
    dat = BUILD_UINT16(p[0], p[1]);
    p += 2;
    MAP_MAC_MlmeSetReq(MAC_PAN_ID, &dat);

    /* build and set coordinator short address */
    dat = BUILD_UINT16(p[0], p[1]);
    p += 2;
    MAP_MAC_MlmeSetReq(MAC_COORD_SHORT_ADDRESS, &dat);

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif
    /* set security PIB */
    MAP_MAC_MlmeSetSecurityReq(MAC_PAN_COORD_SHORT_ADDRESS, &dat);
#if (defined TIMAC_ROM_IMAGE_BUILD)
    }
#endif
#endif

    /* set the channel */
    MAP_MAC_MlmeSetReq(MAC_LOGICAL_CHANNEL, p);
    p++;

    /* set our short address */
    dat = BUILD_UINT16(p[0], p[1]);
    MAP_MAC_MlmeSetReq(MAC_SHORT_ADDRESS, &dat);

    /* ignore channel page for now */

    /* orphan scan successful */
    macScan.scanCnf.hdr.status = MAC_SUCCESS;
  }

  /* scan complete */
  pEvent->hdr.event = MAC_INT_SCAN_COMPLETE_EVT;
  MAP_macExecute(pEvent);
}
#endif

/**************************************************************************************************
 * @fn          macScanComplete
 *
 * @brief       This function handles the completion of the scan procedure.  First the mac Timer is
 *              cancelled if it already running. The Radio scan is stoppedBeacon and RX disabled.
 *              The transmission is resumed by calling macExecute() to send a
 *              MAC_INT_BEACON_RESUME_EVT to the MAC.  Then macRadioStopScan() is called to restore
 *              the receiver settings after scan. If this is not a successful orphan scan then the
 *              channel is set to the original channel before the scan.  Finally, the MAC callback
 *              function is executed with event MAC_MLME_SCAN_CNF.
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
MAC_INTERNAL_API void macScanComplete(macEvent_t *pEvent)
{
  (void)pEvent;  // Intentionally unreferenced parameter

  DBG_PRINT0(DBGSYS, "Inside macScanComplete");

  MAP_macTimerCancel(&macScan.scanTimer);
  MAP_macRadioStopScan();

  /* turn off receiver by clearing all scan flags */
  MAP_macRxSoftDisable(MAC_RX_SCAN);

  /* set channel, channel page and phy id back
     if not orphan or if orphan failed */
  if (!((macScan.scanCnf.scanType == MAC_SCAN_ORPHAN) &&
          (macScan.scanCnf.hdr.status == MAC_SUCCESS)))
  {
    MAP_MAC_MlmeSetReq(MAC_LOGICAL_CHANNEL, &macScan.origChannel);
    MAP_MAC_MlmeSetReq(MAC_CHANNEL_PAGE, &macScan.origChPage);

    if ( macScan.origPhyId != pMacPib->curPhyID )
    {
      MAP_MAC_MlmeSetReq(MAC_PHY_CURRENT_DESCRIPTOR_ID, &macScan.origPhyId);
    }
  }

  MAP_MAC_CbackEvent((macCbackEvent_t *) &macScan.scanCnf);
}

/**************************************************************************************************
 * @fn          macScanFailedInProgress
 *
 * @brief       This function calls the MAC callback with status MAC_SCAN_IN_PROGRESS.
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
MAC_INTERNAL_API void macScanFailedInProgress(macEvent_t *pEvent)
{
  macMlmeScanCnf_t  scanCnf;

  DBG_PRINT0(DBGSYS, "Inside macScanFailedInProgress");

  MAP_macScanCnfInit(&scanCnf, pEvent);
  scanCnf.hdr.status = MAC_SCAN_IN_PROGRESS;
  MAP_MAC_CbackEvent((macCbackEvent_t *) &scanCnf);
}

/**************************************************************************************************
 * @fn          MAC_MlmeScanReq
 *
 * @brief       This function calls macSendMsg() to send an OSAL message containing a
 *              MAC_API_SCAN_REQ_EVT to the MAC.
 *
 * input parameters
 *
 * @param       pData - Pointer to parameters structure.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
ApiMac_status_t MAC_MlmeScanReq(ApiMac_mlmeScanReq_t *pData)
{
  ApiMac_status_t status = ApiMac_status_success;
  macMlmeScanReq_t scanReq;
  uint8* scanResults;

  if (MAP_macStateScanning())
  {
    status = ApiMac_status_scanInProgress;
    return status;
  }

  MAP_osal_memcpy(scanReq.scanChannels, pData->scanChannels,
         (MAC_154G_CHANNEL_BITMAP_SIZ));

  scanReq.scanType = (uint8)pData->scanType;
  scanReq.scanDuration = (uint8)pData->scanDuration;
  scanReq.channelPage = (uint8)pData->channelPage;
  scanReq.phyID = (uint8)pData->phyID;
  scanReq.maxResults = (uint8)pData->maxResults;
  scanReq.permitJoining = (bool)pData->permitJoining;
  scanReq.linkQuality = (uint8)pData->linkQuality;
  scanReq.percentFilter = (uint8)pData->percentFilter;

  scanReq.MPMScan = pData->MPMScan;
  scanReq.MPMScanType = (uint8)pData->MPMScanType;
  scanReq.MPMScanDuration = (uint16)pData->MPMScanDuration;

  MAP_osal_memcpy(&scanReq.sec, &pData->sec, sizeof(ApiMac_sec_t));

  if(pData->scanType == ApiMac_scantype_energyDetect)
  {
    scanResults = (void *)MAP_osal_mem_alloc(APIMAC_154G_MAX_NUM_CHANNEL);
    scanReq.result.pEnergyDetect = (uint8 *)scanResults;
    if (scanResults)
    {
      /* default to high value */
      MAP_osal_memset(scanResults, 0xFF, APIMAC_154G_MAX_NUM_CHANNEL);
    }
    else
    {
        status = ApiMac_status_noResources;
    }
  }
  else
  {
    scanReq.result.pPanDescriptor = NULL;
    if(scanReq.maxResults)
    {
      scanResults = (void *)MAP_osal_mem_alloc(
                    sizeof(macPanDesc_t) * scanReq.maxResults);
      scanReq.result.pPanDescriptor = (macPanDesc_t *)scanResults;
      if(scanResults == NULL)
      {
          status = ApiMac_status_noResources;
      }
    }
  }

  if(status == ApiMac_status_success)
  {
    MAP_macSendMsg(MAC_API_SCAN_REQ_EVT, &scanReq);
  }

  return status;
}
#endif /* (FEATURE_BEACON_MODE) || (FEATURE_NON_BEACON_MODE) */
