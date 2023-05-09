/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_CRYPTO_IFC_H
#define SID_PAL_CRYPTO_IFC_H

/** @file
 *
 * @defgroup sid_pal_lib_crypto sid_pal Cryptography Interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details     Provides cryptography interface to be implemented by platform
 */
#include <sid_error.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SID_PAL_HASH_SHA256 = 1,
    SID_PAL_HASH_SHA512,
} sid_pal_hash_algo_t;

typedef enum {
    SID_PAL_AES_CMAC_128 = 1,
    SID_PAL_AES_CTR_128,
} sid_pal_aes_algo_t;

typedef enum {
    SID_PAL_AEAD_GCM_128 = 1,
    SID_PAL_AEAD_CCM_128,
    SID_PAL_AEAD_CCM_STAR_128,
} sid_pal_aead_algo_t;

typedef enum {
    SID_PAL_ECDH_CURVE25519 = 1,
    SID_PAL_ECDH_SECP256R1,
    SID_PAL_EDDSA_ED25519,
    SID_PAL_ECDSA_SECP256R1,
} sid_pal_ecc_algo_t;

typedef enum {
    SID_PAL_CRYPTO_ENCRYPT = 1,
    SID_PAL_CRYPTO_DECRYPT,
    SID_PAL_CRYPTO_MAC_CALCULATE,
} sid_pal_aes_mode_t;

typedef enum {
    SID_PAL_CRYPTO_SIGN = 1,
    SID_PAL_CRYPTO_VERIFY,
} sid_pal_dsa_mode_t;

typedef struct {
    sid_pal_hash_algo_t algo;
    uint8_t const *data;
    size_t data_size;
    uint8_t *digest;
    size_t digest_size;
} sid_pal_hash_params_t;

typedef struct {
    sid_pal_hash_algo_t algo;
    uint8_t const *key;
    size_t key_size;
    uint8_t const *data;
    size_t data_size;
    uint8_t *digest;
    size_t digest_size;
} sid_pal_hmac_params_t;

typedef struct {
    sid_pal_aes_algo_t algo;
    sid_pal_aes_mode_t mode;
    uint8_t const *key;
    size_t key_size;
    uint8_t const *iv;
    size_t iv_size;
    uint8_t const *in;
    size_t in_size;
    uint8_t *out;
    size_t out_size;
} sid_pal_aes_params_t;

typedef struct {
    sid_pal_aead_algo_t algo;
    sid_pal_aes_mode_t mode;
    uint8_t const *key;
    size_t key_size;
    uint8_t const *iv;
    size_t iv_size;
    uint8_t const *aad;
    size_t aad_size;
    uint8_t const *in;
    size_t in_size;
    uint8_t *out;
    size_t out_size;
    uint8_t *mac;
    size_t mac_size;
} sid_pal_aead_params_t;

typedef struct {
    sid_pal_ecc_algo_t algo;
    sid_pal_dsa_mode_t mode;
    uint8_t const *key;
    size_t key_size;
    uint8_t const *in;
    size_t in_size;
    uint8_t *signature;
    size_t sig_size;
} sid_pal_dsa_params_t;

typedef struct {
    sid_pal_ecc_algo_t algo;
    uint8_t const *prk;
    size_t prk_size;
    uint8_t const *puk;
    size_t puk_size;
    uint8_t *shared_secret;
    size_t shared_secret_sz;
} sid_pal_ecdh_params_t;

typedef struct {
    sid_pal_ecc_algo_t algo;
    uint8_t *prk;
    size_t prk_size;
    uint8_t *puk;
    size_t puk_size;
} sid_pal_ecc_key_gen_params_t;

/**
 * @brief Initialize sid_pal crypto HAL.
 *
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_init(void);

/**
 * @brief Deinitialize crypto HAL.
 *
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_deinit(void);

/**
 * @brief Generate random number.
 *
 * @param[out]  rand   Pointer to rand buffer.
 * @param[in]   size   Size of rand number
 *
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_rand(uint8_t *rand, size_t size);

/**
 * @brief Generate hash.
 *        SHA256 and SHA512 is now supported.
 *
 * @param[in,out]  params  Pointer to the hash parameters.
 *
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_hash(sid_pal_hash_params_t *params);

/**
 * @brief Generate HMAC.
 *        HMAC/SHA256 and HMAC/SHA512 is now supported.
 *
 * @param[in,out]  params  Pointer to the hash parameters.
 *
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_hmac(sid_pal_hmac_params_t* params);

/**
 * @brief Encrypt or decrypt using following AES algorithm.
 *        AES-CMAC
 *        AES-CTR
 *
 * @param[in,out]  params  Pointer to AES parameters.
 *
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_aes_crypt(sid_pal_aes_params_t *params);

/**
 * @brief Encrypt or decrypt using AEAD algorithm.
 *
 * @param[in,out]  params  Pointer to the AEAD parameters.
 *
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_aead_crypt(sid_pal_aead_params_t  *params);

/**
 * @brief Sign or verify elliptic curve digital signature
 *        using given algorithm.
 *
 * @param[in,out]  params  Pointer to the ECC DSA parameters.
 *
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_ecc_dsa(sid_pal_dsa_params_t *params);

/**
 * @brief Generate shared secret using private key and public key
 *
 * @param[in,out]  params  Pointer to the ECDH parameters.
 *
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_ecc_ecdh(sid_pal_ecdh_params_t *params);

/**
 * @brief Generate ECC key pair using  given algorithm.
 *
 * @param[in,out]  params  Generate ECC key pair using given algorithm.
 *
 * @retval  SID_ERROR_NONE    If the function completed successfully.
 *                            Otherwise, an error code is returned.
 */
sid_error_t sid_pal_crypto_ecc_key_gen(sid_pal_ecc_key_gen_params_t *params);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
