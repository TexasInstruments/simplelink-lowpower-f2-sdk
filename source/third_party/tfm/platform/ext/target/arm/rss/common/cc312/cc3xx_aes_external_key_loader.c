/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cc3xx_aes_external_key_loader.h"

#include "cc3xx_stdlib.h"
#include "cc3xx_dev.h"

#include "device_definition.h"

cc3xx_err_t set_key(cc3xx_aes_key_id_t key_id, const uint32_t *key,
                    cc3xx_aes_keysize_t key_size, bool is_tun1)
{
    enum kmu_error_t kmu_err;
    volatile uint32_t *hw_key_buf_ptr;
    size_t key_word_size = 4 + (key_size * 2);

    hw_key_buf_ptr = P_CC3XX->aes.aes_key_0;
#if defined(CC3XX_CONFIG_AES_CCM_ENABLE) && defined(CC3XX_CONFIG_AES_TUNNELLING_ENABLE)
    if (is_tun1) {
        hw_key_buf_ptr = P_CC3XX->aes.aes_key_1;
    }
#endif /* defined(CC3XX_CONFIG_AES_CCM_ENABLE) && defined(CC3XX_CONFIG_AES_TUNNELLING_ENABLE) */

    /* Check if the HOST_FATAL_ERROR mode is enabled */
    if (P_CC3XX->ao.host_ao_lock_bits & 0x1U) {
        return CC3XX_ERR_INVALID_STATE;
    }

    /* Set key0 size */
    if (!is_tun1) {
        P_CC3XX->aes.aes_control &= ~(0b11U << 12);
        P_CC3XX->aes.aes_control |= (key_size & 0b11U) << 12;
    }
#if defined(CC3XX_CONFIG_AES_CCM_ENABLE) && defined(CC3XX_CONFIG_AES_TUNNELLING_ENABLE)
    if (is_tun1) {
        /* Set key1 size */
        P_CC3XX->aes.aes_control &= ~(0b11U << 14);
        P_CC3XX->aes.aes_control |= (key_size & 0b11U) << 14;
    }
#endif /* defined(CC3XX_CONFIG_AES_CCM_ENABLE) && defined(CC3XX_CONFIG_AES_TUNNELLING_ENABLE) */

    if (key != NULL) {
#ifdef CC3XX_CONFIG_DPA_MITIGATIONS_ENABLE
        cc3xx_dpa_hardened_word_copy(hw_key_buf_ptr, key, key_word_size - 1);
        hw_key_buf_ptr[key_word_size - 1] = key[key_word_size - 1];
#else
        hw_key_buf_ptr[0] = key[0];
        hw_key_buf_ptr[1] = key[1];
        hw_key_buf_ptr[2] = key[2];
        hw_key_buf_ptr[3] = key[3];
        if (key_size > CC3XX_AES_KEYSIZE_128) {
            hw_key_buf_ptr[4] = key[4];
            hw_key_buf_ptr[5] = key[5];
        }
        if (key_size > CC3XX_AES_KEYSIZE_192) {
            hw_key_buf_ptr[6] = key[6];
            hw_key_buf_ptr[7] = key[7];
        }
#endif /* CC3XX_CONFIG_DPA_MITIGATIONS_ENABLE */
    } else {
        /* Hardware keys are locked to aes_key_0 */
        if (is_tun1 && key_id < KMU_USER_SLOT_MIN) {
            while(1){}
            return CC3XX_ERR_KEY_IMPORT_FAILED;
        }

        /* For tunnel 1 keys, we assume the keyslot directly after the indicated
         * one is setup to output into AES_KEY_1;
         */
        if (is_tun1) {
            key_id += 1;
        }

        /* It's an error to use an unlocked slot */
        kmu_err = kmu_get_key_export_config_locked(&KMU_DEV_S, key_id);
        if (kmu_err != KMU_ERROR_SLOT_LOCKED) {
            while(1){}
            return CC3XX_ERR_KEY_IMPORT_FAILED;
        }
        kmu_err = kmu_get_key_locked(&KMU_DEV_S, key_id);
        if (kmu_err != KMU_ERROR_SLOT_LOCKED) {
            while(1){}
            return CC3XX_ERR_KEY_IMPORT_FAILED;
        }

        kmu_err = kmu_export_key(&KMU_DEV_S, key_id);
        if (kmu_err != KMU_ERROR_NONE) {
            while(1){}
            return CC3XX_ERR_KEY_IMPORT_FAILED;
        }
    }

    return CC3XX_ERR_SUCCESS;
}
