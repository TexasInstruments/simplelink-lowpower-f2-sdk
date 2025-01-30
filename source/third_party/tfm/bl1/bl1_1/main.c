/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "crypto.h"
#include "otp.h"
#include "tfm_plat_provisioning.h"
#include "tfm_plat_otp.h"
#include "boot_hal.h"
#include "boot_measurement.h"
#include "psa/crypto.h"
#include "region_defs.h"
#include "log.h"
#include "util.h"
#include "image.h"
#include "fih.h"

/* Disable both semihosting code and argv usage for main */
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
__asm("  .global __ARM_use_no_argv\n");
#endif

uint8_t computed_bl1_2_hash[BL1_2_HASH_SIZE];

#ifdef TFM_MEASURED_BOOT_API
#if (BL1_2_HASH_SIZE == 32)
#define BL1_2_HASH_ALG  PSA_ALG_SHA_256
#elif (BL1_2_HASH_SIZE == 64)
#define BL1_2_HASH_ALG  PSA_ALG_SHA_512
#else
#error "The specified BL1_2_HASH_SIZE is not supported with measured boot."
#endif /* BL1_2_HASH_SIZE */

static void collect_boot_measurement(void)
{
    struct boot_measurement_metadata bl1_2_metadata = {
        .measurement_type = BL1_2_HASH_ALG,
        .signer_id = { 0 },
        .signer_id_size = BL1_2_HASH_SIZE,
        .sw_type = "BL1_2",
        .sw_version = { 0 },
    };

    /* Missing metadata:
     * - image version: not available,
     * - signer ID: the BL1_2 image is not signed.
     */
    if (boot_store_measurement(BOOT_MEASUREMENT_SLOT_BL1_2, computed_bl1_2_hash,
                               BL1_2_HASH_SIZE, &bl1_2_metadata, true)) {
        BL1_LOG("[WRN] Failed to store boot measurement of BL1_2\r\n");
    }
}
#endif /* TFM_MEASURED_BOOT_API */

static fih_int validate_image_at_addr(uint8_t *image)
{
    enum tfm_plat_err_t plat_err;
    uint8_t stored_bl1_2_hash[BL1_2_HASH_SIZE];
    fih_int fih_rc = FIH_FAILURE;

    FIH_CALL(bl1_sha256_compute, fih_rc, image, BL1_2_CODE_SIZE,
                                         computed_bl1_2_hash);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(FIH_FAILURE);
    }

    plat_err = tfm_plat_otp_read(PLAT_OTP_ID_BL1_2_IMAGE_HASH, BL1_2_HASH_SIZE,
                                 stored_bl1_2_hash);
    fih_rc = fih_int_encode_zero_equality(plat_err);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(FIH_FAILURE);
    }

    FIH_CALL(bl_fih_memeql, fih_rc, computed_bl1_2_hash,
                                    stored_bl1_2_hash, BL1_2_HASH_SIZE);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(FIH_FAILURE);
    }

    FIH_RET(FIH_SUCCESS);
}

int main(void)
{
    fih_int fih_rc = FIH_FAILURE;

    fih_rc = fih_int_encode_zero_equality(boot_platform_init());
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_PANIC;
    }
    BL1_LOG("[INF] Starting TF-M BL1_1\r\n");

    if (tfm_plat_provisioning_is_required()) {
        if (tfm_plat_provisioning_perform()) {
            BL1_LOG("[ERR] BL1 provisioning failed\r\n");
            FIH_PANIC;
        }
    }

    tfm_plat_provisioning_check_for_dummy_keys();

    fih_rc = fih_int_encode_zero_equality(boot_platform_post_init());
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_PANIC;
    }

#ifdef TEST_BL1_1
    run_bl1_1_testsuite();
#endif /* TEST_BL1_1 */

    fih_rc = fih_int_encode_zero_equality(boot_platform_pre_load(0));
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_PANIC;
    }

    /* Copy BL1_2 from OTP into SRAM*/
    FIH_CALL(bl1_read_bl1_2_image, fih_rc, (uint8_t *)BL1_2_CODE_START);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_PANIC;
    }

    FIH_CALL(validate_image_at_addr, fih_rc, (uint8_t *)BL1_2_CODE_START);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        BL1_LOG("[ERR] BL1_2 image failed to validate\r\n");
        FIH_PANIC;
    }

    fih_rc = fih_int_encode_zero_equality(boot_platform_post_load(0));
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_PANIC;
    }

#ifdef TFM_MEASURED_BOOT_API
    collect_boot_measurement();
#endif /* TFM_MEASURED_BOOT_API */

    BL1_LOG("[INF] Jumping to BL1_2\r\n");
    /* Jump to BL1_2 */
    boot_platform_quit((struct boot_arm_vector_table *)BL1_2_CODE_START);

    /* This should never happen */
    FIH_PANIC;
}
