/**************************************************************************************************
  Filename:       mt_rpc.h
  Revised:        $Date: 2014-06-20 15:25:38 -0700 (Fri, 20 Jun 2014) $
  Revision:       $Revision: 39136 $

  Description:    Public interface file for the RPC Transport Protocol Design.

  Copyright 2007-2014 Texas Instruments Incorporated.

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

#ifndef MT_RPC_H
#define MT_RPC_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/

#include "hal_types.h"


/******************************************************************************
 General RPC constants and definitions
 *****************************************************************************/
/*! Length of MT header for general format RPC frame */
#define MTRPC_FRAME_HDR_SZ  3

/*!
 Position of fields in the general format RPC frame:
   1st byte is length of the data field
   2nd byte is command type/sub-system
   3rd byte is sub-system command ID
   4th+ bytes are data block (if any)
 */
/*! General RPC frame header data length byte position */
#define MTRPC_POS_LEN   0
/*! General RPC frame header command type/sub-system byte position */
#define MTRPC_POS_CMD0  1
/*! General RPC frame header sub-system command ID byte position */
#define MTRPC_POS_CMD1  2
/*! General RPC frame header data block byte position */
#define MTRPC_POS_DAT0  3

/***************************************************************************************************
 * CONSTANTS
 ***************************************************************************************************/

/* 1st byte is the length of the data field, 2nd/3rd bytes are command field. */
#define MT_RPC_FRAME_HDR_SZ   3

/* Maximum length of data in the general frame format. The upper limit is 255 because of the
 * 1-byte length protocol. But the operation limit is lower for code size and ram savings so that
 * the uart driver can use 256 byte rx/tx queues and so
 * (MT_RPC_DATA_MAX + MT_RPC_FRAME_HDR_SZ + MT_UART_FRAME_OVHD) < 256
 */
#define MT_RPC_DATA_MAX       250

/* The 3 MSB's of the 1st command field byte are for command type. */
#define MT_RPC_CMD_TYPE_MASK  0xE0

/* The 5 LSB's of the 1st command field byte are for the subsystem. */
#define MT_RPC_SUBSYSTEM_MASK 0x1F

/* position of fields in the general format frame */
#define MT_RPC_POS_LEN        0
#define MT_RPC_POS_CMD0       1
#define MT_RPC_POS_CMD1       2
#define MT_RPC_POS_DAT0       3

/* Error codes */
#define MT_RPC_SUCCESS        0     /* success */
#define MT_RPC_ERR_SUBSYSTEM  1     /* invalid subsystem */
#define MT_RPC_ERR_COMMAND_ID 2     /* invalid command ID */
#define MT_RPC_ERR_PARAMETER  3     /* invalid parameter */
#define MT_RPC_ERR_LENGTH     4     /* invalid length */

/***************************************************************************************************
 * TYPEDEF
 ***************************************************************************************************/

typedef enum {
  MT_RPC_CMD_POLL = 0x00,
  MT_RPC_CMD_SREQ = 0x20,
  MT_RPC_CMD_AREQ = 0x40,
  MT_RPC_CMD_SRSP = 0x60,
  MT_RPC_CMD_RES4 = 0x80,
  MT_RPC_CMD_RES5 = 0xA0,
  MT_RPC_CMD_RES6 = 0xC0,
  MT_RPC_CMD_RES7 = 0xE0
} mtRpcCmdType_t;

typedef enum {
  MT_RPC_SYS_RES0,   /* Reserved. */
  MT_RPC_SYS_SYS,
  MT_RPC_SYS_MAC,
  MT_RPC_SYS_NWK,
  MT_RPC_SYS_AF,
  MT_RPC_SYS_ZDO,
  MT_RPC_SYS_SAPI,   /* Simple API. */  //DEPRECATED
  MT_RPC_SYS_UTIL,
  MT_RPC_SYS_DBG,
  MT_RPC_SYS_APP,
  MT_RPC_SYS_OTA,
  MT_RPC_SYS_ZNP,
  MT_RPC_SYS_SPARE_12,
  MT_RPC_SYS_UBL = 13,  // 13 to be compatible with existing RemoTI.
  MT_RPC_SYS_RES14,
  MT_RPC_SYS_APP_CNF,
  MT_RPC_SYS_RES16,
  MT_RPC_SYS_PROTOBUF,
  MT_RPC_SYS_RES18,  // RPC_SYS_PB_NWK_MGR
  MT_RPC_SYS_RES19,  // RPC_SYS_PB_GW
  MT_RPC_SYS_RES20,  // RPC_SYS_PB_OTA_MGR
  MT_RPC_SYS_GP = 21,
  MT_RPC_SYS_MAX     /* Maximum value, must be last */
  /* 22-32 available, not yet assigned. */
} mtRpcSysType_t;

//typedef struct
//{
//  uint8_t *(*mt_alloc)(mtRpcCmdType_t type, uint8_t len);
//  void (*mt_send)(uint8_t *pBuf);
//} mtTransport_t;
//
typedef uint8_t (*mtProcessMsg_t)(uint8_t *pBuf);

/***************************************************************************************************
***************************************************************************************************/

#ifdef __cplusplus
};
#endif

#endif /* MT_RPC_H */
