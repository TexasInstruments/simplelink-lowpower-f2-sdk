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

#ifndef HALLO_ALL_APPV1_SECURITY_IFC_H_
#define HALLO_ALL_APPV1_SECURITY_IFC_H_

#include <halo/lib/hall/appv1/security/types.h>
#include <halo/lib/hall/appv1/message.h>
#include <halo/lib/hall/appv1/generic_command_ifc.h>
#include <halo/lib/hall/interface.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct halo_hall_appv1_security_ifc_s* halo_hall_appv1_security_ifc;
typedef const struct halo_hall_appv1_security_gw_ifc_s* halo_hall_appv1_security_gw_ifc;
typedef const struct halo_hall_appv1_security_beams_ifc_s* halo_hall_appv1_security_beams_ifc;

struct halo_hall_appv1_security_ifc_s {
    sid_error_t (*get_encryption_capabilities)(
        const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_ecr requester);

    sid_error_t (*encryption_capabilities_response)(
        const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc, const resp_encryption_capabilities_t* resp);

    sid_error_t (*get_device_ecdh_signature)(const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*device_ecdh_signature_response)(
        const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc, const security_signature_t* resp);

    sid_error_t (*hand_shake)(const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc, const hand_shake_t* req);

    sid_error_t (*hand_shake_response)(
        const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc, const resp_hand_shake_t* req);

    sid_error_t (*provisioning_init_request)(
        const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_security_provisioning_init_t* req);
    sid_error_t (*provisioning_init_request_signature)(
        const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc, const security_signature_t* req);
    sid_error_t (*provisioning_init_response)(const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc,
        const halo_hall_appv1_security_provisioning_init_response_t* req);

    sid_error_t (*provisioning_init_signature)(
        const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc, const security_signature_t* req);
    sid_error_t (*provisioning_init_signature_response)(const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*wan_secure_params)(
        const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_security_wan_params_t* params);

    sid_error_t (*wan_secure_params_response)(const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*app_secure_params)(
        const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_security_app_params_t* params);

    sid_error_t (*app_secure_params_response)(const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*challenge_request)(const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc, const challenge_request_t* req);

    sid_error_t (*challenge_response)(
        const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc, const challenge_response_t* resp);

    sid_error_t (*d2d_secure_params)(
        const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_security_app_params_t* params);

    sid_error_t (*d2d_secure_params_response)(const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*get_d2d_key)(const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*d2d_key_response)(
        const halo_hall_appv1_security_ifc* _this, halo_hall_message_descriptor_t* mdesc, const d2d_gcm_key_t* resp);

};

struct halo_hall_appv1_security_beams_ifc_s {
    sid_error_t (*get_encryption_capabilities)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_ecr requester);

    sid_error_t (*encryption_capabilities_response)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const resp_encryption_capabilities_t* resp);

    sid_error_t (*check_cert_serial)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const check_cert_serial_t* req);

    sid_error_t (*check_cert_serial_response)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*get_device_cert_count_nonce)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*device_cert_count_nonce_response)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const resp_device_cert_count_nonce_t* resp);

    sid_error_t (*get_device_serial_puk)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*device_serial_puk_response)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const resp_device_serial_puk_t* resp);

    sid_error_t (*get_device_signature)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*device_signature_response)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const security_signature_t* resp);

    sid_error_t (*get_model_serial_puk)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*model_serial_puk_response)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const resp_model_serial_puk_t* resp);

    sid_error_t (*get_model_signature)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*model_signature_response)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const security_signature_t* resp);

    sid_error_t (*set_remote_nonce_ecdh_puk_cert_count)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const remote_nonce_ecdh_puk_cert_count_t* req);

    sid_error_t (*remote_nonce_ecdh_puk_cert_count_response)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*set_remote_ecdh_signature)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const security_signature_t* req);

    sid_error_t (*remote_ecdh_signature_response)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*set_remote_cert_serial_puk)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const resp_model_serial_puk_t* req);

    sid_error_t (*remote_cert_serial_puk_response)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*set_remote_signature)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const security_signature_t* req);

    sid_error_t (*set_remote_signature_response)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*get_device_ecdh_puk)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*get_device_ecdh_puk_response)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const ecdh_puk_t* resp);

    sid_error_t (*get_device_ecdh_signature)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*device_ecdh_signature_response)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const security_signature_t* resp);

    sid_error_t (*hand_shake)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const hand_shake_t* req);

    sid_error_t (*hand_shake_response)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const resp_hand_shake_t* req);

    sid_error_t (*security_error)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const security_error_t* req);

    sid_error_t (*provisioning_init_request)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_security_provisioning_init_t* req);
    sid_error_t (*provisioning_init_request_signature)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const security_signature_t* req);
    sid_error_t (*provisioning_init_response)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc,
        const halo_hall_appv1_security_provisioning_init_response_t* req);

    sid_error_t (*provisioning_init_signature)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const security_signature_t* req);
    sid_error_t (*provisioning_init_signature_response)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*wan_secure_params)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_security_wan_params_t* params);

    sid_error_t (*wan_secure_params_response)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*app_secure_params)(
        const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_security_app_params_t* params);

    sid_error_t (*app_secure_params_response)(const halo_hall_appv1_security_beams_ifc* _this, halo_hall_message_descriptor_t* mdesc);
};

struct halo_hall_appv1_security_gw_ifc_s {
    sid_error_t (*get_encryption_capabilities)(
        const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_ecr requester);

    sid_error_t (*encryption_capabilities_response)(
        const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const resp_encryption_capabilities_t* resp);

    sid_error_t (*check_cert_serial)(
        const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const check_cert_serial_t* req);

    sid_error_t (*check_cert_serial_response)(const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*get_device_cert_count_nonce)(const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*device_cert_count_nonce_response)(
        const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const resp_device_cert_count_nonce_t* resp);

    sid_error_t (*get_device_serial_puk)(const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*device_serial_puk_response)(
        const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const resp_device_serial_puk_t* resp);

    sid_error_t (*get_device_signature)(const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*device_signature_response)(
        const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const security_signature_t* resp);

    sid_error_t (*get_model_serial_puk)(const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*model_serial_puk_response)(
        const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const resp_model_serial_puk_t* resp);

    sid_error_t (*get_model_signature)(const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*model_signature_response)(
        const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const security_signature_t* resp);

    sid_error_t (*set_remote_nonce_ecdh_puk_cert_count)(
        const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const remote_nonce_ecdh_puk_cert_count_t* req);

    sid_error_t (*remote_nonce_ecdh_puk_cert_count_response)(const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*set_remote_ecdh_signature)(
        const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const security_signature_t* req);

    sid_error_t (*remote_ecdh_signature_response)(const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*set_remote_cert_serial_puk)(
        const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const resp_model_serial_puk_t* req);

    sid_error_t (*remote_cert_serial_puk_response)(const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*set_remote_signature)(
        const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const security_signature_t* req);

    sid_error_t (*set_remote_signature_response)(const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*get_device_ecdh_puk)(const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*get_device_ecdh_puk_response)(
        const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const ecdh_puk_t* resp);

    sid_error_t (*get_device_ecdh_signature)(const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*device_ecdh_signature_response)(
        const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const security_signature_t* resp);

    sid_error_t (*hand_shake)(const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const hand_shake_t* req);

    sid_error_t (*hand_shake_response)(
        const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const resp_hand_shake_t* req);

    sid_error_t (*wan_secure_params)(
        const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_security_wan_params_t* params);

    sid_error_t (*wan_secure_params_response)(const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*app_secure_params)(
        const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_security_app_params_t* params);

    sid_error_t (*app_secure_params_response)(const halo_hall_appv1_security_gw_ifc* _this, halo_hall_message_descriptor_t* mdesc);
};

typedef struct halo_hall_appv1_security_ext_ifc_s halo_hall_appv1_security_ext_ifc;
typedef struct halo_hall_appv1_security_gw_ext_ifc_s halo_hall_appv1_security_gw_ext_ifc;
typedef struct halo_hall_appv1_security_beams_ext_ifc_s halo_hall_appv1_security_beams_ext_ifc;

struct halo_hall_appv1_security_ext_ifc_s {
    halo_hall_appv1_security_ifc send;
    halo_hall_appv1_security_ifc receive;
    halo_hall_appv1_generic_command_ifc generic;
};

struct halo_hall_appv1_security_gw_ext_ifc_s {
    halo_hall_appv1_security_gw_ifc send;
    halo_hall_appv1_security_gw_ifc receive;
    halo_hall_appv1_generic_command_ifc generic;
};

struct halo_hall_appv1_security_beams_ext_ifc_s {
    halo_hall_appv1_security_beams_ifc send;
    halo_hall_appv1_security_beams_ifc receive;
    halo_hall_appv1_generic_command_ifc generic;
};

sid_error_t halo_hall_appv1_security_create(const halo_hall_appv1_security_ext_ifc** _this, const halo_hall_appv1_security_ifc user_notify_cb,
    halo_hall_appv1_user_ctx_t user_ctx, const void* lib_ctx);

sid_error_t halo_hall_appv1_security_gw_create(const halo_hall_appv1_security_gw_ext_ifc** _this, const halo_hall_appv1_security_gw_ifc user_notify_cb,
    halo_hall_appv1_user_ctx_t user_ctx, const void* lib_ctx);

sid_error_t halo_hall_appv1_security_beams_create(const halo_hall_appv1_security_beams_ext_ifc** _this, const halo_hall_appv1_security_beams_ifc user_notify_cb,
    halo_hall_appv1_user_ctx_t user_ctx, const void* lib_ctx);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HALLO_ALL_APPV1_SECURITY_IFC_H_ */
