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

#ifndef RNET_API_H
#define RNET_API_H

#include <rnet_errors.h>
#include <sid_protocol_defs.h>
#include <sid_protocol_opts.h>
#include "rnet_app_layer.h"
#include "rnet_sdb.h"
#include "rnet_transact_store.h"
#include "rnet_network_gateway_discovery.h"
#include "rnet_dev_addr_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RNET_MAX_CMD_DATA_LEN           48

#define RNET_TX_STATUS_PENDING         0
#define RNET_TX_STATUS_TRANSMITTED     1
#define RNET_TX_STATUS_DELIVERED       2
#define RNET_TX_STATUS_FAILED          3

#define RNET_CMD_HDR_NULL              0x00
#define RNET_CMD_HDR_SCI_ONLY          0x80

#define APP_SEQN_1_BYTE_MAX_VALUE      0x3F
#define APP_SEQN_2_BYTE_MAX_VALUE      0x3FFF
#define APP_SEQN_3_BYTE_MAX_VALUE      0x3FFFFF

typedef enum {
    RNET_MFG_STORE_BRIDGE_CLOUD_AUTH_TOKEN = 1,
    RNET_MFG_STORE_WIFI_MAC = 2,
    RNET_MFG_STORE_ETH_MAC = 3,
    RNET_MFG_STORE_PROXY_FLAG = 4,
} rnet_mfg_store_value_t;

typedef enum {
    RNET_MFG_STORE_BRIDGE_CLOUD_AUTH_TOKEN_SIZE = 37,
    RNET_MFG_STORE_WIFI_MAC_SIZE = 17,
    RNET_MFG_STORE_ETH_MAC_SIZE = 17,
    RNET_MFG_STORE_PROXY_FLAG_SIZE = 4,
} rnet_mfg_store_value_size_t;

typedef void (*rnet_mfg_store_write_callback_t)(rnet_mfg_store_value_t value, const uint8_t *const buf, size_t size);
typedef void (*rnet_mfg_store_read_callback_t)(rnet_mfg_store_value_t value, uint8_t *const buf, size_t size);

typedef struct {
    rnet_mfg_store_write_callback_t write_callback;
    rnet_mfg_store_read_callback_t read_callback;
} rnet_mfg_store_callbacks_t;

typedef int8_t  (*rnet_rx_callback_t)(rnet_cmd_t* p_rnet_cmd);
typedef void    (*rnet_tx_callback_t)(U8 msg_status, rnet_cmd_t* p_rnet_cmd);

typedef struct {
    U8 dev_id[RNET_DEV_ID_SZ];
    volatile U8 pause:1;
    U8 ble_without_hdrs:1;
} rnet_state_t;

typedef struct {
    U32 crystal_32khz;
} rnet_protocol_config_t;

typedef struct {
    rnet_rx_callback_t rx_callback;
    rnet_tx_callback_t tx_callback;
    rnet_protocol_config_t config;
} rnet_init_config_t;

typedef struct {
    uint8_t ep_id[RNET_DEV_ID_SZ];
    uint8_t status_code;
    uint32_t ll_seqn;
    uint8_t phy_mode;
} rnet_nack_t;

int16_t         get_currnet_pkt_rssi(void);
int8_t          get_currnet_pkt_snr(void);
#if !defined(GW_SUPPORT) || !GW_SUPPORT
int16_t         rssi_for_last_rcvd_pkt_from_bridge(void);
int8_t          snr_for_last_rcvd_pkt_from_bridge(void);
#endif

void            RnetGpioInit();
sid_error_t     RnetInit(rnet_init_config_t *config);
S8              RnetSendRaw(U8* buf, U8 len);
S16             RnetTestModeGetReceivedPkt(U8* buf);

#if RNET_SRL_HOST_INTERFACE && USE_SERIAL_CLIENT
void            rnet_serial_client_ifc_init(void* p_serial_ifc);
#endif

void            sid_protocol_mac_pause(void);
bool            sid_protocol_mac_resume(void);
bool            sid_protocol_mac_is_paused(void);

rnet_error_t sid_protocol_set_regional_info(uint8_t region, const char *iso_country);

rnet_error_t      rnet_on_flex_receive(U8* buf, U16 len, rnet_link_t rx_link);
S8                rnet_app_on_receive(U8 tr_idx);

rnet_error_t      rnet_add_transaction(rnet_cmd_t* p_rnet_cmd, rnet_link_t source_link, tr_opts_t* tr_opts, U8* idx);
rnet_error_t      rnet_finish_transaction(U8 txn_idx, rnet_cmd_t* p_resp_cmd);

rnet_error_t      rnet_send_msg_to_group(rnet_cmd_t* p_rnet_cmd, tr_opts_t* opts);
rnet_error_t      rnet_send_msg_to_device(rnet_cmd_t* p_rnet_cmd, const U8* dev_id, tr_opts_t* opts, rnet_link_t src);
rnet_error_t      rnet_send_msg_to_cloud(rnet_cmd_t* p_rnet_cmd, tr_opts_t* opts);
S8              rnet_send_flex_frame(U8 tr_idx);
void            rnet_app_on_transmit(U8 tr_idx);
rnet_error_t      rnet_create_pkt(rnet_pkt_t* pkt, rnet_cmd_t* p_cmd, tr_opts_t* tr_opts);
rnet_error_t      rnet_create_ack(rnet_pkt_t* pkt, tr_opts_t* tr_opts, U32 seq_num);
void            rnet_process();
void            rnet_check_wake_tm();
rnet_error_t      rnet_app_send_response(rnet_link_t rcv_link, ll_frame_t *ll_frame, U8 nwl_encr_ena, U8 asl_sec_ena, rnet_cmd_t * p_resp_cmd);
#define MAX_RNET_CALLBACKS 5

typedef int8_t(*rnet_cmd_callback_t)(uint8_t* data, uint8_t* resp_data);

void rnet_callbacks_set(rnet_rx_callback_t rx_callback, rnet_tx_callback_t tx_callback);

void rnet_mfg_store_callback_set(const rnet_mfg_store_callbacks_t *const mfg_store_cb);

typedef struct{
    uint16_t class_id;
    uint16_t id;
    rnet_cmd_callback_t callback;
}rnet_cmd_callback_entry_t;

rnet_error_t rnet_add_cmd_callback(uint16_t class_id, uint16_t id, rnet_cmd_callback_t callback);
int8_t ring_net_cmd_callback(uint16_t class_id, uint16_t id, uint8_t* data, uint8_t* resp_data);
void rnet_protocol_mismatch(uint16_t cmd_class, uint16_t cmd_id, rnet_link_t rx_link, tr_opts_t *tr);

#if defined(GW_SUPPORT) && GW_SUPPORT
#define DEFAULT_ANNOUNCEMENT_INTERVAL           86400 // one day
#define DEFAULT_ANNOUNCEMENT_OFFSET             0 // 0 for random offset
#define DEFAULT_ANNOUNCEMENT_REPETITIONS        5
#define DEFAULT_DATA_RATE                       NGD_DATA_RATE_NO_PREFERENCE
#define RNET_CMD_ANNOUNCEMENT_DATA_LEN          1

#define NG_DISCOVERY_CTRL_ENABLE_BIT0   0 // bit-0: enabled or disabled

typedef struct {
    uint32_t  interval_time; // interval time
    uint32_t  offset_time; // offset time
    uint32_t  rf_channels; // RF channels bitmap
    uint8_t   control_flags; // neighbor gateway discovery enabled/disabled
    uint8_t   channel_retry; // no of retry should be performed in each channel
    ngd_phy_mode_t phy_data_rate; //Proffered phy data rate to run the discovery: RNET_RADIO_DATA_RATE_INVALID means n preference on data rate
}__attribute__((packed)) rnet_ngd_params_t;    //neighbor gateway discovery parameter

typedef struct {
    uint8_t   prtcl_ver; //protocol version
    uint8_t   cfg_ver;   //configuration version
}__attribute__((packed)) rnet_version_t;

typedef struct {
    rnet_version_t rnet_ver;
    rnet_ngd_params_t rnet_ngd_params;
}__attribute__((packed)) rnet_ngd_params_ver_t;

typedef enum {
    NGD_WAIT_FOR_OFFSET,
    NGD_WAIT_FOR_NEXT_CYCLE,
    NGD_CHANNEL_CYCLING
} rnet_discovery_state_t;

/*
 * @brief Function to get current neighbor gw discovery parameters.
 *
 * @return pointer to the parameter <rnet_ngd_params_t*>
 */
rnet_ngd_params_t* rnet_get_ngd_param(void);
#endif

/*
 * @brief Function to get the flag status
 *      Flag- to know if we need the use the
 *      sequence number set by the cloud
 */
bool rnet_is_use_cld_seq_no(void);

/*
 * @brief Function to set the flag
 */
void rnet_set_use_cld_seq_no_flag(void);

/*
 * @brief Function to clear the flag
 */
void rnet_clr_use_cld_seq_no_flag(void);

/*
 * @brief Function to get cloud provided sequence number
 */
uint32_t rnet_get_cld_seq_no(void);

/*
 * @brief Function to set cloud provided sequence number
 */
void rnet_set_cld_provided_seq_no(uint32_t seq_num);

/*
 * @brief get the sequence size sent by cloud
 */
uint32_t rnet_get_cld_sqn_sz(void);

/*
 * @brief: Function to get if rnet is initialized
 *
 * @return True if Rnet has been initialized
 */
bool rnet_get_is_rnet_init(void);

/*
 * @brief: function to convert link enum to string
 *
 * @param[in] link enum as uint8_t
 * @param[out] string poimter
 */
const char* rnet_get_link_str(uint8_t idx);

/*
 * @brief: function to convert address format enum to string
 *
 * @param[in] address format enum as uint8_t
 * @param[out] string poimter
 */
const char* rnet_get_addr_frmt_str(uint8_t idx);

static inline S16 rnet_memcpy(char *dst, const char *src, size_t buf_len, U16 *buf_max)
{
    if (*buf_max < buf_len) {
        return RNET_NODE_NO_MEM;
    }
    *buf_max -= buf_len;
    memcpy (dst, src, buf_len);
    return 0;
}

rnet_error_t rnet_get_setting(storage_kv_key_t key, U8 sz, U8* p_setting, const U8* p_default);

struct sdb_app_handle *get_sdb_app_handle(void);

#if defined(GW_SUPPORT) && GW_SUPPORT
/**
 * @brief Get/Set network gateway discovery params from flash
 * while checking the cfg and protocol versions
 * If there's a version mismatch, default ngd params will be loaded
 * @param[in] rnet_ngd_params_t* rnet_api_ngd_params : buffer to read flash contents
 * @param[in] rnet_ngd_params_t* def_ngd_params : default params
 * @param[in] cfg_opt_t READ/WRITE operation
 * @ret rnet_error_t error code
 */
rnet_error_t rnet_ngd_params_ver_chckr(rnet_ngd_params_t* rnet_api_ngd_params, rnet_ngd_params_t* def_ngd_params, cfg_opt_t opt);

/**
 * @brief Check whether the message can be forwarded to the application to be processed
 * The messages belonging to custom class for gateways can only be sent by GW host via the serial
 * link and with source and destination format set to device ID. If the messages belonging to custom
 * class that do not satisfy any of this requirements will be dropped by protocol stack
 */
bool notify_app_on_receive(rnet_transact_store_t* tr);
#endif

#ifdef __cplusplus
}
#endif

#endif
