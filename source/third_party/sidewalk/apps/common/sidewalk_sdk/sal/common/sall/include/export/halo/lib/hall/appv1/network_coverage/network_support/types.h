/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef RNETC_PUBLIC_HALO_HALL_APPV1_NETWORK_COVERAGE_SUPPORT_TYPES_H_
#define RNETC_PUBLIC_HALO_HALL_APPV1_NETWORK_COVERAGE_SUPPORT_TYPES_H_

#include <sid_network_address.h>
#include <sid_parser_utils.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Network test

// link type 0 = LoRA-2.5kbps, 1 = FSK-50kbps, 2 = BLE, 3-13 = RESERVED, 14 = AUTO_SUB-GHZ, 15; = AUTO_ALL_PHY.
typedef enum {
    HALO_HALL_APPV1_NETWORK_TEST_LINK_TYPE_LORA = 0x00,
    HALO_HALL_APPV1_NETWORK_TEST_LINK_TYPE_FSK = 0x01,
    HALO_HALL_APPV1_NETWORK_TEST_LINK_TYPE_BLE = 0x02,
    HALO_HALL_APPV1_NETWORK_TEST_LINK_TYPE_AUTO_SUB_GHZ = 0x0E,
    HALO_HALL_APPV1_NETWORK_TEST_LINK_TYPE_AUTO_ALL_PHY = 0x0F,
} halo_hall_appv1_network_test_link_type;

// test type
// 0 = LQ test summary report only
// 1 = LQ test coverage test
// 2 = Raw packet sniffing
// 3 - 7 = RESERVED
typedef enum {
    HALO_HALL_APPV1_NETWORK_TEST_TYPE_REPORT_ONLY = 0x00,
    HALO_HALL_APPV1_NETWORK_TEST_TYPE_COVERAGE_TEST = 0x01,
    HALO_HALL_APPV1_NETWORK_TEST_TYPE_SNIFFER = 0x02,
    HALO_HALL_APPV1_NETWORK_TEST_TYPE_MAX = 0x07,
} halo_hall_appv1_network_test_type;

// direction 0 = unidirectional, 1 = bidirectional
typedef enum {
    HALO_HALL_APPV1_NETWORK_TEST_TYPE_UNIDIRECTIONAL = 0x00,
    HALO_HALL_APPV1_NETWORK_TEST_TYPE_BIDIRECTIONAL = 0x01,
} halo_hall_appv1_network_test_direction;

// test role 0 = receiver, 1 = transmitter, 2 - 3 = RESERVED
typedef enum {
    HALO_HALL_APPV1_NETWORK_TEST_ROLE_RECEIVER = 0x0,
    HALO_HALL_APPV1_NETWORK_TEST_ROLE_TRANSMITTER = 0x1,
} halo_hall_appv1_network_test_role;

// consent 0 = no consent to share gps information, 1 = consent to share gps information
typedef enum {
    HALO_HALL_APPV1_NETWORK_TEST_NO_CONSENT = 0x0,
    HALO_HALL_APPV1_NETWORK_TEST_CONSENT = 0x1,
} halo_hall_appv1_network_test_consent;

// transmit interval 0 = ms, 1 = secs, 2 = minutes, 3 = hours
typedef enum {
    HALO_HALL_APPV1_NETWORK_TEST_TI_MS = 0x00,
    HALO_HALL_APPV1_NETWORK_TEST_TI_SEC = 0x01,
    HALO_HALL_APPV1_NETWORK_TEST_TI_MIN = 0x02,
    HALO_HALL_APPV1_NETWORK_TEST_TI_HOUR = 0x03,
} halo_hall_appv1_network_test_transmit_interval;

// format specifier 0 = ping_pong_format, 1 = sniffer format, 2 - 15 = RESERVED
typedef enum {
    HALO_HALL_APPV1_NETWORK_TEST_PING_PONG_FORMAT = 0x0,
    HALO_HALL_APPV1_NETWORK_TEST_SNIFFER_FORMAT = 0x1,
} halo_hall_appv1_network_test_tx_rx_format_specifier;

typedef struct {
    uint16_t transmit_interval;
    uint16_t duration_s;
    halo_hall_appv1_network_test_transmit_interval ti_unit;
    halo_hall_appv1_network_test_type test_type;
    halo_hall_appv1_network_test_link_type link;
    halo_hall_appv1_network_test_role role;
    halo_hall_appv1_network_test_consent consent;
    halo_hall_appv1_network_test_direction direction;
    uint8_t address[SID_ADDRESS_SIZE_MAX];
} halo_hall_appv1_start_network_test_t;

typedef struct {
    halo_hall_appv1_network_test_type test_type;
    uint16_t remaining_duration;
} halo_hall_appv1_notify_network_test_t;

typedef struct {
    halo_hall_appv1_network_test_type test_type;
} halo_hall_appv1_stop_network_test_t;

typedef struct {
    halo_hall_appv1_network_test_link_type link;
    uint16_t mean_latency;
    uint16_t total_pkts;
    uint16_t total_pongs_rcvd;
    uint8_t output_power;
    uint8_t mean_rssi;
    uint8_t mean_snr;
    uint8_t std_rssi;
    uint8_t std_snr;
    uint8_t receiver_address[SID_ADDRESS_SIZE_MAX];
} halo_hall_appv1_network_test_results_t;

typedef struct {
    halo_hall_appv1_network_test_tx_rx_format_specifier specifier;
    halo_hall_appv1_network_test_link_type link;
    uint8_t snr;
    uint8_t rssi;
    uint8_t ping_num;
    uint32_t gps_time;
    uint8_t ms_unit;
    uint8_t raw_packet_length;
} halo_hall_appv1_rx_event_notify_t;

typedef struct {
    halo_hall_appv1_network_test_tx_rx_format_specifier specifier;
    halo_hall_appv1_network_test_link_type link;
    uint8_t tx_power;
    uint8_t ping_num;
    uint32_t gps_time;
    uint8_t ms_unit;
    uint8_t raw_packet_length;
} halo_hall_appv1_tx_event_notify_t;

typedef struct {
    uint8_t ping_num;
} halo_hall_appv1_network_missed_pong_notify_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_PUBLIC_HALO_HALL_APPV1_NETWORK_COVERAGE_SUPPORT_TYPES_H_ */
