/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SALL_MGM_CONFIGURATION_COMMANDS_H
#define SALL_MGM_CONFIGURATION_COMMANDS_H

#include <sall_mgm_interface.h>
#include <sall_mgm_lib.h>

#ifdef __cplusplus
extern "C" {
#endif

sid_error_t halo_management_on_p2p_channel_set(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc,
                            const halo_hall_appv1_mgm_p2p_channel_t *channel_info);
sid_error_t halo_management_on_p2p_channel_get(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc);
sid_error_t halo_management_on_rf_power_set(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc,
                            const halo_hall_appv1_rf_power_t *rf_power);
 sid_error_t halo_management_on_rf_power_get(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc,
                            const uint8_t data);
sid_error_t halo_management_on_device_profile_set(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc,
                            const halo_hall_appv1_dev_profile_t *data);
sid_error_t halo_management_on_device_profile_get(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc);
sid_error_t halo_management_on_slot_params_get(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc,
                            const uint8_t data);
sid_error_t halo_management_on_slot_params_set(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc,
                            const halo_hall_appv1_slot_params_t *data);
sid_error_t halo_management_on_protocol_on_get(const halo_hall_appv1_mgm_ifc* ifc,
                            halo_hall_message_descriptor_t *mdesc);
sid_error_t halo_management_on_protocol_on_set(const halo_hall_appv1_mgm_ifc* ifc,
                            halo_hall_message_descriptor_t* mdesc,
                            const halo_hall_appv1_mgm_flag_t data);
sid_error_t halo_management_on_max_missed_beacons_get(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc);
sid_error_t halo_management_on_max_missed_beacons_set(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc,
                            const uint8_t data);
sid_error_t halo_management_on_beacons_interval_get(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc);
sid_error_t halo_management_on_beacons_interval_set(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc,
                            const uint8_t data);
sid_error_t halo_management_on_ldr_channel_get(const halo_hall_appv1_mgm_ifc *_this,
                            halo_hall_message_descriptor_t* mdesc);
sid_error_t halo_management_on_ldr_channel_set(const halo_hall_appv1_mgm_ifc *_this,
                            halo_hall_message_descriptor_t *mdesc,
                            const uint8_t data);
sid_error_t halo_management_on_deficit_flag_get(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc);
sid_error_t halo_management_on_deficit_flag_set(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc,
                            const halo_hall_appv1_mgm_flag_t data);
sid_error_t halo_management_on_gw_consent_flag_get(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc);
sid_error_t halo_management_on_gw_consent_flag_set(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc,
                            const halo_hall_appv1_mgm_flag_t data);
sid_error_t halo_management_on_pan_scan_interval_get(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc);
sid_error_t halo_management_on_pan_scan_interval_set(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc,
                            const uint16_t data);
sid_error_t halo_management_on_configuration_params_get(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc);
sid_error_t sall_mgm_900_tmp_prtcl_exp_get(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc);
sid_error_t sall_mgm_900_tmp_prtcl_exp_set(const halo_hall_appv1_mgm_ifc *ifc,
                            halo_hall_message_descriptor_t *mdesc, const mgm_config_tlv_frmt_t *data);

#ifdef __cplusplus
}
#endif

#endif /* SALL_MGM_CONFIGURATION_COMMANDS_H */
