/**************************************************************************************************
  Filename:       ssp.h
  Revised:        $Date: 2014-11-18 02:32:26 -0800 (Tue, 18 Nov 2014) $
  Revision:       $Revision: 41160 $

  Description:    Security Service Provider (SSP) interface


  Copyright 2004-2014 Texas Instruments Incorporated.

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
**************************************************************************************************/

#ifndef SSP_H
#define SSP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcomdef.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
#define SSP_APPLY         0
#define SSP_REMOVE        1
#define SSP_REMOVE_RETRY  2

// Auxiliary header field lengths
#define FRAME_COUNTER_LEN 4
//Threshold after which the frame counter will be reset if a valid APSME-SWITCH-KEY is processed.
#define NWK_FRAME_COUNTER_RESET_THRESHOLD  0x80000000

#define SEC_KEY_LEN  16  // 128/8 octets (128-bit key is standard for ZigBee)
#define INSTALL_CODE_LEN      16
#define INSTALL_CODE_CRC_LEN  2
#define BITS_PER_BYTE         8
#define APS_MIC_LEN           4

// Security Key Indentifiers
#define SEC_KEYID_LINK      0x00
#define SEC_KEYID_NWK       0x01
#define SEC_KEYID_TRANSPORT 0x02
#define SEC_KEYID_LOAD      0x03

// Security Levels
#define SEC_MASK        0x07
#define SEC_NONE        0x00
#define SEC_MIC_32      0x01
#define SEC_MIC_64      0x02

#define SEC_MIC_128     0x03
#define SEC_ENC         0x04
#define SEC_ENC_MIC_32  0x05
#define SEC_ENC_MIC_64  0x06
#define SEC_ENC_MIC_128 0x07

// Key types
#define KEY_TYPE_NWK        1   // Standard Network Key
#define KEY_TYPE_APP_MASTER 2   // Application Master Key
#define KEY_TYPE_APP_LINK   3   // Application Link Key
#define KEY_TYPE_TC_LINK    4   // Trust Center Link Key

#define SSP_AUXHDR_CTRL      0
#define SSP_AUXHDR_FRAMECNTR 1

#define SSP_AUXHDR_KEYID_MASK     0x03
#define SSP_AUXHDR_KEYID_SHIFT    3
#define SSP_AUXHDR_EXTNONCE_SHIFT 5
#define SSP_AUXHDR_EXTNONCE_BIT   0x01
#define SSP_AUXHDR_LEVEL_MASK     0x07

#define SSP_AUXHDR_MIN_LEN    5
#define SSP_AUXHDR_SEQNUM_LEN 1
#define SSP_AUXHDR_EXT_LEN ( SSP_AUXHDR_MIN_LEN + Z_EXTADDR_LEN )
#define SSP_AUXHDR_NWK_LEN ( SSP_AUXHDR_EXT_LEN + SSP_AUXHDR_SEQNUM_LEN  )

#define SSP_MIC_LEN_MAX 16

#define SSP_NONCE_LEN 13

#define SSP_TEXT_LEN 4

// SSP_MacTagData_t::type
#define SSP_MAC_TAGS_SKKE 0
#define SSP_MAC_TAGS_EA   1

// Error value used when security key NV ID is not available
#define SEC_NO_KEY_NV_ID        ZCD_NV_INVALID_INDEX


/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
  uint8_t keySeqNum;
  uint8_t key[SEC_KEY_LEN];
} nwkKeyDesc;

typedef struct
{
  nwkKeyDesc  active;
  uint32_t      frameCounter;
} nwkActiveKeyItems;

typedef struct
{
  uint8_t hdrLen;
  uint8_t auxLen;
  uint8_t msgLen;
  uint8_t secLevel;
  uint8_t keyId;
  uint32_t frameCtr;
  uint8_t *key;
} ssp_ctx;

typedef struct
{
  uint8_t* initExtAddr;
  uint8_t* rspExtAddr;
  uint8_t* key;
  uint8_t* qeu;
  uint8_t* qev;
  uint8_t* text1;
  uint8_t* text2;
  uint8_t* tag1;
  uint8_t* tag2;
  uint8_t* linkKey;
  uint8_t  type;
} SSP_MacTagData_t;

typedef struct
{
  uint8_t  dir;
  uint8_t  secLevel;
  uint8_t  hdrLen;
  uint8_t  sduLen;   //service data unit length
  uint8_t* pdu;      //protocol data unit
  uint8_t  extAddr[Z_EXTADDR_LEN];
  uint8_t  keyID;
  uint16_t keyNvSection; // corresponds to NVINTF_itemID_t itemid
  uint16_t keyNvId;      // corresponds to NVINTF_itemID_t subid
  uint8_t  keySeqNum;
  uint32_t frmCntr;
  uint8_t  auxLen;
  uint8_t  micLen;
  uint8_t  dstExtAddr[Z_EXTADDR_LEN];
  bool   distributedKeyTry;  //Attempting to validate if TransportKey uses distributed key
  bool   defaultKeyTry;      //Attempting to validate if TransportKey uses default key when install code is in use
  uint8_t  seedShift;
} SSP_Info_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern uint32_t nwkFrameCounter;
extern uint32_t distributedFrameCounter;
extern uint16_t nwkFrameCounterChanges;

/*********************************************************************
 * FUNCTIONS
 */

/*
 * SSP Initialization
 */
extern void SSP_Init( void );

/*
 * Parse Auxillary Header
 */
extern void SSP_ParseAuxHdr( SSP_Info_t* si );

/*
 * Process Security Information
 */
extern ZStatus_t SSP_Process( SSP_Info_t* si );

/*
 * Process MAC TAG Data - Generate Tags
 */
extern ZStatus_t SSP_GetMacTags( SSP_MacTagData_t* data );

/*
 * Returns Random Bits
 */
extern void SSP_GetTrueRand( uint8_t len, uint8_t *rand );

/*
 * Returns 8*len random bits using AES based mechanism
 * ( currently less than 128 bits )
 */
extern ZStatus_t SSP_GetTrueRandAES( uint8_t len, uint8_t *rand );

/*
 * Store the 16 byte random seed in NV
 */
extern void SSP_StoreRandomSeedNV( uint8_t *pSeed );

/*
 * Read the network active key information
 */
extern void SSP_ReadNwkActiveKey( nwkActiveKeyItems *items );

/*
 * Get the index for the selected network key in NV
 */
extern uint16_t SSP_GetNwkKey( uint8_t seqNum );

/*
 * Secure/Unsecure a network PDU
 */
extern ZStatus_t SSP_NwkSecurity(uint8_t ed_flag, uint8_t *msg, uint8_t hdrLen, uint8_t nsduLen);

/*
 * Set the alternate network key
 */
extern void SSP_UpdateNwkKey( uint8_t *key, uint8_t keySeqNum );

/*
 * Make the alternate network key as active
 */
extern void SSP_SwitchNwkKey( uint8_t seqNum );

extern void SSP_BuildNonce( uint8_t *addr, uint32_t frameCntr, uint8_t secCtrl, uint8_t *nonce );

extern uint8_t SSP_GetMicLen( uint8_t securityLevel );

/*
 * Duplicate OsalPort_memcpy functionality, but reverse copy
 */
extern uint8_t* SSP_MemCpyReverse( uint8_t* dst, uint8_t* src, unsigned int len );

/*********************************************************************
*********************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* SSP_H */
