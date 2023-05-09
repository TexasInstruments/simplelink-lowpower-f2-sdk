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

#ifndef SALL_MGM_CORE_INTERFACE_H
#define SALL_MGM_CORE_INTERFACE_H

#include <sall_mgm_core_types.h>
#include <sid_network_control_ifc.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Management Library Core functionality interface
 *
 * Exposes API required by user for:
 *  # manage system: @ref
 *  # access configuration
 *  # create message descriptors to send messages
 */

void sall_mgm_core_destroy(struct sall_mgm_core_handle *handle);
void sall_mgm_core_gcs_trust_link_set(struct sall_mgm_core_handle *handle, halo_hall_receive_interface_t trust_net_ifc);
void sall_mgm_core_config_get(const struct sall_mgm_core_handle *const handle, sid_ni_config_t *config);
sid_error_t sall_mgm_core_send_interface_get(const struct sall_mgm_core_handle *const handle,
                                             const halo_hall_appv1_mgm_core_ifc **send);
sid_error_t sall_mgm_core_factory_reset(struct sall_mgm_core_handle *handle, uint8_t factory_reset_type);
sid_error_t sall_mgm_core_pairing_state_get(const struct sall_mgm_core_handle *const handle, uint8_t *state);
sid_error_t sall_mgm_core_get_configuration_complete(const struct sall_mgm_core_handle *const handle, uint8_t *state);
sid_error_t sall_mgm_core_local_get(const struct sall_mgm_core_handle *const handle, struct sid_address *local);
sid_error_t sall_mgm_core_local_set(struct sall_mgm_core_handle *handle, struct sid_address *local);
sid_error_t sall_mgm_core_halo_message_descriptor_to_broadcast(const struct sall_mgm_core_handle *const handle,
                                                               halo_hall_message_descriptor_t *mdesc);
sid_error_t sall_mgm_core_halo_message_descriptor_to_gateway(const struct sall_mgm_core_handle *const handle,
                                                             halo_hall_message_descriptor_t *mdesc);
sid_error_t sall_mgm_core_halo_message_descriptor_to_group(const struct sall_mgm_core_handle *const handle,
                                                           halo_hall_message_descriptor_t *mdesc);
sid_error_t sall_mgm_core_halo_message_descriptor_to_cloud(const struct sall_mgm_core_handle *const handle,
                                                           halo_hall_message_descriptor_t *mdesc,
                                                           const uint8_t *cloud_id);
sid_error_t
sall_mgm_core_network_interface_control_notify_get(const struct sall_mgm_core_handle *const handle,
                                                   const network_interface_data_ifc *data_inteface,
                                                   const network_interface_control_notify_ifc **ctrl_ntfy_interface);
sid_error_t sall_mgm_core_ni_control_notify_bind(struct sall_mgm_core_handle *handle,
                                                 const sall_mgm_core_ni_control_notify_entry_t *ni_control_notify_entry);
sid_error_t sall_mgm_core_user_bind(struct sall_mgm_core_handle *handle, const sall_mgm_core_user_entry_t *user_entry);
sid_error_t sall_mgm_core_get_user_bind(const struct sall_mgm_core_handle *const handle,
                                        const sall_mgm_core_user_entry_t **user_entry);
sid_error_t sall_mgm_core_reset_config_apply_timer(struct sall_mgm_core_handle *handle);
sid_error_t sall_mgm_core_set_auxiliary_group_number(struct sall_mgm_core_handle *handle,
                                                     const uint8_t group_numbers,
                                                     const uint8_t group_numbers_index);
sid_error_t sall_mgm_core_ni_900_get(const struct sall_mgm_core_handle *const handle, const network_interface_ifc **net_ctrl_ifc);
sid_error_t sall_mgm_core_keep_alive_send(struct sall_mgm_core_handle *handle);

sid_error_t sall_mgm_core_create(struct sall_mgm_core_handle **handle,
                                 const sall_mgm_core_config_t *const config,
                                 const sall_mgm_core_params_t *const params);

sid_error_t
sall_mgm_core_config_from_ifc_get(const struct sall_mgm_core_handle *const handle, const sall_mgm_core_config_id_t id, void *data);
#ifdef __cplusplus
}   // extern "C"
#endif

#endif /* SALL_MGM_CORE_INTERFACE_H */
