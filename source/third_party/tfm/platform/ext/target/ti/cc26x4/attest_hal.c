/*
 * Copyright (c) 2022, Texas Instruments Incorporated. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "tfm_attest_hal.h"
#include "tfm_plat_boot_seed.h"
#include "tfm_plat_device_id.h"

/*
 * TI-TFM: Alternate implementation to read attestation claims from attest_region_t
 * This enables Attestation partition to read security_lifecycle, implementation_id,
 * hw_version, and verification_service_url from dedicated Flash region
 */
#define USE_FLASH_DATA

#ifdef USE_FLASH_DATA

typedef struct
{
     /* Initialized to 0xFFFF_FFFF, write zeroes to progress state */
     uint32_t security_lifecycle;
     /* Immutable params */
     uint8_t implementation_id_length;
     uint8_t implementation_id[IMPLEMENTATION_ID_MAX_SIZE];
     uint8_t hw_version_length;
     uint8_t hw_version[HW_VERSION_MAX_SIZE];
     uint8_t verification_service_url_length;
     uint8_t verification_service_url[64];
} attest_region_t;

#define ATTEST_REGION_FLASH_ADDR  0x700

static attest_region_t *attest_region = (attest_region_t *)ATTEST_REGION_FLASH_ADDR;

#else

/* Example verification service URL for initial attestation token */
static const char verification_service_url[] = "www.trustedfirmware.org";


static const uint8_t implementation_id[] = {
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB,
    0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
    0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD,
};

static const uint8_t example_ean_13[] = "060456527282910010";

#endif

/*!
 * \def BOOT_SEED
 *
 * \brief Fixed value for boot seed used for test.
 */
#define BOOT_SEED   0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, \
                    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, \
                    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, \
                    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF

static const uint8_t boot_seed[BOOT_SEED_SIZE] = {BOOT_SEED};


/* Example profile definition document for initial attestation token */
static const char attestation_profile_definition[] = "PSA_IOT_PROFILE_1";


enum tfm_security_lifecycle_t tfm_attest_hal_get_security_lifecycle(void)
{
#ifdef USE_FLASH_DATA
    /* TI-TFM: Obtain security lifecycle from Flash */
    switch(attest_region->security_lifecycle)
    {
        case 0xFFFFFF00:
            return TFM_SLC_PSA_ROT_PROVISIONING;
        case 0xFFFF0000:
            return TFM_SLC_SECURED;
        case 0xFF000000:
            return TFM_SLC_DECOMMISSIONED;
        default:
            return TFM_SLC_UNKNOWN;
    }
#else
    return TFM_SLC_SECURED;
#endif
}

const char *
tfm_attest_hal_get_verification_service(uint32_t *size)
{
#ifdef USE_FLASH_DATA
    /* TI-TFM: Obtain verification service URL from Flash */
    *size = attest_region->verification_service_url_length;

    /* Check if the size of URL - 1 (for null terminator) matches the length of URL */
    if ((*size - 1) != strlen((char *)attest_region->verification_service_url))
    {
        return NULL;
    }
    else
    {
        return (const char *)attest_region->verification_service_url;
    }
#else
    *size = sizeof(verification_service_url) - 1;

    return verification_service_url;
#endif
}

const char *
tfm_attest_hal_get_profile_definition(uint32_t *size)
{
    *size = sizeof(attestation_profile_definition) - 1;

    return attestation_profile_definition;
}

/**
 * \brief Copy data in source buffer to the destination buffer
 *
 * \param[out]  p_dst  Pointer to destation buffer
 * \param[in]   p_src  Pointer to source buffer
 * \param[in]   size   Length of data to be copied
 */
static inline void copy_buf(uint8_t *p_dst, const uint8_t *p_src, size_t size)
{
    uint32_t i;

    for (i = size; i > 0; i--) {
        *p_dst = *p_src;
        p_src++;
        p_dst++;
    }
}

enum tfm_plat_err_t tfm_plat_get_boot_seed(uint32_t size, uint8_t *buf)
{
    /* FixMe: - This getter function must be ported per target platform.
     *        - Platform service shall provide an API to further interact this
     *          getter function to retrieve the boot seed.
     */

    uint8_t *p_dst = buf;
    const uint8_t *p_src = boot_seed;

    if (size != BOOT_SEED_SIZE) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    copy_buf(p_dst, p_src, size);

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t tfm_plat_get_implementation_id(uint32_t *size,
                                                   uint8_t  *buf)
{

    uint32_t impl_id_size;
#ifdef USE_FLASH_DATA
    /* TI-TFM: Obtain implementation id from Flash */
    const uint8_t *p_impl_id = attest_region->implementation_id;
    impl_id_size = attest_region->implementation_id_length;
#else
    const uint8_t *p_impl_id = implementation_id;
    impl_id_size = sizeof(implementation_id);
#endif
    if (*size < impl_id_size) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    copy_buf(buf, p_impl_id, impl_id_size);
    *size = impl_id_size;

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t tfm_plat_get_hw_version(uint32_t *size, uint8_t *buf)
{
    uint32_t hw_version_size;
#ifdef USE_FLASH_DATA
    /* TI-TFM: Obtain hardware version from Flash */
    const uint8_t *p_hw_version = attest_region->hw_version;
    hw_version_size = attest_region->hw_version_length - 1;
#else
    const uint8_t *p_hw_version = example_ean_13;
    hw_version_size = sizeof(example_ean_13) - 1;
#endif

    if (*size < hw_version_size) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    copy_buf(buf, p_hw_version, hw_version_size);
    *size = hw_version_size;

    return TFM_PLAT_ERR_SUCCESS;
}
