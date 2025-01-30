/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_plat_otp.h"
#include "rss_provisioning_bundle.h"

/* This is a stub to make the linker happy */
void __Vectors(){}

extern const struct dm_provisioning_data data;

enum tfm_plat_err_t __attribute__((section("DO_PROVISION"))) do_provision(void) {
    enum tfm_plat_err_t err;
    uint32_t new_lcs;

    err = tfm_plat_otp_write(PLAT_OTP_ID_KEY_BL2_ENCRYPTION,
                             sizeof(data.bl2_encryption_key),
                             data.bl2_encryption_key);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    err = tfm_plat_otp_write(PLAT_OTP_ID_BL1_ROTPK_0,
                             sizeof(data.bl1_rotpk_0),
                             data.bl1_rotpk_0);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    /* Assumes BL2 counters are contiguous*/
    for (int idx = 0; idx < MCUBOOT_IMAGE_NUMBER; idx++) {
        err = tfm_plat_otp_write(PLAT_OTP_ID_BL2_ROTPK_0 + idx,
                                 sizeof(data.bl2_rotpk[idx]),
                                 data.bl2_rotpk[idx]);
        if (err != TFM_PLAT_ERR_SUCCESS) {
            return err;
        }
    }

    err = tfm_plat_otp_write(PLAT_OTP_ID_KEY_SECURE_ENCRYPTION,
                             sizeof(data.s_image_encryption_key),
                             data.s_image_encryption_key);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }
    err = tfm_plat_otp_write(PLAT_OTP_ID_KEY_NON_SECURE_ENCRYPTION,
                             sizeof(data.ns_image_encryption_key),
                             data.ns_image_encryption_key);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    err = tfm_plat_otp_write(PLAT_OTP_ID_HOST_ROTPK_S,
                             sizeof(data.host_rotpk_s),
                             data.host_rotpk_s);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    err = tfm_plat_otp_write(PLAT_OTP_ID_HOST_ROTPK_NS,
                             sizeof(data.host_rotpk_ns),
                             data.host_rotpk_ns);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    err = tfm_plat_otp_write(PLAT_OTP_ID_HOST_ROTPK_CCA,
                             sizeof(data.host_rotpk_cca),
                             data.host_rotpk_cca);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    err = tfm_plat_otp_write(PLAT_OTP_ID_IMPLEMENTATION_ID,
                             sizeof(data.implementation_id),
                             data.implementation_id);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }
    err = tfm_plat_otp_write(PLAT_OTP_ID_VERIFICATION_SERVICE_URL,
                             sizeof(data.verification_service_url),
                             data.verification_service_url);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }
    err = tfm_plat_otp_write(PLAT_OTP_ID_PROFILE_DEFINITION,
                             sizeof(data.profile_definition),
                             data.profile_definition);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    err = tfm_plat_otp_write(PLAT_OTP_ID_SECURE_DEBUG_PK,
                             sizeof(data.secure_debug_pk),
                             data.secure_debug_pk);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    new_lcs = PLAT_OTP_LCS_SECURED;
    err = tfm_plat_otp_write(PLAT_OTP_ID_LCS,
                             sizeof(new_lcs),
                             (uint8_t*)&new_lcs);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    return err;
}
