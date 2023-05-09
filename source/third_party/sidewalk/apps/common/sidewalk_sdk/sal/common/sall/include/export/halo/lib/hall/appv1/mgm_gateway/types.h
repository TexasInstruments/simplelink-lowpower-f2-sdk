/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef HALO_LIB_HALL_INCLUDE_EXPORT_HALO_LIB_HALL_APPV1_MGM_GATEWAY_TYPES_H_
#define HALO_LIB_HALL_INCLUDE_EXPORT_HALO_LIB_HALL_APPV1_MGM_GATEWAY_TYPES_H_

#include <sid_network_address.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HALO_HALL_TIME_PROXY_ENC_KEY_LENGTH = 16,
} halo_hall_time_proxy_e;

typedef struct halo_hall_appv1_proxy_responce_time_s {
    struct sid_address endnode_id;
    uint8_t enc_key[HALO_HALL_TIME_PROXY_ENC_KEY_LENGTH];
    uint32_t rand_key;
} halo_hall_appv1_proxy_response_time_t;

typedef enum { HALO_HALL_MFG_PROXY_FLAG_SIZE = 4 } halo_hall_mfg_froxy_e;

typedef struct {
    uint8_t mfg_flag[HALO_HALL_MFG_PROXY_FLAG_SIZE];
} halo_hall_appv1_mfg_proxy_flag;

typedef enum {
    HALO_HALL_APPV1_MGM_FLAG_DISABLE = 0x0,
    HALO_HALL_APPV1_MGM_FLAG_ENABLE = 0x1,
    HALO_HALL_APPV1_MGM_FLAG_LAST__
} halo_hall_appv1_mgm_flag_t;

typedef enum {
    HALO_HALL_APPV1_MGM_NG_DISCOVERY_CHANNEL_LIST_SIZE = 16,
    HALO_HALL_APPV1_MGM_NG_DISCOVERY_CHANNEL_MAX_NUM = HALO_HALL_APPV1_MGM_NG_DISCOVERY_CHANNEL_LIST_SIZE - 1,
    HALO_HALL_APPV1_MGM_NG_DISCOVERY_CHANNEL_INVALID = HALO_HALL_APPV1_MGM_NG_DISCOVERY_CHANNEL_MAX_NUM,
} halo_hall_appv1_mgm_ng_discovery_prop_t;

typedef struct {
    uint8_t enable_discovery;
} halo_hall_appv1_mgm_ng_discovery_flags_t;

typedef enum {
    HALO_HALL_APPV1_MGM_NG_DISCOVERY_DATA_RATE_PREFERENCE_NO_PREFERENCE = 0x00,
    HALO_HALL_APPV1_MGM_NG_DISCOVERY_DATA_RATE_PREFERENCE_LDR_ONLY = 0x01,
    HALO_HALL_APPV1_MGM_NG_DISCOVERY_DATA_RATE_PREFERENCE_HDR_ONLY = 0x02
} halo_hall_appv1_mgm_ng_discovery_dr_preference_t;

typedef struct {
    uint32_t time_interval_sec;
    uint32_t time_offset_sec;
    uint8_t channel_list[HALO_HALL_APPV1_MGM_NG_DISCOVERY_CHANNEL_LIST_SIZE];
    uint8_t channel_count;
    halo_hall_appv1_mgm_ng_discovery_flags_t ctrl_flags;
    uint8_t data_rate_preference;
} halo_hall_appv1_mgm_ng_discovery_params_t;

typedef enum {
    // Channel list (with channel numbers 0..15) is mapped to 2-bytes bitfield
    HALO_HALL_APPV1_MGM_NG_DISCOVERY_SELIALIZED_DATA_SIZE =
        sizeof(halo_hall_appv1_mgm_ng_discovery_params_t) - sizeof(uint16_t) - HALO_HALL_APPV1_MGM_NG_DISCOVERY_CHANNEL_LIST_SIZE + sizeof(uint16_t),
} halo_hall_appv1_mgm_ng_discovery_buff_size;

typedef enum {
    HALO_HALL_APPV1_MGM_DEDICATED_LINK_ROLE_IN_REG_OPERATION = 0,
    HALO_HALL_APPV1_MGM_DEDICATED_LINK_ROLE_DISTRESS = 1,
    HALO_HALL_APPV1_MGM_DEDICATED_LINK_ROLE_ASSIST = 2,
} halo_hall_appv1_mgm_dedicated_link_role_t;

typedef struct {
    struct sid_address peer_address;
    uint8_t peer_device_class;
    uint8_t channel_id;
    uint8_t role;
} halo_hall_appv1_mgm_dedicated_link_request_t;

typedef enum {
    HALO_HALL_APPV1_MGM_DEDICATED_LINK_ACKNOWLEDGED_NACK = 0,
    HALO_HALL_APPV1_MGM_DEDICATED_LINK_ACKNOWLEDGED_ACK = 1,
} halo_hall_appv1_mgm_dedicated_link_acknowledged_t;

typedef struct {
    struct sid_address peer_address;
    uint8_t role;
    uint8_t acknowledged;
} halo_hall_appv1_mgm_dedicated_link_response_t;

typedef enum {
    HALO_HALL_APPV1_MGM_DEV_STATUS_ANNOUNCEMENT_GW_BACK_ONLINE = 0,
    HALO_HALL_APPV1_MGM_DEV_STATUS_ANNOUNCEMENT_GW_IN_DISTRESS = 1,
    HALO_HALL_APPV1_MGM_DEV_STATUS_ANNOUNCEMENT_START_FTP_SESSION = 2,
    HALO_HALL_APPV1_MGM_DEV_STATUS_ANNOUNCEMENT_STOP_FTP_SESSION = 3,
    HALO_HALL_APPV1_MGM_DEV_STATUS_ANNOUNCEMENT_DEDICATED_LINK_READY = 4,
    HALO_HALL_APPV1_MGM_DEV_STATUS_ANNOUNCEMENT_DEDICATED_LINK_SETUP_FAILED = 5,
    HALO_HALL_APPV1_MGM_DEV_STATUS_ANNOUNCEMENT_DEDICATED_LINK_TERMINATE = 6
} halo_hall_appv1_mgm_dev_status_announcement_t;

typedef struct {
    uint8_t format;
    uint8_t enable_discovery;
    uint8_t random_delay_offset_sec;
    uint8_t msg_max_delay_before_ack_min;
    uint8_t msg_max_delay_after_ack_min;
} halo_hall_appv1_mgm_gw_distress_discovery_param_t;

// Cloud Auth token

typedef enum {
    HALO_HALL_APPV1_CLOUD_AUTH_TOKEN_SIZE = 37
} halo_hall_appv1_cloud_auth_token;

typedef struct {
    uint8_t token[HALO_HALL_APPV1_CLOUD_AUTH_TOKEN_SIZE];
} halo_hall_appv1_cloud_auth_token_t;

typedef enum {
    HALO_HALL_APPV1_MAC_LENGTH = 17,
} halo_hall_appv1_mac_length_t;

typedef struct {
    uint8_t mac[HALO_HALL_APPV1_MAC_LENGTH];
} halo_hall_appv1_mac_t;

//Hardware Id

typedef enum {
    HALO_HALL_APPV1_HW_ID_STATUS_UNDEFINED = ~0,
} halo_hall_appv1_hw_id_status_e;

typedef enum {
    HALO_HALL_APPV1_HW_ID_FORMAT_TYPE_0 = 0,
} halo_hall_appv1_hw_id_format_types_e;

typedef struct {
    uint8_t format_type;
    uint8_t hw_id;
} halo_hall_appv1_hw_id_info_t;

#ifdef __cplusplus
} // extern "C" {
#endif

#endif /* HALO_LIB_HALL_INCLUDE_EXPORT_HALO_LIB_HALL_APPV1_MGM_GATEWAY_TYPES_H_ */
