/******************************************************************************
  Filename:       ota_signature.c
  Revised:        $Date: 2014-05-15 12:54:26 -0700 (Thu, 15 May 2014) $
  Revision:       $Revision: 38555 $

  Description:    This file contains code to calculate and verify OTA
                  signatures based on teh MMO AES Hash function.


  Copyright 2010-2013 Texas Instruments Incorporated.

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
******************************************************************************/

#include "hal_types.h"
#include "ota_common.h"
#include "ota_signature.h"
#include "eccapi.h"

#ifdef _WIN32
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define memset  memset
#define OsalPort_memcpy  OsalPort_memcpy
#define strlen  strlen
#include "aes.h"
#define ssp_HW_KeyInit(a)
#else
#include "rom_jt_154.h"
#include "hal_aes.h"
#include "ssp_hash.h"
#include "zglobals.h"
#include "zcl.h"
#include "osal_nv.h"
#endif

static void OTA_AesHashBlock(uint8_t *pHash, uint8_t *pData);
static void OTA_XorBlock(uint8_t *pHash, uint8_t *pData);

/******************************************************************************
 * @fn      OTA_AesHashBlock
 *
 * @brief   This function performs the AES MMO Hash on a block of data
 *
 * @param   pHash - Pointer to hash
 *          pData - pointer to data
 *
 * @return  none
 */
void OTA_AesHashBlock(uint8_t *pHash, uint8_t *pData)
{
  uint8_t key[OTA_MMO_HASH_SIZE];

  OsalPort_memcpy(key, pHash, OTA_MMO_HASH_SIZE);
  OsalPort_memcpy(pHash, pData, OTA_MMO_HASH_SIZE);
  ssp_HW_KeyInit(key);
  sspAesEncryptHW(key, pHash);
  OTA_XorBlock(pHash, pData);
}

/******************************************************************************
 * @fn      OTA_XorBlock
 *
 * @brief   This function exclusive ORs a block of hash and data and puts the
 *          result into the hash.
 *
 * @param   pHash - Pointer to hash
 *          pData - pointer to data
 *
 * @return  none
 */
void OTA_XorBlock(uint8_t *pHash, uint8_t *pData)
{
  uint8_t i;

  for (i=0; i < OTA_MMO_HASH_SIZE; i++)
  {
    pHash[i] ^= pData[i];
  }
}

/******************************************************************************
 * @fn      OTA_CalculateMmoR3
 *
 * @brief   This function calcualtes a MMO (revision 3) Hash of an OTA Image
 *          The hash must cover the entire image, but the data is received in
 *          smaller blocks.  State information about the hash is passed into
 *          this function with each block of data.
 *
 * @param   pCtrl - The control structure to calculate the MMO AES Hash
 *          pData - A block of data (must be OTA_MMO_HASH_SIZE bytes except for last block)
 *          len - The length of pData (ignored except when lastBlock = TRUE)
 *          lastBlock - Indicates this is the last block of data to be hashed
 *
 * @return  none
 */
void OTA_CalculateMmoR3(OTA_MmoCtrl_t *pCtrl, uint8_t *pData, uint8_t len, uint8_t lastBlock)
{
  if (lastBlock)
  {
    uint32_t m = (pCtrl->length + len) << 3;
    uint8_t ending[OTA_MMO_HASH_SIZE];

    memset(ending, 0, OTA_MMO_HASH_SIZE);

    if ( len >= OTA_MMO_HASH_SIZE )
    {
      len = OTA_MMO_HASH_SIZE - 1;
    }
    if (len)
    {
      OsalPort_memcpy(ending, pData, len);
    }
    ending[len] = 0x80;

    // Different endings are required depending on total length
    if (m < 0x00010000)
    {
      if(len > 13)
      {
        OTA_AesHashBlock(pCtrl->hash, ending);
        memset(ending, 0, OTA_MMO_HASH_SIZE);
      }

      ending[14] = (uint8_t)((m >> 8) &0xFF);
      ending[15] = (uint8_t)(m & 0xFF);

      OTA_AesHashBlock(pCtrl->hash, ending);
    }
    else
    {
      if (len > 9)
      {
        OTA_AesHashBlock(pCtrl->hash, ending);
        memset(ending, 0, OTA_MMO_HASH_SIZE);
      }

      ending[10] = (uint8_t)((m >> 24) & 0xFF);
      ending[11] = (uint8_t)((m >> 16) & 0xFF);
      ending[12] = (uint8_t)((m >> 8) & 0xFF);
      ending[13] = (uint8_t)(m & 0xFF);

      OTA_AesHashBlock(pCtrl->hash, ending);
    }
  }
  else
  {
    OTA_AesHashBlock(pCtrl->hash, pData);
    pCtrl->length += OTA_MMO_HASH_SIZE;
  }
}

/******************************************************************************
 * @fn      OTA_ValidateSignature
 *
 * @brief   This function validates an ECDSA Signature.
 *
 * @param   pHash - The digest created from the OTA Image
 *          pCert - The Signer Certificate
 *          pSig - The signature from the OTA Image
 *          pIEEE - The Signer IEEE
 *
 * @return  none
 */
uint8_t OTA_ValidateSignature(uint8_t *pHash, uint8_t* pCert, uint8_t *pSig, uint8_t *pIEEE)
{
  // silence compiler warnings
  pHash = pHash;
  pCert = pCert;
  pSig = pSig;
  pIEEE = pIEEE;

  return ZSuccess;
}
