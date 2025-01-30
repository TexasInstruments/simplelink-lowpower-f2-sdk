/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "crypto.h"
#include "otp.h"
#include "boot_hal.h"
#include "boot_measurement.h"
#include "psa/crypto.h"
#include "uart_stdout.h"
#include "fih.h"
#include "util.h"
#include "log.h"
#include "image.h"
#include "region_defs.h"
#include "pq_crypto.h"
#include "tfm_plat_nv_counters.h"
#include "tfm_plat_otp.h"
#include <string.h>

/* Disable both semihosting code and argv usage for main */
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
__asm("  .global __ARM_use_no_argv\n");
#endif

#if defined(TFM_MEASURED_BOOT_API) || !defined(TFM_BL1_PQ_CRYPTO)
static uint8_t computed_bl2_hash[BL2_HASH_SIZE];
#endif

#ifdef TFM_MEASURED_BOOT_API
#if (BL2_HASH_SIZE == 32)
#define BL2_HASH_ALG  PSA_ALG_SHA_256
#elif (BL2_HASH_SIZE == 64)
#define BL2_HASH_ALG  PSA_ALG_SHA_512
#else
#error "The specified BL2_HASH_SIZE is not supported with measured boot."
#endif /* BL2_HASH_SIZE */

static void collect_boot_measurement(const struct bl1_2_image_t *image)
{
    struct boot_measurement_metadata bl2_metadata = {
        .measurement_type = BL2_HASH_ALG,
        .signer_id = { 0 },
        .signer_id_size = BL2_HASH_SIZE,
        .sw_type = "BL2",
        .sw_version = {
            image->protected_values.version.major,
            image->protected_values.version.minor,
            image->protected_values.version.revision,
            image->protected_values.version.build_num,
        },
    };

#ifdef TFM_BL1_PQ_CRYPTO
    /* Get the public key hash as the signer ID */
    if (pq_crypto_get_pub_key_hash(TFM_BL1_KEY_ROTPK_0, bl2_metadata.signer_id,
                                   sizeof(bl2_metadata.signer_id),
                                   &bl2_metadata.signer_id_size)) {
        BL1_LOG("[WRN] Signer ID missing in measurement of BL2\r\n");
    }
#endif

    /* Save the boot measurement of the BL2 image. */
    if (boot_store_measurement(BOOT_MEASUREMENT_SLOT_BL2, computed_bl2_hash,
                               BL2_HASH_SIZE, &bl2_metadata, true)) {
        BL1_LOG("[WRN] Failed to store boot measurement of BL2\r\n");
    }
}
#endif /* TFM_MEASURED_BOOT_API */

#ifndef TFM_BL1_PQ_CRYPTO
static fih_int image_hash_check(struct bl1_2_image_t *img)
{
    enum tfm_plat_err_t plat_err;
    uint8_t stored_bl2_hash[BL2_HASH_SIZE];
    fih_int fih_rc = FIH_FAILURE;

    plat_err = tfm_plat_otp_read(PLAT_OTP_ID_BL2_IMAGE_HASH, BL2_HASH_SIZE,
                                 stored_bl2_hash);
    fih_rc = fih_int_encode_zero_equality(plat_err);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(FIH_FAILURE);
    }

    FIH_CALL(bl_fih_memeql, fih_rc, computed_bl2_hash, stored_bl2_hash,
                                    BL2_HASH_SIZE);
    FIH_RET(fih_rc);
}
#endif /* !TFM_BL1_PQ_CRYPTO */

static fih_int is_image_security_counter_valid(struct bl1_2_image_t *img)
{
    uint32_t security_counter;
    fih_int fih_rc;
    enum tfm_plat_err_t plat_err;

    plat_err = tfm_plat_read_nv_counter(PLAT_NV_COUNTER_BL1_0,
                                        sizeof(security_counter),
                                        (uint8_t *)&security_counter);
    fih_rc = fih_int_encode_zero_equality(plat_err);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(FIH_FAILURE);
    }

    /* Encodes 0 to true and 1 to false, so the actual comparison is flipped */
    FIH_RET(
        fih_int_encode_zero_equality(security_counter
                                     > img->protected_values.security_counter));
}

static fih_int is_image_signature_valid(struct bl1_2_image_t *img)
{
    fih_int fih_rc = FIH_FAILURE;

    /* Calculate the image hash for measured boot and/or a hash-locked image */
#if defined(TFM_MEASURED_BOOT_API) || !defined(TFM_BL1_PQ_CRYPTO)
    FIH_CALL(bl1_sha256_compute, fih_rc, (uint8_t *)&img->protected_values,
                                         sizeof(img->protected_values),
                                         computed_bl2_hash);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(fih_rc);
    }
#endif

#ifdef TFM_BL1_PQ_CRYPTO
    FIH_CALL(pq_crypto_verify, fih_rc, TFM_BL1_KEY_ROTPK_0,
                                       (uint8_t *)&img->protected_values,
                                       sizeof(img->protected_values),
                                       img->header.sig,
                                       sizeof(img->header.sig));
#else
    FIH_CALL(image_hash_check, fih_rc, img);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(FIH_FAILURE);
    }
#endif /* TFM_BL1_PQ_CRYPTO */

    FIH_RET(fih_rc);
}

static fih_int validate_image_at_addr(struct bl1_2_image_t *image)
{
    fih_int fih_rc = FIH_FAILURE;
    enum tfm_plat_err_t plat_err;

    FIH_CALL(is_image_signature_valid, fih_rc, image);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        BL1_LOG("[ERR] BL2 image signature failed to validate\r\n");
        FIH_RET(FIH_FAILURE);
    }
    FIH_CALL(is_image_security_counter_valid, fih_rc, image);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        BL1_LOG("[ERR] BL2 image security_counter failed to validate\r\n");
        FIH_RET(FIH_FAILURE);
    }

    /* TODO work out if the image actually boots before updating the counter */
    plat_err = tfm_plat_set_nv_counter(PLAT_NV_COUNTER_BL1_0,
                                       image->protected_values.security_counter);
    fih_rc = fih_int_encode_zero_equality(plat_err);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        BL1_LOG("[ERR] NV counter update failed\r\n");
        FIH_RET(FIH_FAILURE);
    }

    FIH_RET(FIH_SUCCESS);
}

static fih_int copy_and_decrypt_image(uint32_t image_id)
{
    int rc;
    struct bl1_2_image_t *image_to_decrypt;
    struct bl1_2_image_t *image_after_decrypt =
        (struct bl1_2_image_t *)BL2_IMAGE_START;
    uint8_t key_buf[32];
    uint8_t label[] = "BL2_DECRYPTION_KEY";

#ifdef TFM_BL1_MEMORY_MAPPED_FLASH
    /* If we have memory-mapped flash, we can do the decrypt directly from the
     * flash and output to the SRAM. This is significantly faster if the AES
     * invocation calls through to a crypto accelerator with a DMA, and slightly
     * faster otherwise.
     */
    image_to_decrypt = (struct bl1_2_image_t *)(FLASH_BASE_ADDRESS +
                       bl1_image_get_flash_offset(image_id));

    /* Copy everything that isn't encrypted, to prevent TOCTOU attacks and
     * simplify logic.
     */
    memcpy(image_after_decrypt, image_to_decrypt,
           sizeof(struct bl1_2_image_t) -
           sizeof(image_after_decrypt->protected_values.encrypted_data));
#else
    /* If the flash isn't memory-mapped, defer to the flash driver to copy the
     * entire block in to SRAM. We'll then do the decrypt in-place.
     */
    bl1_image_copy_to_sram(image_id, (uint8_t *)BL2_IMAGE_START);
    image_to_decrypt = (struct bl1_2_image_t *)BL2_IMAGE_START;
#endif /* TFM_BL1_MEMORY_MAPPED_FLASH */

    /* As the security counter is an attacker controlled parameter, bound the
     * values to a sensible range. In this case, we choose 1024 as the bound as
     * it is the same as the max amount of signatures as a H=10 LMS key.
     */
    if (image_after_decrypt->protected_values.security_counter >= 1024) {
        FIH_RET(FIH_FAILURE);
    }

    /* The image security counter is used as a KDF input */
    rc = bl1_derive_key(TFM_BL1_KEY_BL2_ENCRYPTION, label, sizeof(label),
                        (uint8_t *)&image_after_decrypt->protected_values.security_counter,
                        sizeof(image_after_decrypt->protected_values.security_counter),
                        key_buf, sizeof(key_buf));
    if (rc) {
        FIH_RET(fih_int_encode_zero_equality(rc));
    }

    rc = bl1_aes_256_ctr_decrypt(TFM_BL1_KEY_USER, key_buf,
                                 image_after_decrypt->header.ctr_iv,
                                 (uint8_t *)&image_to_decrypt->protected_values.encrypted_data,
                                 sizeof(image_after_decrypt->protected_values.encrypted_data),
                                 (uint8_t *)&image_after_decrypt->protected_values.encrypted_data);
    if (rc) {
        FIH_RET(fih_int_encode_zero_equality(rc));
    }

    if (image_after_decrypt->protected_values.encrypted_data.decrypt_magic
            != BL1_2_IMAGE_DECRYPT_MAGIC_EXPECTED) {
        FIH_RET(FIH_FAILURE);
    }

    FIH_RET(FIH_SUCCESS);
}

static fih_int validate_image(uint32_t image_id)
{
    fih_int fih_rc = FIH_FAILURE;
    struct bl1_2_image_t *image;

    FIH_CALL(copy_and_decrypt_image, fih_rc, image_id);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        BL1_LOG("[ERR] BL2 image failed to decrypt\r\n");
        FIH_RET(FIH_FAILURE);
    }
    image = (struct bl1_2_image_t *)BL2_IMAGE_START;

    BL1_LOG("[INF] BL2 image decrypted successfully\r\n");

    FIH_CALL(validate_image_at_addr, fih_rc, image);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        BL1_LOG("[ERR] BL2 image failed to validate\r\n");
        FIH_RET(FIH_FAILURE);
    }

    BL1_LOG("[INF] BL2 image validated successfully\r\n");

    FIH_RET(FIH_SUCCESS);
}

int main(void)
{
    fih_int fih_rc = FIH_FAILURE;

    fih_rc = fih_int_encode_zero_equality(boot_platform_init());
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_PANIC;
    }
    BL1_LOG("[INF] starting TF-M bl1_2\r\n");

    fih_rc = fih_int_encode_zero_equality(boot_platform_post_init());
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_PANIC;
    }

#ifdef TEST_BL1_2
    run_bl1_2_testsuite();
#endif /* TEST_BL1_2 */

    fih_rc = fih_int_encode_zero_equality(boot_platform_pre_load(0));
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_PANIC;
    }

    BL1_LOG("[INF] Attempting to boot image 0\r\n");
    FIH_CALL(validate_image, fih_rc, 0);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        BL1_LOG("[INF] Attempting to boot image 1\r\n");
        FIH_CALL(validate_image, fih_rc, 1);
        if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
            FIH_PANIC;
        }
    }

    fih_rc = fih_int_encode_zero_equality(boot_platform_post_load(0));
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_PANIC;
    }

#ifdef TFM_MEASURED_BOOT_API
    /* At this point there is a valid and decrypted BL2 image in the RAM at
     * address BL2_IMAGE_START.
     */
    collect_boot_measurement((const struct bl1_2_image_t *)BL2_IMAGE_START);
#endif /* TFM_MEASURED_BOOT_API */

    BL1_LOG("[INF] Jumping to BL2\r\n");
    boot_platform_quit((struct boot_arm_vector_table *)BL2_CODE_START);

    FIH_PANIC;
}
