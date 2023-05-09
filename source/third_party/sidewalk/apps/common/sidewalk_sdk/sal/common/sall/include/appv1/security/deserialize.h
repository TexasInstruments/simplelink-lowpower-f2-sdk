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

#ifndef HALO_ALL_APPV1_SECURITY_DESERIALIZE_H_
#define HALO_ALL_APPV1_SECURITY_DESERIALIZE_H_

#include <stddef.h>
#include <halo/lib/hall/hallerr.h>
#include <halo/lib/hall/appv1/security/types.h>
#include <halo/lib/hall/appv1/cmd.h>

#ifdef __cplusplus
extern "C" {
#endif

sid_error_t halo_hall_appv1_security_signature_deserialize(
    const uint8_t* buffer, const size_t len, security_signature_t* data, hall_size_t* out_size);

sid_error_t halo_hall_appv1_security_hand_shake_deserialize(const uint8_t* buffer, const size_t len, hand_shake_t* data, hall_size_t* out_size);

sid_error_t halo_hall_appv1_security_challenge_req_deserialize(const uint8_t* buffer, const size_t len, challenge_request_t* data, hall_size_t* out_size);

sid_error_t halo_hall_appv1_security_ecdh_puk_deserialize(const uint8_t* buffer, const size_t len, ecdh_puk_t* data, hall_size_t* out_size);

sid_error_t halo_hall_appv1_security_resp_encryption_capabilities_deserialize(
    const uint8_t* buffer, const size_t len, resp_encryption_capabilities_t* resp, hall_size_t* out_size);

sid_error_t halo_hall_appv1_security_check_cert_serial_deserialize(
    const uint8_t* buffer, const size_t len, check_cert_serial_t* req, hall_size_t* out_size);

sid_error_t halo_hall_appv1_security_resp_device_cert_count_nonce_deserialize(
    const uint8_t* buffer, const size_t len, resp_device_cert_count_nonce_t* resp, hall_size_t* out_size);

sid_error_t halo_hall_appv1_security_resp_device_serial_puk_deserialize(
    const uint8_t* buffer, const size_t len, resp_device_serial_puk_t* resp, hall_size_t* out_size);

sid_error_t halo_hall_appv1_security_resp_model_serial_puk_deserialize(
    const uint8_t* buffer, const size_t len, resp_model_serial_puk_t* resp, hall_size_t* out_size);

sid_error_t halo_hall_appv1_security_remote_nonce_ecdh_puk_cert_count_deserialize(
    const uint8_t* buffer, const size_t len, remote_nonce_ecdh_puk_cert_count_t* resp, hall_size_t* out_size);

sid_error_t halo_hall_appv1_security_resp_hand_shake_deserialize(
    const uint8_t* buffer, const size_t len, resp_hand_shake_t* resp, hall_size_t* out_size);

sid_error_t halo_hall_appv1_security_challenge_resp_deserialize(
    const uint8_t* buffer, const size_t len, challenge_response_t* resp, hall_size_t* out_size);

sid_error_t halo_hall_appv1_security_d2d_key_response_deserialize(
    const uint8_t* buffer, const size_t len, d2d_gcm_key_t* data, hall_size_t* out_size);

sid_error_t halo_hall_appv1_security_error_deserialize(const uint8_t* buffer, const size_t len, security_error_t* resp, hall_size_t* out_size);

sid_error_t halo_hall_appv1_security_provisioning_init_deserialize(
    const uint8_t* buffer, const size_t len, halo_hall_appv1_security_provisioning_init_t* data, hall_size_t* out_size);

sid_error_t halo_hall_appv1_security_provisioning_init_response_deserialize(
    const uint8_t* buffer, const size_t len, halo_hall_appv1_security_provisioning_init_response_t* data, hall_size_t* out_size);

sid_error_t halo_hall_appv1_security_wan_params_deserialize(
    const uint8_t* buffer, const size_t len, halo_hall_appv1_security_wan_params_t* data, hall_size_t* out_size);

sid_error_t halo_hall_appv1_security_app_params_deserialize(
    const uint8_t* buffer, const size_t len, halo_hall_appv1_security_app_params_t* data, hall_size_t* out_size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HALO_ALL_APPV1_SECURITY_DESERIALIZE_H_ */
