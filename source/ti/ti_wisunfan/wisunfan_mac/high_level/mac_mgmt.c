/******************************************************************************

 @file  mac_mgmt.c

 @brief This module implements MAC management procedures which are common to
        both coordinator and device.

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

#include "hal_defs.h"
#include "hal_mcu.h"
#include "timac_api.h"
#include "mac_spec.h"
#include "mac_low_level.h"
#include "mac_main.h"
#include "mac_pib.h"
#include "mac_radio.h"
#include "mac_data.h"
#include "mac_pwr.h"
#include "mac_timer.h"
#include "mac_symbol_timer.h"
#include "mac.h"
#include "mac_tx.h"
#include "mac_mgmt.h"
#include "mac_scan.h"
#include "mac_security.h"
#include "mac_beacon.h"
#include "mac_beacon_device.h"
#include "mac_assert.h"
#include "mac_ie_build.h"
#include "mac_hl_patch.h"

#ifdef FEATURE_MAC_SECURITY
  #include "mac_security_pib.h"
#endif

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_mgmt_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

#include "rom_jt_154.h"


/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */
uint8 MAC_MlmeFHGetReqSize(uint16 pibAttribute);

void macMgmtReset(void);

#ifdef FEATURE_FREQ_HOP_MODE
MAC_INTERNAL_API void macApiWSAsyncReq(macEvent_t *pEvent);
MAC_INTERNAL_API void macWSAsyncCnf(uint8 status);
MAC_INTERNAL_API uint8 macBuildWSAsyncMsg(macEvent_t *pEvent);
#endif /* FEATURE_FREQ_HOP_MODE */
/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

/* check pan coordinator bit in beacon frame superframe spec */
#define MAC_SFS_PAN_COORD(p)        (p[1] & 0x40)

#if !defined(TIMAC_ROM_PATCH)
/**************************************************************************************************
 * @fn          macMgmtReset
 *
 * @brief       This function initializes the data structures for the mgmt module.
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
void macMgmtReset(void)
{
  /* Action set 1 */
  macMgmtAction1[0] = MAP_macNoAction;
  macMgmtAction1[1] = MAP_macApiBadState;
  macMgmtAction1[2] = MAP_macApiPending;
  macMgmtAction1[3] = MAP_macApiDataReq;
  macMgmtAction1[4] = MAP_macCommStatusInd;
  macMgmtAction1[5] = MAP_macDataSend;
  macMgmtAction1[6] = MAP_macDataRxInd;
#ifdef FEATURE_FREQ_HOP_MODE
  macMgmtAction1[7] = MAP_macApiWSAsyncReq;
#else
  macMgmtAction1[7] = NULL;
#endif /* FEATURE_FREQ_HOP_MODE */
  macMgmtAction1[8] = MAP_macTxCsmaDelay;
#ifdef FEATURE_FREQ_HOP_MODE
  macMgmtAction1[9] = MAP_macBcTimerHandler;
#else
  macMgmtAction1[9] = NULL;
#endif /* FEATURE_FREQ_HOP_MODE */

  /* Action set 2 */
  macMgmtAction2[0] = MAP_macApiPwrOnReq;
  macMgmtAction2[1] = MAP_macApiDisassociateReq;
  macMgmtAction2[2] = MAP_macDisassocComplete;
#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)
  macMgmtAction2[3] = MAP_macRxBeacon;
#else
  macMgmtAction2[3] = NULL;
#endif /* (FEATURE_BEACON_MODE) || (FEATURE_NON_BEACON_MODE) */
  macMgmtAction2[4] = MAP_macRxDisassoc;

  /* initialize macMgmt struct */
  macPanCoordinator = FALSE;
}
#endif /* TIMAC_ROM_PATCH */

/**************************************************************************************************
 * @fn          macGetCoordAddress
 *
 * @brief       This function returns the coordinator address in an sAddr_t struct.
 *
 * input parameters
 *
 * @param       pAddr - pointer to sAddr_t struct.
 *
 * output parameters
 *
 * @param       pAddr - coordinator address in sAddr_t struct.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macGetCoordAddress(sAddr_t *pAddr)
{
  if (pMacPib->coordShortAddress == MAC_ADDR_USE_EXT)
  {
    MAP_sAddrCpy(pAddr, &(pMacPib->coordExtendedAddress));
  }
  else
  {
    pAddr->addrMode = SADDR_MODE_SHORT;
    pAddr->addr.shortAddr = pMacPib->coordShortAddress;
  }
}

/**************************************************************************************************
 * @fn          macGetMyAddrMode
 *
 * @brief       This function returns the device's own address mode.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      Address mode.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macGetMyAddrMode(void)
{
  if (pMacPib->shortAddress >= MAC_ADDR_USE_EXT)
  {
    return SADDR_MODE_EXT;
  }
  else
  {
    return SADDR_MODE_SHORT;
  }
}

/**************************************************************************************************
 * @fn          macDestAddrCmp
 *
 * @brief       This function compares the destination addresses of the two given frames.
 *
 * input parameters
 *
 * @param       p1 - pointer to first data frame bytes.
 * @param       p2 - pointer to second data frame bytes.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      TRUE if addresses match FALSE otherwise.
 **************************************************************************************************
 */
MAC_INTERNAL_API bool macDestAddrCmp(uint8 *p1, uint8 *p2)
{
  uint8   addrMode;

  /* p points to the first byte of the frame control field; destination address
   * mode is in the second byte
   */
  addrMode = MAC_DEST_ADDR_MODE(p1);

  if (addrMode == MAC_DEST_ADDR_MODE(p2))
  {
    if (addrMode == SADDR_MODE_SHORT)
    {
      /* compare short address */
      return ((p1[MAC_DEST_ADDR_OFFSET] == p2[MAC_DEST_ADDR_OFFSET]) &&
              (p1[MAC_DEST_ADDR_OFFSET + 1] == p2[MAC_DEST_ADDR_OFFSET + 1]));
    }
    else if (addrMode == SADDR_MODE_EXT)
    {
      return MAP_sAddrExtCmp(&p1[MAC_DEST_ADDR_OFFSET], &p2[MAC_DEST_ADDR_OFFSET]);
    }
  }
  return FALSE;
}

/**************************************************************************************************
 * @fn          macDestSAddrCmp
 *
 * @brief       This function compares the destination address and PAN ID the given frame
 *              with the given address and PAN ID.
 *
 * input parameters
 *
 * @param       pAddr - pointer to address.
 * @param       panId - PAN ID.
 * @param       p - pointer to data frame bytes.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      TRUE if addresses match FALSE otherwise.
 **************************************************************************************************
 */
MAC_INTERNAL_API bool macDestSAddrCmp(sAddr_t *pAddr, uint16 panId, uint8 *p)
{
  uint8   addrMode;
  bool    result = FALSE;

  /* p points to the first byte of the frame control field; destination address
   * mode is in the second byte
   */
  addrMode = MAC_DEST_ADDR_MODE(p);

  if (addrMode == pAddr->addrMode)
  {
    if (addrMode == SADDR_MODE_SHORT)
    {
      /* compare short address */
      result = ((pAddr->addr.shortAddr ==
                 BUILD_UINT16(p[MAC_DEST_ADDR_OFFSET], p[MAC_DEST_ADDR_OFFSET + 1])) &&
                (panId ==
                 BUILD_UINT16(p[MAC_DEST_PAN_ID_OFFSET], p[MAC_DEST_PAN_ID_OFFSET + 1])));
    }
    else if (addrMode == SADDR_MODE_EXT)
    {
      result = MAP_sAddrExtCmp(pAddr->addr.extAddr, &p[MAC_DEST_ADDR_OFFSET]);
    }
  }
  return result;
}

/**************************************************************************************************
 * @fn          macCoordAddrCmp
 *
 * @brief       This function compares the given address to the coordinator address.
 *
 * input parameters
 *
 * @param       pAddr - pointer to sAddr_t struct.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      TRUE if addresses match FALSE otherwise.
 **************************************************************************************************
 */
MAC_INTERNAL_API bool macCoordAddrCmp(sAddr_t *pAddr)
{
  if (pAddr->addrMode == SADDR_MODE_SHORT)
  {
    return (pMacPib->coordShortAddress == pAddr->addr.shortAddr);
  }
  else
  {
    return MAP_sAddrCmp(&(pMacPib->coordExtendedAddress), pAddr);
  }
}

/**************************************************************************************************
 * @fn          macCoordDestAddrCmp
 *
 * @brief       This function checks of the destination address in the given TX frame matches
 *              the coordinator address.
 *
 * input parameters
 *
 * @param       p - pointer to frame bytes.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      TRUE if addresses match FALSE otherwise.
 **************************************************************************************************
 */
MAC_INTERNAL_API bool macCoordDestAddrCmp(uint8 *p)
{
  uint8   addrMode;

  /* p points to the first byte of the frame control field; destination address
   * mode is in the second byte
   */
  addrMode = MAC_DEST_ADDR_MODE(p);

  /* if destination address not present then can only go to coordinator */
  if (addrMode == SADDR_MODE_NONE)
  {
    return TRUE;
  }
  else if ((addrMode == SADDR_MODE_SHORT) && (pMacPib->coordShortAddress < MAC_ADDR_USE_EXT))
  {
    return (pMacPib->coordShortAddress ==
            BUILD_UINT16(p[MAC_DEST_ADDR_OFFSET], p[MAC_DEST_ADDR_OFFSET + 1]));
  }
  else
  {
    return MAP_sAddrExtCmp(pMacPib->coordExtendedAddress.addr.extAddr, &p[MAC_DEST_ADDR_OFFSET]);
  }
}

#if !defined(TIMAC_ROM_PATCH)
/**************************************************************************************************
 * @fn          macBuildHeader
 *
 * @brief       This function builds the MAC header fields into buffer pBuf.  It builds the
 *              frame control, sequence number, and address fields.  If the frame is a command
 *              frame it builds the command frame identifier.  If the MAC is initialized for
 *              security macSecBuildHeader is called to build the security header.  If building
 *              the security header fails, the event and status are set in pBuf->hdr and a
 *              TX failure event for the frame is sent to the MAC.  The function returns
 *              MAC_SUCESS if successful or the failure status from macSecBuildHeader().
 *
 * input parameters
 *
 * @param       pBuf - Pointer to buffer containing tx struct.
 * @param       srcAddrMode - Source address mode.
 * @param       pDstAddr - Destination address.
 * @param       dstPanId - Destination PAN ID.
 *
 * output parameters
 *
 * None.
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8
macBuildHeader(macTx_t *pBuf, uint8 srcAddrMode, sAddr_t *pDstAddr, uint16 dstPanId, uint8 elideSeqNo)
{
  uint8   *p;
  uint16  fcf = 0;
  uint8   dstAddrMode = pDstAddr->addrMode;
  uint16  srcPanId;
  uint8   frameVersion = MAC_FRAME_VERSION_0;

#ifdef FEATURE_MAC_SECURITY
  uint8   status;
#endif /* FEATURE_MAC_SECURITY */

  if(pBuf->internal.frameType != MAC_INTERNAL_ASYNC)
  {
    MAP_osal_memcpy(&pBuf->internal.dest.dstAddr, pDstAddr, sizeof(sAddr_t));
  }

  /* initialize source PAN ID */
  if (srcAddrMode & MAC_SRC_PAN_ID_BROADCAST)
  {
    srcPanId = MAC_PAN_ID_BROADCAST;
  }
  else
  {
    srcPanId = pMacPib->panId;
  }
  srcAddrMode &= ~MAC_SRC_PAN_ID_BROADCAST;

  /* initialize some internal parameters */
  if (pBuf->internal.txOptions & MAC_TXOPTION_NO_CNF)
  {
    pBuf->internal.txMode = MAC_TX_MODE_INTERNAL;
  }
  else
  {
    pBuf->internal.txMode = 0;
  }

  if (pBuf->internal.txOptions & MAC_TXOPTION_INDIRECT)
  {
    pBuf->internal.txSched = MAC_TX_SCHED_INDIRECT;
  }
  else
  {
    pBuf->internal.txSched = MAC_TX_SCHED_READY;
  }

  /* setup retries */
  if (pBuf->internal.txOptions & MAC_TXOPTION_NO_RETRANS)
  {
    pBuf->internal.retries = 0;
  }
  else
  {
    pBuf->internal.retries = pMacPib->maxFrameRetries;
  }

  /* header is built in reverse order */

  /* set some frame control field bits */
  if (pBuf->internal.txOptions & MAC_TXOPTION_ACK)
  {
    fcf |= MAC_FCF_ACK_REQUEST_MASK;
  }

  /* set pending bit if requested */
  if (pBuf->internal.txOptions & MAC_TXOPTION_PEND_BIT)
  {
    fcf |= MAC_FCF_FRAME_PENDING_MASK;
  }

  /* working backwards  */
  p = pBuf->msdu.p - 1;

  /* if command frame build set fcf frame type field */
  if ((pBuf->internal.frameType > MAC_INTERNAL_DATA) &&
      (pBuf->internal.frameType <= MAC_INTERNAL_ENHANCED_BEACON_REQ))
  {
    if( MAC_INTERNAL_ENHANCED_BEACON_REQ == pBuf->internal.frameType )
    {
      /* Enhanced Beacon Request Command
      * For EBR frames, set Frame Version bits 13,12 to 1,0
      */
      fcf |= ((1 << (MAC_FCF_FRAME_VERSION_POS + 1)) & MAC_FCF_FRAME_VERSION_MASK);
      frameVersion = MAC_FRAME_VERSION_2;

      /* For EBR frames, we use EB Filter IE - indicate that in FCF
      */
      fcf |=  (1 << MAC_FCF_IE_LIST_PRESENT_POS);
    }

    fcf |= MAC_FRAME_TYPE_COMMAND;
  }
  else if (pBuf->internal.frameType > MAC_INTERNAL_ENHANCED_BEACON_REQ)
  {
#ifdef FEATURE_ENHANCED_ACK
    if (pBuf->internal.frameType == MAC_INTERNAL_ENH_ACK)
    {
      fcf |= MAC_FRAME_TYPE_ACK;
    }
    else
#endif /* FEATURE_ENHANCED_ACK */
    {
      fcf |= MAC_INTERNAL_DATA;
    }
  }
  else
  {
    fcf |= pBuf->internal.frameType;
  }

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif
      if (pBuf->sec.securityLevel > MAC_SEC_LEVEL_NONE)
      {
        /* PIB security is enabled, set security enabled in frame control field */
        fcf |= MAC_FCF_SEC_ENABLED_MASK;
      }

      /* If the security enabled in frame control field is enabled but security level is none */
      if ((fcf & MAC_FCF_SEC_ENABLED_MASK) && (pBuf->sec.securityLevel == MAC_SEC_LEVEL_NONE))
      {
        pBuf->hdr.status = MAC_UNSUPPORTED_SECURITY;
        /* Something else set the FCF Security Enabled subfield. */
        return (MAC_UNSUPPORTED_SECURITY);
      }

      /* Security is disabled */
      if (pMacPib->securityEnabled == FALSE)
      {
        if (pBuf->sec.securityLevel > MAC_SEC_LEVEL_NONE)
        {
          pBuf->hdr.status = MAC_UNSUPPORTED_SECURITY;
          /* Security is disabled but the security level is not 0 */
          return (MAC_UNSUPPORTED_SECURITY);
        }
      }

      /* Build auxiliary security header only if security is turned on and security level is not 0 */
      if (fcf & MAC_FCF_SEC_ENABLED_MASK)
      {
        /* Perform security processing; return status from security processing
         * This function will build security auxiliary security header and find the key.
         */
        if ((status = MAP_macOutgoingFrameSecurity(pBuf, pDstAddr, dstPanId,
                                               &(pBuf->internal.pKeyDesc)))
             != MAC_SUCCESS)
        {
          pBuf->hdr.status = status;

          MAP_macLowLevelDiags( MAC_DIAGS_TX_SEC_FAIL );

          return (status);
        }

        if (pBuf->internal.txOptions & MAC_TXOPTION_INDIRECT)
        {
          /* Because we cannot predict whether the pending bit will be set,
           * if we are to encrypt the packet here. As a workaround till
           * the real fix is added to encrypt after pending bit is set,
           * we always set the pending bit */
          fcf |= MAC_FCF_FRAME_PENDING_MASK;
        }

        /* working backwards, msdu.p is pointing to security header, if any.  */
        p = pBuf->msdu.p - 1;
      }

#if (defined TIMAC_ROM_IMAGE_BUILD)
    }
#endif
#endif /* FEATURE_MAC_SECURITY */

  /* Obtain the Frame Version */
  if ( pBuf->internal.ptrHeaderIE || pBuf->internal.ptrPayloadIE )
  {
    fcf |= (((uint16)1 << MAC_FCF_IE_LIST_PRESENT_POS) & MAC_FCF_IE_PRESENT_MASK);

    /* force frame version to 2 if IE's are present */
    fcf |= (((uint16)1 << (MAC_FCF_FRAME_VERSION_POS + 1)) & MAC_FCF_FRAME_VERSION_MASK);
    frameVersion = MAC_FRAME_VERSION_2;
  }
  else if ( ((pMacPib->frameVersionSupport == 1)&&(pBuf->msdu.len > MAC_A_MAX_FRAME_SIZE))
           || (fcf & MAC_FCF_SEC_ENABLED_MASK) )
  {
    fcf |= ((1 << (MAC_FCF_FRAME_VERSION_POS)) & MAC_FCF_FRAME_VERSION_MASK);
    frameVersion = MAC_FRAME_VERSION_1;
  }
  else if ( pMacPib->frameVersionSupport == 2)
  {
    /* Frame version 2 */
    fcf |= ((1 << (MAC_FCF_FRAME_VERSION_POS + 1)) & MAC_FCF_FRAME_VERSION_MASK);
    frameVersion = MAC_FRAME_VERSION_1;
  }

  /*if sensor and checkLRMPhyMode, frame version should be 2 */
  if((!macPanCoordinator) && MAC_FH_ENABLED &&
      (pBuf->internal.fhFrameType == MAC_FH_FRAME_DATA))
  {
    fcf &= ~MAC_FCF_FRAME_VERSION_MASK;
    fcf |= ((1 << (MAC_FCF_FRAME_VERSION_POS + 1)) & MAC_FCF_FRAME_VERSION_MASK);
    frameVersion = MAC_FRAME_VERSION_2;
  }

  /* source address */
  fcf |= (uint16) srcAddrMode << MAC_FCF_SRC_ADDR_MODE_POS;
  if (srcAddrMode == SADDR_MODE_SHORT)
  {
    *p-- = HI_UINT16(pMacPib->shortAddress);
    *p-- = LO_UINT16(pMacPib->shortAddress);
  }
  else if (srcAddrMode == SADDR_MODE_EXT)
  {
    p -= SADDR_EXT_LEN - 1;
    MAP_sAddrExtCpy(p, pMacPib->extendedAddress.addr.extAddr);
    p--;
  }

  /* source PAN ID */
  if(frameVersion == MAC_FRAME_VERSION_2)
  {
    /* Always set PAN Id Compression to 0 */
    if((dstAddrMode == SADDR_MODE_NONE) && (srcAddrMode != SADDR_MODE_NONE))
    {
       *p-- = HI_UINT16(srcPanId);
       *p-- = LO_UINT16(srcPanId);
    }
  }
  else
  {
    if ((srcAddrMode != SADDR_MODE_NONE) && (dstAddrMode != SADDR_MODE_NONE) &&
      (srcPanId == dstPanId))
    {
      fcf |= MAC_FCF_INTRA_PAN_MASK;
    }
    else if (srcAddrMode != SADDR_MODE_NONE)
    {
      *p-- = HI_UINT16(srcPanId);
      *p-- = LO_UINT16(srcPanId);
    }
  }

  /* destination address */
  fcf |= (uint16) dstAddrMode << MAC_FCF_DST_ADDR_MODE_POS;
  if (dstAddrMode == SADDR_MODE_SHORT)
  {
    *p-- = HI_UINT16(pDstAddr->addr.shortAddr);
    *p-- = LO_UINT16(pDstAddr->addr.shortAddr);
  }
  else if (dstAddrMode == SADDR_MODE_EXT)
  {
    p -= SADDR_EXT_LEN - 1;
    MAP_sAddrExtCpy(p, pDstAddr->addr.extAddr);
    p--;
  }

  /* destination PAN ID */
  if (dstAddrMode != SADDR_MODE_NONE)
  {
    *p-- = HI_UINT16(dstPanId);
    *p-- = LO_UINT16(dstPanId);
  }

  /* sequence number */
  if ( elideSeqNo )
  {
    fcf |= ((1 << MAC_FCF_SEQ_NO_SUPPRESS_POS) & MAC_FCF_SEQ_NO_SUPPRESS_MASK);
  }
  else
  {
#ifdef FEATURE_ENHANCED_ACK
    if (pBuf->internal.frameType == MAC_INTERNAL_ENH_ACK)
    {
      *p-- = MAP_macRxSequenceNum();
    }
    else
#endif /* FEATURE_ENHANCED_ACK */
    {
      /* ehanced beacon frame use ebsn, beacon frame use bsn, or else use dsn */
      *p-- = (pBuf->internal.frameType != MAC_INTERNAL_BEACON) ? pMacPib->dsn++ :
        pBuf->internal.ptrHeaderIE ? pMacPib->ebsn++ : pMacPib->bsn++;
    }
  }

  /* frame control field */
  *p-- = HI_UINT16(fcf);
  *p   = LO_UINT16(fcf);

  /* increment length by size of header */
#ifndef MAC_PROTOCOL_TEST
  pBuf->msdu.len += (uint8) (pBuf->msdu.p - p);
  pBuf->msdu.p = p;
#endif

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif

      if (fcf & MAC_FCF_SEC_ENABLED_MASK)
      {
        if (pBuf->internal.ptrPayloadIE != NULL)
        {
          pBuf->internal.ptrMData = pBuf->internal.ptrPayloadIE;
        }

        if ((!MAC_FH_ENABLED) || ((pBuf->internal.frameType == MAC_INTERNAL_ENH_ACK)))
        {
          /* If security is enabled but FH is not enabled,
           * encrypt and authenticate the frame here just once.
           * Frames with UTIE/BTIE will be encrypted/authenticated by LMAC
           */
          MAC_ASSERT(pBuf->internal.pKeyDesc->key != NULL);

          /* Copy outgoing frame counter and increment */
          pBuf->internal.frameCntr = pBuf->internal.pKeyDesc->frameCounter++;

          if ((status = MAP_macCcmStarTransform(pBuf->internal.pKeyDesc->key,
                                            pBuf->internal.frameCntr,
                                            pBuf->sec.securityLevel,
                                            p,
                                            pBuf->msdu.len,
                                            pBuf->internal.ptrMData,
                                            pBuf->msdu.len
                                              - (pBuf->internal.ptrMData - p)))
               != MAC_SUCCESS)
          {
            pBuf->hdr.status = status;
            return (status);
          }
        }
      }
#if (defined TIMAC_ROM_IMAGE_BUILD)
    }
#endif
#endif /* FEATURE_MAC_SECURITY */

  pBuf->hdr.status = MAC_SUCCESS;
  return MAC_SUCCESS;
}

/**************************************************************************************************
 * @fn          macBuildCommonReq
 *
 * @brief       This function builds and sends either a data request, pan ID conflict, orphan
 *              notification, beacon request, or zero length data frame.  First a buffer is allocated
 *              with macAllocTxBuffer().  If successful the TX options and frame type are set and
 *              other parameters in the macTx_t structure are initialized.  Then macBuildHeader()
 *              is called to build the header.  If successful macDataTxEnqueue() is called to
 *              queue the frame for transmission.
 *
 * input parameters
 *
 * @param       cmd - ID of command frame to send.
 * @param       srcAddrMode - Source address mode.
 * @param       pDstAddr - Destination address.
 * @param       dstPanId - Destination PAN ID.
 * @param       txOptions - TX options for this frame.
 * @param       sec - Security settings
 *
 * output parameters
 *
 * None.
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macBuildCommonReq(uint8 cmd, uint8 srcAddrMode, sAddr_t *pDstAddr,
                                         uint16 dstPanId, uint16 txOptions, ApiMac_sec_t *sec)
{
  macTx_t *pMsg;
  uint8   status = MAC_NO_RESOURCES;

  /* allocate buffer */
  if ((pMsg = MAP_macAllocTxBuffer(cmd, sec)) != NULL)
  {
    /* set parameters */
    pMsg->internal.txOptions = txOptions;
    pMsg->internal.frameType = cmd;

    if(MAC_FH_ENABLED && (cmd == MAC_INTERNAL_ZERO_DATA))
    {
        /* MAC_INTERNAL_ZERO_DATA cmd is actually an
         * unicast data packet for sleepy device */
        pMsg->internal.frameType = MAC_INTERNAL_DATA_UNICAST;
    }

    /* set security */
    MAP_osal_memcpy(&pMsg->sec, sec, sizeof(ApiMac_sec_t));

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif

    if ( (MAC_INTERNAL_ZERO_DATA != cmd) && (MAC_INTERNAL_ENH_ACK != cmd) )
    {
      /* Only the private payload field(Command Contents in case of command frame)
         should be secured. */
      pMsg->internal.ptrMData = pMsg->msdu.p + 1;
    }
    else
    {
      /* This is for the zero data packets. It does not have cmd Id */
      pMsg->internal.ptrMData = pMsg->msdu.p;
    }

#if (defined TIMAC_ROM_IMAGE_BUILD)
    }
#endif
#endif


    /* initialize IE pointers */
    pMsg->internal.ptrHeaderIE = NULL;
    pMsg->internal.ptrPayloadIE = NULL;
#ifdef FEATURE_FREQ_HOP_MODE
    /* When Freq. hopping is enabled, data poll request cmd
       is routed as FH Data Frame */
    if( MAC_FH_ENABLED && ( (MAC_INTERNAL_DATA_REQ == cmd) || (MAC_INTERNAL_ZERO_DATA == cmd) ) )
    {

      pMsg->internal.fhFrameType = MAC_FH_FRAME_DATA;

      pMsg->internal.nb = 0;
      pMsg->internal.be =
        (pMsg->internal.txOptions & MAC_TXOPTION_ALT_BE) ? pMacPib->altBe : pMacPib->minBe;

      /* Copy the extended address (needs to be extended) for FH module to use */
      MAP_osal_memcpy(&pMsg->internal.dest.dstAddr, &pDstAddr->addr, sizeof(sAddr_t));

      if(pDstAddr->addrMode == SADDR_MODE_EXT)
      {
        /* Correct address mode since FH does not use short address */
        srcAddrMode = SADDR_MODE_EXT;

        if(pMacPib->rxOnWhenIdle)
        {
          /* build the payload IE's if part of the command */
          if ( macCmdIncludeFHIe[cmd] & MAC_FH_PAYLOAD_IE_MASK )
          {
            pMsg->internal.payloadIeInfo[0].gId = MAC_PAYLOAD_IE_WISUN_GID;
            pMsg->internal.payloadIeInfo[0].subId.subIdBMap =
            macCmdIncludeFHIe[cmd] & MAC_FH_PAYLOAD_IE_MASK;

            MAP_macBuildPayloadIes(pMsg, pMsg->internal.payloadIeInfo, TRUE, FALSE);
          }

          /* build the header IE's if part of the command */
          if ( macCmdIncludeFHIe[cmd] & MAC_FH_HEADER_IE_MASK )
          {
            pMsg->internal.headerIeInfo[0].elementId = MAC_HEADER_IE_WISUN;
            pMsg->internal.headerIeInfo[0].subIdBitMap =
            macCmdIncludeFHIe[cmd] & MAC_FH_HEADER_IE_MASK;

            MAP_macBuildHeaderIes(pMsg, pMsg->internal.headerIeInfo);
          }
        }
      }
    }

#ifdef FEATURE_ENHANCED_ACK
    /* When Freq. hopping is enabled, enhanced ack is processed here */
    if( MAC_FH_ENABLED && (MAC_INTERNAL_ENH_ACK == cmd) )
    {
      pMsg->internal.fhFrameType = MAC_FH_FRAME_ACK;

      /* build the header IE's if part of the ACK */
      /* Check if ACK has another IEs other than BT-IE */
      if ( macCmdIncludeFHIe[cmd] & (MAC_FH_HEADER_IE_MASK & ~MAC_FH_BT_IE) )
      {
        pMsg->internal.headerIeInfo[0].elementId = MAC_HEADER_IE_WISUN;
        pMsg->internal.headerIeInfo[0].subIdBitMap =
          macCmdIncludeFHIe[cmd] & MAC_FH_HEADER_IE_MASK;

        if(!macPanCoordinator)
        {
            pMsg->internal.headerIeInfo[0].subIdBitMap &= ~MAC_FH_BT_IE;
        }

        if(!pMacPib->rxOnWhenIdle)
        {
            pMsg->internal.headerIeInfo[0].subIdBitMap &= ~MAC_FH_UT_IE;
        }

        MAP_macBuildHeaderIes(pMsg, pMsg->internal.headerIeInfo);
      }

      /* Prepare the ACK to be send directly in low level MAC */
      status = MAP_macBuildHeader(pMsg, srcAddrMode, pDstAddr, dstPanId, 0);
      if (status == MAC_SUCCESS)
      {
        MAP_macRadioTxEnhAckCb(pMsg);
      }
      else
      {
#ifdef ENHACK_MALLOC
        /* Message must be freed here for failed status */
        MAP_osal_msg_deallocate( (uint8*)pMsg );
#endif
      }
      return status;
    }
#endif /* FEATURE_ENHANCED_ACK */
#endif /*FEATURE_FREQ_HOP_MODE */
    status = MAP_macBuildHeader(pMsg, srcAddrMode, pDstAddr, dstPanId, 0);
    if (status == MAC_SUCCESS)
    {
      /* handle special cases */
      switch (cmd)
      {
        case MAC_INTERNAL_ORPHAN_NOTIF:
        case MAC_INTERNAL_BEACON_REQ:
          pMsg->internal.txSched |= MAC_TX_SCHED_SCAN_RELATED;

        default:
          break;
      }

      /* queue frame for transmission */
      MAP_macDataTxEnqueue(pMsg);
    }
  }

  return status;
}
#endif

/**************************************************************************************************
 * @fn          macBuildDisassociateReq
 *
 * @brief       This function builds and sends a disassociate request command frame in buffer
 *              pEvent.  The TX options and frame type are set and other parameters in the
 *              macTx_t structure are initialized.  Then macBuildHeader() is called to build
 *              the header with the address information from the API event structure.  If
 *              successful the disassociate reason is built in the frame and macDataTxEnqueue()
 *              is called to queue the frame for transmission.
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
MAC_INTERNAL_API uint8 macBuildDisassociateReq(macEvent_t *pEvent)
{
  uint8 options = MAC_TXOPTION_ACK;
  sAddr_t deviceAddress;
  uint16 devicePanId;
  uint8 status;

  if (pEvent->apiData.mac.disassociateReq.txIndirect)
  {
    options |= MAC_TXOPTION_INDIRECT;
  }

  /* set parameters */
  pEvent->tx.internal.txOptions = options;
  pEvent->tx.internal.frameType = MAC_INTERNAL_DISASSOC_NOTIF;

  /* use a local copy */
  MAP_copyApiMacAddrToMacAddr(&deviceAddress, &pEvent->apiData.mac.disassociateReq.deviceAddress);
  devicePanId = pEvent->apiData.mac.disassociateReq.devicePanId;

  /* buffer is set up to build frame in reverse, so build disassoc reason into frame */
  pEvent->tx.msdu.p--;
  *pEvent->tx.msdu.p-- = (uint8)pEvent->apiData.mac.disassociateReq.disassociateReason;

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

  /* set length */
  pEvent->tx.msdu.len += MAC_DISASSOC_NOTIF_PAYLOAD;

  /* initialize IE pointers */
  pEvent->tx.internal.ptrHeaderIE = NULL;
  pEvent->tx.internal.ptrPayloadIE = NULL;
#ifdef FEATURE_FREQ_HOP_MODE
  /* If FH is enabled include the necessary IE's */
  if ( MAC_FH_ENABLED )
  {
    pEvent->apiData.internal.fhFrameType = MAC_FH_FRAME_DATA;


    MAP_osal_memcpy(&pEvent->tx.internal.dest.destAddr, &pEvent->apiData.mac.disassociateReq.deviceAddress.addr, sizeof(sAddr_t));

    if(pEvent->apiData.mac.disassociateReq.deviceAddress.addrMode == SADDR_MODE_EXT)
    {
      MAP_sAddrExtCpy((uint8 *)&pEvent->tx.internal.dest.destAddr, (const uint8 *)&pEvent->apiData.mac.disassociateReq.deviceAddress.addr.extAddr);
      if(pMacPib->rxOnWhenIdle)
      {
        if ( macCmdIncludeFHIe[MAC_INTERNAL_DISASSOC_NOTIF] & MAC_FH_PAYLOAD_IE_MASK )
        {
          pEvent->tx.internal.payloadIeInfo[0].gId = MAC_PAYLOAD_IE_WISUN_GID;
          pEvent->tx.internal.payloadIeInfo[0].subId.subIdBMap =
          macCmdIncludeFHIe[MAC_INTERNAL_DISASSOC_NOTIF] & MAC_FH_PAYLOAD_IE_MASK;

          MAP_macBuildPayloadIes(&pEvent->tx, pEvent->tx.internal.payloadIeInfo, TRUE, FALSE);
        }

        if ( macCmdIncludeFHIe[MAC_INTERNAL_DISASSOC_NOTIF] & MAC_FH_HEADER_IE_MASK )
        {
          pEvent->tx.internal.headerIeInfo[0].elementId = MAC_HEADER_IE_WISUN;
          pEvent->tx.internal.headerIeInfo[0].subIdBitMap =
          macCmdIncludeFHIe[MAC_INTERNAL_DISASSOC_NOTIF] & MAC_FH_HEADER_IE_MASK;

          MAP_macBuildHeaderIes(&pEvent->tx, pEvent->tx.internal.headerIeInfo);
        }
      }
    }
  }
#endif
  status = MAP_macBuildHeader(&pEvent->tx, SADDR_MODE_EXT,
                          &deviceAddress,
                          devicePanId, 0);

   if(status == MAC_SUCCESS)
   {
     /* queue frame for transmission */
     MAP_macDataTxEnqueue(&pEvent->tx);
   }
  return status;
}

/**************************************************************************************************
 * @fn          macSecCpy
 *
 * @brief       This function copies the security structure.
 *
 * input parameters
 *
 * @param       pDst - Pointer to the destination security structure.
 *              pSrc - Pointer to the source security structure.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************/
MAC_INTERNAL_API void macSecCpy (ApiMac_sec_t *pDst, ApiMac_sec_t *pSrc)
{
  MAP_osal_memcpy(pDst, pSrc, sizeof (ApiMac_sec_t));
}

/**************************************************************************************************
 * @fn          macNoAction
 *
 * @brief       This action function does nothing.
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
MAC_INTERNAL_API void macNoAction(macEvent_t *pEvent)
{
  (void)pEvent;  // Intentionally unreferenced parameter
  return;
}

#if !defined(TIMAC_ROM_PATCH)
/**************************************************************************************************
 * @fn          macCsmaDelay
 *
 * @brief       This action function handles CSMA backoff/timer expiry.
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
MAC_INTERNAL_API void macTxCsmaDelay(macEvent_t *pEvent)
{
  (void)pEvent;  // Intentionally unreferenced parameter

  MAP_txCsmaDelay(0);
  return;
}
#endif

/**************************************************************************************************
 * @fn          macCsmaDelay
 *
 * @brief       This action function handles CSMA backoff/timer expiry.
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
MAC_INTERNAL_API void macBcTimerHandler(macEvent_t *pEvent)
{
  (void)pEvent;  // Intentionally unreferenced parameter
  MAP_macSymbolTimerBCWakeUp();
}

/**************************************************************************************************
 * @fn          macApiUnsupported
 *
 * @brief       This action function is called when an API event is received which is not
 *              supported in the current configuration set via the API initialization functions.
 *              The MAC callback function is called with the callback event set to the event
 *              matching the API event and sets the status to MAC_UNSUPPORTED.
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
MAC_INTERNAL_API void macApiUnsupported(macEvent_t *pEvent)
{
  MAP_macCbackForEvent(pEvent, MAC_UNSUPPORTED);
}

/**************************************************************************************************
 * @fn          macDefaultAction
 *
 * @brief       This action function is executed by the state machine when an action function
 *              set is not present for the enumerated action ID.
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
MAC_INTERNAL_API void macDefaultAction(macEvent_t *pEvent)
{
    MAP_macNoAction(pEvent);
}

/**************************************************************************************************
 * @fn          macApiBadState
 *
 * @brief       This action function is called when an API event is received in a state that
 *              is not allowed by the state machine.  The MAC callback function is called with
 *              the callback event set to the event matching the API event and sets the status
 *              to MAC_BAD_STATE.
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
MAC_INTERNAL_API void macApiBadState(macEvent_t *pEvent)
{
  MAP_macCbackForEvent(pEvent, MAC_BAD_STATE);
}

/**************************************************************************************************
 * @fn          macApiPending
 *
 * @brief       This action function is called when an API event is received and cannot be
 *              processed immediately in the current state.  The event buffer is stored in
 *              macMain.pPending.  If there is already a buffer stored in macMain.pPending,
 *              that buffer is discarded by calling macApiBadState().
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
MAC_INTERNAL_API void macApiPending(macEvent_t *pEvent)
{
  if (macMain.pPending != NULL)
  {
    MAP_macApiBadState(macMain.pPending);
    MAP_mac_msg_deallocate((uint8 **)&macMain.pPending);
  }
  MAP_macMainReserve((uint8 *)pEvent);
  macMain.pPending = pEvent;
}

/**************************************************************************************************
 * @fn          macCommStatusInd
 *
 * @brief       This action function is called to send a comm. status indication to the
 *              application.  It checks the state machine event; if the event is for transmission
 *              failure of a coordinator realignment or associate response, the address is
 *              retrieved from the transmit frame.  Otherwise the event must be the result of a
 *              security failure in a received frame; the address is retrieved from the receive
 *              frame.  Then the MAC callback function is called with event
 *              MAC_MLME_COMM_STATUS_IND.
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
MAC_INTERNAL_API void macCommStatusInd(macEvent_t *pEvent)
{
  macMlmeCommStatusInd_t  commStatusInd;

  /* initialize the security fields */
  MAP_osal_memset( &commStatusInd.sec, 0x0, sizeof(commStatusInd.sec));

  commStatusInd.reason = MAC_COMM_RX_SECURE;
  MAP_sAddrCpy(&commStatusInd.dstAddr, &pEvent->rx.mac.dstAddr);
  MAP_sAddrCpy(&commStatusInd.srcAddr, &pEvent->rx.mac.srcAddr);
  commStatusInd.panId = pMacPib->panId;
  MAP_macSecCpy(&commStatusInd.sec, &pEvent->rx.sec);

  commStatusInd.hdr.event = MAC_MLME_COMM_STATUS_IND;
  commStatusInd.hdr.status = pEvent->hdr.status;

  MAP_MAC_CbackEvent((macCbackEvent_t *) &commStatusInd);
}

/**************************************************************************************************
 * @fn          macUpdateCsmPhyParam
 *
 * @brief       This function updates the value of the csmPhy flag.
 *              A value of TRUE indicates the csm phy is in operation.
 *
 * input parameters
 *
 * @param       val - value to be set the csmPhy flag.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macUpdateCsmPhyParam(bool val)
{
  csmPhy = val;
}

/**************************************************************************************************
 * @fn          macIsCsmOperational
 *
 * @brief       This function returns the value of the csmPhy.
 *              TRUE indicates the csm (common signalling mode) PHY mode is in operation.
 *
 * input parameters
 *
 * None
 *
 * output parameters
 *
 * None.
 *
 * @return      TRUE if the csm is in operation otherwise FALSE.
 **************************************************************************************************
 */
MAC_INTERNAL_API bool macIsCsmOperational(void)
{
  return csmPhy;
}

/**************************************************************************************************
 * @fn          MAC_MlmeResetReq
 *
 * @brief       This direct execute function resets the MAC.  This function
 *              must be called once at system startup before any other
 *              function in the management API is called.
 *
 * input parameters
 *
 * @param       setDefaultPib - Set to TRUE to reset the MAC PIB to its
 *                              default values.
 *
 * output parameters
 *
 * None.
 *
 * @return      Returns MAC_SUCCESS always.
 *
 **************************************************************************************************
 */
uint8 MAC_MlmeResetReq(bool setDefaultPib)
{
  uint8         i;
  halIntState_t intState;

  HAL_ENTER_CRITICAL_SECTION(intState);

  /* initialize csm param to FALSE */
  MAP_macUpdateCsmPhyParam( FALSE );

  /* initialize PIB first */
  if (setDefaultPib)
  {
    MAP_macPibReset();

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif

    MAP_macSecurityPibReset();

#if (defined TIMAC_ROM_IMAGE_BUILD)
    }
#endif

#endif

  }

#ifdef FEATURE_FREQ_HOP_MODE
  /* reset the FH module */
  if ( MAC_FH_ENABLED && MAC_FH_RESET_FN )
  {
    MAC_FH_RESET_FN();
  }
#endif /* FEATURE_FREQ_HOP_MODE */

  /* reset low level */
  MAP_macLowLevelReset();

  /* initialize radio with values from PIB */
  MAP_macRadioSetTxPower(pMacPib->phyTransmitPower);
  MAP_macRadioSetPanCoordinator(macPanCoordinator);
  MAP_macRadioSetPanID(pMacPib->panId);
  MAP_macRadioSetShortAddr(pMacPib->shortAddress);
  MAP_macRadioSetIEEEAddr(pMacPib->extendedAddress.addr.extAddr);

  /* reset run-time configurable high level modules */
  for (i = 0; i < MAC_FEAT_MAX; i++)
  {
    if (macReset[i] != NULL)
    {
      (*macReset[i])();
    }
  }
  /* reset high level modules */
  MAP_macMainReset();
  MAP_macPwrReset();
  MAP_macDataReset();
  MAP_macMgmtReset();

  HAL_EXIT_CRITICAL_SECTION(intState);

  /* turn rx back on if it was on; low level reset turns it off */
  MAP_MAC_MlmeSetReq(MAC_RX_ON_WHEN_IDLE, &pMacPib->rxOnWhenIdle);

  /* This should be outside critical section. */
  MAP_macRadioSetChannel(pMacPib->logicalChannel);

  /* reset low level */
  /* Radio is up, turn RAT timers ON and macSymbolTimerImpending OFF to indicate Radio ON */
  MAP_macLowLevelReset();

  return MAC_SUCCESS;
}

/**************************************************************************************************
 * @fn          MAC_ResumeReq
 *
 * @brief       This direct execute function resumes the MAC after a successful return from
 *              MAC_YieldReq().
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
#if !defined(TIMAC_ROM_PATCH)
void MAC_ResumeReq(void)
{
  halIntState_t s;

  /* Re-initialize PHY */
  MAP_macLowLevelResume();

  HAL_ENTER_CRITICAL_SECTION(s);

  /* Invalidate current channel and power numbers so that they can be
   * re-initialized anyway after PHY re-initialization.
   */
  MAP_macRadioSwInit();

  /* Re-initialize radio with the values from the PIB. */
  MAP_macRadioSetTxPower(pMacPib->phyTransmitPower);
  MAP_macRadioSetPanCoordinator(macPanCoordinator);
  MAP_macRadioSetPanID(pMacPib->panId);
  MAP_macRadioSetShortAddr(pMacPib->shortAddress);
  MAP_macRadioSetIEEEAddr(pMacPib->extendedAddress.addr.extAddr);
  HAL_EXIT_CRITICAL_SECTION(s);

  /* Re-initialize radio channel */
  MAP_macRadioSetChannel(pMacPib->logicalChannel);

  /* Turn RX back on if it should be on - low level reset turns it off. */
  MAP_MAC_MlmeSetReq(MAC_RX_ON_WHEN_IDLE, &pMacPib->rxOnWhenIdle);

  if ( FALSE == MAP_macIsCsmOperational() )
  {
    MAP_macSymbolTimerInit();
  }
}
#endif
/**************************************************************************************************
 * @fn          MAC_YieldReq
 *
 * @brief       Checks if the mac is in idle or polling state by calling macStateIdleOrPolling().
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS - The MAC has yielded.
 *              MAC_DENIED  - The MAC cannot yield now.
 **************************************************************************************************
 */
uint8 MAC_YieldReq(void)
{
  if (MAP_macStateIdleOrPolling())
  {
    if (MAP_macLowLevelYield())
    {
      MAP_macRxDisable(MAC_RX_WHEN_IDLE);
      return MAC_SUCCESS;
    }
  }

  return MAC_DENIED;
}
/**************************************************************************************************
 * @fn          macSetDefaultsByPhyID
 *
 * @brief       This function changes the mac PIB depending on Phy ID.
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
MAC_INTERNAL_API void macSetDefaultsByPhyID(void)
{
  uint8 minBe = MAC_DEFAULT_MIN_BE;
  uint8 maxBe = MAC_DEFAULT_MAX_BE;
  uint8 preamble = MAC_PREAMBLE_LEN_8;
  uint8 rfFreq = MAC_RF_FREQ_SUBG;
  uint8 logicalChannel = MAC_CHAN_LOWEST;
  uint8 responseWaitTime = MAC_DEFAULT_RESPONSE_WAIT_TIME;
  uint16 maxFrameTotalWaitTime = MAC_DEFAULT_FRAME_WAIT_TIME;

  if (MAP_macCheckPhyRate(pMacPib->curPhyID) == PHY_MODE_SLR_5K)
  {
      minBe = MAC_LRM_DEFAULT_MIN_BE;
      maxBe = MAC_LRM_DEFAULT_MAX_BE;
      responseWaitTime = MAC_LRM_DEFAULT_RESPONSE_WAIT_TIME;
  }

  /* change minBE and maxBE to default values */
  if (pMacPib->minBe != minBe)
  {
    MAP_MAC_MlmeSetReq(MAC_MIN_BE, &minBe);
  }
  if (pMacPib->maxBe != maxBe)
  {
    MAP_MAC_MlmeSetReq(MAC_MAX_BE, &maxBe);
  }

  // if data rate == 300kbps or 150, set the pib value for fskPreambleLen
  if(MAP_macCheckPhyRate(pMacPib->curPhyID) == PHY_MODE_FSK_150K)
  {
      preamble = MAC_PREAMBLE_LEN_12;
        // set preamble here
      MAP_MAC_MlmeSetReq(MAC_PHY_FSK_PREAMBLE_LEN, &preamble);
  }
  else if(MAP_macCheckPhyRate(pMacPib->curPhyID) == PHY_MODE_FSK_300K)
  {
      // set preamble here
      preamble = MAC_PREAMBLE_LEN_24;
      MAP_MAC_MlmeSetReq(MAC_PHY_FSK_PREAMBLE_LEN, &preamble);
  }
  else
  {
      MAP_MAC_MlmeSetReq(MAC_PHY_FSK_PREAMBLE_LEN, &preamble);
  }

  if (MAP_macCheckPhyRate(pMacPib->curPhyID) == PHY_MODE_FSK_200K)
  {
      responseWaitTime = MAC_200K_DEFAULT_RESPONSE_WAIT_TIME;
      maxFrameTotalWaitTime = MAC_200K_DEFAULT_FRAME_WAIT_TIME;
  }

  /* change minBE and maxBE to default values */
  if (pMacPib->responseWaitTime != responseWaitTime)
  {
    MAP_MAC_MlmeSetReq(MAC_RESPONSE_WAIT_TIME, &responseWaitTime);
  }
  if (pMacPib->maxFrameTotalWaitTime != maxFrameTotalWaitTime)
  {
    MAP_MAC_MlmeSetReq(MAC_MAX_FRAME_TOTAL_WAIT_TIME, &maxFrameTotalWaitTime);
  }

  MAP_MAC_MlmeSetReq(MAC_RF_FREQ, &rfFreq);
  MAP_MAC_MlmeSetReq(MAC_LOGICAL_CHANNEL, &logicalChannel);
}

/**************************************************************************************************
 * @fn          macSetDefaultsByRE
 *
 * @brief       This function changes the mac PIB depending on Range Extender Mode.
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
MAC_INTERNAL_API void macSetDefaultsByRE(void)
{
  int8 power = MAC_DEFAULT_TX_POWER;

  if (pMacPib->rangeExt)
  {
      power = MAC_RE_DEFAULT_TX_POWER;
  }

  /* change minBE and maxBE to default values */
  if (pMacPib->phyTransmitPower != power)
  {
    MAP_MAC_MlmeSetReq(MAC_PHY_TRANSMIT_POWER_SIGNED, &power);
  }

}

/**************************************************************************************************
 * @fn          MAC_Init
 *
 * @brief       This function initializes the MAC subsystem.  It calls the low level MAC
 *              initalization function.
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
void MAC_Init(void)
{
  /* Initialize action set for actions common to all types of nodes */
  macActionSet[MAC_MGMT_ACTION_1] = macMgmtAction1; /* Mgmt set 1 */
  macActionSet[MAC_MGMT_ACTION_2] = macMgmtAction2; /* Mgmt set 2 */

  macMain.featureMask = 0;

  /* The PIB must reset here. MLME may reset it again. */
  MAP_macPibReset();
}

#if !defined(TIMAC_ROM_PATCH)
/**
 * @brief This function initializes the MAC to allow Frequency
 *        hopping operation.
 *
 * @return None
 */
void MAC_InitFH(void)
{
  fhObject.fhEnabled = FALSE;
#ifdef FEATURE_FREQ_HOP_MODE
  fhObject.pFhResetFn = FHAPI_reset;
  fhObject.pFhStartFn = FHAPI_start;
  fhObject.pFhStartBsFn = FHAPI_startBS;
  fhObject.pFhStopAsyncFn = FHAPI_stopAsync;
  fhObject.pFhSendDataFn = FHAPI_sendData;
  fhObject.pFhGetIeLenFn = FHIE_getLen;
  fhObject.pFhGenIeFn = FHIE_gen;
  fhObject.pFhParsePieFn = FHIE_parsePie;
  fhObject.pFhExtractHieFn = FHIE_extractHie;
  fhObject.pFhExtractPieFn = FHIE_extractPie;
  fhObject.pFhParseHieFn = FHIE_parseHie;
  fhObject.pFhResetPibFn =FHPIB_reset;
  fhObject.pFhSetPibFn = FHPIB_set;
  fhObject.pFhGetPibFn = FHPIB_get;
  fhObject.pFhGetPibLenFn = FHPIB_getLen;
  fhObject.pFhGetTxParamsFn = FHAPI_getTxParams;
  fhObject.pFhCompleteTxCbFn = FHAPI_completeTxCb;
  fhObject.pFhCompleteRxCbFn = FHAPI_completeRxCb;
  fhObject.pFhSetStateCbFn = FHAPI_setStateCb;
  fhObject.pFhAddDeviceCB = FHNT_AddDeviceCB;
  fhObject.pFhDelDeviceCB = FHNT_DelDeviceCB;
  fhObject.pFhGetTxChHopFn = FHAPI_getTxChHoppingFunc;
  fhObject.pFhGetRemDTFn = FHAPI_getRemDT;
  fhObject.pFhUpdateCCA = FHDATA_updateCCA;
  fhObject.pFhGetBitCount = FHAPI_getBitCount;
  fhObject.pFhBcTimerEvent = FHMGR_BCTimerEventUpd;
#endif /* FEATURE_FREQ_HOP */
} /* MAC_InitFH() */
#endif

/**
 * @brief This direct excute function gets the MAC FH PIB
 *        attribute size.
 *
 * @param pibAttribute - The attribute identifier.
 *
 * @return size in bytes.
 */
uint8 MAC_MlmeFHGetReqSize( uint16 pibAttribute )
{
#ifdef FEATURE_FREQ_HOP_MODE
  if ( MAC_FH_ENABLED && MAC_FH_GET_PIB_LEN_FN )
  {
    return MAC_FH_GET_PIB_LEN_FN( pibAttribute );
  }
#endif /* FEATURE_FREQ_HOP_MODE */
}

/**************************************************************************************************
 * @fn          MAC_MlmeFHGetReq
 *
 * @brief       This direct execute function retrieves an attribute value
 *              from the MAC FH(Frequency hopping) PIB.
 *
 * input parameters
 *
 * @param       pibAttribute - The FH attribute identifier.
 * @param       pValue - pointer to the attribute value.
 *
 * output parameters
 *
 * @param       pValue - pointer to the attribute value.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *              MAC_UNSUPPORTED if frequency hopping is not enabled.
 *
 **************************************************************************************************
 */
uint8 MAC_MlmeFHGetReq(uint16 pibAttribute, void *pValue)
{
#ifdef FEATURE_FREQ_HOP_MODE
  if ( MAC_FH_ENABLED && MAC_FH_GET_PIB_FN )
  {
    FHAPI_status status;

    status = MAC_FH_GET_PIB_FN(pibAttribute, pValue);

    return status;
  }
#endif /* FEATURE_FREQ_HOP_MODE */
}

/**************************************************************************************************
 * @fn          MAC_MlmeFHSetReq
 *
 * @brief       This direct execute function sets an attribute value
 *              in the MAC FH(Frequency hopping) PIB.
 *
 * input parameters
 *
 * @param       pibAttribute - The FH attribute identifier.
 * @param       pValue - pointer to the attribute value.
 *
 * output parameters
 *
 * None.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *              MAC_UNSUPPORTED if frequency hopping is not enabled.
 *
 **************************************************************************************************
 */
uint8 MAC_MlmeFHSetReq(uint16 pibAttribute, void *pValue)
{
#ifdef FEATURE_FREQ_HOP_MODE
  if ( MAC_FH_ENABLED && MAC_FH_SET_PIB_FN )
  {
    FHAPI_status status;

    status = MAC_FH_SET_PIB_FN(pibAttribute, pValue);

    return status;
  }
#endif /* FEATURE_FREQ_HOP_MODE */
}

/**
 * @brief Starts the Frequency hopping operation.
 *
 * @return None
 */
uint8 MAC_StartFH(void)
{
#ifdef FEATURE_FREQ_HOP_MODE
  if ( MAC_FH_ENABLED && MAC_FH_START_FN )
  {
    MAC_FH_START_FN();
    return MAC_SUCCESS;
  }
#endif /* FEATURE_FREQ_HOP_MODE */
  return MAC_UNSUPPORTED;
}


/**
 * @brief Enables the Frequency hopping operation.
 *
 * @return MAC_SUCCESS if successful in enabling the frequency
 *         hopping, otherwise MAC_UNSUPPORTED.
 */
uint8 MAC_EnableFH(void)
{
#ifdef FEATURE_FREQ_HOP_MODE
  fhObject.fhEnabled = TRUE;

  /* set default value for FH */
  if (MAC_FH_RESET_FN)
  {
    MAC_FH_RESET_FN();
  }

    return MAC_SUCCESS;
#else
    return MAC_UNSUPPORTED;
#endif /* FEATURE_FREQ_HOP_MODE */
}

/**
 * @brief This function calls macSendDataMsg() to send the OSAL
 *        message containing MAC_API_WS_ASYNC_REQ_EVT to the MAC
 *
 * @param  pData - Pointer to the parameters structure.
 * @return MAC_SUCCESS if successful otherwise MAC error codes.
 */
uint8 MAC_MlmeWSAsyncReq( ApiMac_mlmeWSAsyncReq_t* pData )
{
  return MAP_macSendDataMsg(MAC_API_WS_ASYNC_REQ_EVT, pData, &pData->sec);
}

#ifdef FEATURE_FREQ_HOP_MODE
/**
 * @brief Calls the MAC callback function with
 *        MAC_MLME_WS_ASYNC_CNF event.
 *
 * @param status - WS Async confirm status.
 *
 * @return none
 */
MAC_INTERNAL_API void macWSAsyncCnf(uint8 status)
{
  macMlmeWSAsyncCnf_t asyncCnf;
  asyncCnf.hdr.event = MAC_MLME_WS_ASYNC_CNF;
  asyncCnf.hdr.status = status;

  MAP_MAC_CbackEvent((macCbackEvent_t *) &asyncCnf);
}


/**
 * @brief Builds the specific WiSUN Async message
 *
 * @param pEvent - Pointer to the event data.
 *
 * @return uint8 MAC_SUCCESS if successful. Following status is
 *         returned on failure condition MAC_NO_RESOURCES,
 *         MAC_NO_DATA
 */
MAC_INTERNAL_API uint8 macBuildWSAsyncMsg(macEvent_t *pEvent )
{
  sAddr_t dstAddr;
  uint8 status = MAC_SUCCESS;

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif

      /* Only the private payload field(Command Contents in case of command frame)
         should be secured. */
      pEvent->tx.internal.ptrMData = pEvent->tx.msdu.p;

#if (defined TIMAC_ROM_IMAGE_BUILD)
    }
#endif
#endif

  pEvent->tx.internal.totalLen = MAC_WS_ASYNC_REQ_MAX_PAYLOAD;

  /* initialize IE pointers */
  pEvent->tx.internal.ptrPayloadIE = NULL;
  pEvent->tx.internal.ptrHeaderIE = NULL;

  if ( MAC_PAYLOAD_IE_TERM_GID != pEvent->tx.internal.payloadIeInfo[0].gId )
  {
    status = MAP_macBuildPayloadIes(&pEvent->tx, pEvent->tx.internal.payloadIeInfo, TRUE, FALSE);
  }

  if ( MAC_SUCCESS != status )
  {
    return status;
  }

  if ( (MAC_HEADER_IE_TERM != pEvent->tx.internal.headerIeInfo[0].elementId) ||
       (NULL != pEvent->dataReq.internal.ptrPayloadIE) )
  {
    /* Builds the MAC specific header IE's if any and also adds the header
       termination IE */
    status = MAP_macBuildHeaderIes(&pEvent->tx, pEvent->tx.internal.headerIeInfo );
  }

  if ( MAC_SUCCESS == status )
  {
    dstAddr.addrMode = SADDR_MODE_NONE;

    /* mac build header */
    status = MAP_macBuildHeader(&pEvent->tx, SADDR_MODE_EXT, &dstAddr, 0, 1);
  }

  return status;
}

/**
 * @brief Identifies which WiSUN async message to build based
 *        on the request information and makes a call to build
 *        the message.
 *
 * @param pEvent - pointer to the event data.
 *
 * @return uint8
 */
uint8 macBuildWSAsyncReq(macEvent_t *pEvent)
{
  uint8 status = MAC_SUCCESS;

  /* set parameters */
  pEvent->tx.internal.frameType = MAC_INTERNAL_ASYNC;
  MAP_osal_memcpy( pEvent->tx.internal.dest.chList,
               pEvent->apiData.mac.asyncReq.channels,
               MAC_154G_CHANNEL_BITMAP_SIZ);

  MAP_osal_memset( &(pEvent->tx.internal.headerIeInfo), MAC_HEADER_IE_TERM,
               sizeof(headerIeInfo_t) * MAC_HEADER_IE_MAX );
  MAP_osal_memset( &(pEvent->tx.internal.payloadIeInfo), MAC_PAYLOAD_IE_TERM_GID,
               sizeof(payloadIeInfo_t) * MAC_PAYLOAD_IE_MAX );

  switch ( pEvent->apiData.mac.asyncReq.frameType)
  {
    case MAC_WS_ASYNC_PAN_ADVERT:
    {
      pEvent->tx.internal.headerIeInfo[0].elementId = MAC_HEADER_IE_WISUN;
      pEvent->tx.internal.headerIeInfo[0].subIdBitMap = MAC_FH_UT_IE;

      pEvent->tx.internal.payloadIeInfo[0].gId = MAC_PAYLOAD_IE_WISUN_GID;
      pEvent->tx.internal.payloadIeInfo[0].subId.subIdBMap = MAC_FH_US_IE |
                                                             MAC_FH_PAN_IE |
                                                             MAC_FH_NETNAME_IE;

      pEvent->tx.internal.fhFrameType = MAC_FH_FRAME_PAN_ADVERT;

      status = MAP_macBuildWSAsyncMsg( pEvent );

      if ( status == MAC_SUCCESS)
      {
        /* queue frame for transmission */
        MAP_macDataTxEnqueue(&pEvent->tx);
      }
    }
    return status;

    case MAC_WS_ASYNC_PAN_ADVERT_SOL:
    {
      pEvent->tx.internal.headerIeInfo[0].elementId = MAC_HEADER_IE_WISUN;
      pEvent->tx.internal.headerIeInfo[0].subIdBitMap = MAC_FH_UT_IE;

      pEvent->tx.internal.payloadIeInfo[0].gId = MAC_PAYLOAD_IE_WISUN_GID;
      pEvent->tx.internal.payloadIeInfo[0].subId.subIdBMap = MAC_FH_US_IE |
                                                             MAC_FH_NETNAME_IE;

      pEvent->tx.internal.fhFrameType = MAC_FH_FRAME_PAN_ADVERT_SOLICIT;

      status = MAP_macBuildWSAsyncMsg( pEvent );

      if ( status == MAC_SUCCESS)
      {
        /* queue frame for transmission */
        MAP_macDataTxEnqueue(&pEvent->tx);
      }
    }
    return status;

    case MAC_WS_ASYNC_PAN_CONFIG:
    {
      pEvent->tx.internal.headerIeInfo[0].elementId = MAC_HEADER_IE_WISUN;
      pEvent->tx.internal.headerIeInfo[0].subIdBitMap = MAC_FH_UT_IE |
                                                        MAC_FH_BT_IE;

      pEvent->tx.internal.payloadIeInfo[0].gId = MAC_PAYLOAD_IE_WISUN_GID;
      pEvent->tx.internal.payloadIeInfo[0].subId.subIdBMap = MAC_FH_US_IE |
                                                             MAC_FH_BS_IE |
                                                             MAC_FH_PANVER_IE |
                                                             MAC_FH_GTKHASH_IE;

      pEvent->tx.internal.fhFrameType = MAC_FH_FRAME_PAN_CONFIG;

      status = MAP_macBuildWSAsyncMsg( pEvent );

      if ( status == MAC_SUCCESS)
      {
        /* queue frame for transmission */
        MAP_macDataTxEnqueue(&pEvent->tx);
      }
    }
    return status;

    case MAC_WS_ASYNC_PAN_CONFIG_SOL:
    {
      pEvent->tx.internal.headerIeInfo[0].elementId = MAC_HEADER_IE_WISUN;
      pEvent->tx.internal.headerIeInfo[0].subIdBitMap = MAC_FH_UT_IE;

      pEvent->tx.internal.payloadIeInfo[0].gId = MAC_PAYLOAD_IE_WISUN_GID;
      pEvent->tx.internal.payloadIeInfo[0].subId.subIdBMap = MAC_FH_US_IE |
                                                             MAC_FH_NETNAME_IE;

      pEvent->tx.internal.fhFrameType = MAC_FH_FRAME_PAN_CONFIG_SOLICIT;

      status = MAP_macBuildWSAsyncMsg( pEvent );

      if ( status == MAC_SUCCESS)
      {
        /* queue frame for transmission */
        MAP_macDataTxEnqueue(&pEvent->tx);
      }
    }
    return status;

    default:
      return MAC_BAD_STATE;
  }
}

/**
 * @brief Handles the WiSUN async request.
 *
 * @param pEvent - Pointer to the event data.
 *
 * @return none
 */
MAC_INTERNAL_API void macApiWSAsyncReq(macEvent_t *pEvent)
{
  uint8 status = MAC_SUCCESS;

  if (MAC_WS_OPER_ASYNC_START == pEvent->apiData.mac.asyncReq.operation)
  {
    status = MAP_macBuildWSAsyncReq(pEvent);
    if (MAC_SUCCESS != status )
    {
      MAP_macWSAsyncCnf(status);
    }
  }
  else if ( MAC_WS_OPER_ASYNC_STOP == pEvent->apiData.mac.asyncReq.operation )
  {
    if ( FHAPI_STATUS_SUCCESS == MAC_FH_STOP_ASYNC_FN())
    {
      MAP_macWSAsyncCnf(MAC_SUCCESS);
    }
    else
    {
      MAP_macWSAsyncCnf(MAC_BAD_STATE);
    }
  }

  return;
}
#endif /* FEATURE_FREQ_HOP_MODE */

/**************************************************************************************************
 * @fn          macGetCSMPhy
 *
 * @brief       Provides the corresponding CSM Phy Id to the one provided
 *              through the currPhyId parameter.
 *
 * @param currPhyId Current Phy Id.
 *
 * @return returns the corresponding CSM phy Id.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macGetCSMPhy( uint8 currPhyId )
{
  uint8 phyId;
  switch ( currPhyId )
  {
    case MAC_150KBPS_915MHZ_PHY_2:
      /* CSM mode PHY for US 915 */
      phyId = MAC_50KBPS_915MHZ_PHY_1;
      break;
      // ToDo: Add case statement for new PHY's (which are not by default CSM).
    default:
      phyId = currPhyId;
      break;
  }
  return phyId;
}

/**************************************************************************************************
 * @fn          MAC_RandomByte
 *
 * @brief       This function returns a random byte
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      1 random byte value
 **************************************************************************************************
 */
uint8 MAC_RandomByte(void)
{
  return (MAP_macRadioRandomByte());
}

///**************************************************************************************************
// * @fn          macCheckPhyLRMMode
// *
// * @brief       This function returns 1 if PHY is LRM mode, else 0
// *
// * input parameters
// *
// * None.
// *
// * output parameters
// *
// * None.
// *
// * @return      PHY is LRM mode or not
// **************************************************************************************************
// */
//bool macCheckPhyLRMMode(void)
//{
//    /*If not LRM, return 0 */
//    if ((pMacPib->curPhyID >= MAC_5KBPS_915MHZ_PHY_129)
//     && (pMacPib->curPhyID <= MAC_5KBPS_868MHZ_PHY_131))
//    {
//        return 1;
//    }
//    return 0;
//}
//
//bool macCheckPhyFSKMode(void)
//{
//    /*If not LRM, return 0 */
//    if ((pMacPib->curPhyID >= MAC_50KBPS_915MHZ_PHY_1 )
//     && (pMacPib->curPhyID <= MAC_50KBPS_868MHZ_PHY_3))
//    {
//        return 1;
//    }
//    return 0;
//}


#if 0
// ToDo: Move to a different file
// Temporary home for the below functions
#include "api_mac.h"
/*! Capability Information - Device is capable of becoming a PAN coordinator */
#define CAPABLE_PAN_COORD       0x01
/*! Capability Information - Device is an FFD  */
#define CAPABLE_FFD             0x02
/*!
 Capability Information - Device is mains powered rather than battery powered
 */
#define CAPABLE_MAINS_POWER     0x04
/*! Capability Information - Device has its receiver on when idle  */
#define CAPABLE_RX_ON_IDLE      0x08
/*!
 Capability Information - Device is capable of sending
 and receiving secured frames
 */
#define CAPABLE_SECURITY        0x40
/*!
 Capability Information - Request allocation of a short address in the
 associate procedure
 */
#define CAPABLE_ALLOC_ADDR      0x80


uint8_t ApiMac_convertCapabilityInfo(ApiMac_capabilityInfo_t *pMsgcapInfo)
{
    uint8 capInfo = 0;

    if(pMsgcapInfo->panCoord)
    {
        capInfo |= CAPABLE_PAN_COORD;
    }

    if(pMsgcapInfo->ffd)
    {
        capInfo |= CAPABLE_FFD;
    }

    if(pMsgcapInfo->mainsPower)
    {
        capInfo |= CAPABLE_MAINS_POWER;
    }

    if(pMsgcapInfo->rxOnWhenIdle)
    {
        capInfo |= CAPABLE_RX_ON_IDLE;
    }

    if(pMsgcapInfo->security)
    {
        capInfo |= CAPABLE_SECURITY;
    }

    if(pMsgcapInfo->allocAddr)
    {
        capInfo |= CAPABLE_ALLOC_ADDR;
    }

    return (capInfo);
}

uint16_t convertTxOptions(ApiMac_txOptions_t txOptions)
{
    uint16_t retVal = 0;

    if(txOptions.ack == true)
    {
        retVal |= MAC_TXOPTION_ACK;
    }
    if(txOptions.indirect == true)
    {
        retVal |= MAC_TXOPTION_INDIRECT;
    }
    if(txOptions.pendingBit == true)
    {
        retVal |= MAC_TXOPTION_PEND_BIT;
    }
    if(txOptions.noRetransmits == true)
    {
        retVal |= MAC_TXOPTION_NO_RETRANS;
    }
    if(txOptions.noConfirm == true)
    {
        retVal |= MAC_TXOPTION_NO_CNF;
    }
    if(txOptions.useAltBE == true)
    {
        retVal |= MAC_TXOPTION_ALT_BE;
    }
    if(txOptions.usePowerAndChannel == true)
    {
        retVal |= MAC_TXOPTION_PWR_CHAN;
    }

    return (retVal);
}

void copyApiMacAddrToMacAddr(sAddr_t *pDst, ApiMac_sAddr_t *pSrc)
{
    /* Copy each element of the structure */
    pDst->addrMode = pSrc->addrMode;
    if(pSrc->addrMode == ApiMac_addrType_short)
    {
        pDst->addr.shortAddr = pSrc->addr.shortAddr;
    }
    else
    {
        MAP_osal_memcpy(pDst->addr.extAddr, pSrc->addr.extAddr, sizeof(sAddrExt_t));
    }
}

void copyApiMacSecToMacSec(ApiMac_sec_t *pDst, ApiMac_sec_t *pSrc)
{
    /* Copy each element of the structure */
    MAP_osal_memcpy(pDst->keySource, pSrc->keySource, APIMAC_KEY_SOURCE_MAX_LEN);
    pDst->securityLevel = pSrc->securityLevel;
    pDst->keyIdMode = pSrc->keyIdMode;
    pDst->keyIndex = pSrc->keyIndex;
}
#endif //if 0
