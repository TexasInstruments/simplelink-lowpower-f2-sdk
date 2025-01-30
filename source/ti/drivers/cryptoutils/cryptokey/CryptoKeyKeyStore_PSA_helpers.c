/*
 * Copyright (c) 2022-2024, Texas Instruments Incorporated
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

#include <ti/devices/DeviceFamily.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_helpers.h>

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
    #include <third_party/hsmddk/include/Integration/Adapter_ITS/incl/tfm_internal_trusted_storage.h> /* tfm_its_init() */
    #include <third_party/hsmddk/include/Integration/Adapter_PSA/incl/adapter_psa_key_management.h>
    #include <third_party/hsmddk/include/Integration/Adapter_PSA/incl/adapter_psa_system.h>
    #include <third_party/hsmddk/include/Integration/Adapter_PSA/Adapter_mbedTLS/incl/platform.h>
    #include <ti/drivers/cryptoutils/hsm/HSMLPF3.h>
    #include <ti/drivers/cryptoutils/hsm/HSMLPF3Utility.h>
#elif (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4)
    #include <third_party/tfm/secure_fw/partitions/internal_trusted_storage/tfm_internal_trusted_storage.h> /* tfm_its_init() */
    #include <third_party/mbedtls/include/mbedtls/memory_buffer_alloc.h>
    #include <third_party/mbedtls/include/mbedtls/platform.h>
    #include <third_party/tfm/secure_fw/partitions/internal_trusted_storage/tfm_internal_trusted_storage.h> /* tfm_its_init() */

/* Static buffer for alloc/free. The buffer size is allocated based on
 * assumption of 16 largest symmetric keys (32B) and 16 largest asymmetric
 * public keys (133B) that can be supported by KeyStore, with surplus bytes for
 * additional calloc calls within mbedTLS.
 */
uint8_t allocBuffer[3072];

extern psa_status_t psa_get_and_lock_key_slot_with_policy(mbedtls_svc_key_id_t key,
                                                          psa_key_slot_t **p_slot,
                                                          psa_key_usage_t usage,
                                                          psa_algorithm_t alg);
/** @brief Key handle identifier from mbedTLS 'psa_key_handle_t'. */
typedef psa_key_handle_t KeyStore_PSA_KeyHandle;
#else
    #error "Unsupported DeviceFamily_Parent for CryptoKeyKeyStore_PSA_helpers"
#endif

KeyStore_accessSemaphoreObject KeyStore_semaphoreObject = {.isAcquired = false, .isInitialized = false};

/* Flag to prevent multiple initialization of KeyStore driver */
static bool isKeyStoreInitialized = false;

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
#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
bool KeyStore_acquireLock(void)
#else
static inline bool KeyStore_acquireLock(void)
#endif
{
    SemaphoreP_Status resourceAcquired;

    /* Try and obtain access to the KeyStore module */
    resourceAcquired = SemaphoreP_pend(&KeyStore_semaphoreObject.KeyStore_accessSemaphore, SemaphoreP_WAIT_FOREVER);

    return resourceAcquired == SemaphoreP_OK;
}

/*
 *  ======== KeyStore_releaseLock ========
 */
#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
void KeyStore_releaseLock(void)
#else
static inline void KeyStore_releaseLock(void)
#endif
{
    SemaphoreP_post(&KeyStore_semaphoreObject.KeyStore_accessSemaphore);
}

/** \defgroup key_management Key management
 * @{
 */

/*
 *  ======== KeyStore_PSA_purgeKey ========
 */
int_fast16_t KeyStore_PSA_purgeKey(KeyStore_PSA_KeyFileId key)
{
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;

    if (!KeyStore_semaphoreObject.isAcquired)
    {
        if (!KeyStore_acquireLock())
        {
            status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
            return status;
        }
    }

    /*
     * Only purge persistent keys, volatile keys do not have to be purged.
     * Both type of keys will be destroyed after use by the application using
     * KeyStore_PSA_destroyKey()
     */
#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4)
    #if defined(MBEDTLS_PSA_CRYPTO_KEY_ID_ENCODES_OWNER)
    if (key.MBEDTLS_PRIVATE(key_id) > KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
    #else
    if (key > KEYSTORE_PSA_MAX_VOLATILE_KEY_ID)
    #endif
    {
        status = psa_purge_key(key);
    }
#elif (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
    #if defined(MBEDTLS_PSA_CRYPTO_KEY_ID_ENCODES_OWNER)
    if ((key.MBEDTLS_PRIVATE(key_id) >= KEYSTORE_PSA_KEY_ID_USER_MIN) &&
        (key.MBEDTLS_PRIVATE(key_id) <= KEYSTORE_PSA_KEY_ID_USER_MAX))
    #else
    if ((key >= KEYSTORE_PSA_KEY_ID_USER_MIN) && (key <= KEYSTORE_PSA_KEY_ID_USER_MAX))
    #endif
    {
        status = psa_purge_key(key);
    }
    else
    {
        status = KEYSTORE_PSA_STATUS_SUCCESS;
    }
#endif

    if (!KeyStore_semaphoreObject.isAcquired && (status != KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE))
    {
        KeyStore_releaseLock();
    }
    return status;
}

/*
 *  ======== KeyStore_cleanUp ========
 */
static int_fast16_t KeyStore_cleanUp(int_fast16_t status)
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
#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
        int_fast16_t hsmStatus;

        hsmStatus = HSMLPF3_init();

        if (hsmStatus != HSMLPF3_STATUS_SUCCESS)
        {
            return status;
        }

        /* CC27XX requires the HSM lock before calling psa_crypto_init() because
         * that function call requires a token submission.
         */
        HSMLPF3_constructRTOSObjects();

        if (!HSMLPF3_acquireLock(SemaphoreP_NO_WAIT, (uintptr_t)NULL))
        {
            return KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        }

        status = KeyMgmt_psa_crypto_init();

        HSMLPF3_releaseLock();

#else
        mbedtls_memory_buffer_alloc_init(allocBuffer, sizeof(allocBuffer));

        /*
         * Applications may call psa_crypto_init() function more than once,
         * for example in Key Store and TF-M. Once a call succeeds,
         * subsequent calls are guaranteed to succeed.
         */
        status = psa_crypto_init();
#endif
        if (status == KEYSTORE_PSA_STATUS_SUCCESS)
        {
            status = tfm_its_init();

            if (status != PSA_SUCCESS)
            {
#if (DeviceFamily_PARENT != DeviceFamily_PARENT_CC27XX)
                psa_wipe_all_key_slots();
#endif
                return KEYSTORE_PSA_STATUS_GENERIC_ERROR;
            }
        }
        else
        {
            /* Error already set */
        }

        if (status == KEYSTORE_PSA_STATUS_SUCCESS)
        {
            if (!KeyStore_semaphoreObject.isInitialized)
            {
                SemaphoreP_constructBinary(&KeyStore_semaphoreObject.KeyStore_accessSemaphore, 1);
                KeyStore_semaphoreObject.isInitialized = true;
            }

            isKeyStoreInitialized = true;
        }
        else
        {
            /* Error already set */
        }
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
    psa_status_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;
#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
    psa_key_context_t *slot;
    psa_status_t unlockStatus;
#else
    psa_key_slot_t *slot;
#endif

    if (!KeyStore_acquireLock())
    {
        status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        return status;
    }
    KeyStore_semaphoreObject.isAcquired = true;

    /*
     * Reject a zero-length output buffer now, since this can never be a
     * valid key representation. This way we know that data must be a valid
     * pointer and we can do things like memset(data, ..., dataSize). */
    if (dataSize == 0)
    {
        status = PSA_ERROR_BUFFER_TOO_SMALL;
        return KeyStore_cleanUp(status);
    }

    /*
     * Set the key to empty now, so that even when there are errors, we always
     * set dataLength to a value between 0 and dataSize. On error, setting
     * the key to empty is a good choice because an empty key representation is
     * unlikely to be accepted anywhere. */
    *dataLength = 0;

    /* Fetch key slot from key storage. */
#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
    status = psaInt_KeyMgmtGetAndLockKey(key, &slot);
#else
    status                          = psa_get_and_lock_key_slot_with_policy(key, &slot, usage, alg);
#endif /* (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX) */

    if (status != KEYSTORE_PSA_STATUS_SUCCESS)
    {
        /* Ignore return value for decrement of lock counter, the return value from attempting to fetch key is apt for
         * application. Note that psaInt_KeyMgmtGetAndLockKey does not lock the key slot in the case that retrieving it
         * was not successful - this means we do not need to release the key slot in this error condition.
         */
#if (DeviceFamily_PARENT != DeviceFamily_PARENT_CC27XX)
        (void)psa_unlock_key_slot(slot);
#endif
        return KeyStore_cleanUp(status);
    }

    /* Access the key material then decrement lock counter on key slot */
#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
    status = psaInt_KeyMgmtLoadKey(slot, NULL, alg, usage, data, dataSize, (uint32_t *)dataLength);

    unlockStatus = psaInt_KeyMgmtClrKeyInUse(key);

    /* Overwrite the psaInt_KeyMgmtLoadKey() status with the status of unlocking the key
     * entry, if the unlock fails. We shouldn't overwrite status with unlockStatus in the
     * case that the load fails but the unlock succeeds
     */
    if (unlockStatus != KEYSTORE_PSA_STATUS_SUCCESS)
    {
        status = unlockStatus;
    }
#else
    psa_key_attributes_t attributes = {.MBEDTLS_PRIVATE(core) = slot->attr};

    status = psa_export_key_internal(&attributes, slot->key.data, slot->key.bytes, data, dataSize, dataLength);

    status = psa_unlock_key_slot(slot);
#endif /* (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX) */

    return KeyStore_cleanUp(status);
}

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
/*
 *  ======== KeyStore_PSA_getKeyAssetId ========
 */
int_fast16_t KeyStore_PSA_getKeyAssetId(KeyStore_PSA_KeyFileId key,
                                        uint32_t *const pAssetId,
                                        KeyStore_PSA_Algorithm targetAlg,
                                        KeyStore_PSA_KeyUsage targetUsage)
{
    psa_status_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;
    psa_status_t unlockStatus;
    psa_key_context_t *slot;

    if (!KeyStore_acquireLock())
    {
        status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        return status;
    }
    KeyStore_semaphoreObject.isAcquired = true;

    /* Fetch key slot from key storage. */
    status = psaInt_KeyMgmtGetAndLockKey(key, &slot);

    if (status != KEYSTORE_PSA_STATUS_SUCCESS)
    {
        /* Note that psaInt_KeyMgmtGetAndLockKey does not lock the key slot in the case that retrieving it
         * was not successful - this means we do not need to release the key slot in this error condition.
         */
        return KeyStore_cleanUp(status);
    }

    status = psaInt_KeyMgmtLoadKey(slot, pAssetId, targetAlg, targetUsage, NULL, 0, NULL);

    unlockStatus = psaInt_KeyMgmtClrKeyInUse(key);

    /* Overwrite the psaInt_KeyMgmtLoadKey() status with the status of unlocking the key
     * entry, if the unlock fails. We shouldn't overwrite status with unlockStatus in the
     * case that the load fails but the unlock succeeds
     */
    if (unlockStatus != KEYSTORE_PSA_STATUS_SUCCESS)
    {
        status = unlockStatus;
    }

    return KeyStore_cleanUp(status);
}

/*
 *  ======== KeyStore_PSA_retrieveFromKeyStore ========
 */
int_fast16_t KeyStore_PSA_retrieveFromKeyStore(const CryptoKey *key,
                                               uint8_t *keyBuffer,
                                               size_t keyBufferSize,
                                               uint32_t *keyAssetID,
                                               KeyStore_PSA_Algorithm targetAlg,
                                               KeyStore_PSA_KeyUsage targetUsage)

{
    /* This function retrieves the key material to the provided keyBuffer OR provides the key asset ID to the object
     * if the encoding and key location specify to do so
     */
    int_fast16_t status;
    KeyStore_PSA_KeyAttributes attributes = KEYSTORE_PSA_KEY_ATTRIBUTES_INIT;
    KeyStore_PSA_KeyFileId keyID;
    KeyStore_PSA_KeyLifetime lifetime;
    KeyStore_PSA_KeyLocation location;
    size_t keyBits;
    size_t keyLength = 0;

    GET_KEY_ID(keyID, key->u.keyStore.keyID);

    status = KeyStore_PSA_getKeyAttributes(keyID, &attributes);

    if (status == KEYSTORE_PSA_STATUS_SUCCESS)
    {
        keyBits  = KeyStore_PSA_getKeyBits(&attributes);
        lifetime = KeyStore_PSA_getKeyLifetime(&attributes);
        location = KEYSTORE_PSA_KEY_LIFETIME_GET_LOCATION(lifetime);

        if ((key->encoding == CryptoKey_KEYSTORE) && (location != KEYSTORE_PSA_KEY_LOCATION_LOCAL_STORAGE))
        {
            status = KEYSTORE_PSA_STATUS_NOT_PERMITTED;
        }
        else if (location == KEYSTORE_PSA_KEY_LOCATION_LOCAL_STORAGE)
        {
            status = KeyStore_PSA_getKey(keyID, keyBuffer, keyBufferSize, &keyLength, targetAlg, targetUsage);

            /* If the key is an asymmetric key, then KeyStore stores it in the HSM's sub-vector format.
             * The sub-vector format of a key is based on the associated curve bits, which are converted
             * into a word-aligned byte value, plus some extra for sub-vector header words.
             */
            if (status == KEYSTORE_PSA_STATUS_SUCCESS)
            {
                if ((keyBits != 0) && ((keyLength == HSM_ASYM_DATA_SIZE_VWB(keyBits)) ||
                                       (keyLength == (2 * HSM_ASYM_DATA_SIZE_VWB(keyBits)))))
                {
                    /* Nothing to do. The key length returned is expected for the asymmetric key. */
                }
                else if (keyLength == key->u.keyStore.keyLength)
                {
                    /* Nothing to do. The key length returned is expected for the symmetric key. */
                }
                else
                {
                    /* The key length returned is not expected for the given CryptoKey. */
                    status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;
                }
            }
            else
            {
                /* Error already set */
            }
        }
        else if (location == KEYSTORE_PSA_KEY_LOCATION_HSM_ASSET_STORE)
        {
            status = KeyStore_PSA_getKeyAssetId(keyID, keyAssetID, targetAlg, targetUsage);
        }
        else
        {
            status = KEYSTORE_PSA_STATUS_NOT_SUPPORTED;
        }
    }

    return status;
}

/*
 *  ======== KeyStore_PSA_assetPostProcessing ========
 */
int_fast16_t KeyStore_PSA_assetPostProcessing(KeyStore_PSA_KeyFileId key)
{
    psa_status_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;
    psa_status_t unlockStatus;
    psa_key_context_t *slot;
    KeyStore_PSA_KeyAttributes attributes = KEYSTORE_PSA_KEY_ATTRIBUTES_INIT;
    KeyStore_PSA_KeyLifetime lifetime;
    KeyStore_PSA_KeyPersistence persistence;
    KeyStore_PSA_KeyLocation location;

    if (!KeyStore_acquireLock())
    {
        status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        return status;
    }
    KeyStore_semaphoreObject.isAcquired = true;

    status = psa_get_key_attributes(key, &attributes);

    if (status != KEYSTORE_PSA_STATUS_SUCCESS)
    {
        return KeyStore_cleanUp(status);
    }

    lifetime = psa_get_key_lifetime(&attributes);

    location = KEYSTORE_PSA_KEY_LIFETIME_GET_LOCATION(lifetime);

    /* Crypto drivers should only call this function if the location specifies
     * that an asset was created by KeyStore. This is left to the drivers because of
     * the special cases for drivers that use MAC or asymmetric keys.
     */
    if (location != KEYSTORE_PSA_KEY_LOCATION_HSM_ASSET_STORE)
    {
        status = KEYSTORE_PSA_STATUS_INVALID_ARGUMENT;
        return KeyStore_cleanUp(status);
    }

    persistence = KEYSTORE_PSA_KEY_LIFETIME_GET_PERSISTENCE(lifetime);

    /* If the persistence is KEYSTORE_PSA_KEY_PERSISTENCE_HSM_ASSET_STORE,
     * the asset has been designated to remain inside the HSM Asset Store
     * until the application explicitly removes it - therefore, there is
     * nothing to do for the driver's asset post processing.
     */
    if (persistence == KEYSTORE_PSA_KEY_PERSISTENCE_HSM_ASSET_STORE)
    {
        status = KEYSTORE_PSA_STATUS_SUCCESS;
        return KeyStore_cleanUp(status);
    }

    /* Fetch key slot from key storage. */
    status = psaInt_KeyMgmtGetAndLockKey(key, &slot);

    if (status != KEYSTORE_PSA_STATUS_SUCCESS)
    {
        /* Note that psaInt_KeyMgmtGetAndLockKey does not lock the key slot in the case that retrieving it
         * was not successful - this means we do not need to release the key slot in this error condition.
         */
        return KeyStore_cleanUp(status);
    }

    /* Remove the specified key from the asset store, and clear the Asset ID previously
     * stored in the key slot.
     */
    status = psaInt_KeyMgmtReleaseKey(slot);

    unlockStatus = psaInt_KeyMgmtClrKeyInUse(key);

    /* Overwrite the psaInt_KeyMgmtReleaseKey() status with the status of unlocking the key
     * entry, if the unlock fails. We shouldn't overwrite status with unlockStatus in the
     * case that the load fails but the unlock succeeds
     */
    if (unlockStatus != KEYSTORE_PSA_STATUS_SUCCESS)
    {
        status = unlockStatus;
    }

    return KeyStore_cleanUp(status);
}
#endif /* (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX) */

/*
 *  ======== KeyStore_PSA_importKey ========
 */
int_fast16_t KeyStore_PSA_importKey(KeyStore_PSA_KeyAttributes *attributes,
                                    uint8_t *data,
                                    size_t dataLength,
                                    KeyStore_PSA_KeyFileId *key)
{
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;

    if (!KeyStore_acquireLock())
    {
        status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        return status;
    }
    KeyStore_semaphoreObject.isAcquired = true;

    status = psa_import_key(attributes, data, dataLength, key);

    return KeyStore_cleanUp(status);
}

/*
 *  ======== KeyStore_PSA_exportKey ========
 */
int_fast16_t KeyStore_PSA_exportKey(KeyStore_PSA_KeyFileId key, uint8_t *data, size_t dataSize, size_t *dataLength)
{
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;

    if (!KeyStore_acquireLock())
    {
        status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        return status;
    }
    KeyStore_semaphoreObject.isAcquired = true;

    status = psa_export_key(key, data, dataSize, dataLength);

    return KeyStore_cleanUp(status);
}

/*
 *  ======== KeyStore_PSA_exportPublicKey ========
 */
int_fast16_t KeyStore_PSA_exportPublicKey(KeyStore_PSA_KeyFileId key,
                                          uint8_t *data,
                                          size_t dataSize,
                                          size_t *dataLength)
{
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;

    if (!KeyStore_acquireLock())
    {
        status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        return status;
    }
    KeyStore_semaphoreObject.isAcquired = true;

    status = psa_export_public_key(key, data, dataSize, dataLength);

    return KeyStore_cleanUp(status);
}

/*
 *  ======== KeyStore_PSA_destroyKey ========
 */
int_fast16_t KeyStore_PSA_destroyKey(KeyStore_PSA_KeyFileId key)
{
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;
    volatile uint32_t keyID;

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

    status = psa_destroy_key(key);

    return KeyStore_cleanUp(status);
}

/*
 *  ======== KeyStore_PSA_getKeyAttributes ========
 */
int_fast16_t KeyStore_PSA_getKeyAttributes(KeyStore_PSA_KeyFileId key, KeyStore_PSA_KeyAttributes *attributes)

{
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;

    if (!KeyStore_acquireLock())
    {
        status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        return status;
    }
    KeyStore_semaphoreObject.isAcquired = true;

    status = psa_get_key_attributes(key, attributes);

    return KeyStore_cleanUp(status);
}

/*
 *  ======== KeyStore_PSA_resetKeyAttributes ========
 */
void KeyStore_PSA_resetKeyAttributes(KeyStore_PSA_KeyAttributes *attributes)
{
    mbedtls_free(attributes->MBEDTLS_PRIVATE(domain_parameters));
    memset(attributes, 0, sizeof(*attributes));
}

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
/*
 *  ======== KeyStore_PSA_copyKey ========
 */
int_fast16_t KeyStore_PSA_copyKey(KeyStore_PSA_KeyFileId source_key,
                                  KeyStore_PSA_KeyAttributes *attributes,
                                  KeyStore_PSA_KeyFileId *target_key)
{
    int_fast16_t status = KEYSTORE_PSA_STATUS_GENERIC_ERROR;

    if (!KeyStore_acquireLock())
    {
        status = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
        return status;
    }

    KeyStore_semaphoreObject.isAcquired = true;

    status = psa_copy_key(source_key, attributes, target_key);

    return KeyStore_cleanUp(status);
}

#endif /* (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX) */