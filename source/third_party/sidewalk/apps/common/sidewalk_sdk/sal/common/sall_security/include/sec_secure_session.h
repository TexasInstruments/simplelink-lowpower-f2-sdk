/*
 * Copyright 2020-2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SEC_SECURE_SESSION_H
#define SEC_SECURE_SESSION_H

#include <sec_defines.h>

#include <halo/lib/hall/appv1/security/types.h>
#include <sid_network_address.h>
#include <sec_credentials.h>

#include <sid_error.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sec_ss_generated_info {
    uint8_t gcm_key[SEC_GCM_KEY_SIZE];
};

struct sec_ss_address {
    struct sid_address address;
    bool received_new_address;
};

struct sec_ss_config {
    struct sid_address address;
    uint8_t is_smsn;
    uint8_t mfg_version;
    bool is_device_online;
};

struct sec_ss_handle;

sid_error_t sec_ss_get_encryption_capabilities(struct sec_ss_handle *const handle,
                                               const halo_hall_appv1_ecr req,
                                               resp_encryption_capabilities_t *const resp);

sid_error_t sec_ss_provisioning_init_request(struct sec_ss_handle *const handle,
                                             halo_hall_appv1_security_provisioning_init_t *const req);

sid_error_t
sec_ss_provisioning_init_request_signature(struct sec_ss_handle *const handle,
                                           const halo_hall_appv1_security_provisioning_init_t *const transient,
                                           security_signature_t *const resp);

sid_error_t sec_ss_provisioning_init_response(struct sec_ss_handle *const handle,
                                              const halo_hall_appv1_security_provisioning_init_response_t *const resp,
                                              struct sec_ss_address *const address);

sid_error_t sec_ss_provisioning_init_signature(struct sec_ss_handle *const handle,
                                               const security_signature_t *const req);

sid_error_t sec_ss_get_device_ecdh_signature(struct sec_ss_handle *const handle, security_signature_t *const resp);

sid_error_t sec_ss_get_generated_info(struct sec_ss_handle *const handle, struct sec_ss_generated_info *const keys);

sid_error_t
sec_ss_hand_shake(struct sec_ss_handle *const handle, const hand_shake_t *const req, resp_hand_shake_t *const resp);

sid_error_t sec_ss_challenge(struct sec_ss_handle *const handle,
                             const challenge_request_t *const req,
                             challenge_response_t *const resp);

void sec_ss_deinit(struct sec_ss_handle *const handle);

sid_error_t sec_ss_init(const struct sec_ss_config *const config,
                        const struct sec_credentials *const credentials,
                        struct sec_ss_handle **handle);

#ifdef __cplusplus
}
#endif

#endif /*! SEC_SECURE_SESSION_H */
