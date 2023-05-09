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

#ifndef RNET_LINK_LAYER_H
#define RNET_LINK_LAYER_H

#include <rnet_errors.h>
#include <sid_protocol_defs.h>
#include <sid_protocol_opts.h>
#include "rnet_time.h"
#include "rnet_app_layer.h"
#include "rnet_transact_store.h"
#include "rnet_dev_addr_def.h"

/*Max value: 0xF*/
#define RNET_FLEX_PROTOCOL_VERSION                      0x1
#define RNET_RF_PROTOCOL_VERSION_MIN                    0x2    // minimum protocol version supported by this code
#define RNET_RF_PROTOCOL_VERSION                        0x4    // current protocol version
#define RNET_RF_PROTOCOL_VERSION_MIN_SP                 0x3    // minimum protocol version that supports short preamble
#define RNET_RF_PROTOCOL_HDR_VERSION                    0x0    // version transmitted in hdr beacon used for indicating
                                                               // changes in beacon format and other hdr phy changes
#define RNET_LL_SEQ_SZ_BYTES                            0x2    // maximum allowed bytes in sequence number
#define RNET_SEQN_1_BYTE_MAX_VALUE 0x3F
#define RNET_SEQN_2_BYTE_MAX_VALUE 0x3FFF
#define RNET_SEQN_3_BYTE_MAX_VALUE 0x3FFFFF

#define RNET_RF_PROTOCOL_UNIFIED_ASL_VERSION            0x5

typedef struct {
  U8                   ble_enabled:1;
  U8                   p2p_enabled:1;
  U8                   star_enabled:1;
  U8                   bridge_enabled:1;
  U8                   hdr_enabled:1;
} rnet_ll_settings_t;


typedef struct {
  U8              ble_connected:1;
  U8              p2p_connected:1;
  U8              star_connencted:1;
#if defined(GW_SUPPORT) && GW_SUPPORT
  bool            dfu_in_progress;
  time_def        dfu_start_tdef;
#endif
  S8              ble_rssi;
  S8              p2p_rssi;
  S8              p2p_snr;
  S8              star_rssi;
  time_def        last_p2p_tm;
  time_def        last_star_tm;
  time_def        last_ble_tm;
} rnet_ll_data_t;

typedef struct {
  U8            lbt_enabled:1;
  U8            pending_lbt:1;
  U8            lbt_active:1;
  U8            tx_channel_free:1;
  U8            backoff_num;
  U8            backoff_exp;
  U8            max_backoff_exp;
  U16           backoff_unit_ms;
  U32           total_ch_busy_events;
  U16           fixed_backoff_ms;
  time_def      next_lbt_time;
} rnet_p2p_mac_data_t;


#if !defined(GW_SUPPORT) || !GW_SUPPORT
typedef struct {
    U8 bridge_dev_id[RNET_DEV_ID_SZ];
    S16 last_rssi_from_bridge;
    int8_t last_snr_from_bridge;
} rnet_ll_bridge_params_t;
/*
 * @brief Returns information about the bridge the device is synchronized with
 */
const rnet_ll_bridge_params_t*    rnet_ll_bridge_params_get(void);
void rnet_ll_bridge_params_set(const U8* src, U8 src_size, S16 rssi, int8_t snr);
#endif

/*
 * @brief Function to force the ll sequence number to a certain value
 */
sid_error_t set_linklayer_seq_no(int16_t num);

typedef struct {
    U8 frame_type;
    U8 src_frmt;
    U8 *src;
    U8 dst_frmt;
    U8 dst_sz;
    U8 *dst;
    U8 ack_req;
    U8 suppress_ack;
    U8 proto_ver;
    U8 link_layer_frame_only;
    U16 sleepy_device;
} ll_opts_t;

void            rnet_ll_on_receive(phy_rx_data_t* p_rcvd_pkt, rnet_link_t link_type);
rnet_error_t    rnet_create_ll_frame(rnet_pkt_t* pkt, ll_opts_t* ll_opts);
void            set_ll_seq_no(rnet_pkt_t *pkt, uint32_t *pkt_seq_num);
void            log_ll_frame(ll_frame_t* p_ll_frame);
void            rnet_ll_on_transmit(U8 tr_idx, uint8_t rtc_value);
void            rnet_ll_init();
bool            rnet_ll_dest_includes_this_device(U8 tr_idx);
#if defined(GW_SUPPORT) && GW_SUPPORT
bool            rnet_ll_dest_for_bridge(U8 dst_frmt, U8* dst);
#else
bool            rnet_ll_dest_for_device(U8 dst_frmt, U8* dst);
#endif
bool            rnet_ll_dest_includes_remote(U8 tr_idx);
int16_t rnet_ll_encode_packet(uint8_t idx, uint8_t* buf);
bool            rnet_tr_cancel_seqnum(U32 seq_num);
#if defined(GW_SUPPORT) && GW_SUPPORT
void            rnet_ll_set_dfu_in_progress();
bool            rnet_ll_is_dfu_in_progress();
#endif // GW_SUPPORT

#define MAX_NUM_NODECOUNT 70
#define RNET_HASH_CNT     10
#define RNET_DEVID_HASH(devid) (devid[4]%RNET_HASH_CNT)
#define RNET_PAN_HASH_CNT     10
#define RNET_GRP_NUM_HASH(group_num) (group_num % RNET_PAN_HASH_CNT)

//number of bits per byte
#define NBBY 8
#define SHBITS 0xFFFF
#define RNET_SETTR_INFO(cmd_class, cmd_id) ((cmd_class<<(2*NBBY) | cmd_id))

#define RNET_GETTR_CMDCLASS(trinfo) (trinfo >> (2*NBBY))
#define RNET_GETTR_CMDID(trinfo) (trinfo & SHBITS)

#define RNET_LRU_AGEOUT_MS       (1000)        // 1 sec

#define RNET_PAN_NODE_AGEOUT_SEC (2*60*60)  // 2 Hour timeout
#define RNET_WAN_NODE_AGEOUT_SEC (5*60)    // 5 minutes
#define RNET_WAN_NODE_MAX_AGEOUT_SEC (15*60)    // 15 minutes
#define RNET_AGEOUT_LRU_GAP 21
#define RNET_AGEOUT_LRU_CNT (MAX_NUM_NODECOUNT - RNET_AGEOUT_LRU_GAP)

#if (RNET_AGEOUT_LRU_CNT <= 0)
#error "LRU CNT is negetive"
#endif

void * get_ageout_tmr_hndlr(void);
/*
 * @brief Initializes ageout timer
 *
 */
rnet_error_t rnet_nodedb_init_ageout_timer(void);
/*!
 * @brief schedule the nodeDB ageout callback
 * This will schedule a timer for rnet_ageout_node() for specified time.
 *
 * @param timeout_msecs: ageout timer duration.
 */
void rnet_schedule_node_ageout(U32 timeout_msecs);

/*!
 * @brief ageout processing for nodeDB
 * iterate through the nodeDB and remove nodes that are stale.
 * if any node age is more than 30 secs it will be removed
 * @param context: not used.
 */
void rnet_ageout_node_process(void *context);

/*!
 * @brief dump the nodeDB entries
 * iterate through the nodeDB and print the node entries.
 */
void rnet_nodedb_dump();

rnet_node_t* __rnet_lookup_node(U8 *devid);
rnet_node_t*  rnet_lookup_add(U8 *devid);
void  rnet_remove_node(U8 *devid);
#if defined(PROTOCOL_STANDALONE_BUILD) && PROTOCOL_STANDALONE_BUILD
void rnet_clear_nodedb(void);
#endif
uint32_t rnet_nodedb_get_count(void);

/* @brief : Function to update the Data rate
 * of the node to Sideband.
 * @params[in] devid : pointer to the devid
 * @params[in] new_data_rate : new data_rate that the end device should be
 *    updated to.
 * @params[out] : error
 */
sid_error_t rnet_update_node_data_rate(U8 *devid, tx_sub_mod_t new_data_rate);

/* @brief : Decoder for Compressed TLVs
 * function to take in the payload buffer from the pkt
 * and decode the key, len and the values.
 *
 * @params[in] in_buf : Pointer for the encoded tlv data buffer
 * @params in_buf_max_size : Max size of the input buffer
 * @params[out] len : The size of the data written to the out_buf buffer. The size of the
 *   out_val_len field is 1 byte.
 * @params[out] key : The key of the data decoded. The size of the key field is 1 byte.
 * @params[out] out_buf : The data payload
 * @params  out_buf_max_size: The maximum amount of bytes that can be written into out_buf.
 * @return Returns the amount of data successfully decoded from in_buf.
 *   In the case of an error, 0 is returned.
 */
uint8_t rnet_compressed_tlv_decoder(uint8_t *in_buf, uint8_t in_buf_max_size,
        uint8_t *len, uint8_t *key);
/* @brief Encoder for Compressed TLVs
 * function to take the value that is passed as pyld param and based on the len of
 * the value being passed, call the correct helper function. The first two bits of the first
 * byte denote the size of the payload. The remaining six bits of the byte denote the key.
 *
 * @params[in]  val: Payload to be added in the TLV.
 * @params len: Length of the value
 * @params key: Key that needs to be added
 * @params[out] out_buf: buffer that gets populated with the compressed tlv
 * @params out_buf_max_size: Max amount of bytes that can be written into the out_buf
 * @return: Number of bytes successfully encoded into out_buf. In the case of an error, 0 is returned.
 */
uint8_t rnet_compressed_tlv_encoder(const uint8_t *val, size_t len, uint8_t key,
        uint8_t *out_buf, uint8_t out_buf_max_size);

void rnet_group_update(rnet_pan_group_t *group_info);
rnet_pan_group_t* rnet_group_add(U8 group_num);
rnet_pan_group_t* rnet_group_lookup(U8 group_num);
#if (!defined(GW_SUPPORT) || !GW_SUPPORT) && RNET_SECURITY_BUILD && HALO_PROTCL_USE_UUID
/* @brief: Function to reset sequence number when uuid changes
 * @param ref reference used for tx_uuid
 */
void reset_ll_seq_no(uint32_t ref);
#endif

/*!
 * @brief rx ack frame check
 * @param frame: pointer to the rx pkt
 * @param len: length of the received packet
 * @param tx_pkt: pointer to tr entry of tx pkt
 * @return bool true if a valid ack is received and matched with the packet pointed by txq, false otherwise
 */
bool sid_ll_rx_ack_frame_check(uint8_t *frame, uint8_t len, rnet_transact_store_t *tx_pkt);

/*!
 * @brief Get packet link layer header hexdump string
 * The returned string is static-allocated inside the function,
 * consecutive calls will overwrite the result.
 * @param[in] ll_frame Pointer to link layer frame structure
 * @param[in] pkt_buf Pointer to packet buffer
 * @param[in] buf_len The length of packet buffer
 * return Pointer to the result string
 */
char* sid_ll_get_header_hexdump(ll_frame_t *ll_frame, uint8_t *pkt_buf, uint8_t buf_len);

#endif
