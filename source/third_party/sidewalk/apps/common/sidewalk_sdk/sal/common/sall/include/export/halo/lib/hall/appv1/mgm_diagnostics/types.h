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

#ifndef RNETC_PUBLIC_HALO_HALL_APPV1_MGM_DIAGNOSTICS_TYPES_H_
#define RNETC_PUBLIC_HALO_HALL_APPV1_MGM_DIAGNOSTICS_TYPES_H_

#include <halo/lib/hall/appv1/mgm/types.h>

#include <sid_network_address.h>
#include <sid_parser_utils.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ping
typedef struct {
    int8_t rssi;
    int8_t snr;
    uint8_t num;
    const uint8_t* custom_data;
    size_t custom_data_size;
} halo_hall_appv1_ping_t;

// LQ test

// data rate 0 = HDR, 1 = LDR, 2 = reserved, 3 = automatic.
typedef enum {
    HALO_HALL_APPV1_LQ_TEST_DATA_RATE_HDR = 0x00,
    HALO_HALL_APPV1_LQ_TEST_DATA_RATE_LDR = 0x01,
    HALO_HALL_APPV1_LQ_TEST_DATA_RATE_AUTO = 0x03,
} halo_hall_appv1_lq_test_data_rate;

// direction 0 = unidirectional, 1 = bidirectional
typedef enum {
    HALO_HALL_APPV1_LQ_TEST_TYPE_UNIDIRECTIONAL = 0x00,
    HALO_HALL_APPV1_LQ_TEST_TYPE_BIDIRECTIONAL = 0x01,
} halo_hall_appv1_lq_test_type;

// test role 0 = sniffer, 1 = receiver, 2 = transmitter
typedef enum {
    HALO_HALL_APPV1_LQ_TEST_ROLE_SNIFFER = 0x0,
    HALO_HALL_APPV1_LQ_TEST_ROLE_RECEIVER = 0x1,
    HALO_HALL_APPV1_LQ_TEST_ROLE_TRANSMITTER = 0x2
} halo_hall_appv1_lq_test_role;

/*
 *   BUDDY: |3b unused| |2b data rate| |2b test role| |1b direction|
 *          |2B transmit interval (ms)| |2B test duration (s)| |5B address||     1B RF channel|
 *   V1:    |5b unused               | |2b test role| |1b direction| |2B transmit interval (ms)|
 */
typedef struct {
    uint16_t interval_ms;
    uint16_t duration_s;
    halo_hall_appv1_lq_test_data_rate data_rate;
    halo_hall_appv1_lq_test_role role;
    halo_hall_appv1_lq_test_type direction;
    uint8_t rf_channel;
    uint8_t address[SID_ADDRESS_SIZE_MAX];
} halo_hall_appv1_lq_test_t;

/*|1B RF channel| |5B receiver address| |2B transmit interval (ms)| |2b data rate|
 *|1b direction| |5b output power| |2B total pkts| |1B success rate (%)|
 *|1B mean RSSI| |1B mean SNR| |1B std RSSI| |1B std SNR| |2B mean latency (ms)|*/
typedef struct {
    uint8_t receiver_address[SID_ADDRESS_SIZE_MAX];
    uint16_t mean_latency;
    uint16_t transmit_interval_ms;
    uint16_t total_pkts;
    halo_hall_appv1_lq_test_data_rate data_rate;
    halo_hall_appv1_lq_test_type direction;
    uint8_t rf_channel;
    uint8_t output_power;
    uint8_t success_rate;
    uint8_t mean_rssi;
    uint8_t mean_snr;
    uint8_t std_rssi;
    uint8_t std_snr;
} halo_hall_appv1_lq_test_results_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_PUBLIC_HALO_HALL_APPV1_MGM_DIAGNOSTICS_TYPES_H_ */
