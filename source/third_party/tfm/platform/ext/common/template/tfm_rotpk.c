/*
 * Copyright (c) 2019-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdint.h>
#include "tfm_plat_otp.h"

#ifdef BL2

static enum tfm_plat_err_t get_rotpk_hash(enum tfm_otp_element_id_t id,
                                          uint8_t* rotpk_hash,
                                          uint32_t* rotpk_hash_size)
{
    enum tfm_plat_err_t err;
    size_t otp_size;

    err = tfm_plat_otp_read(id, *rotpk_hash_size, rotpk_hash);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    err = tfm_plat_otp_get_size(id, &otp_size);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    *rotpk_hash_size = otp_size;

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t
tfm_plat_get_rotpk_hash(uint8_t image_id,
                        uint8_t *rotpk_hash,
                        uint32_t *rotpk_hash_size)
{
    /* Assumes BL2 ROTPK are contiguous */
    if (image_id < MCUBOOT_IMAGE_NUMBER) {
        return get_rotpk_hash(PLAT_OTP_ID_BL2_ROTPK_0 + image_id, rotpk_hash,
                              rotpk_hash_size);
    }

    return TFM_PLAT_ERR_INVALID_INPUT;
}

#endif /* BL2 */
