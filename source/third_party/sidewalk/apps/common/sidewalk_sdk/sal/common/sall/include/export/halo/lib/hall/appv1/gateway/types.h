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

#ifndef HALO_ALL_APPV1_GATEWAY_TYPES_H_
#define HALO_ALL_APPV1_GATEWAY_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include <halo/lib/hall/platform.h>
#include "halo/lib/hall/appv1/mgm/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HALO_ALL_GATEWAY_ZWAVE_PARAMS 12
#define HALO_ALL_GATEWAY_MAX_PKT_SIZE 0xFF

#define HALO_HALL_APPV1_GATEWAY_SCH_RESP_FULL_MASK (\
        (1 << HALO_HALL_APPV1_MGM_FSK_WAN_JOIN_RESPONSE_CODE) | (1 << HALO_HALL_APPV1_MGM_SYNCED_GW) | \
        (1 << HALO_HALL_APPV1_MGM_UL_SCHED_PROF_3) | (1 << HALO_HALL_APPV1_MGM_DL_SCHED_PROF_3) | \
        (1 << HALO_HALL_APPV1_MGM_UL_CONTENTION_CHAR) | (1 << HALO_HALL_APPV1_MGM_LINK_DURATION))

#define HALO_HALL_APPV1_GATEWAY_SCH_RESP_REJECT_FULL_MASK (\
    (1 << HALO_HALL_APPV1_MGM_FSK_WAN_JOIN_RESPONSE_CODE) | (1 << HALO_HALL_APPV1_MGM_SYNCED_GW))

#define HALO_HALL_APPV1_GATEWAY_SCH_RESP_MIN_MASK HALO_HALL_APPV1_GATEWAY_SCH_RESP_REJECT_FULL_MASK

#define HALO_HALL_APPV1_GATEWAY_SLOT_ASGMT_RESP_FULL_MASK (\
    (1 << HALO_HALL_APPV1_MGM_FSK_WAN_JOIN_RESPONSE_CODE) | (1 << HALO_HALL_APPV1_MGM_SYNCED_GW) | \
    (1 << HALO_HALL_APPV1_MGM_UL_SCHED_PROF_3) | (1 << HALO_HALL_APPV1_MGM_DL_SCHED_PROF_3) | \
    (1 << HALO_HALL_APPV1_MGM_UL_CONTENTION_CHAR) | (1 << HALO_HALL_APPV1_MGM_LINK_DURATION))

#define HALO_HALL_APPV1_GATEWAY_SLOT_ASGMT_RESP_REJECT_FULL_MASK (\
    (1 << HALO_HALL_APPV1_MGM_FSK_WAN_JOIN_RESPONSE_CODE) | (1 << HALO_HALL_APPV1_MGM_SYNCED_GW))

#define HALO_HALL_APPV1_GATEWAY_SLOT_ASGMT_RESP_MIN_MASK HALO_HALL_APPV1_GATEWAY_SLOT_ASGMT_RESP_REJECT_FULL_MASK

#define HALO_HALL_APPV1_GATEWAY_AUTH_RESP_FULL_MASK (\
        (1 << HALO_HALL_APPV1_MGM_FSK_WAN_JOIN_RESPONSE_CODE) | (1 << HALO_HALL_APPV1_MGM_SYNCED_GW) | \
        (1 << HALO_HALL_APPV1_MGM_UL_CHAR_PROF_3) | (1 << HALO_HALL_APPV1_MGM_DL_CHAR_PROF_3) | \
        (1 << HALO_HALL_APPV1_MGM_LINK_DURATION) | (1 << HALO_HALL_APPV1_MGM_SEC_KEY))

typedef enum {
    halo_all_gateway_zwave_flag_disable = 0x0,
    halo_all_gateway_zwave_flag_enable = 0x3,
} halo_all_gateway_zwave_flag;

typedef struct {
    uint8_t pkt[HALO_ALL_GATEWAY_MAX_PKT_SIZE];
    uint8_t pkt_size;
} halo_all_gateway_pkt_t;

/*|1B Threshold| |1B Timeout value|*/
typedef struct {
    uint8_t threshold;
    uint8_t timeout_value;
} halo_all_gateway_zwave_threshold_t;

/*array of 12 16-bit unsigned integers would record the RSSI values i*/
typedef struct {
    uint16_t rssi[HALO_ALL_GATEWAY_ZWAVE_PARAMS];
} halo_all_gateway_zwave_params_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HALO_ALL_APPV1_GATEWAY_TYPES_H_ */
