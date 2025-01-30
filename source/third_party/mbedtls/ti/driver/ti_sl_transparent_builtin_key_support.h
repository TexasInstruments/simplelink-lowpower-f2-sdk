/*
 * Copyright (c) 2024, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
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

#ifndef TI_SL_TRANSPARENT_BUILTIN_KEY_SUPPORT_H
#define TI_SL_TRANSPARENT_BUILTIN_KEY_SUPPORT_H

#include <stdint.h>
#include <stddef.h>
#include <psa/crypto.h>

#if defined(ENABLE_TI_SL_MBEDTLS_PSA_DRIVER)
    #ifndef PSA_CRYPTO_ACCELERATOR_DRIVER_PRESENT
        #define PSA_CRYPTO_ACCELERATOR_DRIVER_PRESENT
    #endif /* PSA_CRYPTO_ACCELERATOR_DRIVER_PRESENT */
#endif     /* ENABLE_TI_SL_MBEDTLS_PSA_DRIVER */

/* Macro to obtain size of struct member */
#define MEMBER_SIZE(type, member)    sizeof(((type *)0)->member)
/**
 *  Starting address of Pre-provisioned keys.
 *
 * The immutable platform Root of Trust stores the pre-provisioned key's programmed at
 * production. The built-in key implementation will read this address to obtain the
 * mbedtls_svc_key_id_t and other relevant meta data of all the pre-provisioned keys stored at this address
 */
#define TI_SL_BUILT_IN_KEY_AREA_ADDR 0x0000

#define FLETCHER_CHECKSUM_ALGORITHM 32 /* FLETCHER-32 */

/**
 * @brief Area size for pre-provisioned keys, 2KB - 256B (reserved for attestation data)
 */
#define KEYSTORE_PSA_PREPROVISIONED_AREA_SIZE (0x700) /* 1792 B */

/**
 * @brief Macro to indicate empty pre-provisioned key memory
 */
#define KEYSTORE_PSA_PREPROVISIONED_KEYS_EMPTY 0xFFFF

/**
 * @brief Pre-provisioned key storage magic header.
 */
#define KEYSTORE_PSA_PRE_PROVISIONED_KEY_MAGIC_HEADER        "HUK\0KEY"
#define KEYSTORE_PSA_PRE_PROVISIONED_KEYS_END                0
#define KEYSTORE_PSA_PRE_PROVISIONED_KEY_MAGIC_HEADER_LENGTH (sizeof(KEYSTORE_PSA_PRE_PROVISIONED_KEY_MAGIC_HEADER))
#define KEYSTORE_PSA_PRE_PROVISIONED_KEYS_END_LENGTH         (sizeof(KEYSTORE_PSA_PRE_PROVISIONED_KEYS_END))
#define KEYSTORE_PSA_MAX_PREPROVISIONED_KEYS                 0x10

/**
 * @brief Pre-provisioned key Lifetime
 */
#define KEYSTORE_PSA_PRE_PROVISIONED_KEY_VALID_LIFETIME   0xAAAA
#define KEYSTORE_PSA_PRE_PROVISIONED_KEY_INVALID_LIFETIME 0x8888

/**
 * @brief Admissible key ID range for Pre-provisioned keys
 *
 * 0x7fff0000 - 0x7fffefff is reserved to store pre-provisioned keys and certificates.
 */
#define KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MAX MBEDTLS_PSA_KEY_ID_BUILTIN_MAX
#define KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MIN MBEDTLS_PSA_KEY_ID_BUILTIN_MIN

/**
 * @brief Reserved pre-provisioned key IDs
 *
 * Some known pre-provisioned key used by attestation service have reserved key IDs in the admissible range for
 * pre-provisioned keys
 */
#define KEYSTORE_PSA_PRE_PROVISIONED_ATTESTATION_PUB_KEY_ID KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MIN + 1
#define KEYSTORE_PSA_PRE_PROVISIONED_ATTESTATION_PRI_KEY_ID KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MIN

/**
 * @brief Structure for storing pre-provisioned key's meta data
 *
 * - header      : magic header to indicate the start of the pre-provisioned key
 * - tiLifetime  : TI (vendor) lifetime of the key, to invalidate pre-provisioned key
 * - psaLifetime : Lifetime, persistence and location, of the pre-provisioned key
 * - id          : key id (and its owner when applicable)
 * - alg         : Cryptographic algorithm permitted using this key
 * - usage       : Usage for the key as described the KeyStore API
 * - keyLength   : Length of the pre-provisioned key
 * - type        : Key type of the pre-provisioned key, such as #PSA_KEY_TYPE_AES
 * - reserved    : Reserved padding to word-align pre-provisioned keyMaterial
 */
typedef struct
{
    uint8_t header[KEYSTORE_PSA_PRE_PROVISIONED_KEY_MAGIC_HEADER_LENGTH];
    uint32_t tiLifetime;
    psa_key_lifetime_t psaLifetime;
    mbedtls_svc_key_id_t id;
    psa_algorithm_t alg;
    psa_key_usage_t usage;
    uint32_t keyLength;
    psa_key_type_t type;
    uint16_t reserved; /* 2B reserved */
} KeyStore_PreProvisionedKeyMetaData;

/**
 *  @brief Size of the meta data associated with the pre-provisoned key
 */
#define KEYSTORE_PRE_PROVISIONED_KEY_METADATA_SIZE sizeof(KeyStore_PreProvisionedKeyMetaData)

/**
 * @brief Structure for storing pre-provisioned keys and its meta data
 *
 * - meta        : Structure with meta data associated with each pre-provisioned key
 * - KeyMaterial : A pointer to the plaintext key material
 * - fletcher    : 32-bit Fletcher checksum over the pre-provisioned key and its meta data
 */
typedef struct
{
    KeyStore_PreProvisionedKeyMetaData meta;
    uint8_t *keyMaterial;
    uint32_t fletcher;
} KeyStore_preProvisionedKeyStorageFormat;

/**
 * @brief Structure for storing IDs of pre-provisioned keys
 *
 * Also stores minimal meta data required to import pre-provisioned keys from persistent memory
 *
 * - addr            : Starting address of the plaintext key material in pre-provisioned key memory
 * - psaLifetime     : Lifetime, persistence and location, of the pre-provisioned key
 * - type            : Key type of the pre-provisioned key, such as #PSA_KEY_TYPE_AES
 * - id              : Key ID (and its owner when applicable)
 * - alg             : Algorithm the key will be used for
 * - usage           : Key usage
 * - keyLength       : Size of the pre-provisioned key
 */
typedef struct
{
    uint8_t *addr;
    psa_key_lifetime_t psaLifetime;
    psa_key_type_t type;
    mbedtls_svc_key_id_t id;
    psa_algorithm_t alg;
    psa_key_usage_t usage;
    uint32_t keyLength;
} KeyStore_PSA_preProvisionedKeyIDs;

/* Built-in key support functions */
/** Parse the built-in key region and store its metadata.
 *
 * \retval  #PSA_SUCCESS
 *          Success. Built-in key region was parsed and a copy of
 *          the necessary information about each built-in key is stored in
 *          a global array to be used by key management functions.
 * \retval  #PSA_ERROR_DOES_NOT_EXIST
 *          Built-in keys in the expected format are not present.
 */
psa_status_t ti_sl_transparent_builtin_key_loader_init(void);

/** Obtain the built-in key.
 *
 * \param[in]  slot_number          The slot number of built-in key to get.
 * \param[in]  attributes           The attributes of the key to use for the
 *                                  operation.
 * \param[out] key_buffer           The buffer to contain the key data in
 *                                  output format upon successful return.
 * \param[in]  key_buffer_size      Size of the \p key_buffer buffer in bytes.
 * \param[out] key_buffer_length    The length of the data written in \p
 *                                  key_buffer in bytes.
 *
 * \retval  #PSA_SUCCESS
 *          Success. Keypair generated and stored in buffer.
 * \retval  #PSA_ERROR_BUFFER_TOO_SMALL
 *          \p key_buffer_size is too small.
 */
psa_status_t ti_sl_transparent_get_builtin_key(psa_drv_slot_number_t slot_number,
                                               psa_key_attributes_t *attributes,
                                               uint8_t *key_buffer,
                                               size_t key_buffer_size,
                                               size_t *key_buffer_length);

/** Obtain built-in key size.
 *
 * \param[in]  attributes           The attributes of the key to use for the
 *                                  operation.
 * \param[out]  key_buffer_size     Size of the \p key_buffer buffer in bytes.
 *
 * \retval  #PSA_SUCCESS
 *          Success. Keypair generated and stored in buffer.
 * \retval  #PSA_ERROR_CORRUPTION_DETECTED
 *          Memory corruption detected.
 */
psa_status_t ti_sl_transparent_get_key_buffer_size(const psa_key_attributes_t *attributes, size_t *key_buffer_size);

#endif /* TI_SL_TRANSPARENT_BUILTIN_KEY_SUPPORT_H */
