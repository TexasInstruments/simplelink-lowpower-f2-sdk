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

#ifndef HALO_LIB_HALL_CORE_POLICY_H
#define HALO_LIB_HALL_CORE_POLICY_H

#include <sid_network_address.h>
#include <halo/lib/hall/appv1/message.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief halo_hall_policy_resp_req_t
 * Do not modify/add/delete these values,
 * POLICY_RESP_REQUIRED_NO and POLICY_RESP_REQUIRED_YES have direct mapping to bit value
 */
typedef enum {
    POLICY_RESP_REQUIRED_NO = 0x00,
    POLICY_RESP_REQUIRED_YES = 0x01,
    POLICY_RESP_REQUIRED_UNDEFINED = 0xFF
} halo_hall_policy_resp_req_t;

typedef struct {
    uint32_t time_to_live_ms;
    uint8_t retries;
    uint8_t use_msg_params; /** If set use msg params */
    uint8_t no_filter_duplicates; /** If set then duplicates are not filtered */
} halo_hall_policy_params_t;

typedef struct {
    struct sid_address local;
    struct sid_address remote;
    hall_cmd_descriptor_t cmd;
    halo_hall_policy_resp_req_t response_required;
} halo_hall_policy_key_t;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* HALO_LIB_HALL_CORE_POLICY_H */
