/**
 * \file psa/crypto_compat.h
 *
 * \brief PSA cryptography module: Backward compatibility aliases
 *
 * This header declares alternative names for macro and functions.
 * New application code should not use these names.
 * These names may be removed in a future version of Mbed Crypto.
 *
 * \note This file may not be included directly. Applications must
 * include psa/crypto.h.
 */
/*
 *  Copyright The Mbed TLS Contributors
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
 */

#ifndef PSA_CRYPTO_COMPAT_H
#define PSA_CRYPTO_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * To support both openless APIs and psa_open_key() temporarily, define
 * psa_key_handle_t to be equal to mbedtls_svc_key_id_t. Do not mark the
 * type and its utility macros and functions deprecated yet. This will be done
 * in a subsequent phase.
 */
// typedef mbedtls_svc_key_id_t psa_key_handle_t;

#define PSA_KEY_HANDLE_INIT MBEDTLS_SVC_KEY_ID_INIT

/** Check whether an handle is null.
 *
 * \param handle  Handle
 *
 * \return Non-zero if the handle is null, zero otherwise.
 */
static inline int psa_key_handle_is_null(psa_key_handle_t handle)
{
    return (mbedtls_svc_key_id_is_null(handle));
}

#if !defined(MBEDTLS_DEPRECATED_REMOVED)

    /*
     * Mechanism for declaring deprecated values
     */
    #if defined(MBEDTLS_DEPRECATED_WARNING) && !defined(MBEDTLS_PSA_DEPRECATED)
        #define MBEDTLS_PSA_DEPRECATED __attribute__((deprecated))
    #else
        #define MBEDTLS_PSA_DEPRECATED
    #endif

typedef MBEDTLS_PSA_DEPRECATED size_t mbedtls_deprecated_size_t;
typedef MBEDTLS_PSA_DEPRECATED psa_status_t mbedtls_deprecated_psa_status_t;
typedef MBEDTLS_PSA_DEPRECATED psa_key_usage_t mbedtls_deprecated_psa_key_usage_t;
typedef MBEDTLS_PSA_DEPRECATED psa_ecc_family_t mbedtls_deprecated_psa_ecc_family_t;
typedef MBEDTLS_PSA_DEPRECATED psa_dh_family_t mbedtls_deprecated_psa_dh_family_t;
typedef MBEDTLS_PSA_DEPRECATED psa_ecc_family_t psa_ecc_curve_t;
typedef MBEDTLS_PSA_DEPRECATED psa_dh_family_t psa_dh_group_t;
typedef MBEDTLS_PSA_DEPRECATED psa_algorithm_t mbedtls_deprecated_psa_algorithm_t;

    #define PSA_KEY_TYPE_GET_CURVE PSA_KEY_TYPE_ECC_GET_FAMILY
    #define PSA_KEY_TYPE_GET_GROUP PSA_KEY_TYPE_DH_GET_FAMILY

    #define MBEDTLS_DEPRECATED_CONSTANT(type, value) ((mbedtls_deprecated_##type)(value))

    /*
     * Deprecated PSA Crypto error code definitions (PSA Crypto API  <= 1.0 beta2)
     */
    #define PSA_ERROR_UNKNOWN_ERROR         MBEDTLS_DEPRECATED_CONSTANT(psa_status_t, PSA_ERROR_GENERIC_ERROR)
    #define PSA_ERROR_OCCUPIED_SLOT         MBEDTLS_DEPRECATED_CONSTANT(psa_status_t, PSA_ERROR_ALREADY_EXISTS)
    #define PSA_ERROR_EMPTY_SLOT            MBEDTLS_DEPRECATED_CONSTANT(psa_status_t, PSA_ERROR_DOES_NOT_EXIST)
    #define PSA_ERROR_INSUFFICIENT_CAPACITY MBEDTLS_DEPRECATED_CONSTANT(psa_status_t, PSA_ERROR_INSUFFICIENT_DATA)
    #define PSA_ERROR_TAMPERING_DETECTED    MBEDTLS_DEPRECATED_CONSTANT(psa_status_t, PSA_ERROR_CORRUPTION_DETECTED)

    /*
     * Deprecated PSA Crypto numerical encodings (PSA Crypto API  <= 1.0 beta3)
     */
    #define PSA_KEY_USAGE_SIGN   MBEDTLS_DEPRECATED_CONSTANT(psa_key_usage_t, PSA_KEY_USAGE_SIGN_HASH)
    #define PSA_KEY_USAGE_VERIFY MBEDTLS_DEPRECATED_CONSTANT(psa_key_usage_t, PSA_KEY_USAGE_VERIFY_HASH)

    /*
     * Deprecated PSA Crypto size calculation macros (PSA Crypto API  <= 1.0 beta3)
     */
    #define PSA_ASYMMETRIC_SIGNATURE_MAX_SIZE MBEDTLS_DEPRECATED_CONSTANT(size_t, PSA_SIGNATURE_MAX_SIZE)
    #define PSA_ASYMMETRIC_SIGN_OUTPUT_SIZE(key_type, key_bits, alg) \
        MBEDTLS_DEPRECATED_CONSTANT(size_t, PSA_SIGN_OUTPUT_SIZE(key_type, key_bits, alg))
    #define PSA_KEY_EXPORT_MAX_SIZE(key_type, key_bits) \
        MBEDTLS_DEPRECATED_CONSTANT(size_t, PSA_EXPORT_KEY_OUTPUT_SIZE(key_type, key_bits))
    #define PSA_BLOCK_CIPHER_BLOCK_SIZE(type) MBEDTLS_DEPRECATED_CONSTANT(size_t, PSA_BLOCK_CIPHER_BLOCK_LENGTH(type))
    #define PSA_MAX_BLOCK_CIPHER_BLOCK_SIZE   MBEDTLS_DEPRECATED_CONSTANT(size_t, PSA_BLOCK_CIPHER_BLOCK_MAX_SIZE)
    #define PSA_HASH_SIZE(alg)                MBEDTLS_DEPRECATED_CONSTANT(size_t, PSA_HASH_LENGTH(alg))
    #define PSA_MAC_FINAL_SIZE(key_type, key_bits, alg) \
        MBEDTLS_DEPRECATED_CONSTANT(size_t, PSA_MAC_LENGTH(key_type, key_bits, alg))
    #define PSA_ALG_TLS12_PSK_TO_MS_MAX_PSK_LEN MBEDTLS_DEPRECATED_CONSTANT(size_t, PSA_TLS12_PSK_TO_MS_PSK_MAX_SIZE)

/*
 * Deprecated PSA Crypto function names (PSA Crypto API  <= 1.0 beta3)
 */
MBEDTLS_PSA_DEPRECATED static inline psa_status_t psa_asymmetric_sign(psa_key_handle_t key,
                                                                      psa_algorithm_t alg,
                                                                      const uint8_t *hash,
                                                                      size_t hash_length,
                                                                      uint8_t *signature,
                                                                      size_t signature_size,
                                                                      size_t *signature_length)
{
    return psa_sign_hash(key, alg, hash, hash_length, signature, signature_size, signature_length);
}

MBEDTLS_PSA_DEPRECATED static inline psa_status_t psa_asymmetric_verify(psa_key_handle_t key,
                                                                        psa_algorithm_t alg,
                                                                        const uint8_t *hash,
                                                                        size_t hash_length,
                                                                        const uint8_t *signature,
                                                                        size_t signature_length)
{
    return psa_verify_hash(key, alg, hash, hash_length, signature, signature_length);
}

    /*
     * Size-specific elliptic curve families.
     */
    #define PSA_ECC_CURVE_SECP160K1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECP_K1)
    #define PSA_ECC_CURVE_SECP192K1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECP_K1)
    #define PSA_ECC_CURVE_SECP224K1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECP_K1)
    #define PSA_ECC_CURVE_SECP256K1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECP_K1)
    #define PSA_ECC_CURVE_SECP160R1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECP_R1)
    #define PSA_ECC_CURVE_SECP192R1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECP_R1)
    #define PSA_ECC_CURVE_SECP224R1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECP_R1)
    #define PSA_ECC_CURVE_SECP256R1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECP_R1)
    #define PSA_ECC_CURVE_SECP384R1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECP_R1)
    #define PSA_ECC_CURVE_SECP521R1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECP_R1)
    #define PSA_ECC_CURVE_SECP160R2        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECP_R2)
    #define PSA_ECC_CURVE_SECT163K1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECT_K1)
    #define PSA_ECC_CURVE_SECT233K1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECT_K1)
    #define PSA_ECC_CURVE_SECT239K1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECT_K1)
    #define PSA_ECC_CURVE_SECT283K1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECT_K1)
    #define PSA_ECC_CURVE_SECT409K1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECT_K1)
    #define PSA_ECC_CURVE_SECT571K1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECT_K1)
    #define PSA_ECC_CURVE_SECT163R1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECT_R1)
    #define PSA_ECC_CURVE_SECT193R1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECT_R1)
    #define PSA_ECC_CURVE_SECT233R1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECT_R1)
    #define PSA_ECC_CURVE_SECT283R1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECT_R1)
    #define PSA_ECC_CURVE_SECT409R1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECT_R1)
    #define PSA_ECC_CURVE_SECT571R1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECT_R1)
    #define PSA_ECC_CURVE_SECT163R2        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECT_R2)
    #define PSA_ECC_CURVE_SECT193R2        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECT_R2)
    #define PSA_ECC_CURVE_BRAINPOOL_P256R1 MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_BRAINPOOL_P_R1)
    #define PSA_ECC_CURVE_BRAINPOOL_P384R1 MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_BRAINPOOL_P_R1)
    #define PSA_ECC_CURVE_BRAINPOOL_P512R1 MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_BRAINPOOL_P_R1)
    #define PSA_ECC_CURVE_CURVE25519       MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_MONTGOMERY)
    #define PSA_ECC_CURVE_CURVE448         MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_MONTGOMERY)

    /*
     * Curves that changed name due to PSA specification.
     */
    #define PSA_ECC_CURVE_SECP_K1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECP_K1)
    #define PSA_ECC_CURVE_SECP_R1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECP_R1)
    #define PSA_ECC_CURVE_SECP_R2        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECP_R2)
    #define PSA_ECC_CURVE_SECT_K1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECT_K1)
    #define PSA_ECC_CURVE_SECT_R1        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECT_R1)
    #define PSA_ECC_CURVE_SECT_R2        MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_SECT_R2)
    #define PSA_ECC_CURVE_BRAINPOOL_P_R1 MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_BRAINPOOL_P_R1)
    #define PSA_ECC_CURVE_MONTGOMERY     MBEDTLS_DEPRECATED_CONSTANT(psa_ecc_family_t, PSA_ECC_FAMILY_MONTGOMERY)

    /*
     * Finite-field Diffie-Hellman families.
     */
    #define PSA_DH_GROUP_FFDHE2048 MBEDTLS_DEPRECATED_CONSTANT(psa_dh_family_t, PSA_DH_FAMILY_RFC7919)
    #define PSA_DH_GROUP_FFDHE3072 MBEDTLS_DEPRECATED_CONSTANT(psa_dh_family_t, PSA_DH_FAMILY_RFC7919)
    #define PSA_DH_GROUP_FFDHE4096 MBEDTLS_DEPRECATED_CONSTANT(psa_dh_family_t, PSA_DH_FAMILY_RFC7919)
    #define PSA_DH_GROUP_FFDHE6144 MBEDTLS_DEPRECATED_CONSTANT(psa_dh_family_t, PSA_DH_FAMILY_RFC7919)
    #define PSA_DH_GROUP_FFDHE8192 MBEDTLS_DEPRECATED_CONSTANT(psa_dh_family_t, PSA_DH_FAMILY_RFC7919)

    /*
     * Diffie-Hellman families that changed name due to PSA specification.
     */
    #define PSA_DH_GROUP_RFC7919 MBEDTLS_DEPRECATED_CONSTANT(psa_dh_family_t, PSA_DH_FAMILY_RFC7919)
    #define PSA_DH_GROUP_CUSTOM  MBEDTLS_DEPRECATED_CONSTANT(psa_dh_family_t, PSA_DH_FAMILY_CUSTOM)

    /*
     * Deprecated PSA Crypto stream cipher algorithms (PSA Crypto API  <= 1.0 beta3)
     */
    #define PSA_ALG_ARC4     MBEDTLS_DEPRECATED_CONSTANT(psa_algorithm_t, PSA_ALG_STREAM_CIPHER)
    #define PSA_ALG_CHACHA20 MBEDTLS_DEPRECATED_CONSTANT(psa_algorithm_t, PSA_ALG_STREAM_CIPHER)

    /*
     * Renamed AEAD tag length macros (PSA Crypto API  <= 1.0 beta3)
     */
    #define PSA_ALG_AEAD_WITH_DEFAULT_TAG_LENGTH(aead_alg) \
        MBEDTLS_DEPRECATED_CONSTANT(psa_algorithm_t, PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(aead_alg))
    #define PSA_ALG_AEAD_WITH_TAG_LENGTH(aead_alg, tag_length) \
        MBEDTLS_DEPRECATED_CONSTANT(psa_algorithm_t, PSA_ALG_AEAD_WITH_SHORTENED_TAG(aead_alg, tag_length))

    /*
     * Deprecated PSA AEAD output size macros (PSA Crypto API  <= 1.0 beta3)
     */

    /** The tag size for an AEAD algorithm, in bytes.
     *
     * \param alg                 An AEAD algorithm
     *                            (\c PSA_ALG_XXX value such that
     *                            #PSA_ALG_IS_AEAD(\p alg) is true).
     *
     * \return                    The tag size for the specified algorithm.
     *                            If the AEAD algorithm does not have an identified
     *                            tag that can be distinguished from the rest of
     *                            the ciphertext, return 0.
     *                            If the AEAD algorithm is not recognized, return 0.
     */
    #define PSA_AEAD_TAG_LENGTH_1_ARG(alg) \
        MBEDTLS_DEPRECATED_CONSTANT(size_t, PSA_ALG_IS_AEAD(alg) ? PSA_ALG_AEAD_GET_TAG_LENGTH(alg) : 0)

    /** The maximum size of the output of psa_aead_encrypt(), in bytes.
     *
     * If the size of the ciphertext buffer is at least this large, it is
     * guaranteed that psa_aead_encrypt() will not fail due to an
     * insufficient buffer size. Depending on the algorithm, the actual size of
     * the ciphertext may be smaller.
     *
     * \warning This macro may evaluate its arguments multiple times or
     *          zero times, so you should not pass arguments that contain
     *          side effects.
     *
     * \param alg                 An AEAD algorithm
     *                            (\c PSA_ALG_XXX value such that
     *                            #PSA_ALG_IS_AEAD(\p alg) is true).
     * \param plaintext_length    Size of the plaintext in bytes.
     *
     * \return                    The AEAD ciphertext size for the specified
     *                            algorithm.
     *                            If the AEAD algorithm is not recognized, return 0.
     */
    #define PSA_AEAD_ENCRYPT_OUTPUT_SIZE_2_ARG(alg, plaintext_length) \
        MBEDTLS_DEPRECATED_CONSTANT(size_t,                           \
                                    PSA_ALG_IS_AEAD(alg) ? (plaintext_length) + PSA_ALG_AEAD_GET_TAG_LENGTH(alg) : 0)

    /** The maximum size of the output of psa_aead_decrypt(), in bytes.
     *
     * If the size of the plaintext buffer is at least this large, it is
     * guaranteed that psa_aead_decrypt() will not fail due to an
     * insufficient buffer size. Depending on the algorithm, the actual size of
     * the plaintext may be smaller.
     *
     * \warning This macro may evaluate its arguments multiple times or
     *          zero times, so you should not pass arguments that contain
     *          side effects.
     *
     * \param alg                 An AEAD algorithm
     *                            (\c PSA_ALG_XXX value such that
     *                            #PSA_ALG_IS_AEAD(\p alg) is true).
     * \param ciphertext_length   Size of the plaintext in bytes.
     *
     * \return                    The AEAD ciphertext size for the specified
     *                            algorithm.
     *                            If the AEAD algorithm is not recognized, return 0.
     */
    #define PSA_AEAD_DECRYPT_OUTPUT_SIZE_2_ARG(alg, ciphertext_length)                                             \
        MBEDTLS_DEPRECATED_CONSTANT(size_t,                                                                        \
                                    PSA_ALG_IS_AEAD(alg) && (ciphertext_length) > PSA_ALG_AEAD_GET_TAG_LENGTH(alg) \
                                        ? (ciphertext_length)-PSA_ALG_AEAD_GET_TAG_LENGTH(alg)                     \
                                        : 0)

    /** A sufficient output buffer size for psa_aead_update().
     *
     * If the size of the output buffer is at least this large, it is
     * guaranteed that psa_aead_update() will not fail due to an
     * insufficient buffer size. The actual size of the output may be smaller
     * in any given call.
     *
     * \warning This macro may evaluate its arguments multiple times or
     *          zero times, so you should not pass arguments that contain
     *          side effects.
     *
     * \param alg                 An AEAD algorithm
     *                            (\c PSA_ALG_XXX value such that
     *                            #PSA_ALG_IS_AEAD(\p alg) is true).
     * \param input_length        Size of the input in bytes.
     *
     * \return                    A sufficient output buffer size for the specified
     *                            algorithm.
     *                            If the AEAD algorithm is not recognized, return 0.
     */
    /* For all the AEAD modes defined in this specification, it is possible
     * to emit output without delay. However, hardware may not always be
     * capable of this. So for modes based on a block cipher, allow the
     * implementation to delay the output until it has a full block. */
    #define PSA_AEAD_UPDATE_OUTPUT_SIZE_2_ARG(alg, input_length)                                                    \
        MBEDTLS_DEPRECATED_CONSTANT(size_t,                                                                         \
                                    PSA_ALG_IS_AEAD_ON_BLOCK_CIPHER(alg)                                            \
                                        ? PSA_ROUND_UP_TO_MULTIPLE(PSA_BLOCK_CIPHER_BLOCK_MAX_SIZE, (input_length)) \
                                        : (input_length))

    /** A sufficient ciphertext buffer size for psa_aead_finish().
     *
     * If the size of the ciphertext buffer is at least this large, it is
     * guaranteed that psa_aead_finish() will not fail due to an
     * insufficient ciphertext buffer size. The actual size of the output may
     * be smaller in any given call.
     *
     * \param alg                 An AEAD algorithm
     *                            (\c PSA_ALG_XXX value such that
     *                            #PSA_ALG_IS_AEAD(\p alg) is true).
     *
     * \return                    A sufficient ciphertext buffer size for the
     *                            specified algorithm.
     *                            If the AEAD algorithm is not recognized, return 0.
     */
    #define PSA_AEAD_FINISH_OUTPUT_SIZE_1_ARG(alg) \
        MBEDTLS_DEPRECATED_CONSTANT(size_t, PSA_ALG_IS_AEAD_ON_BLOCK_CIPHER(alg) ? PSA_BLOCK_CIPHER_BLOCK_MAX_SIZE : 0)

    /** A sufficient plaintext buffer size for psa_aead_verify().
     *
     * If the size of the plaintext buffer is at least this large, it is
     * guaranteed that psa_aead_verify() will not fail due to an
     * insufficient plaintext buffer size. The actual size of the output may
     * be smaller in any given call.
     *
     * \param alg                 An AEAD algorithm
     *                            (\c PSA_ALG_XXX value such that
     *                            #PSA_ALG_IS_AEAD(\p alg) is true).
     *
     * \return                    A sufficient plaintext buffer size for the
     *                            specified algorithm.
     *                            If the AEAD algorithm is not recognized, return 0.
     */
    #define PSA_AEAD_VERIFY_OUTPUT_SIZE_1_ARG(alg) \
        MBEDTLS_DEPRECATED_CONSTANT(size_t, PSA_ALG_IS_AEAD_ON_BLOCK_CIPHER(alg) ? PSA_BLOCK_CIPHER_BLOCK_MAX_SIZE : 0)

#endif /* MBEDTLS_DEPRECATED_REMOVED */

/* Old values used by mbedCrypto for key types / curves.
    Requried for translation when calling CryptoKeyKeyStore_PSA functions */
#if defined(USE_MBEDCRYPTO_VALUES)
    /* The encoding of curve identifiers is currently aligned with the
     * TLS Supported Groups Registry (formerly known as the
     * TLS EC Named Curve Registry)
     * https://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-8
     * The values are defined by RFC 8422 and RFC 7027. */
    #define OLD_PSA_ECC_CURVE_SECT163K1        ((psa_ecc_curve_t)0x0001)
    #define OLD_PSA_ECC_CURVE_SECT163R1        ((psa_ecc_curve_t)0x0002)
    #define OLD_PSA_ECC_CURVE_SECT163R2        ((psa_ecc_curve_t)0x0003)
    #define OLD_PSA_ECC_CURVE_SECT193R1        ((psa_ecc_curve_t)0x0004)
    #define OLD_PSA_ECC_CURVE_SECT193R2        ((psa_ecc_curve_t)0x0005)
    #define OLD_PSA_ECC_CURVE_SECT233K1        ((psa_ecc_curve_t)0x0006)
    #define OLD_PSA_ECC_CURVE_SECT233R1        ((psa_ecc_curve_t)0x0007)
    #define OLD_PSA_ECC_CURVE_SECT239K1        ((psa_ecc_curve_t)0x0008)
    #define OLD_PSA_ECC_CURVE_SECT283K1        ((psa_ecc_curve_t)0x0009)
    #define OLD_PSA_ECC_CURVE_SECT283R1        ((psa_ecc_curve_t)0x000a)
    #define OLD_PSA_ECC_CURVE_SECT409K1        ((psa_ecc_curve_t)0x000b)
    #define OLD_PSA_ECC_CURVE_SECT409R1        ((psa_ecc_curve_t)0x000c)
    #define OLD_PSA_ECC_CURVE_SECT571K1        ((psa_ecc_curve_t)0x000d)
    #define OLD_PSA_ECC_CURVE_SECT571R1        ((psa_ecc_curve_t)0x000e)
    #define OLD_PSA_ECC_CURVE_SECP160K1        ((psa_ecc_curve_t)0x000f)
    #define OLD_PSA_ECC_CURVE_SECP160R1        ((psa_ecc_curve_t)0x0010)
    #define OLD_PSA_ECC_CURVE_SECP160R2        ((psa_ecc_curve_t)0x0011)
    #define OLD_PSA_ECC_CURVE_SECP192K1        ((psa_ecc_curve_t)0x0012)
    #define OLD_PSA_ECC_CURVE_SECP192R1        ((psa_ecc_curve_t)0x0013)
    #define OLD_PSA_ECC_CURVE_SECP224K1        ((psa_ecc_curve_t)0x0014)
    #define OLD_PSA_ECC_CURVE_SECP224R1        ((psa_ecc_curve_t)0x0015)
    #define OLD_PSA_ECC_CURVE_SECP256K1        ((psa_ecc_curve_t)0x0016)
    #define OLD_PSA_ECC_CURVE_SECP256R1        ((psa_ecc_curve_t)0x0017)
    #define OLD_PSA_ECC_CURVE_SECP384R1        ((psa_ecc_curve_t)0x0018)
    #define OLD_PSA_ECC_CURVE_SECP521R1        ((psa_ecc_curve_t)0x0019)
    #define OLD_PSA_ECC_CURVE_BRAINPOOL_P256R1 ((psa_ecc_curve_t)0x001a)
    #define OLD_PSA_ECC_CURVE_BRAINPOOL_P384R1 ((psa_ecc_curve_t)0x001b)
    #define OLD_PSA_ECC_CURVE_BRAINPOOL_P512R1 ((psa_ecc_curve_t)0x001c)
    /** Curve25519.
     *
     * This is the curve defined in Bernstein et al.,
     * _Curve25519: new Diffie-Hellman speed records_, LNCS 3958, 2006.
     * The algorithm #PSA_ALG_ECDH performs X25519 when used with this curve.
     */
    #define OLD_PSA_ECC_CURVE_CURVE25519       ((psa_ecc_curve_t)0x001d)
    /** Curve448
     *
     * This is the curve defined in Hamburg,
     * _Ed448-Goldilocks, a new elliptic curve_, NIST ECC Workshop, 2015.
     * The algorithm #PSA_ALG_ECDH performs X448 when used with this curve.
     */
    #define OLD_PSA_ECC_CURVE_CURVE448         ((psa_ecc_curve_t)0x001e)
    /** Raw data.
     *
     * A "key" of this type cannot be used for any cryptographic operation.
     * Applications may use this type to store arbitrary data in the keystore. */
    #define OLD_PSA_KEY_TYPE_RAW_DATA          ((psa_key_type_t)0x50000001)

    /** HMAC key.
     *
     * The key policy determines which underlying hash algorithm the key can be
     * used for.
     *
     * HMAC keys should generally have the same size as the underlying hash.
     * This size can be calculated with #PSA_HASH_SIZE(\c alg) where
     * \c alg is the HMAC algorithm or the underlying hash algorithm. */
    #define OLD_PSA_KEY_TYPE_HMAC ((psa_key_type_t)0x51000000)

    /** A secret for key derivation.
     *
     * The key policy determines which key derivation algorithm the key
     * can be used for.
     */
    #define OLD_PSA_KEY_TYPE_DERIVE ((psa_key_type_t)0x52000000)

    /** Key for a cipher, AEAD or MAC algorithm based on the AES block cipher.
     *
     * The size of the key can be 16 bytes (AES-128), 24 bytes (AES-192) or
     * 32 bytes (AES-256).
     */
    #define OLD_PSA_KEY_TYPE_AES ((psa_key_type_t)0x40000001)

    /** Key for a cipher or MAC algorithm based on DES or 3DES (Triple-DES).
     *
     * The size of the key can be 8 bytes (single DES), 16 bytes (2-key 3DES) or
     * 24 bytes (3-key 3DES).
     *
     * Note that single DES and 2-key 3DES are weak and strongly
     * deprecated and should only be used to decrypt legacy data. 3-key 3DES
     * is weak and deprecated and should only be used in legacy protocols.
     */
    #define OLD_PSA_KEY_TYPE_DES ((psa_key_type_t)0x40000002)

    /** Key for a cipher, AEAD or MAC algorithm based on the
     * Camellia block cipher. */
    #define OLD_PSA_KEY_TYPE_CAMELLIA ((psa_key_type_t)0x40000003)

    /** Key for the RC4 stream cipher.
     *
     * Note that RC4 is weak and deprecated and should only be used in
     * legacy protocols. */
    #define OLD_PSA_KEY_TYPE_ARC4 ((psa_key_type_t)0x40000004)

    /** Key for the ChaCha20 stream cipher or the Chacha20-Poly1305 AEAD algorithm.
     *
     * ChaCha20 and the ChaCha20_Poly1305 construction are defined in RFC 7539.
     *
     * Implementations must support 12-byte nonces, may support 8-byte nonces,
     * and should reject other sizes.
     */
    #define OLD_PSA_KEY_TYPE_CHACHA20 ((psa_key_type_t)0x40000005)

    /** RSA public key. */
    #define OLD_PSA_KEY_TYPE_RSA_PUBLIC_KEY ((psa_key_type_t)0x60010000)
    /** RSA key pair (private and public key). */
    #define OLD_PSA_KEY_TYPE_RSA_KEY_PAIR   ((psa_key_type_t)0x70010000)

    #define OLD_PSA_KEY_TYPE_ECC_PUBLIC_KEY_BASE ((psa_key_type_t)0x60030000)
    #define OLD_PSA_KEY_TYPE_ECC_KEY_PAIR_BASE   ((psa_key_type_t)0x70030000)

    #define OLD_PSA_KEY_TYPE_CATEGORY_FLAG_PAIR ((psa_key_type_t)0x10000000)

    #define OLD_PSA_KEY_TYPE_PUBLIC_KEY_OF_KEY_PAIR(type) ((type) & ~OLD_PSA_KEY_TYPE_CATEGORY_FLAG_PAIR)

    #define OLD_PSA_KEY_TYPE_ECC_CURVE_MASK ((psa_key_type_t)0x0000ffff)

    /** Whether a key type is an elliptic curve key (pair or public-only). */
    #define OLD_PSA_KEY_TYPE_IS_ECC(type)                                                      \
        ((OLD_PSA_KEY_TYPE_PUBLIC_KEY_OF_KEY_PAIR(type) & ~OLD_PSA_KEY_TYPE_ECC_CURVE_MASK) == \
         OLD_PSA_KEY_TYPE_ECC_PUBLIC_KEY_BASE)
    /** Whether a key type is an elliptic curve key pair. */
    #define OLD_PSA_KEY_TYPE_IS_ECC_KEY_PAIR(type) \
        (((type) & ~OLD_PSA_KEY_TYPE_ECC_CURVE_MASK) == OLD_PSA_KEY_TYPE_ECC_KEY_PAIR_BASE)

    #define OLD_PSA_KEY_TYPE_GET_CURVE(type) \
        ((psa_ecc_curve_t)(OLD_PSA_KEY_TYPE_IS_ECC(type) ? ((type)&OLD_PSA_KEY_TYPE_ECC_CURVE_MASK) : 0))

    #define OLD_PSA_ALG_ECDH ((psa_algorithm_t)0x30200000)

#endif

#if defined(USE_MBEDCRYPTO_VALUES)

    #define OLD_PSA_ALG_ECB_NO_PADDING ((psa_algorithm_t)0x06401001)
    #define OLD_PSA_ALG_CCM            ((psa_algorithm_t)0x06401001)
    #define OLD_PSA_ALG_CBC_NO_PADDING ((psa_algorithm_t)0x04600100)
    #define OLD_PSA_ALG_CTR            ((psa_algorithm_t)0x04c00001)

    #define OLD_PSA_ALG_CCM ((psa_algorithm_t)0x06401001)
    #define OLD_PSA_ALG_GCM ((psa_algorithm_t)0x06401002)

    #define OLD_PSA_ALG_AEAD_TAG_LENGTH_MASK ((psa_algorithm_t)0x00003f00)
    #define OLD_PSA_AEAD_TAG_LENGTH_OFFSET   8

    #define OLD_PSA_ALG_AEAD_WITH_TAG_LENGTH(aead_alg, tag_length) \
        (((aead_alg) & ~OLD_PSA_ALG_AEAD_TAG_LENGTH_MASK) |        \
         ((tag_length) << OLD_PSA_AEAD_TAG_LENGTH_OFFSET & OLD_PSA_ALG_AEAD_TAG_LENGTH_MASK))

    /** SHA2-224 */
    #define OLD_PSA_ALG_SHA_224 ((psa_algorithm_t)0x01000008)
    /** SHA2-256 */
    #define OLD_PSA_ALG_SHA_256 ((psa_algorithm_t)0x01000009)
    /** SHA2-384 */
    #define OLD_PSA_ALG_SHA_384 ((psa_algorithm_t)0x0100000a)
    /** SHA2-512 */
    #define OLD_PSA_ALG_SHA_512 ((psa_algorithm_t)0x0100000b)

    /** The CBC-MAC construction over a block cipher
     *
     * \warning CBC-MAC is insecure in many cases.
     * A more secure mode, such as #PSA_ALG_CMAC, is recommended.
     */
    #define OLD_PSA_ALG_CBC_MAC ((psa_algorithm_t)0x02c00001)
    /** The CMAC construction over a block cipher */
    #define OLD_PSA_ALG_CMAC    ((psa_algorithm_t)0x02c00002)

#endif

/** Open a handle to an existing persistent key.
 *
 * Open a handle to a persistent key. A key is persistent if it was created
 * with a lifetime other than #PSA_KEY_LIFETIME_VOLATILE. A persistent key
 * always has a nonzero key identifier, set with psa_set_key_id() when
 * creating the key. Implementations may provide additional pre-provisioned
 * keys that can be opened with psa_open_key(). Such keys have an application
 * key identifier in the vendor range, as documented in the description of
 * #psa_key_id_t.
 *
 * The application must eventually close the handle with psa_close_key() or
 * psa_destroy_key() to release associated resources. If the application dies
 * without calling one of these functions, the implementation should perform
 * the equivalent of a call to psa_close_key().
 *
 * Some implementations permit an application to open the same key multiple
 * times. If this is successful, each call to psa_open_key() will return a
 * different key handle.
 *
 * \note This API is not part of the PSA Cryptography API Release 1.0.0
 * specification. It was defined in the 1.0 Beta 3 version of the
 * specification but was removed in the 1.0.0 released version. This API is
 * kept for the time being to not break applications relying on it. It is not
 * deprecated yet but will be in the near future.
 *
 * \note Applications that rely on opening a key multiple times will not be
 * portable to implementations that only permit a single key handle to be
 * opened. See also :ref:\`key-handles\`.
 *
 *
 * \param key           The persistent identifier of the key.
 * \param[out] handle   On success, a handle to the key.
 *
 * \retval #PSA_SUCCESS
 *         Success. The application can now use the value of `*handle`
 *         to access the key.
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY
 *         The implementation does not have sufficient resources to open the
 *         key. This can be due to reaching an implementation limit on the
 *         number of open keys, the number of open key handles, or available
 *         memory.
 * \retval #PSA_ERROR_DOES_NOT_EXIST
 *         There is no persistent key with key identifier \p key.
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         \p key is not a valid persistent key identifier.
 * \retval #PSA_ERROR_NOT_PERMITTED
 *         The specified key exists, but the application does not have the
 *         permission to access it. Note that this specification does not
 *         define any way to create such a key, but it may be possible
 *         through implementation-specific means.
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #PSA_ERROR_CORRUPTION_DETECTED
 * \retval #PSA_ERROR_STORAGE_FAILURE
 * \retval #PSA_ERROR_DATA_INVALID
 * \retval #PSA_ERROR_DATA_CORRUPT
 * \retval #PSA_ERROR_BAD_STATE
 *         The library has not been previously initialized by psa_crypto_init().
 *         It is implementation-dependent whether a failure to initialize
 *         results in this error code.
 */
psa_status_t psa_open_key(mbedtls_svc_key_id_t key, psa_key_handle_t *handle);

/** Close a key handle.
 *
 * If the handle designates a volatile key, this will destroy the key material
 * and free all associated resources, just like psa_destroy_key().
 *
 * If this is the last open handle to a persistent key, then closing the handle
 * will free all resources associated with the key in volatile memory. The key
 * data in persistent storage is not affected and can be opened again later
 * with a call to psa_open_key().
 *
 * Closing the key handle makes the handle invalid, and the key handle
 * must not be used again by the application.
 *
 * \note This API is not part of the PSA Cryptography API Release 1.0.0
 * specification. It was defined in the 1.0 Beta 3 version of the
 * specification but was removed in the 1.0.0 released version. This API is
 * kept for the time being to not break applications relying on it. It is not
 * deprecated yet but will be in the near future.
 *
 * \note If the key handle was used to set up an active
 * :ref:\`multipart operation <multipart-operations>\`, then closing the
 * key handle can cause the multipart operation to fail. Applications should
 * maintain the key handle until after the multipart operation has finished.
 *
 * \param handle        The key handle to close.
 *                      If this is \c 0, do nothing and return \c PSA_SUCCESS.
 *
 * \retval #PSA_SUCCESS
 *         \p handle was a valid handle or \c 0. It is now closed.
 * \retval #PSA_ERROR_INVALID_HANDLE
 *         \p handle is not a valid handle nor \c 0.
 * \retval #PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #PSA_ERROR_CORRUPTION_DETECTED
 * \retval #PSA_ERROR_BAD_STATE
 *         The library has not been previously initialized by psa_crypto_init().
 *         It is implementation-dependent whether a failure to initialize
 *         results in this error code.
 */
psa_status_t psa_close_key(psa_key_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* PSA_CRYPTO_COMPAT_H */
