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

#ifndef SALL_MGM_CORE_GCS_INTERFACE_H
#define SALL_MGM_CORE_GCS_INTERFACE_H

#include <sall_mgm_core_lib.h>

#ifdef __cplusplus
extern "C" {
#endif

void sid_sall_mgm_core_gcs_clear(struct sall_mgm_core_handle *handle);
void sid_sall_mgm_core_gcs_to_one_shot_sync_handle(struct sall_mgm_core_handle *handle, uint16_t *delay);
void sid_sall_mgm_core_gcs_to_network_sync_handle(struct sall_mgm_core_handle *handle);
sid_error_t sid_sall_mgm_core_gcs_event_handle(struct sid_event *event, void *data);
void sall_mgm_core_gcs_status_change(struct sall_mgm_core_handle *handle,
                                     const halo_hall_message_descriptor_t *const mdesc,
                                     const bool status);
void sid_sall_mgm_core_gcs_notify_send(struct sall_mgm_core_handle *handle);
void sid_sall_mgm_core_gcs_clock_synchronize_trigger(struct sall_mgm_core_handle *handle);
sid_error_t on_sall_mgm_core_gcs_on_time_set(const halo_hall_appv1_mgm_core_ifc *ifc,
                                             halo_hall_message_descriptor_t *mdesc,
                                             const halo_hall_appv1_gcs_time_t *data);
sid_error_t on_sall_mgm_core_gcs_on_time_get(const halo_hall_appv1_mgm_core_ifc *ifc,
                                             halo_hall_message_descriptor_t *mdesc,
                                             const halo_hall_appv1_gcs_time_wkey_t *data);
sid_error_t on_sall_mgm_core_gcs_on_time_response(const halo_hall_appv1_mgm_core_ifc *ifc,
                                                  halo_hall_message_descriptor_t *mdesc,
                                                  const halo_hall_appv1_gcs_time_wkey_t *data);
sid_error_t on_sall_mgm_core_gcs_on_time_notify(const halo_hall_appv1_mgm_core_ifc *ifc,
                                                halo_hall_message_descriptor_t *mdesc,
                                                const halo_hall_appv1_gcs_time_t *data);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif /* SALL_MGM_CORE_GCS_INTERFACE_H */
