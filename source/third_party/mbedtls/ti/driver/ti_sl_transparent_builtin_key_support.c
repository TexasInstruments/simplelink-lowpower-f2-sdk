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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ti_sl_transparent_builtin_key_support.h"
#include <mbedtls/private_access.h>
#include <psa/crypto.h>

/**
 * @brief Counter to keep track of the number of pre-provisioned keys available during KeyStore_PSA_init()
 *
 * preProvisionedKeyCount is set to the empty macro to indicate no provisioned keys and later initialized to the number
 * of pre-provisioned key memory if pre-provisioned keys are available during initialization of the KeyStore driver.
 */
static uint32_t preProvisionedKeyCount;

/**
 * @brief Global array of pre-provisioned key IDs
 *
 * During KeyStore_PSA_init(), the pre-provisioned key memory is checked for the existence of pre-provisioned keys. If
 * they exist, pre-provisioned key memory is parsed to copy the necessary information in the the preProvisionedKeyIds
 * including the key ID, algorithm, usage, type, lifetime, address of the plaintext material, and the key length. This
 * array is later used to check the availability of a key in pre-provisioned key memory when an application or requests
 * for the key using key ID.
 */
static KeyStore_PSA_preProvisionedKeyIDs preProvisionedKeyIds[KEYSTORE_PSA_MAX_PREPROVISIONED_KEYS];

/*
 *  ======== KeyStore_PSA_initPreProvisionedKeyIDs ========
 */
static psa_status_t KeyStore_PSA_initPreProvisionedKeyIDs(void)
{
    uint8_t *currentKey;
    uint32_t keySize;
    uint32_t lifetime;
    uint32_t keyLength;
    uint32_t end;

    currentKey = (uint8_t *)(TI_SL_BUILT_IN_KEY_AREA_ADDR);

    /* Check for magic header to identify if pre-provisioned keys were programmed */
    if (memcmp(currentKey,
               KEYSTORE_PSA_PRE_PROVISIONED_KEY_MAGIC_HEADER,
               KEYSTORE_PSA_PRE_PROVISIONED_KEY_MAGIC_HEADER_LENGTH) != 0)
    {
        /* No error when pre-provisioned keys are not available */
        return PSA_SUCCESS;
    }

    /* Rest keySize (size of each key) and preProvisionedKeyCount (length of the pre-provisioned key array) */
    keySize                = 0;
    preProvisionedKeyCount = 0;

    /* Parse the pre-provisioned key memory until reaching the final end pattern for PP keys or empty pattern which
     * allows further addition of pre-provisioned keys to create a local copy of the key ID, size, and starting address
     * to be used by the application
     */
    do
    {
        /* Skip the Magic Header */
        keySize = KEYSTORE_PSA_PRE_PROVISIONED_KEY_MAGIC_HEADER_LENGTH;

        /* Copy Lifetime */
        memcpy(&lifetime, (currentKey + keySize), MEMBER_SIZE(KeyStore_PreProvisionedKeyMetaData, tiLifetime));
        keySize += MEMBER_SIZE(KeyStore_PreProvisionedKeyMetaData, tiLifetime);

        if (lifetime == KEYSTORE_PSA_PRE_PROVISIONED_KEY_VALID_LIFETIME)
        {
            /* Copy the psa_key_lifetime_t */
            memcpy(&(preProvisionedKeyIds[preProvisionedKeyCount].psaLifetime),
                   (currentKey + keySize),
                   sizeof(psa_key_lifetime_t));
            keySize += sizeof(psa_key_lifetime_t);

            /* Copy the Key_file_id_t */
            memcpy(&(preProvisionedKeyIds[preProvisionedKeyCount].id),
                   (currentKey + keySize),
                   sizeof(mbedtls_svc_key_id_t));
            keySize += sizeof(mbedtls_svc_key_id_t);

            /* Copy the Algorithm */
            memcpy(&(preProvisionedKeyIds[preProvisionedKeyCount].alg),
                   (currentKey + keySize),
                   sizeof(psa_algorithm_t));
            keySize += sizeof(psa_algorithm_t);

            /* Copy the Usage */
            memcpy(&(preProvisionedKeyIds[preProvisionedKeyCount].usage),
                   (currentKey + keySize),
                   sizeof(psa_key_usage_t));
            keySize += sizeof(psa_key_usage_t);

            /* Copy the key length */
            memcpy(&(preProvisionedKeyIds[preProvisionedKeyCount].keyLength),
                   (currentKey + keySize),
                   MEMBER_SIZE(KeyStore_PreProvisionedKeyMetaData, keyLength));
            keySize += MEMBER_SIZE(KeyStore_PreProvisionedKeyMetaData, keyLength);

            /* Copy the psa_key_type_t */
            memcpy(&(preProvisionedKeyIds[preProvisionedKeyCount].type),
                   (currentKey + keySize),
                   sizeof(psa_key_type_t));
            keySize += sizeof(psa_key_type_t);

            /* Skip over the reserved memory */
            keySize += MEMBER_SIZE(KeyStore_PreProvisionedKeyMetaData, reserved);

            /* Copy the starting addr of pre-provisioned key psa_key */
            preProvisionedKeyIds[preProvisionedKeyCount].addr = currentKey + keySize;

            keySize += (preProvisionedKeyIds[preProvisionedKeyCount].keyLength +
                        MEMBER_SIZE(KeyStore_preProvisionedKeyStorageFormat, fletcher));

            /* Increment the count of PP keys available in pre-provisioned key memory */
            preProvisionedKeyCount++;
        }
        else
        {
            /* Skip over the invalid key */
            keySize += (sizeof(mbedtls_svc_key_id_t) + sizeof(psa_algorithm_t) + sizeof(psa_key_usage_t) +
                        sizeof(psa_key_lifetime_t));

            memcpy(&keyLength, (currentKey + keySize), MEMBER_SIZE(KeyStore_PreProvisionedKeyMetaData, keyLength));

            keySize += (MEMBER_SIZE(KeyStore_PreProvisionedKeyMetaData, keyLength) + keyLength +
                        sizeof(psa_key_type_t) + MEMBER_SIZE(KeyStore_preProvisionedKeyStorageFormat, fletcher) +
                        MEMBER_SIZE(KeyStore_PreProvisionedKeyMetaData, reserved));
        }

        /* Set to pre-provisioned key address tracking to the next PP key, if available, after the key and checksum */
        currentKey += keySize;

        /* Reset pre-provisioned key size tracking for the next PP key */
        keySize = 0;

        memcpy(&end, currentKey, sizeof(end));
    } while ((end != KEYSTORE_PSA_PRE_PROVISIONED_KEYS_END) && (end != KEYSTORE_PSA_PREPROVISIONED_KEYS_EMPTY));

    return PSA_SUCCESS;
}

/**
 * @brief Compute Fletcher Checksum-32
 *
 * \param[in]  addr - Starting address of data used for checksum
 * \param[in]  len - Length of data that requires checksum
 * \param[out] checksum - Computed 32-bit checksum
 */
/*
 *  ======== KeyStore_PSA_computeFletcherChecksum ========
 */
static uint32_t KeyStore_PSA_computeFletcherChecksum(uint8_t *addr, uint32_t len)
{
    uint32_t sum1, sum2, i, checksum;
    uint16_t mod;
    mod = (2 ^ (FLETCHER_CHECKSUM_ALGORITHM / 2)) - 1;

    /* Initialize both sums to zero*/
    sum1 = 0;
    sum2 = 0;
    for (i = 0; i < len; i++)
    {
        sum1 += *addr++;
        if (sum1 >= mod)
        {
            sum1 -= mod;
        }
        sum2 += sum1;
        if (sum2 >= mod)
        {
            sum2 -= mod;
        }
    }
    checksum = sum2 << mod | sum1;

    return checksum;
}

/**
 * @brief Verify the fletcher checksum of the give pre-provisioned key ID
 *
 * \param[in]  preProvisionedKey - Pointer to the pre-provisioned key ID structure to be verified
 *
 * \retval #PSA_SUCCESS
 *          Successfully verified checksum
 * \retval #PSA_ERROR_CORRUPTION_DETECTED
 *          Fletched checksum verification failed
 * \retval #PSA_ERROR_INSUFFICIENT_MEMORY
 *          Insufficient memory in heap to create a copy of pre-provisioned key
 *          from persistent memory to compute checksum
 */
/*
 *  ======== KeyStore_PSA_verifyFletcherChecksum ========
 */
static int_fast16_t KeyStore_PSA_verifyFletcherChecksum(KeyStore_PSA_preProvisionedKeyIDs *preProvisionedKey)
{
    uint32_t expectedChecksum, computedChecksum;
    int_fast16_t status = PSA_ERROR_GENERIC_ERROR;

    uint8_t keySize = KEYSTORE_PRE_PROVISIONED_KEY_METADATA_SIZE + preProvisionedKey->keyLength;

    memcpy(&expectedChecksum, (preProvisionedKey->addr + preProvisionedKey->keyLength), sizeof(expectedChecksum));

    computedChecksum = KeyStore_PSA_computeFletcherChecksum(preProvisionedKey->addr -
                                                                (KEYSTORE_PRE_PROVISIONED_KEY_METADATA_SIZE),
                                                            keySize);

    if (expectedChecksum == computedChecksum)
    {
        status = PSA_SUCCESS;
    }
    else
    {
        status = PSA_ERROR_CORRUPTION_DETECTED;
    }

    return status;
}

/*
 *  ======== mbedtls_psa_platform_get_builtin_key ========
 */
psa_status_t mbedtls_psa_platform_get_builtin_key(mbedtls_svc_key_id_t key_id,
                                                  psa_key_lifetime_t *lifetime,
                                                  psa_drv_slot_number_t *slot_number)
{
    uint32_t i;
    psa_status_t status;

    /* Check if Key ID matches pre-provisioned key */
    for (i = 0; i <= preProvisionedKeyCount; i++)
    {
        if ((MBEDTLS_SVC_KEY_ID_GET_KEY_ID(key_id) == MBEDTLS_SVC_KEY_ID_GET_KEY_ID(preProvisionedKeyIds[i].id)) &&
            (MBEDTLS_SVC_KEY_ID_GET_OWNER_ID(key_id) == MBEDTLS_SVC_KEY_ID_GET_OWNER_ID(preProvisionedKeyIds[i].id)))
        {
            status = KeyStore_PSA_verifyFletcherChecksum(&preProvisionedKeyIds[i]);
            if (status == PSA_SUCCESS)
            {
                *slot_number = i;
                *lifetime    = preProvisionedKeyIds[i].psaLifetime;
                return PSA_SUCCESS;
            }
            else
            {
                return PSA_ERROR_CORRUPTION_DETECTED;
            }
        }
    }

    return PSA_ERROR_DOES_NOT_EXIST;
}

/*
 *  ======== ti_sl_transparent_builtin_key_loader_init ========
 */
psa_status_t ti_sl_transparent_builtin_key_loader_init(void)
{
    return KeyStore_PSA_initPreProvisionedKeyIDs();
}

/*
 *  ======== ti_sl_transparent_get_builtin_key ========
 */
psa_status_t ti_sl_transparent_get_builtin_key(psa_drv_slot_number_t slot_number,
                                               psa_key_attributes_t *attributes,
                                               uint8_t *key_buffer,
                                               size_t key_buffer_size,
                                               size_t *key_buffer_length)
{
    psa_set_key_type(attributes, preProvisionedKeyIds[slot_number].type);
    psa_set_key_bits(attributes, PSA_BYTES_TO_BITS(preProvisionedKeyIds[slot_number].keyLength));
    psa_set_key_usage_flags(attributes, preProvisionedKeyIds[slot_number].usage);
    psa_set_key_algorithm(attributes, preProvisionedKeyIds[slot_number].alg);

    if (key_buffer_size < preProvisionedKeyIds[slot_number].keyLength)
    {
        return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    *key_buffer_length = preProvisionedKeyIds[slot_number].keyLength;

    /* Copy the actual built-in key */
    memcpy(key_buffer, preProvisionedKeyIds[slot_number].addr, preProvisionedKeyIds[slot_number].keyLength);

    return PSA_SUCCESS;
}

/*
 *  ======== ti_sl_transparent_get_key_buffer_size ========
 */
psa_status_t ti_sl_transparent_get_key_buffer_size(const psa_key_attributes_t *attributes, size_t *key_buffer_size)
{
    uint32_t bits = psa_get_key_bits(attributes);
    if (bits != 0)
    {
        *key_buffer_size = PSA_BITS_TO_BYTES(bits);
        return PSA_SUCCESS;
    }

    return PSA_ERROR_CORRUPTION_DETECTED;
}
