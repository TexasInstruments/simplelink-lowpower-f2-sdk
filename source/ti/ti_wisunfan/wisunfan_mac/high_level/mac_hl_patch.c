/******************************************************************************

 @file  mac_hl_patch.c

 @brief This file implements high level mac patches.

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

#include <crypto_mac_api.h>
#include "timac_api.h"
#include "mac_pib.h"
#include "mac_hl_patch.h"
#include "rom_jt_154.h"
#include "dbg.h"
#include "mac_tx.h"
#include "mac_assert.h"
#include "mac_ie_build.h"
#include "mac_tx.h"
#include "mac_radio_defs.h"
#include "mac_ie_parse.h"

#ifdef FEATURE_WISUN_SUPPORT
#include "api_mac.h"
#endif

#ifdef FH_HOP_DEBUG
#include "fh_data.h"
#include "ns_trace.h"
#define TRACE_GROUP "mhp"
#endif

#ifdef FEATURE_EDFE_TEST_MODE
uint8_t switchToEDFE = 0;
#endif

/*-------------------------------------------------------------------------------------------------
*           Constants
*--------------------------------------------------------------------------------------------------
*/
#define AESCCM_FIELDLEN     2
#define AESCCM_NONCELEN     13

/* Value to determine last channel for end of scan */
#define MAC_SCAN_CHAN_END               (MAC_CHAN_HIGHEST + 1)

/* Value indicating scanning has not started */
#define MAC_SCAN_CHAN_INIT              (MAC_SCAN_CHAN_END + 1)

/* Multiplex ID for Key Management Protocol */
#define MPX_ID_KMP                      0x0001
/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */
#ifdef FEATURE_SECURE_COMMISSIONING
bool IsNetworkKey = false;
#endif

#if defined(TIMAC_ROM_PATCH)
#define SECURITY_LEVEL_PASSED(SEC1, SEC2) ((SEC1 >> 2) >= (SEC2 >> 2)) && ((SEC1 & 0x03) >= (SEC2 & 0x03))

#define MACWRAPPER_INVALID_KEY_INDEX        0

/* Energy detect scan resolution in MS */
#define MAC_ED_SCAN_RESOLUTION (1)

/* Convert clockDuration in milliseconds to ticks. */
#define ED_SCAN_RESOLUTION_CLOCK_TICKS  (MAC_ED_SCAN_RESOLUTION * (1000 / ClockP_getSystemTickPeriod()))

OsalPort_TimerID pClockHandleEDScan;

extern macRx_t  *pRxBuf;
/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */
int8 macWrapper8ByteUnused(const uint8 *extaddr);
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
uint8 macIncomingFrameSecurityMaterialRetrieval( macRx_t *pMsg, uint8 *keyDescriptorIndex,
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
extern uint32 macRadioTxBOBoundary(void);
#endif

/*-------------------------------------------------------------------------------------------------
*           Functions
*--------------------------------------------------------------------------------------------------
*/

/**************************************************************************************************
 * @fn          macCheckPhyRate
 *
 * @brief       This function returns predefined phy rate.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      PHY rate (PHY_MODE_FSK_50K, PHY_MODE_FSK_200K, PHY_MODE_SLR_5K, PHY_MODE_UNDEF)
 **************************************************************************************************
 */
uint8 macCheckPhyRate(uint8 phyID)
{
    /*SLR 5Kbps */
    if ((phyID >= MAC_5KBPS_915MHZ_PHY_129)
     && (phyID <= MAC_5KBPS_868MHZ_PHY_131))
    {
        return PHY_MODE_SLR_5K;
    }

    /* FSK 50Kbps */
    if (phyID == APIMAC_50KBPS_1a || phyID == APIMAC_50KBPS_1b)
    {
        return PHY_MODE_FSK_50K;
    }

    /* FSK 100Kbps */
    if (phyID == APIMAC_100KBPS_2a || phyID == APIMAC_100KBPS_2b)
    {
        return PHY_MODE_FSK_100K;
    }

    /* FSK 150Kbps */
    if (phyID == APIMAC_150KBPS_3)
    {
        return PHY_MODE_FSK_150K;
    }

    /* FSK 200kbps */
    if(phyID == APIMAC_200KBPS_4a || phyID == APIMAC_200KBPS_4b)
    {
        return PHY_MODE_FSK_200K;
    }

    /* FSK 200Kbps */
    if ((phyID >= MAC_200KBPS_915MHZ_PHY_132)
     && (phyID <= MAC_200KBPS_868MHZ_PHY_133))
    {
        return PHY_MODE_FSK_200K;
    }

    if(phyID == APIMAC_300KBPS_5)
    {
        return PHY_MODE_FSK_300K;
    }

    return PHY_MODE_UNDEF;
}

/**************************************************************************************************
 * @fn          macCheckPhyMode
 *
 * @brief       This function returns predefined phy mode.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      PHY mode (PHY_MODE_STD, PHY_MODE_GEN, PHY_MODE_UNDEF)
 **************************************************************************************************
 */
uint8 macCheckPhyMode(uint8 phyID)
{
    /*Generic Phy Mode */
    if ((phyID >= MAC_MRFSK_STD_PHY_ID_BEGIN)
     && (phyID <= MAC_MRFSK_STD_PHY_ID_END))
    {
        return PHY_MODE_STD;
    }

    /* Standard Phy Mode */
    if ((phyID >= MAC_MRFSK_GENERIC_PHY_ID_BEGIN)
     && (phyID <= MAC_MRFSK_GENERIC_PHY_ID_END))
    {
        return PHY_MODE_GEN;
    }

    return PHY_MODE_UNDEF;
}

/**************************************************************************************************
 * @fn          macPibIndex
 *
 * @brief       This function takes an PIB attribute and returns the index in to
 *              macPibTbl for the attribute.
 *
 * input parameters
 *
 * @param       pibAttribute - PIB attribute to look up.
 *
 * output parameters
 *
 * None.
 *
 * @return      Index in to macPibTbl for the attribute or MAC_PIB_INVALID.
 **************************************************************************************************
 */
uint8 macPibIndex(uint8 pibAttribute)
{
  if ((pibAttribute >= MAC_ATTR_SET1_START) && (pibAttribute <= MAC_ATTR_SET1_END))
  {
    return (pibAttribute - MAC_ATTR_SET1_START + MAC_ATTR_SET1_OFFSET);
  }
  else if ((pibAttribute >= MAC_ATTR_SET2_START) && (pibAttribute <= MAC_ATTR_SET2_END))
  {
    return (pibAttribute - MAC_ATTR_SET2_START + MAC_ATTR_SET2_OFFSET);
  }
  else
  {
    return MAC_PIB_INVALID;
  }
}

/**************************************************************************************************
 * @fn          macPibCheckByPatch
 *
 * @brief       This function check some condition to apply this pib.
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
 *              MAC_PIB_PATCH_NONE existing pib.
 *              MAC_PIB_PATCH_DONE no need to patch for new pib.
 *              MAC_PIB_PATCH_MORE more patch at the next step for new patch.
 *
 **************************************************************************************************
 */
uint8 macPibCheckByPatch(uint8 pibAttribute, void *pValue)
{
    (void) pibAttribute;
    (void) pValue;

    return MAC_PIB_PATCH_NONE;
}

/**************************************************************************************************
 * @fn          macSetDefaultsByPatch
 *
 * @brief       This function sets proper changes depending on the pib attribute.
 *
 * input parameters
 *
 * @param       pibAttribute - The attribute identifier.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void macSetDefaultsByPatch(uint8 pibAttribute)
{
#if defined(DeviceFamily_CC13X2) || defined(DeviceFamily_CC13X2X7) || defined(DeviceFamily_CC13X4)
    int8 power = MAC_DEFAULT_TX_POWER;

    if (pibAttribute == MAC_RF_PA_TYPE)
    {
        if (pMacPib->paType)
        {
          power = MAC_HPA_DEFAULT_TX_POWER;
          /* change power to default values */
          if (pMacPib->phyTransmitPower != power)
          {
            MAP_MAC_MlmeSetReq(MAC_PHY_TRANSMIT_POWER_SIGNED, &power);
          }
        }
        else
        {
          if (pMacPib->phyTransmitPower != power)
          {
            MAP_MAC_MlmeSetReq(MAC_PHY_TRANSMIT_POWER_SIGNED, &power);
          }
        }
        MAP_MAC_ResumeReq();
    }
#else
    (void) pibAttribute;
#endif
    return;
}

/**************************************************************************************************
 * @fn          macCcmEncrypt
 *
 * @brief       This function encrypt the packet by AES CCM.
 *
 * input parameters
 *
 * @param       trans - AES CCM transaction pointer.
 *
 * output parameters
 *
 * None.
 *
 * @return      The status of the operation, as follows:
 *
 **************************************************************************************************
 */
uint8 macCcmEncrypt(AESCCM_Transaction *trans)
{
    uint8 status;
#if !defined(DeviceFamily_CC13X2) && !defined(DeviceFamily_CC26X2) && !defined(DeviceFamily_CC13X2X7) && !defined(DeviceFamily_CC13X4)
    int32 statusCrypto;
    CryptoCC26XX_KeyLocation keyLocation = CRYPTOCC26XX_KEY_0;

    /* Initialize AES key */
    statusCrypto = CryptoCC26XX_allocateKey(Crypto_handle, keyLocation,
                                            (uint32_t *) trans->pKey);

    if (statusCrypto != CRYPTOCC26XX_STATUS_ERROR)
    {
      CryptoCC26XX_AESCCM_Transaction transaction;

      /* run it through the authentication + encryption, pass the ccmLVal = 2 */

      transaction.opType = (trans->securityLevel >= MAC_SEC_LEVEL_ENC) ?
                           CRYPTOCC26XX_OP_AES_CCM :
                           CRYPTOCC26XX_OP_AES_CCM_NOCRYPT;
      transaction.mode = CRYPTOCC26XX_MODE_POLLING;
      transaction.keyIndex = keyLocation;
      transaction.authLength = trans->micLength;
      transaction.nonce = (char *)trans->nonce;
      transaction.msgIn = (char *)trans->data;
      transaction.header = (char *)trans->header;
      transaction.msgOut = trans->mic;
      transaction.fieldLength = AESCCM_FIELDLEN;
      transaction.msgInLength = trans->dataLength;
      transaction.headerLength = trans->headerLength;

      statusCrypto = CryptoCC26XX_transactPolling(Crypto_handle,
                                     (CryptoCC26XX_Transaction *) &transaction);

      CryptoCC26XX_releaseKey(Crypto_handle, (int *)&keyLocation);
    }

    status = (statusCrypto == CRYPTOCC26XX_STATUS_SUCCESS) ?
             MAC_SUCCESS : MAC_SECURITY_ERROR;
#else
    int_fast16_t statusCrypto;
    CryptoKey cryptoKey;
    halIntState_t cs;

    HAL_ENTER_CRITICAL_SECTION(cs);

    /* Initialize AES key */
    statusCrypto = CryptoKeyPlaintext_initKey(&cryptoKey, (uint8_t*) trans->pKey, MAC_KEY_MAX_LEN);

    if (statusCrypto == AESCCM_STATUS_SUCCESS)
    {
        AESCCM_Operation operationOneStepEncrypt;

        /* run it through the authentication + encryption, pass the ccmLVal = 2 */

        AESCCM_Operation_init(&operationOneStepEncrypt);
        operationOneStepEncrypt.key            = &cryptoKey;
        operationOneStepEncrypt.aad            = trans->header;
        operationOneStepEncrypt.aadLength      = trans->headerLength;
        operationOneStepEncrypt.input          = trans->data;
        operationOneStepEncrypt.output         = trans->data;
        operationOneStepEncrypt.inputLength    = trans->dataLength;
        operationOneStepEncrypt.nonce          = trans->nonce;
        operationOneStepEncrypt.nonceLength    = AESCCM_NONCELEN;
        operationOneStepEncrypt.mac            = trans->mic;
        operationOneStepEncrypt.macLength      = trans->micLength;

        statusCrypto = AESCCM_oneStepEncrypt(AESCCM_handle, &operationOneStepEncrypt);
    }

    HAL_EXIT_CRITICAL_SECTION(cs);

    status = (statusCrypto == AESCCM_STATUS_SUCCESS) ?
             MAC_SUCCESS : MAC_SECURITY_ERROR;
#endif
    return(status);

}

/**************************************************************************************************
 * @fn          macCcmDecrypt
 *
 * @brief       This function decrypt the packet by AES CCM.
 *
 * input parameters
 *
 * @param       trans - AES CCM transaction pointer.
 *
 * output parameters
 *
 * None.
 *
 * @return      The status of the operation, as follows:
 *
 **************************************************************************************************
 */
uint8 macCcmDecrypt(AESCCM_Transaction *trans)
{
    uint8 status;
#if !defined(DeviceFamily_CC13X2) && !defined(DeviceFamily_CC26X2) && !defined(DeviceFamily_CC13X2X7) && !defined(DeviceFamily_CC13X4)
    int32 statusCrypto;
    CryptoCC26XX_KeyLocation keyLocation = CRYPTOCC26XX_KEY_0;

    /* Initialize AES key */
    statusCrypto = CryptoCC26XX_allocateKey(Crypto_handle, keyLocation,
                                        (uint32_t *) trans->pKey);

    if (statusCrypto != CRYPTOCC26XX_STATUS_ERROR)
    {
      CryptoCC26XX_AESCCM_Transaction transaction;

      /* run it through the authentication + encryption, pass the ccmLVal = 2 */

      transaction.opType = (trans->securityLevel >= MAC_SEC_LEVEL_ENC) ?
                           CRYPTOCC26XX_OP_AES_CCMINV :
                           CRYPTOCC26XX_OP_AES_CCMINV_NOCRYPT;
      transaction.mode = CRYPTOCC26XX_MODE_POLLING;
      transaction.keyIndex = keyLocation;
      transaction.authLength = trans->micLength;
      transaction.nonce = (char *)trans->nonce;
      transaction.msgIn = (char *)trans->data;
      transaction.header = (char *)trans->header;
      transaction.msgOut = trans->mic;
      transaction.fieldLength = AESCCM_FIELDLEN;
      transaction.msgInLength = trans->dataLength;
      transaction.headerLength = trans->headerLength;

      statusCrypto = CryptoCC26XX_transactPolling(Crypto_handle,
                                      (CryptoCC26XX_Transaction *) &transaction);

      CryptoCC26XX_releaseKey(Crypto_handle, (int *)&keyLocation);
    }

    status = (statusCrypto == CRYPTOCC26XX_STATUS_SUCCESS) ?
             MAC_SUCCESS : MAC_SECURITY_ERROR;
#else
    int_fast16_t statusCrypto;
    CryptoKey cryptoKey;
    halIntState_t  cs;

    HAL_ENTER_CRITICAL_SECTION(cs);

    /* Initialize AES key */
    statusCrypto = CryptoKeyPlaintext_initKey(&cryptoKey, (uint8_t*) trans->pKey, MAC_KEY_MAX_LEN);

    if (statusCrypto == AESCCM_STATUS_SUCCESS)
    {
      AESCCM_Operation operationOneStepDecrypt;

      /* run it through the authentication + encryption, pass the ccmLVal = 2 */

      AESCCM_Operation_init(&operationOneStepDecrypt);
      operationOneStepDecrypt.key            = &cryptoKey;
      operationOneStepDecrypt.aad            = trans->header;
      operationOneStepDecrypt.aadLength      = trans->headerLength;
      operationOneStepDecrypt.input          = trans->data;
      operationOneStepDecrypt.output         = trans->data;
      operationOneStepDecrypt.inputLength    = trans->dataLength - trans->micLength;
      operationOneStepDecrypt.nonce          = trans->nonce;
      operationOneStepDecrypt.nonceLength    = AESCCM_NONCELEN;
      operationOneStepDecrypt.mac            = trans->data + (trans->dataLength - trans->micLength);
      operationOneStepDecrypt.macLength      = trans->micLength;

      statusCrypto = AESCCM_oneStepDecrypt(AESCCM_handle, &operationOneStepDecrypt);
    }

    HAL_EXIT_CRITICAL_SECTION(cs);

    status = (statusCrypto == AESCCM_STATUS_SUCCESS) ?
             MAC_SUCCESS : MAC_SECURITY_ERROR;
#endif
    return(status);
}

#if defined(TIMAC_ROM_PATCH)
int8 macWrapper8ByteUnused(const uint8 *extaddr)
{
  int8 k;

  for (k = 0; k < 8; k++)
  {
    if (extaddr[k] != 0xff)
    {
      return 0;
    }
  }
  return 1;
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
#ifdef FEATURE_MAC_SECURITY
  uint8                     i;
  uint8                     securityLevelTableEntries;
  securityLevelDescriptor_t *pSecLevelTbl;

  MAP_MAC_MlmeGetPointerSecurityReq( MAC_SECURITY_LEVEL_TABLE, (void **)&pSecLevelTbl);
  MAP_MAC_MlmeGetSecurityReq(MAC_SECURITY_LEVEL_TABLE_ENTRIES, &securityLevelTableEntries);

  for (i = 0; i < securityLevelTableEntries; i++)
  {
    if ((securityLevel == pSecLevelTbl->securityMinimum) && (frameType == pSecLevelTbl->frameType))
    {
        return (MAC_SUCCESS);
    }
    pSecLevelTbl++;
  }
#endif /*FEATURE_MAC_SECURITY*/
  return (MAC_IMPROPER_SECURITY_LEVEL);
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
#ifdef FEATURE_MAC_SECURITY
  uint8                i;
  keyUsageDescriptor_t *pKeyUsageList;

  for (i = 0; i < keyDescriptor->keyUsageListEntries; i++)
  {
    pKeyUsageList = keyDescriptor->keyUsageList + i;

    if (frameType == pKeyUsageList->frameType)
    {
      return (MAC_SUCCESS);
    }
  }
#endif /*FEATURE_MAC_SECURITY*/
  return (MAC_IMPROPER_KEY_TYPE);

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
#ifdef FEATURE_MAC_SECURITY
  if (deviceLookupSize == MAC_DEVICE_LOOKUP_LONG_LEN)
  {
    if (MAP_sAddrExtCmp(deviceLookupData, deviceDescriptor->extAddress) == TRUE)
    {
      return (MAC_SUCCESS);
    }
  }
#endif /*#ifdef FEATURE_MAC_SECURITY*/
  return (MAC_UNAVAILABLE_KEY);

}

/**************************************************************************************************
 * @fn          macApiStartReq
 *
 * @brief       This action function handles an API start request event.
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
MAC_INTERNAL_API void macApiStartReq(macEvent_t *pEvent)
{
  uint8   status;

  if((pEvent->hdr.status  != MAC_UNAVAILABLE_KEY) &&
       (pEvent->hdr.status  != MAC_UNSUPPORTED_SECURITY) &&
       (pEvent->hdr.status  != MAC_INVALID_PARAMETER) &&
       (pEvent->hdr.status  != MAC_COUNTER_ERROR))
  {
    /* parameter check */
    status = MAP_macStartBegin(pEvent);
    pEvent->hdr.status = status;
  }

  MAP_macStartContinue(pEvent);
}

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
uint8 macKeyDescriptorLookup( uint8 *pLookupData, uint8 lookupDataSize, keyDescriptor_t **ppKeyDescriptor, uint8 *keyDescriptorIndex )
{
#ifdef FEATURE_MAC_SECURITY
  keyIdLookupDescriptor_t *pKeyIdLookupList;
  uint16_t i, j;
  uint16_t  keyTableEntries;
  if (keyDescriptorIndex != NULL)
  {
    *keyDescriptorIndex = 0;
  }


  MAP_MAC_MlmeGetPointerSecurityReq(MAC_KEY_TABLE, (void **)ppKeyDescriptor);
#ifndef FEATURE_WISUN_SUPPORT
  MAP_MAC_MlmeGetSecurityReq(MAC_KEY_TABLE_ENTRIES, &keyTableEntries);

  for (i = 0; i < keyTableEntries; i++)
#else
  for (i = 0; i < MAX_KEY_TABLE_ENTRIES; i++)
#endif
  {
    pKeyIdLookupList = (*ppKeyDescriptor)->keyIdLookupList;
    for (j = 0; j < (*ppKeyDescriptor)->keyIdLookupEntries; j++)
    {
      if (MAP_osal_memcmp(pKeyIdLookupList->lookupData, pLookupData, lookupDataSize) == TRUE )
      {
          if (keyDescriptorIndex != NULL)
          {
            *keyDescriptorIndex = i;
          }
          return(MAC_SUCCESS);
      }
      /* to the next key ID look up list */
      pKeyIdLookupList++;
    }
    /* to the next PIB entry */
    (*ppKeyDescriptor)++;
  }
#endif /*FEATURE_MAC_SECURITY*/

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
#ifdef FEATURE_MAC_SECURITY
  uint16_t i, deviceTableEntries;
  uint16_t deviceHandle;

  MAP_MAC_MlmeGetSecurityReq(MAC_DEVICE_TABLE_ENTRIES, &deviceTableEntries);

  /* search device */
  MAP_MAC_MlmeGetPointerSecurityReq( MAC_DEVICE_TABLE, (void **)ppDeviceDescriptor);
  for(i = 0; i < deviceTableEntries; i++)
  {
      *ppDeviceDescriptor += 1;
      if (!MAP_macDeviceDescriptorLookup(*ppDeviceDescriptor, deviceLookupData, deviceLookupSize))
      {
        deviceHandle = i;
        break;
      }
  }

  /* not found device */
  if(i == deviceTableEntries)
  {
      return (MAC_UNSUPPORTED);
  }

  /* search key device */
  for(i = 0; i < pKeyDescriptor->keyDeviceListEntries; i++)
  {
      *ppKeyDeviceDescriptor = pKeyDescriptor->keyDeviceList + i;
      if((*ppKeyDeviceDescriptor)->deviceDescriptorHandle == deviceHandle)
      {
          break;
      }
  }

  /* add key device if not found key device */
  if(i == pKeyDescriptor->keyDeviceListEntries)
  {
      *ppKeyDeviceDescriptor = pKeyDescriptor->keyDeviceList + i;
      (*ppKeyDeviceDescriptor)->deviceDescriptorHandle = deviceHandle;
      (*ppKeyDeviceDescriptor)->uniqueDevice = false;
      (*ppKeyDeviceDescriptor)->blackListed = false;
      pKeyDescriptor->keyDeviceListEntries++;
  }

  if((*ppKeyDeviceDescriptor)->blackListed)
  {
      return (MAC_UNAVAILABLE_KEY);
  }
#endif /*FEATURE_MAC_SECURITY*/

  return(MAC_SUCCESS);
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
uint8 macIncomingFrameSecurityMaterialRetrieval( macRx_t *pMsg, uint8 *keyDescriptorIndex,
                                                 keyDescriptor_t **ppKeyDescriptor,
                                                 deviceDescriptor_t **ppDeviceDescriptor,
                                                 keyDeviceDescriptor_t **ppKeyDeviceDescriptor)
{
#ifdef FEATURE_MAC_SECURITY
  uint8  defaultKeySource[MAC_KEY_SOURCE_MAX_LEN];
  uint8  lookupData[MAC_MAX_KEY_LOOKUP_LEN];
  uint8  lookupDataSize;
  uint8  srcAddrMode;
  uint8  deviceLoopupData[MAC_DEVICE_LOOKUP_LONG_LEN];
  uint8  deviceLookupSize;

  srcAddrMode = pMsg->mac.srcAddr.addrMode;

  /* Explicit key identification */
  if (pMsg->sec.keyIdMode == MAC_KEY_ID_MODE_1)
  {
      lookupDataSize = MAC_KEY_ID_8_LEN;
      MAP_MAC_MlmeGetSecurityReq (MAC_DEFAULT_KEY_SOURCE, defaultKeySource);
      MAP_osal_memcpy(lookupData, defaultKeySource, lookupDataSize-1);
      lookupData[lookupDataSize-1] = pMsg->sec.keyIndex;
  }
  else
  {
      return (MAC_UNAVAILABLE_KEY);
  }

  if (MAP_macKeyDescriptorLookup( lookupData, lookupDataSize, ppKeyDescriptor, keyDescriptorIndex) != MAC_SUCCESS)
  {
    /* Key descriptor not found */
    return (MAC_UNAVAILABLE_KEY);
  }

  /* Key descriptor found. Determine device lookup data. */
  if (srcAddrMode == SADDR_MODE_EXT)
  {
    deviceLookupSize = MAC_DEVICE_LOOKUP_LONG_LEN;
    MAP_sAddrExtCpy(deviceLoopupData, pMsg->mac.srcAddr.addr.extAddr);
  }
  else {
      /* Invalid short address mode */
      return (MAC_UNAVAILABLE_KEY);
  }

  return (MAP_macBlacklistChecking( *ppKeyDescriptor, deviceLoopupData, deviceLookupSize,
                            ppDeviceDescriptor, ppKeyDeviceDescriptor));
#else
  return (MAC_SUCCESS);
#endif /*FEATURE_MAC_SECURITY  */
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
#ifdef FEATURE_MAC_SECURITY
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
  for (i = 0; i < SADDR_EXT_LEN; i++)
  {
    *pNonce++ = *(pExtAddr + i);
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
#else
  return(MAC_SUCCESS);
#endif /*#ifdef FEATURE_MAC_SECURITY*/
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
#ifdef FEATURE_MAC_SECURITY
  keyDescriptor_t       *pKeyDescriptor = NULL;
  deviceDescriptor_t    *pDeviceDescriptor = NULL;
  keyDeviceDescriptor_t *pKeyDeviceDescriptor = NULL;
  uint8                 securityLevel;
  uint8                 status;
  uint8                 *pSecuredPayload;
  uint16                securedPayloadLen;
  bool                  skip_framecount_verification;
#ifndef FEATURE_WISUN_SUPPORT
  uint8                 frameType;
  uint8                 securityLevelStatus;
#endif
  uint8                 *pExtAddress;
  securityLevel = pMsg->sec.securityLevel;
  uint8_t keyDescriptorIndex;

  skip_framecount_verification = false;

  /* Incoming Frame Security Material Retrieval */
  if ((status = MAP_macIncomingFrameSecurityMaterialRetrieval( pMsg, &keyDescriptorIndex,
                                                           &pKeyDescriptor,
                                                           &pDeviceDescriptor,
                                                           &pKeyDeviceDescriptor))
      == MAC_UNAVAILABLE_KEY)
  {
    pMsg->hdr.status = status;
#ifdef FH_HOP_DEBUG
    FH_Dbg.numKeyRetrievalErr++;
#endif
    return (status);
  }

  /* nanostack does not store device info here until getting neighbor solicit with ARO */
  if(status == MAC_SUCCESS)
  {
      pExtAddress = (uint8 *)pDeviceDescriptor->extAddress;
  }
  else
  {
#ifdef FEATURE_WISUN_SUPPORT
      pExtAddress = (uint8 *)pMsg->mac.srcAddr.addr.extAddr;
      /* During bootstrap (device not in device list), skip framecount verification. This allows
       * the first unicast secure frame from a device (sent during bootstrap) to bypass frame count
       * checking and arrive in nanostack. This frame will trigger nanostack to store the device info
       * to the MAC device table. After after this point, frame counts will be tracked. This prevents
       * replays from occuring, as only the first frame is allowed to bypass frame count checking.
       */
      skip_framecount_verification = true;
#else
      return (MAC_UNAVAILABLE_KEY);
#endif
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
                                            pExtAddress))
      != MAC_SUCCESS)
  {
    pMsg->hdr.status = status;
#ifdef FH_HOP_DEBUG
    FH_Dbg.numDecryptionErr++;
#endif

    return (status);
  }

  /* Subtract MIC length from the msdu.len */
  pMsg->msdu.len -= macAuthTagLen[securityLevel];

  if (pMsg->payloadIePresent)
  {
    MAP_macUpdatePayloadIEPtr(pMsg);
  }
#ifndef FEATURE_WISUN_SUPPORT
  frameType = pMsg->internal.fcf & MAC_FCF_FRAME_TYPE_MASK;

  /* Wi-SUN does not have command frame */
  pMsg->internal.cmdFrameId = 0;

  /* Check the security level */
  if ((status = MAP_macIncomingSecurityLevelChecking( securityLevel, frameType,
                                                  pMsg->internal.cmdFrameId ))
      == MAC_IMPROPER_SECURITY_LEVEL)
  {
    pMsg->hdr.status = status;
    return (status);
  }

  /* Save the security level check result */
  securityLevelStatus = status;

  /* Check the key usage policy */
  if ((status = MAP_macIncomingKeyUsagePolicyChecking( pKeyDescriptor, frameType,
                                                   pMsg->internal.cmdFrameId ))
      != MAC_SUCCESS)
  {
    pMsg->hdr.status = status;
    return (status);
  }
#endif
  if (!skip_framecount_verification)
  {
      /* Check the frame counter and compare with the frame counter in device descriptor */
      if ((pMsg->internal.frameCntr == MAC_MAX_FRAME_COUNTER ) ||
          (pMsg->internal.frameCntr < pDeviceDescriptor->frameCounter[keyDescriptorIndex] ))
      {
        pMsg->hdr.status = MAC_COUNTER_ERROR;
        return (MAC_COUNTER_ERROR);
      }

      /* Increase the frame counter and set the frame counter in the device descriptor */
      pDeviceDescriptor->frameCounter[keyDescriptorIndex] = pMsg->internal.frameCntr + 1;

      /* set the black list element in the key device descriptor */
      if (pDeviceDescriptor->frameCounter[keyDescriptorIndex] == MAC_MAX_FRAME_COUNTER)
      {
        pKeyDeviceDescriptor->blackListed = TRUE;
      }
  }
  pMsg->hdr.status = MAC_SUCCESS;
#endif /*#ifdef FEATURE_MAC_SECURITY  */

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
#ifdef FEATURE_MAC_SECURITY
  uint8            defaultKeySource[MAC_KEY_SOURCE_MAX_LEN];
  uint8            lookupData[MAC_MAX_KEY_LOOKUP_LEN];
  uint8            lookupDataSize;
  keyDescriptor_t  *pKeyDescriptor = NULL;

  lookupDataSize = MAC_KEY_ID_8_LEN;
  if (pSec->keyIdMode == MAC_KEY_ID_MODE_1)
  {
    MAP_MAC_MlmeGetSecurityReq (MAC_DEFAULT_KEY_SOURCE, defaultKeySource);
    MAP_osal_memcpy(lookupData, defaultKeySource, lookupDataSize-1);
    lookupData[lookupDataSize-1] = pSec->keyIndex;
  }
  else
  {
    return NULL;
  }

  if (MAP_macKeyDescriptorLookup( lookupData, lookupDataSize, &pKeyDescriptor, NULL ) != MAC_SUCCESS)
  {
    return NULL;
  }

  /* Key descriptor found */
  return (pKeyDescriptor);
#else
  return (NULL);
#endif /*FEATURE_MAC_SECURITY*/
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
#ifdef FEATURE_MAC_SECURITY
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

  /* build frame counter */
  *p-- = BREAK_UINT32( pBuf->internal.frameCntr, 3 );
  *p-- = BREAK_UINT32( pBuf->internal.frameCntr, 2 );
  *p-- = BREAK_UINT32( pBuf->internal.frameCntr, 1 );
  *p-- = BREAK_UINT32( pBuf->internal.frameCntr, 0 );

  /* build security control field */
  scf  = ((securityLevel << MAC_SCF_SECURITY_LEVEL_POS) & MAC_SCF_SECURITY_LEVEL_MASK);
  scf |= ((keyIdMode << MAC_SCF_KEY_IDENTIFIER_POS) & MAC_SCF_KEY_IDENTIFIER_MASK);
  *p   = scf;

  /* increment length by size of security header */
  pBuf->msdu.len += (uint8) (pBuf->msdu.p - p);
  pBuf->msdu.p = p;

  pBuf->hdr.status = MAC_SUCCESS;
#endif /*FEATURE_MAC_SECURITY*/
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
#ifdef FEATURE_MAC_SECURITY
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
  for (i = 0; i < SADDR_EXT_LEN; i++)
  {
    *pNonce++ = pMacPib->extendedAddress.addr.extAddr[i];
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
#endif /*#ifdef FEATURE_MAC_SECURITY */
  return (MAC_SUCCESS);
 }

/**************************************************************************************************
 * @fn          getLostBeaconCount
 *
 * @brief       This function is called by the low level MAC to get the number of missed beacons.
 *
 * input parameters
 *
 * None
 *
 * output parameters
 *
 * @return      uint8_t Remaining beacons to sync loss. Range of 0 to MAC_A_MAX_LOST_BEACONS.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8_t getLostBeaconCount(void)
{
    return (macBeaconDevice.lostCount);
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
}

/**************************************************************************************************
 * @fn          macAllocTxBuffer
 *
 * @brief       This function allocates an OSAL message buffer to build a transmission frame.
 *
 * input parameters
 *
 * @param       cmd - Command frame identifier.
 * @param       *sec - pointer to MAC security parameters
 *
 * output parameters
 *
 * None.
 *
 * @return      Pointer to allocated buffer or NULL if allocation failed.
 **************************************************************************************************
 */

MAC_INTERNAL_API macTx_t *macAllocTxBuffer(uint8 cmd, ApiMac_sec_t *sec)
{
  macTx_t       *pMsg;
  uint8         len;
  uint8         micLen;

  /* we have table of cmd frame size and sizeof(msg) indexed by cmd */
  len = macCmdBufSize[cmd];
#ifdef FEATURE_FREQ_HOP_MODE
  if ( MAC_FH_ENABLED && MAC_FH_GET_IE_LEN_FN )
  {
    uint16 ieLen = 0;
    /* buffer space for FH header IE's if any for this command */
    ieLen = MAC_FH_GET_IE_LEN_FN( macCmdIncludeFHIe[cmd] & MAC_FH_HEADER_IE_MASK);
    /* buffer space for FH Payload IE's if any for this command */
    ieLen += MAC_FH_GET_IE_LEN_FN( macCmdIncludeFHIe[cmd] & MAC_FH_PAYLOAD_IE_MASK);

    if ( ieLen )
    {
      len += ieLen + MAC_HEADER_IE_HEADER_LEN + MAC_PAYLOAD_IE_HEADER_LEN;
    }
  }
#endif

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif
      if (pMacPib->securityEnabled == TRUE && sec->securityLevel > MAC_SEC_LEVEL_NONE)
      {
        /* Save local copy */
        micLen = macAuthTagLen[sec->securityLevel];

        /* Add security header length as data offset.
         * Do not add MIC length as it is not part of security header.
         */
        len += (MAC_SEC_CONTROL_FIELD_LEN + MAC_FRAME_COUNTER_LEN + macKeySourceLen[sec->keyIdMode]);
      }
      else
      {
        micLen = 0;
      }
#if (defined TIMAC_ROM_IMAGE_BUILD)
    }
    else
    {
        micLen = 0;
    }
#endif
#else
  (void)sec;
  micLen = 0;
#endif /* FEATURE_MAC_SECURITY */

  /* allocation includes micLen including MAC_PHY_PHR_LEN.
   * Low level MAC will fill in PHR.
   */
#ifndef ENHACK_MALLOC
  if(cmd == MAC_INTERNAL_ENH_ACK)
  {
    if((MAC_PHY_PHR_LEN + len + micLen) > 250)
    {
      MAC_ASSERT(0);
    }
    else
    {
      pMsg = (macTx_t *)enhancedAckBuf;
    }
  }
#ifdef FEATURE_WISUN_EDFE_SUPPORT
  else if(cmd == MAC_INTERNAL_EDFE_EFRM)
  {
      if((MAC_PHY_PHR_LEN + len + micLen) > MAC_EDFE_MAX_FRAME)
      {
        MAC_ASSERT(0);
      }
      else
      {
        pMsg = (macTx_t *)edfeEfrmBuf;
      }
  }
#endif
  else
  {
    pMsg = (macTx_t *) MAP_osal_msg_allocate(MAC_PHY_PHR_LEN + len + micLen);
  }
#else
    pMsg = (macTx_t *) MAP_osal_msg_allocate(MAC_PHY_PHR_LEN + len + micLen);
#endif
  if (pMsg)
  {
    MAP_osal_memset( pMsg, 0x0, MAC_PHY_PHR_LEN + len + micLen);

    /* Initialize to zero the internal data structure */
    MAP_osal_memset(&(pMsg->internal), 0x0, sizeof(macTxIntData_t));

    /* Initialize the headerIeInfo and PayloadIeInfo */
    MAP_osal_memset( &(pMsg->internal.headerIeInfo), MAC_HEADER_IE_TERM,
                 sizeof(headerIeInfo_t) * MAC_HEADER_IE_MAX );
    MAP_osal_memset( &(pMsg->internal.payloadIeInfo), MAC_PAYLOAD_IE_TERM_GID,
                 sizeof(payloadIeInfo_t) * MAC_PAYLOAD_IE_MAX );

    /* set up data pointer to end of message (exclude MIC); to be built in reverse */
    pMsg->msdu.p = (uint8 *) pMsg + len;
    pMsg->msdu.len = micLen;
#ifndef FEATURE_WISUN_SUPPORT
    /* set command identifier */
    if ((cmd > MAC_INTERNAL_DATA) && (cmd < MAC_INTERNAL_COEXIST_EB_REQ))
    {
      *(--pMsg->msdu.p) = cmd - 1;
      pMsg->msdu.len++;
      len--;
    }
#endif
    pMsg->internal.totalLen = len;

    return pMsg;
  }
  /* alloc failed; try to alloc a smaller buffer for failure message */
  else
  {
    pMsg = (macTx_t *) MAP_osal_msg_allocate(sizeof(macEventHdr_t));
    if (pMsg)
    {
      /* treat as tx failure; send osal msg to MAC */
      pMsg->hdr.event = MAC_NO_ACTION_EVT;
      pMsg->hdr.status = MAC_NO_RESOURCES;
      MAP_osal_msg_send(macTaskId, (uint8 *) pMsg);
    }
    return NULL;
  }
}

/**************************************************************************************************
 * @fn          macBuildDataFrame
 *
 * @brief       This function builds and sends a data frame in buffer pEvent.
 *
 * input parameters
 *
 * @param       pEvent - Pointer to event data.
 *
 * output parameters
 *
 * None.
 *
 * @return      status.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macBuildDataFrame(macEvent_t *pEvent)
{
  uint8 status = MAC_SUCCESS;
  uint8 elideSeqNo = 0;

  if (MAC_FH_ENABLED)
  {
    /* set parameters */

    if (pEvent->dataReq.mac.dstAddr.addrMode == SADDR_MODE_NONE)
    {
      pEvent->tx.internal.frameType  = MAC_INTERNAL_DATA_BROADCAST;
      pEvent->tx.internal.be = (pMacDataTx->internal.txOptions & MAC_TXOPTION_ALT_BE) ? pMacPib->altBe : pMacPib->minBe;
      pEvent->tx.internal.be += 2;
    }
    else
    {
#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
      if(pEvent->dataReq.mac.includeFhIEs & MAC_FH_FC_IE)
      {
          elideSeqNo = 1;
#ifndef FEATURE_EDFE_TEST_MODE
          pEvent->tx.internal.frameType  = MAC_INTERNAL_DATA_EDFE;
#else
          pEvent->tx.internal.frameType  = MAC_INTERNAL_DATA_UNICAST;
#endif
          pEvent->tx.internal.be = (pMacDataTx->internal.txOptions & MAC_TXOPTION_ALT_BE) ? pMacPib->altBe : pMacPib->minBe;
          macEdfeInfo.ieInfo.txfc = 0;
          macEdfeInfo.ieInfo.rxfc = MAC_EDFE_MAX_RXFC;
      }
      else
#endif
      {
          pEvent->tx.internal.frameType  = MAC_INTERNAL_DATA_UNICAST;
          pEvent->tx.internal.be = (pMacDataTx->internal.txOptions & MAC_TXOPTION_ALT_BE) ? pMacPib->altBe : pMacPib->minBe;
      }
    }
#ifdef FEATURE_WISUN_SUPPORT
    uint16_t mpxId = 0;
    if((pEvent->dataReq.mac.includeFhIEs & MAC_FH_MPX_IE) && pEvent->dataReq.mac.pIEList)
    {
        uint8_t *mpxPtr = pEvent->dataReq.mac.pIEList;
        mpxId = (mpxPtr[3]) | ((mpxPtr[4]) << 8);
    }

    if(mpxId == MPX_ID_KMP)
    {
        pEvent->tx.internal.fhFrameType  = MAC_FH_FRAME_EAPOL;
    }
    else
    {
        pEvent->tx.internal.fhFrameType  = MAC_FH_FRAME_DATA;
    }
#else
    pEvent->tx.internal.fhFrameType  = MAC_FH_FRAME_DATA;
#endif
    pEvent->tx.internal.nb = 0;
    MAP_osal_memcpy(&pEvent->tx.internal.dest.dstAddr, &pEvent->dataReq.mac.dstAddr, sizeof(sAddr_t));
    //MAP_osal_memcpy(pEvent->tx.internal.dest.destAddr, pEvent->dataReq.mac.dstAddr.addr.extAddr, SADDR_EXT_LEN);
  }

  pEvent->tx.internal.txOptions  = pEvent->dataReq.mac.txOptions;
  pEvent->tx.internal.channel    = pEvent->dataReq.mac.channel;
  pEvent->tx.internal.power      = pEvent->dataReq.mac.power;

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif
      /* If there is no IE, only the legacy payload will be secured. */
      pEvent->tx.internal.ptrMData = pEvent->tx.msdu.p;
#if (defined TIMAC_ROM_IMAGE_BUILD)
    }
#endif
#endif

  if ( MAC_PAYLOAD_IE_TERM_GID != pEvent->tx.internal.payloadIeInfo[0].gId )
  {
#ifdef FEATURE_WISUN_SUPPORT
    bool includeTermIe = (0 == pEvent->dataReq.mac.payloadIELen)?TRUE:FALSE;
#else
    bool includeTermIe = 0; // not needed because WiSUN has MPX-IE
#endif
    /* Builds MAC specific payload IE's if any and also adds the payload termination
       IE if requested */
    status = MAP_macBuildPayloadIes(&pEvent->tx, pEvent->tx.internal.payloadIeInfo,
                                includeTermIe, FALSE);
  }

  if (status != MAC_SUCCESS)
  {
    return status;
  }

  if ( (MAC_HEADER_IE_TERM != pEvent->tx.internal.headerIeInfo[0].elementId) ||
       (NULL != pEvent->tx.internal.ptrPayloadIE) )
  {
    /* Builds the MAC specific header IE's if any and also adds the header
       termination IE */
    status = MAP_macBuildHeaderIes(&pEvent->tx, pEvent->tx.internal.headerIeInfo);
  }

  if (status != MAC_SUCCESS)
  {
    return status;
  }

  /* build header */
  status = MAP_macBuildHeader(&pEvent->tx, pEvent->dataReq.mac.srcAddrMode,
                          &pEvent->dataReq.mac.dstAddr, pEvent->dataReq.mac.dstPanId, elideSeqNo);

  if (status == MAC_SUCCESS)
  {
    /* check for the maximum length
     * if the msdu.len is greater than MAX length return error
     *
     */
    if (pEvent->tx.msdu.len > macCfg.macMaxFrameSize)
    { /* TX data packet is too big, just drop */
      macTx_t *pMsg;

      pMsg = &pEvent->tx;
      pMsg->hdr.status = MAC_FRAME_TOO_LONG;
      MAP_macDataTxComplete(pMsg);
    }
    else
    {
      DBG_PRINT0(DBGSYS, "Data Enqueued");

      /* queue frame for transmission */
      MAP_macDataTxEnqueue(&pEvent->tx);
    }
  }
  return status;
}

/**************************************************************************************************
 * @fn          macDataTxComplete
 *
 * @brief       This function is called when a data transmission attempt is complete.  If this
 *              is for a non-internal data frame the MAC callback is executed directly.
 *              Otherwise the appropriate TX event is sent as an OSAL message to the MAC.
 *
 * input parameters
 *
 * @param       pMsg - Pointer message that was transmitted.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macDataTxComplete(macTx_t *pMsg)
{
  /* if data frame call callback directly; this must be done so that the application
   * can free the buffer
   */
  if (((pMsg->internal.frameType == MAC_INTERNAL_DATA) ||
       (pMsg->internal.frameType == MAC_INTERNAL_DATA_UNICAST) ||
       (pMsg->internal.frameType == MAC_INTERNAL_DATA_BROADCAST) ||
#ifndef FEATURE_WISUN_EDFE_SUPPORT
       (pMsg->internal.frameType == MAC_INTERNAL_ASYNC) ) &&
#else
      (pMsg->internal.frameType == MAC_INTERNAL_ASYNC) ||
      (pMsg->internal.frameType == MAC_INTERNAL_DATA_EDFE) ) &&
#endif
      !(pMsg->internal.txMode & MAC_TX_MODE_INTERNAL))
  {
    if(pMacPib->enDataAckPending)
    {
      if ( pMsg->hdr.status == MAC_ACK_PENDING)
      {
        /* turn off receiver */
        MAP_macRxSoftEnable(MAC_RX_POLL);
        /*set frame response timer to wait for the pending data */
        MAP_macSetEvent16(MAC_TX_DATA_REQ_TASK_EVT);
      }
      pMsg->hdr.status = MAC_SUCCESS;
    }
    MAP_macCbackForEvent((macEvent_t *) pMsg, pMsg->hdr.status);
    return;
  }
  /* no action for internal data frames */
  else if (((pMsg->internal.frameType == MAC_INTERNAL_DATA) ||
      (pMsg->internal.frameType == MAC_INTERNAL_DATA_UNICAST)) &&
          (pMsg->internal.txMode & MAC_TX_MODE_INTERNAL))
  {
    pMsg->hdr.event = MAC_NO_ACTION_EVT;
  }
  else if (pMsg->hdr.status == MAC_SUCCESS)
  {
    pMsg->hdr.event = MAC_NO_ACTION_EVT;
  }
  else
  {
    pMsg->hdr.event = MAC_NO_ACTION_EVT;
  }

  /* send event to mac */
  MAP_osal_msg_send(macTaskId, (uint8 *) pMsg);
}

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
#if !defined(FEATURE_WISUN_EDFE_SUPPORT) && !defined(FEATURE_WISUN_MIN_EDFE)
      if ( pHeaderIes[i].subIdBitMap && MAC_FH_RSL_IE )
      {
          FHIE_ieInfo_t rslVal;
          int8_t rsl8 = rxFhRsl;
          int16_t rsl16 = rsl8;
          /* adjust rssi value from -127 ~ +127 to -174 ~ +80 */
          if(rsl8 == MAC_RSSI_NOT_MEASURED)
          {
              rsl16 = MAC_RSL_NOT_MEASURED;
          }
          else if(rsl16 > MAC_RSL_MAX)
          {
              rsl16 = MAC_RSL_MAX;
          }
          /* adjust rssi value to rsl : 0 ~ +255 */
          rsl16 += MAC_RSL_THERMAL_NOISE;
          /* cast to 8 bit value */
          rsl8 = rsl16 & 0xFF;
          rslVal.rslIe.rsl = rsl8;
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
#else
      FHIE_ieInfo_new_t *pIeInfo = &macEdfeInfo.ieInfo;

      pIeInfo->fcIe.txFlowControl = 0;
      pIeInfo->fcIe.rxFlowControl = 0;
      pIeInfo->rslIe.rsl = 0;

      if ( pHeaderIes[i].subIdBitMap & MAC_FH_RSL_IE )
      {
          int8_t rsl8 = rxFhRsl;
          int16_t rsl16 = rsl8;
          /* adjust rssi value from -127 ~ +127 to -174 ~ +80 */
          if(rsl8 == MAC_RSSI_NOT_MEASURED)
          {
              rsl16 = MAC_RSL_NOT_MEASURED;
          }
          else if(rsl16 > MAC_RSL_MAX)
          {
              rsl16 = MAC_RSL_MAX;
          }
          /* adjust rssi value to rsl : 0 ~ +255 */
          rsl16 += MAC_RSL_THERMAL_NOISE;
          /* cast to 8 bit value */
          rsl8 = rsl16 & 0xFF;
          pIeInfo->rslIe.rsl = rsl8;
      }

      if ( pHeaderIes[i].subIdBitMap & MAC_FH_FC_IE )
      {
          pIeInfo->fcIe.txFlowControl = pIeInfo->txfc;
          pIeInfo->fcIe.rxFlowControl = pIeInfo->rxfc;
      }

      if ( 0 == MAC_FH_GENERATE_IE_FN(pBuf->msdu.p, pHeaderIes[i].subIdBitMap, pBuf, (FHIE_ieInfo_t *) pIeInfo))
      {
        status = MAC_NO_DATA;
        return status;
      }
#endif
      pBuf->msdu.len += ieLen;
      pBuf->internal.headerIesLen += ieLen;
    }
  }

  pBuf->internal.ptrHeaderIE = pBuf->msdu.p;
  return status;
}

/**
 * @brief Updates the header IE pointer if header IE is present in macRx_t
 *        structure with the start position of header IE.
 *        It also updates the headerIeLen fields.
 *
 * @param pMsg pointer to the macRx_t containing the incoming decrypted message.
 * @return None
 */
uint8_t dbg_fc_ind = 0;
uint32_t dbg_fc[10] = {0};

void
macUpdateHeaderIEPtr(macRx_t *pMsg)
{
  uint8* ptr;
  uint8 ieContentLen;
  uint16  ieLen = 0;
  uint8 micLen = 0;

  MAC_ASSERT( NULL != pMsg );

#ifdef FEATURE_MAC_SECURITY
  micLen = macAuthTagLen[pMsg->sec.securityLevel];
#endif

  if ( pMsg->internal.fcf & MAC_FCF_IE_PRESENT_MASK )
  {
    ptr = (uint8 *)pMsg->msdu.p;

    if (MAC_HEADER_IE_TYPE(ptr) == MAC_HEADER_IE_TYPE_VAL )
    {
      /* header IE present, update the header ie pointer */
      pMsg->pHdrIes = ptr;
#ifdef FEATURE_WISUN_EDFE_SUPPORT
      if((*ptr == MAC_FC_IE_HDR_BYTE1) && (*(ptr+MAC_FC_IE_HDR_BYTE2_OFFSET) == MAC_FC_IE_HDR_BYTE2)
      && (*(ptr+MAC_FC_IE_HDR_SUBID_OFFSET) == MAC_FC_IE_HDR_SUBID))
      {
          if(macEdfeInfo.txFrameType)
          {
//              uint8 txfc = *(ptr+MAC_FC_IE_TXFC_OFFSET);
              uint8 rxfc = *(ptr+MAC_FC_IE_RXFC_OFFSET);
              if(!rxfc)
              {
                  macEdfeInfo.rxFrameType = MAC_FRAME_TYPE_EDFE_FFRM;
              }
              else
              {
                macEdfeInfo.rxFrameType = MAC_FRAME_TYPE_EDFE_RFRM;
              }
          }
          else
          {
            macEdfeInfo.rxFrameType = MAC_FRAME_TYPE_EDFE_IFRM;
          }
      }
#endif
#ifdef FEATURE_WISUN_MIN_EDFE
      // check for FC-IE and the content of FC-IE to confirm it is not a final frame (0,0)
      if((*ptr == MAC_FC_IE_HDR_BYTE1) && (*(ptr+MAC_FC_IE_HDR_BYTE2_OFFSET) == MAC_FC_IE_HDR_BYTE2)
            && (*(ptr+MAC_FC_IE_HDR_SUBID_OFFSET) == MAC_FC_IE_HDR_SUBID))
      {
          uint8 txfc = *(ptr+MAC_FC_IE_TXFC_OFFSET);
          uint8 rxfc = *(ptr+MAC_FC_IE_RXFC_OFFSET);
          if((txfc == 0) && (rxfc == 0))
          {
              macEdfeInfo.rxFrameType = MAC_FRAME_TYPE_EDFE_FFRM;
          }
          else
          {
#ifdef FH_HOP_DEBUG
    FH_Dbg.numEDFEFrRcvd++;
    //tr_info("Rcvd EDFE frame of length = %d with txfc = %d , rxfc = %d", pMsg->msdu.len, txfc, rxfc);
    dbg_fc_ind = dbg_fc_ind + 1;
    dbg_fc_ind = dbg_fc_ind % 10;
    dbg_fc[dbg_fc_ind] = ((pMsg->msdu.len) << 16) | (txfc << 8) | rxfc;
#endif
              //consider adding a condition to identify malformed pkt if txfc != 0 but rxfc == 0
              macEdfeInfo.rxFrameType = MAC_FRAME_TYPE_EDFE; //some valid EDFE frame which is not a FRFRM
          }
      }
#endif

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
          break;
        }
        else if ( MAC_INTERNAL_HEADER_IE_TERM_NO_PAYIE == MAC_HEADER_IE_ELEMENT_ID(ptr) )
        {
          /* Termination IE */
          pMsg->payloadIePresent = 0;
          /* Update total header ie length */
          ieLen += MAC_HEADER_IE_HEADER_LEN;
          break;
        }

        /* Get Header IE content len */
        ieContentLen = MAC_HEADER_IE_CONTENT_LEN(ptr);
        /* Update length and pointer */
        ieLen += MAC_HEADER_IE_HEADER_LEN + ieContentLen;
        ptr += MAC_HEADER_IE_HEADER_LEN + ieContentLen;

      } while(ieLen < pMsg->msdu.len - micLen );

      if (ieLen <= pMsg->msdu.len - micLen)
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
    macMgmtAction1[1] = MAP_macApiDataReq;
    macMgmtAction1[2] = MAP_macDataSend;
    macMgmtAction1[3] = MAP_macDataRxInd;
    macMgmtAction1[4] = MAP_macApiWSAsyncReq;
    macMgmtAction1[5] = MAP_macTxCsmaDelay;
    macMgmtAction1[6] = macTxBackoffHandler;
    macMgmtAction1[7] = macRxBackoffHandler;

  /* initialize macMgmt struct */
  macPanCoordinator = FALSE;
#ifdef FEATURE_WISUN_EDFE_SUPPORT
  macEdfeInfo.initiator = FALSE;
  macEdfeInfo.asyncPending = FALSE;
  memset(&macEdfeInfo.ieInfo, 0, sizeof(FHIE_ieInfo_new_t));
#endif
}

/**************************************************************************************************
 * @fn          macTxBackoffHandler
 *
 * @brief       This action function handles Tx backoff timer expiry.
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
MAC_INTERNAL_API void macTxBackoffHandler(macEvent_t *pEvent)
{
  (void)pEvent;  // Intentionally unreferenced parameter

  /* Attempt to re-schedule Tx command, if necessary. Treat failure as NO_ACK */
  DBG_PRINT0(DBGSYS, "MAC Execute Backoff(Tx)");

  txComplete(MAC_NO_ACK);

  return;
}
/**************************************************************************************************
 * @fn          macRxBackoffHandler
 *
 * @brief       This action function handles Rx backoff timer expiry.
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
MAC_INTERNAL_API void macRxBackoffHandler(macEvent_t *pEvent)
{
  (void)pEvent;  // Intentionally unreferenced parameter

  /* Attempt to reschedule Rx command */
  DBG_PRINT0(DBGSYS, "MAC Execute Backoff(Rx)");

  macRxOnRequest();

  return;
}

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
  uint8   srcAddrModeEdfe = SADDR_MODE_NONE;

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

#ifdef FEATURE_EDFE_TEST_MODE
  if(switchToEDFE)
  {
      srcAddrModeEdfe = SADDR_MODE_EXT;
  }
#endif
#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
  if(pBuf->internal.frameType == MAC_INTERNAL_DATA_EDFE)
  {
      srcAddrMode = srcAddrModeEdfe;
  }
#endif

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

#ifndef FEATURE_WISUN_SUPPORT
  if (pBuf->internal.frameType > MAC_INTERNAL_ENHANCED_BEACON_REQ)
#else
  if (pBuf->internal.frameType >= MAC_INTERNAL_ENH_ACK)
#endif
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
    MAP_sAddrExtRevCpy(p, pMacPib->extendedAddress.addr.extAddr);
    p--;
  }

  /* Pan Id Compression */
#ifdef FEATURE_WISUN_SUPPORT
    if((pBuf->internal.frameType == MAC_INTERNAL_DATA_EDFE) || (pBuf->internal.frameType == MAC_INTERNAL_EDFE_EFRM)
        || (pBuf->internal.frameType == MAC_INTERNAL_DATA_UNICAST) || (pBuf->internal.frameType == MAC_INTERNAL_ENH_ACK)
        || (pBuf->internal.fhFrameType == MAC_FH_FRAME_PAN_ADVERT_SOLICIT))
    {
        fcf |= MAC_FCF_INTRA_PAN_MASK;
    }
#endif

  /* source PAN ID */
  if(frameVersion == MAC_FRAME_VERSION_2)
  {
    if(!(fcf & MAC_FCF_INTRA_PAN_MASK) && (dstAddrMode == SADDR_MODE_NONE) && (srcAddrMode != SADDR_MODE_NONE))
    {
       *p-- = HI_UINT16(srcPanId);
       *p-- = LO_UINT16(srcPanId);
    }
  }
  else
  {
      return (MAC_UNSUPPORTED);
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
    MAP_sAddrExtRevCpy(p, pDstAddr->addr.extAddr);
    p--;
  }

  /* destination PAN ID */
  if ( (dstAddrMode != SADDR_MODE_NONE) && !(fcf & MAC_FCF_INTRA_PAN_MASK))
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
#if !defined(FEATURE_WISUN_EDFE_SUPPORT) && !defined(FEATURE_WISUN_MIN_EDFE)
        if ((!MAC_FH_ENABLED) || ((pBuf->internal.frameType == MAC_INTERNAL_ENH_ACK)))
#else
        if ((!MAC_FH_ENABLED) || ((pBuf->internal.frameType == MAC_INTERNAL_ENH_ACK)
         || (pBuf->internal.frameType == MAC_INTERNAL_EDFE_EFRM)))
#endif
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
#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
  uint8_t txfc = 0, rxfc = 0;
  uint32 txOffset = 0;
  uint32 defHie = 0;
  uint8 cmdType = cmd;

  if(cmd == MAC_INTERNAL_DATA_EDFE_IFRM)
  {
      txfc = macEdfeInfo.txfc;
      rxfc = macEdfeInfo.rxfc;
      cmd = MAC_INTERNAL_EDFE_EFRM;
      txOffset = macEdfeInfo.txStartTime/1000; //msec
  }
  else if(cmd == MAC_INTERNAL_DATA_EDFE_RFRM)
  {
      txfc = 0;
      rxfc = MAC_EDFE_MAX_RXFC;
      cmd = MAC_INTERNAL_EDFE_EFRM;
      defHie = MAC_FH_RSL_IE;
  }
  else if(cmd == MAC_INTERNAL_DATA_EDFE_FFRM)
  {
      txfc = 0;
      rxfc = 0;
      cmd = MAC_INTERNAL_EDFE_EFRM;
      defHie = MAC_FH_RSL_IE;
  }
  else
  {
      txfc = 0;
      rxfc = 0;
  }
#endif
  /* allocate buffer */
  if ((pMsg = MAP_macAllocTxBuffer(cmd, sec)) != NULL)
  {
    /* set parameters */
#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
    macEdfeInfo.ieInfo.txfc = txfc;
    macEdfeInfo.ieInfo.rxfc = rxfc;
    macEdfeInfo.ieInfo.txOffset = txOffset;
#endif
    pMsg->internal.txOptions = txOptions;
    pMsg->internal.frameType = cmd;
    /* set security */
    MAP_osal_memcpy(&pMsg->sec, sec, sizeof(ApiMac_sec_t));

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif

#if !defined(FEATURE_WISUN_EDFE_SUPPORT) && !defined(FEATURE_WISUN_MIN_EDFE)
    if ( (MAC_INTERNAL_ZERO_DATA != cmd) && (MAC_INTERNAL_ENH_ACK != cmd) )
#else
    if ( (MAC_INTERNAL_ZERO_DATA != cmd) && (MAC_INTERNAL_ENH_ACK != cmd) && (MAC_INTERNAL_EDFE_EFRM != cmd))
#endif
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
#if defined(FEATURE_WISUN_EDFE_SUPPORT) || defined(FEATURE_WISUN_MIN_EDFE)
    /* When Freq. hopping is enabled, enhanced ack is processed here */
    if( MAC_FH_ENABLED && (MAC_INTERNAL_EDFE_EFRM == cmd) )
    {
      if(cmdType == MAC_INTERNAL_DATA_EDFE_IFRM)
      {
          srcAddrMode = SADDR_MODE_EXT;
      }
      else
      {
          srcAddrMode = SADDR_MODE_NONE;
      }
      pMsg->internal.fhFrameType = MAC_FH_FRAME_DATA;

      /* build the header IE's if part of the ACK */
      /* Check if ACK has another IEs other than BT-IE */
      if ( macCmdIncludeFHIe[cmd] & (MAC_FH_HEADER_IE_MASK & ~MAC_FH_BT_IE) )
      {
        pMsg->internal.headerIeInfo[0].elementId = MAC_HEADER_IE_WISUN;
        pMsg->internal.headerIeInfo[0].subIdBitMap =
          macCmdIncludeFHIe[cmd] & MAC_FH_HEADER_IE_MASK;
        pMsg->internal.headerIeInfo[0].subIdBitMap |= defHie;

        if(!macPanCoordinator)
        {
            pMsg->internal.headerIeInfo[0].subIdBitMap &= ~MAC_FH_BT_IE;
        }
        MAP_macBuildHeaderIes(pMsg, pMsg->internal.headerIeInfo);
      }

      /* Prepare the EDFE transaction frame to be send directly in low level MAC */
      status = MAP_macBuildHeader(pMsg, srcAddrMode, pDstAddr, dstPanId, 1);
      if (status == MAC_SUCCESS)
      {
          macRadioTxEDFEEfrmCb(pMsg);
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
#endif
#endif /*FEATURE_FREQ_HOP_MODE */
    status = MAP_macBuildHeader(pMsg, srcAddrMode, pDstAddr, dstPanId, 0);
    if (status == MAC_SUCCESS)
    {
      /* queue frame for transmission */
      MAP_macDataTxEnqueue(pMsg);
    }
  }

  return status;
}

/**************************************************************************************************
 * @fn          macApiDataReq
 *
 * @brief       This function handles an API data request event.  First the parameters are checked.
 *              If any parameters are invalid it sets hdr.status to MAC_INVALID_PARAMETER
 *              and calls the MAC callback function directly.  If ok, it calls
 *              macBuildDataFrame() to build and send a data frame.
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
MAC_INTERNAL_API void macApiDataReq(macEvent_t *pEvent)
{
  uint8   status = MAC_SUCCESS;

  /* check parameters */
  MAC_PARAM_STATUS((macMain.featureMask & MAC_FEAT_COORD_MASK) ||
                   ((pEvent->dataReq.mac.txOptions & MAC_TXOPTION_INDIRECT) == 0), status);
  MAC_PARAM_STATUS((macMain.featureMask & MAC_FEAT_GTS_MASK) ||
                   ((pEvent->dataReq.mac.txOptions & MAC_TXOPTION_GTS) == 0), status);
  MAC_PARAM_STATUS(pEvent->dataReq.msdu.len <= macCfg.macMaxFrameSize, status);

  if ((pEvent->dataReq.mac.srcAddrMode == SADDR_MODE_NONE) &&
      (pEvent->dataReq.mac.dstAddr.addrMode == SADDR_MODE_NONE))
  {
    status = MAC_INVALID_ADDRESS;
  }

  if (status == MAC_SUCCESS)
  {
    if ( MAC_FH_ENABLED )
    {
      if ( pEvent->dataReq.mac.includeFhIEs & MAC_FH_HEADER_IE_MASK )
      {
        pEvent->dataReq.internal.headerIeInfo[0].elementId = MAC_HEADER_IE_WISUN;
        pEvent->dataReq.internal.headerIeInfo[0].subIdBitMap =
          pEvent->dataReq.mac.includeFhIEs & MAC_FH_HEADER_IE_MASK;
      }

      if ((pEvent->dataReq.mac.includeFhIEs & (~MAC_FH_MPX_IE)) & MAC_FH_PAYLOAD_IE_MASK )
      {
        pEvent->dataReq.internal.payloadIeInfo[0].gId = MAC_PAYLOAD_IE_WISUN_GID;
        pEvent->dataReq.internal.payloadIeInfo[0].subId.subIdBMap =
          pEvent->dataReq.mac.includeFhIEs & MAC_FH_PAYLOAD_IE_MASK;
      }
    }

    if (status == MAC_SUCCESS)
    {
      status = MAP_macBuildDataFrame(pEvent);
    }
  }

  if (status != MAC_SUCCESS)
  {
    DBG_PRINT1(DBGSYS, "Failed Status= 0x%X", status);
    MAP_macCbackForEvent(pEvent, status);
  }
}

/**************************************************************************************************
 * @fn          MAC_McpsDataReq
 *
 * @brief       This function sends application data to the MAC for transmission in a MAC data
 *              frame.  The function sets the event to MAC_API_DATA_REQ_EVT and sends the
 *              parameters as an OSAL message to the MAC.  The message buffer containing the
 *              parameters has already been allocated by the application.
 *
 * input parameters
 *
 * @param       pData - Pointer to parameters structure.
 *
 * output parameters
 *
 * None.
 *
 * @return      MAC_SUCCESS if successful otherwise error status.
 *              MAC_NO_RESOURCES if unable to allocate memory.
 *              MAC_TRANSACTION_OVERFLOW if the tx queue has reached the limit.
 **************************************************************************************************
 */
extern uint16_t convertTxOptions(ApiMac_txOptions_t txOptions);
extern void copyApiMacAddrToMacAddr(sAddr_t *pDst, ApiMac_sAddr_t *pSrc);
uint8 MAC_McpsDataReq(ApiMac_mcpsDataReq_t* pData)
{
  macMcpsDataReq_t *pMsg;

  /* check if correct power value is selected */
  if (pData->txOptions.usePowerAndChannel == true)
  {
      if (MAP_macGetRadioTxPowerReg(pData->power) == MAC_RADIO_TX_POWER_INVALID)
      {
        return MAC_INVALID_PARAMETER;
      }
  }

  if ( MAC_FH_ENABLED )
  {
      if( (pData->dstAddr.addrMode == SADDR_MODE_NONE) || (pData->dstAddr.addrMode == SADDR_MODE_EXT) )
      {
        /* FH UT IE + FH BT IE*/
        pData->includeFhIEs |= (MAC_FH_UT_IE | MAC_FH_BT_IE);
        if(pData->includeFhIEs & MAC_FH_FC_IE)
        {
          pData->includeFhIEs |= MAC_FH_RSL_IE;
        }
      }
  }

  if (((FALSE == pData->txOptions.indirect) &&
       (macCfg.txDataMax > macData.directCount)) ||
      ((TRUE == pData->txOptions.indirect) &&
       (macCfg.txDataMax > macData.indirectCount)))
  {

#ifdef FEATURE_EDFE_TEST_MODE
        // switch to EDFE based on global variable for data packet
        if((switchToEDFE) && (pData->includeFhIEs & MAC_FH_MPX_IE) && (pData->dstAddr.addrMode != SADDR_MODE_NONE))
        {
            pData->includeFhIEs |= MAC_FH_FC_IE;
            //pData->srcAddrMode = ApiMac_addrType_none;
        }
#endif

    pMsg = MAP_MAC_McpsDataAlloc(pData->msdu.len, pData->sec.securityLevel,
                             pData->sec.keyIdMode,
                             pData->includeFhIEs,
                             pData->payloadIELen);
    if(pMsg != NULL)
    {
      /* Fill in the message content */
      pMsg->hdr.event = MAC_API_DATA_REQ_EVT;
      pMsg->hdr.status = MAC_SUCCESS;

      //pMsg->msdu.len = pData->msdu.len;
      MAP_osal_memcpy(pMsg->msdu.p, pData->msdu.p, pData->msdu.len);
      MAP_osal_memcpy(&(pMsg->sec), &(pData->sec), sizeof( ApiMac_sec_t));

      MAP_copyApiMacAddrToMacAddr(&(pMsg->mac.dstAddr), &(pData->dstAddr));
      pMsg->mac.dstPanId = pData->dstPanId;
      pMsg->mac.srcAddrMode = pData->srcAddrMode;
      pMsg->mac.msduHandle = pData->msduHandle;
      pMsg->mac.txOptions = MAP_convertTxOptions(pData->txOptions);

#ifdef FEATURE_EDFE_TEST_MODE
        // switch to EDFE based on global variable for data packet
        if((switchToEDFE) && (pData->includeFhIEs & MAC_FH_MPX_IE) && (pData->dstAddr.addrMode != SADDR_MODE_NONE))
        {
            pMsg->mac.txOptions &= ~MAC_TXOPTION_ACK;
            pMsg->mac.txOptions |= MAC_TXOPTION_NO_RETRANS;
        }
#endif
      pMsg->mac.channel = pData->channel;
      pMsg->mac.power = pData->power;
      pMsg->internal.gpDuration = pData->gpDuration;
      pMsg->mac.payloadIELen = pData->payloadIELen;
      pMsg->mac.fhProtoDispatch = pData->fhProtoDispatch;
      pMsg->mac.includeFhIEs = pData->includeFhIEs;

      if(0 == (pMsg->mac.includeFhIEs & MAC_FH_MPX_IE)) /* non wisun case */
      {
          /* add payload termination IE if layers above MAC have sent payload IEs*/
          if ( pMsg->mac.payloadIELen && (NULL != pMsg->mac.pIEList) )
          {
              /* Fill the Payload termination IE*/
              *(--pMsg->msdu.p) = MAC_PAYLOAD_IE_TERM_SECOND_BYTE_PAYIE;
              *(--pMsg->msdu.p) = MAC_PAYLOAD_IE_TERM_FIRST_BYTE;

              /* move the msdu.p pointer appropriately and update the length field */
              pMsg->msdu.len += MAC_PAYLOAD_IE_HEADER_LEN;
           }
      }
      /* else: for wisun case no need to add pay load termination IE */


      if(pMsg->mac.payloadIELen)
      {
          /* copy the payLoadIE List from layer above MAC into the transmit buffer*/
          MAP_osal_memcpy(pMsg->mac.pIEList, pData->pIEList, pMsg->mac.payloadIELen);

          /* move the msdu.p pointer appropriately and update the length field */
          pMsg->msdu.p -= pMsg->mac.payloadIELen;
          pMsg->msdu.len += pMsg->mac.payloadIELen;

          /* update the payload ie ptr */
          pMsg->internal.ptrPayloadIE = pMsg->msdu.p;
      }

      MAP_osal_msg_send(macTaskId, (uint8 *) pMsg);

      return MAC_SUCCESS;
    }
    else
    {
      return MAC_NO_RESOURCES;
    }
  }
  else
  {
    return MAC_TRANSACTION_OVERFLOW;
  }
}

/**
 * @brief       This direct-execute function simplifies the allocation and
 *              preparation of the data buffer MAC_McpsDataReq().  The
 *              function allocates a buffer and prepares the data pointer.
 *
 * @param       len - Length of the application data.
 * @param       securityLevel - security suire used for this frame.
 * @param       keyIdMode - Key Id mode used for this frame.
 *                  Possible values are:
 *                  MAC_KEY_ID_MODE_NONE        0x00
 *                  MAC_KEY_ID_MODE_IMPLICIT    0x00
 *                  MAC_KEY_ID_MODE_1           0x01
 *                  MAC_KEY_ID_MODE_4           0x02
 *                  MAC_KEY_ID_MODE_8           0x03
 *                  Set to MAC_KEY_ID_MODE_NONE if security is
 *                  not used.
 * @param       includeFhIEs - bitmap indicating which FH IE's
 *                           need to be included.
 * @param       payloadIeLen - length of the application payload IE's if any.
 *                             This function will allocate the buffer for the
 *                             payload IE's and set the data element pIEList
 *                             of the macDataReq_t in macMcpsDataReq_t
 *                             appropriately.
 *
 * @return      Returns a pointer to the allocated buffer. If
 *              the function fails for any reason it returns
 *              NULL.
 */
macMcpsDataReq_t *MAC_McpsDataAlloc(uint16 len, uint8 securityLevel, uint8 keyIdMode,
                                    uint32 includeFhIEs, uint16 payloadIeLen)
{
  macMcpsDataReq_t *pData;
  uint16 offset;
  uint8 micLen = 0;

  /* MAC_DATA_OFFSET is assuming the worst case */
  offset = sizeof(macMcpsDataReq_t) + MAC_DATA_OFFSET;

  if (includeFhIEs)
  {
    if (MAC_FH_ENABLED && MAC_FH_GET_IE_LEN_FN)
    {
      int16 ieLen = 0;

      /* allocate space for the freq hopping ie's to be populated by MAC (strictly)*/

      /* space for FH header ie's if any */
      ieLen = MAC_FH_GET_IE_LEN_FN( includeFhIEs & MAC_FH_HEADER_IE_MASK );

      /* space for FH header Termination IE if any */

      if(includeFhIEs & MAC_FH_MPX_IE) /*Wisun Case*/
      {
          /*pay load termination IE will not be there if mpxIE is there*/
          /* but header termination IE is a must */
          ieLen += MAC_HEADER_IE_HEADER_LEN;
      }
      else /* non-Wisun Case */
      {
          /* insert code to check if header Termination IE is needed or not */
      }

      /* space for FH payload ie's if any */
      ieLen += MAC_FH_GET_IE_LEN_FN( includeFhIEs & MAC_FH_PAYLOAD_IE_MASK );

      offset += ieLen;
    }
  }

  /* if payload ies will be included allocate buffer for the termination ie's */
  /* for wisun case : strictly no need for a payload Termination IE*/
  if(0 == (includeFhIEs & MAC_FH_MPX_IE)) /*non-Wisun Case*/
  {
      /* insert code to check if payload Termination IE is needed or not */
      /* adjust payloadIeLen if need be */
      //payloadIeLen += MAC_PAYLOAD_IE_HEADER_LEN;
  }

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif
      if (pMacPib->securityEnabled == TRUE && securityLevel > MAC_SEC_LEVEL_NONE)
      {
        /* Save local copy */
        micLen = macAuthTagLen[securityLevel];

        /* Add security header length as data offset.
         * Do not add MIC length as it is not part of security header.
         */
        offset += (MAC_SEC_CONTROL_FIELD_LEN + MAC_FRAME_COUNTER_LEN + macKeySourceLen[keyIdMode]);
      }

#if (defined TIMAC_ROM_IMAGE_BUILD)
    }
#endif
#else
  (void)keyIdMode;
  (void)securityLevel;
#endif /* FEATURE_MAC_SECURITY */

  /* Allocation including security header length, MIC length, and MAC_PHY_PHR_LEN.
   * Low level MAC will fill in PHR.
   */
  pData = (macMcpsDataReq_t *)
  MAP_osal_msg_allocate(MAC_PHY_PHR_LEN + len + payloadIeLen + offset + micLen);

  if (pData)
  {
    pData->msdu.len = len + micLen;
    pData->msdu.p = (uint8 *) pData + offset + payloadIeLen + MAC_PHY_PHR_LEN;

    if ( payloadIeLen )
    {
      pData->mac.pIEList = (uint8 *)pData + offset + MAC_PHY_PHR_LEN;
    }
    else
    {
      pData->mac.pIEList = NULL;
    }

    /* Initialize to zero the internal data structure */
    MAP_osal_memset(&(pData->internal), 0x0, sizeof(macTxIntData_t));

    pData->internal.totalLen = len + offset + payloadIeLen + MAC_PHY_PHR_LEN + micLen;

    /* Initialize the headerIeInfo and PayloadIeInfo */
    MAP_osal_memset( &(pData->internal.headerIeInfo), MAC_HEADER_IE_TERM,
                 sizeof(headerIeInfo_t) * MAC_HEADER_IE_MAX );
    MAP_osal_memset( &(pData->internal.payloadIeInfo), MAC_PAYLOAD_IE_TERM_GID,
                 sizeof(payloadIeInfo_t) * MAC_PAYLOAD_IE_MAX );

    return pData;
  }
  else
  {
    return NULL;
  }
}


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

  /* pMacDataTx can be NULL when preempted by EDFE.
   * In this case, pMacDataTx was stored back into
   * macData.txQueue. Just return and get the packet
   * from the queue before retry. */
  if(pMacDataTx)
  {
    MAP_txCsmaDelay(0);
  }
  return;
}

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
  fhObject.pFhResetFn = MAP_FHAPI_reset;
  fhObject.pFhStartFn = MAP_FHAPI_start;
  fhObject.pFhStartBsFn = MAP_FHAPI_startBS;
  fhObject.pFhStopAsyncFn = MAP_FHAPI_stopAsync;
  fhObject.pFhSendDataFn = MAP_FHAPI_sendData;
  fhObject.pFhGetIeLenFn = MAP_FHIE_getLen;
  fhObject.pFhGenIeFn = MAP_FHIE_gen;
  fhObject.pFhParsePieFn = MAP_FHIE_parsePie;
  fhObject.pFhExtractHieFn = MAP_FHIE_extractHie;
  fhObject.pFhExtractPieFn = MAP_FHIE_extractPie;
  fhObject.pFhParseHieFn = MAP_FHIE_parseHie;
  fhObject.pFhResetPibFn =MAP_FHPIB_reset;
  fhObject.pFhSetPibFn = MAP_FHPIB_set;
  fhObject.pFhGetPibFn = MAP_FHPIB_get;
  fhObject.pFhGetPibLenFn = MAP_FHPIB_getLen;
  fhObject.pFhGetTxParamsFn = MAP_FHAPI_getTxParams;
  fhObject.pFhCompleteTxCbFn = MAP_FHAPI_completeTxCb;
  fhObject.pFhCompleteRxCbFn = MAP_FHAPI_completeRxCb;
  fhObject.pFhSetStateCbFn = MAP_FHAPI_setStateCb;
  fhObject.pFhAddDeviceCB = MAP_FHNT_AddDeviceCB;
  fhObject.pFhDelDeviceCB = MAP_FHNT_DelDeviceCB;
  fhObject.pFhGetTxChHopFn = MAP_FHAPI_getTxChHoppingFunc;
  fhObject.pFhGetRemDTFn = MAP_FHAPI_getRemDT;
  fhObject.pFhUpdateCCA = MAP_FHDATA_updateCCA;
  fhObject.pFhGetBitCount = MAP_FHAPI_getBitCount;
  fhObject.pFhBcTimerEvent = MAP_FHMGR_BCTimerEventUpd;
#endif /* FEATURE_FREQ_HOP */
} /* MAC_InitFH() */


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
  uint16 ieContentLen;
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


//from mac_beacon.c
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

/**************************************************************************************************
 * @fn          macCbackForEvent
 *
 * @brief       This function looks up the callback event for the given internal event and
 *              calls the MAC callback function.
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
MAC_INTERNAL_API void macCbackForEvent(macEvent_t *pEvent, uint8 status)
{
#ifdef FEATURE_WISUN_SUPPORT
  macMcpsDataCnfAck_t cbackEvent;    /* use data cnf type for all events */
#else
  macMcpsDataCnf_t cbackEvent;    /* use data cnf type for all events */
#endif

  cbackEvent.hdr.event = macCbackEventTbl[pEvent->hdr.event];
  if (cbackEvent.hdr.event != 0)
  {
    /* if data cnf fill in additional fields */
    if (cbackEvent.hdr.event == MAC_MCPS_DATA_CNF)
    {
      if (status == MAC_NO_RESOURCES)
      {
        cbackEvent.pDataReq = NULL;
      }
      else
      {
        cbackEvent.pDataReq   = &(pEvent->dataReq);
        cbackEvent.msduHandle = pEvent->dataReq.mac.msduHandle;
        cbackEvent.timestamp  = pEvent->dataReq.internal.timestamp;
        cbackEvent.timestamp2 = pEvent->dataReq.internal.timestamp2;
        cbackEvent.retries = (pEvent->dataReq.internal.txOptions & MAC_TXOPTION_NO_RETRANS) ?
                             0 : pMacPib->maxFrameRetries - pEvent->dataReq.internal.retries;
        cbackEvent.mpduLinkQuality = pEvent->dataReq.internal.mpduLinkQuality;
        cbackEvent.correlation = pEvent->dataReq.internal.correlation;
        cbackEvent.rssi = pEvent->dataReq.internal.rssi;

#ifdef FEATURE_WISUN_SUPPORT
        if((pEvent->dataReq.mac.txOptions & MAC_TXOPTION_ACK) && (status == MAC_SUCCESS))
        {
            cbackEvent.pHeaderIE = pRxBuf->pHdrIes;
            cbackEvent.headerIeLen = pRxBuf->headerIeLen;
            /* commenting original piece of code for reference */
            //cbackEvent.pPayloadIE = pRxBuf->mac.pPayloadIE;
            /* for dataCnf event, when ACK was requested, pRXBuf holds an
             * enhanced ack frame and ack frame as per spec will not
             * carry any payload IEs. Hence, re-using this field to point
             * to pRxBuf, so that the same can be passed to nanostack level
             */
            cbackEvent.pPayloadIE = (uint8 *)pRxBuf;
            cbackEvent.payloadIeLen = pRxBuf->mac.payloadIeLen;
        }
        else
        {
            cbackEvent.pHeaderIE = NULL;
            cbackEvent.headerIeLen = 0;
            cbackEvent.pPayloadIE = NULL;
            cbackEvent.payloadIeLen = 0;
        }
#endif

#ifdef FEATURE_MAC_SECURITY
#if (defined TIMAC_ROM_IMAGE_BUILD)
    if (macPib.securityEnabled == TRUE)
    {
#endif
        cbackEvent.frameCntr = pEvent->dataReq.internal.frameCntr;
#if (defined TIMAC_ROM_IMAGE_BUILD)
    }
#endif
#endif

      }
    }
    else if (cbackEvent.hdr.event == MAC_MLME_WS_ASYNC_CNF )
    {
      MAP_osal_msg_deallocate( (uint8*)pEvent );
    }

    cbackEvent.hdr.status = status;
    MAP_MAC_CbackEvent((macCbackEvent_t *) &cbackEvent);
  }
}

/**************************************************************************************************
 * @fn          macDataRxInd
 *
 * @brief       This action function is called to handle a received data indication.  It
 *              executes the MAC callback function with event MAC_MCPS_DATA_IND.
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
MAC_INTERNAL_API void macDataRxInd(macEvent_t *pEvent)
{
  DBG_PRINT0(DBGSYS, "macDataRxInd");
  if (pEvent->dataInd.internal.fhFrameType <= MAC_FH_FRAME_PAN_CONFIG_SOLICIT )
  {
    pEvent->hdr.event = MAC_MLME_WS_ASYNC_IND;
  }
  else
  {
    pEvent->hdr.event = MAC_MCPS_DATA_IND;
  }

#ifdef FEATURE_WISUN_SUPPORT
  pEvent->dataInd.mac.pHeaderIE = pEvent->rx.pHdrIes;
  pEvent->dataInd.mac.headerIeLen = pEvent->rx.headerIeLen;
#endif


  MAP_MAC_CbackEvent((macCbackEvent_t *) pEvent);
}

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
  macDeviceAction3[0] = macApiNodeStartReq;
  macDeviceAction4[0] = macNodeStartComplete;

  macDevice.sec.securityLevel = 0;
  macDevice.dataReqMask = 0;
  macDevice.frameTimer.pFunc = MAP_macSetEvent;
  macDevice.frameTimer.parameter = MAC_FRAME_RESPONSE_TASK_EVT;
  macDevice.responseTimer.pFunc = MAP_macSetEvent;
  macDevice.responseTimer.parameter = MAC_RESPONSE_WAIT_TASK_EVT;
}

#ifdef FEATURE_WISUN_SUPPORT
/**************************************************************************************************
 * @fn          macApiStartReq
 *
 * @brief       This action function handles an API start request event.
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
bool nodeStarted = false;
MAC_INTERNAL_API void macApiNodeStartReq(macEvent_t *pEvent)
{
  if(!nodeStarted)
  {
      ApiMac_mlmeStartReq_t *pStartReq = &pEvent->api.mac.startReq;

      MAP_MAC_MlmeSetReq(MAC_PHY_CURRENT_DESCRIPTOR_ID, &pStartReq->phyID);
      MAP_MAC_MlmeSetReq(MAC_CHANNEL_PAGE, &pStartReq->channelPage);

      MAP_MAC_MlmeSetReq(MAC_PAN_ID, &pStartReq->panId);
      MAP_MAC_MlmeSetReq(MAC_LOGICAL_CHANNEL, &pStartReq->logicalChannel);

      if (MAC_FH_ENABLED && MAC_FH_START_FN)
      {
        MAC_FH_START_FN();
      }
      nodeStarted = true;
  }
  /* complete the start */
  pEvent->hdr.status = MAC_SUCCESS;
  pEvent->hdr.event = MAC_INT_START_COMPLETE_EVT;
  MAP_macExecute(pEvent);
}

/**************************************************************************************************
 * @fn          macStartComplete
 *
 * @brief       This action function is called when the start procedure is complete.  The
 *              pMacStartReq buffer is deallocated.  The MAC callback is called with event
 *              MAC_MLME_START_CNF with status from the event.
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
MAC_INTERNAL_API void macNodeStartComplete(macEvent_t *pEvent)
{
  MAP_macRxEnable(MAC_RX_WHEN_IDLE);
  pEvent->hdr.event = MAC_MLME_START_CNF;
  MAP_MAC_CbackEvent((macCbackEvent_t *) pEvent);
}
#endif

/**************************************************************************************************
 * @fn          MAC_InitDevice
 *
 * @brief       This function initializes the MAC to associate with a non
 *              beacon-enabled network.  This function would be used to
 *              initialize a device as an RFD.  If this function is used it
 *              must be called during application initialization before any
 *              other function in the data or management API is called.
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
void MAC_InitDevice(void)
{
  /* Initialize action set for actions specific to Device node */
#ifdef FEATURE_WISUN_SUPPORT
  macActionSet[MAC_COORD_ACTION_1] = macDeviceAction3;
  macActionSet[MAC_COORD_ACTION_2] = macDeviceAction4;
#else
  macStateTbl[MAC_STARTING_ST] = macStartingSt;
  macStateTbl[MAC_ASSOCIATING_ST] = macAssociatingSt;
  macStateTbl[MAC_POLLING_ST] = macPollingSt;
#endif

  macReset[MAC_FEAT_DEVICE] = MAP_macDeviceReset;

  /* tx frame set schedule function */
  macDataSetSched = MAP_macSetSched;
  /* tx frame check schedule function */
  macDataCheckSched = MAP_macCheckSched;

  macMain.featureMask |= MAC_FEAT_DEVICE_MASK;
}

uint8_t FHUTIL_getChannelSpacing(void)
{
    uint8_t channelSpacing;
    uint8 status;
    uint8_t phyId;
    macMRFSKPHYDesc_t mrFskPhy;
    MAP_MAC_MlmeGetReq(MAC_PHY_CURRENT_DESCRIPTOR_ID, &phyId);
    status = MAP_MAC_GetPHYParamReq( MAC_PHY_DESCRIPTOR, phyId, &mrFskPhy );
    if(status == MAC_SUCCESS)
    {
        if (mrFskPhy.channelSpacing == 100 )
            channelSpacing = 3;
        else
            channelSpacing = (mrFskPhy.channelSpacing/200 - 1)%3;
    }
    else
        channelSpacing = FHIE_DEFAULT_CHANNEL_SPACING;
    return(channelSpacing);
}

extern uint8_t FH_ChanPlan;

uint8_t FHIE_getChannelPlan(FHIE_channelPlan_t *pChPlan)
{
    uint8_t i;

    /* get information from the FH Chan plan table
     *
     */
    i=0;
    pChPlan->ch0 = FHIE_channelPlan[i].ch0;
    pChPlan->channelSpacing = FHIE_channelPlan[i].channelSpacing;
    pChPlan->noOfChannels = FHIE_channelPlan[i].noOfChannels;
    pChPlan->regulatoryDomain = FHIE_channelPlan[i].regulatoryDomain;
    pChPlan->operatingClass = FHIE_channelPlan[i].operatingClass;

    if (FH_ChanPlan == 0 )
    {
       return FHIE_CP_REGULATORY_DOMAIN;
    }
    else if ( FH_ChanPlan == 1 )
        return(FHIE_CP_VENDOR_SPECIFIED);
    else
        return FHIE_CP_VENDOR_SPECIFIED;

}

#endif /*#if defined(TIMAC_ROM_PATCH) */
