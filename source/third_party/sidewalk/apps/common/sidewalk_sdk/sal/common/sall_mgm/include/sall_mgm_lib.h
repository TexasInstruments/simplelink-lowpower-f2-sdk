/*
 * Copyright 2020-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SALL_MGM_LIB_H
#define SALL_MGM_LIB_H

#include <sid_sdk_internal_config.h>
#include <sall_mgm_interface.h>

#include <halo/lib/hall/core/dispatcher.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sall_mgm_test_ctx;

typedef struct {
    struct sid_address remote;
    uint32_t msg_id;
    uint8_t sec_ena;
} sall_mgm_channel_survey_ctx_t;

typedef struct {
    uint8_t try_count_max;
    uint8_t request_retry_interval;
} sall_mgm_join_config_t;

typedef struct {
    const network_interface_control_ifc *ni_control;
    struct sid_deferred_event time_event;
    struct sid_address request_address;
    uint32_t delay;
    sall_mgm_join_config_t config;
    uint8_t retry_counter;
    uint8_t join_response;
    uint8_t state;
    uint8_t destination_address;
    uint8_t dev_mode;
    uint8_t join_reason;
    bool pending_nw_join;
    halo_hall_status_notify_t status_notify;
} sid_sall_mgm_core_join_ctx_t;

typedef struct {
    const network_interface_ifc *ni_900_ifc;
    uint8_t dev_mode;
    struct sid_deferred_event ffs_event;
    struct sid_deferred_event registration_retry_event;
    sall_mgm_ffs_callbacks_t ffs_callbacks;
    struct sid_address provisioner;
    int16_t rssi_threshold;
    uint8_t *retry_threshold;
    uint16_t *retry_wait_time_s;
    uint8_t max_retries;
    uint8_t retry_count;
    uint32_t registration_interval_s;
    uint8_t state;
    bool enable;
    bool is_key_refresh_active;
} sid_sall_mgm_ffs_ctx_t;

typedef struct {
    struct sall_mgm_core_handle *mgm_core;
    const halo_hall_appv1_mgm_ext_ifc *hall_class_ifc;
    const sall_mgm_config_cmd_ctx_t *config;
    const struct sid_event_queue *event_queue;
    network_interface_control_notify_ifc ctrl_ntfy_interface;
    struct sall_mgm_test_ctx *test;
    sall_mgm_channel_survey_ctx_t *channel_survey_ctx;
    sall_mgm_ifc lib_ifc;
    sid_sall_mgm_core_join_ctx_t join_ctx;
#if SID_SDK_CONFIG_ENABLE_LINK_TYPE_2_REGISTRATION
    sid_sall_mgm_ffs_ctx_t ffs_ctx;
#endif
} sall_mgm_lib_impl_t;

sall_mgm_lib_impl_t *sall_mgm_lib_impl_get(const halo_hall_appv1_mgm_ifc * ifc);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* SALL_MGM_LIB_H */
