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
 *  ======== rcp_host.c ========
 */

#include "rcp_host.h"
#include "ns_trace.h"
#include "mac_assert.h"

#ifndef LINUX_NANOSTACK
#include "rcp_lmac.h"
#else
#include "string.h"
#endif

#include "rcp_queue.h"
#include "mac_common_defines.h"

#ifdef WISUN_RCP_HOST
#include "mt_msg.h"
#endif //WISUN_RCP_HOST

#define TRACE_GROUP "RCPH"

#ifdef WISUN_RCP_HOST

#define RCP_HOST_SYNC_REQ_MT_CMD0 (0x28) //SUBSYS+CMD0
#define RCP_HOST_ASYNC_REQ_MT_CMD0 (0x48) //SUBSYS+CMD0
#define RCP_HOST_SYNC_RSPS_MT_CMD0 (0x68) //SUBSYS+CMD0

//Different MT commands for RCP Subsystem: //CMD1
//Host to LMAC
#define RCP_HOST_DATA_REQ_MT_CMD1 (0x01)
#define RCP_HOST_MAC_CFG_SET_MT_CMD1 (0x02)
#define RCP_HOST_MAC_CFG_GET_MT_CMD1 (0x03)
#define RCP_HOST_MAC_RESET_MT_CMD1 (0x04)
#define RCP_HOST_FH_INIT_MT_CMD1 (0x05)
#define RCP_HOST_MAC_INIT_MT_CMD1 (0x06)
#define RCP_HOST_RCP_INIT_MT_CMD1 (0x07)

//LMAC to Host
#define RCP_LMAC_DATA_IND_MT_CMD1 (0x81)
#define RCP_LMAC_DATA_CNF_MT_CMD1 (0x82)
#define RCP_LMAC_RCP_INIT_CNF_MT_CMD1 (0x83)
#define RCP_LMAC_MAC_INIT_CNF_MT_CMD1 (0x84)
#define RCP_LMAC_FH_INIT_CNF_MT_CMD1 (0x85)
#define RCP_LMAC_MAC_RESET_CNF_MT_CMD1 (0x86)
#define RCP_LMAC_MAC_CFG_GET_CNF_MT_CMD1 (0x87)
#define RCP_LMAC_MAC_CFG_SET_CNF_MT_CMD1 (0x88)


#define RCP_HOST_DATA_REQ_FIXED_SIZE_BYTES  (sizeof(rcp_data_req_t) - sizeof(uint8_t *))
#define RCP_HOST_MAC_CFG_SET_FIXED_SIZE_BYTES (sizeof(rcp_mac_config_set_t) - sizeof(uint8_t *))
#define RCP_HOST_MAC_CFG_GET_FIXED_SIZE_BYTES (sizeof(rcp_mac_config_get_t))
#define RCP_HOST_RCP_MAC_INIT_FIXED_SIZE_BYTES (sizeof(mlme_start_t) - sizeof(mpmParams_t)) //we dont want to encode mpmParams_t

#define RCP_HOST_RCP_FH_INIT_FIXED_SIZE_BYTES (0x0)
#define RCP_HOST_RCP_INIT_FIXED_SIZE_BYTES  (0x0)
#define RCP_HOST_MAC_RESET_FIXED_SIZE_BYTES (0x0)

/*! Key source maximum length in bytes */
#define APIMAC_KEY_SOURCE_MAX_LEN 8

#endif //WISUN_RCP_HOST

/* Stack and protocol versions */
const char *wisun_stack_version __attribute__((retain)) = "1.1.0";
const char *wisun_protocol_version __attribute__((retain)) = "1.0+";

RCP_HOST_DBG_s rcp_host_dbg;
#ifdef WISUN_RCP_HOST
MT_rcp_Host_dbg_t MT_rcp_Host_dbg;
#endif

rcp_host_internal_t rcp_host_store;
extern struct mt_msg_interface *API_MAC_msg_interface;

int8_t rcp_data_sap_rx_handler(mac_pre_parsed_frame_t *buf, protocol_interface_rf_mac_setup_s *rf_mac_setup, mac_api_t *mac);


void rcp_host_init(mac_api_t* pMac_api, struct protocol_interface_rf_mac_setup *pMac_setup)
{
    rcp_host_store.mac_api = pMac_api;
    rcp_host_store.setup = pMac_setup;

    memset(&rcp_host_dbg, 0x00,sizeof(RCP_HOST_DBG_s));

    rcp_tx_data_queue_init();
}

#ifdef WISUN_RCP_HOST

/*!
 * @brief Allocate a new message
 * @param len - expected payload length, or -1 if unknown
 * @param cmd0 - cmd0 value
 * @param cmd1 - cmd1 value
 * @return pointer to an allocated message, or NULL if no memory
 */
static struct mt_msg *api_new_msg(int len, int cmd0,
    int cmd1,
    const char *dbg_prefix)
{
    struct mt_msg *pMsg;

    pMsg = MT_MSG_alloc(len, cmd0, cmd1);
    if(pMsg)
    {
        pMsg->pLogPrefix = dbg_prefix;
        MT_MSG_setSrcIface(pMsg, NULL);
        MT_MSG_setDestIface(pMsg, API_MAC_msg_interface);
    }
    return (pMsg);
}

static void encode_Sec(struct mt_msg *pMsg, mlme_security_t *pSec)
{
    MT_MSG_wrBuf_DBG(pMsg,
        (void *)(&(pSec->Keysource[0])),
        APIMAC_KEY_SOURCE_MAX_LEN, "keySource");
    MT_MSG_wrU8_DBG(pMsg, pSec->SecurityLevel, "securityLevel");
    MT_MSG_wrU8_DBG(pMsg, pSec->KeyIdMode, "keyIdMode");
    MT_MSG_wrU8_DBG(pMsg, pSec->KeyIndex, "keyIndex");

}

static void encode_FHNT_entry(struct mt_msg *pMsg, fhnt_entry_t *pFHNT_entry)
{
    MT_MSG_wrU32_DBG(pMsg, pFHNT_entry->ufsi, "ufsi"); //4 bytes
    MT_MSG_wrU32_DBG(pMsg, pFHNT_entry->ref_timeStamp, "ref_timeStamp"); //4 bytes
    MT_MSG_wrU8_DBG(pMsg, pFHNT_entry->dwellInterval, "dwellInterval"); //1 byte
    MT_MSG_wrU8_DBG(pMsg, pFHNT_entry->channelFunc, "channelFunc"); //1 byte
    MT_MSG_wrU8_DBG(pMsg, pFHNT_entry->fixedChannel, "fixedChannel"); //1 byte
    MT_MSG_wrBuf_DBG(pMsg, pFHNT_entry->bitMap, MAC_154G_CHANNEL_BITMAP_SIZ, "bitMap"); //17 bytes
    MT_MSG_wrU8_DBG(pMsg, pFHNT_entry->numChannels, "numChannels"); //1 byte
    MT_MSG_wrBuf_DBG(pMsg, pFHNT_entry->extAddr, sizeof(sAddrExt_t), "extAddr"); //8 bytes
}

static void prep_mtmsg_dataReq(struct mt_msg *pMsg, rcp_data_req_t * pDataReq)
{

    MT_MSG_wrU8_DBG(pMsg, pDataReq->req_type, "RequestType"); // 1Byte
    MT_MSG_wrU8_DBG(pMsg, pDataReq->msdu_handle, "MsduHandle"); // 1Byte
    MT_MSG_wrU32_DBG(pMsg, pDataReq->min_tx_frame_count, "MinTxFrameCount"); //4 bytes
    MT_MSG_wrU8_DBG(pMsg, pDataReq->frame_count_offset, "FrameCountOffset"); //1 byte
    encode_Sec(pMsg, &(pDataReq->sec)); //11 bytes
    MT_MSG_wrU16_DBG(pMsg, pDataReq->mdata_offset, "MdataOffset"); //2 Bytes
    MT_MSG_wrU16_DBG(pMsg, pDataReq->utie_offset, "UtieOffset"); //2 Bytes
    MT_MSG_wrU16_DBG(pMsg, pDataReq->btie_offset, "BtieOffset"); //2 Bytes
    encode_FHNT_entry(pMsg, &(pDataReq->fhnt_entry)); //37 bytes
    MT_MSG_wrU16_DBG(pMsg, pDataReq->data_len, "DataLen"); //2 Bytes
    MT_MSG_wrBuf_DBG(pMsg, pDataReq->data_ptr, pDataReq->data_len, "Data");

}

static void prep_mtmsg_cfgGet(struct mt_msg *pMsg, rcp_mac_config_get_t * pCfgGetReq)
{
    MT_MSG_wrU8_DBG(pMsg, pCfgGetReq->attr, "Attr"); // 1Byte
}

static void prep_mtmsg_cfgSet(struct mt_msg *pMsg, rcp_mac_config_set_t * pCfgSetReq)
{
    MT_MSG_wrU16_DBG(pMsg, pCfgSetReq->attr, "Attr"); // 2Bytes
    MT_MSG_wrU16_DBG(pMsg, pCfgSetReq->val_len, "ValLen"); // 2Bytes
    MT_MSG_wrBuf_DBG(pMsg, pCfgSetReq->val, pCfgSetReq->val_len, "Val"); // val_len bytes
}


uint32_t StartTime;                 /**< Start time,  set 0 */
    uint16_t PANId;                     /**< Pan-id */
    uint8_t LogicalChannel;             /**< Operated Logical channel */
    uint8_t ChannelPage;                /**< Operated Logical channel page */
    uint8_t PhyID;
    uint8_t BeaconOrder;                /**< Beacon order,  set 15 */
    uint8_t SuperframeOrder;            /**< Super frame order,  set 15 */
    bool PANCoordinator;                /**< true= Enable beacon response for beacon request, false = disable beacon request responses */
    bool BatteryLifeExtension;          /**< Set false */
    bool CoordRealignment;              /**< Set false */
    mlme_security_t CoordRealignKey;    /**< Coordinator Realignment security parameter's  (Valid only CoordRealignment = true)*/
    mlme_security_t BeaconRealignKey;   /**< Beacon realign security parameter's (Valid only CoordRealignment = true)*/
    mpmParams_t mpmParams;
    bool startFH;


 uint16_t NBPANEBeaconOrder;
    /*!
     pointer to the buffer containing the Information element IDs which needs
     to be sent in Enhanced Beacon.  This field is reserved for future use
     and should be set to NULL.
     */
    uint8_t *pIEIDs;
    /*!
     The number of Information Elements in the buffer (size of buffer
     at pIEIDs.  This field is reserved for future use and should be set to 0.
     */
    uint8_t numIEs;

static void prep_mtmsg_macInit(struct mt_msg *pMsg, mlme_start_t *pMacInit)
{
    MT_MSG_wrU32_DBG(pMsg, pMacInit->StartTime, "StartTime"); // 4Bytes
    MT_MSG_wrU16_DBG(pMsg, pMacInit->PANId, "PANId"); // 2Bytes
    MT_MSG_wrU8_DBG(pMsg, pMacInit->LogicalChannel, "LogicalChannel"); // 1Byte
    MT_MSG_wrU8_DBG(pMsg, pMacInit->ChannelPage, "ChannelPage"); // 1Byte
    MT_MSG_wrU8_DBG(pMsg, pMacInit->PhyID, "PhyID"); // 1Byte
    MT_MSG_wrU8_DBG(pMsg, pMacInit->BeaconOrder, "BeaconOrder"); // 1Byte
    MT_MSG_wrU8_DBG(pMsg, pMacInit->SuperframeOrder, "SuperframeOrder"); // 1Byte
    MT_MSG_wrU8_DBG(pMsg, pMacInit->PANCoordinator, "PANCoordinator"); // 1Byte
    MT_MSG_wrU8_DBG(pMsg, pMacInit->BatteryLifeExtension, "BatteryLifeExtension"); // 1Byte
    MT_MSG_wrU8_DBG(pMsg, pMacInit->CoordRealignment, "CoordRealignment"); // 1Byte
    encode_Sec(pMsg, &(pMacInit->CoordRealignKey)); //11 bytes
    encode_Sec(pMsg, &(pMacInit->BeaconRealignKey)); //11 bytes
    //we dont want  to encode mpmParams_t. let the rcp mac set these to zeroes when reconstructing mlme_start_t
    MT_MSG_wrU8_DBG(pMsg, pMacInit->startFH, "startFH"); // 1Byte
}


static struct mt_msg *rcp_host_prep_mtmsg(rcp_cmd_t *rcp_cmd)
{
    struct mt_msg *pMsg;
    int n; //size to be allocated


    switch(rcp_cmd->rcp_cmd_type)
    {
        case RCP_DATA_REQ:
            rcp_data_req_t *pDataReq = (rcp_data_req_t *)rcp_cmd->rcp_data;
            n = RCP_HOST_DATA_REQ_FIXED_SIZE_BYTES + pDataReq->data_len ;

            //allocate buffer for formulating mt msg
            pMsg = api_new_msg(n, RCP_HOST_SYNC_REQ_MT_CMD0, RCP_HOST_DATA_REQ_MT_CMD1, "RCPDataReq");
            if(!pMsg)
            {
                return (NULL);
            }
            //helper function to formulate the mt message
            prep_mtmsg_dataReq(pMsg, pDataReq);
            break;

        case RCP_MAC_CONFIG_GET:
            rcp_mac_config_get_t *pCfgGetReq = (rcp_mac_config_get_t *)rcp_cmd->rcp_data;
            n = RCP_HOST_MAC_CFG_GET_FIXED_SIZE_BYTES;
            pMsg = api_new_msg(n, RCP_HOST_SYNC_REQ_MT_CMD0, RCP_HOST_MAC_CFG_GET_MT_CMD1, "RCPMacCfgGet");
            if(!pMsg)
            {
                return (NULL);
            }
            //helper function to formulate the mt message
            prep_mtmsg_cfgGet(pMsg, pCfgGetReq);

            break;

        case RCP_MAC_CONFIG_SET:
            rcp_mac_config_set_t *pCfgSetReq = (rcp_mac_config_set_t *)rcp_cmd->rcp_data;
            n = RCP_HOST_MAC_CFG_SET_FIXED_SIZE_BYTES + pCfgSetReq->val_len;
            pMsg = api_new_msg(n, RCP_HOST_SYNC_REQ_MT_CMD0, RCP_HOST_MAC_CFG_SET_MT_CMD1, "RCPMacCfgSet");
            if(!pMsg)
            {
                return (NULL);
            }
            //helper function to formulate the mt message
            prep_mtmsg_cfgSet(pMsg, pCfgSetReq);
            break;

        case RCP_MAC_RESET:
            n = RCP_HOST_MAC_RESET_FIXED_SIZE_BYTES;
            pMsg = api_new_msg(n, RCP_HOST_SYNC_REQ_MT_CMD0, RCP_HOST_MAC_RESET_MT_CMD1, "RCPMacReset");
            if(!pMsg)
            {
                return (NULL);
            }
            break;

        case RCP_FH_INIT:
            n = RCP_HOST_RCP_FH_INIT_FIXED_SIZE_BYTES;
            pMsg = api_new_msg(n, RCP_HOST_SYNC_REQ_MT_CMD0, RCP_HOST_FH_INIT_MT_CMD1, "RCPFHinit");
            if(!pMsg)
            {
                return (NULL);
            }
            break;

        case RCP_MAC_INIT:
            mlme_start_t *pMacInit = (mlme_start_t *)rcp_cmd->rcp_data;
            n = RCP_HOST_RCP_MAC_INIT_FIXED_SIZE_BYTES;
            pMsg = api_new_msg(n, RCP_HOST_SYNC_REQ_MT_CMD0, RCP_HOST_MAC_INIT_MT_CMD1, "RCPMacInit");
            if(!pMsg)
            {
                return (NULL);
            }
            prep_mtmsg_macInit(pMsg, pMacInit);
            break;

        case RCP_INIT:
            n = RCP_HOST_RCP_INIT_FIXED_SIZE_BYTES;
            pMsg = api_new_msg(n, RCP_HOST_SYNC_REQ_MT_CMD0, RCP_HOST_RCP_INIT_MT_CMD1, "RCPinit");
            if(!pMsg)
            {
                return (NULL);
            }
            break;

        default:
            return (0);
    }

    return(pMsg);

}

void process_areq_rcp_data_ind(struct mt_msg *pMsg)
{
    rcp_cmd_t rcp_cmd;
    rcp_data_ind_t data_ind;

    //parse the message
    data_ind.rx_timestamp = MT_MSG_rdU32_DBG(pMsg, "rx_timestamp");
    data_ind.link_quality = MT_MSG_rdU8_DBG(pMsg, "link_quality");
    data_ind.signal_dbm = MT_MSG_rdU8_DBG(pMsg, "signal_dbm");
    data_ind.data_len = MT_MSG_rdU16_DBG(pMsg, "data_len");
    data_ind.data_ptr = &(pMsg->iobuf[ pMsg->iobuf_idx ]);
    pMsg->iobuf_idx = pMsg->iobuf_idx + data_ind.data_len;
    data_ind.buf_free = NULL;
    MT_MSG_parseComplete(pMsg);

    //use the incoming data_ind
    rcp_cmd.rcp_cmd_type = RCP_DATA_IND;
    rcp_cmd.rcp_data = (uint8_t *) &data_ind;
    rcp_host_from_lmac(&rcp_cmd);
}

void process_areq_rcp_data_cnf(struct mt_msg *pMsg)
{
    rcp_cmd_t rcp_cmd;
    rcp_data_cnf_t data_cnf;

    //parse the message
    data_cnf.cnf_type = MT_MSG_rdU8_DBG(pMsg, "cnf_type");
    data_cnf.msdu_handle = MT_MSG_rdU8_DBG(pMsg, "msdu_handle");
    data_cnf.status = MT_MSG_rdU8_DBG(pMsg, "status");
    data_cnf.tx_timestamp = MT_MSG_rdU32_DBG(pMsg, "tx_timestamp");
    data_cnf.rx_ack_timestamp = MT_MSG_rdU32_DBG(pMsg, "rx_ack_timestamp");
    data_cnf.cca_retries = MT_MSG_rdU8_DBG(pMsg, "cca_retries");
    data_cnf.tx_retries = MT_MSG_rdU8_DBG(pMsg, "tx_retries");
    data_cnf.tx_frame_count = MT_MSG_rdU32_DBG(pMsg, "tx_frame_count");
    data_cnf.key_index = MT_MSG_rdU8_DBG(pMsg, "key_index");
    data_cnf.ack_frame_len = MT_MSG_rdU16_DBG(pMsg, "ack_frame_len");
    data_cnf.ack_frame = &(pMsg->iobuf[ pMsg->iobuf_idx ]);
    pMsg->iobuf_idx = pMsg->iobuf_idx + data_cnf.ack_frame_len;
    data_cnf.buf_free = NULL;
    MT_MSG_parseComplete(pMsg);

    //use the incoming data_cnf
    rcp_cmd.rcp_cmd_type = RCP_DATA_CNF;
    rcp_cmd.rcp_data = (uint8_t *) &data_cnf;
    rcp_host_from_lmac(&rcp_cmd);
}


void process_areq_rcp_init_cnf(struct mt_msg *pMsg)
{
    rcp_cmd_t rcp_cmd;
    rcp_init_cnf_t cnfRcpInit;

    //parse the message
    memcpy(&cnfRcpInit.device_ext_addr[0], &pMsg->iobuf[pMsg->iobuf_idx], sizeof(rcp_init_cnf_t)) ;
    pMsg->iobuf_idx = pMsg->iobuf_idx + sizeof(rcp_init_cnf_t);
    MT_MSG_parseComplete(pMsg);

    //use the incoming data_ind
    rcp_cmd.rcp_cmd_type = RCP_INIT_CNF;
    rcp_cmd.rcp_data = (uint8_t *) &cnfRcpInit;
    rcp_host_from_lmac(&rcp_cmd);
}

void process_areq_rcp_mac_cfg_set_cnf(struct mt_msg *pMsg, uint8_t rcp_cnf_type)
{
    rcp_cmd_t rcp_cmd;
    rcp_mac_config_set_cnf_t cnfMacCfgSet;

    //parse the message
    cnfMacCfgSet.attr = MT_MSG_rdU16_DBG(pMsg, "attr");
    cnfMacCfgSet.status = MT_MSG_rdU8_DBG(pMsg, "status");
    cnfMacCfgSet.val_len = MT_MSG_rdU16_DBG(pMsg, "val_len");
    cnfMacCfgSet.val = &(pMsg->iobuf[ pMsg->iobuf_idx ]);
    pMsg->iobuf_idx = pMsg->iobuf_idx + cnfMacCfgSet.val_len;
    MT_MSG_parseComplete(pMsg);

    //use the incoming data_ind
    rcp_cmd.rcp_cmd_type = rcp_cnf_type;
    rcp_cmd.rcp_data = (uint8_t *) &cnfMacCfgSet;
    rcp_host_from_lmac(&rcp_cmd);
}

static void process_areq(struct mt_msg *pMsg)
{
    if(RCP_HOST_ASYNC_REQ_MT_CMD0 == (pMsg->cmd0 & 0x7F))
    {
        switch(pMsg->cmd1)
        {
            case RCP_LMAC_DATA_IND_MT_CMD1:
                process_areq_rcp_data_ind(pMsg);
                MT_rcp_Host_dbg.num_data_ind_rxed++;
                break;
            case RCP_LMAC_DATA_CNF_MT_CMD1:
                process_areq_rcp_data_cnf(pMsg);
                MT_rcp_Host_dbg.num_data_cnf_rxed++;
                break;
            case RCP_LMAC_RCP_INIT_CNF_MT_CMD1:
                process_areq_rcp_init_cnf(pMsg);
                MT_rcp_Host_dbg.num_rcp_init_cnf_rxed++;
                break;
            case RCP_LMAC_MAC_INIT_CNF_MT_CMD1:
                process_areq_rcp_mac_cfg_set_cnf(pMsg, RCP_MAC_INIT_CNF);
                MT_rcp_Host_dbg.num_mac_init_cnf_rxed++;
                break;
            case RCP_LMAC_FH_INIT_CNF_MT_CMD1:
                process_areq_rcp_mac_cfg_set_cnf(pMsg, RCP_FH_INIT_CNF);
                MT_rcp_Host_dbg.num_fh_init_cnf_rxed++;
                break;
            case RCP_LMAC_MAC_RESET_CNF_MT_CMD1:
                process_areq_rcp_mac_cfg_set_cnf(pMsg, RCP_MAC_RESET_CNF);
                MT_rcp_Host_dbg.num_mac_reset_cnf_rxed++;
                break;
            case RCP_LMAC_MAC_CFG_GET_CNF_MT_CMD1:
                //insert code
                break;
            case RCP_LMAC_MAC_CFG_SET_CNF_MT_CMD1:
                process_areq_rcp_mac_cfg_set_cnf(pMsg, RCP_MAC_CONFIG_SET_CNF);
                MT_rcp_Host_dbg.num_mac_cfg_set_cnf_rxed++;
                break;
            default:
                //not an expected message...add a handler for this
        }


    }
    else
    {
        //not sure why we came here...this is specifically for RCP subsystem
    }

}

void processIncomingAresp(void)
{
    struct mt_msg *pMsg;

    pMsg = MT_MSG_LIST_remove(API_MAC_msg_interface,
                              &(API_MAC_msg_interface->rx_list),
                              0);

    if(pMsg == NULL)
    {
        tr_err("no msg from MT Interface\n");
        return;
    }

    /* process the message */
    if(pMsg->m_type == MT_MSG_TYPE_areq|| pMsg->m_type == MT_MSG_TYPE_areq_frag_data)
    {
        process_areq(pMsg);
    }
    else
    {
        //not an expected message...add a handler for this
    }
    MT_MSG_free(pMsg);
}

#endif // WISUN_RCP_HOST

static void rcp_host_to_lmac(uint8_t cmd_type,
                           void *data)
{
    rcp_cmd_t rcp_cmd;

    rcp_cmd.rcp_cmd_type = cmd_type;
    rcp_cmd.rcp_data = data;

    rcp_host_dbg.num_tx++;
#ifndef WISUN_RCP_HOST
    rcp_lmac_from_host(&rcp_cmd);
#else

        struct mt_msg *pMsg;
        int r;

        pMsg = rcp_host_prep_mtmsg(&rcp_cmd);
        r = MT_MSG_txrx(pMsg);

        //free the message
        MT_MSG_free(pMsg);
        
        if(r == 1) //message successfully sent to the LMAC
        {
            switch(cmd_type)
            {
                case RCP_DATA_REQ:
                    MT_rcp_Host_dbg.num_data_req_sent++;
                    break;
                case RCP_MAC_CONFIG_GET:
                    MT_rcp_Host_dbg.num_mac_cfg_get_req_sent++;
                    break;
                case RCP_MAC_CONFIG_SET:
                    MT_rcp_Host_dbg.num_mac_cfg_set_req_sent++;
                    break;
                case RCP_MAC_RESET:
                    MT_rcp_Host_dbg.num_mac_reset_req_sent++;
                    break;
                case RCP_FH_INIT:
                    MT_rcp_Host_dbg.num_fh_init_req_sent++;
                    break;
                case RCP_MAC_INIT:
                    MT_rcp_Host_dbg.num_mac_init_req_sent++;
                    break;
                case RCP_INIT:
                    MT_rcp_Host_dbg.num_rcp_init_req_sent++;
                    break;
                default:
                    tr_err("Unknown RCP command type %d", cmd_type);
                    break;
            }
        }
        else
        {
            MT_rcp_Host_dbg.num_err_mt_msg_send_failure;
        }
        
#endif
}

void handle_rcp_data_cnf(rcp_data_cnf_t * data_cnf)
{
    //MVTODO: for now keeping the cnf path simple and bypassing queues in NS
    // rf_mac_setup->active_pd_data_request does not not significance
    // the cnf can be for any previously sent data packet from the lmac
    // later try to use the NS bc and uc queues

    mcps_data_conf_t mac_data_conf;
    mcps_data_conf_payload_t mac_data_conf_ie;
    memset(&mac_data_conf, 0, sizeof(mcps_data_conf_t));
    memset(&mac_data_conf_ie, 0, sizeof(mcps_data_conf_payload_t));

    // for ack processing
    mac_pre_parsed_frame_t *buffer = NULL;
    mac_fcf_sequence_t fcf_read;

    //populate data_conf
    mac_data_conf.msduHandle = data_cnf->msdu_handle;
    mac_data_conf.status = data_cnf->status;
    mac_data_conf.timestamp = data_cnf->rx_ack_timestamp;
    mac_data_conf.cca_retries = data_cnf->cca_retries;
    mac_data_conf.tx_retries = data_cnf->tx_retries;

    if (data_cnf->status == MLME_SUCCESS) {
        rcp_host_dbg.num_tx_cnf++;
    } else if (data_cnf->status == MLME_TRANSACTION_OVERFLOW) {
        rcp_host_dbg.num_tx_cnf_reject++;
    } else if (data_cnf->status == MLME_MAC_ALLOCATION_ERROR) {
        rcp_host_dbg.num_tx_cnf_alloc_err++;
    } else {
        rcp_host_dbg.num_tx_cnf_failure++;
    }

    //store the tx frame counter per key
    if(data_cnf->key_index != INVALID_KEY_INDEX)
    {
        if (rcp_host_store.setup->secFrameCounterPerKey) {
            mlme_key_descriptor_t *key_desc = mac_sec_key_description_get_by_attribute(rcp_host_store.setup, data_cnf->key_index - 1);
            if (!key_desc) {
                rcp_host_dbg.num_err_data_cnf_no_key++;
                mac_data_conf.status = MLME_UNAVAILABLE_KEY;
                rcp_host_store.mac_api->data_conf_ext_cb(rcp_host_store.mac_api, &mac_data_conf, &mac_data_conf_ie);
                goto ERROR_HANDLER;
            }
            mac_sec_mib_key_outgoing_frame_counter_set(rcp_host_store.setup, key_desc, data_cnf->tx_frame_count);
        } else {
            mac_sec_mib_key_outgoing_frame_counter_set(rcp_host_store.setup, NULL, data_cnf->tx_frame_count);
        }
    }

    if(data_cnf->cnf_type == RCP_DATA_CNF_ASYNC)
    {
        if (data_cnf->status == MLME_SUCCESS) {
            rcp_host_dbg.num_tx_cnf_async++;
            rcp_host_store.async_in_progress = false;
        } else if (data_cnf->status == MLME_MAC_ALLOCATION_ERROR) {
            rcp_host_dbg.num_tx_cnf_async_alloc_err++;
            rcp_host_store.async_in_progress = false;
        } else {
            rcp_host_dbg.num_tx_cnf_async_failure++;
            rcp_host_store.async_in_progress = false;
        }
        rcp_host_store.mac_api->data_conf_ext_cb(rcp_host_store.mac_api, &mac_data_conf, NULL);
    }
    else if(data_cnf->cnf_type == RCP_DATA_CNF_BROADCAST)
    {
        if (data_cnf->status == MLME_SUCCESS) {
            rcp_host_dbg.num_tx_cnf_broadcast++;
            rcp_host_store.bc_in_progress = false;
        } else if (data_cnf->status == MLME_MAC_ALLOCATION_ERROR) {
            rcp_host_dbg.num_tx_cnf_broadcast_alloc_err++;
            rcp_host_store.bc_in_progress = false;
        } else {
            rcp_host_dbg.num_tx_cnf_broadcast_failure++;
            rcp_host_store.bc_in_progress = false;
        }
        rcp_host_store.mac_api->data_conf_ext_cb(rcp_host_store.mac_api, &mac_data_conf, &mac_data_conf_ie);
    }
    else if(data_cnf->cnf_type == RCP_DATA_CNF_UNICAST)
    {
        if (data_cnf->status == MLME_SUCCESS) {
            rcp_host_dbg.num_tx_cnf_unicast++;
            rcp_host_store.uc_in_progress = false;
        } else if (data_cnf->status == MLME_MAC_ALLOCATION_ERROR) {
            rcp_host_dbg.num_tx_cnf_unicast_alloc_err++;
            rcp_host_store.uc_in_progress = false;
            rcp_host_store.mac_api->data_conf_ext_cb(rcp_host_store.mac_api, &mac_data_conf, &mac_data_conf_ie);
            goto ERROR_HANDLER;
        } else {
            rcp_host_dbg.num_tx_cnf_unicast_failure++;
            rcp_host_store.uc_in_progress = false;
            rcp_host_store.mac_api->data_conf_ext_cb(rcp_host_store.mac_api, &mac_data_conf, &mac_data_conf_ie);
            goto ERROR_HANDLER;
        }
        if (data_cnf->ack_frame == NULL || data_cnf->ack_frame_len == 0) {
            mac_data_conf.status = MLME_TX_NO_ACK;
            rcp_host_dbg.num_err_uc_no_ack++;
            rcp_host_store.mac_api->data_conf_ext_cb(rcp_host_store.mac_api, &mac_data_conf, &mac_data_conf_ie);
            goto ERROR_HANDLER;
        }
        //populate data_conf_ie
        const uint8_t *ptr = mac_header_parse_fcf_dsn(&fcf_read, (const uint8_t*)data_cnf->ack_frame);
        buffer = mcps_sap_pre_parsed_frame_buffer_get(data_cnf->ack_frame, data_cnf->ack_frame_len);
        if (!buffer) {
            mac_data_conf.status = MLME_TX_NO_ACK;
            rcp_host_dbg.num_err_mem_alloc_fail_tx_conf++;
            rcp_host_store.mac_api->data_conf_ext_cb(rcp_host_store.mac_api, &mac_data_conf, &mac_data_conf_ie);
            goto ERROR_HANDLER;
        }
        if (mac_parse_length_fields(buffer, data_cnf->ack_frame, data_cnf->ack_frame_len, ptr)) {
            mac_data_conf.status = MLME_TX_NO_ACK;
            rcp_host_dbg.num_err_uc_ack_parse_err_len++;
            rcp_host_store.mac_api->data_conf_ext_cb(rcp_host_store.mac_api, &mac_data_conf, &mac_data_conf_ie);
            goto ERROR_HANDLER;
        }
        if (!mac_header_information_elements_parse(buffer)) {
            mac_data_conf.status = MLME_TX_NO_ACK;
            rcp_host_dbg.num_err_uc_ack_parse_err_ie++;
            rcp_host_store.mac_api->data_conf_ext_cb(rcp_host_store.mac_api, &mac_data_conf, &mac_data_conf_ie);
            goto ERROR_HANDLER;
        }

        mac_data_conf_ie.headerIeList = buffer->headerIePtr;
        mac_data_conf_ie.headerIeListLength = buffer->headerIeLength;
        mac_data_conf_ie.payloadIeList = buffer->payloadsIePtr;
        mac_data_conf_ie.payloadIeListLength = buffer->payloadsIeLength;
        mac_data_conf_ie.payloadPtr = buffer->macPayloadPtr;
        mac_data_conf_ie.payloadLength = buffer->mac_payload_length;
        rcp_host_store.mac_api->data_conf_ext_cb(rcp_host_store.mac_api, &mac_data_conf, &mac_data_conf_ie);

    }
    else
    {
        rcp_host_dbg.num_err_wrong_type++;
        MAC_ASSERT(0);
    }

ERROR_HANDLER:
    mcps_sap_pre_parsed_frame_buffer_free(buffer);
    rcp_host_dbg.num_tx_conf_Txqueue_lmac++;
    // invoke data confirm handler
    rcp_tx_data_queue_tx_confirm_handler(data_cnf->msdu_handle);
}

void handle_rcp_data_ind(rcp_data_ind_t * data_ind)
{
    rcp_host_dbg.num_rx_ind++;

    //MVTODO: consider moving this code to a separate function?
    protocol_interface_rf_mac_setup_s *rf_ptr =  rcp_host_store.setup;

    mac_pre_parsed_frame_t *buffer = NULL;
    mac_fcf_sequence_t fcf_read;

    const uint8_t *ptr = mac_header_parse_fcf_dsn(&fcf_read, (const uint8_t*)data_ind->data_ptr);
    buffer = mcps_sap_pre_parsed_frame_buffer_get(data_ind->data_ptr, data_ind->data_len);
    if (!buffer) {
        rcp_host_dbg.num_err_mem_alloc_fail_data_ind++;
        goto ERROR_HANDLER;
    }

    //Copy Pre Parsed values
    buffer->fcf_dsn = fcf_read; //recheck this assignment
    buffer->timestamp = data_ind->rx_timestamp;
    buffer->ack_pendinfg_status = 0; //no significance for now

    // Set default flags
    buffer->dbm = data_ind->signal_dbm;
    buffer->LQI = data_ind->link_quality;
    buffer->mac_class_ptr = rcp_host_store.setup;

    if (mac_parse_length_fields(buffer, data_ind->data_ptr, data_ind->data_len, ptr)) {
        goto ERROR_HANDLER;
    }
    if (!mac_header_information_elements_parse(buffer)) {
        goto ERROR_HANDLER;
    }

    rcp_data_sap_rx_handler(buffer, rcp_host_store.setup, rcp_host_store.mac_api);

ERROR_HANDLER:
    mcps_sap_pre_parsed_frame_buffer_free(buffer);
}

extern uint8_t deviceExtAddr[8];
extern bool extAddrFlag;
void rcp_host_from_lmac(rcp_cmd_t *rcp_cmd)
{
    switch (rcp_cmd->rcp_cmd_type) {
        case RCP_DATA_CNF:
            mcps_sap_rcp_data_cnf((rcp_data_cnf_t *) rcp_cmd->rcp_data);
            break;
        case RCP_DATA_IND:
            mcps_sap_rcp_data_ind((rcp_data_ind_t *) rcp_cmd->rcp_data);
            break;
        case RCP_INIT_CNF:
            memcpy(deviceExtAddr, ((rcp_init_cnf_t *) rcp_cmd->rcp_data)->device_ext_addr, 8);
            extAddrFlag = true;
#ifdef LINUX_NANOSTACK
            tr_info("<-----------------------RCP init CNF received");
#endif
            break;
        case RCP_MAC_INIT_CNF:
            mcps_sap_rcp_set_cnf(MCPS_SAP_RCP_MAC_INIT_CNF, (rcp_mac_config_set_cnf_t *) rcp_cmd->rcp_data);
            break;
        case RCP_FH_INIT_CNF:
            mcps_sap_rcp_set_cnf(MCPS_SAP_RCP_FH_INIT_CNF, (rcp_mac_config_set_cnf_t *) rcp_cmd->rcp_data);
            break;
        case RCP_MAC_RESET_CNF:
            mcps_sap_rcp_set_cnf(MCPS_SAP_RCP_MAC_RESET_CNF, (rcp_mac_config_set_cnf_t *) rcp_cmd->rcp_data);
            break;
        case RCP_MAC_CONFIG_SET_CNF:
            mcps_sap_rcp_set_cnf(MCPS_SAP_RCP_MAC_CONFIG_SET_CNF, (rcp_mac_config_set_cnf_t *) rcp_cmd->rcp_data);
            break;
        default:
            break;
    }
}

rcp_status_t rcp_host_data_req(rcp_data_req_type data_req_type,
                          uint8_t *data, uint16_t data_len, uint8_t msdu_handle,
                          mlme_security_t *sec, uint16_t mdata_offset,
                          uint32_t frame_count, uint8_t frame_count_offset,
                          uint16_t utie_offset, uint16_t btie_offset, fhnt_entry_t *fhnt_entry)
{
    rcp_data_req_t data_req;

    memset(&data_req, 0, sizeof(data_req));

    switch (data_req_type) {
        case RCP_DATA_REQ_ASYNC:
            if (rcp_host_store.async_in_progress == true) {
                rcp_host_dbg.num_tx_cnf_async_reject++;

                return RCP_ERR_DATA_REQ_REJECT;
            } else {
                rcp_host_store.async_in_progress = true;
                rcp_host_dbg.num_tx_async++;
            }
            break;
        case RCP_DATA_REQ_UNICAST:
            if (rcp_host_store.uc_in_progress == true) {
                rcp_host_dbg.num_tx_cnf_unicast_reject++;
                return RCP_ERR_DATA_REQ_REJECT;
            } else {
                rcp_host_store.uc_in_progress = true;
                rcp_host_dbg.num_tx_unicast++;

                // only for unicast packet, we will use NT entry
                memcpy(&data_req.fhnt_entry, fhnt_entry, sizeof(fhnt_entry_t));
            }
            break;
        case RCP_DATA_REQ_BROADCAST:
            if (rcp_host_store.bc_in_progress == true) {
                rcp_host_dbg.num_tx_cnf_broadcast_reject++;
                return RCP_ERR_DATA_REQ_REJECT;
            } else {
                rcp_host_store.bc_in_progress = true;
                rcp_host_dbg.num_tx_broadcast++;
            }
            break;
        default:
            rcp_host_dbg.num_err_wrong_type++;
            MAC_ASSERT(0);
            break;
    }

    data_req.req_type = data_req_type;
    data_req.msdu_handle = msdu_handle;
    data_req.min_tx_frame_count = frame_count;
    data_req.frame_count_offset = frame_count_offset;
    memcpy(&data_req.sec, sec, sizeof(mlme_security_t));
    data_req.mdata_offset = mdata_offset;
    data_req.data_len = data_len;
    data_req.utie_offset = utie_offset;
    data_req.btie_offset = btie_offset;
    data_req.data_ptr = data;

    rcp_host_dbg.num_data_tx++;
    rcp_host_to_lmac(RCP_DATA_REQ, (void *) &data_req);
    return RCP_SUCCESS;
}

void rcp_init()
{
    rcp_host_dbg.num_config_tx++;
    rcp_host_dbg.num_config_rcp_init++;
#ifdef LINUX_NANOSTACK
    tr_info("<-------------------------RCP Host: Sending RCP INIT");
#endif
    rcp_host_to_lmac(RCP_INIT, NULL);
}

void rcp_host_mac_init(mlme_start_t *start_req)
{
    rcp_host_dbg.num_config_tx++;
    rcp_host_dbg.num_config_mac_init++;

    tr_info("<-------------------------RCP Host: Sending MAC INIT");
    rcp_host_to_lmac(RCP_MAC_INIT, (void *) start_req);
}

void rcp_host_fh_init()
{
    rcp_host_dbg.num_config_tx++;
    rcp_host_dbg.num_config_fh_init++;

    tr_info("<-------------------------RCP Host: Sending FH INIT");
    rcp_host_to_lmac(RCP_FH_INIT, NULL);
}

void rcp_host_mac_reset()
{
    rcp_host_dbg.num_config_tx++;
    rcp_host_dbg.num_config_mac_reset++;

    tr_info("<--------------------------RCP Host: Sending MAC RESET");
    rcp_host_to_lmac(RCP_MAC_RESET, NULL);
}

void rcp_host_set_bc_timing(broadcast_timing_info_t *bc_timing_info, bool force_synch)
{
    rcp_mac_config_bc_timing_t bc_timing_config;

    memcpy(&bc_timing_config.bc_timing_info, bc_timing_info, sizeof(bc_timing_config.bc_timing_info));
    bc_timing_config.force_sync = force_synch;
    rcp_host_set_config(RCP_CONFIG_BC_TIMING_INFO, &bc_timing_config, sizeof(bc_timing_config));
}

void rcp_host_set_sec_key(uint8_t index, uint8_t key[16])
{
    rcp_mac_config_sec_key_t sec_key_config;

    sec_key_config.key_index = index;
    memcpy(&sec_key_config.key, key, sizeof(sec_key_config.key));
    rcp_host_set_config(RCP_CONFIG_SEC_KEY, &sec_key_config, sizeof(sec_key_config));
}

void rcp_host_set_tx_frame_count(uint8_t index, uint32_t frame_count)
{
    rcp_mac_config_sec_frame_count_t sec_frame_count_config;

    sec_frame_count_config.key_index = index;
    sec_frame_count_config.frame_count = frame_count;

    rcp_host_set_config(RCP_CONFIG_SEC_FRAME_COUNT, &sec_frame_count_config, sizeof(sec_frame_count_config));
}

void rcp_host_set_config(uint16_t attr, void *val, uint16_t val_len)
{
    rcp_mac_config_set_t mac_config;
    mac_config.attr = attr;
    mac_config.val_len = val_len;
    mac_config.val = val;

    rcp_host_dbg.num_config_tx++;
    rcp_host_dbg.num_config_set_config++;
    rcp_host_to_lmac(RCP_MAC_CONFIG_SET, (void *) &mac_config);
}
