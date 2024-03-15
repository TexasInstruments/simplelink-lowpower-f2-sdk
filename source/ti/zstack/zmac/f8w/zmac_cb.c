/******************************************************************************

 @file  zmac_cb.c

 @brief This file contains the NWK functions that the ZMAC calls

 Group: CMCU, LPRF
 Target Device: CC2652

 ******************************************************************************

 Copyright (c) 2005-2017, Texas Instruments Incorporated

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
 Release Name: simplelink_zigbee_sdk_plugin_0_95_00_18_s
 Release Date: 2017-11-10 13:43:40
 *****************************************************************************/

/********************************************************************************************************
 *                                               INCLUDES
 ********************************************************************************************************/

#include "zcomdef.h"
#include "rom_jt_154.h"
#include "zmac.h"
#include "mt_mac.h"
#include "hal_mcu.h"

#include "nwk.h"
#include "nwk_bufs.h"
#include "zglobals.h"

#include "mac_security.h"

#include "mac_main.h"

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
#include "cgp_stub.h"
#endif

/********************************************************************************************************
 *                                               CONSTANTS
 ********************************************************************************************************/

/* Lookup table for size of structures. Must match with the order of MAC callback events */
const uint8_t CODE zmacCBSizeTable [] = {
  0,
  sizeof(ZMacAssociateInd_t),       // MAC_MLME_ASSOCIATE_IND      1   Associate indication
  sizeof(ZMacAssociateCnf_t),       // MAC_MLME_ASSOCIATE_CNF      2   Associate confirm
  0,                                // MAC_MLME_DISASSOCIATE_IND   3   Disassociate indication
  0,                                // MAC_MLME_DISASSOCIATE_CNF   4   Disassociate confirm
  sizeof(macMlmeBeaconNotifyInd_t), // MAC_MLME_BEACON_NOTIFY_IND  5   con notify indication
  sizeof(ZMacOrphanInd_t),          // MAC_MLME_ORPHAN_IND         6   Orphan indication
  sizeof(ZMacScanCnf_t),            // MAC_MLME_SCAN_CNF           7   Scan confirm
  sizeof(ZMacStartCnf_t),           // MAC_MLME_START_CNF          8   Start confirm
  0,                                // MAC_MLME_SYNC_LOSS_IND      9   Sync loss indication
  sizeof(ZMacPollCnf_t),            // MAC_MLME_POLL_CNF           10  Poll confirm
  sizeof(ZMacCommStatusInd_t),      // MAC_MLME_COMM_STATUS_IND    11  Comm status indication
  sizeof(ZMacDataCnf_t),            // MAC_MCPS_DATA_CNF           12  Data confirm
  sizeof(macMcpsDataInd_t),         // MAC_MCPS_DATA_IND           13  Data indication
  0,                                // MAC_MCPS_PURGE_CNF          14  Purge confirm
  0,                                // MAC_PWR_ON_CNF              15  Power on confirm
  sizeof(ZMacPollInd_t),            // MAC_MLME_POLL_IND           16  Poll indication
  0,                                // MAC_MLME_WS_ASYNC_CNF       17  WiSUN Async frame confirm
  0                                 // MAC_MLME_WS_ASYNC_IND       18  WiSUN Async frame indication
};

/********************************************************************************************************
 *                                               LOCALS
 ********************************************************************************************************/

/* LQI Adjustment Mode */
static ZMacLqiAdjust_t lqiAdjMode = LQI_ADJ_OFF;

/* LQI Adjustment Function */
static void ZMacLqiAdjust( uint8_t corr, uint8_t* lqi );

/*********************************************************************
 * ZMAC Function Pointers
 */

/*
 * ZMac Application callback function. This function will be called
 * for every MAC message that is received over-the-air or generated
 * locally by MAC for the application.
 *
 * The callback function should return TRUE if it has handled the
 * MAC message and no further action should be taken with it. It
 * should return FALSE if it has not handled the MAC message and
 * normal processing should take place.
 *
 * NOTE: The processing in this function should be kept to the
 *       minimum.
 */
uint8_t (*pZMac_AppCallback)( uint8_t *msgPtr ) = (void*)NULL;

/*********************************************************************
 * ZMAC Functions
 */

/**************************************************************************************************
 * @fn       MAC_CbackEvent()
 *
 * @brief    convert MAC data confirm and indication to ZMac and send to NWK
 *
 * @param    pData - pointer to macCbackEvent_t
 *
 * @return   none
 *************************************************************************************************/
void MAC_CbackEvent(macCbackEvent_t *pData)
{
  uint8_t event = pData->hdr.event;
  uint16_t tmp = zmacCBSizeTable[event];
  macCbackEvent_t *msgPtr;
  bool gpDataCnf = false;

  /* If the Network layer will handle a new MAC callback, a non-zero value must be entered in the
   * corresponding location in the zmacCBSizeTable[] - thus the table acts as "should handle"?
   */
  if (tmp == 0)
  {
    return;
  }

  // MAC_MCPS_DATA_IND is very special - it is the only event where the MAC does not free *pData.
  if ( event == MAC_MCPS_DATA_IND )
  {
#if defined ( ZMAC_MAX_DATA_IND ) && ( ZMAC_MAX_DATA_IND >= 1 )
    // This feature limits the number of unprocessed MAC Data Indications that can be queued
    // into the Network Task's OSAL message queue. To enable ZMAC_MAX_DATA_IND filtering, the
    // command-line option specifies the threshold setting where MAC_MCPS_DATD_IND callback
    // messages will be dropped. For example, adding ZMAC_MAX_DATA_IND=5 to the commnad-line
    // options allows up to five messages to be passed up and queued in the NWK layer buffer.
    uint8_t diCount = osal_msg_count( NWK_TaskID, MAC_MCPS_DATA_IND );
#endif // ZMAC_MAX_DATA_IND
    MAP_MAC_MlmeGetReq( MAC_SHORT_ADDRESS, &tmp );
    if ( (tmp == INVALID_NODE_ADDR) ||
         (tmp == NWK_BROADCAST_SHORTADDR_DEVALL) ||
#if defined ( ZMAC_MAX_DATA_IND ) && ( ZMAC_MAX_DATA_IND >= 1 )
         (diCount > ZMAC_MAX_DATA_IND) ||
#endif // ZMAC_MAX_DATA_IND
         (pData->dataInd.msdu.len == 0) )
    {
      MAP_mac_msg_deallocate( (uint8_t **)&pData );
      return;
    }
    msgPtr = pData;
  }
  else
  {
    if (event == MAC_MLME_BEACON_NOTIFY_IND )
    {
      if (pData->beaconNotifyInd.beaconType == MAC_BEACON_ENHANCED)
      {
          /* Drop the message */
          return;
      }
      tmp += sizeof(macPanDesc_t) + pData->beaconNotifyInd.info.beaconData.sduLength;
    }
    else if (event == MAC_MLME_SCAN_CNF)
    {
      if (pData->scanCnf.scanType == ZMAC_ED_SCAN)
      {
        tmp += ZMAC_ED_SCAN_MAXCHANNELS;
      }
      else
      {
        tmp += sizeof( ZMacPanDesc_t ) * pData->scanCnf.resultListSize;
      }
    }
    else if (event == MAC_MCPS_DATA_CNF)
    {
      if (pData->dataCnf.pDataReq->internal.txOptions & MAC_TXOPTION_GREEN_PWR)
      {
          gpDataCnf = true;
      }
    }

    if ( !(msgPtr = (macCbackEvent_t *)OsalPort_msgAllocate(tmp)) )
    {
      // Not enough memory. If data confirm - try again
      if ((event == MAC_MCPS_DATA_CNF) && (pData->dataCnf.pDataReq != NULL))
      {
        halIntState_t intState;

        // This is not normally deallocated here because the pZMac_AppCallback()
        // application may need it.
        HAL_ENTER_CRITICAL_SECTION( intState );  // Hold off interrupts.

        MAP_mac_msg_deallocate( (uint8_t**)&(pData->dataCnf.pDataReq) );
        if ( !(msgPtr = (macCbackEvent_t *)OsalPort_msgAllocate(tmp)) )
        {
          // Still no allocation, something is wrong
          HAL_EXIT_CRITICAL_SECTION( intState );   // Re-enable interrupts.
          return;
        }
        HAL_EXIT_CRITICAL_SECTION( intState );   // Re-enable interrupts.
      }
      else
      {
        // This message is dropped
        return;
      }
    }
    if (event != MAC_MLME_SCAN_CNF)
    {
      OsalPort_memcpy(msgPtr, pData, zmacCBSizeTable[event]);
    }
  }

  if ( event == MAC_MLME_BEACON_NOTIFY_IND )
  {
    macMlmeBeaconNotifyInd_t *pBeacon = (macMlmeBeaconNotifyInd_t*)msgPtr;

    OsalPort_memcpy(pBeacon+1, pBeacon->info.beaconData.pPanDesc, sizeof(macPanDesc_t));
    pBeacon->info.beaconData.pPanDesc = (macPanDesc_t *)(pBeacon+1);
    OsalPort_memcpy(pBeacon->info.beaconData.pPanDesc+1,
                pBeacon->info.beaconData.pSdu, pBeacon->info.beaconData.sduLength);
    pBeacon->info.beaconData.pSdu = (uint8_t *)(pBeacon->info.beaconData.pPanDesc+1);
  }
  else if (event == MAC_MLME_SCAN_CNF)
  {
    ZMacScanCnf_t *pScan = (ZMacScanCnf_t*)msgPtr;

    pScan->hdr.Event = pData->scanCnf.hdr.event;
    pScan->hdr.Status = pData->scanCnf.hdr.status;
    pScan->ScanType = pData->scanCnf.scanType;
    pScan->ChannelPage = pData->scanCnf.channelPage;
    pScan->UnscannedChannels = BUILD_UINT32(pData->scanCnf.unscannedChannels[0],
                                            pData->scanCnf.unscannedChannels[1],
                                            pData->scanCnf.unscannedChannels[2],
                                            pData->scanCnf.unscannedChannels[3]);

    if (pScan->ScanType == ZMAC_ED_SCAN)
    {
      pScan->ResultListSize = ZMAC_ED_SCAN_MAXCHANNELS;
      OsalPort_memcpy(pScan + 1, pData->scanCnf.result.pEnergyDetect, ZMAC_ED_SCAN_MAXCHANNELS);
      pScan->Result.pEnergyDetect = (uint8_t*) (pScan + 1);

      /* Free allocated memory from scanReq() */
      OsalPort_free(pData->scanCnf.result.pEnergyDetect);
      pData->scanCnf.result.pEnergyDetect = NULL;
    }
    else
    {
      pScan->ResultListSize = pData->scanCnf.resultListSize;

      if ((pScan->ResultListSize > 0) && (pData->scanCnf.result.pPanDescriptor != NULL))
      {
        OsalPort_memcpy(pScan + 1, pData->scanCnf.result.pPanDescriptor,
                    (sizeof( ZMacPanDesc_t ) * pScan->ResultListSize));
        pScan->Result.pPanDescriptor = (ZMacPanDesc_t*) (pScan + 1);
      }
      else
      {
        pScan->Result.pPanDescriptor = NULL;
        pScan->ResultListSize = 0;
      }

      if(pData->scanCnf.result.pPanDescriptor != NULL)
      {
        /* Free allocated memory from scanReq() */
        OsalPort_free(pData->scanCnf.result.pPanDescriptor);
        pData->scanCnf.result.pPanDescriptor = NULL;
        pData->scanCnf.resultListSize = 0;
      }
    }
  }

  if ( ( pZMac_AppCallback == NULL ) || ( pZMac_AppCallback( (uint8_t *)msgPtr ) == FALSE ) )
  {
    // Filter out non-zigbee packets
    if ( event == MAC_MCPS_DATA_IND )
    {
      uint8_t fcFrameType = (pData->dataInd.msdu.p[0] & 0x03);
      uint8_t fcProtoVer = ((pData->dataInd.msdu.p[0] >> 2) & 0x0F);
      uint8_t fcReserve = (pData->dataInd.msdu.p[1] & 0xC0);

      if ( (fcFrameType > 0x01) || (fcProtoVer != _NIB.nwkProtocolVersion) || (fcReserve != 0)
          || (pData->dataInd.mac.srcAddr.addrMode != SADDR_MODE_SHORT) )
      {
#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
        //Is this for GP
        if(fcProtoVer == GP_ZIGBEE_PROTOCOL_VER)
        {

          pData->hdr.event = GP_MAC_MCPS_DATA_IND;
          // Application hasn't already processed this message. Send it to NWK task.
          OsalPort_msgSend( gp_TaskID, (uint8_t *)pData );

          return;
        }
#endif
        // Drop the message
        MAP_mac_msg_deallocate( (uint8_t **)&pData );
        return;
      }
      else
      {
        macDataInd_t *pInd = &msgPtr->dataInd.mac;
        // See if LQI needs adjustment due to frame correlation
        ZMacLqiAdjust( pInd->correlation, &pInd->mpduLinkQuality );

        // Look for broadcast message that has a radius of greater 1
        if ( (pData->dataInd.mac.dstAddr.addr.shortAddr == 0xFFFF)
               && (pData->dataInd.msdu.p[6] > 1) )
        {
          // Send the messsage to a special broadcast queue
          if ( nwk_broadcastSend( (uint8_t *)msgPtr ) != SUCCESS )
          {
            // Drop the message, too many broadcast messages to process
            MAP_mac_msg_deallocate( (uint8_t **)&pData );
          }
          return;
        }
      }
    }
    else if ((event == MAC_MCPS_DATA_CNF) && (pData->hdr.status != MAC_NO_RESOURCES))
    {
      macMcpsDataCnf_t *pCnf = &msgPtr->dataCnf;

      if ( pCnf->pDataReq && (pCnf->pDataReq->internal.txOptions & MAC_TXOPTION_ACK) )
      {
        // See if LQI needs adjustment due to frame correlation
        ZMacLqiAdjust( pCnf->correlation, &pCnf->mpduLinkQuality );
      }
    }

    if(gpDataCnf == true)
    {
#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
      msgPtr->dataCnf.hdr.event = GP_MAC_MCPS_DATA_CNF;
      OsalPort_msgSend( gp_TaskID, (uint8_t *)msgPtr);
#endif
    }
    else
    {
      OsalPort_msgSend( NWK_TaskID, (uint8_t *)msgPtr );
    }
  }

  if (event == MAC_MCPS_DATA_CNF && (pData->dataCnf.pDataReq != NULL))
  {
    // If the application needs 'pDataReq' then we cannot free it here.
    // The application must free it after using it. Note that 'pDataReq'
    // is of macMcpsDataReq_t (and not ZMacDataReq_t) type.

    MAP_mac_msg_deallocate( (uint8_t**)&(pData->dataCnf.pDataReq) );
  }
}

/********************************************************************************************************
 * @fn      MAC_CbackCheckPending
 *
 * @brief   Return number of pending indirect msg
 *
 * @param   None
 *
 * @return  Number of indirect msg holding
 ********************************************************************************************************/
uint8_t MAC_CbackCheckPending(void)
{
  if ( ZSTACK_ROUTER_BUILD )
  {
    return (nwkDB_ReturnIndirectHoldingCnt());
  }
  else
  {
    return (0);
  }
}

/**************************************************************************************************
 * @fn          MAC_CbackQueryRetransmit
 *
 * @brief       This function callback function returns whether or not to continue MAC
 *              retransmission.
 *              A return value '0x00' will indicate no continuation of retry and a return value
 *              '0x01' will indicate to continue retransmission. This callback function shall be
 *              used to stop continuing retransmission for RF4CE.
 *              MAC shall call this callback function whenever it finishes transmitting a packet
 *              for macMaxFrameRetries times.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      0x00 to stop retransmission, 0x01 to continue retransmission.
 **************************************************************************************************
*/

uint8_t MAC_CbackQueryRetransmit(void)
{
  return(0);
}


/********************************************************************************************************
 * @fn      ZMacLqiAdjustMode
 *
 * @brief   Sets/return LQI adjust mode
 *
 * @param   mode - LQI_ADJ_GET = return current mode only
 *                 LQI_ADJ_OFF = disable LQI adjusts
 *                 LQI_ADJ_MODEx = set to LQI adjust MODEx
 *
 * @return  current LQI adjust mode
 ********************************************************************************************************/
ZMacLqiAdjust_t ZMacLqiAdjustMode( ZMacLqiAdjust_t mode )
{
  if ( mode != LQI_ADJ_GET )
  {
    lqiAdjMode = mode;
  }
  return ( lqiAdjMode );
}

/********************************************************************************************************
 * @fn      ZMacLqiAdjust
 *
 * @brief   Adjust LQI according to correlation value
 *
 * @notes - the IEEE 802.15.4 specification provides some general statements on
 *          the subject of LQI. Section 6.7.8: "The minimum and maximum LQI values
 *          (0x00 and 0xFF) should be associated with the lowest and highest IEEE
 *          802.15.4 signals detectable by the receiver, and LQ values should be
 *          uniformly distributed between these two limits." Section E.2.3: "The
 *          LQI (see 6.7.8) measures the received energy and/or SNR for each
 *          received packet. When energy level and SNR information are combined,
 *          they can indicate whether a corrupt packet resulted from low signal
 *          strength or from high signal strength plus interference."
 *        - LQI Adjustment Mode1 provided below is a simple algorithm to use the
 *          packet correlation value (related to SNR) to scale incoming LQI value
 *          (related to signal strength) to 'derate' noisy packets.
 *        - LQI Adjustment Mode2 provided below is a location for a developer to
 *          implement their own proprietary LQI adjustment algorithm.
 *
 * @param   corr - packet correlation value
 * @param   lqi  - ptr to link quality (scaled rssi)
 *
 * @return  *lqi - adjusted link quality
 ********************************************************************************************************/
static void ZMacLqiAdjust( uint8_t corr, uint8_t *lqi )
{
  if ( lqiAdjMode != LQI_ADJ_OFF )
  {
    uint16_t adjLqi = *lqi;

    // Keep correlation within theoretical limits
    if ( corr < LQI_CORR_MIN )
    {
       corr = LQI_CORR_MIN;
    }
    else if ( corr > LQI_CORR_MAX )
    {
       corr = LQI_CORR_MAX;
    }

    if ( lqiAdjMode == LQI_ADJ_MODE1 )
    {
      /* MODE1 - linear scaling of incoming LQI with a "correlation percentage"
                 which is computed from the incoming correlation value between
                 theorectical minimum/maximum values. This is a very simple way
                 of 'derating' the incoming LQI as correlation value drops. */
      adjLqi = (adjLqi * (corr - LQI_CORR_MIN)) / (LQI_CORR_MAX - LQI_CORR_MIN);
    }
    else if ( lqiAdjMode == LQI_ADJ_MODE2 )
    {
      /* MODE2 - location for developer to implement a proprietary algorithm */
    }

    // Replace incoming LQI with scaled value
    *lqi = (adjLqi > 255) ? 255 : (uint8_t)adjLqi;
  }
}

