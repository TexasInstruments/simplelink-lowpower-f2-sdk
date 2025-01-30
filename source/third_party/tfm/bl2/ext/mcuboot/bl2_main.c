/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 * Copyright (c) 2017-2023 Arm Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mcuboot_config/mcuboot_config.h"
#include <assert.h>
#include "target.h"
#include "tfm_hal_device_header.h"
#include "mbedtls/memory_buffer_alloc.h"
#include "bootutil/security_cnt.h"
#include "bootutil/bootutil_log.h"
#include "bootutil/image.h"
#include "bootutil/bootutil.h"
#include "bootutil/boot_record.h"
#include "bootutil/fault_injection_hardening.h"
#include "flash_map_backend/flash_map_backend.h"
#include "boot_hal.h"
#include "uart_stdout.h"
#include "tfm_plat_otp.h"
#include "tfm_plat_provisioning.h"
#ifdef TEST_BL2
#include "mcuboot_suites.h"
#endif /* TEST_BL2 */

#if defined(MCUBOOT_USE_PSA_CRYPTO)
#include "psa/crypto.h"
#endif

/* Avoids the semihosting issue */
#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
__asm("  .global __ARM_use_no_argv\n");
#endif

#ifdef MCUBOOT_ENCRYPT_RSA
#define BL2_MBEDTLS_MEM_BUF_LEN 0x3000
#else
#define BL2_MBEDTLS_MEM_BUF_LEN 0x2000
#endif

/* Static buffer to be used by mbedtls for memory allocation */
static uint8_t mbedtls_mem_buf[BL2_MBEDTLS_MEM_BUF_LEN];
struct boot_rsp rsp;

static void do_boot(struct boot_rsp *rsp)
{
    struct boot_arm_vector_table *vt;
    uintptr_t flash_base;
    int rc;

    /* The beginning of the image is the ARM vector table, containing
     * the initial stack pointer address and the reset vector
     * consecutively. Manually set the stack pointer and jump into the
     * reset vector
     */
    rc = flash_device_base(rsp->br_flash_dev_id, &flash_base);
    assert(rc == 0);

    if (rsp->br_hdr->ih_flags & IMAGE_F_RAM_LOAD) {
       /* The image has been copied to SRAM, find the vector table
        * at the load address instead of image's address in flash
        */
        vt = (struct boot_arm_vector_table *)(rsp->br_hdr->ih_load_addr +
                                         rsp->br_hdr->ih_hdr_size);
    } else {
        /* Using the flash address as not executing in SRAM */
        vt = (struct boot_arm_vector_table *)(flash_base +
                                         rsp->br_image_off +
                                         rsp->br_hdr->ih_hdr_size);
    }

#if MCUBOOT_LOG_LEVEL > MCUBOOT_LOG_LEVEL_OFF || TEST_BL2
    stdio_uninit();
#endif

    /* This function never returns, because it calls the secure application
     * Reset_Handler().
     */
    boot_platform_quit(vt);
}

int main(void)
{
    fih_ret fih_rc = FIH_FAILURE;
    enum tfm_plat_err_t plat_err;
    int32_t image_id;

    /* Initialise the mbedtls static memory allocator so that mbedtls allocates
     * memory from the provided static buffer instead of from the heap.
     */
    mbedtls_memory_buffer_alloc_init(mbedtls_mem_buf, BL2_MBEDTLS_MEM_BUF_LEN);

#if MCUBOOT_LOG_LEVEL > MCUBOOT_LOG_LEVEL_OFF || TEST_BL2
    stdio_init();
#endif

    /* Perform platform specific initialization */
    if (boot_platform_init() != 0) {
        BOOT_LOG_ERR("Platform init failed");
        FIH_PANIC;
    }

    BOOT_LOG_INF("Starting bootloader");

    plat_err = tfm_plat_otp_init();
    if (plat_err != TFM_PLAT_ERR_SUCCESS) {
        BOOT_LOG_ERR("OTP system initialization failed");
        FIH_PANIC;
    }

    if (tfm_plat_provisioning_is_required()) {
        plat_err = tfm_plat_provisioning_perform();
        if (plat_err != TFM_PLAT_ERR_SUCCESS) {
            BOOT_LOG_ERR("Provisioning failed");
            FIH_PANIC;
        }
    }
    tfm_plat_provisioning_check_for_dummy_keys();

    FIH_CALL(boot_nv_security_counter_init, fih_rc);
    if (FIH_NOT_EQ(fih_rc, FIH_SUCCESS)) {
        BOOT_LOG_ERR("Error while initializing the security counter");
        FIH_PANIC;
    }

    /* Perform platform specific post-initialization */
    if (boot_platform_post_init() != 0) {
        BOOT_LOG_ERR("Platform post init failed");
        FIH_PANIC;
    }

#if defined(MCUBOOT_USE_PSA_CRYPTO)
    /* If the bootloader is configured to use PSA Crypto APIs in the
     * abstraction layer, the component needs to be explicitly initialized
     * before MCUboot APIs, as the crypto abstraction expects that the init
     * has already happened
     */
    psa_status_t status = psa_crypto_init();
    if (status != PSA_SUCCESS) {
        BOOT_LOG_ERR("PSA Crypto init failed with error code %d", status);
        FIH_PANIC;
    }
    BOOT_LOG_INF("PSA Crypto init completed");
#endif /* MCUBOOT_USE_PSA_CRYPTO */

#ifdef TEST_BL2
    (void)run_mcuboot_testsuite();
#endif /* TEST_BL2 */

    /* Images are loaded in reverse order so that the last image loaded is the
     * TF-M image, which means the response is filled correctly.
     */
    for (image_id = MCUBOOT_IMAGE_NUMBER - 1; image_id >= 0; image_id--) {
        if (boot_platform_pre_load(image_id)) {
            BOOT_LOG_ERR("Pre-load step for image %d failed", image_id);
            FIH_PANIC;
        }

        /* Primary goal to zeroize the 'rsp' is to avoid to accidentally load
         * the NS image in case of a fault injection attack. However, it is
         * done anyway as a good practice to sanitize memory.
         */
        memset(&rsp, 0, sizeof(struct boot_rsp));
        FIH_CALL(boot_go_for_image_id, fih_rc, &rsp, image_id);
        if (FIH_NOT_EQ(fih_rc, FIH_SUCCESS)) {
            BOOT_LOG_ERR("Unable to find bootable image");
            FIH_PANIC;
        }

        if (boot_platform_post_load(image_id)) {
            BOOT_LOG_ERR("Post-load step for image %d failed", image_id);
            FIH_PANIC;
        }
    }

    BOOT_LOG_INF("Bootloader chainload address offset: 0x%x",
                 rsp.br_image_off);
    BOOT_LOG_INF("Jumping to the first image slot");
    do_boot(&rsp);

    BOOT_LOG_ERR("Never should get here");
    FIH_PANIC;

    /* Dummy return to be compatible with some check tools */
    return FIH_FAILURE;
}

#if defined(MCUBOOT_USE_PSA_CRYPTO)
/* When MCUBOOT_USE_PSA_CRYPTO is set, the PSA Crypto layer is configured
 * to use an external RNG generator through MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG.
 * But the cryptographic APIs required by BL2 don't require access to randomness
 * hence we can just stub this API to always return an error code
 */
psa_status_t mbedtls_psa_external_get_random(
    mbedtls_psa_external_random_context_t *context,
    uint8_t *output, size_t output_size, size_t *output_length)
{
    return PSA_ERROR_NOT_SUPPORTED;
}
#endif /* MCUBOOT_USE_PSA_CRYPTO */
