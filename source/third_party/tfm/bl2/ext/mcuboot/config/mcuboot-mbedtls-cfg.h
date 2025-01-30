/*
 *  Minimal configuration for using TLS in the bootloader
 *
 *  Copyright (C) 2006-2023, Arm Limited. All rights reserved.
 *  Copyright (C) 2016, Linaro Ltd
 *
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

/*
 * Original code taken from mcuboot project at:
 * https://github.com/mcu-tools/mcuboot
 * Git SHA of the original version: ac55554059147fff718015be9f4bd3108123f50a
 */

/*
 * Minimal configuration for using mbed TLS in the bootloader
 *
 * - RSA signature verification
 * - ECDSA signature verification
 * - Optionally, enable support for PSA Crypto APIs
 */

#ifndef __MCUBOOT_MBEDTLS_CFG__
#define __MCUBOOT_MBEDTLS_CFG__

#if defined(MCUBOOT_USE_PSA_CRYPTO)
/* Enable PSA Crypto Core without support for the permanent storage
 * Don't define MBEDTLS_PSA_CRYPTO_STORAGE_C to make sure that support
 * for permanent keys is not enabled, as it is not available during boot
 */
#define MBEDTLS_PSA_CRYPTO_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_PK_WRITE_C
#define MBEDTLS_PK_C
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG
#endif /* MCUBOOT_USE_PSA_CRYPTO */

#if defined(MCUBOOT_SIGN_RSA)
#define MBEDTLS_RSA_C
#define MBEDTLS_PKCS1_V21
/* Save RAM by adjusting to our exact needs */
#if MCUBOOT_SIGN_RSA_LEN == 3072
#define MBEDTLS_MPI_MAX_SIZE 384
#else /* RSA2048 */
#define MBEDTLS_MPI_MAX_SIZE 256
#endif
#endif /* MCUBOOT_SIGN_RSA */

#if defined(MCUBOOT_SIGN_EC384)
#define MBEDTLS_ECP_DP_SECP384R1_ENABLED
/* When the image is signed with EC-P384 the image hash
 * is calculated using SHA-384
 */
#define MBEDTLS_SHA512_C
#define MBEDTLS_SHA384_C
#else
/* All the other supported signing algorithms use SHA-256 to compute the image hash */
#define MBEDTLS_SHA256_C
#define MBEDTLS_SHA224_C
#endif /* MCUBOOT_SIGN_EC384 */

#ifdef MCUBOOT_SIGN_EC256
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#endif /* MCUBOOT_SIGN_EC256 */

/* System support */
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_MEMORY_BUFFER_ALLOC_C
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES

#define MBEDTLS_PLATFORM_EXIT_ALT
#define MBEDTLS_PLATFORM_PRINTF_ALT


/* mbed TLS modules */
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_MD_C
#define MBEDTLS_OID_C
#define MBEDTLS_AES_C
#define MBEDTLS_CIPHER_MODE_CTR
#if defined(MCUBOOT_SIGN_EC256) || \
    defined(MCUBOOT_SIGN_EC384)
#define MBEDTLS_ECP_C
#define MBEDTLS_ECP_NIST_OPTIM
#define MBEDTLS_ECDSA_C
#endif

#define MBEDTLS_SSL_MAX_CONTENT_LEN 1024

/* Save ROM and a few bytes of RAM by specifying our own ciphersuite list */
#define MBEDTLS_SSL_CIPHERSUITES MBEDTLS_TLS_ECJPAKE_WITH_AES_128_CCM_8

#ifdef CRYPTO_HW_ACCELERATOR_OTP_PROVISIONING
#ifndef MBEDTLS_CIPHER_C
#define MBEDTLS_CIPHER_C
#endif
#define MBEDTLS_CCM_C
#define MBEDTLS_ECDSA_C
#define MBEDTLS_ECP_C
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECP_DP_CURVE25519_ENABLED
#endif /* CRYPTO_HW_ACCELERATOR_OTP_PROVISIONING */

#ifdef CRYPTO_HW_ACCELERATOR
#ifndef LEGACY_DRIVER_API_ENABLED
/*
 * Forcing the legacy driver API enabled all the time regardless of
 * cmake configuration in BL2.
 */
#define LEGACY_DRIVER_API_ENABLED
#warning "Use legacy driver API for BL2"
#include "mbedtls_accelerator_config.h"
#undef LEGACY_DRIVER_API_ENABLED
#else
#include "mbedtls_accelerator_config.h"
#endif /* !LEGACY_DRIVER_API_ENABLED */
#endif

#endif /* __MCUBOOT_MBEDTLS_CFG__ */
