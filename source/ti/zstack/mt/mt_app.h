/***************************************************************************************************
  Filename:       MT_APP.h
  Revised:        $Date: 2014-11-19 13:29:24 -0800 (Wed, 19 Nov 2014) $
  Revision:       $Revision: 41175 $

  Description:

  Copyright 2008-2014 Texas Instruments Incorporated.

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

 ***************************************************************************************************/
#ifndef MT_APP_H
#define MT_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
#include "rom_jt_154.h"
#include "af.h"


/***************************************************************************************************
 * CONSTANTS
 ***************************************************************************************************/

#define MT_APP_PB_ZCL_CMD_MSG 0x00 // MT_APP_PB_ZCL_CMD message
#define MT_APP_PB_ZCL_CMD_CFG 0x01 // MT_APP_PB_ZCL_CMD config

#define MT_APP_PB_ZCL_MSG_HDR_LEN 13 // sizeof (uint8_t)  + // AppEndPoint
                                     // sizeof (uint16_t) + // DestAddress
                                     // sizeof (uint8_t)  + // DestEndpoint
                                     // sizeof (uint16_t) + // ClusterID
                                     // sizeof (uint8_t)  + // CommandID
                                     // sizeof (uint8_t)  + // Specific
                                     // sizeof (uint8_t)  + // Direction
                                     // sizeof (uint8_t)  + // DisableDefaultRsp
                                     // sizeof (uint16_t) + // ManuCode
                                     // sizeof (uint8_t)  + // TransSeqNum

#define MT_APP_PB_ZCL_IND_HDR_LEN 13 // sizeof (uint8_t)  + // AppEndPoint
                                     // sizeof (uint16_t) + // SrcAddress
                                     // sizeof (uint8_t)  + // SrcEndpoint
                                     // sizeof (uint16_t) + // ClusterID
                                     // sizeof (uint8_t)  + // CommandID
                                     // sizeof (uint8_t)  + // Specific
                                     // sizeof (uint8_t)  + // Direction
                                     // sizeof (uint8_t)  + // DisableDefaultRsp
                                     // sizeof (uint16_t) + // ManuCode
                                     // sizeof (uint8_t)  + // TransSeqNum

#define MT_APP_PB_ZCL_CFG_HDR_LEN  2 // sizeof (uint8_t)  + // AppEndPoint
                                     // sizeof (uint8_t)  + // Mode


/***************************************************************************************************
 * TYPEDEF
 ***************************************************************************************************/

typedef struct
{
  OsalPort_EventHdr  hdr;
  uint8_t             endpoint;
  uint8_t             appDataLen;
  uint8_t             *appData;
} mtSysAppMsg_t;

typedef struct
{
  OsalPort_EventHdr hdr;
  uint8_t            type;
} mtAppPB_ZCLCmd_t;

typedef struct
{
  OsalPort_EventHdr hdr;
  uint8_t            type;
  uint8_t            appEP;
  afAddrType_t     dstAddr;
  uint16_t           clusterID;
  uint8_t            commandID;
  uint8_t            specific;
  uint8_t            direction;
  uint8_t            disableDefRsp;
  uint16_t           manuCode;
  uint8_t            transSeqNum;
  uint8_t            appPBDataLen;
  uint8_t            *appPBData;
} mtAppPB_ZCLMsg_t;

typedef struct
{
  OsalPort_EventHdr hdr;
  uint8_t            type;
  uint8_t            mode;
} mtAppPB_ZCLCfg_t;

typedef struct
{
  uint8_t  appEP;
  uint16_t srcAddr;
  uint8_t  srcEP;
  uint16_t clusterID;
  uint8_t  commandID;
  uint8_t  specific;
  uint8_t  direction;
  uint8_t  disableDefRsp;
  uint16_t manuCode;
  uint8_t  transSeqNum;
  uint8_t  appPBDataLen;
  uint8_t  *appPBData;
} mtAppPB_ZCLInd_t;


/***************************************************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************************************************/
#if defined (MT_APP_FUNC)
/*
 * Process MT_APP commands
 */
extern uint8_t MT_AppCommandProcessing(uint8_t *pBuf);
#endif

/*
 * Send an MT_APP_PB_ZCL_IND command
 */
extern void MT_AppPB_ZCLInd( mtAppPB_ZCLInd_t *ind );


#ifdef __cplusplus
}
#endif

#endif /* MTEL_H */

/***************************************************************************************************
 ***************************************************************************************************/
