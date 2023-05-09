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

#ifndef RNETC_INCLUDE_APPV1_MGM_SERIALIZE_H_
#define RNETC_INCLUDE_APPV1_MGM_SERIALIZE_H_

#include <halo/lib/hall/hallerr.h>
#include <halo/lib/hall/appv1/mgm/types.h>
#include <halo/lib/hall/appv1/cmd.h>
#include <appv1/mgm_core/serialize.h>

#ifdef __cplusplus
extern "C" {
#endif

sid_error_t halo_hall_appv1_mgm_dfu_params_serialize(
    uint8_t* buffer, const size_t len, const halo_hall_appv1_mgm_dfu_params_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_mgm_rf_power_serialize(
    uint8_t* buffer, const size_t len, const halo_hall_appv1_rf_power_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_mgm_channel_survey_prop_serialize(
    uint8_t* buffer, const size_t len, const halo_hall_appv1_channel_survey_prop_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_mgm_channel_survey_result_serialize(
    uint8_t* buffer, const size_t len, const halo_hall_appv1_channel_survey_results_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_dev_profile_serialize(
    uint8_t* buffer, const size_t len, const halo_hall_appv1_dev_profile_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_dev_profile_resp_serialize(
    uint8_t* buffer, const size_t len, const halo_hall_appv1_dev_profile_resp_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_slot_param_serialize(uint8_t* buffer, const size_t len, const halo_hall_appv1_slot_param_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_slot_params_serialize(
    uint8_t* buffer, const size_t len, const halo_hall_appv1_slot_params_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_config_param_serialize(
    uint8_t* buffer, const size_t len, const halo_hall_appv1_config_param_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_mgm_proxy_resp_time_serialize(
    uint8_t * buffer, const size_t len, const halo_hall_appv1_mgm_proxy_resp_time_t * data, hall_size_t * out_size);
sid_error_t halo_hall_appv1_mgm_security_params_serialize(
    uint8_t* buffer, const size_t len, const halo_hall_appv1_security_params_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_mgm_security_capabilities_serialize(
    uint8_t* buffer, const size_t len, const halo_hall_appv1_security_capabilities_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_mgm_join_message_generic_serialize(
    uint8_t *buffer, const size_t len, const halo_hall_appv1_mgm_join_proc_key_t *data, hall_size_t *out_size);
sid_error_t halo_hall_appv1_mgm_config_tlv_frmt_serialize(uint8_t* buffer, const size_t len,
    const mgm_config_tlv_frmt_t* data, hall_size_t * out_size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_INCLUDE_APPV1_MGM_SERIALIZE_H_ */
