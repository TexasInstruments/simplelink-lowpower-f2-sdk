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

#ifndef SALL_MGM_CORE_DEVICE_INFO_H
#define SALL_MGM_CORE_DEVICE_INFO_H

#include <sall_mgm_core_lib.h>
#include <sall_mgm_core_interface.h>
#include <sid_time_types.h>

#ifdef __cplusplus
}
#endif

sid_error_t on_sall_mgm_core_fw_version_get(const halo_hall_appv1_mgm_core_ifc *ifc,
                                            halo_hall_message_descriptor_t *mdesc);
sid_error_t on_sall_mgm_core_fw_version_response(const halo_hall_appv1_mgm_core_ifc *ifc,
                                                 halo_hall_message_descriptor_t *mdesc,
                                                 const halo_hall_appv1_mgm_firmware_version_t *data);
sid_error_t on_sall_mgm_core_serial_num_get(const halo_hall_appv1_mgm_core_ifc *ifc,
                                            halo_hall_message_descriptor_t *mdesc);
sid_error_t on_sall_mgm_core_reset_info_get(const halo_hall_appv1_mgm_core_ifc *ifc,
                                            halo_hall_message_descriptor_t *mdesc);
sid_error_t on_sall_mgm_core_class_version_get(const halo_hall_appv1_mgm_core_ifc *_this,
                                               halo_hall_message_descriptor_t *mdesc,
                                               const uint16_t class_id);
sid_error_t on_sall_mgm_core_fw_ota_params_get(const halo_hall_appv1_mgm_core_ifc *ifc,
                                               halo_hall_message_descriptor_t *mdesc);
sid_error_t on_sall_mgm_core_status_get(const halo_hall_appv1_mgm_core_ifc *ifc, halo_hall_message_descriptor_t *mdesc);
sid_error_t sall_mgm_core_keep_alive_schedule(struct sall_mgm_core_handle *handle, const bool enable);
sid_error_t sall_mgm_core_keep_alive_reschedule(struct sall_mgm_core_handle *handle, uint32_t ka_send_delay_s);
void sall_mgm_core_boot_reset_report_send(struct sall_mgm_core_handle *handle);
sid_error_t sall_mgm_core_keep_alive_event_handle(struct sid_event *event, void *data);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif /* SALL_MGM_CORE_DEVICE_INFO_H */
