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

#ifndef SID_COVERAGE_API_H
#define SID_COVERAGE_API_H

#include <sid_api.h>

#include <sid_network_address.h>
#include <sid_time_ops.h>

#ifdef __cplusplus
extern "C" {
#endif

enum sid_coverage_test_direction_type {
    SID_COVERAGE_TEST_UNIDIRECTIONAL = 0x00,
    SID_COVERAGE_TEST_BIDRECTIONAL = 0x01
};

enum sid_coverage_test_role {
    SID_COVERAGE_TEST_ROLE_SNIFFER = 0x00,
    SID_COVERAGE_TEST_ROLE_RECEIVER = 0x01,
    SID_COVERAGE_TEST_ROLE_TRANSMITTER = 0x02,
};

enum sid_coverage_test_data_rate {
    SID_COVERAGE_TEST_DATA_RATE_HDR = 0x00,
    SID_COVERAGE_TEST_DATA_RATE_LDR = 0x01,
    SID_COVERAGE_TEST_DATA_RATE_AUTO = 0x03,
};

enum sid_coverage_msg_type {
    SID_COVERAGE_MSG_TYPE_SENT_PING = 0x00,
    SID_COVERAGE_MSG_TYPE_RECEIVED_PONG = 0x01,
    SID_COVERAGE_MSG_TYPE_SENT_SR = 0x2,
    SID_COVERAGE_MSG_TYPE_SENT_DELAY_PING = 0x03,
    SID_COVERAGE_MSG_TYPE_RECEIVED_DELAY_PONG = 0x04,
};

struct sid_coverage_ping_pong_msg {
    int8_t rssi;
    int8_t snr;
    uint8_t sequence_num;
    struct sid_msg payload;
    struct sid_timespec msg_time;
};

struct sid_coverage_sr_msg {
    struct sid_msg payload;
    struct sid_timespec msg_time;
};

struct sid_coverage_msg {
    enum sid_coverage_msg_type type;
    union {
        struct sid_coverage_ping_pong_msg received_pong;
        struct sid_coverage_ping_pong_msg sent_ping;
        struct sid_coverage_ping_pong_msg received_delay_pong;
        struct sid_coverage_ping_pong_msg sent_delay_ping;
        struct sid_coverage_sr_msg sent_sr;
    };
};

struct sid_coverage_test_results {
    uint32_t pong_count;
    uint32_t ping_count;
    uint32_t sr_count;
    float mean_rssi;
    float mean_snr;
    float std_rssi;     /* variance * N value for RSSI */
    float std_snr;      /* variance * N value for SNR */
    float mean_latency; /* in number of ticks */
    enum sid_coverage_test_data_rate data_rate;
    uint8_t rf_channel;
};

struct sid_coverage_event_callbacks {
    void *context;
    void (*get_ping_data_payload)(uint8_t *const data, size_t *data_size, void *context);
    void (*get_delay_ping_data_payload)(uint8_t *const data, size_t *data_size, void *context);
    void (*get_status_report_payload)(uint8_t *const data, size_t *data_size, void *context);
    void (*on_msg_xfer)(const struct sid_coverage_msg *const msg, void *context);
};

struct sid_coverage_config {
    const struct sid_coverage_event_callbacks *callbacks;
    uint16_t ping_interval_ms;
    uint32_t total_pings_to_send;
    uint16_t sr_interval_ms;
    uint32_t total_sr_to_send;
    struct sid_address remote;
    enum sid_coverage_test_direction_type direction;
    enum sid_coverage_test_role role;
    enum sid_coverage_test_data_rate rate;
};


sid_error_t sid_coverage_send_ping(struct sid_handle *handle);

sid_error_t sid_coverage_send_report(struct sid_handle *handle);

sid_error_t sid_coverage_init(struct sid_handle *handle, const struct sid_coverage_config *const config);

sid_error_t sid_coverage_deinit(struct sid_handle *handle);

sid_error_t sid_coverage_get_results(struct sid_handle *handle, struct sid_coverage_test_results *const results);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_COVERAGE_API_H */
