/******************************************************************************
  Filename:       ota_signature.h
  Revised:        $Date: 2010-11-18 08:17:09 -0800 (Thu, 18 Nov 2010) $
  Revision:       $Revision: 24437 $

  Description:    This file contains code to calculate and verify OTA
                  signatures based on teh MMO AES Hash function.


  Copyright 2010 Texas Instruments Incorporated.

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

#ifndef OTA_SIGNATURE_H
#define OTA_SIGNATURE_H

#include "hal_types.h"

#define OTA_MMO_ROUNDS                1
#define OTA_MMO_BLOCKSIZE             32
#define OTA_MMO_HASH_SIZE             16

#define OTA_MMO_IN_PROCESS            0
#define OTA_MMO_ERROR                 1
#define OTA_MMO_COMPLETE              2

#define OTA_SIGNATURE_LEN             42
#define OTA_SIGNATURE_ELEM_LEN        (OTA_SIGNATURE_LEN + Z_EXTADDR_LEN)
#define OTA_CERTIFICATE_LEN           48

typedef struct
{
  uint8_t hash[OTA_MMO_HASH_SIZE];
  uint32_t length;
} OTA_MmoCtrl_t;


#ifdef __cplusplus
extern "C"
{
#endif

// Entry functions
extern void OTA_CalculateMmoR3(OTA_MmoCtrl_t *pCtrl, uint8_t *pData, uint8_t len, uint8_t lastBlock);
extern uint8_t OTA_ValidateSignature(uint8_t *pHash, uint8_t* pCert, uint8_t *pSig, uint8_t *pIEEE);
void sspMMOHash2 (uint8_t *Pb, uint8_t prefix, uint8_t *Mb, uint16_t bitlen, uint8_t *Cstate);

#ifdef __cplusplus
}
#endif

#endif // OTA_SIGNATURE_H
