/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef CC3XX_CONFIG_H
#define CC3XX_CONFIG_H

#ifndef CC3XX_CONFIG_BASE_ADDRESS
#include "platform_base_address.h"
#define CC3XX_CONFIG_BASE_ADDRESS (CC3XX_BASE_S)
#endif /* CC3XX_CONFIG_BASE_ADDRESS */

/* Whether the SHA256 hash support is enabled */
#define CC3XX_CONFIG_HASH_SHA256_ENABLE

/* Whether the SHA224 hash support is enabled */
/* #define CC3XX_CONFIG_HASH_SHA224_ENABLE */

/* Whether the SHA1 hash support is enabled */
/* #define CC3XX_CONFIG_HASH_SHA1_ENABLE */

/* Whether the AES CTR support is enabled */
#define CC3XX_CONFIG_AES_CTR_ENABLE

/* Whether the AES ECB support is enabled */
#define CC3XX_CONFIG_AES_ECB_ENABLE

/* Whether the AES CBC support is enabled */
/* #define CC3XX_CONFIG_AES_CBC_ENABLE */

/* Whether the AES GCM support is enabled */
/* #define CC3XX_CONFIG_AES_GCM_ENABLE */
/* #define CC3XX_CONFIG_AES_GCM_VARIABLE_IV_ENABLE */

/* Whether the AES CMAC support is enabled */
#define CC3XX_CONFIG_AES_CMAC_ENABLE

/* Whether the AES CCM support is enabled */
#define CC3XX_CONFIG_AES_CCM_ENABLE

/* Whether the AES tunnelling support is enabled. Without this, running CCM mode
 * AES will instead only run the CBC_MAC operation with the CCM IVs, with the
 * CTR decryption having to be done seperately. */
#define CC3XX_CONFIG_AES_TUNNELLING_ENABLE

/* Whether an external key-loader should be invoked instead of the standard AES
 * hardware key loading mechanism
 */
#define CC3XX_CONFIG_AES_EXTERNAL_KEY_LOADER

/* Whether CHACHA is enabled */
/* #define CC3XX_CONFIG_CHACHA_ENABLE */

/* Whether CHACHA_POLY1305 is enabled */
/* #define CC3XX_CONFIG_CHACHA_POLY1305_ENABLE */

/* Whether DMA remapping is enabled */
#define CC3XX_CONFIG_DMA_REMAP_ENABLE

/* Whether CC will WFI instead of busy-wait looping while waiting for crypto
 * operations to complete.
 */
/* #define CC3XX_CONFIG_DMA_WFI_WAIT_ENABLE */

/* How many DMA remap regions are available */
#ifndef CC3XX_CONFIG_DMA_REMAP_REGION_AM
#define CC3XX_CONFIG_DMA_REMAP_REGION_AM 4
#endif /* CC3XX_CONFIG_DMA_REMAP_REGION_AM */

/* Whether RNG is enabled */
#define CC3XX_CONFIG_RNG_ENABLE

/* Whether the CTR_DRBG is enabled through the generic interface */
#define CC3XX_CONFIG_DRBG_CTR_ENABLE
/* Whether the HMAC_DRBG is enabled through the generic interface */
#define CC3XX_CONFIG_DRBG_HMAC_ENABLE
/* Whether the HASH_DRBG is enabled through the generic interface */
#define CC3XX_CONFIG_DRBG_HASH_ENABLE

/* Whether an external TRNG should be used in place of the standard CC3XX TRNG */
#ifdef RSS_OTP_TRNG
#define CC3XX_CONFIG_RNG_EXTERNAL_TRNG
#endif /* RSS_OTP_TRNG */

/* The number of times the TRNG will be re-read when it fails a statical test
 * before an error is returned.
 */
#ifndef CC3XX_CONFIG_RNG_MAX_ATTEMPTS
#define CC3XX_CONFIG_RNG_MAX_ATTEMPTS 16
#endif /* CC3XX_CONFIG_RNG_MAX_ATTEMPTS */

/* This is the number of cycles between consecutive samples of the oscillator
 * output. It needs to be set to a _reasonably_ large number, though It's
 * unclear exactly what sort of number is reasonable. In general, if the
 * statistical tests keep failing then increase it, if the RNG is too slow then
 * decrease it.  A sensible default is set here, and has worked correctly with a
 * variety of cc3xx implementations.
 */
#ifndef CC3XX_CONFIG_RNG_SUBSAMPLING_RATE
#define CC3XX_CONFIG_RNG_SUBSAMPLING_RATE 0x1337
#endif /* !CC_RNG_SUBSAMPLING_RATE */

/* Between 0 and 3 inclusive. 0 should be the fastest oscillator ring */
#ifndef CC3XX_CONFIG_RNG_RING_OSCILLATOR_ID
#define CC3XX_CONFIG_RNG_RING_OSCILLATOR_ID 0
#endif /* !CC_RNG_RING_OSCILLATOR_ID */

/* How many virtual registers can be allocated in the PKA engine */
#ifndef CC3XX_CONFIG_PKA_MAX_VIRT_REG_AMOUNT
#define CC3XX_CONFIG_PKA_MAX_VIRT_REG_AMOUNT 64
#endif /* CC3XX_CONFIG_PKA_MAX_VIRT_REG_AMOUNT */

/* Whether DPA mitigations are enabled. Has a code-size and performance cost */
#define CC3XX_CONFIG_DPA_MITIGATIONS_ENABLE

/* Whether DFA mitigations are enabled. Has a code-size and performance cost */
#define CC3XX_CONFIG_DFA_MITIGATIONS_ENABLE

/* Whether an external secure word copying function (for copying keys etc) will
 * be provided by the platform
 */
#define CC3XX_CONFIG_STDLIB_EXTERNAL_SECURE_WORD_COPY

#ifndef CC3XX_CONFIG_STDLIB_LFSR_MAX_ATTEMPTS
#define CC3XX_CONFIG_STDLIB_LFSR_MAX_ATTEMPTS 128
#endif /* CC3XX_CONFIG_STDLIB_LFSR_MAX_ATTEMPTS */

#endif /* CC3XX_CONFIG_H */
