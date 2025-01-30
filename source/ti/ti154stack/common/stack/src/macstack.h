/******************************************************************************

 @file  macstack.h

 @brief Mac Stack interface function definition

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2013-2025, Texas Instruments Incorporated

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

#ifndef MAC_STACK_H
#define MAC_STACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mac_api.h"

typedef struct
{
  macEventHdr_t               hdr;
  uint8                       paramID;
  void                        *paramValue;
} macSetParam_t;

typedef struct
{
  macEventHdr_t               hdr; //!< hdr event field must be set as ICALL_CMD_EVENT
  uint8                       paramID;
  uint8                       len;
  uint8                       *pValue;
} macGetParam_t;

typedef struct
{
  macEventHdr_t               hdr; //!< hdr event field must be set as ICALL_CMD_EVENT
  uint8                       paramID;
  uint8                       len;
  uint8                       **pValue;
} macGetSecurityPtrParam_t;

typedef struct
{
  macEventHdr_t               hdr; //!< hdr event field must be set as ICALL_CMD_EVENT
  uint8                       srctaskid;
  uint8                       retransmit;
  uint8                       pendingMsg;
  uint8 (*pMacCbackQueryRetransmit)();
  uint8 (*pMacCbackCheckPending)();
} macStackInitParams_t;

typedef struct
{
  macEventHdr_t               hdr;
  sAddr_t                     *addr;
  uint16                      panID;
} macSrcMatchEntry_t;

typedef struct
{
  macEventHdr_t               hdr; //!< hdr event field must be set as ICALL_CMD_EVENT
  uint8                       options;
} macSrcMatchAckPend_t;

typedef struct
{
  macEventHdr_t               hdr; //!< hdr event field must be set as ICALL_CMD_EVENT
  uint8                       pend;
} macSrcMatchCheckPend_t;

typedef struct
{
  macEventHdr_t               hdr; //!< hdr event field must be set as ICALL_CMD_EVENT
  uint8                       txPwrTblIdx;
  uint8                       rssiAdjIdx;
} macSetRadioTable_t;

typedef struct
{
  macEventHdr_t               hdr; //!< hdr event field must be set as ICALL_CMD_EVENT
  uint16                      panId;
} macUpdatePanId_t;

typedef struct
{
  macEventHdr_t               hdr; //!< hdr event field must be set as ICALL_CMD_EVENT
  uint8                       setDefaultPib;
  uint8                       status;
}macResetReq_t;

typedef struct
{
  macEventHdr_t               hdr; //!< hdr event field must be set as ICALL_CMD_EVENT
  uint16                      len;
  uint8                       securityLevel;
  uint8                       keyIdMode;
  uint32                      includeFhIEs;
  uint16                      payloadIeLen;
  macMcpsDataReq_t            *pDataReq;
}macMcpsDataAlloc_t;


typedef struct
{
  macEventHdr_t               hdr;
  uint8                       msduhandle;
}macPurgeReq_t;

typedef struct
{
  macEventHdr_t               hdr;
  uint8                       randByte;
}randomByte_t;

typedef struct
{
  macEventHdr_t               hdr;
  ApiMac_mlmeAssociateReq_t   associateReq;
}macMlmeAssociateReqEvt_t;

typedef struct
{
  macEventHdr_t              hdr;
  ApiMac_mlmeAssociateRsp_t  associateRsp;
}macMlmeAssociateRspEvt_t;

typedef struct
{
  macEventHdr_t                hdr;
  ApiMac_mlmeDisassociateReq_t disAssociateReq;
}macMlmeDisassociateReqEvt_t;

typedef struct
{
  macEventHdr_t               hdr;
  ApiMac_mlmeOrphanRsp_t      orphanRsp;
}macMlmeOrphanRspEvt_t;

typedef struct
{
  macEventHdr_t               hdr;
  ApiMac_mlmePollReq_t        pollReq;
}macMlmePollReqEvt_t;

typedef struct
{
  macEventHdr_t               hdr;
  ApiMac_mlmeScanReq_t        scanReq;
}macMlmeScanReqEvt_t;

typedef struct
{
  macEventHdr_t               hdr;
  ApiMac_mlmeStartReq_t       startReq;
}macMlmeStartReqEvt_t;

typedef struct
{
  macEventHdr_t               hdr;
  ApiMac_mlmeSyncReq_t        syncReq;
}macMlmeSyncReqEvt_t;

typedef struct
{
  macEventHdr_t               hdr;
  uint16                      paramID;
  void                        *paramValue;
} macFHSetParam_t;

typedef struct
{
  macEventHdr_t               hdr; //!< hdr event field must be set as ICALL_CMD_EVENT
  uint16                      paramID;
  uint8                       len;
  uint8                       *pValue;
} macFHGetParam_t;


typedef struct
{
  macEventHdr_t               hdr;
  ApiMac_mlmeWSAsyncReq_t     asyncReq;
}macMlmeWSAsyncReqEvt_t;

typedef struct
{
  macEventHdr_t               hdr;
  ApiMac_secAddDevice_t      *param;
} macSecAddDevice_t;

typedef struct
{
  macEventHdr_t               hdr;
  ApiMac_sAddrExt_t          *param;
} macSecDelDevice_t;

typedef struct
{
  macEventHdr_t               hdr;
  uint8                       keyIndex; /* mac secuirty key table index of the key to be removed */
} macSecDelKeyAndDevices_t;

typedef struct
{
  macEventHdr_t               hdr;
  uint8                       keyId; /* key ID */
  uint32                      frameCounter; /* outgoing frame counter of the key */
} macSecGetDefaultSrcKey_t;

typedef struct
{
  macEventHdr_t               hdr;
  ApiMac_secAddKeyInitFrameCounter_t  *param;
} macSecAddKeyInitFC_t;

typedef struct
{
  macEventHdr_t               hdr;
  ApiMac_mcpsDataReq_t        dataReq;
} macMcpsDataReqEvt_t;

typedef union
{
  macEventHdr_t               hdr;
  macStackInitParams_t        macInit;
  macSetParam_t               setParam;
  macGetParam_t               getParam;
  macSrcMatchEntry_t          srcMatchEntry;
  macSrcMatchAckPend_t        srcMatchAckPend;
  macSrcMatchCheckPend_t      srcMatchCheckPend;
  macSetRadioTable_t          setRadioTable;
  macUpdatePanId_t            updatePanId;
  macResetReq_t               resetReq;
  macMcpsDataAlloc_t          mcpsDataAlloc;
  macMcpsDataReqEvt_t         mcpsDataReq;
  randomByte_t                randomByte;
  macPurgeReq_t               purgeReq;
  macMlmeAssociateReqEvt_t    associateReqEvt;
  macMlmeAssociateRspEvt_t    associateRspEvt;
  macMlmeDisassociateReqEvt_t disassociateReqEvt;
  macMlmeOrphanRspEvt_t       orphanRspEvt;
  macMlmePollReqEvt_t         pollReqEvt;
  macMlmeScanReqEvt_t         scanReqEvt;
  macMlmeStartReqEvt_t        startReqEvt;
  macMlmeSyncReqEvt_t         syncReqEvt;
  macGetSecurityPtrParam_t    getPtrParam;
  macFHSetParam_t             fhSetParam;
  macFHGetParam_t             fhGetParam;
  macMlmeWSAsyncReqEvt_t      asyncReqEvt;
  macSecAddDevice_t           secAddDevice;
  macSecDelDevice_t           secDelDevice;
  macSecDelKeyAndDevices_t    secDelKeyAndDevices;
  macSecGetDefaultSrcKey_t    secGetDefaultSrcKey;
  macSecAddKeyInitFC_t        secAddKeyInitFC;
} macCmd_t;


void macStackTaskInit(uint8 taskId);
uint16 macStackEventLoop(uint8 taskId, uint16 events);

#ifdef __cplusplus
};
#endif

#endif /* MAC_STACK_H */


