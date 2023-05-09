/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef RNETC_PUBLIC_HALO_HALL_APPV1_MGM_TYPES_H_
#define RNETC_PUBLIC_HALO_HALL_APPV1_MGM_TYPES_H_

#include <sid_network_address.h>
#include <sid_parser_utils.h>
#include <halo/lib/hall/appv1/mgm_core/types.h>
#include <halo/lib/hall/appv1/mgm_gateway/types.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// DFU
typedef enum {
    HALO_HALL_APPV1_MGM_DFU_PARAMS_V_1 = 0x0,
    HALO_HALL_APPV1_MGM_DFU_PARAMS_V_2 = 0x1,
} halo_hall_appv1_mgm_dfu_params_v;

typedef struct {
    uint8_t channel;
    uint8_t spreading_factor;
} halo_hall_appv1_mgm_dfu_params_v2_t;

typedef struct {
    uint8_t timeout_s;
} halo_hall_appv1_mgm_dfu_params_v1_t;

typedef struct {
    uint8_t version;
    union
    {
        halo_hall_appv1_mgm_dfu_params_v1_t v1;
        halo_hall_appv1_mgm_dfu_params_v2_t v2;
    };
} halo_hall_appv1_mgm_dfu_params_t;

typedef enum {
    HALO_HALL_APPV1_DATA_RATE_INVALID = 0,
    HALO_HALL_APPV1_DATA_RATE_LORA_2KBPS = 1,
    HALO_HALL_APPV1_DATA_RATE_LORA_22KBPS = 2,
    HALO_HALL_APPV1_DATA_RATE_FSK_50KBPS = 3,
    HALO_HALL_APPV1_DATA_RATE_FSK_150KBPS = 4,
    HALO_HALL_APPV1_DATA_RATE_FSK_250KBPS = 5,
    HALO_HALL_APPV1_DATA_RATE_LORA_12_5KBPS = 6,
    HALO_HALL_APPV1_MAX_DATA_RATES = HALO_HALL_APPV1_DATA_RATE_LORA_12_5KBPS, // 0 is an invalid data rate
} halo_hall_appv1_data_rate_t;

// RF power
typedef struct {
    halo_hall_appv1_data_rate_t data_rate;
    int8_t power;
} halo_hall_appv1_rf_power_t;


// channel survey
typedef enum {
    HALO_HALL_APPV1_CHANNEL_SURVEY_MODE_HALO = 0,
    HALO_HALL_APPV1_CHANNEL_SURVEY_MODE_SPECTRUM = 1,

    HALO_HALL_APPV1_CHANNEL_SURVEY_HALO_CHANNELS_NUM = 8,
    HALO_HALL_APPV1_CHANNEL_SURVEY_SPECTRUM_CHANNELS_NUM = 64,

    HALO_HALL_APPV1_CHANNEL_SURVEY_MAX_CHANNELS = HALO_HALL_APPV1_CHANNEL_SURVEY_SPECTRUM_CHANNELS_NUM
} halo_hall_appv1_channel_survey;

typedef struct {
    uint8_t scan_mode;
    uint8_t channel_min;
    uint8_t channel_max;
} halo_hall_appv1_channel_survey_prop_t;

typedef struct {
    halo_hall_appv1_channel_survey_prop_t props;
    uint8_t rssi[HALO_HALL_APPV1_CHANNEL_SURVEY_MAX_CHANNELS];
} halo_hall_appv1_channel_survey_results_t;

// Device Type for GW: 0, Device Type for c40 : 1, Device Type for Fetch : 2
typedef enum {
    HALO_HALL_APPV1_DEV_TYPE_GW = 0x0,
    HALO_HALL_APPV1_DEV_TYPE_PAN = 0x1,
    HALO_HALL_APPV1_DEV_TYPE_WAN = 0x2,
} halo_hall_appv1_dev_type_t;

typedef enum {
    HALO_HALL_APPV1_DEV_MODE_DISABLED = 0xFF,
    HALO_HALL_APPV1_DEV_MODE_MIXED = 0x0,

    HALO_HALL_APPV1_DEV_MODE_GW_PAN = 0x1,
    HALO_HALL_APPV1_DEV_MODE_GW_WAN = 0x2,
    HALO_HALL_APPV1_DEV_MODE_GW_DISTRESS = 0x3,
    HALO_HALL_APPV1_DEV_MODE_GW_MAX,

    HALO_HALL_APPV1_DEV_MODE_HDR = 0x1,
    HALO_HALL_APPV1_DEV_MODE_LDR = 0x2,
    HALO_HALL_APPV1_DEV_MODE_MAX,
} halo_hall_appv1_dev_mode_t;

typedef struct {
    uint8_t type;
    uint8_t mode;
} halo_hall_appv1_dev_profile_t;

typedef enum {
    HALO_HALL_APPV1_MGM_HDR_PARAM_UNICAST = 0x00,
    HALO_HALL_APPV1_MGM_HDR_PARAM_GROUP = 0x01,
} halo_hall_appv1_mgm_hdr_params_t;

typedef struct {
    uint8_t protocol;
    uint8_t hdr;
    uint8_t ldr;
    uint8_t beacon;
    uint8_t dfu;
    uint8_t sdb;
} halo_hall_appv1_protocol_flags_t;

typedef struct {
    halo_hall_appv1_dev_profile_t profile;
    halo_hall_appv1_protocol_flags_t flags;
    uint8_t probe_auth;
} halo_hall_appv1_dev_profile_resp_t;

typedef enum { HALO_DEVICE_ID_SIZE = 5 } halo_device_id_size;

struct halo_hall_appv1_dev_cls_ldr {
    uint8_t dev_profile_id;
    uint8_t rx_count;
    uint16_t ldr_prdcty;
};

struct halo_hall_appv1_dev_cls_fsk {
    uint8_t dev_profile_id;
    uint8_t rx_count;
};

typedef struct {
    halo_hall_appv1_dev_profile_t profile;
    uint32_t mfg_version;
    uint8_t p2p_channel;
    halo_hall_appv1_pan_id_t pan_id;
    uint8_t device_id[HALO_DEVICE_ID_SIZE];
    uint8_t group_num;
    uint8_t consent_flag;
    halo_hall_appv1_protocol_flags_t protocol_switch;
    uint8_t missed_beacons;
    uint8_t beacon_interval;
    uint8_t ldr_channel;
    uint8_t deficit_flag;
    uint16_t scan_interval;
    struct halo_hall_appv1_dev_cls_ldr dev_cls_ldr;
    struct halo_hall_appv1_dev_cls_fsk dev_cls_fsk;
} halo_hall_appv1_config_param_t;

typedef struct {
    uint8_t periodicity;
    uint8_t offset;
    uint8_t wakeup_flag;
} halo_hall_appv1_slot_param_t;

typedef struct {
    uint8_t load;
    uint8_t latency;
} halo_hall_appv1_link_characteristics_param_t;

#define HALO_HALL_APPV1_LINK_CHAR_MAX_LOAD 255
#define HALO_HALL_APPV1_LINK_CHAR_MIN_LATENCY 1
#define HALO_HALL_APPV1_LINK_CHAR_DEFAULT_LOAD HALO_HALL_APPV1_LINK_CHAR_MAX_LOAD
#define HALO_HALL_APPV1_LINK_CHAR_DEFAULT_LATENCY HALO_HALL_APPV1_LINK_CHAR_MIN_LATENCY

typedef struct {
    halo_hall_appv1_slot_param_t unicast;
    halo_hall_appv1_slot_param_t group;
    halo_hall_appv1_link_characteristics_param_t wan_dl;
    halo_hall_appv1_link_characteristics_param_t wan_ul;
} halo_hall_appv1_hdr_param_t;

typedef struct {
    uint8_t identifier;
    halo_hall_appv1_slot_param_t param;
} halo_hall_appv1_slot_params_t;

typedef enum {
    HALO_HALL_APPV1_MGM_PROXY_RESP_TIME_UUID_SIZE = 5,
    HALO_HALL_APPV1_MGM_PROXY_RESP_TIME_KEY_SIZE = 16,
} halo_hall_appv1_mgm_proxy_resp_time_prop_t;

typedef struct {
    uint8_t  uuid[HALO_HALL_APPV1_MGM_PROXY_RESP_TIME_UUID_SIZE];
    uint8_t  key[HALO_HALL_APPV1_MGM_PROXY_RESP_TIME_KEY_SIZE];
    uint32_t rnd_key;
} halo_hall_appv1_mgm_proxy_resp_time_t;

typedef enum {
    MGM_MAC_CHANNEL_SCAN_MODE_HALO     = 0,
    MGM_MAC_CHANNEL_SCAN_MODE_SPECTRUM = 1,
} mgm_mac_channel_scan_mode_t;

// Security
typedef enum {
    HALO_HALL_APPV1_SECURITY_TYPE_AES_AUTH = 0,
    HALO_HALL_APPV1_SECURITY_TYPE_AES_ENCR = 1,

    HALO_HALL_APPV1_SECURITY_GCS_ENABLED = 1
} halo_hall_appv1_security;

typedef struct {
    uint8_t type;
    uint8_t gcs;
} halo_hall_appv1_security_params_t;

typedef struct {
    halo_hall_appv1_security_params_t current;
    halo_hall_appv1_security_params_t capable;
} halo_hall_appv1_security_capabilities_t;

#define HALO_HALL_APPV1_MGM_ASYNC_PROF_A_B_D_JOIN_CMD_DATA_LEN 7
#define HALO_HALL_APPV1_FSK_WAN_JOIN_KEY_SIZE HALO_HALL_APPV1_DEVICE_SECURITY_KEY_LEN

typedef enum {
    HALO_HALL_APPV1_MGM_JOIN_RESPONSE_CODE = 0x00,
    HALO_HALL_APPV1_MGM_FSK_WAN_JOIN_RESPONSE_CODE = 0x08,
    HALO_HALL_APPV1_MGM_DL_SCHED_PROF_3 = 0x01,
    HALO_HALL_APPV1_MGM_GROUP_SCHED = 0x02,
    HALO_HALL_APPV1_MGM_SYNCED_GW = 0x03,
    HALO_HALL_APPV1_MGM_ASYNC_PROF_A_B_D = 0x04,
    HALO_HALL_APPV1_MGM_DL_CHAR_PROF_3 = 0x0C,
    HALO_HALL_APPV1_MGM_UL_SCHED_PROF_3 = 0x06,
    HALO_HALL_APPV1_MGM_UL_CHAR_PROF_3 = 0x0D,
    HALO_HALL_APPV1_MGM_UL_CONTENTION_CHAR = 0x07,
    HALO_HALL_APPV1_MGM_LINK_DURATION = 0x09,
    HALO_HALL_APPV1_MGM_SEC_COUNTER = 0x0A, // Not used
    HALO_HALL_APPV1_MGM_SEC_KEY = 0x0B,
} halo_hall_appv1_mgm_join_keys;

typedef struct {
    uint8_t current_synced_gw[SID_ADDRESS_SIZE_MAX];
    uint8_t unicast_periodicity;
    uint8_t unicast_offset;
    uint8_t fsk_wan_ul_latency;
    uint8_t fsk_wan_ul_load;
    uint8_t fsk_wan_dl_latency;
    uint8_t fsk_wan_dl_load;
    uint8_t fsk_wan_ul_periodicity;
    uint8_t fsk_wan_ul_offset;
    uint8_t fsk_wan_sec_key[HALO_HALL_APPV1_FSK_WAN_JOIN_KEY_SIZE];
    uint8_t subslot_contention_window_size;
    uint8_t slot_contention_window_size;
    uint8_t group_num;
    uint8_t group_rx_enabled;
    uint8_t group_tx_enabled;
    uint8_t group_periodicity;
    uint8_t group_offset;
    uint8_t join_response_code;
    uint8_t device_profile;
    uint8_t rx_window_count;
    uint8_t link_sync_interval;
    uint16_t rx_window_interval;
    uint16_t validity_period_sec;
} halo_hall_appv1_mgm_join_proc_rand_keys_t;

typedef struct {
    uint8_t rx_window_count;
    uint16_t validity_period_sec;
} halo_hall_appv1_mgm_wan_join_proc_params_t;

typedef struct {
    uint8_t device_profile_info;
    uint32_t join_proc_keys_mask;
    halo_hall_appv1_mgm_join_proc_rand_keys_t rand_keys;
} halo_hall_appv1_mgm_join_proc_key_t;

typedef struct {
    struct sid_address ep_addr;
    uint8_t status_code;
    uint32_t ll_seqn;
    uint8_t phy_mode;
} halo_hall_appv1_mgm_nack_t;

/* enum for tags for temporary protocol record */
typedef enum {
    MGM_PRTCL_EXP_TAG_LORA_LI_MODE = 0
} mgm_prtcl_exp_tag_t;

typedef struct {
    struct tl_in_tlv tl_in_tlv;
    uint8_t val;
} mgm_config_tlv_frmt_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_PUBLIC_HALO_HALL_APPV1_MGM_TYPES_H_ */
