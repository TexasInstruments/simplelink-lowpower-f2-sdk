/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "crypto.h"

#include <stdint.h>
#include <string.h>

#include "region_defs.h"
#include "device_definition.h"
#include "otp.h"
#include "fih.h"
#include "cc3xx_drv.h"
#include "kmu_drv.h"

#define KEY_DERIVATION_MAX_BUF_SIZE 128

fih_int bl1_sha256_init(void)
{
    fih_int fih_rc = FIH_FAILURE;

    fih_rc = fih_int_encode_zero_equality(cc3xx_hash_init(CC3XX_HASH_ALG_SHA256));
    if(fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(FIH_FAILURE);
    }

    return FIH_SUCCESS;
}

fih_int bl1_sha256_finish(uint8_t *hash)
{
    uint32_t tmp_buf[32 / sizeof(uint32_t)];

    cc3xx_hash_finish(tmp_buf, 32);

    memcpy(hash, tmp_buf, sizeof(tmp_buf));

    return FIH_SUCCESS;
}

fih_int bl1_sha256_update(uint8_t *data, size_t data_length)
{
    fih_int fih_rc = FIH_FAILURE;

    fih_rc = fih_int_encode_zero_equality(cc3xx_hash_update(data,
                                                            data_length));
    if(fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(FIH_FAILURE);
    }

    return FIH_SUCCESS;
}

fih_int bl1_sha256_compute(const uint8_t *data,
                           size_t data_length,
                           uint8_t *hash)
{
    uint32_t tmp_buf[32 / sizeof(uint32_t)];
    fih_int fih_rc = FIH_FAILURE;

    if (data == NULL || hash == NULL) {
        FIH_RET(FIH_FAILURE);
    }

    fih_rc = fih_int_encode_zero_equality(cc3xx_hash_init(CC3XX_HASH_ALG_SHA256));
    if(fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(FIH_FAILURE);
    }

    fih_rc = fih_int_encode_zero_equality(cc3xx_hash_update(data,
                                                            data_length));
    if(fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(FIH_FAILURE);
    }
    cc3xx_hash_finish(tmp_buf, 32);

    memcpy(hash, tmp_buf, sizeof(tmp_buf));

    FIH_RET(FIH_SUCCESS);
}

static int32_t bl1_key_to_kmu_key(enum tfm_bl1_key_id_t key_id,
                                    enum kmu_hardware_keyslot_t *cc3xx_key_type,
                                    uint8_t **key_buf, size_t key_buf_size)
{
    int32_t rc;

    switch(key_id) {
    case TFM_BL1_KEY_HUK:
        *cc3xx_key_type = KMU_HW_SLOT_HUK;
        *key_buf = NULL;
        break;
    case TFM_BL1_KEY_GUK:
        *cc3xx_key_type = KMU_HW_SLOT_GUK;
        *key_buf = NULL;
        break;
    default:
        rc = bl1_otp_read_key(key_id, *key_buf);
        if (rc) {
            memset(*key_buf, 0, key_buf_size);
            return rc;
        }
        break;
    }

    return 0;
}

int32_t bl1_aes_256_ctr_decrypt(enum tfm_bl1_key_id_t key_id,
                                const uint8_t *key_material,
                                uint8_t *counter,
                                const uint8_t *ciphertext,
                                size_t ciphertext_length,
                                uint8_t *plaintext)
{
    enum kmu_hardware_keyslot_t kmu_key_slot;
    uint32_t key_buf[32 / sizeof(uint32_t)];
    int32_t rc = 0;
    uint8_t *input_key = (uint8_t *)key_buf;
    cc3xx_err_t err;

    if (ciphertext_length == 0) {
        return 0;
    }

    if (counter == NULL || ciphertext == NULL || plaintext == NULL) {
        return -1;
    }

    if ((uintptr_t)counter & 0x3) {
        return -1;
    }

    if (key_material == NULL) {
        rc = bl1_key_to_kmu_key(key_id, &kmu_key_slot, &input_key,
                                sizeof(key_buf));
        if (rc) {
            return rc;
        }
    } else {
        input_key = (uint8_t *)key_material;
    }

    err = cc3xx_aes_init(CC3XX_AES_DIRECTION_DECRYPT, CC3XX_AES_MODE_CTR,
                         kmu_key_slot, (uint32_t *)input_key,
                         CC3XX_AES_KEYSIZE_256, (uint32_t *)counter, 16);
    if (err != CC3XX_ERR_SUCCESS) {
        return 1;
    }

    cc3xx_aes_set_output_buffer(plaintext, ciphertext_length);
    cc3xx_aes_update(ciphertext, ciphertext_length);
    cc3xx_aes_finish(NULL, NULL);

    return 0;
}

int32_t bl1_derive_key(enum tfm_bl1_key_id_t key_id, const uint8_t *label,
                       size_t label_length, const uint8_t *context,
                       size_t context_length, uint8_t *output_key,
                       size_t output_length)
{
    enum kmu_hardware_keyslot_t kmu_key_slot;
    uint32_t key_buf[32 / sizeof(uint32_t)];
    uint8_t *input_key = (uint8_t *)key_buf;
    int32_t rc = 0;
    cc3xx_err_t err;

    rc = bl1_key_to_kmu_key(key_id, &kmu_key_slot, &input_key, sizeof(key_buf));
    if (rc) {
        return rc;
    }

    err = cc3xx_kdf_cmac(kmu_key_slot, (uint32_t *)input_key,
                         CC3XX_AES_KEYSIZE_256, label, label_length, context,
                         context_length, (uint32_t *)output_key, output_length);
    if (err != CC3XX_ERR_SUCCESS) {
        return 1;
    }

    return 0;
}
