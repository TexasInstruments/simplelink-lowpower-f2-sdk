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

#ifndef SALL_MGM_CORE_LIB_H
#define SALL_MGM_CORE_LIB_H

#include <sall_mgm_core_interface.h>

#include <halo/lib/hall/core/dispatcher.h>
#include <sid_network_address.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    SID_MGM_CORE_GCS_STATE_CLEAR = 0x00,
    SID_MGM_CORE_GCS_STATE_NTW_SYNC = 0x01,
    SID_MGM_CORE_GCS_STATE_ONE_SHOT = 0x02,
    SID_MGM_CORE_GCS_STATE_LAST
};

typedef struct {
    halo_hall_receive_interface_t trust_net_ifc;
    struct sid_deferred_event time_event;
    sall_mgm_gcs_config_t config;
    struct sid_timespec last_sent_uptime;
    struct sid_address request_address;
    uint32_t key;
    uint8_t counter;
    uint8_t suspend_interval_idx;
    uint8_t state_handle_id;
    bool periodic_sync_started;
} sid_sall_mgm_core_gcs_ctx_t;

struct sall_mgm_core_handle {
    const halo_hall_appv1_mgm_core_ext_ifc *hall_class_core_ifc;
    const halo_hall_dispatcher_ifc *dispatcher;
    const sall_mgm_core_config_t *config;
    const struct sid_event_queue *event_queue;
    config_provider_t ext_conf;
    network_interface_control_notify_ifc ctrl_ntfy_interface;
    sall_mgm_core_ni_control_notify_entry_t ni_ctrl_ntfy_entry;
    sall_mgm_core_user_entry_t user_entry;
    sall_mgm_core_callbacks_t callbacks;
    sid_sall_mgm_core_gcs_ctx_t gcs_ctx;
    struct sid_deferred_event keep_alive_event;
    struct sid_deferred_event change_config_event;
    struct sid_deferred_event factory_reset;
    uint8_t factory_reset_type;
    uint8_t device_id[SID_ADDRESS_SIZE_MAX];
    uint8_t ni_scan_flag;
    uint8_t boot_reset_report;
    uint8_t auxiliary_group_id[SID_MAX_GROUP_NUM_COUNT];
    const network_interface_ifc *net_interfaces[];
};

struct sall_mgm_core_handle *sall_mgm_core_lib_impl_get(const halo_hall_appv1_mgm_core_ifc *ifc);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* SALL_MGM_CORE_LIB_H */
