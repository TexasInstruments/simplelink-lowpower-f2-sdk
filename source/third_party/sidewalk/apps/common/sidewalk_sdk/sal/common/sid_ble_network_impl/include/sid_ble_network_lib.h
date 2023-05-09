/*
 * Copyright 2021-2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_BLE_NETWORK_LIB_H
#define SID_BLE_NETWORK_LIB_H

#include <sid_ble_network_ifc.h>
#include <sid_ama_transport_ifc.h>

#include <sid_network_control_ifc.h>
#include <sid_network_data_ifc.h>

#include <sid_flex_parser.h>
#include <sid_asl_parser.h>
#include <sid_asl_types.h>
#include <sid_event_queue_ifc.h>

#include <sid_pal_timer_ifc.h>

#include <sid_error.h>

#include <lk/list.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_LL_SEQN 0x3FFFFF
#define MAX_COUNTER_VALUE 0x7FFFFFFF
#define MAX_DATA_PAYLOAD_SIZE 326   // 255 payload + 7B header + 64B flex headers

#define MERGE_CMD_ID_OP_CODE(x_, y_) (x_ << 2) | (y_ & 0x3)
#define RESP_CMD_TYPE 3
#define NW_DATA_BLOB_SIZE 32
#define ADDITIONAL_APP_DATA_SIZE 64

#define MAX_MANU_SPECIFIC_DATA_SIZE 20
#define MAX_TX_ID_SIZE 5
#define MAX_PAYLOAD_SIZE (MAX_MANU_SPECIFIC_DATA_SIZE - MAX_TX_ID_SIZE - 3)
#define MIN_ADV_SIZE 8
#define BLE_TYPE 1

#define NORMAL_MODE 0
#define OOBE_MODE 1
#define CONNECTION_REQUEST_MODE 2
#define RESERVED_MODE 3

#define MAX_BATTERY_LEVEL 100
#define CRITICAL_BATTERY_RANGE 10
#define LOW_BATTERY_RANGE 50
#define CRITICAL_BATTERY_LVL 0
#define LOW_BATTERY_LVL 1
#define NORMAL_BATTERY_LVL 2
#define BATTERY_RESERVED 3

#if UNIFIED_ASL_SUPPORT
#define ADVERTISEMENT_VERSION 1
#else
#define ADVERTISEMENT_VERSION 0
#endif
#define SID_EDC_SERVER 0x06   // Sidewalk Edge Device Connection
#define SID_TIME_SYNC_SERVER 0x03
#define SID_REGISTRATION_SERVER 0x04

#define TX_ID_TYPE_TIME_SYNC 0x0
#define TX_ID_TYPE_COUNTER 0x1
#define TX_ID_TYPE_SMSN_SIDEWALK_ID 0x2
#define PAYLOAD_DATA_INDICATOR 0x0
#define PAYLOAD_DATA_INDICATE_AUTHENTICATED_TIME_SYNC_SUPPORTED 0x02

#if UNIFIED_ASL_SUPPORT
#define UNIFIED_ASL_VERSION 0
#endif

#define ADV_ROTATION_TIMEOUT_SEC 900

#define BLE_MAX_DATA_LEN 247
#define BLE_MAX_BUFFERS 4
#define BLE_MTU_SIZE 255

typedef sid_error_t (*sid_ble_data_send_cb_t)(void *ctx, uint8_t *data, uint16_t length);

enum ble_error_type {
    BLE_ERROR_DATA_INVALID = 0,
    BLE_ERROR_EVENT_OOM = 1,
};

union beacon_state {
    struct {
        uint8_t device_type : 1;
        uint8_t device_mode : 2;
        uint8_t battery_lvl : 2;
        uint8_t adv_version : 3;
    };
    uint8_t value;
};

#if UNIFIED_ASL_SUPPORT
union frame_indicator {
    struct {
        uint8_t data_indicator : 4;
        uint8_t asl_version : 2;
        uint8_t tx_id_format : 2;
    };
    uint8_t value;
};
#else
union frame_indicator {
    struct {
        uint8_t data_indicator : 6;
        uint8_t tx_id_format : 2;
    };
    uint8_t value;
};
#endif

union ble_beacon {
    struct {
        uint8_t application_id;
        union beacon_state state;
        union frame_indicator indicator;
        uint8_t tx_id[MAX_TX_ID_SIZE];
        uint8_t payload[MAX_PAYLOAD_SIZE];
    };
    uint8_t value[MAX_MANU_SPECIFIC_DATA_SIZE];
};

enum ble_control_event {
    BLE_CONNECTION_EVENT = 1,
    BLE_INDICATION_EVENT = 2,
    BLE_NOTIFY_EVENT = 3,
    BLE_DATA_EVENT = 4,
    BLE_ERROR_EVENT = 5,
};

struct ble_conn {
    bool connected;
    uint8_t bt_addr[BLE_ADDR_MAX_LEN];
};

struct ble_ind {
    bool status;
};

struct ble_notify {
    sid_ble_cfg_service_identifier_t id;
    bool state;
};

struct ble_data {
    sid_ble_cfg_service_identifier_t id;
    uint16_t len;
    uint8_t buf[];
};

struct ble_error {
    enum ble_error_type type;
    enum ble_control_event event;
};

struct ble_msg {
    enum ble_control_event evt;
    union {
        struct ble_conn conn_state;
        struct ble_ind indication;
        struct ble_notify notify;
        struct ble_data data_msg;
        struct ble_error error;
    };
};

struct ble_periodic_timesync {
    sid_pal_timer_t timer;
    uint32_t interval;
    uint32_t duration;
    bool timer_expired;
    bool adv_expired;
};

struct ble_msg_event {
    list_node_t node;
    struct sid_event event;
    void *context;
    struct ble_msg msg;
};

struct ble_msg_buffer {
    uint8_t buf[sizeof(struct ble_msg_event) + BLE_MAX_DATA_LEN] __attribute__((aligned));
};

struct ble_network_interface_impl {
    uint8_t application_id;
    uint8_t device_mode;
    uint16_t mtu_size;
    uint16_t metric;
    uint16_t class_id;
    uint16_t cmd_id;
    uint8_t cmd_type;
    uint32_t send_ll_seqn;
    uint32_t send_counter_value;
    uint32_t tx_msg_id;
    uint32_t tx_id_reference;
    uint32_t connection_request_timeout_ms;
    uint16_t app_pld_size;
    bool is_advertising;
    bool is_connected;
    bool adv_timer_expired;
    bool inactivity_timer_expired;
    bool started;
    bool pending_tx_event;
    bool is_mobile_gateway;
    enum sid_ama_transport_state transport_state;
    struct sid_address local;
    struct sid_address remote;
    uint8_t send_buf[MAX_DATA_PAYLOAD_SIZE];
    uint8_t receive_buf[MAX_DATA_PAYLOAD_SIZE];
    struct sid_flex flex;
    uint8_t scratch[MAX_DATA_PAYLOAD_SIZE];
    struct app_pl_frame_deserialized_buffer deserialize;
    struct app_pl_frame_serialized_buffer serialize;
    list_node_t msg_event_list;
    list_node_t input_msg_queue;
    list_node_t output_msg_queue;
    network_interface_ifc ifc;
    const network_interface_control_notify_ifc *ctrl_ntfy_interface;
    struct sid_timespec connect_time;
    sid_pal_timer_t adv_timer;
    sid_pal_timer_t inactivity_timer;
    bool disable_inactivity_timer;
    struct ble_periodic_timesync periodic_timesync;
    struct sid_event timer_expired_event;
    struct sid_deferred_event connection_request_timeout_event;
    const network_interface_data_notify_ifc *ntfy;
    const halo_hall_resource_manager_ifc *resource_manager;
    sid_pal_ble_adapter_interface_t adapter_ifc;
    union ble_beacon adv_data;
    struct ble_msg_buffer msg_buffers[BLE_MAX_BUFFERS];
    struct ble_msg_buffer error_buffer;
    const struct sid_event_queue *event_queue;
    struct sid_link_statistics stats;
};

sid_error_t call_on_received_ble_message(struct ble_network_interface_impl *const impl,
                                         const uint8_t *const data, uint16_t length);
void call_on_sent_ble_message(struct ble_network_interface_impl *const impl, sid_error_t status);

sid_error_t send_ble_message(struct ble_network_interface_impl *const impl,
                             const uint8_t *const data, const uint16_t length);

sid_error_t ble_network_interface_control_init(
    struct ble_network_interface_impl *const impl,
    const struct sid_ble_network_interface_config *const cfg,
    const network_interface_control_notify_ifc *const ctrl_ntfy_interface);

void ble_network_interface_data_init(struct ble_network_interface_impl *const impl);
void ble_free_output_messsage(struct ble_network_interface_impl *const impl);

#ifdef __cplusplus
}
#endif

#endif /*! SID_BLE_NETWORK_LIB_H */
