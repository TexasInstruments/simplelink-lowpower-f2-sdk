/******************************************************************************

 @file OADProtocol.c

 @brief Sub1G Over the Air Download Protocol Module

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

/***** Includes *****/

#include <string.h>

#ifndef __unix__
#include <native_oad/oad_protocol.h>
#else
#include <oad_protocol.h>
#endif

/***** Defines *****/

typedef OADProtocol_Status_t (*incomingPacketProcessFn_t)(void* pSrcAddress, uint8_t *pIncomingPacket);

typedef struct {
    uint8_t cmdId;
    incomingPacketProcessFn_t packetprocessFn;
}incomingPacketProcess_t;

static OADProtocol_Status_t processFwVersioReq(void* pSrcAddress, uint8_t *pIncomingPacket);
static OADProtocol_Status_t processFwVersioRsp(void* pSrcAddress, uint8_t *pIncomingPacket);
static OADProtocol_Status_t processOadImgIdentifyReq(void* pSrcAddress, uint8_t *pIncomingPacket);
static OADProtocol_Status_t processOadImgIdentifyRsp(void* pSrcAddress, uint8_t *pIncomingPacket);
static OADProtocol_Status_t processOadImgBlockReq(void* pSrcAddr, uint8_t *pIncomingPacket);
static OADProtocol_Status_t processOadImgBlockRsp(void* pSrcAddress, uint8_t *pIncomingPacket);
#ifdef OAD_ONCHIP
static OADProtocol_Status_t processOadResetReq(void* pSrcAddress, uint8_t *pIncomingPacket);
static OADProtocol_Status_t processOadResetRsp(void* pSrcAddress, uint8_t *pIncomingPacket);
#endif

static incomingPacketProcess_t incomingPacketProcessTable[] =
{
    {OADProtocol_PACKET_TYPE_FW_VERSION_REQ,       processFwVersioReq},
    {OADProtocol_PACKET_TYPE_FW_VERSION_RSP,       processFwVersioRsp},
    {OADProtocol_PACKET_TYPE_OAD_IMG_IDENTIFY_REQ, processOadImgIdentifyReq},
    {OADProtocol_PACKET_TYPE_OAD_IMG_IDENTIFY_RSP, processOadImgIdentifyRsp},
    {OADProtocol_PACKET_TYPE_OAD_BLOCK_REQ,        processOadImgBlockReq},
    {OADProtocol_PACKET_TYPE_OAD_BLOCK_RSP,        processOadImgBlockRsp},
#ifdef OAD_ONCHIP
    {OADProtocol_PACKET_TYPE_OAD_RESET_REQ,        processOadResetReq},
    {OADProtocol_PACKET_TYPE_OAD_RESET_RSP,        processOadResetRsp},
#endif
};

/***** Variable declarations *****/

/* Set Default parameters structure */
static const OADProtocol_Params_t OADProtocol_defaultParams = {
    .pRadioAccessFxns       = 0,
    .pProtocolMsgCallbacks  = 0,
};

static OADProtocol_Params_t OADProtocol_params;

/***** Function definitions *****/
void OADProtocol_init(void) {

}

void OADProtocol_Params_init(OADProtocol_Params_t *params)
{
    *params = OADProtocol_defaultParams;
}

void OADProtocol_open(OADProtocol_Params_t *params)
{
    // Populate default params if not provided
    if (params == NULL)
    {
        OADProtocol_Params_init(&OADProtocol_params);
        params = &OADProtocol_params;
        memcpy(params, &OADProtocol_params, sizeof(OADProtocol_Params_t));
    }
    else
    {
        memcpy(&OADProtocol_params, params, sizeof(OADProtocol_Params_t));
    }
}

OADProtocol_Status_t OADProtocol_ParseIncoming(void* pSrcAddress, uint8_t* incomingPacket)
{    
    uint8_t cbIdx;
    OADProtocol_Status_t status = OADProtocol_Failed;

    /* Process the message */
    for(cbIdx = 0; cbIdx < (sizeof(incomingPacketProcessTable) / sizeof(incomingPacketProcess_t)) ;cbIdx++)
    {
        if(incomingPacketProcessTable[cbIdx].cmdId == incomingPacket[OADProtocol_PKT_CMDID_OFFSET])
        {
            status = incomingPacketProcessTable[cbIdx].packetprocessFn(pSrcAddress, incomingPacket);
        }
    }

    return status;
}

OADProtocol_Status_t OADProtocol_sendFwVersionReq(void* pDstAddress)
{
    OADProtocol_Status_t status = OADProtocol_Failed;
    uint8_t* pFwVersionReqPacket = NULL;

    //Allocate the buffer
    if(OADProtocol_params.pRadioAccessFxns->pfnRadioAccessAllocMsg)
    {
        pFwVersionReqPacket = OADProtocol_params.pRadioAccessFxns->pfnRadioAccessAllocMsg(
                                        OADProtocol_PACKET_TYPE_FW_VERSION_REQ_LEN);
    }

    if(pFwVersionReqPacket == NULL)
    {
        return status;
    }

    pFwVersionReqPacket[OADProtocol_PKT_CMDID_OFFSET] = OADProtocol_PACKET_TYPE_FW_VERSION_REQ;

    if(OADProtocol_params.pRadioAccessFxns->pfnRadioAccessPacketSend)
    {
        status = OADProtocol_params.pRadioAccessFxns->pfnRadioAccessPacketSend(pDstAddress,
                                        pFwVersionReqPacket,
                                        OADProtocol_PACKET_TYPE_FW_VERSION_REQ_LEN);
    }

    return status;
}

OADProtocol_Status_t OADProtocol_sendFwVersionRsp(void* pDstAddress, char *fwVersion)
{
    OADProtocol_Status_t status = OADProtocol_Failed;
    uint8_t* pFwVersionRspPacket = NULL;

    //Allocate the buffer
    if(OADProtocol_params.pRadioAccessFxns->pfnRadioAccessAllocMsg)
    {
        pFwVersionRspPacket =OADProtocol_params.pRadioAccessFxns->pfnRadioAccessAllocMsg(
                                        OADProtocol_PACKET_TYPE_FW_VERSION_RSP_LEN);
    }

    if(pFwVersionRspPacket == NULL)
    {
        return status;
    }

    pFwVersionRspPacket[OADProtocol_PKT_CMDID_OFFSET] = OADProtocol_PACKET_TYPE_FW_VERSION_RSP;
    memcpy(&(pFwVersionRspPacket[OADProtocol_VER_RSP_VERSIONSTRING_OFFSET]), fwVersion, OADProtocol_FW_VERSION_STR_LEN);

    if(OADProtocol_params.pRadioAccessFxns->pfnRadioAccessPacketSend)
    {
        status = OADProtocol_params.pRadioAccessFxns->pfnRadioAccessPacketSend(pDstAddress,
                                        pFwVersionRspPacket,
                                        OADProtocol_PACKET_TYPE_FW_VERSION_RSP_LEN);
    }
    
    return status;
}

OADProtocol_Status_t OADProtocol_sendImgIdentifyReq(void* pDstAddress, uint8_t imgId, uint8_t *pImgInfoData)
{
    OADProtocol_Status_t status = OADProtocol_Failed;
    uint8_t* pOadImgIdentifyReqPacket = NULL;

    //Allocate the buffer
    if(OADProtocol_params.pRadioAccessFxns->pfnRadioAccessAllocMsg)
    {
        pOadImgIdentifyReqPacket = OADProtocol_params.pRadioAccessFxns->pfnRadioAccessAllocMsg(
                                        OADProtocol_PACKET_TYPE_OAD_IMG_IDENTIFY_REQ_LEN);
    }

    if(pOadImgIdentifyReqPacket == NULL)
    {
        return status;
    }

    pOadImgIdentifyReqPacket[OADProtocol_PKT_CMDID_OFFSET] = OADProtocol_PACKET_TYPE_OAD_IMG_IDENTIFY_REQ;
    pOadImgIdentifyReqPacket[OADProtocol_IMG_IDENTIFY_REQ_IMG_ID_OFFSET] = imgId;
    memcpy(&(pOadImgIdentifyReqPacket[OADProtocol_IMG_IDENTIFY_REQ_IMG_HDR_OFFSET]), pImgInfoData, OADProtocol_IMAGE_HDR_LEN);

    if(OADProtocol_params.pRadioAccessFxns->pfnRadioAccessPacketSend)
    {
        status = OADProtocol_params.pRadioAccessFxns->pfnRadioAccessPacketSend(pDstAddress,
                                        pOadImgIdentifyReqPacket,
                                        OADProtocol_PACKET_TYPE_OAD_IMG_IDENTIFY_REQ_LEN);
    }

    return status;
}

OADProtocol_Status_t OADProtocol_sendOadIdentifyImgRsp(void* pDstAddress, uint8_t rspStatus)
{
    OADProtocol_Status_t status = OADProtocol_Failed;
    uint8_t* pOadImgIdentifyRspPacket = NULL;

    //Allocate the buffer
    if(OADProtocol_params.pRadioAccessFxns->pfnRadioAccessAllocMsg)
    {
        pOadImgIdentifyRspPacket = OADProtocol_params.pRadioAccessFxns->pfnRadioAccessAllocMsg(
                                        OADProtocol_PACKET_TYPE_OAD_IMG_IDENTIFY_RSP_LEN);
    }

    if(pOadImgIdentifyRspPacket == NULL)
    {
        return status; 
    }

    pOadImgIdentifyRspPacket[OADProtocol_PKT_CMDID_OFFSET] = OADProtocol_PACKET_TYPE_OAD_IMG_IDENTIFY_RSP;
    pOadImgIdentifyRspPacket[OADProtocol_IMG_IDENTIFY_RSP_STATUS_OFFSET] = rspStatus;

    if(OADProtocol_params.pRadioAccessFxns->pfnRadioAccessPacketSend)
    {
        status = OADProtocol_params.pRadioAccessFxns->pfnRadioAccessPacketSend(pDstAddress,
                                        pOadImgIdentifyRspPacket,
                                        OADProtocol_PACKET_TYPE_OAD_IMG_IDENTIFY_RSP_LEN);
    }

    return status;
}

OADProtocol_Status_t OADProtocol_sendOadImgBlockReq(void* pDstAddress, uint8_t imgId, uint16_t blockNum, uint16_t MultiBlockSize)
{
    OADProtocol_Status_t status = OADProtocol_Failed;
    uint8_t* pOadBlockReqPacket = NULL;

    //Allocate the buffer
    if(OADProtocol_params.pRadioAccessFxns->pfnRadioAccessAllocMsg)
    {
        pOadBlockReqPacket = OADProtocol_params.pRadioAccessFxns->pfnRadioAccessAllocMsg(
                                        OADProtocol_PACKET_TYPE_OAD_BLOCK_REQ_LEN);
    }

    if(pOadBlockReqPacket == NULL)
    { 
        return status; 
    }

    pOadBlockReqPacket[OADProtocol_PKT_CMDID_OFFSET] = OADProtocol_PACKET_TYPE_OAD_BLOCK_REQ;

    pOadBlockReqPacket[OADProtocol_BLOCK_REQ_IMG_ID_OFFSET] = imgId;

    pOadBlockReqPacket[OADProtocol_BLOCK_REQ_BLOCK_NUM_OFFSET]  = blockNum & 0xFF;
    pOadBlockReqPacket[OADProtocol_BLOCK_REQ_BLOCK_NUM_OFFSET + 1]  = (blockNum >> 8) & 0xFF;

    pOadBlockReqPacket[OADProtocol_BLOCK_REQ_MULTI_BLOCK_SIZE_OFFSET]  = MultiBlockSize & 0xFF;
    pOadBlockReqPacket[OADProtocol_BLOCK_REQ_MULTI_BLOCK_SIZE_OFFSET + 1]  = (MultiBlockSize >> 8) & 0xFF;

    if(OADProtocol_params.pRadioAccessFxns->pfnRadioAccessPacketSend)
    {
        status = OADProtocol_params.pRadioAccessFxns->pfnRadioAccessPacketSend(pDstAddress,
                                        pOadBlockReqPacket,
                                        OADProtocol_PACKET_TYPE_OAD_BLOCK_REQ_LEN);
    }    

    return status;
}

OADProtocol_Status_t OADProtocol_sendOadImgBlockRsp(void* pDstAddress, uint8_t imgId, uint16_t blockNum, uint8_t *block)
{

    OADProtocol_Status_t status = OADProtocol_Failed;
    uint8_t* pOadBlockRspPacket = NULL;

    //Allocate the buffer
    if(OADProtocol_params.pRadioAccessFxns->pfnRadioAccessAllocMsg)
    {
        pOadBlockRspPacket = OADProtocol_params.pRadioAccessFxns->pfnRadioAccessAllocMsg(
                                        OADProtocol_PACKET_TYPE_OAD_BLOCK_RSP_LEN);
    }

    if(pOadBlockRspPacket == NULL)
    {
        return status; 
    }

    pOadBlockRspPacket[OADProtocol_PKT_CMDID_OFFSET] = OADProtocol_PACKET_TYPE_OAD_BLOCK_RSP;

    pOadBlockRspPacket[OADProtocol_BLOCK_RSP_IMG_ID_OFFSET] = imgId;

    pOadBlockRspPacket[OADProtocol_BLOCK_RSP_BLOCK_NUM_OFFSET]  = blockNum & 0xFF;
    pOadBlockRspPacket[OADProtocol_BLOCK_RSP_BLOCK_NUM_OFFSET + 1]  = (blockNum >> 8) & 0xFF;

    memcpy(&(pOadBlockRspPacket[OADProtocol_BLOCK_RSP_BLOCK_DATA_OFFSET]), block, (OADStorage_BLOCK_SIZE - OADStorage_BLK_NUM_HDR_SZ));

    if(OADProtocol_params.pRadioAccessFxns->pfnRadioAccessPacketSend)
    {
        status = OADProtocol_params.pRadioAccessFxns->pfnRadioAccessPacketSend(pDstAddress,
                                        pOadBlockRspPacket,
                                        OADProtocol_PACKET_TYPE_OAD_BLOCK_RSP_LEN);
    }     

    return status;
}

#ifdef OAD_ONCHIP
OADProtocol_Status_t OADProtocol_sendOadResetReq(void* pDstAddress)
{
    OADProtocol_Status_t status = OADProtocol_Failed;
    uint8_t* pOadResetReqPacket = NULL;

    //Allocate the buffer
    if(OADProtocol_params.pRadioAccessFxns->pfnRadioAccessAllocMsg)
    {
        pOadResetReqPacket = OADProtocol_params.pRadioAccessFxns->pfnRadioAccessAllocMsg(
                                                OADProtocol_PACKET_TYPE_OAD_RESET_REQ_LEN);
    }

    if(pOadResetReqPacket == NULL)
    {
        return status;
    }

    pOadResetReqPacket[OADProtocol_PKT_CMDID_OFFSET] = OADProtocol_PACKET_TYPE_OAD_RESET_REQ;

    if(OADProtocol_params.pRadioAccessFxns->pfnRadioAccessPacketSend)
    {
        status = OADProtocol_params.pRadioAccessFxns->pfnRadioAccessPacketSend(pDstAddress,
                                        pOadResetReqPacket,
                                        OADProtocol_PACKET_TYPE_OAD_RESET_REQ_LEN);
    }

    return status;
}

OADProtocol_Status_t OADProtocol_sendOadResetRsp(void* pDstAddress)
{
    OADProtocol_Status_t status = OADProtocol_Failed;
    uint8_t* pOadResetRspPacket = NULL;

    //Allocate the buffer
    if(OADProtocol_params.pRadioAccessFxns->pfnRadioAccessAllocMsg)
    {
        pOadResetRspPacket = OADProtocol_params.pRadioAccessFxns->pfnRadioAccessAllocMsg(
                                                OADProtocol_PACKET_TYPE_OAD_RESET_RSP_LEN);
    }

    if(pOadResetRspPacket == NULL)
    {
        return status;
    }

    pOadResetRspPacket[OADProtocol_PKT_CMDID_OFFSET] = OADProtocol_PACKET_TYPE_OAD_RESET_RSP;

    if(OADProtocol_params.pRadioAccessFxns->pfnRadioAccessPacketSend)
    {
        status = OADProtocol_params.pRadioAccessFxns->pfnRadioAccessPacketSend(pDstAddress,
                                        pOadResetRspPacket,
                                        OADProtocol_PACKET_TYPE_OAD_RESET_RSP_LEN);
    }

    return status;
}
#endif //OAD_ONCHIP

static OADProtocol_Status_t processFwVersioReq(void* pSrcAddress, uint8_t* pIncomingPacket)
{
    OADProtocol_Status_t status = OADProtocol_Failed;

    //call application callback
    if(OADProtocol_params.pProtocolMsgCallbacks->pfnFwVersionReqCb != NULL)
    {
        OADProtocol_params.pProtocolMsgCallbacks->pfnFwVersionReqCb(pSrcAddress);
    }

    //TODO: process packet and send response if needed
    status = OADProtocol_Status_Success;

    return status;
}

static OADProtocol_Status_t processFwVersioRsp(void* pSrcAddress, uint8_t *pIncomingPacket)
{
    OADProtocol_Status_t status = OADProtocol_Failed;

    //call application callback
    if(OADProtocol_params.pProtocolMsgCallbacks->pfnFwVersionRspCb != NULL)
    {
        OADProtocol_params.pProtocolMsgCallbacks->pfnFwVersionRspCb(pSrcAddress, (char*) &(pIncomingPacket[OADProtocol_VER_RSP_VERSIONSTRING_OFFSET]));
    }

    //TODO: process packet and send response if needed
    status = OADProtocol_Status_Success;

    return status;
}

static OADProtocol_Status_t processOadImgIdentifyReq(void* pSrcAddress, uint8_t *pIncomingPacket)
{
    OADProtocol_Status_t status = OADProtocol_Failed;

    //call application callback
    if(OADProtocol_params.pProtocolMsgCallbacks->pfnOadImgIdentifyReqCb != NULL)
    {
        OADProtocol_params.pProtocolMsgCallbacks->pfnOadImgIdentifyReqCb(pSrcAddress,
                 pIncomingPacket[OADProtocol_IMG_IDENTIFY_REQ_IMG_ID_OFFSET],
                 &(pIncomingPacket[OADProtocol_IMG_IDENTIFY_REQ_IMG_HDR_OFFSET]));
    }

    //TODO: process packet and send response if needed
    status = OADProtocol_Status_Success;

    return status;
}

static OADProtocol_Status_t processOadImgIdentifyRsp(void* pSrcAddress, uint8_t *pIncomingPacket)
{
    OADProtocol_Status_t status = OADProtocol_Failed;

    //call application callback
    if(OADProtocol_params.pProtocolMsgCallbacks->pfnOadImgIdentifyRspCb != NULL)
    {
        OADProtocol_params.pProtocolMsgCallbacks->pfnOadImgIdentifyRspCb(pSrcAddress,
                 pIncomingPacket[OADProtocol_IMG_IDENTIFY_RSP_STATUS_OFFSET]);
    }

    //TODO: process packet and send response if needed
    status = OADProtocol_Status_Success;

    return status;
}

static OADProtocol_Status_t processOadImgBlockReq(void* pSrcAddr, uint8_t *pIncomingPacket)
{
    OADProtocol_Status_t status = OADProtocol_Failed;

    uint8_t imgHdr = pIncomingPacket[OADProtocol_BLOCK_REQ_IMG_ID_OFFSET];

    uint16_t blockNum = ( pIncomingPacket[OADProtocol_BLOCK_REQ_BLOCK_NUM_OFFSET] & 0xFF) |
                        ((pIncomingPacket[OADProtocol_BLOCK_REQ_BLOCK_NUM_OFFSET + 1] & 0xFF) << 8);

    uint16_t multiBlockSize = ( pIncomingPacket[OADProtocol_BLOCK_REQ_MULTI_BLOCK_SIZE_OFFSET] & 0xFF) |
                             ((pIncomingPacket[OADProtocol_BLOCK_REQ_MULTI_BLOCK_SIZE_OFFSET + 1] & 0xFF) << 8);

    //call application callback
    if(OADProtocol_params.pProtocolMsgCallbacks->pfnOadBlockReqCb != NULL)
    {
        OADProtocol_params.pProtocolMsgCallbacks->pfnOadBlockReqCb(pSrcAddr, imgHdr, blockNum, multiBlockSize);
    }

    status = OADProtocol_Status_Success;

    return status;
}

static OADProtocol_Status_t processOadImgBlockRsp(void* pSrcAddress, uint8_t *pIncomingPacket)
{
    OADProtocol_Status_t status = OADProtocol_Failed;

    uint8_t imgHdr = pIncomingPacket[OADProtocol_BLOCK_RSP_IMG_ID_OFFSET];

    uint16_t blockNum = ( pIncomingPacket[OADProtocol_BLOCK_RSP_BLOCK_NUM_OFFSET] & 0xFF) |
                        ((pIncomingPacket[OADProtocol_BLOCK_RSP_BLOCK_NUM_OFFSET + 1] & 0xFF) << 8);

    //call application callback
    if(OADProtocol_params.pProtocolMsgCallbacks->pfnOadBlockRspCb != NULL)
    {
        OADProtocol_params.pProtocolMsgCallbacks->pfnOadBlockRspCb(pSrcAddress,
                  imgHdr,
                  blockNum,
                  &(pIncomingPacket[OADProtocol_BLOCK_RSP_BLOCK_NUM_OFFSET]));
    }

    status = OADProtocol_Status_Success;

    return status;
}

#ifdef OAD_ONCHIP

static OADProtocol_Status_t processOadResetReq(void* pSrcAddress, uint8_t* pIncomingPacket)
{
    OADProtocol_Status_t status = OADProtocol_Failed;

    //call application callback
    if(OADProtocol_params.pProtocolMsgCallbacks->pfnOadResetReqCb != NULL)
    {
        OADProtocol_params.pProtocolMsgCallbacks->pfnOadResetReqCb(pSrcAddress);
    }

    //TODO: process packet and send response if needed
    status = OADProtocol_Status_Success;

    return status;
}

static OADProtocol_Status_t processOadResetRsp(void* pSrcAddress, uint8_t* pIncomingPacket)
{
    OADProtocol_Status_t status = OADProtocol_Failed;

    //call application callback
    if(OADProtocol_params.pProtocolMsgCallbacks->pfnOadResetRspCb != NULL)
    {
        OADProtocol_params.pProtocolMsgCallbacks->pfnOadResetRspCb(pSrcAddress);
    }

    //TODO: process packet and send response if needed
    status = OADProtocol_Status_Success;

    return status;
}

#endif //OAD_ONCHIP
