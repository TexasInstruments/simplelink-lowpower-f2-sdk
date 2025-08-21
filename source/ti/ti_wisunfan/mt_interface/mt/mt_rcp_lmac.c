/******************************************************************************

 @file  mt_rcp_lmac.c

 @brief Monitor/Test functions for MT RCP LMAC commands/callbacks

 Group: Connectivity 
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2025, Texas Instruments Incorporated
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

/******************************************************************************
 Includes
 *****************************************************************************/
#include <string.h>

#include "api_mac.h"

#include "mt_rcp_lmac.h"
#include "mt_pkt.h"
#include "mt_rpc.h"
#include "mt_util.h"
#include "mlme.h"

#include "hmac_map_direct.h"

#if defined(TIMAC_ROM_IMAGE_BUILD)
#include "icall_osal_rom_jt.h"
#else
#include "icall_osal_map_direct.h"
#endif

#if defined(MT_RCP_LMAC_FUNC)

/******************************************************************************
 * Macros
 *****************************************************************************/
/*! AREQ RPC response for MT_MAC callbacks (indications/confirms) */
#define MT_ARSP_RCP_LMAC ((uint8_t)MTRPC_CMD_AREQ | (uint8_t)MTRPC_SYS_RCP_LMAC)

/*! SRSP RPC response for MT_MAC requests (commands/responses) */
#define MT_SRSP_RCP_LMAC ((uint8_t)MTRPC_CMD_SRSP | (uint8_t)MTRPC_SYS_RCP_LMAC)

#define MT_RCP_LMAC_FIXED_LEN_DATA_IND (sizeof(rcp_data_ind_t) - 2*sizeof(uint8_t*))
#define MT_RCP_LMAC_FIXED_LEN_DATA_CNF (sizeof(rcp_data_cnf_t) - 2*sizeof(uint8_t*))
#define MT_RCP_LMAC_FIXED_LEN_MAC_CFG_SET_CNF  (sizeof(rcp_mac_config_set_cnf_t) - sizeof(uint8_t*))


/*!
 * @brief   Wrapper for MT_sendResponse() for MT_MAC Callback AREQ
 *
 * @param   rspId  - response ID
 * @param   rspLen - length of response buffer
 * @param   pRsp   - pointer to response buffer
 */
static void sendCRSP(uint8_t rspId, uint16_t rspLen, uint8_t *pRsp)
{
    // for RCP LMAC case keeping code simple - no toggling and checking on what callbacks are enabled
    (void)MT_sendResponse(MT_ARSP_RCP_LMAC, rspId, rspLen, pRsp);    
}

/*!
 * @brief   Wrapper for MT_sendResponse() for data SRSP
 *
 * @param   rspId  - response ID
 * @param   rspLen - length of response buffer
 * @param   pRsp   - pointer to response buffer
 */
static void sendDRSP(uint8_t rspId, uint16_t rspLen, uint8_t *pRsp)
{
    (void)MT_sendResponse(MT_SRSP_RCP_LMAC, rspId, rspLen, pRsp);
}

/*!
 * @brief   Wrapper for MT_sendResponse() for status SRSP
 *
 * @param   rspId - response ID
 * @param   rsp   - response status
 */
static void sendSRSP(uint8_t rspId, uint8_t rsp)
{
    (void)MT_sendResponse(MT_SRSP_RCP_LMAC, rspId, 1, &rsp);
}

/*!
 * @brief   Copy security parameters from serial byte array to ApiMac struct
 *
 * @param   pSec - Pointer to security structure
 * @param   pSba - Pointer to serial byte array
 */
static void rcpLMacSbaToSec(mlme_security_t *pSec, uint8_t *pSba)
{
    /* Key source */
    memcpy(pSec->Keysource, pSba, APIMAC_KEY_SOURCE_MAX_LEN);
    pSba += APIMAC_KEY_SOURCE_MAX_LEN;

    /* Security level */
    pSec->SecurityLevel = *pSba++;

    /* Key identifier mode */
    pSec->KeyIdMode = *pSba++;

    /* Key index */
    pSec->KeyIndex = *pSba;
}

/*!
 * @brief   Copy security parameters from serial byte array to fhnt_entry_t struct
 *
 * @param   pFHNTentry - Pointer to fhnt_entry_t structure
 * @param   pSba - Pointer to serial byte array
 */
static void rcpLMacSbaToFHNTentry(fhnt_entry_t *pFHNTentry, uint8_t *pSba)
{
    pFHNTentry->ufsi = Util_parseUint32(pSba); 
    pSba += 4;
    pFHNTentry->ref_timeStamp = Util_parseUint32(pSba); 
    pSba += 4;
    pFHNTentry->dwellInterval = *pSba++;
    pFHNTentry->channelFunc = *pSba++;
    pFHNTentry->fixedChannel = *pSba++;
    memcpy(pFHNTentry->bitMap, pSba, MAC_154G_CHANNEL_BITMAP_SIZ);
    pSba += MAC_154G_CHANNEL_BITMAP_SIZ;
    pFHNTentry->numChannels = *pSba++;
    memcpy(pFHNTentry->extAddr, pSba, APIMAC_SADDR_EXT_LEN);    
}


static void rcpLMacDataReq(Mt_mpb_t *pMpb)
{
    uint8_t status = RCP_SUCCESS;
   
    rcp_cmd_t hostReq;
    rcp_data_req_t dReq;
    uint8_t *pBuf = pMpb->pData;

    dReq.req_type = *pBuf++;
    dReq.msdu_handle = *pBuf++;
    dReq.min_tx_frame_count = Util_parseUint32(pBuf) ; 
    pBuf += 4;
    dReq.frame_count_offset = *pBuf++;

    rcpLMacSbaToSec(&dReq.sec, pBuf);
    pBuf += sizeof(mlme_security_t); 
    
    dReq.mdata_offset = Util_parseUint16(pBuf) ; // Offset to encrypted data in data_ptr
    pBuf += 2;
    dReq.utie_offset = Util_parseUint16(pBuf) ; // Offset to encrypted data in data_ptr
    pBuf += 2;
    dReq.btie_offset = Util_parseUint16(pBuf) ; // Offset to encrypted data in data_ptr
    pBuf += 2;

    rcpLMacSbaToFHNTentry(&dReq.fhnt_entry, pBuf);
    pBuf += sizeof(fhnt_entry_t);

    dReq.data_len = Util_parseUint16(pBuf) ; // Offset to encrypted data in data_ptr
    pBuf += 2;
    dReq.data_ptr = pBuf;

    //mvtodo: to do check of length inconsistencies here

    /* populate host Req */
    hostReq.rcp_cmd_type = RCP_DATA_REQ;
    hostReq.rcp_data = (void *)&dReq;

    /* Send host a response */
    sendSRSP(MT_RCP_LMAC_DATA_REQ, status);

     /* Send for further processing */
    rcp_lmac_from_host(&hostReq);   
    
}

static void rcpLMacCfgGet(Mt_mpb_t *pMpb)
{
    uint8_t status = RCP_SUCCESS;

    rcp_cmd_t hostReq;    
    rcp_mac_config_get_t cfgGet;
    uint8_t *pBuf = pMpb->pData;

    cfgGet.attr = *pBuf++;    

    /* populate host Req */
    hostReq.rcp_cmd_type = RCP_MAC_CONFIG_GET;
    hostReq.rcp_data = (void *)&cfgGet;

    /* Send host a response */
    sendSRSP(MT_RCP_LMAC_CFG_GET, status);

     /* Send for further processing */
    rcp_lmac_from_host(&hostReq);   
}

static void rcpLMacCfgSet(Mt_mpb_t *pMpb)
{
    uint8_t status = RCP_SUCCESS;

    rcp_cmd_t hostReq;
    rcp_mac_config_set_t cfgSet;
    uint8_t *pBuf = pMpb->pData;

    cfgSet.attr =  Util_parseUint16(pBuf);
    pBuf += 2;

    cfgSet.val_len = Util_parseUint16(pBuf);
    pBuf += 2;

    cfgSet.val = pBuf;

    /* populate host Req */
    hostReq.rcp_cmd_type = RCP_MAC_CONFIG_SET;
    hostReq.rcp_data = (void *)&cfgSet;

    /* Send host a response */
    sendSRSP(MT_RCP_LMAC_CFG_SET, status);

     /* Send for further processing */
    rcp_lmac_from_host(&hostReq);   
}

static void rcpLMacMacInit(Mt_mpb_t *pMpb)
{
    uint8_t status = RCP_SUCCESS;

    rcp_cmd_t hostReq;
    mlme_start_t macStart;
    uint8_t *pBuf = pMpb->pData;

    macStart.StartTime = Util_parseUint32(pBuf);
    pBuf += 4;
    macStart.PANId = Util_parseUint16(pBuf);
    pBuf += 2;
    macStart.LogicalChannel = *pBuf++;
    macStart.ChannelPage = *pBuf++;
    macStart.PhyID = *pBuf++;
    macStart.SuperframeOrder = *pBuf++;
    macStart.BeaconOrder = *pBuf++;
    macStart.PANCoordinator = *pBuf++;
    macStart.BatteryLifeExtension = *pBuf++;
    macStart.CoordRealignment = *pBuf++;

    rcpLMacSbaToSec(&macStart.CoordRealignKey, pBuf);
    pBuf += sizeof(mlme_security_t); 

    rcpLMacSbaToSec(&macStart.BeaconRealignKey, pBuf);
    pBuf += sizeof(mlme_security_t); 
    
    macStart.mpmParams.eBeaconOrder = 0;
    macStart.mpmParams.offsetTimeSlot = 0;
    macStart.mpmParams.NBPANEBeaconOrder = 0;
    macStart.mpmParams.pIEIDs = NULL;
    macStart.mpmParams.numIEs = 0;

    macStart.startFH = *pBuf++;

    /* populate host Req */
    hostReq.rcp_cmd_type = RCP_MAC_INIT;
    hostReq.rcp_data = (void *)&macStart;

    /* Send host a response */
    sendSRSP(MT_RCP_LMAC_MAC_INIT, status);

     /* Send for further processing */
    rcp_lmac_from_host(&hostReq);   
}


static void rcpLMacRcpInit(Mt_mpb_t *pMpb)
{
    uint8_t status = RCP_SUCCESS;
    rcp_cmd_t hostReq;

    /* populate host Req */
    hostReq.rcp_cmd_type = RCP_INIT;
    hostReq.rcp_data = NULL;

    /* Send host a response */
    sendSRSP(MT_RCP_LMAC_RCP_INIT, status);

     /* Send for further processing */
    rcp_lmac_from_host(&hostReq);   
}


static void rcpLMacFHInit(Mt_mpb_t *pMpb)
{
    uint8_t status = RCP_SUCCESS;
    rcp_cmd_t hostReq;

    /* populate host Req */
    hostReq.rcp_cmd_type = RCP_FH_INIT;
    hostReq.rcp_data = NULL;

    /* Send host a response */
    sendSRSP(MT_RCP_LMAC_FH_INIT, status);

     /* Send for further processing */
    rcp_lmac_from_host(&hostReq);   
}

static void rcpLMacMacReset(Mt_mpb_t *pMpb)
{
    uint8_t status = RCP_SUCCESS;
    rcp_cmd_t hostReq;

    /* populate host Req */
    hostReq.rcp_cmd_type = RCP_MAC_RESET;
    hostReq.rcp_data = NULL;

    /* Send host a response */
    sendSRSP(MT_RCP_LMAC_MAC_RESET, status);

     /* Send for further processing */
    rcp_lmac_from_host(&hostReq);   
}



/*!
 Processes MT MAC commands received from the host

 Public function that is defined in mt_mac.h
 */
uint8_t MtRcpLMac_commandProcessing(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_success;

    switch(pMpb->cmd1)
    {
        case MT_RCP_LMAC_DATA_REQ:
            MT_rcp_LMAC_dbg.num_data_req_rxed++;
            rcpLMacDataReq(pMpb);
            break;

        case MT_RCP_LMAC_RCP_INIT:
            MT_rcp_LMAC_dbg.num_rcp_init_req_rxed++;
            rcpLMacRcpInit(pMpb);
            break;

        case MT_RCP_LMAC_MAC_INIT:
            MT_rcp_LMAC_dbg.num_mac_init_req_rxed++;
            rcpLMacMacInit(pMpb);
            break;

        case MT_RCP_LMAC_FH_INIT:
            MT_rcp_LMAC_dbg.num_fh_init_req_rxed++;
            rcpLMacFHInit(pMpb);
            break;

        case MT_RCP_LMAC_MAC_RESET:
            MT_rcp_LMAC_dbg.num_mac_reset_req_rxed++;
            rcpLMacMacReset(pMpb);
            break;

        case MT_RCP_LMAC_CFG_GET:
            MT_rcp_LMAC_dbg.num_mac_cfg_get_req_rxed++;
            rcpLMacCfgGet(pMpb);
            break;  

        case MT_RCP_LMAC_CFG_SET:
            MT_rcp_LMAC_dbg.num_mac_cfg_set_req_rxed++;
            rcpLMacCfgSet(pMpb);
            break;

        default:
            status = ApiMac_status_commandIDError;
            break;
    }

    return(status);
}


/*!
 * Process MAC_DATA_IND callback issued by MAC
 *
 * Public function that is defined in mt_mac.h
 */
void MtRcpLMac_DataInd(rcp_data_ind_t *pInd)
{
    uint16_t rspLen;
    uint8_t *pRspBuf;
    
    rspLen = MT_RCP_LMAC_FIXED_LEN_DATA_IND + pInd->data_len; //LMAC does not parse IEs - so len contains data and IEs
    pRspBuf = MAP_ICall_malloc(rspLen);

    if(pRspBuf != NULL)
    {
        /* populate the buffer */
        uint8_t *pBuf = pRspBuf;

         /* rx_timestamp */
        pBuf = Util_bufferUint32(pBuf, pInd->rx_timestamp);

        /* link quality  */
        *pBuf++ = pInd->link_quality;

        /* signal_dbm*/
        *pBuf++ = pInd->signal_dbm;   

        /* data len */
        pBuf = Util_bufferUint16(pBuf, pInd->data_len);

        /* copy actual packet */
        if(pInd->data_len)
        {
            /*copy packet content */
            memcpy(pBuf, pInd->data_ptr, pInd->data_len);             
        }
          
        
        //free the pRxBuf holding the pointer to ack frame 
        if(pInd->buf_free != NULL)
        {
            uint8_t *bufptr = pInd->buf_free;
            MAP_mac_msg_deallocate((uint8_t **)&bufptr);             
        }

        //free memory of data_ind
        MAP_mac_msg_deallocate((uint8_t **)&pInd);

        MT_rcp_LMAC_dbg.num_data_ind_sent++;
        
        /* Send host a response */
        sendCRSP(MT_RCP_LMAC_DATA_IND, rspLen, pRspBuf);

        /* Give back RespBuf memory */
        MAP_ICall_free(pRspBuf);
    }    
}

void MtRcpLMac_DataCnf(rcp_data_cnf_t *pCnf)
{
    uint16_t rspLen;
    uint8_t *pRspBuf;
    
    rspLen = MT_RCP_LMAC_FIXED_LEN_DATA_CNF + pCnf->ack_frame_len; //LMAC does not parse IEs - so len contains data and IEs
    pRspBuf = MAP_ICall_malloc(rspLen);

    if(pRspBuf != NULL)
    {
        /* populate the buffer */
        uint8_t *pBuf = pRspBuf;

        /* cnf type */
        *pBuf++ = pCnf->cnf_type;

        /* MSDU handle */
        *pBuf++ = pCnf->msdu_handle;

        /* Status */    
        *pBuf++ = pCnf->status;
        
        /* Timestamp (backoffs) */
        pBuf = Util_bufferUint32(pBuf, pCnf->tx_timestamp);

        /* Timestamp2 (MAC units) */
        pBuf = Util_bufferUint32(pBuf, pCnf->rx_ack_timestamp);

        /*CCA Retries */
        *pBuf++ = pCnf->cca_retries;

        /*TX Retries */
        *pBuf++ = pCnf->tx_retries;

        /* tx_frame_count */
        pBuf = Util_bufferUint32(pBuf, pCnf->tx_frame_count);

        /* key_index */
        *pBuf++ = pCnf->key_index;

        /* ack_frame_len */
        pBuf = Util_bufferUint16(pBuf, pCnf->ack_frame_len);

        if(pCnf->ack_frame_len)
        {
            /*copy ack content */
            memcpy(pBuf, pCnf->ack_frame, pCnf->ack_frame_len);            
        }

        //free the pRxBuf holding the pointer to ack frame 
        if(pCnf->buf_free != NULL)
        {
            uint8_t *bufptr = pCnf->buf_free;
            MAP_mac_msg_deallocate((uint8_t **)&bufptr);            
           
        }

        MT_rcp_LMAC_dbg.num_data_cnf_sent++;
        if (pCnf->cnf_type == RCP_DATA_CNF_ASYNC) {
            MT_rcp_LMAC_dbg.num_data_cnf_async_sent++;
        } else if (pCnf->cnf_type == RCP_DATA_CNF_BROADCAST) {
            MT_rcp_LMAC_dbg.num_data_cnf_bc_sent++;
        } else if (pCnf->cnf_type == RCP_DATA_CNF_UNICAST) {
            MT_rcp_LMAC_dbg.num_data_cnf_uc_sent++;
        }

        //free memory of data_cnf
        MAP_mac_msg_deallocate((uint8_t **)&pCnf);


        /* Send host a response */
        sendCRSP(MT_RCP_LMAC_DATA_CNF, rspLen, pRspBuf);

        /* Give back RespBuf memory */
        MAP_ICall_free(pRspBuf);
    }
}


void MtRcpLMac_RcpInitCnf(rcp_init_cnf_t *pCnfRcpInit)
{
    uint16_t rspLen;
    uint8_t *pRspBuf;
    
    rspLen = sizeof(rcp_init_cnf_t); //LMAC does not parse IEs - so len contains data and IEs
    pRspBuf = MAP_ICall_malloc(rspLen);

    if(pRspBuf != NULL)
    {               
        memcpy(pRspBuf, pCnfRcpInit->device_ext_addr, rspLen);       
    }

    MT_rcp_LMAC_dbg.num_rcp_init_cnf_sent++;

    /* Send host a response */
    sendCRSP(MT_RCP_LMAC_RCP_INIT_CNF, rspLen, pRspBuf);

    /* Give back RespBuf memory */
    MAP_ICall_free(pRspBuf);
}

void MtRcpLMac_MacCfgGetCnf(rcp_mac_config_get_cnf_t *pCnfMacCfgGet)
{
    uint16_t rspLen;
    uint8_t *pRspBuf;
    
    rspLen = sizeof(rcp_mac_config_get_cnf_t); //revisit this
    pRspBuf = MAP_ICall_malloc(rspLen);

    if(pRspBuf != NULL)
    {
        //populate later when get cfg is implemented
    }

    MT_rcp_LMAC_dbg.num_mac_cfg_get_cnf_sent++;

    /* Send host a response */
    sendCRSP(MT_RCP_LMAC_MAC_CFG_GET_CNF, rspLen, pRspBuf);

    /* Give back RespBuf memory */
    MAP_ICall_free(pRspBuf);
}

void MtRcpLMac_MacCfgSetCnf(rcp_mac_config_set_cnf_t *pCnfMacCfgSet, uint8_t mt_cmd1)
{
    uint16_t rspLen;
    uint8_t *pRspBuf;
    
    rspLen = MT_RCP_LMAC_FIXED_LEN_MAC_CFG_SET_CNF + pCnfMacCfgSet->val_len;     
    pRspBuf = MAP_ICall_malloc(rspLen);

    if(pRspBuf != NULL)
    {
        /* populate the buffer */
        uint8_t *pBuf = pRspBuf;

        pBuf = Util_bufferUint16(pBuf, pCnfMacCfgSet->attr);
        *pBuf++ = pCnfMacCfgSet->status;
        pBuf = Util_bufferUint16(pBuf, pCnfMacCfgSet->val_len);
        if(pCnfMacCfgSet->val_len)
        {
            /* copy attribute value */
            memcpy(pBuf, pCnfMacCfgSet->val, pCnfMacCfgSet->val_len); 
            
            /* free the memory  */
            uint8_t *bufptr = pCnfMacCfgSet->val;
            MAP_mac_msg_deallocate((uint8_t **)&bufptr);           
        }
    }

    switch(mt_cmd1)
    {
        case MT_RCP_LMAC_MAC_CFG_SET_CNF:
            MT_rcp_LMAC_dbg.num_mac_cfg_set_cnf_sent++;
            break;
        case MT_RCP_LMAC_MAC_INIT_CNF:
            MT_rcp_LMAC_dbg.num_mac_init_cnf_sent++;
            break;
        case MT_RCP_LMAC_FH_INIT_CNF:
            MT_rcp_LMAC_dbg.num_fh_init_cnf_sent++;
            break;
        case MT_RCP_LMAC_MAC_RESET_CNF:
            MT_rcp_LMAC_dbg.num_mac_reset_cnf_sent++;
            break;
        default:
            // do nothing
            break;
    }

    /* Send host a response */
    sendCRSP(mt_cmd1, rspLen, pRspBuf);

    /* Give back RespBuf memory */
    MAP_ICall_free(pRspBuf);
}

#endif /* MT_RCP_LMAC_FUNC */