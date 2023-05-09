/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_900_NETWORK_LIB_H_
#define SID_900_NETWORK_LIB_H_

#include <halo/lib/hall/core/resource_manager.h>
#include <sid_900_network_ifc.h>
#include <sid_network_data_ifc.h>
#include <sid_900_types.h>
#include <sid_network_control_ifc.h>

#include "rnet_hall.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    struct sid_event notify_event;
    struct sid_event state_event;
    struct sid_event radio_tx_event;
    struct sid_event ctrl_req_event;
    const struct sid_event_queue *event_queue;
    void *channel_survey_data;
    rnet_hall_state_t state;
    sid_ni_900_radio_tx_info_t radio_tx_info;
} network_interface_ctx_t;

typedef struct {
    network_interface_ifc ifc;
    const network_interface_data_notify_ifc *ntfy;
    const network_interface_control_notify_ifc *control_ntfy;
    const halo_hall_resource_manager_ifc *resource_manager;
    const sid_900_network_interface_control_config_t *ni_control_config;
    network_interface_ctx_t *interface_ctx;
    list_node_t input_msg_queue;
    list_node_t ctrl_req_msg_queue;
    rnet_hall_subscriber_t subscription;
    struct sid_address local;
    enum sid_ni_900_dev_param_sync_status dev_param_sync_status;
    enum network_interface_mode_id sid_900_ni_mode;
    network_power_operation_mode sid_900_op_mode;
} network_interface_impl_t;

void sid_900_network_interface_data_ifc_get(network_interface_data_ifc * ifc);
ret_code_t on_message_cb(void *cb_ctx, const uint8_t tr_idx);
ret_code_t sid_900_ni_control_on_tx_prepare_handle(void *arg, const uint8_t tr_idx, const uint32_t tx_duration_ms);
sid_error_t sid_900_ni_control_on_ctrl_req_event_handler(struct sid_event * event, void * data);
ret_code_t sid_900_ni_control_on_ctrl_req_handle(void *arg, enum rnet_hall_ctrl_info_req req_type, const void *data);
sid_error_t network_interface_control_init(network_interface_impl_t* impl,
                            const sid_900_network_interface_config * cfg);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* PROJECTS_SID_SAL_COMMON_SID_900_NETWORK_IFC_INCLUDE_EXPORT_SID_900_NETWORK_LIB_H_ */
