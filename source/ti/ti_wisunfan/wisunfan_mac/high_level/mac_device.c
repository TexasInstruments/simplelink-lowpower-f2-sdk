/******************************************************************************

 @file  mac_device.c

 @brief This module implements MAC device management procedures for a device.

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

#include "timac_api.h"
#include "mac_low_level.h"
#include "mac_spec.h"
#include "mac_main.h"
#include "mac_timer.h"
#include "mac_mgmt.h"
#include "mac_pib.h"
#include "mac_security_pib.h"
#include "mac_data.h"
#include "mac_scan.h"
#include "mac_device.h"
#include "mac_ie_build.h"

#include "rom_jt_154.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_device_c
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
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */

void macAssociateCnf(uint8 status, uint16 addr);
void macPollCnf(uint8 status, uint8 framePending);
uint8 macBuildAssociateReq(macEvent_t *pEvent);

#if !defined(TIMAC_ROM_PATCH)
/**************************************************************************************************
 * @fn          macDeviceReset
 *
 * @brief       This function initializes the data structures for the device module.
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
MAC_INTERNAL_API void macDeviceReset(void)
{
  /* Device action set 1 */
  macDeviceAction1[0] = MAP_macApiAssociateReq;
  macDeviceAction1[1] = MAP_macApiPollReq;
  macDeviceAction1[2] = MAP_macAssocDataReq;
  macDeviceAction1[3] = MAP_macAssocDataReqComplete;
  macDeviceAction1[4] = MAP_macAssocDataRxInd;
  macDeviceAction1[5] = MAP_macAssocFailed;
  macDeviceAction1[6] = MAP_macAssocFrameResponseTimeout;
  macDeviceAction1[7] = MAP_macPollDataReqComplete;
  macDeviceAction1[8] = MAP_macPollDataRxInd;
  macDeviceAction1[9] = MAP_macPollFrameResponseTimeout;
  macDeviceAction1[10] = MAP_macPollRxAssocRsp;
  macDeviceAction1[11] = MAP_macRxAssocRsp;
  macDeviceAction1[12] = MAP_macStartFrameResponseTimer;
  macDeviceAction1[13] = MAP_macStartResponseTimer;

  /* Device action set 2 */
  macDeviceAction2[0] = MAP_macAssocRxDisassoc;
  macDeviceAction2[1] = MAP_macPollRxDisassoc;
#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)
  macDeviceAction2[2] = MAP_macScanRxCoordRealign;
#else
  macDeviceAction2[2] = NULL;
#endif /* (FEATURE_BEACON_MODE) || (FEATURE_NON_BEACON_MODE) */
  macDeviceAction2[3] = MAP_macRxCoordRealign;
  macDeviceAction2[4] = MAP_macPanConflictComplete;

  macDevice.sec.securityLevel = 0;
  macDevice.dataReqMask = 0;
  macDevice.frameTimer.pFunc = MAP_macSetEvent;
  macDevice.frameTimer.parameter = MAC_FRAME_RESPONSE_TASK_EVT;
  macDevice.responseTimer.pFunc = MAP_macSetEvent;
  macDevice.responseTimer.parameter = MAC_RESPONSE_WAIT_TASK_EVT;
}
#endif

/**************************************************************************************************
 * @fn          macAssociateCnf
 *
 * @brief       Call the MAC callback with a MAC_MLME_ASSOCIATE_CNF event.
 *
 * input parameters
 *
 * @param       status - Associate confirm status.
 * @param       addr - Associate confirm short address.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void macAssociateCnf(uint8 status, uint16 addr)
{
  macMlmeAssociateCnf_t associateCnf;

  associateCnf.hdr.event = MAC_MLME_ASSOCIATE_CNF;
  associateCnf.hdr.status = status;
  if (status != MAC_SUCCESS)
  {
    addr = MAC_SHORT_ADDR_NONE;
  }
  associateCnf.assocShortAddress = addr;

  DBG_PRINT0(DBGSYS, "macAssociateCnf");

  /* copy the purported security parameters */
  MAP_osal_memcpy(&associateCnf.sec, &macDevice.sec, sizeof(ApiMac_sec_t));

  MAP_MAC_CbackEvent((macCbackEvent_t *) &associateCnf);
}

/**************************************************************************************************
 * @fn          macPollCnf
 *
 * @brief       Call the MAC callback with a MAC_MLME_POLL_CNF event.
 *
 * input parameters
 *
 * @param       status - Poll confirm status.
 *              framePending - set if frame pending is set in data packet
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void macPollCnf(uint8 status, uint8 framePending)
{
  macMlmePollCnf_t pollCnf;

  pollCnf.hdr.event = MAC_MLME_POLL_CNF;
  pollCnf.hdr.status = status;
  pollCnf.framePending = framePending;

  MAP_MAC_CbackEvent((macCbackEvent_t *) &pollCnf);
}

/**************************************************************************************************
 * @fn          macBuildAssociateReq
 *
 * @brief       This function builds and sends a associate request command frame in buffer pEvent.
 *              The TX options and frame type are set and other parameters in the macTx_t
 *              structure are initialized.  Then macBuildHeader() is called to build the header
 *              with the address information from the API event structure.  If successful the
 *              capability information is built in the frame and macDataTxEnqueue() is called to
 *              queue the frame for transmission.
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
uint8 macBuildAssociateReq(macEvent_t *pEvent)
{
  sAddr_t  coordAddress;
  uint16   coordPanId;
  uint8    status;

  /* set parameters */
  pEvent->tx.internal.txOptions = MAC_TXOPTION_ACK;
  pEvent->tx.internal.frameType = MAC_INTERNAL_ASSOC_REQ;

  /* Prepare a local copy */
  MAP_copyApiMacAddrToMacAddr(&coordAddress, &pEvent->apiData.mac.associateReq.coordAddress);
  coordPanId = pEvent->apiData.mac.associateReq.coordPanId;

  /* buffer is set up to build frame in reverse, so build capability information into frame */
  pEvent->tx.msdu.p--;
  *(pEvent->tx.msdu.p)-- = MAP_ApiMac_convertCapabilityInfo(&(pEvent->apiData.mac.associateReq.capabilityInformation));


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

  pEvent->tx.msdu.len += MAC_ASSOC_REQ_PAYLOAD;

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
    MAP_osal_memcpy(&pEvent->tx.internal.dest.dstAddr, &pEvent->apiData.mac.associateReq.coordAddress.addr, sizeof(sAddr_t));

    if(pEvent->apiData.mac.associateReq.coordAddress.addrMode == SADDR_MODE_EXT)
    {
      MAP_sAddrExtCpy(pEvent->tx.internal.dest.destAddr, pEvent->apiData.mac.associateReq.coordAddress.addr.extAddr);
      if(pMacPib->rxOnWhenIdle)
      {
        /* include the necessary Payaload IE's */
        if ( macCmdIncludeFHIe[MAC_INTERNAL_ASSOC_REQ] & MAC_FH_PAYLOAD_IE_MASK )
        {
          pEvent->tx.internal.payloadIeInfo[0].gId = MAC_PAYLOAD_IE_WISUN_GID;
          pEvent->tx.internal.payloadIeInfo[0].subId.subIdBMap =
          macCmdIncludeFHIe[MAC_INTERNAL_ASSOC_REQ] & MAC_FH_PAYLOAD_IE_MASK;

          MAP_macBuildPayloadIes(&pEvent->tx, pEvent->tx.internal.payloadIeInfo, TRUE, FALSE);
        }

        /* include the necessary header IE's */
        if ( macCmdIncludeFHIe[MAC_INTERNAL_ASSOC_REQ] & MAC_FH_HEADER_IE_MASK )
        {
          pEvent->tx.internal.headerIeInfo[0].elementId = MAC_HEADER_IE_WISUN;
          pEvent->tx.internal.headerIeInfo[0].subIdBitMap =
          macCmdIncludeFHIe[MAC_INTERNAL_ASSOC_REQ] & MAC_FH_HEADER_IE_MASK;

          MAP_macBuildHeaderIes(&pEvent->tx, pEvent->tx.internal.headerIeInfo);
        }
      }
    }
  }
#endif
  status = MAP_macBuildHeader(&pEvent->tx, (SADDR_MODE_EXT | MAC_SRC_PAN_ID_BROADCAST),
                          &coordAddress, coordPanId, 0);
  if ( status == MAC_SUCCESS)
  {
    /* queue frame for transmission */
    MAP_macDataTxEnqueue(&pEvent->tx);
  }
  return status;
}
