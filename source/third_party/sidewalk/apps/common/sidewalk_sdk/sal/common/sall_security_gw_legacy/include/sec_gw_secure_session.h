/*
 *Copyright 2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 *AMAZON PROPRIETARY/CONFIDENTIAL
 *
 *You may not use this file except in compliance with the terms and conditions
 *set forth in the accompanying LICENSE.TXT file.
 *
 *THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 *DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 *IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 *FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SEC_GW_SECURE_SESSION_H
#define SEC_GW_SECURE_SESSION_H

#include <sec_defines.h>
#include <sec_gw_credentials.h>

#include <halo/lib/hall/appv1/security/types.h>
#include <sid_network_address.h>
#include <sid_error.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sec_gw_ss_generated_info {
    uint8_t gcm_key[SEC_GCM_KEY_SIZE];
    uint8_t shared_secret[SEC_ECDH_SHARED_SECRET_SIZE];
    uint8_t peer_nonce[SEC_BEAMS_NONCE_SIZE];
};

struct sec_gw_ss_config {
    struct sid_address address;
    uint8_t mfg_version;
};

struct sec_gw_ss_handle;

sid_error_t sec_gw_ss_get_encryption_capabilities(struct sec_gw_ss_handle *const handle,
                                                  const halo_hall_appv1_ecr req,
                                                  resp_encryption_capabilities_t *const resp);

sid_error_t sec_gw_ss_check_cert_serial(struct sec_gw_ss_handle *const handle, const check_cert_serial_t *const req);

sid_error_t sec_gw_ss_get_device_cert_count_nonce(struct sec_gw_ss_handle *const handle,
                                                  resp_device_cert_count_nonce_t *const resp);

void sec_gw_ss_get_device_serial_puk(struct sec_gw_ss_handle *const handle,
                                     resp_device_serial_puk_t *const serial_puk_resp);

void sec_gw_ss_get_device_signature(struct sec_gw_ss_handle *const handle,
                                    security_signature_t *const sec_signature_resp);

void sec_gw_ss_get_model_serial_puk(struct sec_gw_ss_handle *const handle,
                                    resp_model_serial_puk_t *const mode_serial_puk_resp);

void sec_gw_ss_get_model_signature(struct sec_gw_ss_handle *const handle,
                                   security_signature_t *const model_signature_resp);

sid_error_t
sec_gw_ss_set_remote_nonce_ecdh_puk_cert_count(struct sec_gw_ss_handle *const handle,
                                               const remote_nonce_ecdh_puk_cert_count_t *const ecdh_puk_cert_count_req);

sid_error_t sec_gw_ss_set_remote_ecdh_signature(struct sec_gw_ss_handle *const handle,
                                                const security_signature_t *const security_signature_req);

void sec_gw_ss_set_remote_cert_serial_puk(struct sec_gw_ss_handle *const handle,
                                          const resp_model_serial_puk_t *const model_serial_puk_req);

sid_error_t sec_gw_ss_set_remote_signature(struct sec_gw_ss_handle *const handle,
                                           const security_signature_t *const security_signature_req);

sid_error_t sec_gw_ss_get_device_ecdh_puk(struct sec_gw_ss_handle *const handle, ecdh_puk_t *const ecdh_puk_resp);

sid_error_t sec_gw_ss_get_device_ecdh_signature(struct sec_gw_ss_handle *const handle,
                                                security_signature_t *const security_signature_resp);

void sec_gw_ss_get_generated_info(struct sec_gw_ss_handle *const handle, struct sec_gw_ss_generated_info *const info);

sid_error_t sec_gw_ss_hand_shake(struct sec_gw_ss_handle *const handle,
                                 const hand_shake_t *const req,
                                 resp_hand_shake_t *const resp);

void sec_gw_ss_deinit(struct sec_gw_ss_handle *const handle);

void sec_gw_ss_init(const struct sec_gw_ss_config *const config,
                    const struct sec_gw_credentials *const credentials,
                    struct sec_gw_ss_handle **handle);

#ifdef __cplusplus
}
#endif

#endif /*! SEC_GW_SECURE_SESSION_H */
