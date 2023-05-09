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

#ifndef RNETC_PUBLIC_HALO_HALL_APPV1_NETWORK_COVERAGE_DIAG_TYPES_H_
#define RNETC_PUBLIC_HALO_HALL_APPV1_NETWORK_COVERAGE_DIAG_TYPES_H_

#include <sid_network_address.h>
#include <sid_parser_utils.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t horizontal_accuracy;
    uint32_t latitude;
    uint32_t longitude;
    uint16_t altitude;
    uint32_t timestamp;
} halo_hall_appv1_network_gps_t;

// ping
typedef struct {
    uint8_t num;
    int8_t rssi;
    int8_t snr;
    bool gps_data_available;
    halo_hall_appv1_network_gps_t data;
} halo_hall_appv1_network_ping_t;

// pong
typedef struct {
    uint8_t num;
    int8_t rssi;
    int8_t snr;
} halo_hall_appv1_network_pong_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_PUBLIC_HALO_HALL_APPV1_NETWORK_COVERAGE_DIAG_TYPES_H_ */
