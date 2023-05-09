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

#ifndef RNETC_INCLUDE_APPV1_MGM_CORE_DESERIALIZE_H_
#define RNETC_INCLUDE_APPV1_MGM_CORE_DESERIALIZE_H_

#include <halo/lib/hall/hallerr.h>
#include <halo/lib/hall/appv1/mgm_core/types.h>
#include <halo/lib/hall/appv1/cmd.h>

#ifdef __cplusplus
extern "C" {
#endif

sid_error_t halo_hall_appv1_mgm_device_id_deserialize(
    const uint8_t* buffer, const size_t len, halo_hall_appv1_device_id_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_mgm_beams_security_key_deserialize(
    const uint8_t* buffer, const size_t len, halo_hall_appv1_beams_security_key_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_mgm_firmware_version_deserialize(
    const uint8_t* buffer, const size_t len, halo_hall_appv1_mgm_firmware_version_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_mgm_ka_timeout_deserialize(
    const uint8_t* buffer, const size_t len, uint32_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_mgm_reset_info_deserialize(
    const uint8_t* buffer, const size_t len, halo_hall_appv1_reset_info_t* ri, hall_size_t* out_size);
sid_error_t halo_hall_appv1_mgm_class_version_deserialize(
    const uint8_t* buffer, const size_t len, halo_hall_appv1_class_versions_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_mgm_ota_params_deserialize(
    const uint8_t* buffer, const size_t len, halo_hall_appv1_ota_params_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_mgm_p2p_channel_deserialize(
    const uint8_t* buffer, const size_t len, halo_hall_appv1_mgm_p2p_channel_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_mgm_gcs_time_deserialize(
    const uint8_t* buffer, const size_t len, halo_hall_appv1_gcs_time_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_mgm_gcs_time_wkey_deserialize(
    const uint8_t* buffer, const size_t len, halo_hall_appv1_gcs_time_wkey_t* data, hall_size_t* out_size);
sid_error_t sall_mgm_factory_reset_deserialize(const uint8_t *buffer,
                                               const size_t len,
                                               struct sall_mgm_factory_reset *data,
                                               hall_size_t *out_size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_INCLUDE_APPV1_MGM_CORE_DESERIALIZE_H_ */
