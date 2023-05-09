/*
 * Copyright 2021-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_900_TYPES_H
#define SID_900_TYPES_H

#include <halo/lib/hall/appv1/mgm/types.h>
#include <halo/lib/hall/appv1/message.h>

#include <sid_network_type.h>
#include <sid_time_types.h>

#include <stdint.h>

#define SAL_METRICS_REQUEST_BUFFER_SIZE_MAX 128

#define DEDICATED_LINK_P2P_UNICAST_KEY_LEN 16

#ifdef __cplusplus
extern "C" {
#endif

// Todo: Add an assert to make sure "RNET_MAC_SLOT_DURATION_MS" defined in "rnet_mac_submodule_common.h"
// is same as "MAC_SLOT_DURATION_MS"
// https://issues.labcollab.net/browse/HALO-19630
#define MAC_SLOT_DURATION_MS 63

typedef struct {
    uint8_t dflogic;
    uint8_t probe_auth;
} sid_ni_900_mac_switch_t;

typedef struct {
    uint8_t unicast_periodicity;
    uint8_t group_periodicity;
} sid_ni_900_hdr_param_t;

typedef struct {
    uint8_t config_vers;
    uint8_t prtcl_ver;
    uint8_t prtcl_flag;
} sid_ni_900_protocol_version_t;

typedef struct {
    uint8_t config_vers;
    uint8_t prtcl_ver;
    uint8_t ldr_channel;
} sid_ni_900_ldr_channel_t;

typedef struct {
    halo_hall_appv1_dev_profile_t dev_profile;
    uint8_t config_vers;
    uint8_t prtcl_ver;
} sid_ni_900_device_version_t;

struct sid_ni_900_dev_cls_fsk {
    uint8_t dev_profile_id;
    uint8_t rx_count;
};

struct sid_ni_900_dev_cls_ldr {
    uint8_t dev_profile_id;
    uint8_t rx_count;
  uint16_t ldr_prdcty;
};

struct sid_ni_900_tmp_prtcl_exp {
    uint8_t li_mode;
    uint8_t config_vers;
    uint8_t prtcl_ver;
};

typedef struct {
    uint16_t   pan_scan_interval;
    halo_hall_appv1_hdr_param_t  hdr_param;
    sid_ni_900_mac_switch_t mac_switch;
    sid_ni_900_protocol_version_t protocol_version;
    halo_hall_appv1_protocol_flags_t protocol_switch;
    sid_ni_900_ldr_channel_t ldr_channel;
    uint8_t sar_dcr;
    int8_t rf_power[HALO_HALL_APPV1_MAX_DATA_RATES];
    uint8_t max_missed_beacons;
    uint8_t beacons_interval;
    sid_ni_900_device_version_t device_version;
    struct sid_ni_900_dev_cls_fsk dev_cls_fsk_params;
    struct sid_ni_900_dev_cls_ldr dev_cls_ldr_params;
    struct sid_ni_900_tmp_prtcl_exp tmp_prtcl_exp;
} sid_ni_900_internal_config_t;

typedef enum {
    DEDICATED_LINK_INACTIVE = 0,
    DEDICATED_LINK_SETUP = 1,
    DEDICATED_LINK_ESTABLISHED = 2,
} dedicated_link_state_e;

typedef struct {
    uint8_t key[DEDICATED_LINK_P2P_UNICAST_KEY_LEN];
    uint8_t type;
} halo_hall_appv1_p2p_unicast_key_t;

typedef struct {
    uint8_t dedicated_link_state;
    halo_hall_appv1_p2p_unicast_key_t p2p_key;
    halo_hall_appv1_mgm_dedicated_link_request_t request;
} dedicated_link_params_t;

typedef struct {
    uint8_t gw_consent_flag;
} sid_ni_900_gw_config_t;

typedef struct {
    sid_ni_config_t config_core;
    sid_ni_900_internal_config_t config_900;
    sid_ni_900_gw_config_t config_gw;
} sid_ni_900_config_t;

enum network_interface_mode_id {
    MAC_HDR_FSK_PAN = 0,
    MAC_LDR_WAN,
    MAC_HDR_FSK_WAN_ADHOC,
    MAC_HDR_FSK_WAN_ALLOCATED,
    MAC_MODE_ID_LAST,
};

typedef enum {
    SID_NI_900_METRIC_KEY_P2P = 0x00,
    SID_NI_900_METRIC_KEY_EP2P = 0x01,
    SID_NI_900_METRIC_KEY_SCHED = 0x02,
    SID_NI_900_METRIC_KEY_NOISE = 0x03,
    SID_NI_900_METRIC_KEY_ZWAVE= 0x04,
    SID_NI_900_METRIC_KEY_LDR = 0x05,
    SID_NI_900_METRIC_KEY_EN_LDR_PHY_IMP1 = 0x06,
    SID_NI_900_METRIC_KEY_GW_LDR_PHY_IMP = 0x07,
    SID_NI_900_METRIC_KEY_CLEAR_METRICS = 0x8
} sid_ni_900_metric_key_t;

enum sid_ni_900_nw_sync_event {
    SID_NI_900_NW_SYNC_EVENT_FIRST = 0x01,
    SID_NI_900_NW_SYNC_EVENT_RSP = SID_NI_900_NW_SYNC_EVENT_FIRST,
    SID_NI_900_NW_SYNC_EVENT_TERM,
    SID_NI_900_NW_SYNC_EVENT_NTFY,
    SID_NI_900_NW_SYNC_EVENT_LAST = SID_NI_900_NW_SYNC_EVENT_NTFY
};

enum sid_ni_900_dev_param_sync_status {
    SID_NI_900_DEV_PARAM_SYNC_NONE = 0x0,
    SID_NI_900_DEV_PARAM_SYNC_IN_PROGRESS = 0x1,
    SID_NI_900_DEV_PARAM_SYNC_DONE = 0x2,
    SID_NI_900_DEV_PARAM_SYNC_TERMINATE = 0x3,
    SID_NI_900_DEV_PARAM_SYNC_STATUS_LAST
};

enum sid_ni_900_fsk_wan_event {
    SID_NI_900_FSK_WAN_EVENT_AUTH_RESP,
    SID_NI_900_FSK_WAN_EVENT_SCH_RESP,
};

typedef struct sid_ni_900_fsk_wan_info_s {
    enum sid_ni_900_fsk_wan_event event;
    halo_hall_appv1_mgm_join_proc_key_t *data;
} sid_ni_900_fsk_wan_info_t;

typedef struct ni_metrics_request_s {
    uint32_t key;
    size_t size;
    uint8_t clear;
    uint8_t buffer[SAL_METRICS_REQUEST_BUFFER_SIZE_MAX];
    bool uplink_900_link;
} sid_ni_900_metrics_request_t;

typedef struct ni_metrics_rollover_handle_s {
    void (*on_rollover)(void *ctx);
    void *ctx;
} sid_ni_900_metrics_rollover_handle_t;

struct time_proxy_notify_info_s {
    struct sid_address cloud_id;
    uint8_t enc_key[HALO_HALL_TIME_PROXY_ENC_KEY_LENGTH];
    uint32_t ll_seqn;
    uint32_t epoch_sec;
    uint32_t epoch_nsec;
    uint32_t sec_ref_counter;
    uint8_t phy_mode;
    uint8_t rf_frame_proto_version;
};

// Events for SDB MAC Module
enum sid_sdb_gw_event {
    SID_SDB_GW_EVENT_DISCOVERING = 0x00,
    SID_SDB_GW_EVENT_CONNECTING = 0x01,
    SID_SDB_GW_EVENT_CONNECTED = 0x02,
    SID_SDB_GW_EVENT_DISCONNECTED = 0x03,
};

// Structure for sending events and parameters for SDB MAC Module
struct sid_sdb_gw_event_info_s {
    enum sid_sdb_gw_event event;
    void *data;
};

struct sid_ni_900_tx_prepare_data_s {
    uint32_t duration_ms;
    uint16_t cmd_class_id;
    uint16_t cmd_id;
    uint8_t *payload;
    uint8_t payload_size;
};

/**
 * Possible packet event types
 */
typedef enum {
    SID_NI_900_PKT_EVENT_TX_ERROR,    /**< TX failed      */
    SID_NI_900_PKT_EVENT_TX_SUCCESS,  /**< TX success     */
    SID_NI_900_PKT_EVENT_RX,          /**< RX             */
    SID_NI_900_PKT_EVENT_TX_START,    /**< TX start       */
    SID_NI_900_PKT_EVENT_UNKNOWN,     /**< Unknown event  */
} sid_ni_900_pkt_event;

typedef struct {
    struct sid_timespec start;
    uint32_t duration_ms;
    sid_ni_900_pkt_event pkt_event;
}sid_ni_900_radio_tx_info_t;

struct sid_ni_900_join_node_info {
    uint8_t *node_adr;
    const halo_hall_appv1_mgm_join_proc_key_t *param;
};

_Static_assert(HALO_HALL_MSG_INFO_TYPE_TIME_PROXY_CTX_SIZE == sizeof(struct time_proxy_notify_info_s),
              "HALO_HALL_MSG_INFO_TYPE_TIME_PROXY_CTX_SIZE must be equal to sizeof(struct time_proxy_notify_info_s)");

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* SID_900_TYPES_H */
