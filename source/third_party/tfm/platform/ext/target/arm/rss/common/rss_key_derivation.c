/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "rss_key_derivation.h"

#include "device_definition.h"
#include "tfm_plat_otp.h"
#include "dpa_hardened_word_copy.h"
#include "cc3xx_drv.h"
#include "kmu_drv.h"

#include <stdint.h>
#include <string.h>

extern uint8_t computed_bl1_2_hash[];

static struct kmu_key_export_config_t kmu_key0_export_config = {
    CC3XX_BASE_S + 0x400, /* CC3XX AES_KEY_0 register */
    0, /* No delay */
    0x01, /* Increment by 4 bytes with each write */
    KMU_DESTINATION_PORT_WIDTH_32_BITS, /* Write 32 bits with each write */
    KMU_DESTINATION_PORT_WIDTH_8_WRITES, /* Perform 8 writes (total 256 bits) */
    false, /* Don't refresh the masking */
    false, /* Don't disable the masking */
};

static struct kmu_key_export_config_t kmu_key1_export_config = {
    CC3XX_BASE_S + 0x420, /* CC3XX AES_KEY_1 register */
    0, /* No delay */
    0x01, /* Increment by 4 bytes with each write */
    KMU_DESTINATION_PORT_WIDTH_32_BITS, /* Write 32 bits with each write */
    KMU_DESTINATION_PORT_WIDTH_8_WRITES, /* Perform 8 writes (total 256 bits) */
    false, /* Don't refresh the masking */
    false, /* Don't disable the masking */
};


static int rss_get_boot_state(uint8_t *state, size_t state_buf_len,
                              size_t *state_size)
{
    int rc;
    enum plat_otp_lcs_t lcs;
    uint32_t reprovisioning_bits;
    cc3xx_err_t err;

    if (state_buf_len < 32) {
        return 1;
    }

    rc = tfm_plat_otp_read(PLAT_OTP_ID_LCS, sizeof(lcs), (uint8_t *)&lcs);
    if (rc) {
        return rc;
    }

    rc = tfm_plat_otp_read(PLAT_OTP_ID_REPROVISIONING_BITS,
                           sizeof(reprovisioning_bits),
                           (uint8_t *)&reprovisioning_bits);
    if (rc) {
        return rc;
    }

    err = cc3xx_hash_init(CC3XX_HASH_ALG_SHA256);
    if (err != CC3XX_ERR_SUCCESS) {
        return -1;
    }

    err = cc3xx_hash_update((uint8_t *)&lcs, sizeof(lcs));
    if (err != CC3XX_ERR_SUCCESS) {
        return -1;
    }
    err = cc3xx_hash_update((uint8_t *)&reprovisioning_bits, sizeof(reprovisioning_bits));
    if (err != CC3XX_ERR_SUCCESS) {
        return -1;
    }
    err = cc3xx_hash_update(computed_bl1_2_hash, 32);
    if (err != CC3XX_ERR_SUCCESS) {
        return -1;
    }

    cc3xx_hash_finish((uint32_t *)state, SHA256_OUTPUT_SIZE);
    *state_size = SHA256_OUTPUT_SIZE;

    return 0;
}

static int rss_derive_key(enum kmu_hardware_keyslot_t key_id, uint32_t *key_buf,
                          const uint8_t *label, size_t label_len,
                          enum rss_kmu_slot_id_t slot,
                          bool duplicate_into_next_slot)
{
    int rc;
    uint8_t context[32] = {0};
    size_t context_len;
    enum kmu_error_t kmu_err;
    volatile uint32_t *p_kmu_slot_buf;
    volatile uint32_t *p_kmu_secondary_slot_buf;
    size_t kmu_slot_size;

    rc = rss_get_boot_state(context, sizeof(context), &context_len);
    if (rc) {
        return rc;
    }

    kmu_err = kmu_get_key_buffer_ptr(&KMU_DEV_S, slot,
                                     &p_kmu_slot_buf, &kmu_slot_size);
    if (kmu_err != KMU_ERROR_NONE) {
        return -1;
    }

    rc = cc3xx_kdf_cmac(key_id, key_buf, CC3XX_AES_KEYSIZE_256, label,
                        label_len, context, context_len,
                        (uint32_t *)p_kmu_slot_buf, 32);
    if (rc) {
        return rc;
    }

    /* Due to limitations in CryptoCell, any key that needs to be used for
     * AES-CCM needs to be duplicated into a second slot.
     */
    if (duplicate_into_next_slot) {
        kmu_err = kmu_get_key_buffer_ptr(&KMU_DEV_S, slot + 1,
                                         &p_kmu_secondary_slot_buf,
                                         &kmu_slot_size);
        if (kmu_err != KMU_ERROR_NONE) {
            return -2;
        }

        dpa_hardened_word_copy(p_kmu_secondary_slot_buf, p_kmu_slot_buf,
                        kmu_slot_size / sizeof(uint32_t));

        /* TODO lock keyslots once the runtime CC3XX driver supports locked KMU
         * keyslots
         */
        /* kmu_err = kmu_set_key_locked(&KMU_DEV_S, slot + 1); */
        /* if (kmu_err != KMU_ERROR_NONE) { */
        /*     return -3; */
        /* } */

        kmu_err = kmu_set_key_export_config(&KMU_DEV_S, slot + 1,
                                            &kmu_key1_export_config);
        if (kmu_err != KMU_ERROR_NONE) {
            return -4;
        }

        kmu_err = kmu_set_key_export_config_locked(&KMU_DEV_S, slot + 1);
        if (kmu_err != KMU_ERROR_NONE) {
            return -5;
        }
    }

    /* TODO lock keyslots once the runtime CC3XX driver supports locked KMU
     * keyslots
     */
    /* kmu_err = kmu_set_key_locked(&KMU_DEV_S, slot); */
    /* if (kmu_err != KMU_ERROR_NONE) { */
    /*     return -6; */
    /* } */


    kmu_err = kmu_set_key_export_config(&KMU_DEV_S, slot,
                                        &kmu_key0_export_config);
    if (kmu_err != KMU_ERROR_NONE) {
        return -7;
    }

    kmu_err = kmu_set_key_export_config_locked(&KMU_DEV_S, slot);
    if (kmu_err != KMU_ERROR_NONE) {
        return -8;
    }

    return rc;
}

int rss_derive_cpak_seed(enum rss_kmu_slot_id_t slot)
{
    uint8_t cpak_seed_label[] = "BL1_CPAK_SEED_DERIVATION";

    return rss_derive_key(KMU_HW_SLOT_GUK, NULL, cpak_seed_label,
                          sizeof(cpak_seed_label), slot, false);
}

int rss_derive_dak_seed(enum rss_kmu_slot_id_t slot)
{
    uint8_t dak_seed_label[]  = "BL1_DAK_SEED_DERIVATION";

    return rss_derive_key(KMU_HW_SLOT_GUK, NULL, dak_seed_label,
                          sizeof(dak_seed_label), slot, false);
}

int rss_derive_rot_cdi(enum rss_kmu_slot_id_t slot)
{
    uint8_t rot_cdi_label[] = "BL1_ROT_CDI_DERIVATION";

    return rss_derive_key(KMU_HW_SLOT_HUK, NULL, rot_cdi_label,
                          sizeof(rot_cdi_label), slot, false);
}

int rss_derive_vhuk_seed(uint32_t *vhuk_seed, size_t vhuk_seed_buf_len,
                         size_t *vhuk_seed_size)
{
    uint8_t vhuk_seed_label[]  = "VHUK_SEED_DERIVATION";
    int rc;

    if (vhuk_seed_buf_len != 32) {
        return 1;
    }

    rc = cc3xx_kdf_cmac(KMU_HW_SLOT_HUK, NULL, CC3XX_AES_KEYSIZE_256,
                        vhuk_seed_label, sizeof(vhuk_seed_label), NULL, 0,
                        vhuk_seed, 32);
    if (rc) {
        return rc;
    }

    *vhuk_seed_size = 32;

    return 0;
}

int rss_derive_vhuk(const uint8_t *vhuk_seeds, size_t vhuk_seeds_len,
                    enum rss_kmu_slot_id_t slot)
{
    return rss_derive_key(KMU_HW_SLOT_GUK, NULL, vhuk_seeds, vhuk_seeds_len,
                          slot, false);
}

int rss_derive_session_key(const uint8_t *ivs, size_t ivs_len,
                           enum rss_kmu_slot_id_t slot)
{
    return rss_derive_key(KMU_HW_SLOT_GUK, NULL, ivs, ivs_len, slot, true);
}

int derive_using_krtl_or_zero_key(uint8_t *label, size_t label_len,
                                  enum rss_kmu_slot_id_t output_slot)
{
    int rc;
    uint32_t zero_key[8] = {0};
    enum kmu_error_t kmu_err;
    enum lcm_tp_mode_t tp_mode;
    enum lcm_error_t lcm_err;
    enum rss_kmu_slot_id_t input_slot;
    uint32_t *key_buf;

    lcm_err = lcm_get_tp_mode(&LCM_DEV_S, &tp_mode);
    if (lcm_err != LCM_ERROR_NONE) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    switch(tp_mode) {
    case LCM_TP_MODE_PCI:
        input_slot = (enum rss_kmu_slot_id_t)KMU_HW_SLOT_KRTL;
        key_buf = NULL;
        break;
    case LCM_TP_MODE_TCI:
        input_slot = (enum rss_kmu_slot_id_t)0;
        key_buf = zero_key;
        break;
    default:
        return -1;
    }

    rc = rss_derive_key(input_slot, key_buf, label, label_len,
                        output_slot, true);
    if (rc) {
        return rc;
    }

    /* Until we can lock all the keys, just lock the ones only used in
     * BL1/provisioning.
     */
    kmu_err = kmu_set_key_locked(&KMU_DEV_S, output_slot);
    if (kmu_err != KMU_ERROR_NONE) {
        return -1;
    }

    kmu_err = kmu_set_key_locked(&KMU_DEV_S, output_slot + 1);
    if (kmu_err != KMU_ERROR_NONE) {
        return -1;
    }

    return 0;
}

int rss_derive_cm_provisioning_key(enum rss_kmu_slot_id_t slot)
{
    uint8_t cm_provisioning_label[] = "CM_PROVISIONING";

    return derive_using_krtl_or_zero_key(cm_provisioning_label,
                                         sizeof(cm_provisioning_label), slot);
}

int rss_derive_dm_provisioning_key(enum rss_kmu_slot_id_t slot)
{
    uint8_t dm_provisioning_label[] = "DM_PROVISIONING";

    return derive_using_krtl_or_zero_key(dm_provisioning_label,
                                         sizeof(dm_provisioning_label), slot);
}
