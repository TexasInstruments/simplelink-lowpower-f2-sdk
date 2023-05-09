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

#ifndef SALL_MGM_CORE_CONFIG_COMMANDS_H
#define SALL_MGM_CORE_CONFIG_COMMANDS_H

#include <sall_mgm_core_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

sid_error_t on_sall_mgm_core_dev_id_get(const halo_hall_appv1_mgm_core_ifc *ifc, halo_hall_message_descriptor_t *mdesc);
sid_error_t on_sall_mgm_core_pan_id_set(const halo_hall_appv1_mgm_core_ifc *ifc,
                                        halo_hall_message_descriptor_t *mdesc,
                                        const halo_hall_appv1_pan_id_t *id);
sid_error_t on_sall_mgm_core_pan_id_get(const halo_hall_appv1_mgm_core_ifc *ifc, halo_hall_message_descriptor_t *mdesc);
sid_error_t on_sall_mgm_core_p2p_bcast_key_set(const halo_hall_appv1_mgm_core_ifc *ifc,
                                               halo_hall_message_descriptor_t *mdesc,
                                               const halo_hall_appv1_beams_security_key_t *key);
sid_error_t on_sall_mgm_core_p2p_bcast_key_get(const halo_hall_appv1_mgm_core_ifc *ifc,
                                               halo_hall_message_descriptor_t *mdesc);
sid_error_t on_sall_mgm_core_p2p_brk_key_set(const halo_hall_appv1_mgm_core_ifc *ifc,
                                             halo_hall_message_descriptor_t *mdesc,
                                             const halo_hall_appv1_beams_security_key_t *key);
sid_error_t on_sall_mgm_core_p2p_brk_key_get(const halo_hall_appv1_mgm_core_ifc *ifc,
                                             halo_hall_message_descriptor_t *mdesc);
sid_error_t on_sall_mgm_core_p2p_channel_set(const halo_hall_appv1_mgm_core_ifc *ifc,
                                             halo_hall_message_descriptor_t *mdesc,
                                             const halo_hall_appv1_mgm_p2p_channel_t *channel_info);
sid_error_t on_sall_mgm_core_p2p_channel_get(const halo_hall_appv1_mgm_core_ifc *ifc,
                                             halo_hall_message_descriptor_t *mdesc);
sid_error_t on_sall_mgm_core_group_id_set(const halo_hall_appv1_mgm_core_ifc *ifc,
                                          halo_hall_message_descriptor_t *mdesc,
                                          const uint8_t id);
sid_error_t on_sall_mgm_core_group_id_get(const halo_hall_appv1_mgm_core_ifc *ifc,
                                          halo_hall_message_descriptor_t *mdesc);
sid_error_t on_sall_mgm_core_pairing_state_set(const halo_hall_appv1_mgm_core_ifc *ifc,
                                               halo_hall_message_descriptor_t *mdesc,
                                               const uint8_t state);
sid_error_t on_sall_mgm_core_pairing_state_get(const halo_hall_appv1_mgm_core_ifc *ifc,
                                               halo_hall_message_descriptor_t *mdesc);
sid_error_t on_sall_mgm_core_keep_alive_interval_set(const halo_hall_appv1_mgm_core_ifc *ifc,
                                                     halo_hall_message_descriptor_t *mdesc,
                                                     const uint32_t interval_s);
sid_error_t on_sall_mgm_core_keep_alive_interval_get(const halo_hall_appv1_mgm_core_ifc *ifc,
                                                     halo_hall_message_descriptor_t *mdesc);
sid_error_t on_sall_mgm_core_config_complete_set(const halo_hall_appv1_mgm_core_ifc *ifc,
                                                 halo_hall_message_descriptor_t *mdesc,
                                                 const uint8_t state);

#ifdef __cplusplus
}
#endif

#endif /* SALL_MGM_CORE_CONFIG_COMMANDS_H */
