/*
 * Copyright (c) 2022, Texas Instruments Incorporated
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

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <third_party/tfm/secure_fw/partitions/internal_trusted_storage/tfm_internal_trusted_storage.h> /* tfm_its_init() */

#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_helpers.h>

extern psa_status_t psa_get_key_from_slot(psa_key_handle_t handle,
                                          psa_key_slot_t **p_slot,
                                          psa_key_usage_t usage,
                                          psa_algorithm_t alg);

extern psa_status_t psa_internal_export_key(const psa_key_slot_t *slot,
                                            uint8_t *data,
                                            size_t dataSize,
                                            size_t *dataLength,
                                            int export_public_key);

KeyStore_accessSemaphoreObject KeyStore_semaphoreObject = {.isAcquired = false, .isInitialized = false};

/** @brief Key handle identifier from mbedTLS 'psa_key_handle_t'. */
typedef psa_key_handle_t KeyStore_PSA_KeyHandle;

/**
 * @brief Counter to keep track of the number of pre-provisioned keys available during KeyStore_PSA_init()
 *
 * preProvisionedKeyCount is set to the empty macro to indicate no provisioned keys and later initialized to the length
 * of pre-provisioned key memory if pre-provisioned keys are available during initialization of the KeyStore driver.
 */
static uint32_t preProvisionedKeyCount;

/**
 * @brief Global array of pre-provisioned key IDs
 *
 * During KeyStore_PSA_init(), the pre-provisioned key memory is checked for the existence of pre-provisioned keys. If
 * they exist, pre-provisioned key memory is parsed to copy the necessary information in the the preProvisionedKeyIds
 * including the key ID, algorithm, usage, exportability, address of the plaintext material, and the key length. This
 * array is later used to check the availability of a key in pre-provisioned key memory when an application or requests
 * for the key using key ID.
 */
static KeyStore_PSA_preProvisionedKeyIDs preProvisionedKeyIds[KEYSTORE_PSA_MAX_PREPROVISIONED_KEYS];

/* Flag to prevent multiple initialization of KeyStore driver */
static bool isKeyStoreInitialized = false;

/**
 * @brief Obtain pre-provisioned KeyStore_PSA_KeyFileId if any.
 *
 * This function is called by KeyStore_PSA_init() to parse the pre-provisioned key memory
 * area and identify available key IDs and store the key IDs and other information along
 * with the starting address of the key in pre-provisioned key memory in a global array. The application
 * must provide the necessary key ID and owner information from this array to retrieve the key.
 *
 * \retval #KEYSTORE_PSA_STATUS_SUCCESS
 *         Success.
 * \retval #KEYSTORE_PSA_STATUS_DOES_NOT_EXIST
 *         No pre-provisioned key available in pre-provisioned key memory
 */
/*
 *  ======== KeyStore_PSA_getPreProvisionedKeyIDs ========
 */
static int_fast16_t KeyStore_PSA_getPreProvisionedKeyIDs(void)
{
    uint8_t *currentKey;
    uint32_t keySize;
    uint32_t lifetime;
    uint32_t keyLength;
    uint32_t end;

    currentKey = (uint8_t *)(KEYSTORE_PSA_PREPROVISIONED_AREA_ADDR);

    /* Check for magic header to identify if pre-provisioned keys were programmed */
    if (memcmp(currentKey,
               KEYSTORE_PSA_PRE_PROVISIONED_KEY_MAGIC_HEADER,
               KEYSTORE_PSA_PRE_PROVISIONED_KEY_MAGIC_HEADER_LENGTH) != 0)
    {
        return KEYSTORE_PSA_STATUS_DOES_NOT_EXIST;
    }

    /* Rest keySize (size of each key) and preProvisionedKeyCount (length of the pre-provisioned key array) */
    keySize                = 0;
    preProvisionedKeyCount = 0;

    /*
     * Parse the pre-provisioned key memory until reaching the final end pattern for PP keys or empty pattern which
     * allows further addition of pre-provisioned keys to create a local copy of the key ID, size, and starting address
     * to be used by the application
     */
    do
    {
        /* Skip the Magic Header */
        keySize = KEYSTORE_PSA_PRE_PROVISIONED_KEY_MAGIC_HEADER_LENGTH;

        /* Copy Lifetime */
        memcpy(&lifetime, (currentKey + keySize), MEMBER_SIZE(KeyStore_PreProvisionedKeyMetaData, lifetime));
        keySize += MEMBER_SIZE(KeyStore_PreProvisionedKeyMetaData, lifetime);

        if (lifetime == KEYSTORE_PSA_PRE_PROVISIONED_KEY_VALID_LIFETIME)
        {
            /* Copy the Key_file_id_t */
            memcpy(&(preProvisionedKeyIds[preProvisionedKeyCount].id),
                   (currentKey + keySize),
                   sizeof(KeyStore_PSA_KeyFileId));
            keySize += sizeof(KeyStore_PSA_KeyFileId);

            /* Copy the export flag */
            memcpy(&(preProvisionedKeyIds[preProvisionedKeyCount].export),
                   (currentKey + keySize),
                   sizeof(KEYSTORE_PSA_PRE_PROVISIONED_KEY_EXPORT_FLAG));
            keySize += sizeof(KEYSTORE_PSA_PRE_PROVISIONED_KEY_EXPORT_FLAG);

            /* Copy the Algorithm */
            memcpy(&(preProvisionedKeyIds[preProvisionedKeyCount].alg),
                   (currentKey + keySize),
                   sizeof(KeyStore_PSA_Algorithm));
            keySize += sizeof(KeyStore_PSA_Algorithm);

            /* Copy the Usage */
            memcpy(&(preProvisionedKeyIds[preProvisionedKeyCount].usage),
                   (currentKey + keySize),
                   sizeof(KeyStore_PSA_KeyUsage));
            keySize += sizeof(KeyStore_PSA_KeyUsage);

            /* Copy the key length */
            memcpy(&(preProvisionedKeyIds[preProvisionedKeyCount].keyLength),
                   (currentKey + keySize),
                   MEMBER_SIZE(KeyStore_PreProvisionedKeyMetaData, keyLength));
            keySize += MEMBER_SIZE(KeyStore_PreProvisionedKeyMetaData, keyLength);

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
            keySize += (sizeof(KeyStore_PSA_KeyFileId) + sizeof(KEYSTORE_PSA_PRE_PROVISIONED_KEY_EXPORT_FLAG) +
                        sizeof(KeyStore_PSA_Algorithm) + sizeof(KeyStore_PSA_KeyUsage));

            memcpy(&keyLength, (currentKey + keySize), MEMBER_SIZE(KeyStore_PreProvisionedKeyMetaData, keyLength));

            keySize += (MEMBER_SIZE(KeyStore_PreProvisionedKeyMetaData, keyLength) + keyLength +
                        MEMBER_SIZE(KeyStore_preProvisionedKeyStorageFormat, fletcher));
        }

        /* Set to pre-provisioned key address tracking to the next PP key, if available, after the key and checksum */
        currentKey += keySize;

        /* Reset pre-provisioned key size tracking for the next PP key */
        keySize = 0;

        memcpy(&end, currentKey, sizeof(end));
    } while ((end != KEYSTORE_PSA_PRE_PROVISIONED_KEYS_END) && (end != KEYSTORE_PSA_PREPROVISIONED_KEYS_EMPTY));

    return KEYSTORE_PSA_STATUS_SUCCESS;
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
 * \param[in]  preProvisionedKey - Structure to the pre-provisioned key ID to be verified
 *
 * \retval #KEYSTORE_PSA_STATUS_SUCCESS
 *          Successfully verified checksum
 * \retval #KEYSTORE_PSA_STATUS_CORRUPTION_DETECTED
 *          Fletched checksum verification failed
 * \retval #KEYSTORE_PSA_STATUS_INSUFFICIENT_MEMORY
 *          Insufficient memory in heap to create a copy of pre-provisioned key
 *          from persistent memory to compute checksum
 */
/*
 *  ======== KeyStore_PSA_verifyFletcherChecksum ========
 */
static int_fast16_t KeyStore_PSA_verifyFletcherChecksum(KeyStore_PSA_preProvisionedKeyIDs *preProvisionedKey)
{
    uint32_t expectedChecksum, computedChecksum;
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;

    uint8_t keySize = KEYSTORE_PRE_PROVISIONED_KEY_METADATA_SIZE + preProvisionedKey->keyLength;

    memcpy(&expectedChecksum, (preProvisionedKey->addr + preProvisionedKey->keyLength), sizeof(expectedChecksum));

    computedChecksum = KeyStore_PSA_computeFletcherChecksum(preProvisionedKey->addr -
                                                                (KEYSTORE_PRE_PROVISIONED_KEY_METADATA_SIZE),
                                                            keySize);

    if (expectedChecksum == computedChecksum)
    {
        status = KEYSTORE_PSA_STATUS_SUCCESS;
    }
    else
    {
        status = KEYSTORE_PSA_STATUS_CORRUPTION_DETECTED;
    }

    return status;
}

/* Copy the pre-provisioned key from persistent memory to the provided data buffer and set the dataLength */
/*
 *  ======== KeyStore_PSA_copyKeyDataFromFlash ========
 */
static int_fast16_t KeyStore_PSA_copyKeyDataFromFlash(KeyStore_PSA_preProvisionedKeyIDs *preProvisionedKey,
                                                      uint8_t *data,
                                                      size_t dataSize,
                                                      size_t *dataLength)
{
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;

    if (dataSize == preProvisionedKey->keyLength)
    {
        memcpy(data, preProvisionedKey->addr, preProvisionedKey->keyLength);
        *dataLength = preProvisionedKey->keyLength;
        status      = KEYSTORE_PSA_STATUS_SUCCESS;
    }
    else
    {
        status = KEYSTORE_PSA_STATUS_BUFFER_TOO_SMALL;
    }
    return status;
}

/*
 * Copy the pre-provisioned key and its length in the provided buffers after verifying fletcher checksum and key
 * properties
 */
/*
 *  ======== KeyStore_PSA_fetchPreProvisionedData ========
 */
static int_fast16_t KeyStore_PSA_fetchPreProvisionedData(KeyStore_PSA_KeyFileId key,
                                                         uint8_t *data,
                                                         size_t dataSize,
                                                         size_t *dataLength,
                                                         bool export,
                                                         KeyStore_PSA_Algorithm alg,
                                                         KeyStore_PSA_KeyUsage usage)
{
    int_fast16_t status = KEYSTORE_PSA_STATUS_DOES_NOT_EXIST;

    if (preProvisionedKeyCount != KEYSTORE_PSA_PREPROVISIONED_KEYS_EMPTY)
    {
        /* Check if Key ID matches pre-provisioned key */
        uint32_t i;
        for (i = 0; i <= preProvisionedKeyCount; i++)
        {
            if ((key.owner == preProvisionedKeyIds[i].id.owner) && (key.key_id == preProvisionedKeyIds[i].id.key_id))
            {
                status = KeyStore_PSA_verifyFletcherChecksum(&preProvisionedKeyIds[i]);
                if (status == KEYSTORE_PSA_STATUS_SUCCESS)
                {
                    if (export)
                    {
                        if (preProvisionedKeyIds[i].export == KEYSTORE_PSA_PRE_PROVISIONED_KEY_EXPORT_FLAG)
                        {
                            status = KeyStore_PSA_copyKeyDataFromFlash(&preProvisionedKeyIds[i],
                                                                       data,
                                                                       dataSize,
                                                                       dataLength);
                        }
                        else
                        {
                            status = KEYSTORE_PSA_STATUS_NOT_PERMITTED;
                        }
                    }
                    else
                    {
                        if (preProvisionedKeyIds->alg == alg && preProvisionedKeyIds->usage == usage)
                        {
                            status = KeyStore_PSA_copyKeyDataFromFlash(&preProvisionedKeyIds[i],
                                                                       data,
                                                                       dataSize,
                                                                       dataLength);
                        }
                        else
                        {
                            status = KEYSTORE_PSA_STATUS_NOT_PERMITTED;
                        }
                    }
                }

                /* Exit for loop since matching key ID & owner was found */
                break;
            }
        }
    }

    return status;
}

/*!
 *  @cond NODOC
 *  @brief Non-public functions required by other drivers
 *
 *  The functions may be required by other drivers and are required to
 *  ensure thread-safe behavior across multiple calls.
 *  @endcond
 */

/*
 *  ======== KeyStore_acquireLock ========
 */
static inline bool KeyStore_acquireLock(void)
{
    SemaphoreP_Status resourceAcquired;

    /* Try and obtain access to the KeyStore module */
    resourceAcquired = SemaphoreP_pend(&KeyStore_semaphoreObject.KeyStore_accessSemaphore, SemaphoreP_WAIT_FOREVER);

    return resourceAcquired == SemaphoreP_OK;
}

/*
 *  ======== KeyStore_releaseLock ========
 */
static inline void KeyStore_releaseLock(void)
{
    SemaphoreP_post(&KeyStore_semaphoreObject.KeyStore_accessSemaphore);
}

/** \defgroup key_management Key management
 * @{
 */

/** Open a handle to an existing persistent key.
 *
 * Open a handle to a persistent key. A key is persistent if it was created
 * with a lifetime other than #PSA_KEY_LIFETIME_VOLATILE. A persistent key
 * always has a nonzero key identifier, set with KeyStore_PSA_setKeyId() when
 * creating the key. Implementations may provide additional pre-provisioned
 * keys that can be opened with KeyStore_openKey(). Such keys have a keyFF
 * identifier in the vendor range, as documented in the description of
 * #psa_key_id_t.
 *
 * The application must eventually close the handle with KeyStore_PSA_purgeKey()
 * or KeyStore_PSA_destroyKey() to release associated resources. If the
 * application dies without calling one of these functions, the
 * implementation should perform the equivalent of a call to
 * KeyStore_PSA_purgeKey().
 *
 * Some implementations permit an application to open the same key multiple
 * times. If this is successful, each call to KeyStore_openKey() will return
 * a different key handle.
 *
 * @note Applications that rely on opening a key multiple times will not be
 * portable to implementations that only permit a single key handle to be
 * opened. See also :ref:\`key-handles\`.
 *
 * @param id            The persistent identifier of the key.
 * @param[out] handle   On success, a handle to the key.
 *
 * @retval #KEYSTORE_PSA_SUCCESS
 *         Success. The application can now use the value of `*handle`
 *         to access the key.
 * @retval #KEYSTORE_PSA_ERROR_INSUFFICIENT_MEMORY
 *         The implementation does not have sufficient resources to open the
 *         key. This can be due to reaching an implementation limit on the
 *         number of open keys, the number of open key handles, or available
 *         memory.
 * @retval #KEYSTORE_PSA_ERROR_DOES_NOT_EXIST
 *         There is no persistent key with key identifier \p id.
 * @retval #KEYSTORE_PSA_ERROR_INVALID_ARGUMENT
 *         \p id is not a valid persistent key identifier.
 * @retval #KEYSTORE_PSA_ERROR_NOT_PERMITTED
 *         The specified key exists, but the application does not have the
 *         permission to access it. Note that this specification does not
 *         define any way to create such a key, but it may be possible
 *         through implementation-specific means.
 * @retval #KEYSTORE_PSA_ERROR_COMMUNICATION_FAILURE
 * @retval #KEYSTORE_PSA_ERROR_CORRUPTION_DETECTED
 * @retval #KEYSTORE_PSA_ERROR_STORAGE_FAILURE
 * @retval #KEYSTORE_PSA_ERROR_BAD_STATE
 *         The library has not been previously initialized by
 * psa_crypto_init(). It is implementation-dependent whether a failure to
 * initialize results in this error code.
 */
static int_fast16_t KeyStore_openKey(KeyStore_PSA_KeyFileId id, KeyStore_PSA_KeyHandle *handle)
{
    int_fast16_t status;
    if (!KeyStore_semaphoreObject.isAcquired)
    {
        if (!KeyStore_acquireLock())
        {
            return KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        }
        KeyStore_semaphoreObject.isAcquired = true;
    }

    status = psa_open_key(id, handle);

    if (!KeyStore_semaphoreObject.isAcquired)
    {
        KeyStore_semaphoreObject.isAcquired = false;
        KeyStore_releaseLock();
    }

    return status;
}

/*
 *  ======== KeyStore_PSA_purgeKey ========
 */
int_fast16_t KeyStore_PSA_purgeKey(KeyStore_PSA_KeyFileId key)
{
    KeyStore_PSA_KeyHandle handle;
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;

    if (!KeyStore_semaphoreObject.isAcquired)
    {
        if (!KeyStore_acquireLock())
        {
            status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
            return status;
        }
    }
    /* Remove references to handle when porting to TF-Mv1.5 */
#if defined(MBEDTLS_PSA_CRYPTO_KEY_FILE_ID_ENCODES_OWNER)
    if (key.key_id > KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
#else
    if (key > KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
#endif
    {
        status = KeyStore_openKey(key, &handle);

        if (status != KEYSTORE_PSA_STATUS_SUCCESS)
        {
            KeyStore_releaseLock();
            return status;
        }
    }
#if defined(MBEDTLS_PSA_CRYPTO_KEY_FILE_ID_ENCODES_OWNER)
    else if (key.key_id > 0 && key.key_id <= KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
    {
        handle = key.key_id;
    }
#else
    else if (key > 0 && key <= KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
    {
        handle = key;
    }
#endif

    /*
     * psa_close_key is similar to psa_purge_key(); introduced in PSA API 1.0.0
     * psa_purge_key() does not delete volatile keys whereas
     * psa_close_key() is equivalent to psa_destroy_key() for volatile keys
     * Only close persistent keys, volatile keys do not have to be closed.
     * Both type of keys will be destroyed after use by the application using
     * KeyStore_PSA_destroyKey()
     */
#if defined(MBEDTLS_PSA_CRYPTO_KEY_FILE_ID_ENCODES_OWNER)
    if (key.key_id > KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
#else
    if (key > KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
#endif
    {
        status = psa_close_key(handle);
    }

    if (!KeyStore_semaphoreObject.isAcquired && (status != KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE))
    {
        KeyStore_releaseLock();
    }
    return status;
}

/*
 *  ======== KeyStore_cleanUp ========
 */
static int_fast16_t KeyStore_cleanUp(KeyStore_PSA_KeyFileId key, int_fast16_t status, KeyStore_PSA_KeyHandle handle)
{
    /*
     * psa_close_key() is equivalent to psa_destroy_key() for volatile keys.
     * Only close persistent keys, volatile keys do not have to be closed.
     * This removes volatile copy of persistent keys from global key slots.
     */
#if defined(MBEDTLS_PSA_CRYPTO_KEY_FILE_ID_ENCODES_OWNER)
    if (key.key_id > KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
#else
    if (key > KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
#endif
    {
        (void)psa_close_key(handle);
    }

    KeyStore_semaphoreObject.isAcquired = false;
    if (status != KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE)
    {
        KeyStore_releaseLock();
    }

    return status;
}

/*
 *  ======== KeyStore_preProvisionedKeyCleanUp ========
 */
static int_fast16_t KeyStore_preProvisionedKeyCleanUp(int_fast16_t status)
{
    KeyStore_semaphoreObject.isAcquired = false;
    if (status != KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE)
    {
        KeyStore_releaseLock();
    }

    return status;
}

/*
 *  ======== KeyStore_PSA_init ========
 */
int_fast16_t KeyStore_PSA_init(void)
{
    psa_status_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;

    if (!isKeyStoreInitialized)
    {
        /*
         * Applications may call psa_crypto_init() function more than once,
         * for example in Key Store and TF-M. Once a call succeeds,
         * subsequent calls are guaranteed to succeed.
         */
        status = psa_crypto_init();

        if (status != PSA_SUCCESS)
        {
            return status;
        }

        status = tfm_its_init();

        if (status != PSA_SUCCESS)
        {
            psa_wipe_all_key_slots();
            return KEYSTORE_PSA_STATUS_GENERIC_ERROR;
        }

        if (!KeyStore_semaphoreObject.isInitialized)
        {
            SemaphoreP_constructBinary(&KeyStore_semaphoreObject.KeyStore_accessSemaphore, 1);
            KeyStore_semaphoreObject.isInitialized = true;
        }

        status = KeyStore_PSA_getPreProvisionedKeyIDs();

        if (status == KEYSTORE_PSA_STATUS_DOES_NOT_EXIST)
        {
            preProvisionedKeyCount = KEYSTORE_PSA_PREPROVISIONED_KEYS_EMPTY;
            status                 = KEYSTORE_PSA_STATUS_SUCCESS;
        }

        isKeyStoreInitialized = true;
    }
    else
    {
        status = KEYSTORE_PSA_STATUS_SUCCESS;
    }

    return status;
}

/*
 *  ======== KeyStore_PSA_getKey ========
 */
int_fast16_t KeyStore_PSA_getKey(KeyStore_PSA_KeyFileId key,
                                 uint8_t *data,
                                 size_t dataSize,
                                 size_t *dataLength,
                                 KeyStore_PSA_Algorithm alg,
                                 KeyStore_PSA_KeyUsage usage)
{
    KeyStore_PSA_KeyHandle handle;
    psa_status_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;
    psa_key_slot_t *slot;
    uint32_t keyID;

    /* Create a copy of the key ID */
    SET_KEY_ID(keyID, key);

    if (!KeyStore_acquireLock())
    {
        status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        return status;
    }
    KeyStore_semaphoreObject.isAcquired = true;

    /* Check if Key is in pre-provisioned key memory before checking in ITS */
    if ((keyID >= KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MIN) && (keyID <= KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MAX))
    {
        status = KeyStore_PSA_fetchPreProvisionedData(key, data, dataSize, dataLength, false, alg, usage);
        return KeyStore_preProvisionedKeyCleanUp(status);
    }

    if (keyID > KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
    {
        status = KeyStore_openKey(key, &handle);

        if (status != KEYSTORE_PSA_STATUS_SUCCESS)
        {
            return KeyStore_cleanUp(key, status, handle);
        }
    }
    else if (keyID > 0 && keyID <= KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
    {
        handle = keyID;
    }

    /*
     * Reject a zero-length output buffer now, since this can never be a
     * valid key representation. This way we know that data must be a valid
     * pointer and we can do things like memset(data, ..., dataSize). */
    if (dataSize == 0)
    {
        status = PSA_ERROR_BUFFER_TOO_SMALL;
        return KeyStore_cleanUp(key, status, handle);
    }

    /*
     * Set the key to empty now, so that even when there are errors, we always
     * set dataLength to a value between 0 and dataSize. On error, setting
     * the key to empty is a good choice because an empty key representation is
     * unlikely to be accepted anywhere. */
    *dataLength = 0;

    /* Fetch key material from key storage. */

    status = psa_get_key_from_slot(handle, &slot, usage, alg);

    if (status != KEYSTORE_PSA_STATUS_SUCCESS)
    {
        return KeyStore_cleanUp(key, status, handle);
    }

    status = psa_internal_export_key(slot, data, dataSize, dataLength, 0);

    return KeyStore_cleanUp(key, status, handle);
}

/*
 *  ======== KeyStore_PSA_importKey ========
 */
int_fast16_t KeyStore_PSA_importKey(KeyStore_PSA_KeyAttributes *attributes,
                                    uint8_t *data,
                                    size_t dataLength,
                                    KeyStore_PSA_KeyFileId *key)
{
    KeyStore_PSA_KeyHandle handle;
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;

    /* Check if the persistent keyID is already used by the pre-provisioned keys or is in range reserved for volatile
     * keys */
    if (attributes->core.lifetime == KEYSTORE_PSA_KEY_LIFETIME_PERSISTENT)
    {
#if defined(MBEDTLS_PSA_CRYPTO_KEY_FILE_ID_ENCODES_OWNER)
        if (((key->key_id >= KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MIN) &&
             (key->key_id <= KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MAX)) ||
            (key->key_id <= KEYSTORE_PSA_MAX_VOLATILE_KEY_ID))
#else
        if (((*key >= KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MIN) && (*key <= KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MAX)) ||
            (*key <= KEYSTORE_PSA_MAX_VOLATILE_KEY_ID))
#endif
        {
            return KEYSTORE_PSA_STATUS_INVALID_KEY_ID;
        }
    }

    if (!KeyStore_acquireLock())
    {
        status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        return status;
    }
    KeyStore_semaphoreObject.isAcquired = true;

    status = psa_import_key(attributes, data, dataLength, &handle);
    /*
     * For persistent keys, the key identifier is the same identifier as the
     * one specified in the key attributes used to create the key.
     * For volatile keys, the key identifier is the handle returned by
     * psa_import_key() in mbedCrypto3.0
     */
    if (status == KEYSTORE_PSA_STATUS_SUCCESS)
    {
        if (attributes->core.lifetime == KEYSTORE_PSA_KEY_LIFETIME_VOLATILE)
        {
#if defined(MBEDTLS_PSA_CRYPTO_KEY_FILE_ID_ENCODES_OWNER)
            key->key_id = handle;
            key->owner  = KEYSTORE_PSA_DEFAULT_OWNER;
#else
            *key = handle;
#endif
        }
    }

    return KeyStore_cleanUp(*key, status, handle);
}

/*
 *  ======== KeyStore_PSA_exportKey ========
 */
int_fast16_t KeyStore_PSA_exportKey(KeyStore_PSA_KeyFileId key, uint8_t *data, size_t dataSize, size_t *dataLength)
{
    KeyStore_PSA_KeyHandle handle;
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;
    uint32_t keyID;

    /* Create a copy of the key ID */
    SET_KEY_ID(keyID, key);

    if (!KeyStore_acquireLock())
    {
        status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        return status;
    }
    KeyStore_semaphoreObject.isAcquired = true;

    /* Check if Key is in pre-provisioned key memory before checking in ITS */
    if ((keyID >= KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MIN) && (keyID <= KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MAX))
    {
        status = KeyStore_PSA_fetchPreProvisionedData(key, data, dataSize, dataLength, true, 0, 0);
        return KeyStore_preProvisionedKeyCleanUp(status);
    }

    if (keyID > KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
    {
        status = KeyStore_openKey(key, &handle);

        if (status != KEYSTORE_PSA_STATUS_SUCCESS)
        {
            return KeyStore_cleanUp(key, status, handle);
        }
    }
    else if (keyID > 0 && keyID <= KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
    {
        handle = keyID;
    }

    status = psa_export_key(handle, data, dataSize, dataLength);

    return KeyStore_cleanUp(key, status, handle);
}

/*
 *  ======== KeyStore_PSA_exportPublicKey ========
 */
int_fast16_t KeyStore_PSA_exportPublicKey(KeyStore_PSA_KeyFileId key,
                                          uint8_t *data,
                                          size_t dataSize,
                                          size_t *dataLength)
{
    KeyStore_PSA_KeyHandle handle;
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;
    uint32_t keyID;

    /* Create a copy of the key ID */
    SET_KEY_ID(keyID, key);

    if (!KeyStore_acquireLock())
    {
        status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        return status;
    }
    KeyStore_semaphoreObject.isAcquired = true;

    /* Check if Key is in pre-provisioned key memory before checking in ITS */
    if ((keyID >= KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MIN) && (keyID <= KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MAX))
    {
        status = KeyStore_PSA_fetchPreProvisionedData(key, data, dataSize, dataLength, true, 0, 0);
        return KeyStore_preProvisionedKeyCleanUp(status);
    }

    if (keyID > KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
    {
        status = KeyStore_openKey(key, &handle);

        if (status != KEYSTORE_PSA_STATUS_SUCCESS)
        {
            return KeyStore_cleanUp(key, status, handle);
        }
    }
    else if (keyID > 0 && keyID <= KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
    {
        handle = keyID;
    }

    status = psa_export_public_key(handle, data, dataSize, dataLength);

    return KeyStore_cleanUp(key, status, handle);
}

/*
 *  ======== KeyStore_PSA_importCertificate ========
 */
int_fast16_t KeyStore_PSA_importCertificate(KeyStore_PSA_KeyAttributes *attributes,
                                            KeyStore_PSA_KeyFileId *key,
                                            uint8_t *data,
                                            size_t dataLength)
{
    KeyStore_PSA_KeyHandle handle;
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;

    uint32_t keyID;
#if defined(MBEDTLS_PSA_CRYPTO_KEY_FILE_ID_ENCODES_OWNER)
    keyID = key->key_id;
#else
    keyID               = *key;
#endif

    /*
     * Only support persistent certificates with TF-M 1.1
     */
    if (attributes->core.lifetime != KEYSTORE_PSA_KEY_LIFETIME_PERSISTENT)
    {
        status = KEYSTORE_PSA_STATUS_INVALID_ARGUMENT;
        return status;
    }

    if (keyID > KEYSTORE_PSA_KEY_ID_WITH_CERTIFICATE_USER_MAX)
    {
        return KEYSTORE_PSA_STATUS_INVALID_KEY_ID;
    }

    /* Check if a certificate already exists with the same ID in pre-provisioned keys storage or is in range reserved
     * for volatile keys */
    if (((keyID >= KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MIN) && (keyID <= KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MAX)) ||
        (keyID <= KEYSTORE_PSA_MAX_VOLATILE_KEY_ID))
    {
        return KEYSTORE_PSA_STATUS_INVALID_KEY_ID;
    }

    /* Compute certificate ID from associated public key ID */
#if defined(MBEDTLS_PSA_CRYPTO_KEY_FILE_ID_ENCODES_OWNER)
    attributes->core.id.key_id = key->key_id | KEYSTORE_PSA_KEY_ID_CERTIFICATE_BIT;
    attributes->core.id.owner  = key->owner;
#else
    attributes->core.id = *key | KEYSTORE_PSA_KEY_ID_CERTIFICATE_BIT;
#endif

    if (!KeyStore_acquireLock())
    {
        status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        return status;
    }
    KeyStore_semaphoreObject.isAcquired = true;

    /*
     * Import certificate into key slot, this will return
     * KEYSTORE_PSA_STATUS_ALREADY_EXISTS if a certificate is associated with
     * the provided keyID.
     */
    status = psa_import_key(attributes, data, dataLength, &handle);

    return KeyStore_cleanUp(*key, status, handle);
}

/*
 *  ======== KeyStore_PSA_exportCertificate ========
 */
int_fast16_t KeyStore_PSA_exportCertificate(KeyStore_PSA_KeyFileId key,
                                            uint8_t *data,
                                            size_t dataSize,
                                            size_t *dataLength)
{
    KeyStore_PSA_KeyHandle handle;
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;
    KeyStore_PSA_KeyFileId certificateID;
    uint32_t keyID;

    /* Create a copy of the key ID */
    SET_KEY_ID(keyID, key);

    if (keyID > KEYSTORE_PSA_KEY_ID_WITH_CERTIFICATE_USER_MAX)
    {
        return KEYSTORE_PSA_STATUS_INVALID_KEY_ID;
    }

    /* Compute certificate ID from associated public key ID */
    GET_KEY_ID(certificateID, keyID | KEYSTORE_PSA_KEY_ID_CERTIFICATE_BIT);

    /* Check if Certificate is in pre-provisioned key memory before checking in ITS */
    if ((keyID >= KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MIN) && (keyID <= KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MAX))
    {
        status = KeyStore_PSA_fetchPreProvisionedData(certificateID, data, dataSize, dataLength, true, 0, 0);
        return KeyStore_preProvisionedKeyCleanUp(status);
    }

    if (!KeyStore_acquireLock())
    {
        status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        return status;
    }
    KeyStore_semaphoreObject.isAcquired = true;

    status = KeyStore_openKey(certificateID, &handle);
    if (status != KEYSTORE_PSA_STATUS_SUCCESS)
    {
        /* Remove references to KEYSTORE_PSA_STATUS_INVALID_KEY_ID when porting to TF-Mv1.5 */
        status = KEYSTORE_PSA_STATUS_INVALID_KEY_ID;
        return KeyStore_cleanUp(key, status, handle);
    }

    status = psa_export_key(handle, data, dataSize, dataLength);

    return KeyStore_cleanUp(key, status, handle);
}

/*
 *  ======== KeyStore_PSA_destroyCertificate ========
 */
int_fast16_t KeyStore_PSA_destroyCertificate(KeyStore_PSA_KeyFileId key)
{
    KeyStore_PSA_KeyHandle handle;
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;
    uint32_t keyID;

    /* Create a copy of the key ID */
    SET_KEY_ID(keyID, key);

    if (keyID >= KEYSTORE_PSA_PRE_PROVISIONED_CERTIFICATE_ID_MIN &&
        keyID <= KEYSTORE_PSA_PRE_PROVISIONED_CERTIFICATE_ID_MAX)
    {
        return KEYSTORE_PSA_STATUS_NOT_SUPPORTED;
    }

    if (!KeyStore_acquireLock())
    {
        status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        return status;
    }
    KeyStore_semaphoreObject.isAcquired = true;

    if (keyID > KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
    {
        status = KeyStore_openKey(key, &handle);

        if (status != KEYSTORE_PSA_STATUS_SUCCESS)
        {
            return KeyStore_cleanUp(key, status, handle);
        }
    }
    else if (keyID > 0 && keyID <= KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
    {
        handle = keyID;
    }

    status = psa_destroy_key(handle);

    return KeyStore_cleanUp(key, status, handle);
}

/*
 *  ======== KeyStore_PSA_destroyKey ========
 */
int_fast16_t KeyStore_PSA_destroyKey(KeyStore_PSA_KeyFileId key)
{
    KeyStore_PSA_KeyHandle handle, certHandle;
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;
    KeyStore_PSA_KeyFileId certificateID;
    uint32_t keyID;

    /* Create a copy of the key ID */
    SET_KEY_ID(keyID, key);

    if ((keyID >= KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MIN) && (keyID <= KEYSTORE_PSA_PRE_PROVISIONED_KEY_ID_MAX))
    {
        return KEYSTORE_PSA_STATUS_NOT_SUPPORTED;
    }

    if (!KeyStore_acquireLock())
    {
        status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        return status;
    }
    KeyStore_semaphoreObject.isAcquired = true;

    /* Check that there is no associated certificate with the given keyID */
#if defined(MBEDTLS_PSA_CRYPTO_KEY_FILE_ID_ENCODES_OWNER)
    certificateID.key_id = key.key_id | KEYSTORE_PSA_KEY_ID_CERTIFICATE_BIT;
    certificateID.owner  = key.owner;
#else
    certificateID       = key | KEYSTORE_PSA_KEY_ID_CERTIFICATE_BIT;
#endif

    status = KeyStore_openKey(certificateID, &certHandle);

    if (status == KEYSTORE_PSA_STATUS_SUCCESS)
    {
        /* Cannot delete keys with associated certificates, the application must first delete the certificate */
        status = KEYSTORE_PSA_STATUS_NOT_PERMITTED;
        return KeyStore_cleanUp(certificateID, status, certHandle);
    }

    if (keyID > KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
    {
        status = KeyStore_openKey(key, &handle);

        if (status != KEYSTORE_PSA_STATUS_SUCCESS)
        {
            return KeyStore_cleanUp(key, status, handle);
        }
    }
    else if (keyID > 0 && keyID <= KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
    {
        handle = keyID;
    }

    status = psa_destroy_key(handle);

    return KeyStore_cleanUp(key, status, handle);
}

/*
 *  ======== KeyStore_PSA_getKeyAttributes ========
 */
int_fast16_t KeyStore_PSA_getKeyAttributes(KeyStore_PSA_KeyFileId key, KeyStore_PSA_KeyAttributes *attributes)

{
    KeyStore_PSA_KeyHandle handle;
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;
    uint32_t keyID;

    /* Create a copy of the key ID */
    SET_KEY_ID(keyID, key);

    if (!KeyStore_acquireLock())
    {
        status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        return status;
    }
    KeyStore_semaphoreObject.isAcquired = true;

    if (keyID > KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
    {
        status = KeyStore_openKey(key, &handle);

        if (status != KEYSTORE_PSA_STATUS_SUCCESS)
        {
            return KeyStore_cleanUp(key, status, handle);
        }
    }
    else if (keyID > 0 && keyID <= KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
    {
        handle = keyID;
    }

    status = psa_get_key_attributes(handle, attributes);

    return KeyStore_cleanUp(key, status, handle);
}

/*
 *  ======== KeyStore_PSA_resetKeyAttributes ========
 */
void KeyStore_PSA_resetKeyAttributes(KeyStore_PSA_KeyAttributes *attributes)
{
    psa_reset_key_attributes(attributes);
}
