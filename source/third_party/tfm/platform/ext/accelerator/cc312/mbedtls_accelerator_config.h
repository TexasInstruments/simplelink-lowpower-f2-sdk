/*
 * Copyright (c) 2019-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef MBEDTLS_ACCELERATOR_CONF_H
#define MBEDTLS_ACCELERATOR_CONF_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* RNG Config */
#undef MBEDTLS_ENTROPY_NV_SEED
#undef MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES

#define MBEDTLS_ENTROPY_C
#define MBEDTLS_CTR_DRBG_C

#define MBEDTLS_ENTROPY_HARDWARE_ALT

#ifndef MCUBOOT_SIGN_EC384
#define MBEDTLS_ENTROPY_FORCE_SHA256
#endif

#ifdef USE_MBEDTLS_CRYPTOCELL
#define MBEDTLS_MD_C
#endif

/* Main Config */
#ifdef LEGACY_DRIVER_API_ENABLED
#ifdef MBEDTLS_DHM_C
#define MBEDTLS_DHM_ALT
#endif /* MBEDTLS_DHM_C */

#ifdef MBEDTLS_NIST_KW_C
#define MBEDTLS_NIST_KW_ALT
#endif /* MBEDTLS_NIST_KW_C */

#ifdef MBEDTLS_POLY1305_C
#define MBEDTLS_POLY1305_ALT
#endif /* MBEDTLS_POLY1305_C */
#endif /* LEGACY_DRIVER_API_ENABLED */

/****************************************************************/
/* Infer PSA requirements from Mbed TLS capabilities */
/****************************************************************/
#ifndef MBEDTLS_PSA_CRYPTO_CONFIG

#define MBEDTLS_AES_C

/* The CC312 does not support CFB mode */
#ifdef MBEDTLS_CIPHER_MODE_CFB
#undef MBEDTLS_CIPHER_MODE_CFB
#endif /* MBEDTLS_CIPHER_MODE_CFB */

#ifdef LEGACY_DRIVER_API_ENABLED

#ifdef MBEDTLS_AES_C
#define MBEDTLS_AES_ALT
#define MBEDTLS_AES_SETKEY_ENC_ALT
#define MBEDTLS_AES_SETKEY_DEC_ALT
#define MBEDTLS_AES_ENCRYPT_ALT
#define MBEDTLS_AES_DECRYPT_ALT
#endif /* MBEDTLS_AES_C */

#ifdef MBEDTLS_ARIA_C
#define MBEDTLS_ARIA_ALT
#endif /* MBEDTLS_ARIA_C */

#ifdef MBEDTLS_CCM_C
#define MBEDTLS_CCM_ALT
#endif /* MBEDTLS_CCM_C */

#ifdef MBEDTLS_CHACHA20_C
#define MBEDTLS_CHACHA20_ALT
#endif /* MBEDTLS_CHACHA20_C */

#ifdef MBEDTLS_CHACHAPOLY_C
#define MBEDTLS_CHACHAPOLY_ALT
#endif /* MBEDTLS_CHACHAPOLY_C */

#ifdef MBEDTLS_CMAC_C
#define MBEDTLS_CMAC_ALT
#endif /* MBEDTLS_CMAC_C */

#ifdef MBEDTLS_ECDH_C
#define MBEDTLS_ECDH_GEN_PUBLIC_ALT
#define MBEDTLS_ECDH_COMPUTE_SHARED_ALT
#endif /* MBEDTLS_ECDH_C */

#ifdef MBEDTLS_ECDSA_C
#define MBEDTLS_ECDSA_VERIFY_ALT
#define MBEDTLS_ECDSA_SIGN_ALT

#ifndef CRYPTO_HW_ACCELERATOR_OTP_PROVISIONING
#define MBEDTLS_ECDSA_GENKEY_ALT
#endif
#endif /* MBEDTLS_ECDSA_C */

#ifdef MBEDTLS_GCM_C
#define MBEDTLS_GCM_ALT
#endif /* MBEDTLS_GCM_C */

#ifdef MBEDTLS_SHA1_C
#define MBEDTLS_SHA1_ALT
#define MBEDTLS_SHA1_PROCESS_ALT
#endif /* MBEDTLS_SHA1_C */

#ifdef MBEDTLS_SHA256_C
#define MBEDTLS_SHA256_ALT
#define MBEDTLS_SHA256_PROCESS_ALT
#endif /* MBEDTLS_SHA256_C */

#endif /* LEGACY_DRIVER_API_ENABLED */

#ifdef MBEDTLS_RSA_C
#ifdef LEGACY_DRIVER_API_ENABLED
#define MBEDTLS_RSA_ALT
#define MBEDTLS_PK_RSA_ALT_SUPPORT
#endif /* LEGACY_DRIVER_API_ENABLED */
#define MBEDTLS_GENPRIME
#endif /* MBEDTLS_RSA_C */

#endif /* MBEDTLS_PSA_CRYPTO_CONFIG */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MBEDTLS_ACCELERATOR_CONF_H */
