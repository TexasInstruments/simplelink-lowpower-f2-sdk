/******************************************************************************

 @file  mac_security_pib.c

 @brief This module contains procedures for the Security-related MAC PIB.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2010-2023, Texas Instruments Incorporated

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

#ifdef FEATURE_MAC_SECURITY
/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_mcu.h"
#include "timac_api.h"
#include "mac_spec.h"
#include "mac_low_level.h"
#include "mac_main.h"
#include "mac_security_pib.h"
#include "mac_pib.h"
#include "mac_assert.h"
#include <stddef.h>
#ifdef FEATURE_WISUN_SUPPORT
#include "api_mac.h"
#endif

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

/* Attribute index constants, based on attribute ID values from spec */
#define MAC_ATTR_SECURITY_SET1_START       0x81
#define MAC_ATTR_SECURITY_SET1_END         0x8B
#define MAC_ATTR_SECURITY_SET1_OFFSET      0
#define MAC_ATTR_SECURITY_SET2_START       0xD0
#define MAC_ATTR_SECURITY_SET2_END         0xD5

#define EXT_ADDR_LEN                       8



uint8 MAC_MlmeGetSecurityReqSize( uint8 pibAttribute );

#if defined( FEATURE_MAC_PIB_PTR )

/**************************************************************************************************
 * @fn          MAC_MlmeSetActiveSecurityPib
 *
 * @brief       This direct execute function sets the active MAC security PIB.
 *
 * input parameters
 *
 * @param       pPib - pointer to the PIB structure.
 *
 * output parameters
 *
 * @return      None.
 *
 **************************************************************************************************
 */
void MAC_MlmeSetActiveSecurityPib( void* pSecPib)
{
  pMacSecurityPib = (macSecurityPib_t *)pSecPib;
}
#endif /* FEATURE_MAC_PIB_PTR */

/**************************************************************************************************
 * @fn          macSecurityPibReset
 *
 * @brief       This function initializes the security related PIB.
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
MAC_INTERNAL_API void macSecurityPibReset(void)
{
  uint16 i;
  keyDescriptor_t *pKeyTable;
  deviceDescriptor_t *pDeviceTable;

  /* init the macPib to zero */
  MAP_osal_memset(&macSecurityPib, 0x00, sizeof(macSecurityPib));

  /* Alloc macKeyTable[macCfg.maxKeyTableEntries] */
  pKeyTable = (keyDescriptor_t *)pMacSecurityPib->macKeyTable;
  for(i = 0; i < macCfg.maxKeyTableEntries; i++)
  {
      (pKeyTable + i)->keyIdLookupList = &pMacSecurityPib->macKeyIdLookupList[i];
      (pKeyTable + i)->keyIdLookupEntries = macCfg.maxKeyIdLookUpEntries;

      (pKeyTable + i)->keyDeviceList = &pMacSecurityPib->macKeyDeviceList[i][0];
      (pKeyTable + i)->keyDeviceListEntries = 0;

      (pKeyTable + i)->keyUsageList = &pMacSecurityPib->macKeyUsageList[0];
      (pKeyTable + i)->keyUsageListEntries = MAX_KEY_USAGE_TABLE_ENTRIES;

      (pKeyTable + i)->frameCounter = 0;

      memset((pKeyTable + i)->key, 0, 16);
  }

  pDeviceTable = pMacSecurityPib->macDeviceTable;

  for (i = 0; i < macCfg.maxDeviceTableEntries; i++)
  {
      (pDeviceTable + i)->panID = 0xFFFF;
      memset((pDeviceTable + i)->frameCounter, 0, sizeof((pDeviceTable + i)->frameCounter));
      (pDeviceTable + i)->shortAddress = 0xFFFF;
      memset((pDeviceTable + i)->extAddress, 0, EXT_ADDR_LEN);
      (pDeviceTable + i)->exempt = true;
  }
  pMacSecurityPib->deviceTableEntries = macCfg.maxDeviceTableEntries;

  MAP_osal_memset(&macSecurityPib.autoRequestKeySource[0], 0xFF, sizeof(macSecurityPib.autoRequestKeySource));
  macSecurityPib.autoRequestKeyIndex = 0xFF;
  MAP_osal_memset(&macSecurityPib.defaultKeySource[0], 0xFF, sizeof(macSecurityPib.defaultKeySource));

  macSecurityPib.panCoordExtendedAddress.addrMode = SADDR_MODE_EXT;
  macSecurityPib.panCoordShortAddress = MAC_SHORT_ADDR_NONE;

  /* macKeyusageList : MAX_KEY_USAGE_TABLE_ENTRIES should be 2 */
  keyUsageDescriptor_t *pKeyUsageList = &pMacSecurityPib->macKeyUsageList[0];
  (pKeyUsageList + 0)->frameType = MAC_FRAME_TYPE_DATA;
  (pKeyUsageList + 0)->cmdFrameId = 0;

  (pKeyUsageList + 1)->frameType = MAC_FRAME_TYPE_ACK;
  (pKeyUsageList + 1)->cmdFrameId = 0;

  /* key secuirty level table : MAX_SECURITY_LEVEL_TABLE_ENTRIES should be 2 */
  for (i = 0 ; i < MAX_SECURITY_LEVEL_TABLE_ENTRIES; i++)
  {
      macSecurityPib.macSecurityLevelTable[0].commandFrameIdentifier = 0;
      macSecurityPib.macSecurityLevelTable[0].securityMinimum = MAC_SEC_LEVEL_ENC_MIC_64;
      macSecurityPib.macSecurityLevelTable[0].securityOverrideSecurityMinimum = FALSE;
  }
  macSecurityPib.macSecurityLevelTable[0].frameType = MAC_FRAME_TYPE_DATA;
  macSecurityPib.macSecurityLevelTable[1].frameType = MAC_FRAME_TYPE_ACK;
  macSecurityPib.securityLevelTableEntries = MAX_SECURITY_LEVEL_TABLE_ENTRIES;
}

/**************************************************************************************************
 * @fn          macSecurityPibIndex
 *
 * @brief       This function takes an security related PIB attribute and returns the index in to
 *              macSecurityPibTbl for the attribute.
 *
 * input parameters
 *
 * @param       pibAttribute - Security related PIB attribute to look up.
 *
 * output parameters
 *
 * None.
 *
 * @return      Index in to macSecurityPibTbl for the attribute or MAC_SECURITY_PIB_INVALID.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macSecurityPibIndex(uint8 pibAttribute)
{
  if ((pibAttribute >= MAC_ATTR_SECURITY_SET1_START) && (pibAttribute <= MAC_ATTR_SECURITY_SET1_END))
  {
    return (pibAttribute - MAC_ATTR_SECURITY_SET1_START + MAC_ATTR_SECURITY_SET1_OFFSET);
  }
  else
  {
    return MAC_SECURITY_PIB_INVALID;
  }
}


/**************************************************************************************************
 * @fn          MAC_MlmeGetSecurityReq
 *
 * @brief       This direct execute function retrieves an attribute value
 *              from the MAC security related PIB.
 *
 * input parameters
 *
 * @param       pibAttribute - The attribute identifier.
 * @param       pValue - pointer to the attribute value.
 *
 * output parameters
 *
 * @param       pValue - pointer to the attribute value.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *
 **************************************************************************************************
 */
uint8 MAC_MlmeGetSecurityReq(uint8 pibAttribute, void *pValue)
{
  uint16        i, keyIndex;
  halIntState_t intState;
  keyDescriptor_t *pKeyTable;

  /* Special handling for proprietary Security PIB Get and Set Attributes */
  switch (pibAttribute)
  {
      case MAC_FRAME_COUNTER:
        keyIndex = ((macSecurityGet_t *)pValue)->keyIndex;
        pKeyTable = ((keyDescriptor_t *)pMacSecurityPib->macKeyTable) + keyIndex;
        *(uint32 *)(((macSecurityGet_t *)pValue)->data) = pKeyTable->frameCounter;
        return MAC_SUCCESS;
      default:
        break;
  }

  if ((i = MAP_macSecurityPibIndex(pibAttribute)) == MAC_SECURITY_PIB_INVALID)
  {
    return MAC_UNSUPPORTED_ATTRIBUTE;
  }

  HAL_ENTER_CRITICAL_SECTION(intState);
  MAP_osal_memcpy(pValue, (uint8 *) pMacSecurityPib + macSecurityPibTbl[i].offset, macSecurityPibTbl[i].len);
  HAL_EXIT_CRITICAL_SECTION(intState);
  return MAC_SUCCESS;
}


/**************************************************************************************************
 * @fn          MAC_MlmeGetPointerSecurityReq
 *
 * @brief       This direct execute function gets the MAC security PIB attribute pointers
 *
 * input parameters
 *
 * @param       pibAttribute - The attribute identifier.
 *
 * output parameters
 *
 * @param       pValue will contain a pointer to the attribute in macPib
 *
 * @return      MAC_SUCCESS
 *
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 MAC_MlmeGetPointerSecurityReq(uint8 pibAttribute, void **pValue)
{
  switch (pibAttribute)
  {
  case MAC_KEY_TABLE:
    *pValue = pMacSecurityPib->macKeyTable;
    return MAC_SUCCESS;
  case MAC_DEVICE_TABLE:
    *pValue =  pMacSecurityPib->macDeviceTable;
    return MAC_SUCCESS;
  case MAC_SECURITY_LEVEL_TABLE:
    *pValue = pMacSecurityPib->macSecurityLevelTable;
    return MAC_SUCCESS;
  default:
    return MAC_INVALID_PARAMETER;
  }
}


/**************************************************************************************************
 * @fn          MAC_MlmeGetSecurityReqSize
 *
 * @brief       This direct execute function gets the MAC security PIB attribute size
 *
 * input parameters
 *
 * @param       pibAttribute - The attribute identifier.
 *
 * output parameters
 *
 * None.
 *
 * @return      size in bytes
 *
 **************************************************************************************************
 */
uint8 MAC_MlmeGetSecurityReqSize( uint8 pibAttribute )
{
  uint8 len = 0;

  /* Special handling for proprietary Security PIB Get and Set Attributes */
  switch (pibAttribute)
  {
    case MAC_KEY_ID_LOOKUP_ENTRY:
      len = (uint8)(sizeof(macSecurityPibKeyIdLookupEntry_t));
      break;

    case MAC_KEY_DEVICE_ENTRY:
      len = (uint8)(sizeof(macSecurityPibKeyDeviceEntry_t));
      break;

    case MAC_KEY_USAGE_ENTRY:
      len = (uint8)(sizeof(macSecurityPibKeyUsageEntry_t));
      break;

    case MAC_KEY_ENTRY:
      len = (uint8)(sizeof(macSecurityPibKeyEntry_t));
      break;

    case MAC_DEVICE_ENTRY:
      len = (uint8)(sizeof(macSecurityPibDeviceEntry_t));
      break;

    case MAC_SECURITY_LEVEL_ENTRY:
      len = (uint8)(sizeof(macSecurityPibSecurityLevelEntry_t));
      break;

    default:
      {
        uint8 i;
        if ((i = MAP_macSecurityPibIndex(pibAttribute)) != MAC_SECURITY_PIB_INVALID)
        {
          len = macSecurityPibTbl[i].len;
        }
      }
      break;
  }

  return ( len );
}


/**************************************************************************************************
 * @fn          MAC_MlmeSetSecurityReq
 *
 * @brief       This direct execute function sets an attribute value
 *              in the MAC security related PIB.
 *
 * input parameters
 *
 * @param       pibAttribute - The attribute identifier.
 * @param       pValue - pointer to the attribute value.
 *
 * output parameters
 *
 * None.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS Operation successful.
 *              MAC_UNSUPPORTED_ATTRIBUTE Attribute not found.
 *
 **************************************************************************************************
 */
uint8 MAC_MlmeSetSecurityReq(uint8 pibAttribute, void *pValue)
{
#define EXTADDR_LEN     8
  uint16        i, keyIndex;
  uint16        deviceIndex;
  halIntState_t intState;
  keyDescriptor_t *pKeyTable;
  keyDescriptor_t *pKeyEntry;
  deviceDescriptor_t *pDeviceTable;
  deviceDescriptor_t *pDeviceEntry;
  uint8_t allOnes[EXTADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  if (!pValue)
  {
      return MAC_INVALID_PARAMETER;
  }

  /* Special handling for proprietary Security PIB Get and Set Attributes */
  switch (pibAttribute)
  {
    case MAC_FRAME_COUNTER:
      keyIndex = ((keyIndexDesc_t *)pValue)->keyIndex;
#ifdef FEATURE_WISUN_SUPPORT
      if(keyIndex >= MAX_KEY_TABLE_ENTRIES)
      {
          return MAC_INVALID_INDEX;
      }
#endif
      pKeyTable = ((keyDescriptor_t *)pMacSecurityPib->macKeyTable) + keyIndex;
      pKeyTable->frameCounter = ((keyIndexDesc_t *)pValue)->keyDesc.frameCounter;
      return MAC_SUCCESS;

    case MAC_KEY_TABLE:
      keyIndex = ((keyIndexDesc_t *)pValue)->keyIndex;
#ifdef FEATURE_WISUN_SUPPORT
      if(keyIndex >= MAX_KEY_TABLE_ENTRIES)
      {
          return MAC_INVALID_INDEX;
      }
#endif
      pKeyTable = ((keyDescriptor_t *)pMacSecurityPib->macKeyTable) + keyIndex;
      pKeyEntry = &((keyIndexDesc_t *)pValue)->keyDesc;

      /* add key table */
      if(pKeyEntry->keyIdLookupEntries)
      {
          MAP_osal_memcpy(pKeyTable->keyIdLookupList, pKeyEntry->keyIdLookupList, pKeyEntry->keyIdLookupEntries * sizeof(keyIdLookupDescriptor_t));
          pMacSecurityPib->keyTableEntries++;
      }
      /* remove key table */
      else
      {
          MAP_osal_memset(pKeyTable->keyIdLookupList, 0, pKeyEntry->keyIdLookupEntries * sizeof(keyIdLookupDescriptor_t));
          MAP_osal_memset(pKeyTable->keyDeviceList, 0, pKeyEntry->keyDeviceListEntries * sizeof(keyDeviceDescriptor_t));
          pMacSecurityPib->keyTableEntries = pMacSecurityPib->keyTableEntries ? (pMacSecurityPib->keyTableEntries - 1) : 0;
      }
      pKeyTable->keyIdLookupEntries = pKeyEntry->keyIdLookupEntries;
      pKeyTable->keyDeviceListEntries = pKeyEntry->keyDeviceListEntries;
      MAP_osal_memcpy(pKeyTable->key, pKeyEntry->key, MAC_KEY_MAX_LEN);
      pKeyTable->frameCounter = 0;
      return MAC_SUCCESS;

    case MAC_DEVICE_TABLE:
      deviceIndex = ((deviceIndexDesc_t *)pValue)->deviceIndex;
      pDeviceTable = ((deviceDescriptor_t *)pMacSecurityPib->macDeviceTable) + deviceIndex;
      pDeviceEntry = &((deviceIndexDesc_t *)pValue)->deviceDesc;

      if(deviceIndex >= pMacSecurityPib->deviceTableEntries)
      {
          return MAC_INVALID_INDEX;
      }

      MAP_osal_memcpy(pDeviceTable, pDeviceEntry, sizeof(deviceDescriptor_t));

      /* nanostack will send all ones to remove the entry */
      if(MAP_osal_memcmp(pDeviceEntry->extAddress, allOnes, EXTADDR_LEN))
      {
          /* To Do */
          /* remove keyDeviceList together if exist */
      }

      return MAC_SUCCESS;

    case MAC_SECURITY_LEVEL_TABLE:
      return MAC_SUCCESS;
  }

  /* look up attribute in security related PIB table */
  if ((i = MAP_macSecurityPibIndex(pibAttribute)) == MAC_SECURITY_PIB_INVALID)
  {
    return MAC_UNSUPPORTED_ATTRIBUTE;
  }

  /* do range check; no range check if min and max are zero */
  if ((macSecurityPibTbl[i].min != 0) || (macSecurityPibTbl[i].max != 0))
  {
    /* if min == max, this is a read-only attribute */
    if (macSecurityPibTbl[i].min == macSecurityPibTbl[i].max)
    {
      return MAC_READ_ONLY;
    }

    /* range check for general case */
    if ((*((uint8 *) pValue) < macSecurityPibTbl[i].min) || (*((uint8 *) pValue) > macSecurityPibTbl[i].max))
    {
      return MAC_INVALID_PARAMETER;
    }
  }

  /* set value in security related PIB */
  HAL_ENTER_CRITICAL_SECTION(intState);
  MAP_osal_memcpy((uint8 *) pMacSecurityPib + macSecurityPibTbl[i].offset, pValue, macSecurityPibTbl[i].len);
  HAL_EXIT_CRITICAL_SECTION(intState);

  return MAC_SUCCESS;
}

#endif
