/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef SID_NETWORK_COVERAGE_API_H
#define SID_NETWORK_COVERAGE_API_H

#include <sid_api.h>
#include <sid_time_ops.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SID_NETWORK_COVERAGE_ADDRESS_SIZE 7

// link type 0 = LoRA-2.5kbps, 1 = FSK-50kbps, 2 = BLE, 3-13 = RESERVED, 14 = AUTO_SUB-GHZ, 15; = AUTO_ALL_PHY.
enum sid_network_test_link_type {
    SID_NETWORK_TEST_LINK_TYPE_LORA = 0x00,
    SID_NETWORK_TEST_LINK_TYPE_FSK = 0x01,
    SID_NETWORK_TEST_LINK_TYPE_BLE = 0x02,
    SID_NETWORK_TEST_LINK_TYPE_AUTO_SUB_GHZ = 0x0E,
    SID_NETWORK_TEST_LINK_TYPE_AUTO_ALL_PHY = 0x0F,
};

// test type
// 0 = LQ test summary report only
// 1 = LQ test coverage test
// 2 = Raw packet sniffing
// 3 - 7 = RESERVED
enum sid_network_test_type {
    SID_NETWORK_TEST_TYPE_REPORT_ONLY = 0x00,
    SID_NETWORK_TEST_TYPE_COVERAGE_TEST = 0x01,
    SID_NETWORK_TEST_TYPE_SNIFFER = 0x02,
};

// direction 0 = unidirectional, 1 = bidirectional
enum sid_network_test_direction {
    SID_NETWORK_TEST_TYPE_UNIDIRECTIONAL = 0x00,
    SID_NETWORK_TEST_TYPE_BIDIRECTIONAL = 0x01,
};

// test role 0 = receiver, 1 = transmitter, 2 - 3 = RESERVED
enum sid_network_test_role {
    SID_NETWORK_TEST_ROLE_RECEIVER = 0x0,
    SID_NETWORK_TEST_ROLE_TRANSMITTER = 0x1,
};

// consent 0 = no consent to share gps information, 1 = consent to share gps information
enum sid_network_test_consent{
    SID_NETWORK_TEST_NO_CONSENT = 0x0,
    SID_NETWORK_TEST_CONSENT = 0x1,
};

// transmit interval 0 = ms, 1 = secs, 2 = minutes, 3 = hours
enum sid_network_test_transmit_interval {
    SID_NETWORK_TEST_TI_MS = 0x00,
    SID_NETWORK_TEST_TI_SEC = 0x01,
    SID_NETWORK_TEST_TI_MIN = 0x02,
    SID_NETWORK_TEST_TI_HOUR = 0x03,
};

// format specifier 0 = ping_pong_format, 1 = sniffer format, 2 - 15 = RESERVED
enum sid_network_tx_rx_format_specifier {
    SID_NETWORK_TEST_PING_PONG_FORMAT = 0x0,
    SID_NETWORK_TEST_SNIFFER_FORMAT = 0x1,
};

struct sid_network_gps_data {
    uint32_t latitude;
    uint32_t longitude;
    uint32_t timestamp;
    uint16_t altitude;
    uint8_t horizontal_accuracy;
};

struct sid_network_ping_pong_msg {
    int8_t rssi;
    int8_t snr;
    uint8_t sequence_num;
    bool data_available;
    struct sid_network_gps_data data;
};

struct sid_network_event_callbacks {
    void *context;
    void (*get_gps_data_payload)(struct sid_network_gps_data *data, bool *available, void *context);
};

struct sid_network_coverage_config_callback {
    const struct sid_network_event_callbacks *callbacks;
};

struct sid_network_coverage_config {
    uint16_t ping_interval_s;
    uint16_t duration_s;
    enum sid_network_test_type test_type;
    enum sid_network_test_link_type link;
    enum sid_network_test_role role;
    enum sid_network_test_consent consent;
    enum sid_network_test_direction direction;
    uint8_t address[SID_NETWORK_COVERAGE_ADDRESS_SIZE];
};

sid_error_t sid_network_coverage_set_callback(struct sid_handle *handle, const struct sid_network_coverage_config_callback *const cb);

sid_error_t sid_network_coverage_start_test(struct sid_handle *handle, const struct sid_network_coverage_config *const config);

sid_error_t sid_network_coverage_stop_test(struct sid_handle *handle);

bool sid_network_coverage_is_test_running(struct sid_handle *handle);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_NETWORK_COVERAGE_API_H */
