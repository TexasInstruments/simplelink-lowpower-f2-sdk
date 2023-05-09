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

#ifndef SID_API_INTERNAL_H
#define SID_API_INTERNAL_H

#include <halo/lib/hall/appv1/data_pipe/ifc.h>
#include <halo/lib/hall/interface.h>

#include <sall_mgm_core_interface.h>
#include <sall_mgm_interface.h>
#include <sall_link_metric_interface.h>

#include <sid_event_queue_ifc.h>
#include <sid_network_control_ifc.h>

#include <sid_api.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//Private data types
typedef struct internal_handle {
    struct sid_config config;
    bool initialized;
    uint32_t started;
    uint32_t dup_msg_count;
    sid_error_t last_error;
    struct sid_status status;
    struct sid_event_queue event_queue;
    struct sid_deferred_event mdm_event;
    halo_hall_lib_ctx_t sall_handle;
    halo_hall_status_notify_t command_status_notify;
    on_timer_t mdm_timer_cb;
    mdm_ctx_t mdm_timer_cb_ctx;
    struct sid_memory_pool *mem_pool;
    const halo_hall_appv1_data_pipe_ext_ifc *data_pipe_ifc;
    const network_interface_ifc *net_adapter_ble;
    const network_interface_ifc *net_adapter_900;
    uint16_t mtu_ble;
    uint16_t mtu_900;
    struct sall_mgm_core_handle *sall_mgm_core;
    const halo_hall_stats_ifc hall_stats_instance;
    struct sid_address sidewalk_id;
    struct sid_address cloud_dest_addr;
    bool connection_request;
    bool secure_connection;
    bool allow_status_changed;
    bool filter_duplicates;
    uint8_t current_active_900_link_idx;
    network_interface_control_notify_ifc ble_ctrl_ntfy_interface;
    const sall_mgm_ifc *sall_mgm;
    const sall_link_metric_ifc_t *link_metrics_ifc;
    struct sid_timespec sid_stop_ts;
    void *diag_test_context;
} internal_handle_t;

struct sid_network_coverage_user_cb {
    void *user_ctx;
    void (*on_authenticated_mobile_connection)(void *user_ctx, bool valid);
};

sid_error_t sid_network_coverage_init(struct sid_handle *handle, struct sid_network_coverage_user_cb *callback);

sid_error_t sid_network_coverage_start(struct sid_handle *h, uint32_t link_mask);

sid_error_t sid_network_coverage_stop(struct sid_handle *handle, uint32_t link_mask);

sid_error_t sid_network_coverage_deinit(struct sid_handle *handle);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_API_INTERNAL_H */
