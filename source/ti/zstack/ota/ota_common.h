/******************************************************************************
  Filename:       ota_common.h
  Revised:        $Date: 2013-12-10 07:42:48 -0800 (Tue, 10 Dec 2013) $
  Revision:       $Revision: 36527 $

  Description:    This file contains code common to the OTA server,
                  client, and dongle.


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

#ifndef OTA_COMMON_H
#define OTA_COMMON_H

#if !defined HAL_OTA_BOOT_CODE
#include "af.h"
#endif

#ifndef _MSC_VER
#include "hal_mcu.h"
#endif

#define OTA_APP_MAX_ATTRIBUTES              7

// SYS App message format byte positions
#define MT_APP_ENDPOINT_POS                 0
#define MT_APP_COMMAND_POS                  1
#define MT_APP_DATA_POS                     2

// SYS APP Commands for dongle communication with OTA Console
#define OTA_APP_READ_ATTRIBUTE_REQ          0
#define OTA_APP_IMAGE_NOTIFY_REQ            1
#define OTA_APP_DISCOVERY_REQ               2
#define OTA_APP_JOIN_REQ                    3
#define OTA_APP_LEAVE_REQ                   4

#define OTA_APP_READ_ATTRIBUTE_IND          0x80
#define OTA_APP_IMAGE_NOTIFY_RSP            0x81
#define OTA_APP_DEVICE_IND                  0x82
#define OTA_APP_JOIN_IND                    0x83
#define OTA_APP_ENDPOINT_IND                0x85
#define OTA_APP_DONGLE_IND                  0x8A

// Sys App Message Lengths
#define OTA_APP_READ_ATTRIBUTE_REQ_LEN      (8 + OTA_APP_MAX_ATTRIBUTES*2)
#define OTA_APP_IMAGE_NOTIFY_REQ_LEN        15
#define OTA_APP_DISCOVERY_REQ_LEN           2
#define OTA_APP_JOIN_REQ_LEN                5
#define OTA_APP_LEAVE_REQ_LEN               2

#define OTA_APP_READ_ATTRIBUTE_IND_LEN      21
#define OTA_APP_IMAGE_NOTIFY_RSP_LEN        16
#define OTA_APP_DEVICE_IND_LEN              6
#define OTA_APP_JOIN_IND_LEN                4
#define OTA_APP_ENDPOINT_IND_LEN            7
#define OTA_APP_DONGLE_IND_LEN              10

#define OTA_INVALID_ID                      0xFF

// OTA Header constants
#define OTA_HDR_MAGIC_NUMBER                0x0BEEF11E
#define OTA_HDR_BLOCK_SIZE                  128
#define OTA_HDR_STACK_VERSION               2
#define OTA_HDR_HEADER_VERSION              0x0100
#define OTA_HDR_FIELD_CTRL                  0

#define OTA_HEADER_LEN_MIN                  56
#define OTA_HEADER_LEN_MAX                  69
#define OTA_HEADER_LEN_MIN_ECDSA            166
#define OTA_HEADER_STR_LEN                  32

#define OTA_HEADER_IMAGE_SIZE_POS           52
// OTA_HEADER_FILE_ID_POS is needed for windows tools
#define OTA_HEADER_FILE_ID_POS              10

#define OTA_FC_SCV_PRESENT                  (0x1 << 0)
#define OTA_FC_DSF_PRESENT                  (0x1 << 1)
#define OTA_FC_HWV_PRESENT                  (0x1 << 2)

#define OTA_SUB_ELEMENT_HDR_LEN             6

#define OTA_UPGRADE_IMAGE_TAG_ID            0
#define OTA_ECDSA_SIGNATURE_TAG_ID          1
#define OTA_EDCSA_CERTIFICATE_TAG_ID        2

// MT_OtaGeImage options
#define MT_OTA_HW_VER_PRESENT_OPTION        0x01
#define MT_OTA_QUERY_SPECIFIC_OPTION        0x02

// MT OTA Status Indication Types
#define MT_OTA_DL_COMPLETE                  0

#if defined HAL_MCU_CC2538
#pragma pack(2)
#endif
typedef struct
{
  uint16_t manufacturer;
  uint16_t type;
  uint32_t version;
} zclOTA_FileID_t;

typedef struct
{
  uint16_t tag;
  uint32_t length;
} OTA_SubElementHdr_t;

typedef struct
{
  uint32_t magicNumber;
  uint16_t headerVersion;
  uint16_t headerLength;
  uint16_t fieldControl;
  zclOTA_FileID_t fileId;
  uint16_t stackVersion;
  uint8_t headerString[OTA_HEADER_STR_LEN];
  uint32_t imageSize;
  uint8_t secCredentialVer;
  uint8_t destIEEE[8];
  uint16_t minHwVer;
  uint16_t maxHwVer;
} OTA_ImageHeader_t;
#if defined HAL_MCU_CC2538
#pragma pack()
#endif

#ifdef __cplusplus
extern "C"
{
#endif

extern uint8_t *OTA_WriteHeader(OTA_ImageHeader_t *pHdr, uint8_t *pBuf);
extern uint8_t *OTA_ParseHeader(OTA_ImageHeader_t *pHdr, uint8_t *pBuf);

extern void OTA_GetFileName(char *pName, zclOTA_FileID_t *pFileId, char *text);
extern void OTA_SplitFileName(char *pName, zclOTA_FileID_t *pFileId);

extern uint8_t *OTA_StreamToFileId(zclOTA_FileID_t *pFileId, uint8_t *pStream);
extern uint8_t *OTA_FileIdToStream(zclOTA_FileID_t *pFileId, uint8_t *pStream);

#if !defined HAL_OTA_BOOT_CODE
extern uint8_t *OTA_AfAddrToStream(afAddrType_t *pAddr, uint8_t *pStream);
extern uint8_t *OTA_StreamToAfAddr(afAddrType_t *pAddr, uint8_t *pStream);
#endif

#ifdef __cplusplus
}
#endif

#endif // OTA_COMMON_H
