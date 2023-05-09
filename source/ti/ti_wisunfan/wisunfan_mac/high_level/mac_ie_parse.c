/******************************************************************************

 @file  mac_ie_parse.c

 @brief This file implements parsing of MAC header and payload IE.

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

#include "mac_ie_parse.h"
#include "mac_main.h"
#include "mac_assert.h"
#include "mac_spec.h"

/******************************************************************************
  ROM jump table will support
  1. TIMAC ROM image build
  2. TIMAC stack build without ROM image
  3. TIMAC stack with calling ROM image
 *****************************************************************************/
#include "rom_jt_154.h"

/*-------------------------------------------------------------------------------------------------
*           Functions
*--------------------------------------------------------------------------------------------------
*/

#if !defined(TIMAC_ROM_PATCH)
/**
 * @brief Updates the header IE pointer if header IE is present in macRx_t
 *        structure with the start position of header IE.
 *        It also updates the headerIeLen fields.
 *
 * @param pMsg pointer to the macRx_t containing the incoming decrypted message.
 * @return None
 */
void
macUpdateHeaderIEPtr(macRx_t *pMsg)
{
  uint8* ptr;
  uint8 ieContentLen,foundTermIE=0;
  uint16  ieLen = 0;

  MAC_ASSERT( NULL != pMsg );

  if ( pMsg->internal.fcf & MAC_FCF_IE_PRESENT_MASK )
  {
    ptr = (uint8 *)pMsg->msdu.p;

    if (MAC_HEADER_IE_TYPE(ptr) == MAC_HEADER_IE_TYPE_VAL )
    {
      /* header IE present, update the header ie pointer */
      pMsg->pHdrIes = ptr;

      /* cycle through header ie's until we find the termination ie */
      do
      {
        ieContentLen = 0;
        if ( MAC_INTERNAL_HEADER_IE_TERM_WITH_PAYIE == MAC_HEADER_IE_ELEMENT_ID(ptr) )
        {
          /* Termination IE followed by payload ie's, set the payload IE flag */
          pMsg->payloadIePresent = 1;
          /* Update total header ie length */
          ieLen += MAC_HEADER_IE_HEADER_LEN;
          foundTermIE =1;
          break;
        }
        else if ( MAC_INTERNAL_HEADER_IE_TERM_NO_PAYIE == MAC_HEADER_IE_ELEMENT_ID(ptr) )
        {
          /* Termination IE */
          pMsg->payloadIePresent = 0;
          /* Update total header ie length */
          ieLen += MAC_HEADER_IE_HEADER_LEN;
          foundTermIE =1;
          break;
        }

        /* Get Header IE content len */
        ieContentLen = MAC_HEADER_IE_CONTENT_LEN(ptr);
        /* Update length and pointer */
        ieLen += MAC_HEADER_IE_HEADER_LEN + ieContentLen;
        ptr += MAC_HEADER_IE_HEADER_LEN + ieContentLen;

      } while(ieLen < pMsg->msdu.len );

      if (foundTermIE && (ieLen <= pMsg->msdu.len) )
      {
        pMsg->headerIeLen = ieLen;
        /* update msdu */
        pMsg->msdu.p += ieLen;
        pMsg->msdu.len -= ieLen;
      }
      else
      { /* HIE is invalid, update IE length to zero */
        pMsg->headerIeLen = 0;
      }
    }
  }
}


/**
 * @brief Updates the payload IE pointer if payload IE's are present in macRx_t
 *        structure with the start position of payload IE in the buffer.
 *        It also updates the payloadIeLen fields.
 *        Call this function only if the payload IE to precess exists.
 *
 * @param pMsg pointer to the macRx_t containing the incoming decrypted message.
 * @return None
 */

void macUpdatePayloadIEPtr(macRx_t *pMsg)
{
  uint8* ptr;
  uint8 ieContentLen;
  uint16  ieLen = 0;

  MAC_ASSERT( NULL != pMsg );

  ptr = (uint8 *)pMsg->msdu.p;
  ieLen = 0;

  if (MAC_PAYLOAD_IE_TYPE(ptr) == MAC_PAYLOAD_IE_TYPE_VAL )
  {
    pMsg->mac.pPayloadIE = ptr;

    do
    {
      ieContentLen = 0;
      if ( MAC_PAYLOAD_IE_TERM_GID == MAC_PAYLOAD_IE_GROUP_ID(ptr) )
      {
        /* Termination IE found */
        ieLen += MAC_PAYLOAD_IE_HEADER_LEN;

        break;
      }

      /* Get Payload IE information */
      ieContentLen = MAC_PAYLOAD_IE_CONTENT_LEN(ptr);
      /* Update length and pointer */
      ieLen += MAC_PAYLOAD_IE_HEADER_LEN + ieContentLen;
      ptr += MAC_PAYLOAD_IE_HEADER_LEN + ieContentLen;

    } while(ieLen < pMsg->msdu.len );

    /* sanity checking */
    if (ieLen > pMsg->msdu.len)
    { /* we should drop this packet */
      pMsg->mac.payloadIeLen = 0;
    }
    else
    {
      /* update the payload ie len */
      pMsg->mac.payloadIeLen = ieLen;

      /* update msdu */
      pMsg->msdu.p += ieLen;
      pMsg->msdu.len -= ieLen;
    }
  }
}
#endif

/**
 * @brief Parses the header information element.
 *        This function creates the link list of the header IEs.
 *        The caller is responsible to release the memory for
 *        the same. It also updates the payload pointer and the
 *        length.
 *
 * @param pMsg pointer to the macRx_t structure containing the
 *             payload data.
 *
 * @return returns MAC_SUCCESS if successful in parsing the
 *         payload ie's. MAC_NO_DATA if it does not find any
 *         payload ie's. MAC_NO_RESOURCES if memory allocation
 *         fails.
 */
uint8
macParseHeaderIes(macRx_t *pMsg)
{
  uint8* ptr;
  macHeaderIeInfo_t* pHeaderIe = NULL;
  macHeaderIeInfo_t* pTempHeaderIe;
  uint16  lenHeaderIe = 0;
  uint8 status = MAC_SUCCESS;

  MAC_ASSERT( NULL != pMsg );

  ptr = (uint8*)pMsg->msdu.p;

  if (MAC_HEADER_IE_TYPE(ptr) != MAC_HEADER_IE_TYPE_VAL )
  {
    /* no header IE present */
    status = MAC_NO_DATA;
    return status;
  }

  do
  {
    if ( MAC_INTERNAL_HEADER_IE_TERM_WITH_PAYIE == MAC_HEADER_IE_ELEMENT_ID(ptr) )
    {
      /* Termination IE followed by payload ie's, set the payload IE flag */
      pMsg->payloadIePresent = 1;
      /* Update total header ie length */
      lenHeaderIe += MAC_HEADER_IE_HEADER_LEN;
      break;
    }
    else if ( MAC_INTERNAL_HEADER_IE_TERM_NO_PAYIE == MAC_HEADER_IE_ELEMENT_ID(ptr) )
    {
      /* Termination IE */
      /* Update total header ie length */
      lenHeaderIe += MAC_HEADER_IE_HEADER_LEN;
      break;
    }

    pTempHeaderIe = (macHeaderIeInfo_t *) MAP_osal_mem_alloc(sizeof(macHeaderIeInfo_t));

    if ( NULL == pTempHeaderIe )
    {
      status = MAC_NO_RESOURCES;
      break;
    }

    /* If nothing in the list, add the node first otherwise
       add it to the end of the list */
    if ( NULL == pMsg->pHeaderIes  )
    {
      pMsg->pHeaderIes  = pTempHeaderIe;
    }
    else
    {
      /* Make sure pHeaderIe is initialized */
      MAC_ASSERT(pHeaderIe != NULL);
      /* pHeaderIe should point to the previous node,
         since it was allocated in the previous iteration */
      pHeaderIe->pNext = pTempHeaderIe;
    }

    pHeaderIe = pTempHeaderIe;
    pTempHeaderIe = NULL;

    MAP_osal_memset( pHeaderIe, 0x0, sizeof(macHeaderIeInfo_t));

    /* Get Header IE information */
    pHeaderIe->ieType = MAC_HEADER_IE_TYPE(ptr);
    pHeaderIe->ieContentLen = MAC_HEADER_IE_CONTENT_LEN(ptr);
    pHeaderIe->ieElementId = MAC_HEADER_IE_ELEMENT_ID(ptr);
    pHeaderIe->ieContentPtr = ptr + MAC_HEADER_IE_HEADER_LEN;
    /* Update length and pointer */
    lenHeaderIe += MAC_HEADER_IE_HEADER_LEN + pHeaderIe->ieContentLen;
    ptr += MAC_HEADER_IE_HEADER_LEN + pHeaderIe->ieContentLen;

  } while(lenHeaderIe < pMsg->headerIeLen );


  if ( (MAC_NO_RESOURCES == status) && (NULL != pMsg->pHeaderIes ) )
  {
    /* not successful in parsing all header ie's, free the linked list */
    pHeaderIe = pMsg->pHeaderIes ;
    while (NULL != pHeaderIe )
    {
      pTempHeaderIe = pHeaderIe->pNext;
      MAP_osal_mem_free(pHeaderIe);
      pHeaderIe = pTempHeaderIe;
    }
    pMsg->pHeaderIes  = NULL;
  }
  else
  {
    /* update headerIeLen */
    pMsg->headerIeLen = lenHeaderIe;
  }

  return status;
}

/**
 * @brief Parses the payload information element.
 *        This function creates the link list of the Payload IE.
 *        The caller is responsible to release the memory for
 *        the same. It also updates the payload pointer and the
 *        length.
 *
 * @param pMsg pointer to the macRx_t structure containing the
 *             payload data.
 *
 * @return returns MAC_SUCCESS if successful in parsing the
 *         payload ie's. MAC_NO_DATA if it does not find any
 *         payload ie's. MAC_NO_RESOURCES if memory allocation
 *         fails.
 */
uint8
macParsePayloadIes( macRx_t *pMsg )
{
  uint8* payloadIePtr;
  macPayloadIeInfo_t* pPayloadIe = NULL;
  macPayloadIeInfo_t* pTempPayloadIe;
  uint16 lenPayloadIes = 0;
  uint8 status = MAC_SUCCESS;

  payloadIePtr = (uint8 *)pMsg->mac.pPayloadIE;

  if (MAC_PAYLOAD_IE_TYPE(payloadIePtr) != MAC_PAYLOAD_IE_TYPE_VAL )
  {
    /* no payload IE present */
    status = MAC_NO_DATA;
    return status;
  }

  do
  {
    if ( MAC_PAYLOAD_IE_TERM_GID ==
        MAC_PAYLOAD_IE_GROUP_ID(payloadIePtr) )
    {
      /* Termination IE found */
      lenPayloadIes += MAC_PAYLOAD_IE_HEADER_LEN;
      break;
    }

    pTempPayloadIe = (macPayloadIeInfo_t *)MAP_osal_mem_alloc( sizeof(macPayloadIeInfo_t) );

    if ( NULL == pTempPayloadIe )
    {
      status = MAC_NO_RESOURCES;
      break;
    }

    /* If nothing in the list, add the node first otherwise
       add it to the end of the list */
    if ( NULL == pMsg->pPayloadIes )
    {
      pMsg->pPayloadIes = pTempPayloadIe;
    }
    else
    {
      /* Make sure pPayloadIe is initialized */
      MAC_ASSERT(pPayloadIe != NULL);    
      /* pPayloadIe should point to the previous node,
         since it was allocated in the previous iteration */
      pPayloadIe->pNext = pTempPayloadIe;
    }

    pPayloadIe = pTempPayloadIe;
    pTempPayloadIe = NULL;

    MAP_osal_memset( pPayloadIe, 0x0, sizeof(macPayloadIeInfo_t) );

    /* Get Payload IE information */
    pPayloadIe->ieType = MAC_PAYLOAD_IE_TYPE(payloadIePtr);
    pPayloadIe->ieContentLen = MAC_PAYLOAD_IE_CONTENT_LEN(payloadIePtr);
    pPayloadIe->ieGroupId = MAC_PAYLOAD_IE_GROUP_ID(payloadIePtr);

    if ( pPayloadIe->ieContentLen )
    {
      pPayloadIe->ieContentPtr = payloadIePtr + MAC_PAYLOAD_IE_HEADER_LEN;
    }
    else
    {
      pPayloadIe->ieContentPtr = NULL;
    }

    /* Update length and pointer */
    lenPayloadIes += MAC_PAYLOAD_IE_HEADER_LEN + pPayloadIe->ieContentLen;
    payloadIePtr += MAC_PAYLOAD_IE_HEADER_LEN + pPayloadIe->ieContentLen;

  } while( lenPayloadIes < pMsg->mac.payloadIeLen );

  if ( (MAC_NO_RESOURCES == status) && (NULL != pMsg->pPayloadIes) )
  {
    /* not successful in parsing all header ie's, free the linked list */
    pPayloadIe = pMsg->pPayloadIes;
    while (NULL != pPayloadIe )
    {
      pTempPayloadIe = pPayloadIe->pNext;
      MAP_osal_mem_free(pPayloadIe);
      pPayloadIe = pTempPayloadIe;
    }
    pMsg->pPayloadIes = NULL;
  }

  return status;
}

/**
 * @brief Frees the memory associated with each element in the link list
 * pointed by the pList
 *
 * @param pList pointer to the payload IE information element.
 *
 * @return None
 */
void macIeFreeList(macPayloadIeInfo_t* pList)
{
  /* Loop through the list */
  while(pList)
  {
    macPayloadIeInfo_t *pTemp = pList;

    /* Move to the next item in the list */
    pList = pTemp->pNext;

    /* free the current item */
    MAP_osal_mem_free( pTemp );
  }
}

/**
 * @brief Sets the value of the location pointed by the pIePresent parameter
 *        to 1 and returns the pointer to the start of the coexistence IE content
 *        if the coexistence is found in the list pointed by pPayloadIE param.
 *
 * @param pPayloadIe pointer to the payload IE list.
 * @param pIePresent pointer to the location of the iePresent field.
 *
 * @return uint8* pointer to the start of coexistence IE content if available
 *                otherwise NULL.
 */
uint8* macIeCoexistIePresent( macPayloadIeInfo_t* pPayloadIe, uint8* pIePresent )
{
  uint8* pCoexistIeContent = NULL;

  *pIePresent = 0;

  while ( pPayloadIe && (NULL == pCoexistIeContent) )
  {
    /* Check for Payload IE of type MLME */
    if (MAC_PAYLOAD_IE_MLME_GID == pPayloadIe->ieType)
    {
      uint16 mlmeIeLen = pPayloadIe->ieContentLen;
      uint8* ptr = pPayloadIe->ieContentPtr;
      uint8 ieTypeLong = 0;

      /* Find Coexistence Sub IE if exist */
      while (mlmeIeLen > 0)
      {
        uint16 len = 0;
        ieTypeLong = MAC_PAYLOAD_IE_TYPE(ptr);

        if (ieTypeLong)
        {
          len = MAC_PAYLOAD_LONG_SUBIE_LEN(ptr);
        }
        else
        {
          len = MAC_PAYLOAD_SHORT_SUBIE_LEN(ptr);
          if ( MAC_PAYLOAD_MLME_SUB_IE_COEXIST == MAC_PAYLOAD_SHORT_SUBIE_ID(ptr) )
          {
            if (len)
            {
              pCoexistIeContent = ptr + MAC_PAYLOAD_SUB_IE_HEADER_LEN;
            }
            *pIePresent = 1;
            break;
          }
        }
        mlmeIeLen -= len + MAC_PAYLOAD_SUB_IE_HEADER_LEN;
      }
    }
    pPayloadIe = pPayloadIe->pNext;
  }

  return pCoexistIeContent;
}

/**
 * @brief Cycles through the MLME IE content to find the Sub-IE indicated by the
 *        param mlmeSubIeType. If found set the value of the field pointed by
 *        pIePresent value to 1 and if the content of the IE is present returns
 *        the pointer to the content.
 *
 * @param pPayloadIe    pointer to the buffer containing the MLME IE content.
 * @param mlmeSubIeType Sub-IE to find within the MLME IE content.
 * @param pIePresent    pointer to the IE present field which is updated by this
 *                      function.
 *
 * @return uint8*       pointer to the Sub-IE content if present otherwise NULL.
 */
uint8* macMlmeSubIePresent( macPayloadIeInfo_t* pPayloadIe, uint8 mlmeSubIeType,
                            uint8* pIePresent )
{
  uint8* pIeContent = NULL;

  *pIePresent = 0;

  while ( pPayloadIe && (0 == *pIePresent) )
  {
    /* Check for Payload IE of type MLME */
    if (MAC_PAYLOAD_IE_MLME_GID == pPayloadIe->ieType)
    {
      uint16 mlmeIeLen = pPayloadIe->ieContentLen;
      uint8* ptr = pPayloadIe->ieContentPtr;
      uint8 ieTypeLong = 0;

      /* Find the requested Sub IE if it exist */
      while (mlmeIeLen > 0)
      {
        uint16 len = 0;
        ieTypeLong = MAC_PAYLOAD_IE_TYPE(ptr);

        if (ieTypeLong)
        {
          len = MAC_PAYLOAD_LONG_SUBIE_LEN(ptr);
        }
        else
        {
          len = MAC_PAYLOAD_SHORT_SUBIE_LEN(ptr);
          if ( mlmeSubIeType == MAC_PAYLOAD_SHORT_SUBIE_ID(ptr) )
          {
            if (len)
            {
              pIeContent = ptr + MAC_PAYLOAD_SUB_IE_HEADER_LEN;
            }

            *pIePresent = 1;
            break;
          }
        }
        mlmeIeLen -= len + MAC_PAYLOAD_SUB_IE_HEADER_LEN;
      }
    }
    pPayloadIe = pPayloadIe->pNext;
  }

  return pIeContent;
}

/*******************************************************************************/
