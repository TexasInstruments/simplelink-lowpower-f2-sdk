/******************************************************************************

 @file  mac_ie_build.c

 @brief This file implements building of MAC header and payload IEs.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2015-2023, Texas Instruments Incorporated

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

/*-------------------------------------------------------------------------------------------------
*           Include
*--------------------------------------------------------------------------------------------------
*/

#ifndef LIBRARY
#include "ti_wisunfan_features.h"
#endif

#include "mac_ie_build.h"
#include "mac_ie_parse.h"

#include "mac_mgmt.h"
#include "mac_spec.h"
#include "mac_pib.h"

#include "rom_jt_154.h"
/*-------------------------------------------------------------------------------------------------
*           Macros
*--------------------------------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------------------------------------
 *           Functions
 *--------------------------------------------------------------------------------------------------
 */

#if !defined(TIMAC_ROM_PATCH)
/**
 * @brief This function builds and inserts header ies in the
 *        transmit buffer.
 *
 * @param pBuf           pointer to the structure containing the
 *                       transmit message buffer.
 * @param pHeaderIes     pointer to the list of header ie's that
 *                       needs to be built and inserted in the
 *                       transmit buffer.
 *
 * @return uint8         returns MAC_SUCCESS if successful.
 *                       MAC_NO_DATA if it fails to build the
 *                       requested IE.
 */
MAC_INTERNAL_API uint8
macBuildHeaderIes(macTx_t* pBuf, headerIeInfo_t *pHeaderIes )
{
  uint8 i;
  uint8 status = MAC_SUCCESS;

  MAC_PARAM_STATUS(pHeaderIes, status);
  MAC_PARAM_STATUS(pBuf, status);

  if ( MAC_SUCCESS != status )
  {
    return status;
  }

  /**
   * Start backward, with the header termination IE.
   */
  *(--pBuf->msdu.p) = MAC_HEADER_IE_TERM_SECOND_BYTE;

  if ( pBuf->internal.ptrPayloadIE )
  {
    *(--pBuf->msdu.p) = MAC_HEADER_IE_TERM_FIRST_BYTE_PAYIE;
  }
  else
  {
    *(--pBuf->msdu.p) = MAC_HEADER_IE_TERM_FIRST_BYTE_NO_PAYIE;
  }


  pBuf->msdu.len += 2;
  pBuf->internal.headerIesLen = 2;

  for (i = 0; i < MAC_HEADER_IE_MAX; i++ )
  {
    if ( MAC_HEADER_IE_TERM == pHeaderIes[i].elementId )
    {
      break;
    }

    /**
     * For now we have only WiSUN header IE. As and when we have
     * more IE's, build support to insert it, here.
     */

    if ( (MAC_HEADER_IE_WISUN == pHeaderIes[i].elementId) && (pHeaderIes[i].subIdBitMap) )
    {
      uint16 ieLen;

      /* Get the WiSUN Sub IE's len */
      ieLen = MAC_FH_GET_IE_LEN_FN( pHeaderIes[i].subIdBitMap );

      pBuf->msdu.p = pBuf->msdu.p - ieLen;

      //ToDo: Check to make sure we do not cross the buffer boundary.

      /* Insert the WiSUN Sub IE's */
      if ( pHeaderIes[i].subIdBitMap && MAC_FH_RSL_IE )
      {
          FHIE_ieInfo_t rslVal;

          //rslVal.rslIe.rsl = 0x99; /* for debug */
          rslVal.rslIe.rsl = rxFhRsl;
          if ( 0 == MAC_FH_GENERATE_IE_FN(pBuf->msdu.p, pHeaderIes[i].subIdBitMap, pBuf, &rslVal))
          {
            status = MAC_NO_DATA;
            return status;
          }
      }
      else
      {
          if ( 0 == MAC_FH_GENERATE_IE_FN(pBuf->msdu.p, pHeaderIes[i].subIdBitMap, pBuf, NULL))
          {
            status = MAC_NO_DATA;
            return status;
          }
      }
      pBuf->msdu.len += ieLen;
      pBuf->internal.headerIesLen += ieLen;
    }
  }

  pBuf->internal.ptrHeaderIE = pBuf->msdu.p;
  return status;
}
#endif

/**
 * @brief Builds the MLME payload co-existence sub IE.
 *
 * @param pBuf      pointer to the structure containing the transmitpBuf
 *                  message buffer.
 * @param ieIdOnly  if TRUE no IE content will be populated and the length
 *                  field will be set to zero.
 *
 * @return uint16 returns the length of the IE if successful,
 *         otherwise zero.
 */
uint16 macBuildPayloadCoexistIe( macTx_t* pBuf, bool ieIdOnly )
{
  if ( FALSE == ieIdOnly )
  {
    uint8 finalCapSlot;

    if ( (MAC_PAYLOAD_MLME_SUB_IE_COEXIST_LEN + MAC_PAYLOAD_SUB_IE_HEADER_LEN +
          2 * MAC_PAYLOAD_IE_HEADER_LEN + pBuf->msdu.len) >
         pBuf->internal.totalLen )
    {
      return 0;
    }

    /**
     * +-------+-------+-------+-------+-------+-------+-------+
     * |Bits0-3| 4-7   | 8-11  | 12-15 | 16-19 | 20-23 | 24-39 |
     * +-------+-------+-------+-------+-------+-------+-------+
     * |Beacon | Super | Final |E.Becon| Offset| CAP   |NBPAN  |
     * | Order | Order | C.slot| Order |T. slot| Backof|E. ordr|
     * +-------+-------+-------+-------+-------+-------+-------+
     */
    /* Working backwards */
    *(--pBuf->msdu.p) = HI_UINT16(pMacPib->eBeaconOrderNBPAN);
    *(--pBuf->msdu.p) = LO_UINT16(pMacPib->eBeaconOrderNBPAN);

    //ToDo: The Cap Backoff offset needs to be filled in by low level MAC,
    //      May need the location of the field.
    *(--pBuf->msdu.p) = 0;

    /* time offset */
    *(pBuf->msdu.p) = (pMacPib->offsetTimeSlot << MAC_COEXIST_IE_OFFSET_TIME_SLOT_POS);

    /* get final cap slot */
    finalCapSlot = MAC_A_NUM_SUPERFRAME_SLOTS - 1;

    *(--pBuf->msdu.p) = (finalCapSlot << MAC_COEXIST_IE_FINAL_CAP_SLOT_POS) |
             (pMacPib->eBeaconOrder << MAC_COEXIST_IE_ENHANCED_BEACON_ORDER_POS);

    *(--pBuf->msdu.p) = (pMacPib->beaconOrder << MAC_COEXIST_IE_BEACON_ORDER_POS) |
              (pMacPib->superframeOrder << MAC_COEXIST_IE_SUPERFRAME_ORDER_POS);

    *(--pBuf->msdu.p) = MAC_PAYLOAD_MLME_SUB_IE_COEXIST;

    *(--pBuf->msdu.p) = MAC_PAYLOAD_MLME_SUB_IE_COEXIST_LEN;

    return (MAC_PAYLOAD_MLME_SUB_IE_COEXIST_LEN +
            MAC_PAYLOAD_SUB_IE_HEADER_LEN);
  }
  else
  {
    if ( ( MAC_PAYLOAD_SUB_IE_HEADER_LEN + 2 * MAC_PAYLOAD_IE_HEADER_LEN + pBuf->msdu.len) >
         pBuf->internal.totalLen )
    {
      return 0;
    }

    *(--pBuf->msdu.p) = MAC_PAYLOAD_MLME_SUB_IE_COEXIST;

    /* zero length for ieIdOnly */
    *(--pBuf->msdu.p) = 0;

    return (MAC_PAYLOAD_SUB_IE_HEADER_LEN);
  }
}

/**
 * @brief This function builds and inserts payload ies in the
 *        transmit buffer.
 *
 * @param pBuf           pointer to the structure containing the
 *                       transmit message buffer.
 * @param pPayloadIeInfo pointer to the list of payload ie's
 *                       that needs to be built and inserted in
 *                       the transmit buffer.
 * @param ieIdOnly       if TRUE no IE content will be populated and the length
 *                       field will be set to zero.
 *
 * @return uint8         returns MAC_SUCCESS if successful.
 *                       MAC_NO_RESOURCES if it fails to build the
 *                       requested IE.
 */
MAC_INTERNAL_API uint8
macBuildPayloadIes( macTx_t* pBuf, payloadIeInfo_t *pPayloadIeInfo, bool includeTermIe, bool ieIdOnly )
{
  uint8 status = MAC_SUCCESS;
  uint8 i, j;

  MAC_PARAM_STATUS( (NULL != pBuf), status );
  MAC_PARAM_STATUS( (NULL != pPayloadIeInfo), status );

  if ( MAC_SUCCESS != status )
  {
    return status;
  }

  if (includeTermIe)
  {
    /**
     * Fill the Payload termination IE.
     */
    *(--pBuf->msdu.p) = MAC_PAYLOAD_IE_TERM_SECOND_BYTE_PAYIE;
    *(--pBuf->msdu.p) = MAC_PAYLOAD_IE_TERM_FIRST_BYTE;

    pBuf->msdu.len += MAC_PAYLOAD_IE_HEADER_LEN;
  }

  for ( i = 0;
        (i < MAC_PAYLOAD_IE_MAX) && (MAC_PAYLOAD_IE_TERM_GID != (pPayloadIeInfo[i].gId & 0x0F));
        i++ )
  {
    if ( MAC_PAYLOAD_IE_MLME_GID == pPayloadIeInfo[i].gId )
    {
      uint16 mlmeContentLen = 0;
      /* Check if MLME has sub id's*/
      for (j = 0; j < MAC_PAYLOAD_SUB_IE_MAX; j++)
      {
        if ( MAC_PAYLOAD_SUB_IE_TERM == pPayloadIeInfo[i].subId.id[j])
        {
          break;
        }

        switch (pPayloadIeInfo[i].subId.id[j] )
        {
          case MAC_PAYLOAD_MLME_SUB_IE_COEXIST:
          {
            uint16 len = MAP_macBuildPayloadCoexistIe( pBuf, ieIdOnly );
            if (0 == len)
            {
              status = MAC_NO_RESOURCES;
              return status;
            }
            mlmeContentLen += len;
          }
          break;
        }
      }

      /* Fill the MLME header */
      *(--pBuf->msdu.p) = (HI_UINT16(mlmeContentLen) & 0x03) |
                          (((uint8)MAC_PAYLOAD_IE_MLME_GID << MAC_PAYLOAD_GID_POS) & 0x78) |
                          (((uint8)MAC_PAYLOAD_IE_TYPE_VAL << MAC_PAYLOAD_TYPE_POS) & 0x80) ;
      *(--pBuf->msdu.p) = LO_UINT16(mlmeContentLen);

      pBuf->msdu.len += mlmeContentLen + MAC_PAYLOAD_IE_HEADER_LEN;
    }
    else if ( MAC_PAYLOAD_IE_WISUN_GID == pPayloadIeInfo[i].gId )
    {
      int len = 0;
      if (MAC_FH_ENABLED)
      {
        len = MAC_FH_GET_IE_LEN_FN( pPayloadIeInfo[i].subId.subIdBMap);
        if (len &&
            ((len + pBuf->msdu.len + MAC_PAYLOAD_IE_HEADER_LEN) <= pBuf->internal.totalLen) )
        {
          uint8* ptr = pBuf->msdu.p - len;

          if ( 0 == MAC_FH_GENERATE_IE_FN(ptr, pPayloadIeInfo[i].subId.subIdBMap, pBuf, NULL) )
          {
            status = MAC_NO_RESOURCES;
            return status;
          }

          pBuf->msdu.p -= len;
          pBuf->msdu.len += len;
        }
        else
        {
          status = MAC_NO_RESOURCES;
          return status;
        }
      }
    }
  }

  pBuf->internal.ptrPayloadIE = pBuf->msdu.p;
  return status;
}

