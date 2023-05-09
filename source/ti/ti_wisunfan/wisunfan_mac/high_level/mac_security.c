/******************************************************************************

 @file  mac_security.c

 @brief This module contains procedures for MAC security.

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
#include "mac_spec.h"
#include "timac_api.h"
#include "mac_pib.h"
#include "mac_main.h"
#include "mac_security_pib.h"
#include "mac_security.h"
#include "mac_hl_patch.h"
#include "mac_mgmt.h"
#include "mac_assert.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_security_c
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
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */
#if !defined(TIMAC_ROM_PATCH)
#define SECURITY_LEVEL_PASSED(SEC1, SEC2) ((SEC1 >> 2) >= (SEC2 >> 2)) && ((SEC1 & 0x03) >= (SEC2 & 0x03))


/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */
uint8 macDeviceDescriptorLookup( deviceDescriptor_t *deviceDescriptor,
                                 uint8 *deviceLookupData,
                                 uint8 deviceLookupSize );
uint8 macIncomingKeyUsagePolicyChecking( keyDescriptor_t *keyDescriptor,
                                         uint8 frameType,
                                         uint8 cmdFrameId );
uint8 macIncomingSecurityLevelChecking( uint8 securityLevel,
                                        uint8 frameType,
                                        uint8 cmdFrameId );
uint8 macBlacklistChecking( keyDescriptor_t *pKeyDescriptor,
                            uint8 *deviceLookupData,
                            uint8 deviceLookupSize,
                            deviceDescriptor_t **ppDeviceDescriptor,
                            keyDeviceDescriptor_t **ppKeyDeviceDescriptor );
uint8 macIncomingFrameSecurityMaterialRetrieval( macRx_t *pMsg,
                                                 keyDescriptor_t **ppKeyDescriptor,
                                                 deviceDescriptor_t **ppDeviceDescriptor,
                                                 keyDeviceDescriptor_t **ppKeyDeviceDescriptor);
uint8 macCcmStarInverseTransform( uint8    *pKey,
                                  uint32   frameCounter,
                                  uint8    securityLevel,
                                  uint8    *pAData,
                                  uint16   aDataLen,
                                  uint8    *pCData,
                                  uint16   cDataLen,
                                  uint8    *pExtAddr );
uint8 macKeyDescriptorLookup( uint8 *pLookupData, uint8 lookupDataSize, keyDescriptor_t **ppKeyDescriptor, uint8 *keyDescriptorIndex );
keyDescriptor_t* macOutgoingFrameKeyDescRetrieval( ApiMac_sec_t  *pSec,
                                                   sAddr_t  *pDstAddr,
                                                   uint16   dstPanId );

#endif /*TIMAC_ROM_PATCH */



/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Functions
 * ------------------------------------------------------------------------------------------------
 */
#if !defined(TIMAC_ROM_PATCH)
/**************************************************************************************************
 * @fn          macKeyDescriptorLookup
 *
 * @brief       The inputs to this procedure are the key lookup data and the key lookup size. The
 *              outputs from this procedure are a passed or failed status and, if passed, a
 *              KeyDescriptor.
 *
 * input parameters
 *
 * @param       pLookupData - keyIdLookupDescriptor - pointer to keyIdLookupDescriptor_t
 * @param       lookupDataSize - key lookup data size
 *
 * output parameters
 *
 * @param       ppKeyDescriptor - pointer to pointer to keyDescriptor_t
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */

uint8 macKeyDescriptorLookup( uint8 *pLookupData, uint8 lookupDataSize, keyDescriptor_t **ppKeyDescriptor )
{
  uint8 status = MAC_UNAVAILABLE_KEY;
  uint8 descriptorDataSize;
  keyIdLookupDescriptor_t *pKeyIdLookupList;
  uint16_t i, j, keyTableEntries;

  MAP_MAC_MlmeGetPointerSecurityReq(MAC_KEY_TABLE, (void **)ppKeyDescriptor);
  MAP_MAC_MlmeGetSecurityReq(MAC_KEY_TABLE_ENTRIES, &keyTableEntries);

  for (i = 0; i < keyTableEntries; i++)
  {
    pKeyIdLookupList = (*ppKeyDescriptor)->keyIdLookupList;
    for (j = 0; j < (*ppKeyDescriptor)->keyIdLookupEntries; j++)
    {
      descriptorDataSize = (pKeyIdLookupList->lookupDataSize == 0)?
                           MAC_KEY_LOOKUP_SHORT_LEN : MAC_KEY_LOOKUP_LONG_LEN;
      if ( descriptorDataSize == lookupDataSize &&
           MAP_osal_memcmp(pKeyIdLookupList->lookupData, pLookupData, lookupDataSize) == TRUE )
      {
        /* break key ID lookup (j) loop */
        status = MAC_SUCCESS;
        break;
      }

      /* to the next key ID look up list */
      pKeyIdLookupList++;
    }

    if (status == MAC_SUCCESS)
    {
      /* break key table (i) loop */
      break;
    }

    /* to the next PIB entry */
    (*ppKeyDescriptor)++;
  }

  return (status);
}

/**************************************************************************************************
 * @fn          macIncomingSecurityLevelChecking
 *
 * @brief       The inputs to this procedure are the incoming security level, the frame type and
 *              the command frame identifier. The output from this procedure is a passed, failed,
 *              or "conditionally passed" status.
 *
 * input parameters
 *
 * @param       securityLevel - incoming frame security level
 * @param       frameType - MAC frame type
 * @param       cmdFrameId - Command frame Identifier
 *
 * output parameters
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */
uint8 macIncomingSecurityLevelChecking( uint8 securityLevel,
                                        uint8 frameType,
                                        uint8 cmdFrameId )
{
  uint8                     i;
  uint8                     pibSecurityLevel, securityLevelTableEntries;
  securityLevelDescriptor_t *pSecLevelTbl;

  MAP_MAC_MlmeGetPointerSecurityReq( MAC_SECURITY_LEVEL_TABLE, (void **)&pSecLevelTbl);
  MAP_MAC_MlmeGetSecurityReq(MAC_SECURITY_LEVEL_TABLE_ENTRIES, &securityLevelTableEntries);

  for (i = 0; i < securityLevelTableEntries; i++)
  {
    pibSecurityLevel = pSecLevelTbl->securityMinimum;
    if ((frameType != MAC_FRAME_TYPE_COMMAND) &&
        ((frameType == pSecLevelTbl->frameType) || ((MAC_FH_ENABLED) &&(frameType == MAC_FRAME_TYPE_ACK ))))
    {
      if (!SECURITY_LEVEL_PASSED(securityLevel, pibSecurityLevel))
      {
        if ((pSecLevelTbl->securityOverrideSecurityMinimum == TRUE) &&
            (securityLevel == MAC_SEC_LEVEL_NONE))
        {
          /* Conditionally passed */
          return (MAC_CONDITIONALLY_PASSED);
        }
      }
      else
      {
        /* Passed the security level check */
        return (MAC_SUCCESS);
      }
    }
    else if ((frameType == MAC_FRAME_TYPE_COMMAND) &&
             (frameType == pSecLevelTbl->frameType) &&
             (cmdFrameId == pSecLevelTbl->commandFrameIdentifier))
    {
      if (!SECURITY_LEVEL_PASSED(securityLevel, pibSecurityLevel))
      {
        if ((pSecLevelTbl->securityOverrideSecurityMinimum == TRUE) &&
            (securityLevel == MAC_SEC_LEVEL_NONE))
        {
          /* Conditionally passed */
          return (MAC_CONDITIONALLY_PASSED);
        }
      }
      else
      {
        /* Passed the security level check */
        return (MAC_SUCCESS);
      }
    }

    /* Walk through the table */
    pSecLevelTbl++;
  }

  /* Walked through the PIB table and a security level match
   * could not be found.
   */
  return (MAC_IMPROPER_SECURITY_LEVEL);
}

/**************************************************************************************************
 * @fn          macDeviceDescriptorLookup
 *
 * @brief       The inputs to this procedure are the DeviceDescriptor, the device lookup data, and
 *              the device lookup size. The output from this procedure is a passed or failed status.
 *
 * input parameters
 *
 * @param       deviceDescriptor - pointer to deviceDescriptor_t
 * @param       deviceLookupData - pointer to uint8
 * @param       deviceLookupSize - divice lookup size (4 or 8)
 *
 * output parameters
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */
uint8 macDeviceDescriptorLookup( deviceDescriptor_t *deviceDescriptor,
                                 uint8 *deviceLookupData,
                                 uint8 deviceLookupSize )
{
  if (deviceLookupSize == MAC_DEVICE_LOOKUP_SHORT_LEN)
  {
    uint8 descriptorLoopupData[MAC_DEVICE_LOOKUP_SHORT_LEN];

    descriptorLoopupData[0] = LO_UINT16(deviceDescriptor->panID);
    descriptorLoopupData[1] = HI_UINT16(deviceDescriptor->panID);
    descriptorLoopupData[2] = LO_UINT16(deviceDescriptor->shortAddress);
    descriptorLoopupData[3] = HI_UINT16(deviceDescriptor->shortAddress);

    if (MAP_osal_memcmp(descriptorLoopupData,
                    deviceLookupData,
                    MAC_DEVICE_LOOKUP_SHORT_LEN) == TRUE)
    {
      return (MAC_SUCCESS);
    }
  }
  else if (deviceLookupSize == MAC_DEVICE_LOOKUP_LONG_LEN)
  {
    if (MAP_sAddrExtCmp(deviceLookupData, deviceDescriptor->extAddress) == TRUE)
    {
      return (MAC_SUCCESS);
    }
  }

  return (MAC_UNAVAILABLE_KEY);
}


/**************************************************************************************************
 * @fn          macBlacklistChecking
 *
 * @brief       The inputs to this procedure are the KeyDescriptor, the device lookup data, and the
 *              device lookup size. The outputs from this procedure are a passed or failed status
 *              and, if passed, a DeviceDescriptor and a KeyDeviceDescriptor.
 *
 * input parameters
 *
 * @param       keyDescriptor - pointer to keyDescriptor_t
 * @param       deviceLookupData - pointer to uint8
 * @param       deviceLookupSize - divice lookup size (4 or 8)
 *
 * output parameters
 *
 * @param       ppDeviceDescriptor - pointer to pointer to deviceDescriptor_t
 * @param       ppKeyDeviceDescriptor - pointer to pointer to keyDeviceDescriptor_t
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */

uint8 macBlacklistChecking( keyDescriptor_t *pKeyDescriptor,
                            uint8 *deviceLookupData,
                            uint8 deviceLookupSize,
                            deviceDescriptor_t **ppDeviceDescriptor,
                            keyDeviceDescriptor_t **ppKeyDeviceDescriptor )
{
  uint16_t i, numDevices, deviceTableEntries;
  uint8 status;

  MAP_MAC_MlmeGetSecurityReq(MAC_DEVICE_TABLE_ENTRIES, &deviceTableEntries);

  /* Temporary workaround till the key device list entry can be maintained to represent actual size of the list */
  numDevices = (pKeyDescriptor->keyDeviceListEntries < deviceTableEntries)?
                pKeyDescriptor->keyDeviceListEntries : deviceTableEntries;
  for (i = 0; i < numDevices; i++)
  {
    MAP_MAC_MlmeGetPointerSecurityReq( MAC_DEVICE_TABLE, (void **)ppDeviceDescriptor);

    *ppKeyDeviceDescriptor = pKeyDescriptor->keyDeviceList + i;

    /* Use the DeviceDescriptorHandle to find (or index to) the DeviceDescriptor */
    if ((*ppKeyDeviceDescriptor)->deviceDescriptorHandle >= deviceTableEntries)
    {
      continue;
    }
    *ppDeviceDescriptor += (*ppKeyDeviceDescriptor)->deviceDescriptorHandle;

    if ( (*ppKeyDeviceDescriptor)->uniqueDevice == TRUE)
    {
      /* Device is unique */
      return ( ((*ppKeyDeviceDescriptor)->blackListed == FALSE)? MAC_SUCCESS : MAC_UNAVAILABLE_KEY );
    }
    else
    {
      /* Device is not unique */
      if ((*ppKeyDeviceDescriptor)->blackListed == FALSE)
      {
        /* Device is not black-listed */
        if (( status = MAP_macDeviceDescriptorLookup( *ppDeviceDescriptor,
                                                  deviceLookupData,
                                                  deviceLookupSize )) == MAC_SUCCESS )
        {
          return (status);
        }
      }
      else
      {
        /* Device is black-listed */
        return (MAC_UNAVAILABLE_KEY);
      }
    }
  }
  return (MAC_UNAVAILABLE_KEY);
}

/**************************************************************************************************
 * @fn          macIncomingFrameSecurityMaterialRetrieval
 *
 * @brief       The input to this procedure is the frame to be unsecured. The outputs from this
 *              procedure are a passed or failed status and, if passed, a KeyDescriptor, a
 *              DeviceDescriptor, and a KeyDeviceDescriptor.
 *
 * input parameters
 *
 * @param       pMsg - pointer to the frame to be unsecured
 *
 * output parameters
 *
 * @param       ppKeyDescriptor - pointer to pointer to keyDescriptor_t
 * @param       ppDeviceDescriptor - pointer to pointer to deviceDescriptor_t
 * @param       ppKeyDeviceDescriptor - pointer to pointer to keyDeviceDescriptor_t
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */
uint8 macIncomingFrameSecurityMaterialRetrieval( macRx_t *pMsg,
                                                 keyDescriptor_t **ppKeyDescriptor,
                                                 deviceDescriptor_t **ppDeviceDescriptor,
                                                 keyDeviceDescriptor_t **ppKeyDeviceDescriptor)
{
  sAddr_t panCoordExtendedAddress;
  uint16 panCoordShortAddress;
  uint8  defaultKeySource[MAC_KEY_SOURCE_MAX_LEN];
  uint8  lookupData[MAC_MAX_KEY_LOOKUP_LEN];
  uint8  lookupDataSize;
  uint8  srcAddrMode;
  uint8  deviceLoopupData[MAC_DEVICE_LOOKUP_LONG_LEN];
  uint8  deviceLookupSize;


  srcAddrMode = pMsg->mac.srcAddr.addrMode;
  MAP_MAC_MlmeGetSecurityReq(MAC_PAN_COORD_SHORT_ADDRESS, &panCoordShortAddress);
  MAP_MAC_MlmeGetSecurityReq(MAC_PAN_COORD_EXTENDED_ADDRESS, &panCoordExtendedAddress);

  if (pMsg->sec.keyIdMode == MAC_KEY_ID_MODE_IMPLICIT)
  {
    /* Implicit key identification. This mode cannot be with broadcast message.
     * Look for key descriptor.
     */
    if (srcAddrMode == SADDR_MODE_NONE)
    {
      if (panCoordShortAddress < MAC_ADDR_USE_EXT)
      {
        /* short address is used */
        lookupDataSize = MAC_KEY_ID_4_LEN;
        lookupData[0]  = LO_UINT16(pMsg->mac.dstPanId);
        lookupData[1]  = HI_UINT16(pMsg->mac.dstPanId);
        lookupData[2]  = LO_UINT16(panCoordShortAddress);
        lookupData[3]  = HI_UINT16(panCoordShortAddress);
      }
      else if (panCoordShortAddress == MAC_ADDR_USE_EXT)
      {
        /* extended address is used */
        lookupDataSize = MAC_KEY_ID_8_LEN;
        MAP_sAddrExtCpy(lookupData, panCoordExtendedAddress.addr.extAddr);
      }
    }
    else if (srcAddrMode == SADDR_MODE_SHORT)
    {
      lookupDataSize = MAC_KEY_ID_4_LEN;
      if (!(pMsg->internal.flags & MAC_RX_FLAG_INTRA_PAN))
      {
        /* No PAN ID compression */
        lookupData[0] = LO_UINT16(pMsg->mac.srcPanId);
        lookupData[1] = HI_UINT16(pMsg->mac.srcPanId);
      }
      else
      {
        /* PAN ID compression */
        lookupData[0] = LO_UINT16(pMsg->mac.dstPanId);
        lookupData[1] = HI_UINT16(pMsg->mac.dstPanId);
      }
      lookupData[2] = LO_UINT16(pMsg->mac.srcAddr.addr.shortAddr);
      lookupData[3] = HI_UINT16(pMsg->mac.srcAddr.addr.shortAddr);
    }
    else if (srcAddrMode == SADDR_MODE_EXT)
    {
      lookupDataSize = MAC_KEY_ID_8_LEN;
      MAP_sAddrExtCpy(lookupData, pMsg->mac.srcAddr.addr.extAddr);
    }

    /* the lookup data always ends up with a zero */
    lookupData[lookupDataSize-1] = 0;
  }
  else
  {
    /* Explicit key identification */
    lookupDataSize = (pMsg->sec.keyIdMode == MAC_KEY_ID_MODE_4) ? MAC_KEY_ID_4_LEN : MAC_KEY_ID_8_LEN;
    if (pMsg->sec.keyIdMode == MAC_KEY_ID_MODE_1)
    {
      MAP_MAC_MlmeGetSecurityReq (MAC_DEFAULT_KEY_SOURCE, defaultKeySource);
      MAP_osal_memcpy(lookupData, defaultKeySource, lookupDataSize);
    }
    else
    {
      MAP_osal_memcpy(lookupData, pMsg->sec.keySource, lookupDataSize);
    }
    lookupData[lookupDataSize-1] = pMsg->sec.keyIndex;
  }

  if (MAP_macKeyDescriptorLookup( lookupData, lookupDataSize, ppKeyDescriptor ) != MAC_SUCCESS)
  {
    /* Key descriptor not found */
    return (MAC_UNAVAILABLE_KEY);
  }

  /* Key descriptor found. Determine device lookup data. */
  if (srcAddrMode == SADDR_MODE_NONE)
  {
    if (panCoordShortAddress < MAC_ADDR_USE_EXT)
    {
      /* short address is used */
      deviceLookupSize    = MAC_DEVICE_LOOKUP_SHORT_LEN;
      deviceLoopupData[0] = LO_UINT16(pMsg->mac.dstPanId);
      deviceLoopupData[1] = HI_UINT16(pMsg->mac.dstPanId);
      deviceLoopupData[2] = LO_UINT16(panCoordShortAddress);
      deviceLoopupData[3] = HI_UINT16(panCoordShortAddress);
    }
    else if (panCoordShortAddress == MAC_ADDR_USE_EXT)
    {
      /* extended address is used */
      deviceLookupSize = MAC_DEVICE_LOOKUP_LONG_LEN;
      MAP_sAddrExtCpy(deviceLoopupData, panCoordExtendedAddress.addr.extAddr);
    }
  }
  else if (srcAddrMode == SADDR_MODE_SHORT)
  {
    deviceLookupSize = MAC_DEVICE_LOOKUP_SHORT_LEN;
    if (!(pMsg->internal.flags & MAC_RX_FLAG_INTRA_PAN))
    {
      /* No PAN ID compression */
      deviceLoopupData[0] = LO_UINT16(pMsg->mac.srcPanId);
      deviceLoopupData[1] = HI_UINT16(pMsg->mac.srcPanId);
    }
    else
    {
      /* PAN ID compression */
      deviceLoopupData[0] = LO_UINT16(pMsg->mac.dstPanId);
      deviceLoopupData[1] = HI_UINT16(pMsg->mac.dstPanId);
    }
    deviceLoopupData[2] = LO_UINT16(pMsg->mac.srcAddr.addr.shortAddr);
    deviceLoopupData[3] = HI_UINT16(pMsg->mac.srcAddr.addr.shortAddr);
  }
  else if (srcAddrMode == SADDR_MODE_EXT)
  {
    deviceLookupSize = MAC_DEVICE_LOOKUP_LONG_LEN;
    MAP_sAddrExtCpy(deviceLoopupData, pMsg->mac.srcAddr.addr.extAddr);
  }

  if (MAP_macBlacklistChecking( *ppKeyDescriptor,
                            deviceLoopupData,
                            deviceLookupSize,
                            ppDeviceDescriptor,
                            ppKeyDeviceDescriptor) != MAC_SUCCESS)
  {
    /* Key descriptor not found */
    return (MAC_UNAVAILABLE_KEY);
  }

  return (MAC_SUCCESS);
}


/**************************************************************************************************
 * @fn          macIncomingKeyUsagePolicyChecking
 *
 * @brief       The inputs to this procedure are the KeyDescriptor, the frame type, and the command
 *              frame identifier. The output from this procedure is a passed or failed status.
 *
 * input parameters
 *
 * @param       keyDescriptor - pointer to keyDescriptor_t
 * @param       frameType - Frame Type
 * @param       cmdFrameId - Command Frame Identifier
 *
 * output parameters
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */
uint8 macIncomingKeyUsagePolicyChecking( keyDescriptor_t *keyDescriptor,
                                         uint8 frameType,
                                         uint8 cmdFrameId )
{
  uint8                i;
  keyUsageDescriptor_t *pKeyUsageList;

  for (i = 0; i < keyDescriptor->keyUsageListEntries; i++)
  {
    pKeyUsageList = keyDescriptor->keyUsageList + i;

    if (frameType != MAC_FRAME_TYPE_COMMAND)
    {
      /* Not command frame type */
      if ((frameType == pKeyUsageList->frameType)
        || ((MAC_FH_ENABLED) &&(frameType == MAC_FRAME_TYPE_ACK )))
      {
        return (MAC_SUCCESS);
      }
    }
    else
    {
      /* Command frame type */
      if (frameType == pKeyUsageList->frameType && cmdFrameId == pKeyUsageList->cmdFrameId)
      {
        return (MAC_SUCCESS);
      }
    }
  }

  return (MAC_IMPROPER_KEY_TYPE);
}

/**************************************************************************************************
 * @fn          macCcmStarInverseTransform
 *
 * @brief       This function is used to do CCM* inverse transformation. The inputs to this
 *              procedure are ExtAddress of the device descriptor, the frame counter, the security
 *              level, the key, and the secured frame. The output from this procedure is c data.
 *
 * input parameters
 *
 * @param       pKey - pointer to key
 * @param       frameCounter - frame counter
 * @param       securityLevel - security level
 * @param       pAData - pointer to a data
 * @param       aDataLen - a data length
 * @param       pCData - pointer to c data
 * @param       cDataLen - c data length
 * @param       pExtAddr - pointer to exteneded address
 *
 * output parameters
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */
uint8 macCcmStarInverseTransform( uint8    *pKey,
                                  uint32   frameCounter,
                                  uint8    securityLevel,
                                  uint8    *pAData,
                                  uint16   aDataLen,
                                  uint8    *pCData,
                                  uint16   cDataLen,
                                  uint8    *pExtAddr )
{
  uint8 nonce[MAC_NONCE_LEN];
  uint8 *pNonce;
  uint8 mic[MAC_MIC_LEN];
  uint8 status = MAC_SUCCESS;
  uint8 mLen;
  uint8 i;

  if (securityLevel == MAC_SEC_LEVEL_NONE)
  {
    return (MAC_SUCCESS);
  }

#ifdef DEBUG_SECURITY_TRACE
  DBG_PRINT1(DBGSYS, "aDataLen len, 0x%X", aDataLen );
  DBG_PRINT4(DBGSYS, "aData[0..3]=0x%X, 0x%X, 0x%X, 0x%X", pAData[0], pAData[1], pAData[2], pAData[3]);
  DBG_PRINT4(DBGSYS, "aData[4..7]=0x%X, 0x%X, 0x%X, 0x%X", pAData[4], pAData[5], pAData[6], pAData[7]);
  DBG_PRINT4(DBGSYS, "aData[8..11]=0x%X, 0x%X, 0x%X, 0x%X", pAData[8], pAData[9], pAData[10], pAData[11]);
  DBG_PRINT4(DBGSYS, "aData[12..15]=0x%X, 0x%X, 0x%X, 0x%X", pAData[12], pAData[13], pAData[14], pAData[15]);
  DBG_PRINT4(DBGSYS, "aData[16..19]=0x%X, 0x%X, 0x%X, 0x%X", pAData[16], pAData[17], pAData[18], pAData[19]);

  DBG_PRINT1(DBGSYS, "Encrypt len, 0x%X", cDataLen );
  DBG_PRINT4(DBGSYS, "Encrypt[0..3]=0x%X, 0x%X, 0x%X, 0x%X", pCData[0], pCData[1], pCData[2], pCData[3]);
  DBG_PRINT4(DBGSYS, "Encrypt[4..7]=0x%X, 0x%X, 0x%X, 0x%X", pCData[4], pCData[5], pCData[6], pCData[7]);

  DBG_PRINTL1(DBGSYS, "FrameCntr= %ul", frameCounter);

  DBG_PRINT4(DBGSYS, "Key[0..3]=0x%X, 0x%X, 0x%X, 0x%X", pKey[0], pKey[1], pKey[2], pKey[3]);
  DBG_PRINT4(DBGSYS, "Key[4..7]=0x%X, 0x%X, 0x%X, 0x%X", pKey[4], pKey[5], pKey[6], pKey[7]);
  DBG_PRINT4(DBGSYS, "Key[8..11]=0x%X, 0x%X, 0x%X, 0x%X", pKey[8], pKey[9], pKey[10], pKey[11]);
  DBG_PRINT4(DBGSYS, "Key[12..15]=0x%X, 0x%X, 0x%X, 0x%X", pKey[12], pKey[13], pKey[14], pKey[15]);
#endif /* DEBUG_SECURITY_TRACE */

  /* Data authenticity length */
  mLen = macAuthTagLen[securityLevel];

  /* determine a data length and m data length */
  if ( securityLevel < MAC_SEC_LEVEL_ENC )
  {
    /* Auth only, the MIC will be appended at the end of secured payload fields */
    pCData += (cDataLen - mLen);
    cDataLen = mLen;

    /* Should not include MIC length */
    aDataLen -= mLen;
  }
  else
  {
    /* Auth and Encrypt */
    aDataLen -= cDataLen;
  }

  /* Build nonce, start with extended address */
  pNonce = &nonce[0];
  for (i = SADDR_EXT_LEN; i > 0; i--)
  {
    *pNonce++ = *(pExtAddr + i - 1);
  }

  /* follow with frame counter */
  *pNonce++ = BREAK_UINT32( frameCounter, 3 );
  *pNonce++ = BREAK_UINT32( frameCounter, 2 );
  *pNonce++ = BREAK_UINT32( frameCounter, 1 );
  *pNonce++ = BREAK_UINT32( frameCounter, 0 );

  /* follow with security level */
  *pNonce = securityLevel;

  AESCCM_Transaction trans;
  trans.securityLevel = securityLevel;
  trans.pKey = pKey;
  trans.header = pAData;
  trans.data = pCData;
  trans.mic = mic;
  trans.nonce = nonce;
  trans.headerLength = aDataLen;
  trans.dataLength = cDataLen;
  trans.micLength = mLen;

  status = MAP_macCcmDecrypt(&trans);

#ifdef DEBUG_SECURITY_TRACE
  DBG_PRINT4(DBGSYS, "Decrypt[0..3]=0x%X, 0x%X, 0x%X, 0x%X", pCData[0], pCData[1], pCData[2], pCData[3]);
  DBG_PRINT4(DBGSYS, "Decrypt[4..7]=0x%X, 0x%X, 0x%X, 0x%X", pCData[4], pCData[5], pCData[6], pCData[7]);
#endif /* DEBUG_SECURITY_TRACE */

#ifdef DEBUG_SECURITY_TRACE
  DBG_PRINT1(DBGSYS, "Status= 0x%X", status );
#endif /* DEBUG_SECURITY_TRACE */
  return status;
}


/**************************************************************************************************
 * @fn          macIncomingFrameSecurity
 *
 * @brief       The input to this procedure is the frame to be unsecured. The outputs from this
 *              procedure are the unsecured frame, the security level, the key identifier mode, the
 *              key source, the key index, and the status of the procedure. All outputs of this
 *              procedure are assumed to be invalid unless and until explicitly set in this
 *              procedure. It is assumed that the PIB attributes associating KeyDescriptors in
 *              macKeyTable with a single, unique device or a number of devices will have been
 *              established by the next higher layer.
 *
 * input parameters
 *
 * @param       pMsg - pointer to the frame to be unsecured.
 *
 * output parameters
 *
 * @param       pMsg - pointer to the unsecured frame
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macIncomingFrameSecurity( macRx_t *pMsg )
{
  keyDescriptor_t       *pKeyDescriptor = NULL;
  deviceDescriptor_t    *pDeviceDescriptor = NULL;
  keyDeviceDescriptor_t *pKeyDeviceDescriptor = NULL;
  uint8                 securityLevel;
  uint8                 frameType;
  uint8                 status;
  uint8                 *pSecuredPayload;
  uint16                securedPayloadLen;
  uint8                 securityLevelStatus;
  uint16                keyIndex = 0;
  uint8                 kk;

  securityLevel = pMsg->sec.securityLevel;

  /* Incoming Frame Security Material Retrieval */
  if ((status = MAP_macIncomingFrameSecurityMaterialRetrieval( pMsg,
                                                           &pKeyDescriptor,
                                                           &pDeviceDescriptor,
                                                           &pKeyDeviceDescriptor))
      != MAC_SUCCESS)
  {
    pMsg->hdr.status = status;
    return (status);
  }

  /* Initialize secured payload pointer and length */
  pSecuredPayload   = pMsg->internal.ptrCData;
  securedPayloadLen = pMsg->msdu.len - (pMsg->internal.ptrCData - pMsg->msdu.p);

  /* CCM needs to be done here.
   * This function will replace the secured frame with a unsecured frame.
   */
  MAC_ASSERT(pKeyDescriptor != NULL);

  if ((status = MAP_macCcmStarInverseTransform( pKeyDescriptor->key,
                                            pMsg->internal.frameCntr,
                                            securityLevel,
                                            pMsg->mhr.p,
                                            pMsg->mhr.len,
                                            pSecuredPayload,
                                            securedPayloadLen,
                                            pDeviceDescriptor->extAddress))
      != MAC_SUCCESS)
  {
    pMsg->hdr.status = status;
    return (status);
  }

  /* Subtract MIC length from the msdu.len */
  pMsg->msdu.len -= macAuthTagLen[securityLevel];

  if (pMsg->payloadIePresent)
  {
    MAP_macUpdatePayloadIEPtr(pMsg);
  }

  frameType = pMsg->internal.fcf & MAC_FCF_FRAME_TYPE_MASK;

  if (frameType == MAC_FRAME_TYPE_COMMAND)
  {
    // Decrypt only Command Contents
    pMsg->internal.cmdFrameId = pMsg->msdu.p[0];
  }

  /* Check the security level */
  if ((status = MAP_macIncomingSecurityLevelChecking( securityLevel,
                                                  frameType,
                                                  pMsg->internal.cmdFrameId ))
      == MAC_IMPROPER_SECURITY_LEVEL)
  {
    pMsg->hdr.status = status;
    return (status);
  }

  /* Save the security level check result */
  securityLevelStatus = status;

  /* Check the key usage policy */
  if ((status = MAP_macIncomingKeyUsagePolicyChecking( pKeyDescriptor,
                                                   frameType,
                                                   pMsg->internal.cmdFrameId ))
      != MAC_SUCCESS)
  {
    pMsg->hdr.status = status;
    return (status);
  }

  /* Check exempt element */
  if (pDeviceDescriptor->exempt == FALSE && securityLevelStatus == MAC_CONDITIONALLY_PASSED)
  {
    pMsg->hdr.status = MAC_IMPROPER_SECURITY_LEVEL;
    return (MAC_IMPROPER_SECURITY_LEVEL);
  }

  /* Check the frame counter and compare with the frame counter in device descriptor */
  if (pMsg->internal.frameCntr == MAC_MAX_FRAME_COUNTER )
  {
    pMsg->hdr.status = MAC_COUNTER_ERROR;
    return (MAC_COUNTER_ERROR);
  }
  else
  {
    uint16_t k;
    keyDescriptor_t *pkeyDescriptor;
    MAP_MAC_MlmeGetPointerSecurityReq( MAC_KEY_TABLE, (void **)&pkeyDescriptor);

    for ( k = 0; k < macCfg.maxKeyTableEntries; k++)
    {
      if ( pKeyDescriptor == &pkeyDescriptor[k] )
      {
        keyIndex = k;
        for(kk = 0; kk < macCfg.maxNodeKeyTableEntries; kk++)
        {
            if(keyIndex == pDeviceDescriptor->frameCtr[kk].frameCntrIdx)
                break;
        }
        if( (kk == macCfg.maxNodeKeyTableEntries) ||
             (pMsg->internal.frameCntr < pDeviceDescriptor->frameCtr[kk].frameCounter ))
        //if( pMsg->internal.frameCntr < pDeviceDescriptor->frameCounter[k] )
        {
          return (MAC_COUNTER_ERROR);
        }
      }
    }
  }

  /* Increase the frame counter and set the frame counter in the device descriptor */
  pDeviceDescriptor->frameCtr[kk].frameCounter = pMsg->internal.frameCntr + 1;
  //pDeviceDescriptor->frameCounter[keyIndex] = pMsg->internal.frameCntr + 1;

  /* set the black list element in the key device descriptor */
  if (pDeviceDescriptor->frameCtr[kk].frameCounter == MAC_MAX_FRAME_COUNTER)
  {
    pKeyDeviceDescriptor->blackListed = TRUE;
  }
  pMsg->hdr.status = MAC_SUCCESS;
  return (MAC_SUCCESS);
}

/**************************************************************************************************
 * @fn          macOutgoingFrameKeyDescRetrieval
 *
 * @brief       The inputs to this procedure are the frame to be secured and the security
 *              parameters from the originating primitive. The output from this procedure is
 *              the pointer to the corresponding key descriptor if passed, or NULL otherwise.
 *
 * @param       pBuf - Pointer to buffer containing tx struct.
 * @param       pDstAddr - Destination address.
 * @param       dstPanId - Destination PAN ID.
 *
 * @return      pointer to key desriptor if successful, otherwise NULL.
 **************************************************************************************************
 */
keyDescriptor_t* macOutgoingFrameKeyDescRetrieval( ApiMac_sec_t  *pSec,
                                    sAddr_t  *pDstAddr,
                                    uint16   dstPanId )
{
  sAddr_t          panCoordExtendedAddress;
  uint16           panCoordShortAddress;
  uint8            defaultKeySource[MAC_KEY_SOURCE_MAX_LEN];
  uint8            lookupData[MAC_MAX_KEY_LOOKUP_LEN];
  uint8            lookupDataSize;
  keyDescriptor_t  *pKeyDescriptor = NULL;
  uint8            destAddrMode;

  MAP_MAC_MlmeGetSecurityReq(MAC_PAN_COORD_SHORT_ADDRESS, &panCoordShortAddress);
  MAP_MAC_MlmeGetSecurityReq(MAC_PAN_COORD_EXTENDED_ADDRESS, &panCoordExtendedAddress);

  if (pSec->keyIdMode == MAC_KEY_ID_MODE_IMPLICIT)
  {
    /* Implicit key identification */
    destAddrMode = pDstAddr->addrMode;

    /* This mode cannot be with broadcast message */
    if (destAddrMode == SADDR_MODE_NONE)
    {
      if (panCoordShortAddress < MAC_ADDR_USE_EXT)
      {
        /* short address is used */
        lookupDataSize = MAC_KEY_ID_4_LEN;
        lookupData[0]  = LO_UINT16(pMacPib->panId);
        lookupData[1]  = HI_UINT16(pMacPib->panId);
        lookupData[2]  = LO_UINT16(panCoordShortAddress);
        lookupData[3]  = HI_UINT16(panCoordShortAddress);
      }
      else if (panCoordShortAddress == MAC_ADDR_USE_EXT)
      {
        /* extended address is used */
        lookupDataSize = MAC_KEY_ID_8_LEN;
        MAP_sAddrExtCpy(lookupData, panCoordExtendedAddress.addr.extAddr);
      }
    }
    else if (destAddrMode == SADDR_MODE_SHORT)
    {
      /* short address is used */
      lookupDataSize = MAC_KEY_ID_4_LEN;
      lookupData[0]  = LO_UINT16(dstPanId);
      lookupData[1]  = HI_UINT16(dstPanId);
      lookupData[2]  = LO_UINT16(pDstAddr->addr.shortAddr);
      lookupData[3]  = HI_UINT16(pDstAddr->addr.shortAddr);
    }
    else if (destAddrMode == SADDR_MODE_EXT)
    {
      /* extended address is used */
      lookupDataSize = MAC_KEY_ID_8_LEN;
      MAP_sAddrExtCpy(lookupData, pDstAddr->addr.extAddr);
    }

    /* the lookupData always ends up with a zero */
    lookupData[lookupDataSize-1] = 0;
  }
  else
  {
    /* Explicit key identification */
    lookupDataSize = (pSec->keyIdMode == MAC_KEY_ID_MODE_4) ? MAC_KEY_ID_4_LEN : MAC_KEY_ID_8_LEN;
    if (pSec->keyIdMode == MAC_KEY_ID_MODE_1)
    {
      MAP_MAC_MlmeGetSecurityReq (MAC_DEFAULT_KEY_SOURCE, defaultKeySource);
      MAP_osal_memcpy(lookupData, defaultKeySource, lookupDataSize);
    }
    else
    {
      MAP_osal_memcpy(lookupData, pSec->keySource, lookupDataSize);
    }

    /* the lookupData always ends up with the key index */
    lookupData[lookupDataSize-1] = pSec->keyIndex;
  }

  if (MAP_macKeyDescriptorLookup( lookupData, lookupDataSize, &pKeyDescriptor ) != MAC_SUCCESS)
  {
    return NULL;
  }

  /* Key descriptor found */
  return (pKeyDescriptor);
}

/**************************************************************************************************
 * @fn          macOutgoingFrameSecurity
 *
 * @brief       The inputs to this procedure are the frame to be secured and the security
 *              parameters from the originating primitive or automatic request PIB attributes.
 *              The outputs from this procedure are the status of the procedure and, if this status
 *              is MAC_SUCCESS, pointer to the key to be used to secure the outgoing frame.
 *
 * input parameters
 *
 * @param       pBuf - Pointer to buffer containing tx struct.
 * @param       pDstAddr - Destination address.
 * @param       dstPanId - Destination PAN ID.
 *
 * output parameters
 *
 * @param       ppKeyDesc - Pointer to pointer to key descripter to be used to secure
 *                          the outgoing frame.
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macOutgoingFrameSecurity( macTx_t  *pBuf,
                                                 sAddr_t  *pDstAddr,
                                                 uint16   dstPanId,
                                                 keyDescriptor_t **ppKeyDesc )
{
  uint8   i;
  uint8   expansionLen;
  uint8   keyIdMode;
  uint8   securityLevel;
  uint8   *p;
  uint8   scf;

  /* Copy keyIdMode and securityLevel to local variables */
  keyIdMode     = pBuf->sec.keyIdMode;
  securityLevel = pBuf->sec.securityLevel;

  /* Determine the data expansion as AuxLen + AuthTagLen */
  expansionLen = MAC_SEC_CONTROL_FIELD_LEN  + MAC_FRAME_COUNTER_LEN +
                 macKeySourceLen[keyIdMode] + macAuthTagLen[securityLevel];

  /* check whether the length of the frame to be secured, including data expansion, is less
   * than or equal to aMaxPHYPacketSize. If this check fails, the procedure shall return with a status
   * of FRAME_TOO_LONG.
   */
  if (expansionLen + pBuf->msdu.len > macCfg.macMaxFrameSize)
  {
    pBuf->hdr.status = MAC_FRAME_TOO_LONG;
    return (MAC_FRAME_TOO_LONG);
  }

  /* security level is zero */
  if (securityLevel == MAC_SEC_LEVEL_NONE)
  {
    pBuf->hdr.status = MAC_SUCCESS;
    return (MAC_SUCCESS);
  }

  /* obtain the key descriptor using the outgoing frame key retrieval procedure */
  if ((*ppKeyDesc = MAP_macOutgoingFrameKeyDescRetrieval(&pBuf->sec, pDstAddr, dstPanId)) == NULL)
  {
    pBuf->hdr.status = MAC_UNAVAILABLE_KEY;
    return (MAC_UNAVAILABLE_KEY);
  }

  //TODO: This should be moved to LMAC as well
  if ((*ppKeyDesc)->frameCounter == MAC_MAX_FRAME_COUNTER)
  {
    /* Note that to be strictly compliant, MAC should be able to transmit at max frame counter value
     * as well and then should reject further usage of the frame counter.
     * But such compliance requiremnet is too trivial to add more code and RAM for it. */
    pBuf->hdr.status = MAC_COUNTER_ERROR;
    return (MAC_COUNTER_ERROR);
  }

  /* Insert auxiliary security header */
  p = pBuf->msdu.p - 1;

  /* build key identifier field */
  if (keyIdMode > MAC_KEY_ID_MODE_IMPLICIT)
  {
    *p-- = pBuf->sec.keyIndex;

    /* build key source for MAC_KEY_ID_MODE_4 and MAC_KEY_ID_MODE_8 */
    for (i = macKeySourceLen[keyIdMode]-MAC_KEY_ID_1_LEN; i > 0; i--)
    {
      *p-- = pBuf->sec.keySource[i-1];
    }
  }
  /* save the frame count location for later use (FH mode) */
  pBuf->internal.ptrFrameCnt = p ;
  pBuf->internal.frameCntr = (*ppKeyDesc)->frameCounter;

#if 0
  /* build frame counter */
  *p-- = BREAK_UINT32( (*ppKeyDesc)->frameCounter, 3 );
  *p-- = BREAK_UINT32( (*ppKeyDesc)->frameCounter, 2 );
  *p-- = BREAK_UINT32( (*ppKeyDesc)->frameCounter, 1 );
  *p-- = BREAK_UINT32( (*ppKeyDesc)->frameCounter, 0 );
#else
  /* build frame counter */
  *p-- = BREAK_UINT32( pBuf->internal.frameCntr, 3 );
  *p-- = BREAK_UINT32( pBuf->internal.frameCntr, 2 );
  *p-- = BREAK_UINT32( pBuf->internal.frameCntr, 1 );
  *p-- = BREAK_UINT32( pBuf->internal.frameCntr, 0 );

#endif
  /* build security control field */
  scf  = ((securityLevel << MAC_SCF_SECURITY_LEVEL_POS) & MAC_SCF_SECURITY_LEVEL_MASK);
  scf |= ((keyIdMode << MAC_SCF_KEY_IDENTIFIER_POS) & MAC_SCF_KEY_IDENTIFIER_MASK);
  *p   = scf;

  /* increment length by size of security header */
  pBuf->msdu.len += (uint8) (pBuf->msdu.p - p);
  pBuf->msdu.p = p;

  pBuf->hdr.status = MAC_SUCCESS;
  return MAC_SUCCESS;
}

/**************************************************************************************************
 * @fn          macCcmStarTransform
 *
 * @brief       This function is used to do CCM* transformation. The inputs to this procedure are
 *              the key, nonce, a data, m data. The output from this procedure is c data.
 *
 * input parameters
 *
 * @param       pKey - pointer to key
 * @param       frameCounter - frame counter value
 * @param       securityLevel - security level
 * @param       pAData - pointer to a data
 * @param       aDataLen - a data length
 * @param       pMData - pointer to m data
 * @param       mDataLen - m data length
 *
 * output parameters
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macCcmStarTransform( uint8    *pKey,
                                            uint32   frameCounter,
                                            uint8    securityLevel,
                                            uint8    *pAData,
                                            uint16   aDataLen,
                                            uint8    *pMData,
                                            uint16   mDataLen )
{
  uint8 nonce[MAC_NONCE_LEN];
  uint8 *pNonce;
  uint8 mic[MAC_MIC_LEN];
  uint8 status;
  uint8 mLen;
  uint8 i;

  MAC_ASSERT( aDataLen > mDataLen && pMData > pAData );

  if (securityLevel == MAC_SEC_LEVEL_NONE)
  {
    return (MAC_SUCCESS);
  }

#ifdef DEBUG_SECURITY_TRACE
  DBG_PRINT1(DBGSYS, "aDataLen len, 0x%X", aDataLen );
  DBG_PRINT4(DBGSYS, "aData[0..3]=0x%X, 0x%X, 0x%X, 0x%X", pAData[0], pAData[1], pAData[2], pAData[3]);
  DBG_PRINT4(DBGSYS, "aData[4..7]=0x%X, 0x%X, 0x%X, 0x%X", pAData[4], pAData[5], pAData[6], pAData[7]);
  DBG_PRINT4(DBGSYS, "aData[8..11]=0x%X, 0x%X, 0x%X, 0x%X", pAData[8], pAData[9], pAData[10], pAData[11]);
  DBG_PRINT4(DBGSYS, "aData[12..15]=0x%X, 0x%X, 0x%X, 0x%X", pAData[12], pAData[13], pAData[14], pAData[15]);
  DBG_PRINT4(DBGSYS, "aData[16..19]=0x%X, 0x%X, 0x%X, 0x%X", pAData[16], pAData[17], pAData[18], pAData[19]);

  DBG_PRINT1(DBGSYS, "PlainText len, 0x%X", mDataLen );
  DBG_PRINT4(DBGSYS, "MData[0..3]=0x%X, 0x%X, 0x%X, 0x%X", pMData[0], pMData[1], pMData[2], pMData[3]);
  DBG_PRINT4(DBGSYS, "MData[4..7]=0x%X, 0x%X, 0x%X, 0x%X", pMData[4], pMData[5], pMData[6], pMData[7]);

  DBG_PRINTL1(DBGSYS, "FrameCntr= %ul", frameCounter);

  DBG_PRINT4(DBGSYS, "Key[0..3]=0x%X, 0x%X, 0x%X, 0x%X", pKey[0], pKey[1], pKey[2], pKey[3]);
  DBG_PRINT4(DBGSYS, "Key[4..7]=0x%X, 0x%X, 0x%X, 0x%X", pKey[4], pKey[5], pKey[6], pKey[7]);
  DBG_PRINT4(DBGSYS, "Key[8..11]=0x%X, 0x%X, 0x%X, 0x%X", pKey[8], pKey[9], pKey[10], pKey[11]);
  DBG_PRINT4(DBGSYS, "Key[12..15]=0x%X, 0x%X, 0x%X, 0x%X", pKey[12], pKey[13], pKey[14], pKey[15]);
#endif /* DEBUG_SECURITY_TRACE */



  /* Data authenticity length */
  mLen = macAuthTagLen[securityLevel];

  /* Should not include MIC length */
  mDataLen -= mLen;

  /* determine a data length and m data length */
  if ( securityLevel < MAC_SEC_LEVEL_ENC )
  {
    /* Auth only, the MIC will be appended at the end of unsecured payload fields */
    pMData += mDataLen;
    mDataLen = 0;
    aDataLen -= mLen;
  }
  else
  {
    /* Auth and Encrypt */
    aDataLen -= (mDataLen + mLen);
  }
  /* Build nonce, start with extended address */
  pNonce = &nonce[0];
  for (i = SADDR_EXT_LEN; i > 0; i--)
  {
    *pNonce++ = pMacPib->extendedAddress.addr.extAddr[i-1];
  }

  /* follow with frame counter */
  *pNonce++ = BREAK_UINT32( frameCounter, 3 );
  *pNonce++ = BREAK_UINT32( frameCounter, 2 );
  *pNonce++ = BREAK_UINT32( frameCounter, 1 );
  *pNonce++ = BREAK_UINT32( frameCounter, 0 );

  /* follow with security level */
  *pNonce = securityLevel;

  AESCCM_Transaction trans;
  trans.securityLevel = securityLevel;
  trans.pKey = pKey;
  trans.header = pAData;
  trans.data = pMData;
  trans.mic = mic;
  trans.nonce = nonce;
  trans.headerLength = aDataLen;
  trans.dataLength = mDataLen;
  trans.micLength = mLen;

  status = MAP_macCcmEncrypt(&trans);

  if (status)
  {
    return status;
  }

  /* Append MIC to end of message */
  for (i=0; i < mLen; i++)
  {
    *(pMData + mDataLen + i) = mic[i];
  }

#ifdef DEBUG_SECURITY_TRACE
  DBG_PRINT1(DBGSYS, "CipherText len, 0x%X", mDataLen );
  DBG_PRINT4(DBGSYS, "CData[0..3]=0x%X, 0x%X, 0x%X, 0x%X", pMData[0], pMData[1], pMData[2], pMData[3]);
  DBG_PRINT4(DBGSYS, "CData[4..7]=0x%X, 0x%X, 0x%X, 0x%X", pMData[4], pMData[5], pMData[6], pMData[7]);
#endif /* DEBUG_SECURITY_TRACE */
  return (MAC_SUCCESS);
}
#endif /*TIMAC_ROM_PATCH*/
/**************************************************************************************************
 * @fn          macUpdatePanId
 *
 * @brief       Update Device Table Entry with new PAN ID
 *
 * input parameters
 *
 * @param       panId - the new Pan ID
 *
 * output parameters
 *
 * @return      None
 **************************************************************************************************/
MAC_INTERNAL_API void macUpdatePanId(uint16 panId)
{
  halIntState_t intState;
  uint16 entry;
  deviceDescriptor_t *pDeviceDescriptor;

  HAL_ENTER_CRITICAL_SECTION(intState);
  MAP_MAC_MlmeGetPointerSecurityReq( MAC_DEVICE_TABLE, (void** )&pDeviceDescriptor);

  for (entry = 0; entry < macCfg.maxDeviceTableEntries; entry++)
  {
    if (pDeviceDescriptor[entry].panID == pMacPib->panId)
    {
      MAP_osal_memcpy(&pDeviceDescriptor[entry].panID, &panId,
                  sizeof(pDeviceDescriptor[entry].panID));
    }
  }
  MAP_MAC_MlmeSetReq(MAC_PAN_ID, &panId);
  HAL_EXIT_CRITICAL_SECTION(intState);
}

/**************************************************************************************************
 * @fn          macGetEUIIndex
 *
 * @brief       get the EUI table index based on given EUI address
 *
 * input parameters
 *
 * @param       pEUI - the pointer of EUI address
 *
 * output parameters
 *
 * @return      0xFF ==> invalid table index, EUI address is not found
 *              other value: index of EUI table
 **************************************************************************************************/
MAC_INTERNAL_API uint16_t macGetEUIIndex(sAddrExt_t *pEUI)
{
  halIntState_t intState;
  uint16 entry,found_index;
  deviceDescriptor_t *pDeviceDescriptor;

  HAL_ENTER_CRITICAL_SECTION(intState);
  found_index = MAC_EUI_INVALID_INDEX;

  MAP_MAC_MlmeGetPointerSecurityReq( MAC_DEVICE_TABLE, (void** )&pDeviceDescriptor);

  for (entry = 0; entry < macCfg.maxDeviceTableEntries; entry++)
  {
    if (MAP_sAddrExtCmp((uint8 *)pEUI, (uint8 *)&(pDeviceDescriptor->extAddress)) == TRUE)
    {
        found_index = entry;
        break;
    }

    pDeviceDescriptor++;
  }

  HAL_EXIT_CRITICAL_SECTION(intState);

  return found_index;
}

/**************************************************************************************************
 * @fn          macGetEUI
 *
 * @brief       get the EUI address based on given EUI index
 *
 * input parameters
 *
 * @param       euiIndex -  index of EUI table
 *
 * output parameters
 *
 * @return      pointer of EUI address
 **************************************************************************************************/
MAC_INTERNAL_API sAddrExt_t* macGetEUI(uint16_t euiIndex)
{
  halIntState_t intState;
  deviceDescriptor_t *pDeviceDescriptor;

  HAL_ENTER_CRITICAL_SECTION(intState);

  MAP_MAC_MlmeGetPointerSecurityReq( MAC_DEVICE_TABLE, (void** )&pDeviceDescriptor);

  /* point the entry
   * (may add the range checking
   */
  pDeviceDescriptor += euiIndex;

  HAL_EXIT_CRITICAL_SECTION(intState);

  return &(pDeviceDescriptor->extAddress);
}

#endif

