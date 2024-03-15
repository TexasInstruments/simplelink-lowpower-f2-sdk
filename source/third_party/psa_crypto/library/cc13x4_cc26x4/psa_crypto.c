/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <psa/crypto.h>
#include <psa/crypto_compat.h>
#include <cc13x4_cc26x4/crypto_helper.h>

#include <stdlib.h>
#include <string.h>

#include <ti/drivers/SHA2.h>
#include <ti/drivers/AESECB.h>
#include <ti/drivers/AESCCM.h>
#include <ti/drivers/AESGCM.h>
#include <ti/drivers/AESCTR.h>
#include <ti/drivers/AESCBC.h>
#include <ti/drivers/AESCMAC.h>
#include <ti/drivers/ECDH.h>
#include <ti/drivers/cryptoutils/ecc/ECCParams.h>
#include <ti/drivers/TRNG.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_init.h>

#if TFM_ENABLED == 0
    /* For KeyStore_PSA_init() in secure only build */
    #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_helpers.h>
    /* For psa_hash_suspend and psa_hash_resume */
    #include <ti/drivers/sha2/SHA2CC26X2.h>
#endif

/* no s/ns support for ECDSA and EDDSA */
#if TFM_ENABLED == 0
    #include "ti/drivers/ECDSA.h"
    #include <ti/drivers/EDDSA.h>
#endif

#if TFM_ENABLED == 1
    /*
     * Array for storing the mbedTLS algorithm value of a key
     * before translating to the mbedCrypto value in psa_import_key.
     * Used to recover the original algorithm value in psa_get_key_attributes.
     * For now, the application can create only up to ten keys at once on
     * tz_enabled configurations.
     */
    #define TZ_ENABLED_KEY_STORAGE_LIMIT 10

typedef struct
{
    psa_key_id_t keyID;
    psa_algorithm_t alg;
} psa_key_data_to_reverse_mapping;

static psa_key_data_to_reverse_mapping keyAlgorithms[TZ_ENABLED_KEY_STORAGE_LIMIT];
static uint8_t keyAlgorithmsLength = 0;
#endif

#define PSA_CRYPTO_TI_DRIVER 1

AESECB_Handle AESECB_Hand      = NULL;
AESCCM_Handle AESCCM_Hand      = NULL;
ECDH_Handle ECDH_Hand          = NULL;
AESGCM_Handle AESGCM_Hand      = NULL;
AESCTR_Handle AESCTR_Hand      = NULL;
AESCMAC_Handle AESCMAC_Hand    = NULL;
AESCBC_Handle AESCBC_Hand      = NULL;
AESCMAC_Handle AESCBC_MAC_Hand = NULL;
SHA2_Handle SHA2_Hand          = NULL;
TRNG_Handle TRNG_Hand          = NULL;

/* No S/NS support for ECDSA and EDDSA */
#if TFM_ENABLED == 0
ECDSA_Handle ECDSA_Hand = NULL;
EDDSA_Handle EDDSA_Hand = NULL;
#endif

typedef enum
{
    AES_ENC = 0,
    AES_DEC
} aes_function_t;

/****************************************************************/
/* Global data, support functions and library management */
/****************************************************************/

#if defined(PSA_DEBUG)
static uint8_t *iv;
static uint8_t iv_set = 0;
psa_status_t psa_debug_get_iv(uint8_t **new_iv)
{
    if (!iv_set)
    {
        return PSA_ERROR_BAD_STATE;
    }

    iv_set  = 0;
    *new_iv = iv;
    return PSA_SUCCESS;
}

void psa_debug_set_iv(uint8_t *new_iv)
{
    iv_set = 1;
    iv     = new_iv;
}
#endif

/****************************************************************/
/* Key management */
/****************************************************************/

/** Check whether a given key type is valid for use with a given MAC algorithm
 *
 * Upon successful return of this function, the behavior of #PSA_MAC_LENGTH
 * when called with the validated \p algorithm and \p key_type is well-defined.
 *
 * \param[in] algorithm     The specific MAC algorithm (can be wildcard).
 * \param[in] key_type      The key type of the key to be used with the
 *                          \p algorithm.
 *
 * \retval #PSA_SUCCESS
 *         The \p key_type is valid for use with the \p algorithm
 * \retval #PSA_ERROR_INVALID_ARGUMENT
 *         The \p key_type is not valid for use with the \p algorithm
 */
static psa_status_t psa_mac_key_can_do(psa_algorithm_t algorithm, psa_key_type_t key_type)
{

    psa_status_t status = PSA_ERROR_NOT_SUPPORTED;

    /*if the key is not compatible with alg, then return PSA_ERROR_INVALID_ARGUMENT*/
    if (PSA_ALG_IS_HMAC(algorithm))
    {
        /* for now, importing a key of type HMAC is not supported,
         * so users will need to use raw data key type for HMAC */
        if (key_type == PSA_KEY_TYPE_HMAC || key_type == PSA_KEY_TYPE_RAW_DATA)
        {
            status = PSA_SUCCESS;
        }
        else
        {
            status = PSA_ERROR_INVALID_ARGUMENT;
        }
    }
    else
        /* our device only supports AES-MAC */
        if (PSA_ALG_IS_MAC(algorithm))
        {
            if (key_type == PSA_KEY_TYPE_AES)
            {
                status = PSA_SUCCESS;
            }
            else
            {
                status = PSA_ERROR_INVALID_ARGUMENT;
            }
        }

    return status;
}

static int psa_key_algorithm_permits(psa_key_type_t key_type, psa_algorithm_t policy_alg, psa_algorithm_t requested_alg)
{
    /* If it isn't explicitly permitted, it's forbidden. */
    int permits = 0;

    /* Common case: the policy only allows requested_alg. */
    if (requested_alg == policy_alg)
    {
        permits = 1;
    }
    /* If policy_alg is a hash-and-sign with a wildcard for the hash,
     * and requested_alg is the same hash-and-sign family with any hash,
     * then requested_alg is compliant with policy_alg. */
    else if (PSA_ALG_IS_HASH_AND_SIGN(requested_alg) && PSA_ALG_SIGN_GET_HASH(policy_alg) == PSA_ALG_ANY_HASH)
    {
        permits = ((policy_alg & ~PSA_ALG_HASH_MASK) == (requested_alg & ~PSA_ALG_HASH_MASK));
    }
    /* If policy_alg is a wildcard AEAD algorithm of the same base as
     * the requested algorithm, check the requested tag length to be
     * equal-length or longer than the wildcard-specified length. */
    else if (PSA_ALG_IS_AEAD(policy_alg) && PSA_ALG_IS_AEAD(requested_alg) &&
             (PSA_ALG_AEAD_WITH_SHORTENED_TAG(policy_alg, 0) == PSA_ALG_AEAD_WITH_SHORTENED_TAG(requested_alg, 0)) &&
             ((policy_alg & PSA_ALG_AEAD_AT_LEAST_THIS_LENGTH_FLAG) != 0))
    {
        permits = (PSA_ALG_AEAD_GET_TAG_LENGTH(policy_alg) <= PSA_ALG_AEAD_GET_TAG_LENGTH(requested_alg));
    }
    /* If policy_alg is a MAC algorithm of the same base as the requested
     * algorithm, check whether their MAC lengths are compatible. */
    else if (PSA_ALG_IS_MAC(policy_alg) && PSA_ALG_IS_MAC(requested_alg) &&
             (PSA_ALG_FULL_LENGTH_MAC(policy_alg) == PSA_ALG_FULL_LENGTH_MAC(requested_alg)))
    {
        /* Validate the combination of key type and algorithm. Since the policy
         * and requested algorithms are the same, we only need this once. */
        if (PSA_SUCCESS != psa_mac_key_can_do(policy_alg, key_type))
        {
            permits = 1;
        }
        else
        {
            /* Get both the requested output length for the algorithm which is to be
             * verified, and the default output length for the base algorithm.
             * Note that none of the currently supported algorithms have an output
             * length dependent on actual key size, so setting it to a bogus value
             * of 0 is currently OK. */
            size_t requested_output_length = PSA_MAC_LENGTH(key_type, 0, requested_alg);
            size_t default_output_length   = PSA_MAC_LENGTH(key_type, 0, PSA_ALG_FULL_LENGTH_MAC(requested_alg));

            /* If the policy is default-length, only allow an algorithm with
             * a declared exact-length matching the default. */
            if (PSA_MAC_TRUNCATED_LENGTH(policy_alg) == 0)
            {
                permits = (requested_output_length == default_output_length);
            }
            /* If the requested algorithm is default-length, allow it if the policy
             * length exactly matches the default length. */
            else if (PSA_MAC_TRUNCATED_LENGTH(requested_alg) == 0 &&
                     PSA_MAC_TRUNCATED_LENGTH(policy_alg) == default_output_length)
            {
                permits = (1);
            }
            /* If policy_alg is an at-least-this-length wildcard MAC algorithm,
             * check for the requested MAC length to be equal to or longer than the
             * minimum allowed length. */
            else if ((policy_alg & PSA_ALG_MAC_AT_LEAST_THIS_LENGTH_FLAG) != 0)
            {
                permits = (PSA_MAC_TRUNCATED_LENGTH(policy_alg) <= requested_output_length);
            }
        }
    }
    /* If policy_alg is a generic key agreement operation, then using it for
     * a key derivation with that key agreement should also be allowed. This
     * behaviour is expected to be defined in a future specification version. */
    else if (PSA_ALG_IS_RAW_KEY_AGREEMENT(policy_alg) && PSA_ALG_IS_KEY_AGREEMENT(requested_alg))
    {
        permits = (PSA_ALG_KEY_AGREEMENT_GET_BASE(requested_alg) == policy_alg);
    }

    return permits;
}

/** Test whether a policy permits an algorithm.
 *
 * The caller must test usage flags separately.
 *
 * \note This function requires providing the key type for which the policy is
 *       being validated, since some algorithm policy definitions (e.g. MAC)
 *       have different properties depending on what kind of cipher it is
 *       combined with.
 *
 * \retval PSA_SUCCESS                  When \p alg is a specific algorithm
 *                                      allowed by the \p policy.
 * \retval PSA_ERROR_INVALID_ARGUMENT   When \p alg is not a specific algorithm
 * \retval PSA_ERROR_NOT_PERMITTED      When \p alg is a specific algorithm, but
 *                                      the \p policy does not allow it.
 */
static psa_status_t psa_key_policy_permits(const psa_key_policy_t *policy, psa_key_type_t key_type, psa_algorithm_t alg)
{

    psa_status_t status;

    /* '0' is not a valid algorithm */
    if (alg == 0)
    {
        status = (PSA_ERROR_INVALID_ARGUMENT);
    }
    /* A requested algorithm cannot be a wildcard. */
    else if (PSA_ALG_IS_WILDCARD(alg))
    {
        status = (PSA_ERROR_INVALID_ARGUMENT);
    }
    else if (psa_key_algorithm_permits(key_type, policy->alg, alg) ||
             psa_key_algorithm_permits(key_type, policy->alg2, alg))
    {
        status = (PSA_SUCCESS);
    }
    else
    {
        status = (PSA_ERROR_NOT_PERMITTED);
    }

    return status;
}

/** Check whether a set of key attributes allows the given usage policy and algorithm
 *
 * The key must have allow all the usage flags set in \p usage. If \p alg is
 * nonzero, the key must allow operations with this algorithm. If \p alg is
 * zero, the algorithm is not checked.
 *
 *
 */
static psa_status_t psa_key_attributes_usage_check(psa_key_attributes_t *attributes,
                                                   psa_key_usage_t usage,
                                                   psa_algorithm_t alg)
{
    psa_status_t status       = PSA_SUCCESS;
    psa_key_type_t key_type   = psa_get_key_type(attributes);
    psa_key_usage_t key_usage = psa_get_key_usage_flags(attributes);

#if TFM_ENABLED == 0
    psa_key_policy_t key_policy = attributes->core.policy;
#else
    psa_key_policy_t key_policy = {.usage = key_usage, .alg = attributes->alg};
#endif

    /* Enforce that usage policy for the key contains all the flags
     * required by the usage parameter. There is one exception: public
     * keys can always be exported, so we treat public key objects as
     * if they had the export flag. */
    if (PSA_KEY_TYPE_IS_PUBLIC_KEY(key_type))
    {
        usage &= ~PSA_KEY_USAGE_EXPORT;
    }

    if ((key_usage & usage) != usage)
    {
        status = PSA_ERROR_NOT_PERMITTED;
    }
    /* Enforce that the usage policy permits the requested algorthihm. */
    else if (alg != 0)
    {
        status = psa_key_policy_permits(&key_policy, key_type, alg);
    }

    return status;
}

inline psa_key_attributes_t psa_key_attributes_init(void)
{
    const psa_key_attributes_t v = PSA_KEY_ATTRIBUTES_INIT;
    return (v);
}

inline void psa_set_key_id(psa_key_attributes_t *attributes, psa_key_id_t key)
{
    KeyStore_PSA_setKeyId(attributes, toKeyStoreKeyID(key));
}

inline psa_key_id_t psa_get_key_id(const psa_key_attributes_t *attributes)
{
    return toKeyID(KeyStore_PSA_getKeyId((psa_key_attributes_t *)attributes));
}

inline void psa_set_key_lifetime(psa_key_attributes_t *attributes, psa_key_lifetime_t lifetime)
{
    KeyStore_PSA_setKeyLifetime(attributes, lifetime);
}

inline psa_key_lifetime_t psa_get_key_lifetime(const psa_key_attributes_t *attributes)
{
    return KeyStore_PSA_getKeyLifetime((psa_key_attributes_t *)attributes);
}

inline void psa_set_key_usage_flags(psa_key_attributes_t *attributes, psa_key_usage_t usage_flags)
{
    KeyStore_PSA_setKeyUsageFlags(attributes, usage_flags);
}

psa_key_usage_t psa_get_key_usage_flags(const psa_key_attributes_t *attributes)
{
    return KeyStore_PSA_getKeyUsageFlags((psa_key_attributes_t *)attributes);
}

inline void psa_set_key_algorithm(psa_key_attributes_t *attributes, psa_algorithm_t alg)
{
    KeyStore_PSA_setKeyAlgorithm(attributes, alg);
}

inline psa_algorithm_t psa_get_key_algorithm(const psa_key_attributes_t *attributes)
{
    return KeyStore_PSA_getKeyAlgorithm((psa_key_attributes_t *)attributes);
}

inline void psa_set_key_type(psa_key_attributes_t *attributes, psa_key_type_t type)
{
    KeyStore_PSA_setKeyType(attributes, type);
}

inline psa_key_type_t psa_get_key_type(const psa_key_attributes_t *attributes)
{
    return KeyStore_PSA_getKeyType((psa_key_attributes_t *)attributes);
}

inline void psa_set_key_bits(psa_key_attributes_t *attributes, size_t bits)
{
    KeyStore_PSA_setKeyBits(attributes, bits);
}

inline size_t psa_get_key_bits(const psa_key_attributes_t *attributes)
{
    return KeyStore_PSA_getKeyBits((psa_key_attributes_t *)attributes);
}

psa_status_t psa_destroy_key(psa_key_id_t key)
{

#if TFM_ENABLED == 1
    /* get index of element to remove */
    int8_t toRemove = -1;
    for (uint8_t i = 0; i < keyAlgorithmsLength; i++)
    {
        if (keyAlgorithms[i].keyID == key)
        {
            toRemove = i;
            break;
        }
    }

    if (toRemove >= 0)
    {
        /* remove element by shifting everything to the left */
        for (uint8_t i = toRemove; i < keyAlgorithmsLength - 1; i++)
        {
            keyAlgorithms[i] = keyAlgorithms[i + 1];
        }

        keyAlgorithmsLength--;
    }
#endif

    return KeyStore_PSA_destroyKey(toKeyStoreKeyID(key));
}

psa_status_t psa_purge_key(psa_key_id_t key)
{

    psa_status_t status             = PSA_SUCCESS;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_get_key_attributes(key, &attributes);
    psa_key_lifetime_t lifetime = psa_get_key_lifetime(&attributes);

    /* Nothing to be done for peristent keys */
    if (lifetime != PSA_KEY_PERSISTENCE_VOLATILE)
    {
        status = KeyStore_PSA_purgeKey(toKeyStoreKeyID(key));
    }

    psa_reset_key_attributes(&attributes);

    return status;
}

/*
 * Retrieve all the publicly-accessible attributes of a key.
 */
psa_status_t psa_get_key_attributes(psa_key_id_t key, psa_key_attributes_t *attributes)
{
    *attributes         = PSA_KEY_ATTRIBUTES_INIT;
    psa_status_t status = KeyStore_PSA_getKeyAttributes(toKeyStoreKeyID(key), attributes);
    if (status != PSA_SUCCESS)
    {
        return status;
    }

    /*
     * retreve the algorithm set by the application,
     * before the algorithm was mapped to the value needed by the drivers
     */
    psa_algorithm_t originalAlgorithm = 0;
#if TFM_ENABLED == 0
    originalAlgorithm = attributes->core.policy.alg2;
#else
    for (uint8_t i = 0; i < keyAlgorithmsLength; i++)
    {
        if (keyAlgorithms[i].keyID == key)
        {
            originalAlgorithm = keyAlgorithms[i].alg;
            break;
        }
    }

    if (originalAlgorithm == 0)
    {
        return PSA_ERROR_INVALID_HANDLE;
    }
#endif

    psa_set_key_algorithm(attributes, originalAlgorithm);
    return PSA_SUCCESS;
}

void psa_reset_key_attributes(psa_key_attributes_t *attributes)
{
#if TFM_ENABLED == 1
    memset(attributes, 0, sizeof(*attributes));
#else
    KeyStore_PSA_resetKeyAttributes(attributes);
#endif
}

psa_status_t psa_export_key(psa_key_id_t key, uint8_t *data, size_t data_size, size_t *data_length)
{
    return KeyStore_PSA_exportKey(toKeyStoreKeyID(key), data, data_size, data_length);
}

psa_status_t psa_export_public_key(psa_key_id_t key, uint8_t *data, size_t data_size, size_t *data_length)
{

    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_status_t status             = psa_get_key_attributes(key, &attributes);
    if (status != PSA_SUCCESS)
    {
        return status;
    }

    psa_key_type_t keyType = psa_get_key_type(&attributes);

    /*
     * KeyStore_PSA does not support exporting the public part
     * of a key pair, so in that case generate the public part
     * using ECDH_generatePublicKey
     */
    if (PSA_KEY_TYPE_IS_ECC_KEY_PAIR(keyType))
    {
        size_t keyBits  = psa_get_key_bits(&attributes);
        size_t keyBytes = PSA_BITS_TO_BYTES(keyBits);
        size_t publicKeyBytes;
        ECDH_KeyMaterialEndianness endianess;

        /* for most families key size is twice the private key size + 1 */
        psa_ecc_family_t keyFamily = PSA_KEY_TYPE_ECC_GET_FAMILY(keyType);
        if (keyFamily == PSA_ECC_FAMILY_MONTGOMERY)
        {
            publicKeyBytes = keyBytes;
            endianess      = ECDH_LITTLE_ENDIAN_KEY;
        }
        else
        {
            publicKeyBytes = keyBytes * 2 + 1;
            endianess      = ECDH_BIG_ENDIAN_KEY;
        }

        if (publicKeyBytes > data_size)
        {
            status = PSA_ERROR_BUFFER_TOO_SMALL;
        }
        else
        {
            /* init cryptoKeys for use in the GeneratePublicKey operation */
            CryptoKey myPublicKey;
            CryptoKey myPrivateKey;
            CryptoKeyPlaintext_initBlankKey(&myPublicKey, data, publicKeyBytes);
            KeyStore_PSA_initKey(&myPrivateKey, toKeyStoreKeyID(key), keyBytes, NULL);

            /* init the operation */
            ECDH_OperationGeneratePublicKey opererationGeneratePublicKey;
            ECDH_OperationGeneratePublicKey_init(&opererationGeneratePublicKey);
            opererationGeneratePublicKey.myPrivateKey          = &myPrivateKey;
            opererationGeneratePublicKey.myPublicKey           = &myPublicKey;
            /* get the correct parameters, some curves are not supported */
            opererationGeneratePublicKey.curve                 = map_keyTypeToECCParams(keyType, keyBits);
            opererationGeneratePublicKey.keyMaterialEndianness = endianess;
            if (opererationGeneratePublicKey.curve == NULL)
            {
                status = PSA_ERROR_NOT_SUPPORTED;
            }
            else
            {
                psa_status_t status = ECDH_generatePublicKey(ECDH_Hand, &opererationGeneratePublicKey);
                if (status != PSA_SUCCESS)
                {
                    *data_length = 0;
                    status       = map_ECDHError_to_PSA_error(status);
                }
                else
                {
                    *data_length = publicKeyBytes;
                    status       = PSA_SUCCESS;
                }
            }
        }
    }
    else
    {
        status = KeyStore_PSA_exportPublicKey(toKeyStoreKeyID(key), data, data_size, data_length);
    }

    psa_reset_key_attributes(&attributes);
    return status;
}

psa_status_t psa_import_key(const psa_key_attributes_t *attributes,
                            const uint8_t *data,
                            size_t data_length,
                            psa_key_id_t *key)
{
    psa_status_t status;
    psa_key_attributes_t attributesCopy = *attributes;
    psa_key_type_t psaKeyType           = psa_get_key_type(attributes);
    psa_algorithm_t originalAlgorithm   = psa_get_key_algorithm(attributes);
    psa_key_file_id_t keyStoreKeyID;

    /* SL KeyStore driver does not support distinct algorithms for AEAD with different tag sizes
     * The SL Crypto driver for CCM and GCM can only accept the standard PSA_ALG_CCM or 
     * PSA_ALG_GCM algorithm */
    if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(originalAlgorithm) == PSA_ALG_CCM)
    {
        psa_set_key_algorithm(&attributesCopy, PSA_ALG_CCM);
    }
    if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(originalAlgorithm) == PSA_ALG_GCM)
    {
        psa_set_key_algorithm(&attributesCopy, PSA_ALG_GCM);
    }
    psa_set_key_type(&attributesCopy, psaKeyType);

    /*
     * Since ECDSA driver does not support SKS,
     * all ECDSA keys are given the export flag so that the drivers
     * can access the key material.
     */
    if (PSA_ALG_IS_ECDSA(originalAlgorithm) || originalAlgorithm == PSA_ALG_ED25519PH)
    {
        psa_key_usage_t usage = psa_get_key_usage_flags(attributes);
        if ((usage & PSA_KEY_USAGE_EXPORT) != PSA_KEY_USAGE_EXPORT)
        {
            usage |= PSA_KEY_USAGE_EXPORT;
            psa_set_key_usage_flags(&attributesCopy, usage);
        }
    }

    keyStoreKeyID = toKeyStoreKeyID(*key);

/*
 * store the original algorithm value so that it
 * can be restored by psa_get_key_attributes
 * */
#if TFM_ENABLED == 0
    attributesCopy.core.policy.alg2 = originalAlgorithm;
#endif

    status = KeyStore_PSA_importKey(&attributesCopy, (uint8_t *)data, data_length, &keyStoreKeyID);
    *key   = toKeyID(keyStoreKeyID);

    if (status != KEYSTORE_PSA_STATUS_SUCCESS)
    {
        KeyStore_PSA_destroyKey(keyStoreKeyID);
        return status;
    }

/*
 * With TFM_ENABLED=1, key attributes struct only has one alg slot,
 * so store original algorithms in an array of length
 * TZ_ENABLED_KEY_STORAGE_LIMIT (TZ_ENABLED_KEY_STORAGE_LIMIT is the
 * max number of keys that can be stored at any one time)
 */
#if TFM_ENABLED == 1
    // if ((keyAlgorithmsLength < TZ_ENABLED_KEY_STORAGE_LIMIT) && (status == PSA_SUCCESS))
    // {
    //     keyAlgorithms[keyAlgorithmsLength].keyID = *key;
    //     keyAlgorithms[keyAlgorithmsLength].alg   = originalAlgorithm;
    //     keyAlgorithmsLength++;
    // }
    // else
    // {
    //     /* Too many keys have been created, so destroy the key */
    //     KeyStore_PSA_destroyKey(*key);
    //     status = PSA_ERROR_NOT_SUPPORTED;
    // }

    if (keyAlgorithmsLength < TZ_ENABLED_KEY_STORAGE_LIMIT)
    {
        keyAlgorithms[keyAlgorithmsLength].keyID = *key;
        keyAlgorithms[keyAlgorithmsLength].alg   = originalAlgorithm;
        keyAlgorithmsLength++;
    }
    else
    {
        KeyStore_PSA_destroyKey(keyStoreKeyID);
        status = PSA_ERROR_NOT_SUPPORTED;
    }
#endif

    return status;
}

psa_status_t psa_copy_key(psa_key_id_t source_key,
                          const psa_key_attributes_t *specified_attributes,
                          psa_key_id_t *target_key)
{
    return PSA_ERROR_NOT_SUPPORTED;
}

/****************************************************************/
/* Message digests */
/****************************************************************/

/*
 * addIntToCharArrayBigEndian, charArrayBigEndianToInt
 * Helper functions for psa_hash_suspend and psa_hash_resume
 */

/* Adds a 'numBytes' byte integer to a uint8 array in big endian format. */
#if TFM_ENABLED == 0
void static addIntToCharArrayBigEndian(uint32_t value, uint8_t numBytes, uint8_t *array)
{
    uint8_t rightShiftAmount = (numBytes * 8) - 8;
    uint32_t bitMask         = 0xff;
    for (uint8_t i = 0; i < numBytes; i++)
    {
        uint8_t byte = (value >> rightShiftAmount) & bitMask;
        array[i]     = byte;
        rightShiftAmount -= 8;
    }
}
#endif

/*
 * Extracts a 'numBytes' byte integer from the front of a uint8 array in big endian format.
 * numBytes must be <= 4
 */
#if TFM_ENABLED == 0
uint32_t static charArrayBigEndianToInt(const uint8_t *array, uint8_t numBytes)
{
    uint32_t value          = 0;
    uint8_t leftShiftAmount = (numBytes * 8) - 8;
    for (uint8_t i = 0; i < numBytes; i++)
    {
        value |= array[i] << leftShiftAmount;
        leftShiftAmount -= 8;
    }
    return value;
}
#endif

psa_status_t psa_hash_resume(psa_hash_operation_t *operation, const uint8_t *hash_state, size_t hash_state_length)
{
    /* Not supported for S/NS */
#if TFM_ENABLED == 1
    return PSA_ERROR_NOT_SUPPORTED;
#else
    SHA2CC26X2_Object *object = (SHA2CC26X2_Object *)SHA2_Hand->object;
    psa_algorithm_t alg       = charArrayBigEndianToInt(hash_state, 4);
    hash_state += 4;
    uint8_t inputLengthNumBytes = PSA_SHA2_SUSPEND_INPUT_LENGTH_FIELD_LENGTH(alg);
    uint8_t digestBufferLength  = PSA_SHA2_SUSPEND_HASH_STATE_FIELD_LENGTH(alg);
    uint8_t blockSize           = PSA_SHA2_BLOCK_SIZE(alg);

    /* Operation must be in an inactive state */
    if (operation->id != 0 || operation->alg != 0)
    {
        return PSA_ERROR_BAD_STATE;
    }

    /* SL drivers only support the following algorithms for now. */
    if (alg != PSA_ALG_SHA_224 && alg != PSA_ALG_SHA_256 && alg != PSA_ALG_SHA_384 && alg != PSA_ALG_SHA_512)
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    /* Check that the input length does not require more than 4 bytes to store */
    /* inputLengthNumBytes is either 8 or 16 but SHA2 driver stores input length as a 4 byte integer.
       Integer lengths that require more than 4 bytes are not supported. */
    for (uint8_t i = 0; i < inputLengthNumBytes - 4; i += 4)
    {
        uint32_t leadingZero = charArrayBigEndianToInt(hash_state, 4);
        hash_state += 4;
        if (leadingZero != 0)
        {
            return PSA_ERROR_NOT_SUPPORTED;
        }
    }

    uint32_t inputLength = charArrayBigEndianToInt(hash_state, 4);
    hash_state += 4;

    psa_status_t status = psa_hash_setup(operation, alg);
    if (status == PSA_SUCCESS)
    {
        object->bytesProcessed = (inputLength / blockSize) * blockSize;

        /* copy state to SHA2 driver object */
        memcpy(object->digest, hash_state, digestBufferLength);
        hash_state += digestBufferLength;

        /* copy any unprocessed data to SHA2 driver object */
        memcpy(object->buffer, hash_state, blockSize);
        object->bytesInBuffer = inputLength % blockSize;
    }
    return status;
#endif
}

psa_status_t psa_hash_suspend(psa_hash_operation_t *operation,
                              uint8_t *hash_state,
                              size_t hash_state_size,
                              size_t *hash_state_length)
{
    /* Not supported for S/NS */
#if TFM_ENABLED == 1
    return PSA_ERROR_NOT_SUPPORTED;
#else
    SHA2CC26X2_Object *object   = (SHA2CC26X2_Object *)SHA2_Hand->object;
    psa_algorithm_t alg         = operation->alg;
    uint8_t inputLengthNumBytes = PSA_SHA2_SUSPEND_INPUT_LENGTH_FIELD_LENGTH(alg);
    uint8_t hashStateNumBytes   = PSA_SHA2_SUSPEND_HASH_STATE_FIELD_LENGTH(alg);

    /* Operation must be in an active state */
    if (operation->id == 0 || operation->alg == 0)
    {
        return PSA_ERROR_BAD_STATE;
    }

    /* Buffer must be large enough to fit all the data */
    *hash_state_length = 4 + inputLengthNumBytes + hashStateNumBytes + object->bytesInBuffer;
    if (hash_state_size < *hash_state_length)
    {
        return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    /* SHA3 does not support neither suspend nor resume. */
    if (alg == PSA_ALG_SHA3_224 || alg == PSA_ALG_SHA3_256 || alg == PSA_ALG_SHA3_384 || alg == PSA_ALG_SHA3_512)
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    /*
     * As per the PSA spec:
     * The hash suspend state has the following format:
     * hash-suspend-state = algorithm || input-length || hash-state || unprocessed-input
     */

    /* Add alg (32 bit integer value) to uint8 array in big endian format */
    addIntToCharArrayBigEndian(operation->alg, 4, hash_state);
    hash_state += 4;

    uint32_t inputLength = object->bytesProcessed + object->bytesInBuffer;

    /* Add input length to uint8 array in big endian format  */
    addIntToCharArrayBigEndian(inputLength, inputLengthNumBytes, hash_state);
    hash_state += inputLengthNumBytes;

    /* Add the current hash data */

    /*
     * This may not be correct. Should change this to guaruntee big
     * endian encoding. The format of the hash-state as per the spec:
     * For SHA-224 and SHA-256: 8x 32-bit integers, in big-endian
     * encoding. For SHA-512, SHA-384 and SHA-512/256: 8x 64-bit integers,
     * in big-endian encoding.
     */
    memcpy(hash_state, object->digest, hashStateNumBytes);
    hash_state += hashStateNumBytes;

    /* Add the unprocessed-input */
    memcpy(hash_state, object->buffer, object->bytesInBuffer);

    /* operation should return to an inactive state */
    psa_hash_abort(operation);

    return PSA_SUCCESS;
#endif
}

psa_status_t psa_hash_abort(psa_hash_operation_t *operation)
{
    /* Aborting a non-active operation is allowed */
    if (operation->alg == 0)
    {
        return (PSA_SUCCESS);
    }

    /*
     * Call reset to clear the internal buffers for SHA2 and any
     * intermediate digests.
     */
    SHA2_reset(SHA2_Hand);
    *operation = PSA_HASH_OPERATION_INIT;

    return (PSA_SUCCESS);
}

psa_status_t psa_hash_setup(psa_hash_operation_t *operation, psa_algorithm_t alg)
{
    psa_status_t status = PSA_ERROR_CORRUPTION_DETECTED;

    /*
     * Except for first call to this fuction, this function cannot be
     * called witout calling abort psa_hash_abort operation, which resets t
     * the internal buffers for SHA2.
     */

    /* return this error to prevent out of order execution */
    if (operation->id != 0)
    {
        return PSA_ERROR_BAD_STATE;
    }
    /* accept only SHA2 alg */
    if (!PSA_ALG_IS_SHA2(alg))
    {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    /* there should be no alg in this object yet */
    if (operation->alg != 0)
    {
        return (PSA_ERROR_BAD_STATE);
    }

    SHA2_HashType hashType = map_hashAlgToHashType(alg);
    status                 = SHA2_setHashType(SHA2_Hand, hashType);
    operation->hashSize    = PSA_HASH_LENGTH(alg);

    if (status == PSA_SUCCESS)
    {
        operation->id = PSA_CRYPTO_TI_DRIVER;

        operation->alg = alg;
    }
    else
    {
        status = map_SHA2Error_to_PSA_error(status);
    }

    return status;
}

psa_status_t psa_hash_update(psa_hash_operation_t *operation, const uint8_t *input, size_t input_length)
{
    psa_status_t status = PSA_ERROR_CORRUPTION_DETECTED;

    /* return this error to prevent out of order execution */
    if (operation->id == 0)
    {
        return PSA_ERROR_BAD_STATE;
    }
    /* Don't require hash implementations to behave correctly on a
     * zero-length input, which may have an invalid pointer. */
    if (input_length == 0)
    {
        return (PSA_SUCCESS);
    }

    /*
     * Note replacing this with PSA_ALG_IS_HASH(operation->alg) is not
     * sufficent since we only support SHA)
     * */
    if (PSA_ALG_IS_SHA2(operation->alg))
    {

        status = SHA2_addData(SHA2_Hand, input, input_length);
    }
    /* support only for SHA2 at the moment */
    else
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    if (status != PSA_SUCCESS)
    {
        status = map_SHA2Error_to_PSA_error(status);
    }

    return (status);
}

psa_status_t psa_hash_finish(psa_hash_operation_t *operation, uint8_t *hash, size_t hash_size, size_t *hash_length)
{

    psa_status_t status       = PSA_SUCCESS;
    size_t actual_hash_length = PSA_HASH_LENGTH(operation->alg);
    /*
     * Fill the output buffer with something that isn't a valid hash
     * (barring an attack on the hash and deliberately-crafted input),
     * in case the caller doesn't check the return status properly.
     */
    *hash_length              = actual_hash_length;

    /* If hash_size is 0 then hash may be NULL and then the
     * call to memset would have undefined behavior. */
    if (hash_size != 0)
    {
        memset(hash, '!', hash_size);
    }

    /*
     * If the output length is smaller than the hash, then report user
     * mistake for this alg that does not support a shorten hash.
     */
    if (hash_size < actual_hash_length)
    {
        status = PSA_ERROR_BUFFER_TOO_SMALL;
    }
    else if (operation->id == 0)
    {
        status = PSA_ERROR_BAD_STATE;
    }
    /*
     * Note replacing this with PSA_ALG_IS_HASH(operation->alg)
     * is not sufficent since we only support SHA)
     */
    else if (operation->alg >= PSA_ALG_SHA_224 && operation->alg <= PSA_ALG_SHA_512)
    {
        status = SHA2_finalize(SHA2_Hand, hash);
        if (status != SHA2_STATUS_SUCCESS)
        {
            status = map_SHA2Error_to_PSA_error(status);
        }
    }
    else
    {
        status = PSA_ERROR_NOT_SUPPORTED;
    }

    psa_hash_abort(operation);
    return (status);
}

psa_status_t psa_hash_verify(psa_hash_operation_t *operation, const uint8_t *hash, size_t hash_length)
{
    /* max TI supported hash size */
    uint8_t actual_hash[512];
    memset(actual_hash, 0, 512);
    size_t actual_hash_length;

    /* first compute the hash */
    psa_status_t status = psa_hash_finish(operation, actual_hash, sizeof(actual_hash), &actual_hash_length);

    if (status != PSA_SUCCESS)
    {
        if (status == PSA_ERROR_BUFFER_TOO_SMALL)
        {
            status = PSA_ERROR_INVALID_SIGNATURE;
        }
    }
    else
    {
        /*
         * for verfication the hash_length provided must be the same
         * as the actual_hash_length otherwise it would not make sense
         * to verify.
         */
        if (actual_hash_length != hash_length)
        {
            status = PSA_ERROR_INVALID_SIGNATURE;
        }
        else
        {
            status = memcmp(hash, actual_hash, actual_hash_length);
            if (status != 0)
            {
                status = PSA_ERROR_INVALID_SIGNATURE;
            }
            else
            {
                status = PSA_SUCCESS;
            }
        }
    }

    /* flush the internal hash buffers */
    if (status != PSA_SUCCESS)
    {
        psa_hash_abort(operation);
    }

    return (status);
}

psa_status_t psa_hash_compute(psa_algorithm_t alg,
                              const uint8_t *input,
                              size_t input_length,
                              uint8_t *hash,
                              size_t hash_size,
                              size_t *hash_length)
{

    psa_status_t status       = PSA_ERROR_CORRUPTION_DETECTED;
    size_t actual_hash_length = PSA_HASH_LENGTH(alg);

    /* check if operation is a hash */
    if (!PSA_ALG_IS_HASH(alg))
    {
        return (PSA_ERROR_NOT_SUPPORTED);
    }

    /*
     * Fill the output buffer with something that isn't a valid hash
     * (barring an attack on the hash and deliberately-crafted input),
     * in case the caller doesn't check the return status properly.
     */
    *hash_length = actual_hash_length;

    /*
     * If hash_size is 0 then hash may be NULL and then the
     * call to memset would have undefined behavior.
     */
    if (hash_size != 0)
    {
        memset(hash, '!', hash_size);
    }

    /*
     * If the output length is smaller than the hash,
     * then report user mistake for this
     * alg that does not support a shorten hash.
     */
    if (hash_size < actual_hash_length)
    {
        status = PSA_ERROR_BUFFER_TOO_SMALL;
    }
    /*
     * Note replacing this with PSA_ALG_IS_HASH(operation->alg)
     * is not sufficent since we only support SHA)
     */
    else if (alg >= PSA_ALG_SHA_224 && alg <= PSA_ALG_SHA_512)
    {

        if (SHA2_Hand == NULL)
        {
            status = PSA_ERROR_BAD_STATE;
        }
        else
        {
            switch (alg)
            {
                case PSA_ALG_SHA_224:
                    status = SHA2_setHashType(SHA2_Hand, SHA2_HASH_TYPE_224);
                    break;

                case PSA_ALG_SHA_256:
                    status = SHA2_setHashType(SHA2_Hand, SHA2_HASH_TYPE_256);
                    break;

                case PSA_ALG_SHA_384:
                    status = SHA2_setHashType(SHA2_Hand, SHA2_HASH_TYPE_384);
                    break;

                case PSA_ALG_SHA_512:
                    status = SHA2_setHashType(SHA2_Hand, SHA2_HASH_TYPE_512);
                    break;
                default:
                    status = PSA_ERROR_NOT_SUPPORTED;
            }
        }

        if (status == PSA_SUCCESS)
        {
            status = SHA2_hashData(SHA2_Hand, input, input_length, hash);
        }

        if (status != SHA2_STATUS_SUCCESS)
        {
            status = map_SHA2Error_to_PSA_error(status);
        }
    }
    else
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    return status;
}

psa_status_t psa_hash_compare(psa_algorithm_t alg,
                              const uint8_t *input,
                              size_t input_length,
                              const uint8_t *hash,
                              size_t hash_length)
{
    /* max TI-SUPPORTED LENGTH */
    uint8_t actual_hash[512];
    size_t actual_hash_length;

    if (!PSA_ALG_IS_HASH(alg))
    {
        return (PSA_ERROR_NOT_SUPPORTED);
    }

    psa_status_t status = psa_hash_compute(alg, input, input_length, actual_hash, hash_length, &actual_hash_length);
    if (status != PSA_SUCCESS)
    {
        /*
         * In case the length was not correct for the output, we report
         * a invalid signature. Check crypto_values.h under the
         * PSA_ERROR_INVALID_ARGUMENT macro for further explanation
         * (Only in compare operation).
         */
        if (status == PSA_ERROR_BUFFER_TOO_SMALL)
        {
            status = PSA_ERROR_INVALID_ARGUMENT;
        }
    }
    else
    {
        status = memcmp(actual_hash, hash, hash_length);
        if (status != 0)
        {
            status = PSA_ERROR_INVALID_SIGNATURE;
        }
    }
    return status;
}

psa_status_t psa_hash_clone(const psa_hash_operation_t *source_operation, psa_hash_operation_t *target_operation)
{
    return (PSA_ERROR_NOT_SUPPORTED);
}

/****************************************************************/
/* MAC */
/****************************************************************/

psa_status_t psa_mac_abort(psa_mac_operation_t *operation)
{
    psa_status_t status = PSA_SUCCESS;

    /* Aborting a non-active operation is allowed */
    if (operation->id == 0)
    {
        return (PSA_SUCCESS);
    }

    if (PSA_ALG_IS_HMAC(operation->alg))
    {
        /* clear internal sha2 buffers */
        SHA2_reset(SHA2_Hand);
    }
    else
    {
        switch (operation->alg)
        {
            case PSA_ALG_CMAC:
                status = AESCMAC_cancelOperation(AESCMAC_Hand);
                break;

            case PSA_ALG_CBC_MAC:
                status = AESCMAC_cancelOperation(AESCBC_MAC_Hand);
                break;

            default:
                status = PSA_ERROR_NOT_SUPPORTED;
        }
    }

    if (status != PSA_SUCCESS)
    {
        status = map_AESError_to_PSA_error(status);
    }

    *operation = PSA_MAC_OPERATION_INIT;

    return status;
}

static psa_status_t psa_mac_finalize_alg_and_key_validation(psa_algorithm_t alg,
                                                            const psa_key_attributes_t *attributes,
                                                            uint8_t *mac_size)
{
    psa_status_t status     = PSA_ERROR_CORRUPTION_DETECTED;
    psa_key_type_t key_type = psa_get_key_type(attributes);
    size_t key_bits         = psa_get_key_bits(attributes);

    if (!PSA_ALG_IS_MAC(alg))
    {
        return (PSA_ERROR_INVALID_ARGUMENT);
    }

    /* Validate the combination of key type and algorithm */
    status = psa_mac_key_can_do(alg, key_type);
    if (status == PSA_SUCCESS)
    {

        /* Get the output length for the algorithm and key combination */
        *mac_size = PSA_MAC_LENGTH(key_type, key_bits, alg);

        if (*mac_size < 4)
        {
            /* A very short MAC is too short for security since it can be
             * brute-forced. Ancient protocols with 32-bit MACs do exist,
             * so we make this our minimum, even though 32 bits is still
             * too small for security. */
            status = (PSA_ERROR_NOT_SUPPORTED);
        }
        else if (*mac_size > PSA_MAC_LENGTH(key_type, key_bits, PSA_ALG_FULL_LENGTH_MAC(alg)))
        {
            /* It's impossible to "truncate" to a larger length than the full length
             * of the algorithm. */
            status = (PSA_ERROR_INVALID_ARGUMENT);
        }
        else
        {
            status = PSA_SUCCESS;
        }
    }
    return status;
}

static psa_status_t psa_mac_setup(psa_mac_operation_t *operation, psa_key_id_t key, psa_algorithm_t alg, int is_sign)
{
    psa_status_t status             = PSA_ERROR_CORRUPTION_DETECTED;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

    if (operation->id != 0)
    {
        return PSA_ERROR_BAD_STATE;
    }

    status = psa_get_key_attributes(key, &attributes);
    if (status != PSA_SUCCESS)
    {
        return status;
    }

    status = psa_mac_finalize_alg_and_key_validation(alg, &attributes, &operation->mac_size);
    if (status != PSA_SUCCESS)
    {
        psa_reset_key_attributes(&attributes);
        return status;
    }

    status = psa_key_attributes_usage_check(&attributes,
                                            is_sign ? PSA_KEY_USAGE_SIGN_MESSAGE : PSA_KEY_USAGE_VERIFY_MESSAGE,
                                            alg);
    if (status != PSA_SUCCESS)
    {
        psa_reset_key_attributes(&attributes);
        return status;
    }

    size_t key_bits = psa_get_key_bits(&attributes);

    KeyStore_PSA_initKey(&operation->cryptoKey, toKeyStoreKeyID(key), PSA_BITS_TO_BYTES(key_bits), NULL);

    operation->is_sign = is_sign;

    if (PSA_ALG_IS_HMAC(alg))
    {
        psa_algorithm_t hashAlg = PSA_ALG_HMAC_GET_HASH(alg);

        if (PSA_ALG_IS_SHA2(hashAlg))
        {

            SHA2_HashType hashType = map_hashAlgToHashType(hashAlg);
            status                 = SHA2_setHashType(SHA2_Hand, hashType);

            if (status == PSA_SUCCESS)
            {
                status = SHA2_setupHmac(SHA2_Hand, &operation->cryptoKey);
            }

            if (status != PSA_SUCCESS)
            {
                status = map_SHA2Error_to_PSA_error(status);
            }
        }
        else
        {
            status = PSA_ERROR_NOT_SUPPORTED;
        }
    }
    else
    {
        if (is_sign)
        {
            switch (PSA_ALG_FULL_LENGTH_MAC(alg))
            {
                case PSA_ALG_CMAC:
                    status = AESCMAC_setupSign(AESCMAC_Hand, &operation->cryptoKey);
                    break;

                case PSA_ALG_CBC_MAC:
                    status = AESCMAC_setupSign(AESCBC_MAC_Hand, &operation->cryptoKey);
                    break;

                default:
                    status = PSA_ERROR_NOT_SUPPORTED;
            }
        }
        else
        {
            switch (PSA_ALG_FULL_LENGTH_MAC(alg))
            {
                case PSA_ALG_CMAC:
                    status = AESCMAC_setupVerify(AESCMAC_Hand, &operation->cryptoKey);
                    break;

                case PSA_ALG_CBC_MAC:
                    status = AESCMAC_setupVerify(AESCBC_MAC_Hand, &operation->cryptoKey);
                    break;

                default:
                    status = PSA_ERROR_NOT_SUPPORTED;
            }
        }

        if (status != PSA_SUCCESS && status != PSA_ERROR_NOT_SUPPORTED)
        {
            status = map_AESError_to_PSA_error(status);
        }
    }

    if (status == PSA_SUCCESS)
    {
        /*
         * Store the algorithm as the full length version to
         * make algorithm checking easier.
         * No information is lost since the mac length is
         * stored in operation->mac_length
         */
        operation->alg             = PSA_ALG_FULL_LENGTH_MAC(alg);
        operation->id              = PSA_CRYPTO_TI_DRIVER;
        operation->unprocessed_len = 0;
        operation->lastBlockSet    = false;
    }

    psa_reset_key_attributes(&attributes);

    return status;
}

psa_status_t psa_mac_sign_setup(psa_mac_operation_t *operation, psa_key_id_t key, psa_algorithm_t alg)
{
    return (psa_mac_setup(operation, key, alg, 1));
}

psa_status_t psa_mac_verify_setup(psa_mac_operation_t *operation, psa_key_id_t key, psa_algorithm_t alg)
{
    return (psa_mac_setup(operation, key, alg, 0));
}

static psa_status_t psa_mac_update_internal(psa_mac_operation_t *operation, const uint8_t *input, size_t input_length)
{

    psa_status_t status;
    AESCMAC_Operation Operation;
    Operation.input       = (uint8_t *)input;
    Operation.inputLength = input_length;

    if (operation->alg == PSA_ALG_CMAC)
    {
        status = AESCMAC_addData(AESCMAC_Hand, &Operation);
    }
    else
    {
        status = AESCMAC_addData(AESCBC_MAC_Hand, &Operation);
    }

    if (status != AESCMAC_STATUS_SUCCESS)
    {
        status = map_AESError_to_PSA_error(status);
    }
    return status;
}

psa_status_t psa_mac_update(psa_mac_operation_t *operation, const uint8_t *input, size_t input_length)
{
    psa_status_t status = PSA_SUCCESS;
    if (operation->id == 0)
    {
        return PSA_ERROR_BAD_STATE;
    }

    /* Don't require hash implementations to behave correctly on a
     * zero-length input, which may have an invalid pointer. */
    if (input_length == 0)
        return (PSA_SUCCESS);

    if (operation->alg == 0)
    {
        status = PSA_ERROR_BAD_STATE;
        goto exit;
    }

    if (input == NULL)
    {
        status = PSA_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    if (!PSA_ALG_IS_MAC(operation->alg))
    {
        status = PSA_ERROR_NOT_SUPPORTED;
        goto exit;
    }

    if (operation->alg == PSA_ALG_CMAC || operation->alg == PSA_ALG_CBC_MAC)
    {
        /* Input needs to be buffered (and the final block of data needs to be saved for use in
         * psa_mac_sign_finish). the code must buffer all input but and must leave the unprocessedData buffer
         * unempty. In other words, AESCMAC_addData must not be called on any data that could potentially be the
         * last data passed in using this function. */
        size_t totalUnprocessedLen = operation->unprocessed_len + input_length;
        /* not enough total data to make up one block */
        if (totalUnprocessedLen < PSA_AES_BLOCK_SIZE)
        {
            /* buffer the data and wait for more data to arrive */
            memcpy(operation->unprocessedData + operation->unprocessed_len, input, input_length);
            operation->unprocessed_len = totalUnprocessedLen;
            return PSA_SUCCESS;
        }

        /* enough data to make up at least a single block */
        /* empty the buffer by filling it up and adding its data */
        /* we need to do this before adding the remaining data */
        size_t amountToCopy = PSA_AES_BLOCK_SIZE - operation->unprocessed_len;
        memcpy(operation->unprocessedData + operation->unprocessed_len, input, amountToCopy);
        input += amountToCopy;
        operation->unprocessed_len += amountToCopy;
        totalUnprocessedLen -= PSA_AES_BLOCK_SIZE;

        /* do not add the data if it could be the last block of data */
        if (totalUnprocessedLen == 0)
            return PSA_SUCCESS;

        status = psa_mac_update_internal(operation, operation->unprocessedData, PSA_AES_BLOCK_SIZE);

        if (status != PSA_SUCCESS)
            return status;

        operation->unprocessed_len = 0;

        /* Get the largest multiple of the block size that is less than the number of bytes of unprocessed data */
        size_t amountToAdd = ((int)(totalUnprocessedLen / PSA_AES_BLOCK_SIZE)) * PSA_AES_BLOCK_SIZE;

        /* to ensure that not all of the data is added */
        if (amountToAdd == totalUnprocessedLen)
        {
            amountToAdd -= PSA_AES_BLOCK_SIZE;
        }

        if (amountToAdd > 0)
        {

            status = psa_mac_update_internal(operation, input, amountToAdd);
            if (status != PSA_SUCCESS)
                return status;

            totalUnprocessedLen -= amountToAdd;
            input += amountToAdd;
        }

        /* add any left over data to the unprocessed data buffer */
        memcpy(operation->unprocessedData, input, totalUnprocessedLen);
        operation->unprocessed_len = totalUnprocessedLen;
    }

    else if (PSA_ALG_IS_HMAC(operation->alg))

    {

        status = SHA2_addData(SHA2_Hand, input, input_length);
        if (status != SHA2_STATUS_SUCCESS)
        {
            status = PSA_ERROR_HARDWARE_FAILURE;
        }
    }

    else
    {
        status = PSA_ERROR_NOT_SUPPORTED;
    }

exit:

    return (status);
}

psa_status_t psa_mac_sign_finish(psa_mac_operation_t *operation, uint8_t *mac, size_t mac_size, size_t *mac_length)
{
    psa_status_t status = PSA_SUCCESS;

    if (operation->id == 0)
    {
        status = PSA_ERROR_BAD_STATE;
        goto exit;
    }

    if (!operation->is_sign)
    {
        status = PSA_ERROR_BAD_STATE;
        goto exit;
    }

    /* Sanity check. This will guarantee that mac_size != 0 (and so mac != NULL)
     * once all the error checks are done. */
    if (operation->mac_size == 0)
    {
        status = PSA_ERROR_BAD_STATE;
        goto exit;
    }

    if (mac_size < operation->mac_size)
    {
        status = PSA_ERROR_BUFFER_TOO_SMALL;
        goto exit;
    }

    /* use the default computed mac size or the mac_size passed in */
    if (operation->alg == PSA_ALG_CMAC || operation->alg == PSA_ALG_CBC_MAC)
    {

        AESCMAC_Operation Operation;
        Operation.input       = operation->unprocessedData;
        Operation.inputLength = operation->unprocessed_len;
        Operation.mac         = mac;
        Operation.macLength   = operation->mac_size;

        /* AESCMAC_finalize does not accept input lengths of zero,
         * so if there is no input, call oneStepSign, otherwise call finalize */
        if (operation->unprocessed_len == 0)
        {
            if (operation->alg == PSA_ALG_CMAC)
            {
                AESCMAC_cancelOperation(AESCMAC_Hand);
                status = AESCMAC_oneStepSign(AESCMAC_Hand, &Operation, &operation->cryptoKey);
            }
            else
            {
                AESCMAC_cancelOperation(AESCBC_MAC_Hand);
                status = AESCMAC_oneStepSign(AESCBC_MAC_Hand, &Operation, &operation->cryptoKey);
            }

            if (status != AESCMAC_STATUS_SUCCESS)
            {
                status = map_AESError_to_PSA_error(status);
                goto exit;
            }
        }
        else
        {
            if (operation->alg == PSA_ALG_CMAC)
                status = AESCMAC_finalize(AESCMAC_Hand, &Operation);
            else
                status = AESCMAC_finalize(AESCBC_MAC_Hand, &Operation);

            if (status != AESCMAC_STATUS_SUCCESS)
            {
                status = map_AESError_to_PSA_error(status);
                goto exit;
            }
        }
    }

    else if (PSA_ALG_IS_HMAC(operation->alg))

    {

        status = SHA2_finalizeHmac(SHA2_Hand, mac);
        if (status != SHA2_STATUS_SUCCESS)
        {
            status = map_SHA2Error_to_PSA_error(status);
        }
    }

exit:
    /* In case of success, set the potential excess room in the output buffer
     * to an invalid value, to avoid potentially leaking a longer MAC.
     * In case of error, set the output length and content to a safe default,
     * such that in case the caller misses an error check, the output would be
     * an unachievable MAC.
     */
    if (status != PSA_SUCCESS)
    {
        *mac_length         = 0;
        operation->mac_size = 0;
    }

    *mac_length = operation->mac_size;

    if (mac_size > operation->mac_size)
        memset(&mac[operation->mac_size], '!', mac_size - operation->mac_size);

    psa_mac_abort(operation);

    return status;
}

psa_status_t psa_mac_verify_finish(psa_mac_operation_t *operation, const uint8_t *mac, size_t mac_length)
{

    psa_status_t status = PSA_SUCCESS;

    if (operation->id == 0)
    {
        status = PSA_ERROR_BAD_STATE;
        goto exit;
    }

    if (operation->is_sign)
    {
        status = PSA_ERROR_BAD_STATE;
        goto exit;
    }

    /* Sanity check. This will guarantee that mac_size != 0 (and so mac != NULL)
     * once all the error checks are done. */
    if (operation->mac_size == 0)
    {
        status = PSA_ERROR_BAD_STATE;
        goto exit;
    }

    if (mac_length != operation->mac_size)
    {
        status = PSA_ERROR_INVALID_SIGNATURE;
        goto exit;
    }

    /* use the default computed mac size or the mac_size passed in */
    if (operation->alg == PSA_ALG_CMAC || operation->alg == PSA_ALG_CBC_MAC)
    {

        AESCMAC_Operation Operation;
        Operation.input       = operation->unprocessedData;
        Operation.inputLength = operation->unprocessed_len;
        Operation.mac         = (uint8_t *)mac;
        Operation.macLength   = operation->mac_size;

        /* AESCMAC_finalize does not accept input lengths of zero,
         * so if there is no input, call oneStepVerify, otherwise call finalize */
        if (operation->unprocessed_len == 0)
        {
            if (operation->alg == PSA_ALG_CMAC)
            {
                AESCMAC_cancelOperation(AESCMAC_Hand);
                status = AESCMAC_oneStepVerify(AESCMAC_Hand, &Operation, &operation->cryptoKey);
            }
            else
            {
                AESCMAC_cancelOperation(AESCBC_MAC_Hand);
                status = AESCMAC_oneStepVerify(AESCBC_MAC_Hand, &Operation, &operation->cryptoKey);
            }

            if (status != AESCMAC_STATUS_SUCCESS)
            {
                status = map_AESError_to_PSA_error(status);
                goto exit;
            }
        }
        else
        {
            if (operation->alg == PSA_ALG_CMAC)
                status = AESCMAC_finalize(AESCMAC_Hand, &Operation);
            else
                status = AESCMAC_finalize(AESCBC_MAC_Hand, &Operation);

            if (status != AESCMAC_STATUS_SUCCESS)
            {
                status = map_AESError_to_PSA_error(status);
                goto exit;
            }
        }
    }
    else if (PSA_ALG_IS_HMAC(operation->alg))

    {
        uint8_t actualMAC[PSA_HMAC_MAX_HASH_BLOCK_SIZE] = {0};
        status                                          = SHA2_finalizeHmac(SHA2_Hand, actualMAC);
        if (status != SHA2_STATUS_SUCCESS)
        {
            status = map_SHA2Error_to_PSA_error(status);
        }

        status = memcmp(actualMAC, mac, operation->mac_size);
        if (status != 0)
        {
            status = PSA_ERROR_INVALID_SIGNATURE;
        }
    }

exit:

    psa_mac_abort(operation);

    return status;
}

psa_status_t psa_mac_compute(psa_key_id_t key,
                             psa_algorithm_t alg,
                             const uint8_t *input,
                             size_t input_length,
                             uint8_t *mac,
                             size_t mac_size,
                             size_t *mac_length)
{
    psa_status_t status             = PSA_ERROR_CORRUPTION_DETECTED;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    uint8_t operation_mac_size      = 0;
    psa_algorithm_t algBase;
    size_t key_bits;
    CryptoKey cryptoKey;

    status = psa_get_key_attributes(key, &attributes);
    if (status != PSA_SUCCESS)
        goto exit;

    key_bits = psa_get_key_bits(&attributes);

    status = psa_key_attributes_usage_check(&attributes, PSA_KEY_USAGE_SIGN_MESSAGE, alg);
    if (status != PSA_SUCCESS)
        goto exit;

    /* compute operation mac size */
    status = psa_mac_finalize_alg_and_key_validation(alg, &attributes, &operation_mac_size);
    if (status != PSA_SUCCESS)
        goto exit;

    if (mac_size < operation_mac_size)
    {
        status = PSA_ERROR_BUFFER_TOO_SMALL;
        goto exit;
    }

    KeyStore_PSA_initKey(&cryptoKey, toKeyStoreKeyID(key), PSA_BITS_TO_BYTES(key_bits), NULL);

    algBase = PSA_ALG_FULL_LENGTH_MAC(alg);

    if (algBase == PSA_ALG_CMAC || algBase == PSA_ALG_CBC_MAC)
    {
        if (algBase == PSA_ALG_CMAC && AESCMAC_Hand == NULL)
        {
            status = PSA_ERROR_BAD_STATE;
            goto exit;
        }
        else if (algBase == PSA_ALG_CBC_MAC && AESCBC_MAC_Hand == NULL)
        {
            status = PSA_ERROR_BAD_STATE;
            goto exit;
        }

        AESCMAC_Operation operation;
        operation.input       = (uint8_t *)input;
        operation.inputLength = input_length;
        operation.mac         = mac;
        operation.macLength   = operation_mac_size;
        if (algBase == PSA_ALG_CMAC)
            status = AESCMAC_oneStepSign(AESCMAC_Hand, &operation, &cryptoKey);
        else
            status = AESCMAC_oneStepSign(AESCBC_MAC_Hand, &operation, &cryptoKey);

        if (status != AESCMAC_STATUS_SUCCESS)
        {
            status = map_AESError_to_PSA_error(status);
            goto exit;
        }
    }
    else if (PSA_ALG_IS_HMAC(alg))
    {
        psa_algorithm_t algorithm = PSA_ALG_HMAC_GET_HASH(alg);
        if (SHA2_Hand == NULL)
        {
            status = PSA_ERROR_BAD_STATE;
            goto exit;
        }
        switch (algorithm)
        {
            case PSA_ALG_SHA_224:
                status = SHA2_setHashType(SHA2_Hand, SHA2_HASH_TYPE_224);
                break;

            case PSA_ALG_SHA_256:

                status = SHA2_setHashType(SHA2_Hand, SHA2_HASH_TYPE_256);

                break;
            case PSA_ALG_SHA_384:
                status = SHA2_setHashType(SHA2_Hand, SHA2_HASH_TYPE_384);

                break;

            case PSA_ALG_SHA_512:
                status = SHA2_setHashType(SHA2_Hand, SHA2_HASH_TYPE_512);
                break;

            default:
                status = PSA_ERROR_INVALID_ARGUMENT;
        }

        if (status == SHA2_STATUS_SUCCESS)
        {

            status = SHA2_hmac((SHA2_Hand), &cryptoKey, input, input_length, mac);
            if (status != SHA2_STATUS_SUCCESS)
            {
                status = map_SHA2Error_to_PSA_error(status);
            }
        }
        else
        {
            status = map_SHA2Error_to_PSA_error(status);
        }
    }
    else
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }

exit:

    if (status == PSA_SUCCESS)
    {
        *mac_length = operation_mac_size;
    }
    psa_reset_key_attributes(&attributes);

    return status;
}

psa_status_t psa_mac_verify(psa_key_id_t key,
                            psa_algorithm_t alg,
                            const uint8_t *input,
                            size_t input_length,
                            const uint8_t *mac,
                            size_t mac_length)
{
    psa_status_t status             = PSA_ERROR_CORRUPTION_DETECTED;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    uint8_t operation_mac_size      = 0;
    size_t key_bits;
    psa_algorithm_t algBase;
    CryptoKey cryptoKey;

    status = psa_get_key_attributes(key, &attributes);
    if (status != PSA_SUCCESS)
        goto exit;

    key_bits = psa_get_key_bits(&attributes);

    status = psa_key_attributes_usage_check(&attributes, PSA_KEY_USAGE_VERIFY_MESSAGE, alg);
    if (status != PSA_SUCCESS)
        goto exit;

    /* compute operation mac size */
    status = psa_mac_finalize_alg_and_key_validation(alg, &attributes, &operation_mac_size);
    if (status != PSA_SUCCESS)
        goto exit;

    if (mac_length < operation_mac_size)
    {
        status = PSA_ERROR_INVALID_SIGNATURE;
        goto exit;
    }

    KeyStore_PSA_initKey(&cryptoKey, toKeyStoreKeyID(key), PSA_BITS_TO_BYTES(key_bits), NULL);

    algBase = PSA_ALG_FULL_LENGTH_MAC(alg);
    if (algBase == PSA_ALG_CMAC || algBase == PSA_ALG_CBC_MAC)
    {
        if (algBase == PSA_ALG_CMAC && AESCMAC_Hand == NULL)
        {
            status = PSA_ERROR_BAD_STATE;
            goto exit;
        }
        else if (algBase == PSA_ALG_CBC_MAC && AESCBC_MAC_Hand == NULL)
        {
            status = PSA_ERROR_BAD_STATE;
            goto exit;
        }

        AESCMAC_Operation operation;
        operation.input       = (uint8_t *)input;
        operation.inputLength = input_length;
        operation.mac         = (uint8_t *)mac;
        operation.macLength   = mac_length;
        if (algBase == PSA_ALG_CMAC)
            status = AESCMAC_oneStepVerify(AESCMAC_Hand, &operation, &cryptoKey);
        else
            status = AESCMAC_oneStepVerify(AESCBC_MAC_Hand, &operation, &cryptoKey);

        if (status != AESCMAC_STATUS_SUCCESS)
        {
            status = map_AESError_to_PSA_error(status);
            goto exit;
        }
    }
    else if (PSA_ALG_IS_HMAC(alg))
    {
        psa_algorithm_t algorithm = PSA_ALG_HMAC_GET_HASH(alg);

        if (SHA2_Hand == NULL)
        {
            status = PSA_ERROR_BAD_STATE;
            goto exit;
        }
        switch (algorithm)
        {
            case (PSA_ALG_SHA_224):
                status = SHA2_setHashType(SHA2_Hand, SHA2_HASH_TYPE_224);
                break;
            case (PSA_ALG_SHA_256):
                status = SHA2_setHashType(SHA2_Hand, SHA2_HASH_TYPE_256);
                break;
            case (PSA_ALG_SHA_384):
                status = SHA2_setHashType(SHA2_Hand, SHA2_HASH_TYPE_384);
                break;
            case (PSA_ALG_SHA_512):
                status = SHA2_setHashType(SHA2_Hand, SHA2_HASH_TYPE_512);
                break;
            default:
                status = PSA_ERROR_INVALID_ARGUMENT;
        }

        if (status == SHA2_STATUS_SUCCESS)
        {
            uint8_t tempOutputMac[SHA2_DIGEST_LENGTH_BYTES_512];
            status = SHA2_hmac((SHA2_Hand), &cryptoKey, input, input_length, tempOutputMac);
            if (status != SHA2_STATUS_SUCCESS)
            {
                status = map_SHA2Error_to_PSA_error(status);
            }

            /* add a verfication step here */
            status = memcmp(tempOutputMac, mac, mac_length);
            if (status != 0)
            {
                /* No match */
                status = PSA_ERROR_INVALID_SIGNATURE;
            }
        }
        else
        {
            status = map_SHA2Error_to_PSA_error(status);
        }
    }
    else
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }

exit:

    psa_reset_key_attributes(&attributes);

    return status;
}

/****************************************************************/
/* Asymmetric cryptography */
/****************************************************************/

#if TFM_ENABLED == 0
static psa_status_t psa_sign_verify_check_alg(int input_is_message, psa_algorithm_t alg)
{
    if (input_is_message)
    {
        if (!PSA_ALG_IS_SIGN_MESSAGE(alg))
            return (PSA_ERROR_INVALID_ARGUMENT);

        if (PSA_ALG_IS_HASH_AND_SIGN(alg))
        {
            if (!PSA_ALG_IS_HASH(PSA_ALG_SIGN_GET_HASH(alg)))
                return (PSA_ERROR_INVALID_ARGUMENT);
        }
    }
    else
    {
        if (!PSA_ALG_IS_HASH_AND_SIGN(alg))
            return (PSA_ERROR_INVALID_ARGUMENT);
    }

    return (PSA_SUCCESS);
}
#endif

psa_status_t psa_sign_message(psa_key_id_t key,
                              psa_algorithm_t alg,
                              const uint8_t *input,
                              size_t input_length,
                              uint8_t *signature,
                              size_t signature_size,
                              size_t *signature_length)
{
/* No S/NS support for ECDSA and EDDSA */
#if TFM_ENABLED == 1
    return PSA_ERROR_NOT_SUPPORTED;
#else
    psa_status_t status             = PSA_ERROR_CORRUPTION_DETECTED;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    uint8_t *myPublicKeyMaterial    = NULL;
    uint8_t *keyMaterial            = NULL;

    size_t hash_length;
    uint8_t *hashed_input = NULL;
    size_t hash_size;
    psa_key_type_t key_type;
    size_t key_bits;

    CryptoKey myPrivateKey;
    CryptoKey myPublicKey;

    status = psa_get_key_attributes(key, &attributes);
    if (status != PSA_SUCCESS)
        return status;

    status = psa_key_attributes_usage_check(&attributes, PSA_KEY_USAGE_SIGN_MESSAGE, alg);
    if (status != PSA_SUCCESS)
        goto exit;

    key_type        = psa_get_key_type(&attributes);
    key_bits = psa_get_key_bits(&attributes);

    *signature_length = PSA_SIGN_OUTPUT_SIZE(key_type, key_bits, alg);

    /* 1 to rep message not hash */
    status = psa_sign_verify_check_alg(1, alg);
    if (status != PSA_SUCCESS)
        return status;

    /* Immediately reject a zero-length signature buffer. This guarantees
     * that signature must be a valid pointer. (On the other hand, the input
     * buffer can in principle be empty since it doesn't actually have
     * to be a hash.) */
    if (signature_size == 0)
        return (PSA_ERROR_BUFFER_TOO_SMALL);

    if (!PSA_KEY_TYPE_IS_KEY_PAIR(key_type))
    {
        status = PSA_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    size_t keyBytes = PSA_BITS_TO_BYTES(key_bits);
    keyMaterial     = (uint8_t *)malloc(keyBytes);
    psa_export_key(key, keyMaterial, keyBytes, &keyBytes);
    CryptoKeyPlaintext_initKey(&myPrivateKey, keyMaterial, keyBytes);

    psa_algorithm_t algorithmSHA2 = PSA_ALG_SIGN_GET_HASH(alg);

    /* The twisted Edwards curves Ed25519
     * Public key generated in this step */

    if (alg == PSA_ALG_ED25519PH)
    {

        if (EDDSA_Hand == NULL)
        {
            status = PSA_ERROR_BAD_STATE;
            goto exit;
        }

        /* first generate public key */
        myPublicKeyMaterial = (uint8_t *)malloc(ECCParams_CURVE25519_LENGTH);
        CryptoKeyPlaintext_initBlankKey(&myPublicKey, myPublicKeyMaterial, ECCParams_CURVE25519_LENGTH);
        EDDSA_OperationGeneratePublicKey EDDSA_operationGeneratePublicKey;
        EDDSA_OperationGeneratePublicKey_init(&EDDSA_operationGeneratePublicKey);
        EDDSA_operationGeneratePublicKey.myPrivateKey = &myPrivateKey;
        EDDSA_operationGeneratePublicKey.myPublicKey  = &myPublicKey;
        EDDSA_operationGeneratePublicKey.curve        = &ECCParams_Ed25519;

        status = EDDSA_generatePublicKey(EDDSA_Hand, &EDDSA_operationGeneratePublicKey);

        if (status != EDDSA_STATUS_SUCCESS)
        {
            status = map_EDDSAError_to_PSA_error(status);
            goto exit;
        }

        /* change the key handle data to the public key content */

        EDDSA_OperationSign EDDSA_operationSign;
        EDDSA_OperationSign_init(&EDDSA_operationSign);
        EDDSA_operationSign.curve                  = &ECCParams_Ed25519;
        EDDSA_operationSign.myPrivateKey           = &myPrivateKey;
        EDDSA_operationSign.myPublicKey            = &myPublicKey;
        EDDSA_operationSign.preHashedMessage       = input;
        EDDSA_operationSign.preHashedMessageLength = input_length;
        EDDSA_operationSign.R                      = signature;
        EDDSA_operationSign.S                      = signature + *signature_length / 2;

        status = EDDSA_sign(EDDSA_Hand, &EDDSA_operationSign);
        if (status != EDDSA_STATUS_SUCCESS)
        {
            status = map_EDDSAError_to_PSA_error(status);
            goto exit;
        }
    }
    else if (PSA_ALG_IS_RANDOMIZED_ECDSA(alg) && PSA_KEY_TYPE_IS_ECC(key_type))
    {
        if (ECDSA_Hand == NULL)
        {
            status = PSA_ERROR_BAD_STATE;
            goto exit;
        }
        ECDSA_OperationSign operationSign;
        ECDSA_OperationSign_init(&operationSign);

        operationSign.curve = map_keyTypeToECCParams(key_type, key_bits);

        if (operationSign.curve == NULL)
        {
            status = PSA_ERROR_NOT_SUPPORTED;
            goto exit;
        }

        switch (alg)
        {
            case PSA_ALG_ECDSA(PSA_ALG_SHA_256):
                hash_size = SHA2_DIGEST_LENGTH_BYTES_256;
                break;
            case PSA_ALG_ECDSA(PSA_ALG_SHA_384):
                hash_size = SHA2_DIGEST_LENGTH_BYTES_384;
                break;
            case PSA_ALG_ECDSA(PSA_ALG_SHA_512):
                hash_size = SHA2_DIGEST_LENGTH_BYTES_512;
                break;
            default:
                status = PSA_ERROR_NOT_SUPPORTED;
        }

        /* handle curves for family */
        operationSign.myPrivateKey = &myPrivateKey;
        /* input must be hashed */
        hashed_input               = malloc(hash_size);
        if (hashed_input == NULL)
        {
            status = PSA_ERROR_INSUFFICIENT_MEMORY;
            goto exit;
        }

        /* compute the hash */
        status = psa_hash_compute(algorithmSHA2, input, input_length, hashed_input, hash_size, &hash_length);

        if (status != PSA_SUCCESS)
        {
            status = map_SHA2Error_to_PSA_error(status);
            goto exit;
        }

        operationSign.hash = hashed_input;
        operationSign.r    = signature;
        operationSign.s    = signature + *signature_length / 2;

        /* generates the signature */
        status = ECDSA_sign(ECDSA_Hand, &operationSign);
        if (status != ECDSA_STATUS_SUCCESS)
        {
            status = map_ECDSAError_to_PSA_error(status);
        }
    }
    else
    {
        status = PSA_ERROR_NOT_SUPPORTED;
    }
    /* check the  Supported Curve Types in the ECDSACC26X2.h file
     * Montgomery and  Edwards curve types are not supported, yet. */

exit:

    if (hashed_input != NULL)
    {
        free(hashed_input);
    }

    if (myPublicKeyMaterial != NULL)
    {
        free(myPublicKeyMaterial);
    }

    if (keyMaterial != NULL)
    {
        free(keyMaterial);
    }

    psa_reset_key_attributes(&attributes);

    return (status);
#endif
}

psa_status_t psa_verify_message_builtin(const psa_key_attributes_t *attributes,
                                        const uint8_t *key_buffer,
                                        size_t key_buffer_size,
                                        psa_algorithm_t alg,
                                        const uint8_t *input,
                                        size_t input_length,
                                        const uint8_t *signature,
                                        size_t signature_length)
{
    return (PSA_ERROR_NOT_SUPPORTED);
}

psa_status_t psa_verify_message(psa_key_id_t key,
                                psa_algorithm_t alg,
                                const uint8_t *input,
                                size_t input_length,
                                const uint8_t *signature,
                                size_t signature_length)
{
/* No S/NS support for ECDSA and EDDSA */
#if TFM_ENABLED == 1
    return PSA_ERROR_NOT_SUPPORTED;
#else
    psa_status_t status = PSA_ERROR_CORRUPTION_DETECTED;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    CryptoKey myPublicKey;
    uint8_t *keyMaterial = NULL;

    size_t hash_length;
    uint8_t *hashed_input = NULL;
    size_t hash_size;

    status = psa_get_key_attributes(key, &attributes);
    if (status != PSA_SUCCESS)
        goto exit;

    status = psa_key_attributes_usage_check(&attributes, PSA_KEY_USAGE_VERIFY_MESSAGE, alg);
    if (status != PSA_SUCCESS)
        goto exit;

    psa_key_type_t key_type = psa_get_key_type(&attributes);
    psa_key_bits_t key_bits = psa_get_key_bits(&attributes);

    /* 1 to rep message not hash */
    status = psa_sign_verify_check_alg(1, alg);
    if (status != PSA_SUCCESS)
        return status;

    /* Immediately reject a zero-length signature buffer. This guarantees
     * that signature must be a valid pointer. (On the other hand, the input
     * buffer can in principle be empty since it doesn't actually have
     * to be a hash.) */
    if (signature_length == 0)
        return (PSA_ERROR_BUFFER_TOO_SMALL);

    psa_ecc_curve_t curve = PSA_KEY_TYPE_ECC_GET_FAMILY(key_type);
    size_t keyBytes = (curve == PSA_ECC_FAMILY_MONTGOMERY || curve == PSA_ECC_FAMILY_TWISTED_EDWARDS)
                          ? PSA_BITS_TO_BYTES(key_bits)
                          : PSA_KEY_EXPORT_ECC_PUBLIC_KEY_MAX_SIZE(key_bits);
    keyMaterial = malloc(keyBytes);
    psa_export_key(key, keyMaterial, keyBytes, &keyBytes);
    CryptoKeyPlaintext_initKey(&myPublicKey, keyMaterial, keyBytes);

    if (alg == PSA_ALG_ED25519PH)
    {
        if (EDDSA_Hand == NULL)
        {
            status = PSA_ERROR_BAD_STATE;
            goto exit;
        }

        /* The twisted Edwards curves Ed25519 */

        EDDSA_OperationVerify EDDSA_operationVerify;
        EDDSA_OperationVerify_init(&EDDSA_operationVerify);
        EDDSA_operationVerify.curve = &ECCParams_Ed25519;
        EDDSA_operationVerify.theirPublicKey = &myPublicKey;
        EDDSA_operationVerify.preHashedMessage = input;
        EDDSA_operationVerify.preHashedMessageLength = input_length;
        EDDSA_operationVerify.R = signature;
        EDDSA_operationVerify.S = signature + signature_length / 2;

        status = EDDSA_verify(EDDSA_Hand, &EDDSA_operationVerify);
        if (status != EDDSA_STATUS_SUCCESS)
            status = map_EDDSAError_to_PSA_error(status);
    }
    else if (PSA_ALG_IS_RANDOMIZED_ECDSA(alg) && PSA_KEY_TYPE_IS_ECC(key_type))
    {
        if (ECDSA_Hand == NULL)
        {
            status = PSA_ERROR_BAD_STATE;
            goto exit;
        }
        ECDSA_OperationVerify operationVerify;
        ECDSA_OperationVerify_init(&operationVerify);

        operationVerify.curve = map_keyTypeToECCParams(key_type, key_bits);

        if (operationVerify.curve == NULL)
        {
            status = PSA_ERROR_NOT_SUPPORTED;
            goto exit;
        }

        switch (alg)
        {
            case PSA_ALG_ECDSA(PSA_ALG_SHA_256):
                hash_size = SHA2_DIGEST_LENGTH_BYTES_256;
                break;
            case PSA_ALG_ECDSA(PSA_ALG_SHA_384):
                hash_size = SHA2_DIGEST_LENGTH_BYTES_384;
                break;
            case PSA_ALG_ECDSA(PSA_ALG_SHA_512):
                hash_size = SHA2_DIGEST_LENGTH_BYTES_512;
                break;
            default:
                status = PSA_ERROR_NOT_SUPPORTED;
                goto exit;
        }

        operationVerify.theirPublicKey = &myPublicKey;
        /* input must be hashed */
        hashed_input = malloc(hash_size);
        if (hashed_input == NULL)
        {
            status = PSA_ERROR_INSUFFICIENT_MEMORY;
            goto exit;
        }

        status = psa_hash_compute(PSA_ALG_SHA_256, input, input_length, hashed_input, hash_size, &hash_length);
        if (status != PSA_SUCCESS)
        {
            goto exit;
        }

        operationVerify.hash = hashed_input;
        operationVerify.r = signature;
        operationVerify.s = signature + signature_length / 2;

        /* generates the signature */
        status = ECDSA_verify(ECDSA_Hand, &operationVerify);
        if (status != ECDSA_STATUS_SUCCESS)
        {
            status = map_ECDSAError_to_PSA_error(status);
        }
    }
    else
    {
        status = PSA_ERROR_NOT_SUPPORTED;
    }

exit:

    if (hashed_input != NULL)
    {
        free(hashed_input);
    }

    if (keyMaterial != NULL)
    {
        free(keyMaterial);
    }

    psa_reset_key_attributes(&attributes);
    return status;
#endif
}

psa_status_t psa_sign_hash_builtin(const psa_key_attributes_t *attributes,
                                   const uint8_t *key_buffer,
                                   size_t key_buffer_size,
                                   psa_algorithm_t alg,
                                   const uint8_t *hash,
                                   size_t hash_length,
                                   uint8_t *signature,
                                   size_t signature_size,
                                   size_t *signature_length)
{

    return (PSA_ERROR_NOT_SUPPORTED);
}

psa_status_t psa_sign_hash(psa_key_id_t key,
                           psa_algorithm_t alg,
                           const uint8_t *hash,
                           size_t hash_length,
                           uint8_t *signature,
                           size_t signature_size,
                           size_t *signature_length)
{
/* No S/NS support for ECDSA and EDDSA */
#if TFM_ENABLED == 1
    return PSA_ERROR_NOT_SUPPORTED;
#else
    psa_status_t status = PSA_ERROR_CORRUPTION_DETECTED;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    uint8_t *keyMaterial = NULL;

    CryptoKey myPrivateKey;

    *signature_length = 0;

    /* 0 to rep hash */
    status = psa_sign_verify_check_alg(0, alg);
    if (status != PSA_SUCCESS)
        return status;

    /* Immediately reject a zero-length signature buffer. This guarantees
     * that signature must be a valid pointer. (On the other hand, the input
     * buffer can in principle be empty since it doesn't actually have
     * to be a hash.) */
    if (signature_size == 0)
        return (PSA_ERROR_BUFFER_TOO_SMALL);

    status = psa_get_key_attributes(key, &attributes);
    if (status != PSA_SUCCESS)
        goto exit;

    status = psa_key_attributes_usage_check(&attributes, PSA_KEY_USAGE_SIGN_HASH, alg);
    if (status != PSA_SUCCESS)
        goto exit;

    /* use key_type to map to curve type */
    psa_key_type_t privateKeyType = psa_get_key_type(&attributes);
    size_t key_bits = psa_get_key_bits(&attributes);

    /* SL Crypto drivers only support ECC */
    if (!PSA_KEY_TYPE_IS_ECC_KEY_PAIR(privateKeyType))
    {
        status = PSA_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    /* only support ECDSA for psa_sign_hash */
    if (!PSA_ALG_IS_ECDSA(alg))
    {
        status = PSA_ERROR_NOT_SUPPORTED;
        goto exit;
    }

    /* no support for deterministic ecdsa */

    if (PSA_ALG_IS_DETERMINISTIC_ECDSA(alg))
    {
        status = PSA_ERROR_NOT_SUPPORTED;
        goto exit;
    }

    size_t curveBits = psa_get_key_bits(&attributes);
    size_t curveBytes = PSA_BITS_TO_BYTES(curveBits);
    size_t outputSize = PSA_SIGN_OUTPUT_SIZE(privateKeyType, curveBits, alg);
    if (signature_size < outputSize)
    {
        status = PSA_ERROR_BUFFER_TOO_SMALL;
        goto exit;
    }

    if (hash_length < curveBytes)
    {
        status = PSA_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    size_t keyBytes = PSA_BITS_TO_BYTES(key_bits);
    keyMaterial = malloc(keyBytes);
    psa_export_key(key, keyMaterial, keyBytes, &keyBytes);
    CryptoKeyPlaintext_initKey(&myPrivateKey, keyMaterial, keyBytes);

    ECDSA_OperationSign operationSign;
    ECDSA_OperationSign_init(&operationSign);
    /* Short Weierstrass curves */

    operationSign.curve = map_keyTypeToECCParams(privateKeyType, key_bits);
    if (operationSign.curve == NULL)
    {
        status = PSA_ERROR_NOT_SUPPORTED;
        goto exit;
    }

    operationSign.myPrivateKey = &myPrivateKey;
    operationSign.hash = hash;
    operationSign.r = signature;
    operationSign.s = signature + curveBytes;

    /* generates the signature */
    status = ECDSA_sign(ECDSA_Hand, &operationSign);
    if (status != 0)
    {
        status = map_ECDSAError_to_PSA_error(status);
        goto exit;
    }

    *signature_length = outputSize;

exit:
    psa_reset_key_attributes(&attributes);

    if (keyMaterial != NULL)
    {
        free(keyMaterial);
    }

    return status;
#endif
}

psa_status_t psa_verify_hash_builtin(const psa_key_attributes_t *attributes,
                                     const uint8_t *key_buffer,
                                     size_t key_buffer_size,
                                     psa_algorithm_t alg,
                                     const uint8_t *hash,
                                     size_t hash_length,
                                     const uint8_t *signature,
                                     size_t signature_length)
{

    return (PSA_ERROR_NOT_SUPPORTED);
}

psa_status_t psa_verify_hash(psa_key_id_t key,
                             psa_algorithm_t alg,
                             const uint8_t *hash,
                             size_t hash_length,
                             const uint8_t *signature,
                             size_t signature_length)
{
/* No S/NS support for ECDSA and EDDSA */
#if TFM_ENABLED == 1
    return PSA_ERROR_NOT_SUPPORTED;
#else
    psa_status_t status = PSA_ERROR_CORRUPTION_DETECTED;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    uint8_t *keyMaterial;

    CryptoKey myPublicKey;

    /* 0 to rep hash */
    status = psa_sign_verify_check_alg(0, alg);
    if (status != PSA_SUCCESS)
        return status;

    status = psa_get_key_attributes(key, &attributes);
    if (status != PSA_SUCCESS)
        goto exit;

    status = psa_key_attributes_usage_check(&attributes, PSA_KEY_USAGE_VERIFY_HASH, alg);
    if (status != PSA_SUCCESS)
        goto exit;

    /* use key_type to map to curve type */
    psa_key_type_t keyType = psa_get_key_type(&attributes);
    size_t key_bits = psa_get_key_bits(&attributes);

    /* only accept PSA_KEY_TYPE_ECC_KEY_PAIR keys here */
    if (!PSA_KEY_TYPE_ECC_PUBLIC_KEY(keyType))
    {
        status = PSA_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    /* only support ECDSA */
    if (!PSA_ALG_IS_ECDSA(alg))
    {
        status = PSA_ERROR_NOT_SUPPORTED;
        goto exit;
    }

    /* no support for deterministic ecdsa */
    if (PSA_ALG_IS_DETERMINISTIC_ECDSA(alg))
    {
        status = PSA_ERROR_NOT_SUPPORTED;
        goto exit;
    }

    /* Currently, the key bits attribute associated with public keys is an attribute of the curve
     * and not the acutal key itself. If this changes, then curveBits would need to be key_bits / 2 for
     * all types of public keys apart from montomery.
     */
    size_t curveBits = key_bits;

    size_t expectedOutputSize = PSA_SIGN_OUTPUT_SIZE(keyType, curveBits, alg);

    if (signature_length != expectedOutputSize)
    {
        status = PSA_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    psa_ecc_family_t curve = PSA_KEY_TYPE_ECC_GET_FAMILY(keyType);
    size_t publicKeyBytes = (curve == PSA_ECC_FAMILY_MONTGOMERY || curve == PSA_ECC_FAMILY_TWISTED_EDWARDS)
                                ? PSA_BITS_TO_BYTES(key_bits)
                                : PSA_KEY_EXPORT_ECC_PUBLIC_KEY_MAX_SIZE(key_bits);
    keyMaterial = malloc(publicKeyBytes);
    psa_export_key(key, keyMaterial, publicKeyBytes, &publicKeyBytes);
    CryptoKeyPlaintext_initKey(&myPublicKey, keyMaterial, publicKeyBytes);

    ECDSA_OperationVerify operationVerify;
    ECDSA_OperationVerify_init(&operationVerify);
    /* Short Weierstrass curves */

    operationVerify.curve = map_keyTypeToECCParams(keyType, key_bits);

    if (operationVerify.curve == NULL)
    {
        status = PSA_ERROR_NOT_SUPPORTED;
        goto exit;
    }

    operationVerify.theirPublicKey = &myPublicKey;
    operationVerify.hash = hash;
    operationVerify.r = signature;
    operationVerify.s = signature + signature_length / 2;

    /* generates the signature */
    status = ECDSA_verify(ECDSA_Hand, &operationVerify);
    if (status != 0)
    {
        status = map_ECDSAError_to_PSA_error(status);
    }

exit:

    return status;
#endif
}

psa_status_t psa_asymmetric_encrypt(psa_key_id_t key,
                                    psa_algorithm_t alg,
                                    const uint8_t *input,
                                    size_t input_length,
                                    const uint8_t *salt,
                                    size_t salt_length,
                                    uint8_t *output,
                                    size_t output_size,
                                    size_t *output_length)
{
    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_asymmetric_decrypt(psa_key_id_t key,
                                    psa_algorithm_t alg,
                                    const uint8_t *input,
                                    size_t input_length,
                                    const uint8_t *salt,
                                    size_t salt_length,
                                    uint8_t *output,
                                    size_t output_size,
                                    size_t *output_length)
{
    return PSA_ERROR_NOT_SUPPORTED;
}

/****************************************************************/
/* Symmetric cryptography */
/****************************************************************/

/* puts the operation in an error state, resets the key attributes if necessary,
 * and spits back the error code to return to the application */
static psa_status_t psa_cipher_error(psa_cipher_operation_t *operation,
                                     psa_status_t error,
                                     psa_key_attributes_t *attributes)
{
    operation->in_error_state = 1;
    if (attributes != NULL)
        psa_reset_key_attributes(attributes);
    return error;
}

/* added */
psa_status_t psa_cipher_encrypt(psa_key_id_t key,
                                psa_algorithm_t alg,
                                const uint8_t *input,
                                size_t input_length,
                                uint8_t *output,
                                size_t output_size,
                                size_t *output_length)
{

    psa_status_t status             = PSA_ERROR_CORRUPTION_DETECTED;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_type_t symKeyType;
    size_t blockLength;
    size_t key_bits;
    size_t ivSize;
    size_t outputSize;
    CryptoKey cryptoKey;
#if defined(PSA_DEBUG)
    uint8_t tempArray[16] = {0};
    uint8_t *iv_array     = tempArray;
#else
    uint8_t iv_array[16] = {0};
#endif

    *output_length                  = 0;

    /* The requested algorithm must be one that can be processed by cipher. */
    if (!PSA_ALG_IS_CIPHER(alg))
    {
        status = PSA_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    /* Get key attributes from key id*/
    status = psa_get_key_attributes(key, &attributes);
    if (status != PSA_SUCCESS)
    {
        goto exit;
    }

    status = psa_key_attributes_usage_check(&attributes, PSA_KEY_USAGE_ENCRYPT, alg);
    if (status != PSA_SUCCESS)
    {
        goto exit;
    }

    symKeyType  = psa_get_key_type(&attributes);
    blockLength = PSA_BLOCK_CIPHER_BLOCK_LENGTH(symKeyType);
    if (input_length < blockLength)
    {
        /* if no padding is required, then input must be at least blockLength */
        if (alg == PSA_ALG_ECB_NO_PADDING || alg == PSA_ALG_CBC_NO_PADDING)
        {
            status = PSA_ERROR_INVALID_ARGUMENT;
            goto exit;
        }
    }

    /* iv length */
    ivSize     = PSA_CIPHER_IV_LENGTH(symKeyType, alg);
    /* this macro accounts for the iv as well  */
    outputSize = PSA_CIPHER_ENCRYPT_OUTPUT_SIZE(symKeyType, alg, input_length);
    /* make sure output_size is big enough to take in the (iv if there is one and) ciphertext */
    if (output_size < outputSize)
    {
        status = PSA_ERROR_BUFFER_TOO_SMALL;
        goto exit;
    }

    key_bits = psa_get_key_bits(&attributes);
    KeyStore_PSA_initKey(&cryptoKey, toKeyStoreKeyID(key), PSA_BITS_TO_BYTES(key_bits), NULL);

    if (alg == PSA_ALG_ECB_NO_PADDING)
    {

        AESECB_Operation operation;
        AESECB_Operation_init(&operation);
        operation.key         = &cryptoKey;
        operation.input       = (uint8_t *)input;
        operation.inputLength = input_length;
        operation.output      = output;
        status                = AESECB_oneStepEncrypt(AESECB_Hand, &operation);
        if (status != 0)
        {
            status = map_AESError_to_PSA_error(status);
        }
    }
    /* generate random iv for the below ops and pass that back in the output according to description on page 142 */
    else if (alg == PSA_ALG_CTR)
    {
        AESCTR_Operation operation;
        AESCTR_Operation_init(&operation);
        operation.key         = &cryptoKey;
        operation.input       = input;
        operation.inputLength = input_length;
        /* page 141 psa_cipher_encrypt iv is followed by ciphertext */
        operation.output      = output + ivSize;
        /* must use TRNG to get an inital counter */

#if defined(PSA_DEBUG)
        status = psa_debug_get_iv(&iv_array);
        if (status != PSA_SUCCESS)
        {
            status = psa_generate_random(iv_array, ivSize);
            if (status != 0)
            {
                goto exit;
            }
        }
#else
        status = psa_generate_random(iv_array, ivSize);
        if (status != PSA_SUCCESS)
        {
            goto exit;
        }
#endif

        operation.initialCounter = iv_array;
        status                   = AESCTR_oneStepEncrypt(AESCTR_Hand, &operation);
        if (status != AESCTR_STATUS_SUCCESS)
        {
            status = map_AESError_to_PSA_error(status);
            goto exit;
        }
        memcpy(output, iv_array, ivSize);
    }

    else if (alg == PSA_ALG_CBC_NO_PADDING)
    {
        AESCBC_Operation operation;
        AESCBC_Operation_init(&operation);
        operation.key         = &cryptoKey;
        operation.input       = (uint8_t *)input;
        operation.inputLength = input_length;
        /* page 141 psa_cipher_encrypt iv is followed by ciphertext */
        operation.output      = output + ivSize;

#if defined(PSA_DEBUG)
        status = psa_debug_get_iv(&iv_array);
        if (status != PSA_SUCCESS)
        {
            status = psa_generate_random(iv_array, ivSize);
            if (status != 0)
            {
                goto exit;
            }
        }
#else
        status = psa_generate_random(iv_array, ivSize);
        if (status != 0)
        {
            goto exit;
        }
#endif

        operation.iv = iv_array;
        status       = AESCBC_oneStepEncrypt(AESCBC_Hand, &operation);
        if (status != AESCBC_STATUS_SUCCESS)
        {
            status = map_AESError_to_PSA_error(status);
            goto exit;
        }
        /* write the iv to the correct place */
        memcpy(output, iv_array, ivSize);
    }

    else
    {
        status = PSA_ERROR_NOT_SUPPORTED;
    }

exit:
    if (status == PSA_SUCCESS)
    {

        *output_length = outputSize;
    }
    psa_reset_key_attributes(&attributes);
    return status;
}

psa_status_t psa_cipher_decrypt(psa_key_id_t key,
                                psa_algorithm_t alg,
                                const uint8_t *input,
                                size_t input_length,
                                uint8_t *output,
                                size_t output_size,
                                size_t *output_length)
{

    psa_status_t status             = PSA_ERROR_CORRUPTION_DETECTED;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_type_t symKeyType;
    /* iv length (note ivSize = 0) for AESECB, code can be generic.*/
    size_t ivSize;
    /* just need space for the plain text */
    size_t payloadSize;
    size_t blockLength;
    size_t key_bits;

    *output_length                  = 0;

    CryptoKey cryptoKey;

    /* The requested algorithm must be one that can be processed by cipher. */
    if (!PSA_ALG_IS_CIPHER(alg))
    {
        status = PSA_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    /* Get ket attributes from key id */
    status = psa_get_key_attributes(key, &attributes);
    if (status != PSA_SUCCESS)
    {
        goto exit;
    }

    status = psa_key_attributes_usage_check(&attributes, PSA_KEY_USAGE_DECRYPT, alg);
    if (status != PSA_SUCCESS)
    {
        goto exit;
    }

    symKeyType  = psa_get_key_type(&attributes);
    ivSize      = PSA_CIPHER_IV_LENGTH(symKeyType, alg);
    payloadSize = PSA_CIPHER_DECRYPT_OUTPUT_SIZE(symKeyType, alg, input_length - ivSize);

    /* this is needed to check that there is an iv the input since if there is none, the payloadSize will be a large
     * unreasonable number due to previous opeartion*/
    if (payloadSize > PSA_CIPHER_DECRYPT_OUTPUT_MAX_SIZE(input_length))
    {
        status = PSA_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    if (output_size < payloadSize)
    {
        status = PSA_ERROR_BUFFER_TOO_SMALL;
        goto exit;
    }

    blockLength = PSA_BLOCK_CIPHER_BLOCK_LENGTH(symKeyType);
    /* if payload size is smaller than the actual block length, then there is a problem if there is no padding*/
    if (payloadSize < blockLength)
    {
        /* if no padding is required, then input must be at least blockLength */
        if (alg == PSA_ALG_ECB_NO_PADDING || alg == PSA_ALG_CBC_NO_PADDING)
        {
            status = PSA_ERROR_INVALID_ARGUMENT;
            goto exit;
        }
    }

    key_bits = psa_get_key_bits(&attributes);
    KeyStore_PSA_initKey(&cryptoKey, toKeyStoreKeyID(key), PSA_BITS_TO_BYTES(key_bits), NULL);

    if (alg == PSA_ALG_ECB_NO_PADDING)
    {

        AESECB_Operation operation;
        AESECB_Operation_init(&operation);
        operation.key         = &cryptoKey;
        operation.input       = (uint8_t *)(input + ivSize);
        operation.inputLength = input_length - ivSize;
        operation.output      = output;
        status                = AESECB_oneStepDecrypt(AESECB_Hand, &operation);
        if (status != 0)
        {
            status = PSA_ERROR_HARDWARE_FAILURE;
        }
    }

    else if (alg == PSA_ALG_CTR)
    {

        AESCTR_Operation operation;
        AESCTR_Operation_init(&operation);
        operation.key            = &cryptoKey;
        operation.input          = input + ivSize;
        operation.inputLength    = input_length - ivSize;
        operation.output         = output;
        operation.initialCounter = input;
        status                   = AESCTR_oneStepDecrypt(AESCTR_Hand, &operation);
        if (status != 0)
        {
            status = PSA_ERROR_HARDWARE_FAILURE;
        }
    }

    else if (alg == PSA_ALG_CBC_NO_PADDING)
    {

        AESCBC_Operation operation;
        AESCBC_Operation_init(&operation);
        operation.key         = &cryptoKey;
        operation.input       = (uint8_t *)(input + ivSize);
        operation.inputLength = input_length - ivSize;
        operation.output      = output;
        operation.iv          = (uint8_t *)input;
        status                = AESCBC_oneStepDecrypt(AESCBC_Hand, &operation);
        if (status != 0)
        {
            // todo: change
            status = PSA_ERROR_HARDWARE_FAILURE;
        }
    }

    else
    {
        status = PSA_ERROR_NOT_SUPPORTED;
    }

exit:
    if (status == PSA_SUCCESS)
    {
        *output_length = payloadSize;
    }

    psa_reset_key_attributes(&attributes);
    return status;
}

static psa_status_t psa_cipher_setup(psa_cipher_operation_t *operation,
                                     psa_key_id_t key,
                                     psa_algorithm_t alg,
                                     aes_function_t cipher_operation)
{
    psa_status_t status             = PSA_ERROR_CORRUPTION_DETECTED;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_usage_t usage           = (cipher_operation == AES_ENC ? PSA_KEY_USAGE_ENCRYPT : PSA_KEY_USAGE_DECRYPT);

    /* A context must be freshly initialized before it can be set up. */
    if (operation->id != 0 || operation->in_error_state)
        return psa_cipher_error(operation, PSA_ERROR_BAD_STATE, NULL);

    /* The requested algorithm must be one that can be processed by cipher. */
    if (!PSA_ALG_IS_CIPHER(alg))
        return psa_cipher_error(operation, PSA_ERROR_NOT_SUPPORTED, NULL);

    /* Get key attributes from key id */
    status = psa_get_key_attributes(key, &attributes);
    if (status != PSA_SUCCESS)
        return psa_cipher_error(operation, status, &attributes);

    status = psa_key_attributes_usage_check(&attributes, usage, alg);
    if (status != PSA_SUCCESS)
        return psa_cipher_error(operation, status, &attributes);

    /* Added to validate the key type since PSA_KEY_TYPE_RAW_DATA is not allowed and anything other than
     * PSA_KEY_TYPE_AES is invalid.  */
    psa_key_type_t keyType = psa_get_key_type(&attributes);
    if (keyType != PSA_KEY_TYPE_AES)
        return psa_cipher_error(operation, PSA_ERROR_INVALID_ARGUMENT, &attributes);

    /* Initialize the operation struct members, except for id. The id member
     * is used to indicate to psa_cipher_abort that there are resources to free,
     * so we only set it (in the driver wrapper) after resources have been
     * allocated/initialized. */
    operation->iv_set = 0;
    if (alg == PSA_ALG_ECB_NO_PADDING)
        operation->iv_required = 0;
    else
        operation->iv_required = 1;

    operation->default_iv_length = PSA_CIPHER_IV_LENGTH(keyType, alg);

    operation->unprocessed_len = 0;

    /* should always be AES_BLOCK_LENGTH */
    operation->block_length = PSA_BLOCK_CIPHER_BLOCK_LENGTH(keyType);

    size_t key_bits = psa_get_key_bits(&attributes);
    KeyStore_PSA_initKey(&operation->cryptoKey, toKeyStoreKeyID(key), PSA_BITS_TO_BYTES(key_bits), NULL);

    /* Try doing the operation through a driver before using software fallback. */
    if (cipher_operation == AES_ENC)
    {
        switch (alg)
        {
            case PSA_ALG_ECB_NO_PADDING:
                status = AESECB_setupEncrypt(AESECB_Hand, &operation->cryptoKey);
                if (status != AESECB_STATUS_SUCCESS)
                {
                    status = map_AESError_to_PSA_error(status);
                    return psa_cipher_error(operation, status, &attributes);
                }
                break;
            case PSA_ALG_CBC_NO_PADDING:
                status = AESCBC_setupEncrypt(AESCBC_Hand, &operation->cryptoKey);
                if (status != AESCBC_STATUS_SUCCESS)
                {
                    status = map_AESError_to_PSA_error(status);
                    return psa_cipher_error(operation, status, &attributes);
                }
                break;
            case PSA_ALG_CTR:
                status = AESCTR_setupEncrypt(AESCTR_Hand, &operation->cryptoKey, NULL);
                if (status != AESCTR_STATUS_SUCCESS)
                {
                    status = map_AESError_to_PSA_error(status);
                    return psa_cipher_error(operation, status, &attributes);
                }
                break;
            default:
                return psa_cipher_error(operation, PSA_ERROR_NOT_SUPPORTED, &attributes);
        }
    }
    else
    {
        switch (alg)
        {
            case PSA_ALG_ECB_NO_PADDING:
                status = AESECB_setupDecrypt(AESECB_Hand, &operation->cryptoKey);
                if (status != AESECB_STATUS_SUCCESS)
                {
                    status = map_AESError_to_PSA_error(status);
                    return psa_cipher_error(operation, status, &attributes);
                }
                break;
            case PSA_ALG_CBC_NO_PADDING:
                status = AESCBC_setupDecrypt(AESCBC_Hand, &operation->cryptoKey);
                if (status != AESCBC_STATUS_SUCCESS)
                {
                    status = map_AESError_to_PSA_error(status);
                    return psa_cipher_error(operation, status, &attributes);
                }

                break;
            case PSA_ALG_CTR:
                status = AESCTR_setupDecrypt(AESCTR_Hand, &operation->cryptoKey, NULL);
                if (status != AESCTR_STATUS_SUCCESS)
                {
                    status = map_AESError_to_PSA_error(status);
                    return psa_cipher_error(operation, status, &attributes);
                }
                break;
            default:
                return psa_cipher_error(operation, PSA_ERROR_NOT_SUPPORTED, &attributes);
        }
    }

    operation->alg = alg;
    operation->id  = PSA_CRYPTO_TI_DRIVER;

    psa_reset_key_attributes(&attributes);

    return PSA_SUCCESS;
}

psa_status_t psa_cipher_encrypt_setup(psa_cipher_operation_t *operation, psa_key_id_t key, psa_algorithm_t alg)
{
    operation->is_encrypt = 1;
    return (psa_cipher_setup(operation, key, alg, AES_ENC));
}

psa_status_t psa_cipher_decrypt_setup(psa_cipher_operation_t *operation, psa_key_id_t key, psa_algorithm_t alg)
{
    operation->is_encrypt = 0;
    return (psa_cipher_setup(operation, key, alg, AES_DEC));
}

psa_status_t psa_cipher_generate_iv(psa_cipher_operation_t *operation, uint8_t *iv, size_t iv_size, size_t *iv_length)
{
    psa_status_t status = PSA_ERROR_CORRUPTION_DETECTED;

    *iv_length = 0;

    if (operation->id == 0 || operation->in_error_state)
        return psa_cipher_error(operation, PSA_ERROR_BAD_STATE, NULL);

    if (operation->iv_set || !operation->iv_required)
        return psa_cipher_error(operation, PSA_ERROR_BAD_STATE, NULL);

    if (iv_size < operation->default_iv_length)
        return psa_cipher_error(operation, PSA_ERROR_BUFFER_TOO_SMALL, NULL);

    status = psa_generate_random(iv, operation->default_iv_length);
    if (status != PSA_SUCCESS)
        return psa_cipher_error(operation, status, NULL);

    status = psa_cipher_set_iv(operation, iv, operation->default_iv_length);
    if (status != PSA_SUCCESS)
        return psa_cipher_error(operation, status, NULL);

    operation->iv_set = 1;
    *iv_length        = operation->default_iv_length;

    return PSA_SUCCESS;
}

psa_status_t psa_cipher_set_iv(psa_cipher_operation_t *operation, const uint8_t *iv, size_t iv_length)
{
    /* PSA_SUCCESS as initial status needed */
    psa_status_t status = PSA_SUCCESS;

    if (operation->id == 0 || operation->in_error_state)
        return psa_cipher_error(operation, PSA_ERROR_BAD_STATE, NULL);

    if (operation->iv_set || !operation->iv_required)
        return psa_cipher_error(operation, PSA_ERROR_BAD_STATE, NULL);

    if (iv_length > PSA_CIPHER_IV_MAX_SIZE)
        return psa_cipher_error(operation, PSA_ERROR_INVALID_ARGUMENT, NULL);

    if (iv_length < operation->default_iv_length)
        return psa_cipher_error(operation, PSA_ERROR_INVALID_ARGUMENT, NULL);

    switch (operation->alg)
    {
        case PSA_ALG_CBC_NO_PADDING:
            {
                status = AESCBC_setIV(AESCBC_Hand, iv, iv_length);
                break;
            }
        case PSA_ALG_CTR:
            {
                /* AESCTR driver does not have a setIV function, but the initial counter can be set
                 * with setupEncrypt */
                AESCTR_cancelOperation(AESCTR_Hand);
                if (operation->is_encrypt)
                {
                    status = AESCTR_setupEncrypt(AESCTR_Hand, &operation->cryptoKey, iv);
                }
                else
                {
                    status = AESCTR_setupDecrypt(AESCTR_Hand, &operation->cryptoKey, iv);
                }
                break;
            }
        default:
            return psa_cipher_error(operation, PSA_ERROR_NOT_SUPPORTED, NULL);
    }

    if (status != 0)
    {
        status = map_AESError_to_PSA_error(status);
        return psa_cipher_error(operation, status, NULL);
    }

    operation->iv_set = 1;
    return PSA_SUCCESS;
}

/*
 * psa_aesAddSingleBlock, psa_aesAddDataNoPassing
 * Helper functions for psa_cipher_update.
 */

static psa_status_t psa_aesAddBlock(psa_algorithm_t alg,
                                    uint8_t numBlocks,
                                    const uint8_t *input,
                                    uint8_t *output,
                                    CryptoKey *cryptoKey)
{

    psa_status_t status = PSA_SUCCESS;

    switch (alg)
    {
        case PSA_ALG_ECB_NO_PADDING:
            {
                AESECB_Operation Operation;
                AESECB_Operation_init(&Operation);
                Operation.input       = (uint8_t *)input;
                Operation.key         = cryptoKey;
                Operation.inputLength = PSA_AES_BLOCK_SIZE * numBlocks;
                Operation.output      = output;
                status                = AESECB_addData(AESECB_Hand, &Operation);
                break;
            }
        case PSA_ALG_CTR:
            {
                AESCTR_SegmentedOperation Operation;
                AESCTR_SegmentedOperation_init(&Operation);
                Operation.input       = (uint8_t *)input;
                Operation.inputLength = PSA_AES_BLOCK_SIZE * numBlocks;
                Operation.output      = output;
                status                = AESCTR_addData(AESCTR_Hand, &Operation);
                break;
            }
        case PSA_ALG_CBC_NO_PADDING:
            {
                AESCBC_SegmentedOperation Operation;
                AESCBC_SegmentedOperation_init(&Operation);
                Operation.input       = (uint8_t *)input;
                Operation.inputLength = PSA_AES_BLOCK_SIZE * numBlocks;
                Operation.output      = output;
                status                = AESCBC_addData(AESCBC_Hand, &Operation);
                break;
            }
        default:
            status = PSA_ERROR_NOT_SUPPORTED;
    }

    if (status != PSA_SUCCESS)
    {
        status = map_AESError_to_PSA_error(status);
    }

    return status;
}

static psa_status_t psa_aesAddDataNoPadding(psa_cipher_operation_t *operation,
                                            const uint8_t *input,
                                            size_t input_length,
                                            uint8_t *output,
                                            size_t output_size)
{

    psa_status_t status = PSA_SUCCESS;

    /* first check if there is enough input data to process */
    if (input_length + operation->unprocessed_len < PSA_AES_BLOCK_SIZE)
    {
        /* buffer for next time */
        memcpy((operation->unprocessed_data + operation->unprocessed_len), input, input_length);
        operation->unprocessed_len += input_length;
        return PSA_SUCCESS;
    }

    /* If there is data to process, then first process buffered data */
    if (operation->unprocessed_len != 0)
    {

        /* figure out how much more is needed to get to a block size */
        size_t copy_len = PSA_AES_BLOCK_SIZE - operation->unprocessed_len;
        memcpy((operation->unprocessed_data + operation->unprocessed_len), input, copy_len);

        status = psa_aesAddBlock(operation->alg, 1, operation->unprocessed_data, output, &operation->cryptoKey);

        /* update outputs */
        output += PSA_AES_BLOCK_SIZE;
        /* update inputs */
        input += copy_len;
        input_length -= copy_len;
        /* reset to show buffer is empty */
        operation->unprocessed_len = 0;
    }

    /* process all the remaining blocks of data */
    if (status == PSA_SUCCESS && input_length >= PSA_AES_BLOCK_SIZE)
    {

        size_t remainingFullBlocks = (input_length / PSA_AES_BLOCK_SIZE);
        size_t dataToAdd           = remainingFullBlocks * PSA_AES_BLOCK_SIZE;

        status = psa_aesAddBlock(operation->alg, remainingFullBlocks, input, output, &operation->cryptoKey);
        /* update inputs */
        input += dataToAdd;
        input_length -= dataToAdd;
    }

    /* check if there is any data left to be buffered (data left should always be < PSA_AES_BLOCK_SIZE)*/
    if (status == PSA_SUCCESS && input_length != 0)
    {
        memcpy(operation->unprocessed_data, input, input_length);
        operation->unprocessed_len = input_length;
    }

    return status;
}

psa_status_t psa_cipher_update(psa_cipher_operation_t *operation,
                               const uint8_t *input,
                               size_t input_length,
                               uint8_t *output,
                               size_t output_size,
                               size_t *output_length)
{
    psa_status_t status = PSA_ERROR_CORRUPTION_DETECTED;
    *output_length      = 0;

    if (operation->id == 0 || operation->in_error_state)
        return psa_cipher_error(operation, PSA_ERROR_BAD_STATE, NULL);

    if (operation->iv_required && !operation->iv_set)
        return psa_cipher_error(operation, PSA_ERROR_BAD_STATE, NULL);

    if (operation->alg == 0)
        return psa_cipher_error(operation, PSA_ERROR_BAD_STATE, NULL);

    /* The requested algorithm must be one that can be processed by cipher. */
    if (!PSA_ALG_IS_CIPHER(operation->alg))
        return psa_cipher_error(operation, PSA_ERROR_INVALID_ARGUMENT, NULL);

    if (input_length == 0)
    {
        return PSA_SUCCESS;
    }

    /* true for ciphers supported (only AES based ciphers - this includes CTR) */
    *output_length = ((int)(operation->unprocessed_len + input_length) / (operation->block_length)) *
                     operation->block_length;

    if (output_size < *output_length)
        return psa_cipher_error(operation, PSA_ERROR_BUFFER_TOO_SMALL, NULL);

    switch (operation->alg)
    {

            /*Note no padding means if the input is not BLOCK LENGTH,
             * then the input data needs to be buffered and processed when the data buffered size  = BLOCK LENGTH*/

        case PSA_ALG_ECB_NO_PADDING:
        case PSA_ALG_CTR:
        case PSA_ALG_CBC_NO_PADDING:
            status = psa_aesAddDataNoPadding(operation, input, input_length, output, output_size);
            break;
        default:
            status = PSA_ERROR_NOT_SUPPORTED;
    }

    if (status != PSA_SUCCESS)
        return psa_cipher_error(operation, status, NULL);

    return PSA_SUCCESS;
}

psa_status_t psa_cipher_finish(psa_cipher_operation_t *operation,
                               uint8_t *output,
                               size_t output_size,
                               size_t *output_length)
{
    psa_status_t status = PSA_ERROR_GENERIC_ERROR;

    if (operation->id == 0 || operation->in_error_state)
        return psa_cipher_error(operation, PSA_ERROR_BAD_STATE, NULL);

    if (operation->iv_required && !operation->iv_set)
        return psa_cipher_error(operation, PSA_ERROR_BAD_STATE, NULL);

    /* The requested algorithm must be one that can be processed by cipher. */
    if (!PSA_ALG_IS_CIPHER(operation->alg))
        return psa_cipher_error(operation, PSA_ERROR_INVALID_ARGUMENT, NULL);

    /* true for ciphers supported/tested */
    *output_length = PSA_CIPHER_FINISH_OUTPUT_SIZE(PSA_KEY_TYPE_AES, operation->alg);
    if (output_size < *output_length)
        return psa_cipher_error(operation, PSA_ERROR_BUFFER_TOO_SMALL, NULL);

    if (operation->unprocessed_len != 0)
    {
        if (operation->alg == PSA_ALG_ECB_NO_PADDING || operation->alg == PSA_ALG_CBC_NO_PADDING)
            return psa_cipher_error(operation, PSA_ERROR_BAD_STATE, NULL);

        else if (output_size < operation->unprocessed_len)
            return psa_cipher_error(operation, PSA_ERROR_BUFFER_TOO_SMALL, NULL);
    }

    switch (operation->alg)
    {

        case PSA_ALG_ECB_NO_PADDING:
            {
                AESECB_Operation Operation;
                Operation.output      = output;
                Operation.key         = &operation->cryptoKey;
                Operation.inputLength = 0;

                status = AESECB_finalize(AESECB_Hand, &Operation);
                if (status != AESECB_STATUS_SUCCESS)
                {
                    status = map_AESError_to_PSA_error(status);
                }
                break;
            }
        case PSA_ALG_CTR:
            {
                /* CTR input length does not need to be a multiple of the block size, so there
                may be more input data left to process */
                AESCTR_SegmentedOperation Operation;
                Operation.output      = output;
                Operation.input       = operation->unprocessed_data;
                Operation.inputLength = operation->unprocessed_len;
                /* make sure output size is enough */
                status                = AESCTR_finalize(AESCTR_Hand, &Operation);
                if (status != AESCTR_STATUS_SUCCESS)
                {
                    status = map_AESError_to_PSA_error(status);
                }
                *output_length += operation->unprocessed_len;
                break;
            }
        case PSA_ALG_CBC_NO_PADDING:
            {
                AESCBC_SegmentedOperation Operation;
                AESCBC_SegmentedOperation_init(&Operation);
                Operation.inputLength = 0;
                Operation.output      = output;

                status = AESCBC_finalize(AESCBC_Hand, &Operation);
                if (status != AESCBC_STATUS_SUCCESS)
                {
                    status = map_AESError_to_PSA_error(status);
                }
                break;
            }

        default:
            status = PSA_ERROR_NOT_SUPPORTED;
    }

/* On tz_enabled, there is an issue with AESCBC_finalize returning an error code
 * even when the operation completed successfully. In those cases, abort the operation
 * and return psa_success. */
#if TFM_ENABLED == 1
    if (operation->alg == PSA_ALG_CBC_NO_PADDING && status == PSA_ERROR_GENERIC_ERROR)
    {
        psa_cipher_abort(operation);
        status = PSA_SUCCESS;
    }
#endif

    if (status != PSA_SUCCESS)
    {
        *output_length = 0;
        return psa_cipher_error(operation, status, NULL);
    }
    return PSA_SUCCESS;
}

psa_status_t psa_cipher_abort(psa_cipher_operation_t *operation)
{
    psa_status_t status = PSA_SUCCESS;
    if (operation->id == 0)
    {
        /* The object has (apparently) been initialized but it is not (yet)
         * in use. It's ok to call abort on such an object, and there's
         * nothing to do. */
        return (PSA_SUCCESS);
    }

    switch (operation->alg)
    {
        case PSA_ALG_ECB_NO_PADDING:
            /* cancel operation does not work for AESECB, so close and reopen
             * the driver instance */
            AESECB_close(AESECB_Hand);

            AESECB_Params AESECB_params;
            AESECB_Params_init(&AESECB_params);
            AESECB_params.returnBehavior = AESECB_RETURN_BEHAVIOR_POLLING;

            AESECB_Hand = AESECB_open(0, &AESECB_params);
            break;

        case PSA_ALG_CTR:
            status = AESCTR_cancelOperation(AESCTR_Hand);
            if (status != AESCTR_STATUS_SUCCESS)
            {
                status = map_AESError_to_PSA_error(status);
            }

            break;

        case PSA_ALG_CBC_NO_PADDING:
            /* cancel operation does not work for AESCBC, so close and reopen
             * the driver instance */
            AESCBC_close(AESCBC_Hand);

            AESCBC_Params AESCBC_params;
            AESCBC_Params_init(&AESCBC_params);
            AESCBC_params.returnBehavior = AESCBC_RETURN_BEHAVIOR_POLLING;
            AESCBC_Hand                  = AESCBC_open(0, &AESCBC_params);
            break;

        default:
            status = PSA_ERROR_NOT_SUPPORTED;
    }

    *operation = PSA_CIPHER_OPERATION_INIT;

    if (status != PSA_SUCCESS)
        return psa_cipher_error(operation, status, NULL);

    return (PSA_SUCCESS);
}

/****************************************************************/
/* AEAD */
/****************************************************************/

/* added  */

/* puts the operation into an error state, resets key attributes if necessary,
    and spits back out the error code to be returned to the application */
static psa_status_t psa_aead_error(psa_aead_operation_t *operation,
                                   psa_status_t error,
                                   psa_key_attributes_t *attributes)
{
    operation->in_error_state = 1;
    if (attributes != NULL)
        psa_reset_key_attributes(attributes);
    return error;
}

static psa_status_t psa_aead_set_lengths_internal(psa_aead_operation_t *operation,
                                                  size_t ad_length,
                                                  size_t plaintext_length)
{
    psa_status_t status;
    switch (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(operation->alg))
    {
        case PSA_ALG_CCM:
            status = AESCCM_setLengths(AESCCM_Hand, ad_length, plaintext_length, operation->tagSize);
            break;
        case PSA_ALG_GCM:
            status = AESGCM_setLengths(AESGCM_Hand, ad_length, plaintext_length);
            break;
        default:
            return psa_aead_error(operation, PSA_ERROR_NOT_SUPPORTED, NULL);
    }

    if (status != PSA_SUCCESS)
        return psa_aead_error(operation, status, NULL);
    return PSA_SUCCESS;
}

psa_status_t psa_aead_set_lengths(psa_aead_operation_t *operation, size_t ad_length, size_t plaintext_length)
{

    if (operation->id == 0 || operation->iv_set || operation->in_error_state)
        return psa_aead_error(operation, PSA_ERROR_BAD_STATE, NULL);

    operation->length_set      = 1;
    operation->adLength        = ad_length;
    operation->plaintextLength = plaintext_length;

    return psa_aead_set_lengths_internal(operation, ad_length, plaintext_length);
}

/* multi-setp operations */
psa_status_t psa_aead_encrypt_setup(psa_aead_operation_t *operation, psa_key_id_t key, psa_algorithm_t alg)
{

    psa_status_t status             = PSA_SUCCESS;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

    if (operation->id != 0 || operation->in_error_state)
        return psa_aead_error(operation, PSA_ERROR_BAD_STATE, NULL);

    /* check if the algorithim is part of the AEAD (auth encryptions with associated data)*/
    if (!(PSA_ALG_IS_AEAD(alg)))
        return psa_aead_error(operation, PSA_ERROR_INVALID_ARGUMENT, NULL);

    /* Fetch key attributes using key id */
    status = psa_get_key_attributes(key, &attributes);
    if (status != PSA_SUCCESS)
        return psa_aead_error(operation, status, &attributes);

    /* check the key policies (if allowed to use it for the alg of interest ..etc) . */
    status = psa_key_attributes_usage_check(&attributes, PSA_KEY_USAGE_ENCRYPT, alg);
    if (status != PSA_SUCCESS)
        return psa_aead_error(operation, status, &attributes);

    /* call the hardware related operations */
    size_t key_bits = psa_get_key_bits(&attributes);
    size_t key_type = psa_get_key_type(&attributes);
    KeyStore_PSA_initKey(&operation->cryptoKey, toKeyStoreKeyID(key), PSA_BITS_TO_BYTES(key_bits), NULL);

    size_t tagLength = PSA_AEAD_TAG_LENGTH(key_type, key_bits, alg);

    /* true for supported AEAD */
    if (PSA_BLOCK_CIPHER_BLOCK_LENGTH(key_type) != PSA_AES_BLOCK_SIZE)
        return psa_aead_error(operation, PSA_ERROR_INVALID_ARGUMENT, &attributes);

    /* setup based on ALG */

    if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(alg) == PSA_ALG_CCM)
    {
        /* check for correct tag size
         * Test assumes tagLength=0 is invalid */
        if (tagLength != 4 && tagLength != 6 && tagLength != 8 && tagLength != 10 && tagLength != 12 &&
            tagLength != 14 && tagLength != 16)
            return psa_aead_error(operation, PSA_ERROR_INVALID_ARGUMENT, &attributes);

        /* the remaining parameters are not added until later using other PSA calls. The driver allows such
         * functionality.*/
        status = AESCCM_setupEncrypt(AESCCM_Hand, &operation->cryptoKey, 0, 0, tagLength);
        if (status != AESCCM_STATUS_SUCCESS)
            return psa_aead_error(operation, PSA_ERROR_HARDWARE_FAILURE, &attributes);
    }
    else if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(alg) == PSA_ALG_GCM)
    {
        /* check for correct tag size
         * Test assumes tagLength=0 is invalid */
        if (tagLength != 4 && tagLength != 8 && tagLength != 12 && tagLength != 13 && tagLength != 14 &&
            tagLength != 15 && tagLength != 16)
            return psa_aead_error(operation, PSA_ERROR_INVALID_ARGUMENT, &attributes);

        status = AESGCM_setupEncrypt(AESGCM_Hand, &operation->cryptoKey, 0, tagLength);
        if (status != AESGCM_STATUS_SUCCESS)
            return psa_aead_error(operation, PSA_ERROR_HARDWARE_FAILURE, &attributes);
    }
    else
    {
        return psa_aead_error(operation, PSA_ERROR_NOT_SUPPORTED, &attributes);
    }

    /* only set what is changed here */
    operation->alg     = alg;
    operation->key_set = 1;
    operation->tagSize = tagLength;
    operation->id      = PSA_CRYPTO_TI_DRIVER;

    psa_reset_key_attributes(&attributes);

    return PSA_SUCCESS;
}

psa_status_t psa_aead_decrypt_setup(psa_aead_operation_t *operation, psa_key_id_t key, psa_algorithm_t alg)
{

    psa_status_t status             = PSA_SUCCESS;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

    if (operation->id != 0 || operation->in_error_state)
        return psa_aead_error(operation, PSA_ERROR_BAD_STATE, NULL);

    /* check if the algorithim is part of the AEAD (auth encryptions with associated data)*/
    if (!(PSA_ALG_IS_AEAD(alg)))
        return psa_aead_error(operation, PSA_ERROR_INVALID_ARGUMENT, NULL);

    /* Fetch key attributes using key id 8? */
    status = psa_get_key_attributes(key, &attributes);
    if (status != PSA_SUCCESS)
        return psa_aead_error(operation, status, &attributes);

    /* check key policies (if allowed to use it for the alg of interest ..etc) . */
    status = psa_key_attributes_usage_check(&attributes, PSA_KEY_USAGE_DECRYPT, alg);
    if (status != PSA_SUCCESS)
        return psa_aead_error(operation, status, &attributes);

    /* call the hardware related operations */
    size_t key_bits = psa_get_key_bits(&attributes);
    size_t key_type = psa_get_key_type(&attributes);
    KeyStore_PSA_initKey(&operation->cryptoKey, toKeyStoreKeyID(key), PSA_BITS_TO_BYTES(key_bits), NULL);

    size_t tagLength = PSA_AEAD_TAG_LENGTH(key_type, key_bits, alg);

    /* true for supported AEAD */
    if (PSA_BLOCK_CIPHER_BLOCK_LENGTH(key_type) != PSA_AES_BLOCK_SIZE)
        return psa_aead_error(operation, PSA_ERROR_INVALID_ARGUMENT, &attributes);

    /* setup based on ALG */

    if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(alg) == PSA_ALG_CCM)
    {
        /* check for correct tag size
         * Test assumes tagLength=0 is invalid */
        if (tagLength != 4 && tagLength != 6 && tagLength != 8 && tagLength != 10 && tagLength != 12 &&
            tagLength != 14 && tagLength != 16)
            return psa_aead_error(operation, PSA_ERROR_INVALID_ARGUMENT, &attributes);
        /* the remaining parameters are not added until later using other PSA calls. The driver allows such
         * functionality.*/
        status = AESCCM_setupDecrypt(AESCCM_Hand, &operation->cryptoKey, 0, 0, tagLength);
        if (status != AESCCM_STATUS_SUCCESS)
            return psa_aead_error(operation, PSA_ERROR_HARDWARE_FAILURE, &attributes);
    }
    else if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(alg) == PSA_ALG_GCM)
    {
        /* check for correct tag size
         * Test assumes tagLength=0 is invalid */
        if (tagLength != 4 && tagLength != 8 && tagLength != 12 && tagLength != 13 && tagLength != 14 &&
            tagLength != 15 && tagLength != 16)
            return psa_aead_error(operation, PSA_ERROR_INVALID_ARGUMENT, &attributes);

        status = AESGCM_setupDecrypt(AESGCM_Hand, &operation->cryptoKey, 0, 0);
        if (status != AESGCM_STATUS_SUCCESS)
            return psa_aead_error(operation, PSA_ERROR_HARDWARE_FAILURE, &attributes);
    }
    else
    {
        return psa_aead_error(operation, PSA_ERROR_NOT_SUPPORTED, &attributes);
    }

    /* only set what is changed here */
    operation->alg     = alg;
    operation->key_set = 1;
    operation->tagSize = tagLength;
    operation->id      = PSA_CRYPTO_TI_DRIVER;

    psa_reset_key_attributes(&attributes);

    return PSA_SUCCESS;
}

/* uses the appropriate driver depending on the alg to update the additional data,
unless it is the final call to this function for the operation, input_length must be a multiple of the block size */
static psa_status_t psa_aead_update_ad_internal(psa_algorithm_t alg, const uint8_t *input, size_t input_length)
{
    psa_status_t status = PSA_SUCCESS;

    if (input_length == 0)
        return PSA_SUCCESS;

    if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(alg) == PSA_ALG_CCM)
    {
        AESCCM_SegmentedAADOperation segmentedAADOperation;
        AESCCM_SegmentedAADOperation_init(&segmentedAADOperation);
        segmentedAADOperation.aad       = (uint8_t *)input;
        segmentedAADOperation.aadLength = input_length;

        status = AESCCM_addAAD(AESCCM_Hand, &segmentedAADOperation);
        if (status != AESCCM_STATUS_SUCCESS)
            return PSA_ERROR_HARDWARE_FAILURE;
    }

    else if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(alg) == PSA_ALG_GCM)
    {
        AESGCM_SegmentedAADOperation segmentedAADOperation;
        AESGCM_SegmentedAADOperation_init(&segmentedAADOperation);
        segmentedAADOperation.aad       = (uint8_t *)input;
        segmentedAADOperation.aadLength = input_length;

        status = AESGCM_addAAD(AESGCM_Hand, &segmentedAADOperation);
        if (status != AESCCM_STATUS_SUCCESS)
            return PSA_ERROR_HARDWARE_FAILURE;
    }
    else
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }
    return PSA_SUCCESS;
}

/* uses the appropriate driver depending on the alg to pass additional data to the operation,
unless it is the final call to this function for the operation, input_length must be a multiple of the block size */
psa_status_t psa_aead_update_internal(psa_algorithm_t alg, const uint8_t *input, size_t input_length, uint8_t *output)
{

    psa_status_t status = PSA_SUCCESS;

    if (input_length == 0)
    {
        return PSA_SUCCESS;
    }

    if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(alg) == PSA_ALG_CCM)
    {
        AESCCM_SegmentedDataOperation segmentedDataOperation;
        AESCCM_SegmentedDataOperation_init(&segmentedDataOperation);

        segmentedDataOperation.input       = (uint8_t *)input;
        segmentedDataOperation.output      = output;
        segmentedDataOperation.inputLength = input_length;

        status = AESCCM_addData(AESCCM_Hand, &segmentedDataOperation);
        if (status != AESCCM_STATUS_SUCCESS)
        {
            return PSA_ERROR_HARDWARE_FAILURE;
        }
    }
    else if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(alg) == PSA_ALG_GCM)
    {
        AESGCM_SegmentedDataOperation segmentedDataOperation;
        AESGCM_SegmentedDataOperation_init(&segmentedDataOperation);

        segmentedDataOperation.input       = (uint8_t *)input;
        segmentedDataOperation.output      = output;
        segmentedDataOperation.inputLength = input_length;

        status = AESGCM_addData(AESGCM_Hand, &segmentedDataOperation);
        if (status != AESGCM_STATUS_SUCCESS)
        {
            return PSA_ERROR_HARDWARE_FAILURE;
        }
    }
    else
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    return PSA_SUCCESS;
}

psa_status_t psa_aead_update(psa_aead_operation_t *operation,
                             const uint8_t *input,
                             size_t input_length,
                             uint8_t *output,
                             size_t output_size,
                             size_t *output_length)
{
    psa_status_t status = PSA_SUCCESS;

    operation->runningPlaintextLength += input_length;

    if (!operation->done_updating_ad)
    {
        operation->done_updating_ad = 1;
        /* make sure than any remaining additional data is added */
        status = psa_aead_update_ad_internal(operation->alg, operation->unprocessedData, operation->unprocessed_len);
        if (status != PSA_SUCCESS)
            return psa_aead_error(operation, status, NULL);
        operation->unprocessed_len = 0;
    }

    *output_length = 0;

    if (!operation->iv_set || operation->id == 0 || operation->in_error_state)
        return psa_aead_error(operation, PSA_ERROR_BAD_STATE, NULL);

    if (operation->length_set && operation->runningPlaintextLength > operation->plaintextLength)
        return psa_aead_error(operation, PSA_ERROR_INVALID_ARGUMENT, NULL);

    if (operation->length_set && operation->runningADLength < operation->adLength)
        return psa_aead_error(operation, PSA_ERROR_INVALID_ARGUMENT, NULL);

    /* CCM requires lengths to be set */
    if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(operation->alg) == PSA_ALG_CCM && !operation->length_set)
        return psa_aead_error(operation, PSA_ERROR_BAD_STATE, NULL);

    /* for GCM, update the length so that the AESGCM driver dosent complain */
    if (!operation->length_set)
    {
        psa_aead_set_lengths_internal(operation, operation->runningADLength, operation->runningPlaintextLength);
        operation->length_set = 0;
    }

    /* verify that the output is large enough */
    /* true for ciphers supported (only AES based ciphers - this includes CTR) */
    *output_length = ((int)(operation->unprocessed_len + input_length) / (PSA_AES_BLOCK_SIZE)) * PSA_AES_BLOCK_SIZE;

    if (output_size < *output_length)
    {
        return (psa_aead_error(operation, PSA_ERROR_BUFFER_TOO_SMALL, NULL));
    }

    size_t totalUnprocessedLen = operation->unprocessed_len + input_length;
    /* not enough total data to make up one block */
    if (totalUnprocessedLen < PSA_AES_BLOCK_SIZE)
    {
        /* buffer the data and wait for more data to arrive */
        memcpy(operation->unprocessedData + operation->unprocessed_len, input, input_length);
        operation->unprocessed_len = totalUnprocessedLen;
        return PSA_SUCCESS;
    }

    /* enough data to make up at least a single block */
    /* empty the buffer by filling it up and adding its data */
    /* we need to do this before adding the remaining data */
    size_t amountToCopy = PSA_AES_BLOCK_SIZE - operation->unprocessed_len;
    memcpy(operation->unprocessedData + operation->unprocessed_len, input, amountToCopy);
    input += amountToCopy;
    status = psa_aead_update_internal(operation->alg, operation->unprocessedData, PSA_AES_BLOCK_SIZE, output);

    if (status != PSA_SUCCESS)
    {
        return (psa_aead_error(operation, status, NULL));
    }

    output += PSA_AES_BLOCK_SIZE;
    totalUnprocessedLen -= PSA_AES_BLOCK_SIZE;
    operation->unprocessed_len = 0;

    /* Get the largest multiple of the block size that is less than the number of bytes of unprocessed data */
    size_t amountToAdd = ((int)(totalUnprocessedLen / PSA_AES_BLOCK_SIZE)) * PSA_AES_BLOCK_SIZE;

    if (amountToAdd > 0)
    {
        status = psa_aead_update_internal(operation->alg, input, amountToAdd, output);
        if (status != PSA_SUCCESS)
        {
            return (psa_aead_error(operation, status, NULL));
        }

        totalUnprocessedLen -= amountToAdd;
        input += amountToAdd;
        output += amountToAdd;
    }

    /* add any left over data to the unprocessed data buffer */
    if (totalUnprocessedLen > 0)
    {
        memcpy(operation->unprocessedData, input, totalUnprocessedLen);
        operation->unprocessed_len = totalUnprocessedLen;
    }

    return PSA_SUCCESS;
}

psa_status_t psa_aead_update_ad(psa_aead_operation_t *operation, const uint8_t *input, size_t input_length)
{

    psa_status_t status = PSA_SUCCESS;

    operation->runningADLength += input_length;

    if (!operation->iv_set || operation->id == 0 || operation->done_updating_ad || operation->in_error_state)
        return psa_aead_error(operation, PSA_ERROR_BAD_STATE, NULL);

    if (operation->length_set && operation->runningADLength > operation->adLength)
        return psa_aead_error(operation, PSA_ERROR_INVALID_ARGUMENT, NULL);

    size_t totalUnprocessedLen = operation->unprocessed_len + input_length;

    /* CCM requires lengths to be set */
    if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(operation->alg) == PSA_ALG_CCM && !operation->length_set)
        return psa_aead_error(operation, PSA_ERROR_BAD_STATE, NULL);

    /* for GCM, update the length so that the AESGCM driver dosent complain */
    if (!operation->length_set)
    {
        psa_aead_set_lengths_internal(operation, operation->runningADLength, 0);
        operation->length_set = 0;
    }

    /* not enough total data to make up one block */
    if (totalUnprocessedLen < PSA_AES_BLOCK_SIZE)
    {
        /* buffer the data and wait for more data to arrive */
        memcpy(operation->unprocessedData + operation->unprocessed_len, input, input_length);
        operation->unprocessed_len = totalUnprocessedLen;
        return PSA_SUCCESS;
    }

    /* enough data to make up at least a single block */
    /* empty the buffer by filling it up and adding its data */
    /* we need to do this before adding the remaining data */
    size_t amountToCopy = PSA_AES_BLOCK_SIZE - operation->unprocessed_len;
    memcpy(operation->unprocessedData + operation->unprocessed_len, input, amountToCopy);
    input += amountToCopy;

    status = psa_aead_update_ad_internal(operation->alg, operation->unprocessedData, PSA_AES_BLOCK_SIZE);
    if (status != PSA_SUCCESS)
        return psa_aead_error(operation, status, NULL);

    totalUnprocessedLen -= PSA_AES_BLOCK_SIZE;
    operation->unprocessed_len = 0;

    /* Get the largest multiple of the block size that is less than the number of bytes of unprocessed data */
    size_t amountToAdd = ((int)(totalUnprocessedLen / PSA_AES_BLOCK_SIZE)) * PSA_AES_BLOCK_SIZE;

    if (amountToAdd > 0)
    {
        status = psa_aead_update_ad_internal(operation->alg, input, amountToAdd);
        if (status != PSA_SUCCESS)
            return psa_aead_error(operation, status, NULL);
        totalUnprocessedLen -= amountToAdd;
        input += amountToAdd;
    }

    /* add any left over data to the unprocessed data buffer */
    if (totalUnprocessedLen > 0)
    {
        memcpy(operation->unprocessedData, input, totalUnprocessedLen);
        operation->unprocessed_len = totalUnprocessedLen;
    }

    return PSA_SUCCESS;
}

psa_status_t psa_aead_set_nonce(psa_aead_operation_t *operation, const uint8_t *nonce, size_t nonce_length)
{

    psa_status_t status = PSA_SUCCESS;

    if (operation->id == 0 || operation->iv_set || operation->in_error_state)
        return psa_aead_error(operation, PSA_ERROR_BAD_STATE, NULL);

    if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(operation->alg) == PSA_ALG_CCM)
    {
        /* CCM requires that the length be set before setting the nonce */
        if (!operation->length_set)
            return psa_aead_error(operation, PSA_ERROR_BAD_STATE, NULL);

        /* verify proper nonce length */
        if (nonce_length < 7 || nonce_length > 13)
            return psa_aead_error(operation, PSA_ERROR_INVALID_ARGUMENT, NULL);
        ;

        status = AESCCM_setNonce(AESCCM_Hand, nonce, nonce_length);
        if (status != AESCCM_STATUS_SUCCESS)
            return psa_aead_error(operation, PSA_ERROR_HARDWARE_FAILURE, NULL);
    }
    else if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(operation->alg) == PSA_ALG_GCM)
    {
        /* verify proper nonce length */
        if (nonce_length < 1 || nonce_length > PSA_AEAD_NONCE_MAX_SIZE)
            return psa_aead_error(operation, PSA_ERROR_INVALID_ARGUMENT, NULL);

        /* As of writting this, AESGCM driver only supports nonce lengths of 12 */
        status = AESGCM_setIV(AESGCM_Hand, nonce, nonce_length);
        if (status != AESCCM_STATUS_SUCCESS)
            return psa_aead_error(operation, PSA_ERROR_HARDWARE_FAILURE, NULL);
    }
    else
    {
        return psa_aead_error(operation, PSA_ERROR_NOT_SUPPORTED, NULL);
    }

    operation->iv_set  = 1;
    operation->iv_size = nonce_length;

    return PSA_SUCCESS;
}

psa_status_t psa_aead_generate_nonce(psa_aead_operation_t *operation,
                                     uint8_t *nonce,
                                     size_t nonce_size,
                                     size_t *nonce_length)
{

    psa_status_t status;

    if (operation->id == 0 || operation->iv_set || operation->in_error_state)
        return psa_aead_error(operation, PSA_ERROR_BAD_STATE, NULL);

    if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(operation->alg) == PSA_ALG_CCM && !operation->length_set)
        return psa_aead_error(operation, PSA_ERROR_BAD_STATE, NULL);

    /* For CCM, this size may not be correct according to the PSA
     * specification. The PSA Crypto 1.0.1 specification states:
     *
     * CCM encodes the plaintext length pLen in L octets, with L the smallest
     * integer >= 2 where pLen < 2^(8L). The nonce length is then 15 - L bytes.
     *
     * However this restriction that L has to be the smallest integer is not
     * applied in practice, and it is not implementable here since the
     * plaintext length may or may not be known at this time. */
    *nonce_length = PSA_AEAD_NONCE_LENGTH(PSA_KEY_TYPE_AES, operation->alg);

    if (*nonce_length > nonce_size)
        return psa_aead_error(operation, PSA_ERROR_BUFFER_TOO_SMALL, NULL);

    status = psa_generate_random(nonce, *nonce_length);
    if (status != PSA_SUCCESS)
        return psa_aead_error(operation, status, NULL);

    return psa_aead_set_nonce(operation, nonce, *nonce_length);
}

psa_status_t psa_aead_finish(psa_aead_operation_t *operation,
                             uint8_t *ciphertext,
                             size_t ciphertext_size,
                             size_t *ciphertext_length,
                             uint8_t *tag,
                             size_t tag_size,
                             size_t *tag_length)
{

    psa_status_t status = PSA_SUCCESS;

    *ciphertext_length = 0;

    if (operation->id == 0 || !operation->iv_set || operation->in_error_state)
        return psa_aead_error(operation, PSA_ERROR_BAD_STATE, NULL);

    if (ciphertext_size < operation->unprocessed_len)
        return psa_aead_error(operation, PSA_ERROR_BUFFER_TOO_SMALL, NULL);

    if (tag_size < operation->tagSize)
        return psa_aead_error(operation, PSA_ERROR_BUFFER_TOO_SMALL, NULL);

    if (operation->length_set && operation->runningPlaintextLength < operation->plaintextLength)
        return psa_aead_error(operation, PSA_ERROR_INVALID_ARGUMENT, NULL);

    if (operation->length_set && operation->runningADLength < operation->adLength)
        return psa_aead_error(operation, PSA_ERROR_INVALID_ARGUMENT, NULL);

    /* to handle the case when update is never called because there is zero plaintext,
     * but there is still additonal data that has not been processed */
    if (!operation->done_updating_ad)
    {
        status = psa_aead_update_ad_internal(operation->alg, operation->unprocessedData, operation->unprocessed_len);
        if (status != PSA_SUCCESS)
            return psa_aead_error(operation, status, NULL);

        operation->done_updating_ad = 1;
        operation->unprocessed_len  = 0;
    }

    if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(operation->alg) == PSA_ALG_CCM)
    {

        AESCCM_SegmentedFinalizeOperation segmentedFinalizeOperation;
        AESCCM_SegmentedFinalizeOperation_init(&segmentedFinalizeOperation);
        segmentedFinalizeOperation.input       = operation->unprocessedData;
        segmentedFinalizeOperation.output      = ciphertext;
        segmentedFinalizeOperation.inputLength = operation->unprocessed_len;
        segmentedFinalizeOperation.mac         = tag;
        segmentedFinalizeOperation.macLength   = operation->tagSize;

        status                     = AESCCM_finalizeEncrypt(AESCCM_Hand, &segmentedFinalizeOperation);
        *tag_length                = segmentedFinalizeOperation.macLength;
        *ciphertext_length         = segmentedFinalizeOperation.inputLength;
        operation->unprocessed_len = 0;
        if (status != AESCCM_STATUS_SUCCESS)
            return psa_aead_error(operation, PSA_ERROR_HARDWARE_FAILURE, NULL);
    }
    else if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(operation->alg) == PSA_ALG_GCM)
    {

        AESGCM_SegmentedFinalizeOperation segmentedFinalizeOperation;
        AESGCM_SegmentedFinalizeOperation_init(&segmentedFinalizeOperation);
        segmentedFinalizeOperation.input       = operation->unprocessedData;
        segmentedFinalizeOperation.output      = ciphertext;
        segmentedFinalizeOperation.inputLength = operation->unprocessed_len;
        segmentedFinalizeOperation.mac         = tag;
        segmentedFinalizeOperation.macLength   = operation->tagSize;

        status                     = AESGCM_finalizeEncrypt(AESGCM_Hand, &segmentedFinalizeOperation);
        *tag_length                = segmentedFinalizeOperation.macLength;
        *ciphertext_length         = segmentedFinalizeOperation.inputLength;
        operation->unprocessed_len = 0;
        if (status != AESGCM_STATUS_SUCCESS)
            return psa_aead_error(operation, PSA_ERROR_HARDWARE_FAILURE, NULL);
    }
    else
    {
        return psa_aead_error(operation, PSA_ERROR_NOT_SUPPORTED, NULL);
    }
    return PSA_SUCCESS;
}

psa_status_t psa_aead_verify(psa_aead_operation_t *operation,
                             uint8_t *plaintext,
                             size_t plaintext_size,
                             size_t *plaintext_length,
                             uint8_t *tag,
                             size_t tag_length)

{
    psa_status_t status = PSA_SUCCESS;

    *plaintext_length = 0;

    if (operation->id == 0 || !operation->iv_set || operation->in_error_state)
        return psa_aead_error(operation, PSA_ERROR_BAD_STATE, NULL);

    if (plaintext_size < operation->unprocessed_len)
        return psa_aead_error(operation, PSA_ERROR_BUFFER_TOO_SMALL, NULL);

    if (operation->length_set && operation->runningPlaintextLength < operation->plaintextLength)
        return psa_aead_error(operation, PSA_ERROR_INVALID_ARGUMENT, NULL);

    if (operation->length_set && operation->runningADLength < operation->adLength)
        return psa_aead_error(operation, PSA_ERROR_INVALID_ARGUMENT, NULL);

    /* to handle the case when update is never called because there is zero ciphertext,
     * but there is still additonal data that has not been processed */
    if (!operation->done_updating_ad)
    {
        status = psa_aead_update_ad_internal(operation->alg, operation->unprocessedData, operation->unprocessed_len);
        if (status != PSA_SUCCESS)
            return psa_aead_error(operation, status, NULL);

        operation->done_updating_ad = 1;
        operation->unprocessed_len  = 0;
    }

    if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(operation->alg) == PSA_ALG_CCM)
    {

        AESCCM_SegmentedFinalizeOperation segmentedFinalizeOperation;
        AESCCM_SegmentedFinalizeOperation_init(&segmentedFinalizeOperation);
        segmentedFinalizeOperation.input       = operation->unprocessedData;
        segmentedFinalizeOperation.output      = plaintext;
        segmentedFinalizeOperation.inputLength = operation->unprocessed_len;
        segmentedFinalizeOperation.mac         = tag;
        segmentedFinalizeOperation.macLength   = operation->tagSize;

        status            = AESCCM_finalizeDecrypt(AESCCM_Hand, &segmentedFinalizeOperation);
        *plaintext_length = segmentedFinalizeOperation.inputLength;
        if (status != AESCCM_STATUS_SUCCESS)
            return psa_aead_error(operation, PSA_ERROR_HARDWARE_FAILURE, NULL);
    }
    else if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(operation->alg) == PSA_ALG_GCM)
    {

        AESGCM_SegmentedFinalizeOperation segmentedFinalizeOperation;
        AESGCM_SegmentedFinalizeOperation_init(&segmentedFinalizeOperation);
        segmentedFinalizeOperation.input       = operation->unprocessedData;
        segmentedFinalizeOperation.output      = plaintext;
        segmentedFinalizeOperation.inputLength = operation->unprocessed_len;
        segmentedFinalizeOperation.mac         = tag;
        segmentedFinalizeOperation.macLength   = operation->tagSize;

        status            = AESGCM_finalizeDecrypt(AESGCM_Hand, &segmentedFinalizeOperation);
        *plaintext_length = segmentedFinalizeOperation.inputLength;
        if (status != AESGCM_STATUS_SUCCESS)
            return psa_aead_error(operation, PSA_ERROR_HARDWARE_FAILURE, NULL);
    }
    else
    {
        return psa_aead_error(operation, PSA_ERROR_NOT_SUPPORTED, NULL);
    }
    return PSA_SUCCESS;
}

psa_status_t psa_aead_abort(psa_aead_operation_t *operation)
{
    psa_status_t status = PSA_SUCCESS;
    if (operation->id == 0)
    {
        /* The object has (apparently) been initialized but it is not (yet)
         * in use. It's ok to call abort on such an object, and there's
         * nothing to do. */
        return (PSA_SUCCESS);
    }

    switch (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(operation->alg))
    {
        case PSA_ALG_CCM:
            /* No AESCCM cancellation operation for polling mode */
            break;

        case PSA_ALG_GCM:
            /* No AESGCM cancellation operation for polling mode */
            break;
        default:
            status = PSA_ERROR_NOT_SUPPORTED;
    }

    if (status != AES_STATUS_SUCCESS)
    {
        return psa_aead_error(operation, PSA_ERROR_HARDWARE_FAILURE, NULL);
    }

    *operation = PSA_AEAD_OPERATION_INIT;

    return PSA_SUCCESS;
}

/* one step */

psa_status_t psa_aead_encrypt(psa_key_id_t key,
                              psa_algorithm_t alg,
                              const uint8_t *nonce,
                              size_t nonce_length,
                              const uint8_t *additional_data,
                              size_t additional_data_length,
                              const uint8_t *plaintext,
                              size_t plaintext_length,
                              uint8_t *ciphertext,
                              size_t ciphertext_size,
                              size_t *ciphertext_length)
{
    psa_status_t status             = PSA_ERROR_CORRUPTION_DETECTED;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    uint8_t *tag;
    psa_key_type_t keyType;
    size_t keyBits;
    size_t tagLength;

    *ciphertext_length = 0;

    CryptoKey cryptoKey;

    if (!PSA_ALG_IS_AEAD(alg) || PSA_ALG_IS_WILDCARD(alg))
        return (PSA_ERROR_NOT_SUPPORTED);

    status = psa_get_key_attributes(key, &attributes);
    if (status != PSA_SUCCESS)
        goto exit;

    status = psa_key_attributes_usage_check(&attributes, PSA_KEY_USAGE_ENCRYPT, alg);
    if (status != PSA_SUCCESS)
        goto exit;

    
    keyType = psa_get_key_type(&attributes);
    keyBits = psa_get_key_bits(&attributes);
    tagLength = PSA_AEAD_TAG_LENGTH(keyType, keyBits, alg);
    if (tagLength > PSA_AEAD_TAG_MAX_SIZE)
    {
        status = PSA_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    /* user might pass in bigger
     if (ciphertext_size > PSA_AEAD_ENCRYPT_OUTPUT_MAX_SIZE(plaintext_length))
     {
     status = PSA_ERROR_INVALID_ARGUMENT;
     goto exit;
     } */

    if (ciphertext_size < (plaintext_length + tagLength))
    {
        status = PSA_ERROR_BUFFER_TOO_SMALL;
        goto exit;
    }

    tag = ciphertext + plaintext_length;

    /* true for supported AEAD */
    if (PSA_BLOCK_CIPHER_BLOCK_LENGTH(keyType) != PSA_AES_BLOCK_SIZE)
    {
        status = (PSA_ERROR_INVALID_ARGUMENT);
        goto exit;
    }

    if ((tag == NULL && tagLength != 0) || (additional_data == NULL && additional_data_length != 0) ||
        (plaintext == NULL && plaintext_length != 0) || (ciphertext == NULL && ciphertext_size != 0) || (nonce == NULL))
    {
        status = (PSA_ERROR_INVALID_ARGUMENT);
        goto exit;
    }

    KeyStore_PSA_initKey(&cryptoKey, toKeyStoreKeyID(key), PSA_BITS_TO_BYTES(keyBits), NULL);

    switch (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg, 0))
    {
        case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0):
            {

                /* check for correct tag size
                 * Test assumes tagLength=0 is invalid */
                if (tagLength != 4 && tagLength != 6 && tagLength != 8 && tagLength != 10 && tagLength != 12 &&
                    tagLength != 14 && tagLength != 16)
                {
                    status = (PSA_ERROR_INVALID_ARGUMENT);
                    goto exit;
                }
                /*add a check here to make sure the handle is open */
                if (AESCCM_Hand == NULL)
                {
                    status = PSA_ERROR_BAD_STATE;
                    goto exit;
                }

                AESCCM_OneStepOperation operation;
                AESCCM_OneStepOperation_init(&operation);
                operation.key         = &cryptoKey;
                operation.aad         = (uint8_t *)additional_data;
                operation.aadLength   = additional_data_length;
                operation.input       = (uint8_t *)plaintext;
                operation.output      = ciphertext;
                operation.inputLength = plaintext_length;
                operation.nonce       = (uint8_t *)nonce;
                operation.nonceLength = nonce_length;
                operation.mac         = tag;
                operation.macLength   = tagLength;
                status                = AESCCM_oneStepEncrypt(AESCCM_Hand, &operation);
                if (status != AESCCM_STATUS_SUCCESS)
                {
                    status = map_AESError_to_PSA_error(status);
                    goto exit;
                }
                break;
            }
        case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, 0):
            {

                /* check for correct tag size
                 * Test assumes tagLength=0 is invalid */
                if (tagLength != 4 && tagLength != 8 && tagLength != 12 && tagLength != 13 && tagLength != 14 &&
                    tagLength != 15 && tagLength != 16)
                {
                    status = (PSA_ERROR_INVALID_ARGUMENT);
                    goto exit;
                }

                if (AESGCM_Hand == NULL)
                {
                    return PSA_ERROR_BAD_STATE;
                }
                AESGCM_OneStepOperation operation;
                AESGCM_OneStepOperation_init(&operation);
                operation.key                   = &cryptoKey;
                operation.aad                   = (uint8_t *)additional_data;
                operation.aadLength             = additional_data_length;
                operation.input                 = (uint8_t *)plaintext;
                operation.output                = ciphertext;
                operation.inputLength           = plaintext_length;
                operation.iv                    = (uint8_t *)nonce;
                operation.ivLength              = nonce_length;
                operation.ivInternallyGenerated = 0;
                operation.mac                   = tag;
                operation.macLength             = tagLength;
                status                          = AESGCM_oneStepEncrypt(AESGCM_Hand, &operation);
                if (status != AESGCM_STATUS_SUCCESS)
                {
                    status = map_AESError_to_PSA_error(status);
                    goto exit;
                }
                break;
            }

        default:
            status = PSA_ERROR_NOT_SUPPORTED;
    }

exit:
    if (status != PSA_SUCCESS && ciphertext_size != 0)
        memset(ciphertext, 0, ciphertext_size);

    if (status == PSA_SUCCESS)
        *ciphertext_length = PSA_AEAD_ENCRYPT_OUTPUT_SIZE(keyType, alg, plaintext_length);

    psa_reset_key_attributes(&attributes);
    return (status);
}

psa_status_t psa_aead_decrypt(psa_key_id_t key,
                              psa_algorithm_t alg,
                              const uint8_t *nonce,
                              size_t nonce_length,
                              const uint8_t *additional_data,
                              size_t additional_data_length,
                              const uint8_t *ciphertext,
                              size_t ciphertext_length,
                              uint8_t *plaintext,
                              size_t plaintext_size,
                              size_t *plaintext_length)
{
    psa_status_t status             = PSA_ERROR_CORRUPTION_DETECTED;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    uint8_t *tag;

    CryptoKey cryptoKey;
    psa_key_type_t keyType;
    size_t keyBits;
    size_t tagLength;
    size_t payload_length;

    *plaintext_length = 0;

    if (!PSA_ALG_IS_AEAD(alg) || PSA_ALG_IS_WILDCARD(alg))
        return (PSA_ERROR_NOT_SUPPORTED);

    status = psa_get_key_attributes(key, &attributes);
    if (status != PSA_SUCCESS)
    {
        goto exit;
    }

    status = psa_key_attributes_usage_check(&attributes, PSA_KEY_USAGE_DECRYPT, alg);
    if (status != PSA_SUCCESS)
    {
        goto exit;
    }

    keyType = psa_get_key_type(&attributes);
    keyBits = psa_get_key_bits(&attributes);
    tagLength = PSA_AEAD_TAG_LENGTH(keyType, keyBits, alg);
    payload_length = ciphertext_length - tagLength;
    if (tagLength > PSA_AEAD_TAG_MAX_SIZE)
    {
        status = PSA_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    if (tagLength > ciphertext_length)
    {
        status = PSA_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    if (payload_length > plaintext_size)
    {
        status = PSA_ERROR_BUFFER_TOO_SMALL;
        goto exit;
    }
    tag = (uint8_t *)(ciphertext + payload_length);

    /* true for supported AEAD */
    if (PSA_BLOCK_CIPHER_BLOCK_LENGTH(keyType) != PSA_AES_BLOCK_SIZE)
    {
        status = (PSA_ERROR_INVALID_ARGUMENT);
        goto exit;
    }

    if ((tag == NULL && tagLength != 0) || (additional_data == NULL && additional_data_length != 0) ||
        (plaintext == NULL && plaintext_length != 0) || (ciphertext == NULL && ciphertext_length != 0) ||
        (nonce == NULL))
    {
        status = (PSA_ERROR_INVALID_ARGUMENT);
        goto exit;
    }

    KeyStore_PSA_initKey(&cryptoKey, toKeyStoreKeyID(key), PSA_BITS_TO_BYTES(keyBits), NULL);

    switch (PSA_ALG_AEAD_WITH_SHORTENED_TAG(alg, 0))
    {
        case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 0):
            {

                /* check for correct tag size
                 * Test assumes tagLength=0 is invalid */
                if (tagLength != 4 && tagLength != 6 && tagLength != 8 && tagLength != 10 && tagLength != 12 &&
                    tagLength != 14 && tagLength != 16)
                {
                    status = (PSA_ERROR_INVALID_ARGUMENT);
                    goto exit;
                }

                /*add a check here to make sure the handle is open */
                if (AESCCM_Hand == NULL)
                {
                    status = PSA_ERROR_BAD_STATE;
                    goto exit;
                }

                AESCCM_OneStepOperation operation;
                AESCCM_OneStepOperation_init(&operation);
                operation.key         = &cryptoKey;
                operation.aad         = (uint8_t *)additional_data;
                operation.aadLength   = additional_data_length;
                operation.input       = (uint8_t *)ciphertext;
                operation.output      = (uint8_t *)plaintext;
                operation.inputLength = payload_length;
                operation.nonce       = (uint8_t *)nonce;
                operation.nonceLength = nonce_length;
                operation.mac         = tag;
                operation.macLength   = tagLength;
                status                = AESCCM_oneStepDecrypt(AESCCM_Hand, &operation);
                if (status != AESCCM_STATUS_SUCCESS)
                {
                    status = map_AESError_to_PSA_error(status);
                    goto exit;
                }
                break;
            }
        case PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_GCM, 0):
            {

                /* check for correct tag size
                 * Test assumes tagLength=0 is invalid */
                if (tagLength != 4 && tagLength != 8 && tagLength != 12 && tagLength != 13 && tagLength != 14 &&
                    tagLength != 15 && tagLength != 16)
                {
                    status = (PSA_ERROR_INVALID_ARGUMENT);
                    goto exit;
                }

                if (AESGCM_Hand == NULL)
                {
                    return PSA_ERROR_BAD_STATE;
                }
                AESGCM_OneStepOperation operation;
                AESGCM_OneStepOperation_init(&operation);
                operation.key                   = &cryptoKey;
                operation.aad                   = (uint8_t *)additional_data;
                operation.aadLength             = additional_data_length;
                operation.input                 = (uint8_t *)ciphertext;
                operation.output                = plaintext;
                operation.inputLength           = payload_length;
                operation.iv                    = (uint8_t *)nonce;
                operation.ivLength              = nonce_length;
                operation.ivInternallyGenerated = 0;
                operation.mac                   = tag;
                operation.macLength             = tagLength;
                status                          = AESGCM_oneStepDecrypt(AESGCM_Hand, &operation);
                if (status != AESGCM_STATUS_SUCCESS)
                {
                    status = map_AESError_to_PSA_error(status);
                    goto exit;
                }
                break;
            }

        default:
            status = PSA_ERROR_NOT_SUPPORTED;
    }

exit:

    if (status == PSA_SUCCESS)
        *plaintext_length = PSA_AEAD_DECRYPT_OUTPUT_SIZE(keyType, alg, ciphertext_length);

    psa_reset_key_attributes(&attributes);
    return (status);
}

/****************************************************************/
/* Generators */
/****************************************************************/

psa_status_t psa_key_derivation_abort(psa_key_derivation_operation_t *operation)
{
    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_key_derivation_get_capacity(const psa_key_derivation_operation_t *operation, size_t *capacity)
{
    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_key_derivation_set_capacity(psa_key_derivation_operation_t *operation, size_t capacity)
{
    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_key_derivation_output_bytes(psa_key_derivation_operation_t *operation,
                                             uint8_t *output,
                                             size_t output_length)
{
    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_key_derivation_output_key(const psa_key_attributes_t *attributes,
                                           psa_key_derivation_operation_t *operation,
                                           psa_key_id_t *key)
{
    return PSA_ERROR_NOT_SUPPORTED;
}

/****************************************************************/
/* Key derivation */
/****************************************************************/
psa_status_t psa_key_derivation_setup(psa_key_derivation_operation_t *operation, psa_algorithm_t alg)
{

    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_key_derivation_input_bytes(psa_key_derivation_operation_t *operation,
                                            psa_key_derivation_step_t step,
                                            const uint8_t *data,
                                            size_t data_length)
{
    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t psa_key_derivation_input_key(psa_key_derivation_operation_t *operation,
                                          psa_key_derivation_step_t step,
                                          psa_key_id_t key)
{
    return PSA_ERROR_NOT_SUPPORTED;
}

/****************************************************************/
/* Key agreement */
/****************************************************************/

psa_status_t psa_raw_key_agreement(psa_algorithm_t alg,
                                   psa_key_id_t private_key,
                                   const uint8_t *peer_key,
                                   size_t peer_key_length,
                                   uint8_t *shared_secret,
                                   size_t shared_secret_size,
                                   size_t *shared_secret_length)
{
    psa_status_t status                         = PSA_ERROR_CORRUPTION_DETECTED;
    psa_key_attributes_t private_key_attributes = PSA_KEY_ATTRIBUTES_INIT;
    CryptoKey myPrivateKey;
    CryptoKey sharedSecret;
    CryptoKey myPublicKey;
    psa_key_type_t privateKeyType;
    size_t key_bits;
    psa_ecc_family_t curveFamily;
    size_t curveBits;
    size_t publicKeyBytes;

    if (ECDH_Hand == NULL)
    {
        return PSA_ERROR_BAD_STATE;
    }

    /* For now, ECDH is the only supported key agreement algorithm */
    if (alg != PSA_ALG_ECDH)
    {
        status = PSA_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    status = psa_get_key_attributes(private_key, &private_key_attributes);
    if (status != PSA_SUCCESS)
    {
        goto exit;
    }

    status = psa_key_attributes_usage_check(&private_key_attributes, PSA_KEY_USAGE_DERIVE, alg);
    if (status != PSA_SUCCESS)
    {
        goto exit;
    }
    
    privateKeyType = psa_get_key_type(&private_key_attributes);
    curveFamily = PSA_KEY_TYPE_ECC_GET_FAMILY(privateKeyType);
    key_bits = psa_get_key_bits(&private_key_attributes);
    curveBits = psa_get_key_bits(&private_key_attributes);
    
    /* the only correct type here is PSA_KEY_TYPE_ECC_PUBLIC_KEY */
    if (!PSA_KEY_TYPE_IS_ECC_KEY_PAIR(privateKeyType))
    {
        status = PSA_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    ECDH_OperationComputeSharedSecret OperationComputeSharedSecret;
    ECDH_OperationComputeSharedSecret_init(&OperationComputeSharedSecret);

    OperationComputeSharedSecret.curve = map_keyTypeToECCParams(privateKeyType, curveBits);
    if (OperationComputeSharedSecret.curve == NULL)
    {
        status = PSA_ERROR_NOT_SUPPORTED;
        goto exit;
    }

    if (curveFamily == PSA_ECC_FAMILY_MONTGOMERY)
    {
        publicKeyBytes                                     = PSA_BITS_TO_BYTES(curveBits);
        OperationComputeSharedSecret.keyMaterialEndianness = ECDH_LITTLE_ENDIAN_KEY;
    }
    else
    {
        publicKeyBytes = 2 * PSA_BITS_TO_BYTES(curveBits) + 1;

        /* Check that the first byte is 0x04 */
        if (*peer_key != 0x04)
        {
            /* Key format required for platform implementation. For more info, see TI Drivers API documentation. */
            status = PSA_ERROR_INVALID_ARGUMENT;
            goto exit;
        }
    }

    /* Check if peer key has the correct length to verify that it is on the same curve. This check needs to change
     * in the future to a better check that verifies the public key is on the correct (chosen) curve. */
    if (peer_key_length != publicKeyBytes)
    {
        status = PSA_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    if (shared_secret_size < publicKeyBytes)
    {
        status = PSA_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    /* The device supports Montgomery and Short Weierstrass */
    KeyStore_PSA_initKey(&myPrivateKey, toKeyStoreKeyID(private_key), PSA_BITS_TO_BYTES(key_bits), NULL);

    CryptoKeyPlaintext_initKey(&myPublicKey, (uint8_t *)peer_key, peer_key_length);
    CryptoKeyPlaintext_initBlankKey(&sharedSecret, shared_secret, publicKeyBytes);

    if (status != 0)
    {
        status = PSA_ERROR_HARDWARE_FAILURE;
        goto exit;
    }

    OperationComputeSharedSecret.myPrivateKey   = &myPrivateKey;
    OperationComputeSharedSecret.theirPublicKey = &myPublicKey;
    OperationComputeSharedSecret.sharedSecret   = &sharedSecret;

    status = ECDH_computeSharedSecret(ECDH_Hand, &OperationComputeSharedSecret);
    if (status != 0)
    {
        status = PSA_ERROR_HARDWARE_FAILURE;
        goto exit;
    }

    *shared_secret_length = publicKeyBytes;

exit:
    psa_reset_key_attributes(&private_key_attributes);
    return status;
}

psa_status_t psa_key_derivation_key_agreement(psa_key_derivation_operation_t *operation,
                                              psa_key_derivation_step_t step,
                                              psa_key_id_t private_key,
                                              const uint8_t *peer_key,
                                              size_t peer_key_length)
{
    return PSA_ERROR_NOT_SUPPORTED;
}

/****************************************************************/
/* Random generation */
/****************************************************************/

psa_status_t psa_generate_random(uint8_t *output, size_t output_size)
{
    psa_status_t status;

    if (TRNG_Hand == NULL)
    {
        return PSA_ERROR_BAD_STATE;
    }

    status = TRNG_getRandomBytes(TRNG_Hand, output, output_size);
    if (status != 0)
    {
        status = map_TRNGError_to_PSA_error(status);
    }

    return status;
}

psa_status_t psa_generate_key(const psa_key_attributes_t *attributes, psa_key_id_t *key)
{

    psa_key_type_t keyType = psa_get_key_type(attributes);
    size_t keyBytes        = PSA_BITS_TO_BYTES(psa_get_key_bits(attributes));

    if (PSA_KEY_TYPE_IS_PUBLIC_KEY(keyType))
    {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    CryptoKey cryptoKey;
    uint8_t *keyMaterial = (uint8_t *)malloc(keyBytes);

    CryptoKeyPlaintext_initBlankKey(&cryptoKey, keyMaterial, keyBytes);

    psa_status_t status = TRNG_generateKey(TRNG_Hand, &cryptoKey);

    if (status == PSA_SUCCESS)
    {
        status = psa_import_key(attributes, keyMaterial, keyBytes, key);
    }

    if (status != PSA_SUCCESS)
    {
        status = map_TRNGError_to_PSA_error(status);
    }

    if (keyMaterial != NULL)
    {
        free(keyMaterial);
    }

    return status;
}

/****************************************************************/
/* Module setup */
/****************************************************************/

psa_status_t psa_crypto_init(void)
{
    /* Driver init */
    psa_status_t status = PSA_SUCCESS;
    if (AESCMAC_Hand == NULL)
    {
        AESCMAC_init();

        AESCMAC_Params AESCMAC_params;
        AESCMAC_Params_init(&AESCMAC_params);
        AESCMAC_params.returnBehavior  = AESCMAC_RETURN_BEHAVIOR_POLLING;
        AESCMAC_params.operationalMode = AESCMAC_OPMODE_CMAC;
        AESCMAC_Hand                   = AESCMAC_open(0, &AESCMAC_params);
    }

    if (AESCBC_MAC_Hand == NULL)
    {
        AESCMAC_init();

        AESCMAC_Params AESCBCMAC_params;
        AESCMAC_Params_init(&AESCBCMAC_params);
        AESCBCMAC_params.returnBehavior  = AESCMAC_RETURN_BEHAVIOR_POLLING;
        AESCBCMAC_params.operationalMode = AESCMAC_OPMODE_CBCMAC;
        AESCBC_MAC_Hand                  = AESCMAC_open(1, &AESCBCMAC_params);
    }

    if (SHA2_Hand == NULL)
    {
        SHA2_init();

        SHA2_Params SHA2_params;
        SHA2_Params_init(&SHA2_params);
        SHA2_params.returnBehavior = SHA2_RETURN_BEHAVIOR_POLLING;
        SHA2_Hand                  = SHA2_open(0, &SHA2_params);
    }

    if (ECDH_Hand == NULL)
    {
        ECDH_init();

        ECDH_Params ECDH_params;
        ECDH_Params_init(&ECDH_params);
        ECDH_params.returnBehavior = ECDH_RETURN_BEHAVIOR_BLOCKING;
        ECDH_Hand = ECDH_open(0, &ECDH_params);
    }

    if (AESECB_Hand == NULL)
    {
        AESECB_init();

        AESECB_Params AESECB_params;
        AESECB_Params_init(&AESECB_params);
        AESECB_params.returnBehavior = AESECB_RETURN_BEHAVIOR_POLLING;
        AESECB_Hand = AESECB_open(0, &AESECB_params);
    }

    if (AESCBC_Hand == NULL)
    {
        AESCBC_init();

        AESCBC_Params AESCBC_params;
        AESCBC_Params_init(&AESCBC_params);
        AESCBC_params.returnBehavior = AESCBC_RETURN_BEHAVIOR_POLLING;
        AESCBC_Hand = AESCBC_open(0, &AESCBC_params);
    }

    if (AESCTR_Hand == NULL)
    {
        AESCTR_init();

        AESCTR_Params AESCTR_params;
        AESCTR_Params_init(&AESCTR_params);
        AESCTR_params.returnBehavior = AESCTR_RETURN_BEHAVIOR_POLLING;
        AESCTR_Hand = AESCTR_open(0, &AESCTR_params);
    }

    if (AESCCM_Hand == NULL)
    {
        AESCCM_init();

        AESCCM_Params AESCCM_params;
        AESCCM_Params_init(&AESCCM_params);
        AESCCM_params.returnBehavior = AESCCM_RETURN_BEHAVIOR_POLLING;
        AESCCM_Hand                  = AESCCM_open(0, &AESCCM_params);
    }

    if (AESGCM_Hand == NULL)
    {
        AESGCM_init();

        AESGCM_Params AESGCM_params;
        AESGCM_Params_init(&AESGCM_params);
        AESGCM_params.returnBehavior = AESGCM_RETURN_BEHAVIOR_POLLING;
        AESGCM_Hand = AESGCM_open(0, &AESGCM_params);
    }

    if (TRNG_Hand == NULL)
    {
        TRNG_init();

        TRNG_Params TRNG_params;
        TRNG_Params_init(&TRNG_params);
        TRNG_params.returnBehavior = TRNG_RETURN_BEHAVIOR_POLLING;
        TRNG_Hand                  = TRNG_open(0, &TRNG_params);
    }

#if (TFM_ENABLED == 0)
    /* No TZ-enabled support for ECDSA and EDDSA */
    if (ECDSA_Hand == NULL)
    {
        ECDSA_init();

        ECDSA_Params ECDSA_params;
        ECDSA_Params_init(&ECDSA_params);
        ECDSA_params.returnBehavior = ECDSA_RETURN_BEHAVIOR_POLLING;
        ECDSA_Hand                  = ECDSA_open(0, &ECDSA_params);
    }

    if (EDDSA_Hand == NULL)
    {
        EDDSA_init();

        EDDSA_Params EDDSA_params;
        EDDSA_Params_init(&EDDSA_params);
        EDDSA_params.returnBehavior = EDDSA_RETURN_BEHAVIOR_POLLING;
        EDDSA_Hand                  = EDDSA_open(0, &EDDSA_params);
    }

    /* Only need to initialize in TFM-disabled build */
    status = KeyStore_PSA_init();
#endif

    return status;
}
