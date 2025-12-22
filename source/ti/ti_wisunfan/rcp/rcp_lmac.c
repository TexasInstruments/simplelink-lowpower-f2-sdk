/*
 * Copyright (c) 2024, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,

 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== rcp_lmac.c ========
 */

#include "rcp_types.h"
#include "rcp_lmac.h"

#ifndef WISUN_RCP_LMAC
#include "rcp_host.h"
#endif // WISUN_RCP_LMAC


#include "mac_data.h"
#include "hmac_map_direct.h"
#include "osal_port.h"
#include "mac_assert.h"

/* RCP global struct */
rcp_lmac_internal_t rcp_lmac_store;

/* Debug structs */
rcp_btie_debug_t rcp_btie_dbg;
MAC_RCP_Tasklet_DBG_s MacRcpTaskletDbg;
rcp_lmac_dbg_t rcp_lmac_dbg;
MT_rcp_LMAC_dbg_t MT_rcp_LMAC_dbg;

/* Extern functions */
extern void rcp_timacSetTrackParent(broadcast_timing_info_t *bc_timing_info, const bool force_synch);
extern void ccfg_read_mac_addr(uint8_t *mac_addr);
extern void timac_setup_Test_GPIO(void);

/* Local prototypes */
static rcp_status_t handle_rcp_mac_config_set(rcp_mac_config_set_t *mac_config);
static void handle_rcp_init();

void rcp_lmac_to_host(uint8_t cmd_type, void *data);
rcp_status_t rcp_lmac_from_host(rcp_cmd_t *rcp_cmd);
rcp_status_t handle_rcp_data_req(rcp_data_req_t *data_req);
void rcp_lmac_data_ind_to_host(macCbackEvent_t *pData);
void rcp_lmac_data_cnf_to_host(macCbackEvent_t *pData);

static void handle_rcp_init()
{
    rcp_init_cnf_t init_cnf;

    ccfg_read_mac_addr(init_cnf.device_ext_addr);
    rcp_lmac_to_host(RCP_INIT_CNF, &init_cnf);
}

static rcp_status_t handle_rcp_mac_config_set(rcp_mac_config_set_t *mac_config)
{
    uint8_t ret;
    rcp_status_t rcp_status = RCP_SUCCESS;
    rcp_mac_config_set_cnf_t mac_config_set_cnf;

    rcp_mac_config_bc_timing_t *rcp_bc_timing;
    rcp_mac_config_sec_key_t *rcp_sec_key;
    rcp_mac_config_sec_frame_count_t *rcp_sec_frame_count;
    rcp_mac_config_phy_init *phy_init_config;

    // MAC PIB values
    if (mac_config->attr < 0x1000) {
        switch (mac_config->attr) {
#ifdef MAC_OVERRIDE_TX_DELAY
            case RCP_CONFIG_CUSTOM_MIN_TX_OFF_ENABLE:
                customMinTxOffEnabled = *((uint8_t *) mac_config->val);
                break;
            case RCP_CONFIG_CUSTOM_MIN_TX_OFF_TIME:
                minTxOffTime = *((uint32_t *) mac_config->val);
                break;
#endif // MAC_OVERRIDE_TX_DELAY
#ifdef MAC_DUTY_CYCLE_CHECKING
            case RCP_CONFIG_DUTY_CYCLE_ENABLE:
                dcEnabled = *((uint8_t *) mac_config->val);
                break;
            case RCP_CONFIG_DUTY_CYCLE_REGULATED:
                dcRegulated = *((uint32_t *) mac_config->val);
                break;
#endif // MAC_DUTY_CYCLE_CHECKING
            case RCP_CONFIG_REG_DOMAIN:
                regDomain = *((uint8_t *) mac_config->val);
                break;
            default:
                ret = MAC_MlmeSetReq(mac_config->attr, (void *) mac_config->val);
                if (ret != MAC_SUCCESS) {
                    rcp_status = RCP_ERR_MLME_SET;
                }
                break;
        }
    }
    // New config values
    else if (mac_config->attr >= 0x1000 && mac_config->attr < 0x2000) {
        switch (mac_config->attr) {
            case RCP_CONFIG_ASYNC_CHANNEL_MASK:
                memcpy(rcp_lmac_store.async_channel_list, mac_config->val, mac_config->val_len);
                break;
            case RCP_CONFIG_BC_TIMING_INFO:
                rcp_bc_timing = (rcp_mac_config_bc_timing_t *) mac_config->val;
                rcp_timacSetTrackParent(&rcp_bc_timing->bc_timing_info, rcp_bc_timing->force_sync);
                break;
            case RCP_CONFIG_SEC_KEY:
                rcp_sec_key = (rcp_mac_config_sec_key_t *) mac_config->val;
                keyDescriptor_t *key_descriptor = &rcp_lmac_store.key_descriptors[rcp_sec_key->key_index];
                memset(key_descriptor, 0, sizeof(*key_descriptor));
                memcpy(key_descriptor->key, rcp_sec_key->key, sizeof(key_descriptor->key));
                break;
            case RCP_CONFIG_SEC_FRAME_COUNT:
                rcp_sec_frame_count = (rcp_mac_config_sec_frame_count_t *) mac_config->val;
                rcp_lmac_store.key_descriptors[rcp_sec_frame_count->key_index].frameCounter = rcp_sec_frame_count->frame_count;
                break;
            case RCP_CONFIG_PHY_INIT:
                phy_init_config = (rcp_mac_config_phy_init *) mac_config->val;
                memset(&rcp_lmac_store.phy_descriptor, 0, sizeof(rcp_lmac_store.phy_descriptor));
                rcp_lmac_store.ffd = phy_init_config->ffd;
                rcp_lmac_store.phy_descriptor.firstChCentrFreq = phy_init_config->ch0_center_frequency;
                rcp_lmac_store.phy_descriptor.channelSpacing = phy_init_config->config_channel_spacing;
                rcp_lmac_store.phy_descriptor.noOfChannels = phy_init_config->config_number_of_channels;
                rcp_lmac_store.phy_descriptor.fskModIndex = phy_init_config->mod_index;
                rcp_lmac_store.phy_descriptor.fskModScheme = phy_init_config->mod_scheme;
                rcp_lmac_store.phy_descriptor.symbolRate = phy_init_config->symbol_rate;
                rcp_lmac_store.phy_descriptor.ccaType = phy_init_config->cca_type;
                rcp_lmac_store.start_flag = true; // Set the start flag to start the MAC task
                break;
            default:
                rcp_lmac_dbg.num_err_wrong_type++;
                MAC_ASSERT(0);
        }
    }
    // FH PIB values
    else if (mac_config->attr >= 0x2000 && mac_config->attr < 0x3000) {
        ret = MAC_MlmeFHSetReq(mac_config->attr, (void *) mac_config->val);
        if (ret != FHAPI_STATUS_SUCCESS) {
            rcp_status = RCP_ERR_MLME_SET;
        }
    } else {
        rcp_lmac_dbg.num_err_wrong_type++;
        MAC_ASSERT(0);
    }

    /* Send RCP set cnf */
    //TBD: consider checking for val_len and allocating memory only if needed
    uint8_t *pAttrVal = MAP_osal_msg_allocate(mac_config->val_len); //free this in MT layer or later

    if(pAttrVal == NULL) {
        rcp_lmac_dbg.num_err_mem_alloc_fail++;
        return RCP_ERR_MLME_SET;
    }
    else
    {
        mac_config_set_cnf.status = rcp_status;
        mac_config_set_cnf.attr = mac_config->attr;
        memcpy(pAttrVal, mac_config->val, mac_config->val_len);
        mac_config_set_cnf.val = pAttrVal;
        mac_config_set_cnf.val_len = mac_config->val_len; 
    }    
    
    rcp_lmac_to_host(RCP_MAC_CONFIG_SET_CNF, &mac_config_set_cnf);

    return rcp_status;
}

void rcp_lmac_to_host(uint8_t cmd_type, void *pData)
{
 
#ifndef WISUN_RCP_LMAC
    rcp_cmd_t rcp_cmd;
    rcp_cmd.rcp_cmd_type = cmd_type;
    rcp_cmd.rcp_data = pData;
    rcp_host_from_lmac(&rcp_cmd);
#else
    macRcpMsg_t *pMsg = (macRcpMsg_t *) MAP_osal_msg_allocate(sizeof(macRcpMsg_t));
    if (pMsg == NULL)
    {
        rcp_lmac_dbg.num_err_mem_alloc_fail++;
        return;
    }
    memset(pMsg, 0, sizeof(macRcpMsg_t));
    pMsg->hdr.event = MAC_RCP_TO_HOST_MSG;
    pMsg->hdr.status = 0;
    pMsg->rcp_cmd_type = cmd_type;

    switch(cmd_type)
    {
        case RCP_DATA_IND:
            pMsg->rcp_data = MAP_osal_msg_allocate(sizeof(rcp_data_ind_t));
            memset (pMsg->rcp_data, 0, sizeof(rcp_data_ind_t));
            memcpy(pMsg->rcp_data, pData, sizeof(rcp_data_ind_t)); 
            break;
        case RCP_DATA_CNF:
            pMsg->rcp_data = MAP_osal_msg_allocate(sizeof(rcp_data_cnf_t));
            memset (pMsg->rcp_data, 0, sizeof(rcp_data_cnf_t));
            memcpy(pMsg->rcp_data, pData, sizeof(rcp_data_cnf_t));
            break ;  
        case RCP_INIT_CNF:
            pMsg->rcp_data = MAP_osal_msg_allocate(sizeof(rcp_init_cnf_t));
            memset (pMsg->rcp_data, 0, sizeof(rcp_init_cnf_t));
            memcpy(pMsg->rcp_data, pData, sizeof(rcp_init_cnf_t));
            break;
       case RCP_MAC_INIT_CNF:
            pMsg->rcp_data = MAP_osal_msg_allocate(sizeof(rcp_mac_config_set_cnf_t));
            memset (pMsg->rcp_data, 0, sizeof(rcp_mac_config_set_cnf_t));   
            memcpy(pMsg->rcp_data, pData, sizeof(rcp_mac_config_set_cnf_t));
            break;
        case RCP_FH_INIT_CNF:
            pMsg->rcp_data = MAP_osal_msg_allocate(sizeof(rcp_mac_config_set_cnf_t));
            memset (pMsg->rcp_data, 0, sizeof(rcp_mac_config_set_cnf_t));
            memcpy(pMsg->rcp_data, pData, sizeof(rcp_mac_config_set_cnf_t));
            break;
        case RCP_MAC_RESET_CNF:
            pMsg->rcp_data = MAP_osal_msg_allocate(sizeof(rcp_mac_config_set_cnf_t));
            memset (pMsg->rcp_data, 0, sizeof(rcp_mac_config_set_cnf_t));
            memcpy(pMsg->rcp_data, pData, sizeof(rcp_mac_config_set_cnf_t));
            break;
        case RCP_MAC_CONFIG_GET_CNF:
            pMsg->rcp_data = MAP_osal_msg_allocate(sizeof(rcp_mac_config_get_cnf_t));
            memset (pMsg->rcp_data, 0, sizeof(rcp_mac_config_get_cnf_t));
            memcpy(pMsg->rcp_data, pData, sizeof(rcp_mac_config_get_cnf_t));
            break;
        case RCP_MAC_CONFIG_SET_CNF:
            pMsg->rcp_data = MAP_osal_msg_allocate(sizeof(rcp_mac_config_set_cnf_t));
            memset (pMsg->rcp_data, 0, sizeof(rcp_mac_config_set_cnf_t));
            memcpy(pMsg->rcp_data, pData, sizeof(rcp_mac_config_set_cnf_t));
            break;
        default:
            break;
    }

    
    //send message to mcp/ app task 
    if(pMsg != NULL)
    {
        OsalPort_msgSend(rcp_lmac_store.app_task_id, (uint8 *)pMsg);
    }

#endif // WISUN_RCP_LMAC

}

rcp_status_t rcp_lmac_from_host(rcp_cmd_t *rcp_cmd)
{
    uint8_t ret = RCP_SUCCESS;
    uint8_t mac_ret;
    rcp_data_cnf_t data_cnf_reject;
    rcp_mac_config_set_cnf_t mac_config_set_cnf;
    ApiMac_mlmeStartReq_t *mac_init_start_req;

    switch(rcp_cmd->rcp_cmd_type) {
        case RCP_DATA_REQ:
            ret = handle_rcp_data_req((rcp_data_req_t *) rcp_cmd->rcp_data);
            if (ret != RCP_SUCCESS) {
                rcp_data_req_t *data_req = (rcp_data_req_t *) rcp_cmd->rcp_data;
                memset(&data_cnf_reject, 0, sizeof(rcp_data_cnf_t));
                switch (data_req->req_type) {
                    case RCP_DATA_REQ_ASYNC:
                        data_cnf_reject.cnf_type = RCP_DATA_CNF_ASYNC;
                        break;
                    case RCP_DATA_REQ_UNICAST:
                        data_cnf_reject.cnf_type = RCP_DATA_CNF_UNICAST;
                        break;
                    case RCP_DATA_REQ_BROADCAST:
                        data_cnf_reject.cnf_type = RCP_DATA_CNF_BROADCAST;
                        break;
                }
                data_cnf_reject.status = MLME_MAC_ALLOCATION_ERROR;
                data_cnf_reject.msdu_handle = data_req->msdu_handle;
                data_cnf_reject.key_index = INVALID_KEY_INDEX;
                rcp_lmac_to_host(RCP_DATA_CNF, (void *) &data_cnf_reject);
            }
            break;
        case RCP_INIT:
            handle_rcp_init(); // Sends RCP init CNF
            break;
        case RCP_MAC_INIT:
            mac_init_start_req = (ApiMac_mlmeStartReq_t *) rcp_cmd->rcp_data;
            mac_ret = MAC_MlmeStartReq(mac_init_start_req);
            if (mac_ret == MAC_SUCCESS) {
                mac_config_set_cnf.status = RCP_SUCCESS;
            } else {
                mac_config_set_cnf.status = RCP_ERR_MLME_SET;
            }
            mac_config_set_cnf.attr = 0; // Unused
            mac_config_set_cnf.val = 0; // Unused
            mac_config_set_cnf.val_len = 0; // Unused
            rcp_lmac_to_host(RCP_MAC_INIT_CNF, &mac_config_set_cnf);
            break;
        case RCP_FH_INIT:
            /* enable GPIO for testing */
            timac_setup_Test_GPIO();
            /* init debugt counts */
            memset(&MacRcpTaskletDbg, 0x0, sizeof(MacRcpTaskletDbg));
            memset(&rcp_btie_dbg, 0x0, sizeof(rcp_btie_dbg));
            rcp_btie_dbg.btieMinDelay = 0xffff;

            mac_ret = MAC_EnableFH();
            if (mac_ret == MAC_SUCCESS) {
                mac_config_set_cnf.status = RCP_SUCCESS;
            } else {
                mac_config_set_cnf.status = RCP_ERR_MLME_SET;
            }
            mac_config_set_cnf.attr = 0; // Unused
            mac_config_set_cnf.val = 0; // Unused
            mac_config_set_cnf.val_len = 0; // Unused
            rcp_lmac_to_host(RCP_FH_INIT_CNF, &mac_config_set_cnf);
            break;
        case RCP_MAC_RESET:
            mac_ret = MAC_MlmeResetReq(true);
            if (mac_ret == MAC_SUCCESS) {
                mac_config_set_cnf.status = RCP_SUCCESS;
            } else {
                mac_config_set_cnf.status = RCP_ERR_MLME_SET;
            }
            mac_config_set_cnf.attr = 0; // Unused
            mac_config_set_cnf.val = 0; // Unused
            mac_config_set_cnf.val_len = 0; // Unused
            rcp_lmac_to_host(RCP_MAC_RESET_CNF, &mac_config_set_cnf);
            break;
        case RCP_MAC_CONFIG_GET:
            //to be implemented later
            break;
        case RCP_MAC_CONFIG_SET:
            handle_rcp_mac_config_set((rcp_mac_config_set_t *) rcp_cmd->rcp_data);
            break;
        default:
            break;
    }
    return ret;
}

rcp_status_t handle_rcp_data_req(rcp_data_req_t *data_req)
{
    macMcpsDataReq_t *pMsg;
    uint8_t mic_len = 0;
    uint16_t mcps_data_req_len = sizeof(macMcpsDataReq_t) + MAC_PHY_PHR_LEN + data_req->data_len;
    // Allocate osal msg buffer
    pMsg = (macMcpsDataReq_t *) MAP_osal_msg_allocate(mcps_data_req_len);
    if (pMsg == NULL) {
        rcp_lmac_dbg.num_err_mem_alloc_fail_data_req++;
        return RCP_ERR_DATA_REQ_ALLOC_FAIL;
    }
    // Zero out data structure before use
    memset(pMsg, 0, mcps_data_req_len);

    switch (data_req->req_type) {
        case RCP_DATA_REQ_ASYNC:
            rcp_lmac_dbg.num_tx_async++;
            pMsg->hdr.event = MAC_API_WS_ASYNC_REQ_EVT;
            pMsg->internal.frameType = MAC_INTERNAL_ASYNC;
            pMsg->internal.txOptions = 0;
            pMsg->mac.txOptions = 0;
            // only async request packet will use channel list
            MAP_osal_memcpy(pMsg->internal.dest.chList, rcp_lmac_store.async_channel_list, sizeof(pMsg->internal.dest.chList));
            break;
        case RCP_DATA_REQ_UNICAST:
            rcp_lmac_dbg.num_tx_unicast++;
            pMsg->hdr.event = MAC_API_DATA_REQ_EVT;
            pMsg->internal.frameType = MAC_INTERNAL_DATA_UNICAST;
            pMsg->internal.txOptions |= MAC_TXOPTION_ACK;
            pMsg->mac.txOptions |= MAC_TXOPTION_ACK;
            pMsg->internal.be = (pMsg->internal.txOptions & MAC_TXOPTION_ALT_BE) ? pMacPib->altBe : pMacPib->minBe;
            /* only unicast packet has NT entry */
            // set the address mode and extended address
            pMsg->internal.dest.dstAddr.addrMode = SADDR_MODE_EXT;
            MAP_osal_memcpy(pMsg->internal.dest.destAddr,data_req->fhnt_entry.extAddr, SADDR_EXT_LEN);
            MAP_osal_memcpy(&pMsg->internal.fhnt_entry, &data_req->fhnt_entry, sizeof(fhnt_entry_t));
            break;
        case RCP_DATA_REQ_BROADCAST:
            rcp_lmac_dbg.num_tx_broadcast++;
            pMsg->hdr.event = MAC_API_DATA_REQ_EVT;
            pMsg->internal.frameType = MAC_INTERNAL_DATA_BROADCAST;
            pMsg->internal.txOptions = 0;
            pMsg->mac.txOptions = 0;
            pMsg->internal.be = (pMsg->internal.txOptions & MAC_TXOPTION_ALT_BE) ? pMacPib->altBe : pMacPib->minBe;
            // set the address mode, default it is zero.
            pMsg->internal.dest.dstAddr.addrMode = SADDR_MODE_NONE;
            break;
        default:
            rcp_lmac_dbg.num_err_wrong_type++;
            MAC_ASSERT(0);
            break;
    }
    pMsg->hdr.status = MAC_SUCCESS;
    pMsg->mac.msduHandle = data_req->msdu_handle;

    pMsg->msdu.p = ((uint8_t *) pMsg) + sizeof(macMcpsDataReq_t) + MAC_PHY_PHR_LEN;
    pMsg->msdu.len = data_req->data_len; // MIC length already included in data len
    if (data_req->data_len)
    {
        MAP_osal_memcpy(pMsg->msdu.p, data_req->data_ptr, data_req->data_len);
    }
    // set FH frame type for UT/BT IE update
    pMsg->internal.fhFrameType = MAC_FH_FRAME_DATA;
    if (data_req->utie_offset)
    {
        pMsg->internal.ptrUTIE = pMsg->msdu.p + data_req->utie_offset;
    }
    if (data_req->btie_offset)
    {
        pMsg->internal.ptrBTIE = pMsg->msdu.p + data_req->btie_offset;
    }

    pMsg->internal.frameCntr = data_req->min_tx_frame_count;
    // Point to the last byte of frame count in the aux header
    pMsg->internal.ptrFrameCnt = pMsg->msdu.p + data_req->frame_count_offset + MAC_FRAME_COUNTER_LEN - 1;
    pMsg->internal.ptrMData = pMsg->msdu.p + data_req->mdata_offset;
    pMsg->internal.pKeyDesc = &rcp_lmac_store.key_descriptors[data_req->sec.KeyIndex - 1]; // LMAC key index is 0 indexed, NS data req is 1 indexed
    pMsg->internal.txSched |= MAC_TX_SCHED_READY;
    pMsg->internal.txMode = 0;
    pMsg->internal.nb = 0;
    pMsg->internal.retries = pMacPib->maxFrameRetries;
    
    //MAP_osal_memcpy(&pMsg->internal.fhnt_entry, &data_req->fhnt_entry, sizeof(fhnt_entry_t));
    MAP_osal_memcpy(&(pMsg->sec), &(data_req->sec), sizeof(ApiMac_sec_t)); // TODO: Unify ApiMac_sec_t with mlme_security_t

    MAP_osal_msg_send(macTaskId, (uint8 *) pMsg);
    return RCP_SUCCESS;
}

void rcp_lmac_data_ind_to_host(macCbackEvent_t *pData)
{
    rcp_data_ind_t data_ind;
    memset(&data_ind, 0, sizeof(rcp_data_ind_t));

    switch(pData->hdr.event) {
        case MAC_MCPS_DATA_IND:
            data_ind.rx_timestamp = pData->dataInd.mac.timestamp;
            data_ind.link_quality = pData->dataInd.mac.mpduLinkQuality;
            data_ind.signal_dbm = pData->dataInd.mac.rssi;
            data_ind.data_len = pData->dataInd.msdu.len;
            data_ind.data_ptr = pData->dataInd.msdu.p;

            data_ind.buf_free = (uint8_t *) pData;
            rcp_lmac_to_host(RCP_DATA_IND, (void *) &data_ind);
            break;
        default:
            rcp_lmac_dbg.num_err_wrong_type++;
            MAC_ASSERT(0);
            break;
    }
}

void rcp_lmac_data_cnf_to_host(macCbackEvent_t *pData)
{
    rcp_data_cnf_t data_cnf;
    macRx_t *pRxBuf = NULL;
    
    memset(&data_cnf, 0, sizeof(rcp_data_cnf_t));

    switch(pData->hdr.event) {
        case MAC_MLME_WS_ASYNC_CNF:
            data_cnf.cnf_type = RCP_DATA_CNF_ASYNC;
            data_cnf.status = pData->hdr.status;
            data_cnf.msdu_handle = pData->ackCnf.msduHandle;
            data_cnf.tx_frame_count = pData->ackCnf.frameCntr;
            if (pData->ackCnf.pDataReq->sec.securityLevel == 0) {
                data_cnf.key_index = INVALID_KEY_INDEX;
            } else {
                data_cnf.key_index = pData->ackCnf.pDataReq->sec.keyIndex;
            }
            break;
        case MAC_MCPS_DATA_CNF:
            switch(pData->ackCnf.pDataReq->internal.frameType) {
                case MAC_INTERNAL_DATA_UNICAST:
                    data_cnf.cnf_type = RCP_DATA_CNF_UNICAST;
                    if (pData->ackCnf.pPayloadIE != NULL) {
                        pRxBuf = (macRx_t *) (pData->ackCnf.pPayloadIE);
                        data_cnf.ack_frame_len = pRxBuf->msdu.len;
                        data_cnf.ack_frame = pRxBuf->msdu.p;
                        data_cnf.rx_ack_timestamp = pRxBuf->mac.timestamp;
                    } else {
                        data_cnf.ack_frame_len = 0;
                        data_cnf.ack_frame = NULL;
                        data_cnf.rx_ack_timestamp = 0;
                    }
                    break;
                case MAC_INTERNAL_DATA_BROADCAST:
                    data_cnf.cnf_type = RCP_DATA_CNF_BROADCAST;
                    data_cnf.ack_frame_len = 0;
                    data_cnf.ack_frame = NULL;
                    break;
                default:
                    rcp_lmac_dbg.num_err_wrong_type++;
                    MAC_ASSERT(0);
                    break;
            }

            data_cnf.status = pData->hdr.status;
            data_cnf.msdu_handle = pData->ackCnf.msduHandle;
            data_cnf.tx_timestamp = pData->ackCnf.timestamp;
            data_cnf.cca_retries = 0; // Placeholder
            data_cnf.tx_retries = 0; // Placeholder
            data_cnf.tx_frame_count = pData->ackCnf.frameCntr;
            if (pData->ackCnf.pDataReq->sec.securityLevel == 0) {
                data_cnf.key_index = INVALID_KEY_INDEX;
            } else {
                data_cnf.key_index = pData->ackCnf.pDataReq->sec.keyIndex;
            }
            break;
        default:
            rcp_lmac_dbg.num_err_wrong_type++;
            MAC_ASSERT(0);
            break;
    }
    if (pRxBuf != NULL) {
        data_cnf.buf_free = (uint8_t *) pRxBuf;
    }
    rcp_lmac_to_host(RCP_DATA_CNF, (void *) &data_cnf);
    MAP_mac_msg_deallocate((uint8 **)&pData->ackCnf.pDataReq);
}

