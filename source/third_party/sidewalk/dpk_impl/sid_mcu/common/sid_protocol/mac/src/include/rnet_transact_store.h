/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef RNET_TRANSACT_STORE_H
#define RNET_TRANSACT_STORE_H

#include <sid_protocol_defs.h>
#include "rnet_time.h"
#include "rnet_frames.h"
#include <sid_900_cfg.h>
#include "rnet_ll_phy_interface.h"
#include <sid_protocol_opts.h>
#include "rnet_dev_addr_def.h"

#include <sid_pal_radio_ifc.h>
#include <sid_security_ifc.h>
#include <sid_error.h>

#if defined(GW_SUPPORT) && GW_SUPPORT
#define TRANSACT_STORE_DEPTH                    40
#define TRANSACT_STORE_MEM_POOL_SIZE            (TRANSACT_STORE_DEPTH * 94)
#else
#define TRANSACT_STORE_DEPTH                    20
#define TRANSACT_STORE_MEM_POOL_SIZE            (TRANSACT_STORE_DEPTH * 94)
#endif

//Low water mark for transact store below which we will drop tx frames
#define TRSTORE_LOW_WATER_MARK_TX               4
//Low water mark for transact store below which we will drop rx frames
#define TRSTORE_LOW_WATER_MARK_RX               2

#define RX_TSTAMP_GCS_SZ_BYTES                  6
#define OUTER_FLEX_SEQ_SZ                       0X02
#define NUM_OF_ACTIVE_SEQ_BIT_IN_FIRST_BYE      6
#define MAX_SEQ_SZ_BYTES                        3

/*Gateway Encryption related definition*/
#define GW_ENCR_AUTH_TAG_LEN                    12

/* max lifetime for passthrough packets is twice the TTL of the time reference
 * TODO: packet lifetime should be based on the could timestamp
 * instead of MCU timestamp: https://issues.labcollab.net/browse/SWCLOUD-9554
 * TODO: flush mac txq packets once high watermark is reached
 * https://issues.labcollab.net/browse/HALO-29957
 */
#define SID_MAX_PKT_LIFETIME_MS (2*SID_SEC_IV_REF_TTL*1000)

typedef enum {
    FLEX_KEY_DST = 0x1,
    FLEX_KEY_DST_FRMT = 0x2,
    FLEX_KEY_ACK_REQ = 0x3,
    FLEX_KEY_ENCR_ENA = 0x4,
    FLEX_KEY_AUTH_CODE_SZ = 0x5,
    FLEX_KEY_RESP_REQ = 0x6,
    FLEX_KEY_APP_SEQN_SZ = 0x7,
    FLEX_KEY_APP_SEQN = 0x8,
    FLEX_KEY_LINK_FOR_TX = 0x9,
    FLEX_KEY_RETRIES = 0xA,
    FLEX_KEY_RSSI = 0xB,
    FLEX_KEY_SNR = 0xC,
    FLEX_KEY_CARRIER_OFFSET = 0xD,
    FLEX_KEY_RX_GCS_TSTAMP = 0xE,
    FLEX_SEC_ENABLED = 0xF,
    FLEX_KEY_SRC = 0x10,
    FLEX_KEY_SRC_FRMT = 0x11,
    FLEX_KEY_PROTOCOL_VER = 0x12,
    FLEX_KEY_SUPPRESS_BCAST_ACK = 0x13,
    FLEX_KEY_SLEEPY_DEVICE = 0x14,
    FLEX_KEY_CHANNEL_NUMBER = 0x15,
    FLEX_KEY_PHY_MODE = 0x16,
    FLEX_KEY_NETWORK_CAPABILITY = 0x17,
    FLEX_KEY_AVG_NOISE = 0x18,
    FLEX_KEY_RX_EPOCH_SEC = 0x19,
    FLEX_KEY_RX_EPOCH_NANO_SEC = 0x1A,
    FLEX_KEY_SEC_REF = 0x1B,
    FLEX_KEY_SEC_REF_COUNTER_VAL = 0x1C,
    FLEX_KEY_PROCESSING_DELAY_MS = 0x1D,
    FLEX_KEY_SEC_TOPOLOGY = 0x1E,
    FLEX_KEY_NACK_MSG = 0x1F,
    FLEX_KEY_RF_FRAME_PROTO_VRSN = 0x20,
    FLEX_KEY_LL_SEQN = 0x21,
    FLEX_KEY_IS_PASS_THROUGH_PLD = 0x22,
    FLEX_KEY_AUTH_TAG = 0x23,
    FLEX_KEY_RX_WINDOW_INTERVAL = 0x24,
    FLEX_KEY_BCN_TIME = 0x25,
    FLEX_KEY_INNER_SRC_FRMT = 0x26,
    FLEX_KEY_INNER_SRC = 0x27,
    FLEX_KEY_DEVICE_CLASS = 0x28,
    FLEX_KEY_INNER_DST_FRMT = 0x29,
    FLEX_KEY_INNER_DST = 0x2A,
    FLEX_KEY_IS_DATA_ASL_FORMATTED = 0x2B,
    FLEX_KEY_LDR_TX_BIN_IDX = 0x2C,
    FLEX_KEY_DESPREAD_RSSI = 0x2D,
} flex_k_t;


typedef enum {
    TR_PROC_ST_JUST_RCVD=1,
    TR_PROC_ST_WAITING_FOR_PROCESSING,
    TR_PROC_ST_PROCESSING,
    TR_PROC_ST_PROCESSED,
    TR_PROC_ST_RDY_TO_SEND,
    TR_PROC_ST_DONE,
    TR_PROC_ST_NTFY_TX_DONE,
} tr_proc_st;

/* Call back function pointer */
typedef void(*tx_callback_t)(void *rnet_tr, S32 status);

// Arguments supplied to stack when creating a transaction
typedef struct {
    // ll arguments
    U8 proto_ver:4;
    U8 dst_frmt:2;
    U8 src_frmt:2;

    U8 dst[RNET_DEV_ID_SZ];
    U8 src[RNET_DEV_ID_SZ];

    U32 ll_seqn;

    U8 ack_req:1;
    U8 suppress_broadcast_ack:1;
    U8 flex_proto_ver:4;
    U8 link_layer_frame_only:1;

    //nwl arguments
    nwl_opts_t nw_opts;

    // app layer args
    U8 is_pass_through_pld:1;
    U8 resp_req:1;
    U8 seqn_sz:2;
    U8 tx_bypass_block:1;
    U8 is_unified_asl:1;
    U8 sec_ena;
    U32 seqn;
    U8 enr_data_sz_byte;
    U8* encr_data;

    // Other arguments to the stack not directly encoded in frames
    rnet_link_t link_for_tx;
    U8 retries:4;
    U8 is_cld_nack:1;
    U16 sleepy_device;
    time_def device_window_epoch; //especially required for end-devices in LDR mode.
    sid_pal_radio_data_rate_t phy_mode;
    enum sid_device_profile_id device_class;

    /*additional parameters to support encoding*/
    void* frame_support_ctx;
    /* Callback function pointer on tx done */
    tx_callback_t tx_done_callback;
    // context for tx_callback
    void *tx_done_cb_ctx;
} tr_opts_t;


/*=================================*/
/*RX meta data*/
/*=================================*/
typedef struct {
    U32 rcv_ts_tu;
    time_def rcv_tm;
    rnet_link_t rcv_link;
    S16 rssi;
    S16 signal_rssi;
    S8 snr;
    S8 avg_noise;
    sid_pal_radio_data_rate_t data_rate;
    uint8_t rx_channel;
    uint32_t relative_rx_time; //packet sync time relative to the receiver's wake up time in tu for Halo 2.0 packets. Undefined for Halo 1.0.
} transact_rcv_st_t;

/*=================================*/
/*TX meta data*/
/*=================================*/
/*Tx priority definition*/
#define TX_PRIORITY_HIGH                0x0
#define TX_PRIORITY_MEDIUM              0x1
#define TX_PRIORITY_LOW                 0x2

#define SET_BIT_FLAG                    0x01
#define CLR_BIT_FLAG                    0x00

/*Tx error bit flag*/
typedef union {
    struct {
        uint8_t radio_tx_error :1;
        uint8_t queue_malloc_err :1;
        uint8_t sched_cnflct_err :1;
        uint8_t pkt_lifetime_err :1;
        uint8_t sched_evt_add_err :1;
        uint8_t invld_pkt_err :1;
        uint8_t no_link_available:1;
        uint8_t rfu :1;
    } snt_err_bit_flag;
    uint8_t err_code;
} snt_error_t;

/*Transaction send state*/
typedef struct {
    U32 last_tx_tu;     // Time of most recent transmit
    rnet_link_t snd_link; // Which link we will send the message on
    U8 tx_priority;
    U8 tx_cnt: 4;
    U8 ll_retry: 1; // flag to indicate if the pkt is being retried for tx
    U8 ack_req: 1;
    U8 ll_ack_rcvd: 1;
    U8 reserved: 1;
    U8 tx_sub_mod : 3;
#if defined(GW_SUPPORT) && GW_SUPPORT
    U8 resched_cnt: 2;   //Temp place holder for packet lifetime in LDR
    uint8_t device_class; // note: <enum sid_dev_profile_id> value will be fed here
    time_def ldr_epoch;
    uint8_t ldr_wndw_interval; //<enum sid_ldr_rx_wndw_interval_ms>
#else
#if LDR_TIME_DIVERSITY
    // When set, TX frame and retries are all EPOCH frames for synchronization
    U8 ldr_sync_enabled: 1;
#endif
#endif //GW_SUPPORT
    snt_error_t snt_err;
    U8 channel;
#if !defined(GW_SUPPORT) || !GW_SUPPORT
    int8_t tx_power;
#endif
} transact_snd_st_t;

/*=================================*/
/* Main data structure stored in transaction buffers*/
/*=================================*/
typedef struct {
    volatile U8 rsvd:1;
    volatile U8 vld:1;
    volatile U8 trsmt_on_dflt_chnl:1;
    volatile U8 is_cld_nack:1;
    volatile U8 is_pass_through_pld:1;
    volatile U8 is_ffs_pkt:1;
    tr_proc_st proc_st;
    S8 tr_idx_next; //index of the next transaction entry for the node
    rnet_pkt_t pkt;
    transact_rcv_st_t rcv_st;
    transact_snd_st_t snd_st;
    tx_callback_t tx_done_callback;
    int32_t tx_done_status;
    void *tx_done_cb_ctx;
#if !defined(GW_SUPPORT) || !GW_SUPPORT
    uint32_t msg_ttl_secs; //msg expiration time in sec
    uint8_t raw_packet_length;
#endif
} rnet_transact_store_t;

/* Additional structure to support flex key FLEX_KEY_INNER_SRC
 * Used to classify roaming and local traffic */
typedef struct {
    U8 src_addr_frmt;
    U8 src_id_sz;
    U8 src_id[RNET_DEV_ID_SZ];
    U8 dst_addr_frmt;
    U8 dst_id_sz;
    U8 dst_id[RNET_DEV_ID_SZ];
}rnet_inner_dev_info_t;

/*=================================*/
/* Function definitions*/
/*=================================*/

rnet_transact_store_t* rnet_transact_store_get(U8 idx);

/* @brief: Function to map the packet to the tr_idx
 * @params[in] : pointer to the transaction store entry
 * @param[out] : returns the idx in case of successfully
 * mapping the index of the transaction store entry
 * returns -1 in case of a null pointer being passed
 * returns -1 in corner case of packet not being part of
 * the transaction store
 */
int8_t rnet_get_tr_id(rnet_transact_store_t* tr_ptr);

sid_error_t rnet_init_tr_buffer(void);

U16 parse_flex_hdr(tr_opts_t* tr, U8* buf, U16 flex_hdr_len);

#if UNIFIED_ASL_SUPPORT
S16 encode_flex_frame(U8* buf, U16 buf_sz, U8 tr_idx, rnet_inner_dev_info_t *inner_dev_info, bool cmd_frame_has_payload);
#else
S16 encode_flex_frame(U8* buf, U16 buf_sz, U8 tr_idx, rnet_inner_dev_info_t *inner_dev_info);
#endif

S16 encode_flex_ntfy_rcvd(U8* buf, U16 buf_sz, U8 tr_idx);

U8 transact_store_num_vld();

U8 transact_store_num_free();

S8 transact_store_reserve();

S8 transact_store_delete(U8 idx);

S8 transact_store_delete_entry(rnet_transact_store_t* tr_entry);

rnet_transact_store_t* rnet_transact_store_get(U8 idx);

/* @brief: Function to allocate packet payload buffer for the payload structure
 * @params[in] : Pointer to the transaction store pkt
 * @params[in] : Payload type
 * @params[in] : Payload size
 * return : True if allocated successfully
 */
bool transact_store_allocate_pkt_pld_buffer(rnet_pkt_t *pkt, rnet_pkt_pld_type_t pld_type, size_t pld_sz);

/*
 * @brief: clears packets queued in the transactions store
 * this API is to be called before the mac is paused in order
 * to clear all the pending transactions
 *
 * If a transaction is RX side it will be dropped so that the
 * upper layers do not allocate memory and we cause a memory leak
 *
 * If a transaction is tx, the tx callback must be called in order to
 * free allocated buffers in the upper layers. The status of the
 * transaction will be set depending on its state.
 */
void transact_store_clear(void);

void tr_store_process();

S8 transact_store_get_ready_tx_dfu_frame();
S8 transact_store_get_ready_tx_start_dfu_on_dflt_chnl();

#if (!defined(GW_SUPPORT) || !GW_SUPPORT) && RNET_SECURITY_BUILD && HALO_PROTCL_USE_UUID
/*!
 * @brief: function to generate uuid from ringnet id
 * @param tr: pointer to the transaction store element carrying the packet.
 * @param reference: time reference denoting the start of the current UUID slot;
 * @param intvl_secs: pointer to carry out the UUID validity time in second.
 * @return: "HALO_ERROR_NONE" in case of success; otherwise returns corresponding error code.
 */
sid_error_t rnet_mac_get_uuid(rnet_transact_store_t *tr, uint32_t *reference, uint32_t *intvl_secs);
#endif

#endif // RNET_TRANSACT_STORE_H
