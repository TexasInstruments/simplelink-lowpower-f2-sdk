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

/*!*****************************************************************************
 *  @file       CryptoKeyKeyStore_PSA_helpers.h
 *  @brief      CryptoKeyKeyStore driver header
 *
 *  @anchor ti_drivers_cryptoutils_cryptokey_CryptoKeyKeyStore_PSA_helpers_Overview
 *  # Overview
 *      The CryptoKeyKeyStore driver provides API to initialize keys and get plaintext
 *      keys from KeyStore. This file provides definitions that are only available to the
 *      the secure side, in both TF-M disabled and TF-M enabled environments.
 *
 *******************************************************************************
 */

#ifndef ti_drivers_cryptoutils_cryptokey_CryptoKeyKeyStore_PSA_helpers__include
#define ti_drivers_cryptoutils_cryptokey_CryptoKeyKeyStore_PSA_helpers__include

#include <stddef.h>
#include <stdint.h>

#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA.h>

#include <ti/drivers/dpl/SemaphoreP.h>
#include <ti/devices/DeviceFamily.h>

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4)
    #include <third_party/mbedtls/library/psa_crypto_core.h>
    #include <third_party/mbedtls/library/psa_crypto_slot_management.h>
    #include <third_party/mbedtls/library/psa_crypto_storage.h>
#elif (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
    #include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>
#else
    #error "Unsupported DeviceFamily_Parent for CryptoKeyKeyStore_PSA_helpers"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** KeyStore driver semaphore used to synchronize accesses to the keyStore
 *
 * isAcquired: used by openKey() and purgeKey() to check if the KeyStore semaphore is acquired by
 * other KeyStore functions before opening and closing key handles passed to mbedTLS functions.
 */
typedef struct
{
    SemaphoreP_Struct KeyStore_accessSemaphore;
    bool isInitialized;
    bool isAcquired;
} KeyStore_accessSemaphoreObject;

extern KeyStore_accessSemaphoreObject KeyStore_semaphoreObject;

#define FLETCHER_CHECKSUM_ALGORITHM 32 /* FLETCHER-32 */

/**
 * @brief Get the plaintext key in binary format.
 *
 * This function can only be called on secure side of SPM. It is used by SL crypto drivers
 * to obtain plaintext keys, using keyIDs provided by non-secure application, which will be loaded onto crypto engine
 *
 * Implementations must reject an attempt to import a certificate of size 0.
 *
 * @param [in] key    The key ID for the key in keystore.
 * @param [out] data    On success, the buffer contains the plaintext key
 * @param [in] dataSize  Size of the @p data buffer in bytes. It must be
 * greater than or equal to the plaintext key material
 * @param [out] dataLength Size of the returned key material in bytes.
 * @param [in] alg    Algorithm the key will be used for, it should match the orignal @p alg used to import the key.
 * @param [in] usage   Key usage, it must match the original @p usage used to import the key.
 *
 * @retval #KEYSTORE_PSA_STATUS_SUCCESS
 *         Success.
 *         If the key ID exists, matches the @p alg and @p usage , and the @p dataSize is sufficient
 *         the key is returned in @p data
 * @retval #KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE
 *         If the KeyStore lock cannot be acquired, the KeyStore
 *         module is in use elsewhere.
 * @retval #KEYSTORE_PSA_STATUS_INVALID_KEY_ID
 *         The key identifier does not exist.
 * @retval #KEYSTORE_PSA_STATUS_NOT_PERMITTED
 *         The key does not have matching @p alg and @p usage
 * @retval #KEYSTORE_PSA_STATUS_BAD_STATE
 *         The library has not been previously initialized by
 *         KeyStore_PSA_init(). It is implementation-dependent whether a failure to
 *         initialize results in this error code.
 */
int_fast16_t KeyStore_PSA_getKey(KeyStore_PSA_KeyFileId key,
                                 uint8_t *data,
                                 size_t dataSize,
                                 size_t *dataLength,
                                 KeyStore_PSA_Algorithm alg,
                                 KeyStore_PSA_KeyUsage usage);

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
/**
 * @brief Attempt to acquire lock to access KeyStore.
 * This function is used to synchronize drivers and the application
 * when both are attempting to use KeyStore. For example, if a driver
 * is retrieving key material from a key slot to perform an operation,
 * it must be protected from the application making a call to
 * psa_destroy_key() on that same slot.
 *
 * @retval true     Successfully acquired lock
 * @retval false    Failed to acquire lock
 */
bool KeyStore_acquireLock(void);

/**
 * @brief Release lock to access KeyStore.
 *
 * Once done accessing KeyStore, either the CryptoKeyKeyStore_PSA_helpers APIs
 * or the PSA Crypto APIs should release this lock so that other entities
 * can use KeyStore.
 */
void KeyStore_releaseLock(void);

/**
 * @brief Retrieve the key in either plaintext format or as an Asset ID.
 *
 * This function handles the logic of retrieving a key from CC27XX KeyStore, which
 * depends both on the CryptoKey encoding and the key lifetime/location. If the key
 * location is #KEYSTORE_PSA_KEY_LOCATION_HSM_ASSET_STORE, then the key will be returned
 * via asset ID. If the requested key was not already in the asset store upon request, it
 * will be loaded before the asset ID is returned.
 *
 *  @param [in]  key           Pointer to the CryptoKey object containing the encoding and keyID
 *  @param [out] keyBuffer     Buffer in which to place the key if it is retrievable in plaintext
 *  @param [in]  keyBufferSize Size of the provided buffer
 *  @param [out] keyAssetID    Pointer to keyAssetID output, if the key location is HSM_ASSET_STORE
 *  @param [in]  targetAlg     Desired algorithm to use the key for. Before retrieving the key material,
 *                             it must be verified that it is allowed to be used for a given algorithm.
 *  @param [in]  targetUsage   Desired usage of the resulting key - only necessary for symmetric keys
 *                             that will be returned as HSM assets. Must be one of
 *                             #KEYSTORE_PSA_KEY_USAGE_ENCRYPT or #KEYSTORE_PSA_KEY_USAGE_DECRYPT.
 *
 *  @retval #KEYSTORE_PSA_STATUS_SUCCESS
 *  @retval #KEYSTORE_PSA_STATUS_GENERIC_ERROR
 *          The key length retrieved from KeyStore doesn't match the expected
 *          length. Or, other generic error.
 *  @retval #KEYSTORE_PSA_STATUS_NOT_SUPPORTED
 *          The CryptoKey encoding has an unexpected/unsupported value.
 *  @retval #KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE
 *  @retval #KEYSTORE_PSA_STATUS_INVALID_KEY_ID
 *          The key identifier does not exist.
 *  @retval #KEYSTORE_PSA_STATUS_NOT_PERMITTED
 *          The key does not have matching @p alg and @p usage
 *  @retval #KEYSTORE_PSA_STATUS_BAD_STATE
 *          The library has not been previously initialized by
 *          KeyStore_PSA_init(). It is implementation-dependent whether a failure to
 *          initialize results in this error code.
 */
int_fast16_t KeyStore_PSA_retrieveFromKeyStore(const CryptoKey *key,
                                               uint8_t *keyBuffer,
                                               size_t keyBufferSize,
                                               uint32_t *keyAssetID,
                                               KeyStore_PSA_Algorithm targetAlg,
                                               KeyStore_PSA_KeyUsage targetUsage);

/**
 * @brief Get the asset ID for a given key ID.
 *
 * This function can only be called on secure side of SPM. It is used by SL crypto drivers
 * to obtain assetIDs to refer to keys in the HSM, using keyIDs provided by non-secure application.
 * The asset ID can then be used directly with the HSM for a crypto operation.
 *
 * If the key is not already stored in the HSM's Asset Store, this function will perform that
 * allocation and load before returning the new asset ID.
 *
 * @param [in] key          The key ID for the key in keystore.
 * @param [out] pAssetId    On success, the asset ID for the corresponding key ID
 * @param [in] targetAlg    Desired algorithm to use the key for. Before retrieving the key asset,
 *                          it must be verified that it is allowed to be used for a given algorithm.
 * @param [in] targetUsage  Desired usage of the resulting asset - only used for symmetric keys. Must
 *                          be one of #KEYSTORE_PSA_KEY_USAGE_ENCRYPT or #KEYSTORE_PSA_KEY_USAGE_DECRYPT.
 *
 * @retval #KEYSTORE_PSA_STATUS_SUCCESS
 *         Success.
 *         If the key ID exists, the asset ID
 *         is returned in @p pAssetId
 * @retval #KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE
 *         If the KeyStore lock cannot be acquired, the KeyStore
 *         module is in use elsewhere.
 * @retval #KEYSTORE_PSA_STATUS_INVALID_KEY_ID
 *         The key identifier does not exist.
 * @retval #KEYSTORE_PSA_STATUS_NOT_PERMITTED
 *         The provided pAssetId is NULL
 * @retval #KEYSTORE_PSA_STATUS_BAD_STATE
 *         The library has not been previously initialized by
 *         KeyStore_PSA_init(). It is implementation-dependent whether a failure to
 *         initialize results in this error code.
 */
int_fast16_t KeyStore_PSA_getKeyAssetId(KeyStore_PSA_KeyFileId key,
                                        uint32_t *const pAssetId,
                                        KeyStore_PSA_Algorithm targetAlg,
                                        KeyStore_PSA_KeyUsage targetUsage);

/**
 * @cond NODOC
 *
 * @brief Release the HSM Asset for a given key ID if the key's persistence allows.
 *
 * This function can only be called on secure side of SPM. It is used by SL crypto drivers
 * to clear assets from the HSM Asset Store when necessary. The drivers will do this after each
 * step of an HSM crypto operation utilizing assets only if the key persistence is not
 * #KEYSTORE_PSA_KEY_PERSISTENCE_HSM_ASSET_STORE. Drivers should only call this function when
 * KeyStore is enabled - it is only for freeing assets that the KeyStore module created. Further,
 * it should only be called when the key's location is #KEYSTORE_PSA_KEY_LOCATION_HSM_ASSET_STORE.
 * This is the only key location for which the driver will have used the KeyStore module to create
 * the asset.
 *
 * @param [in] key          The key ID for the key in keystore.
 *
 * @retval #KEYSTORE_PSA_STATUS_SUCCESS
 *         Success. The associated Asset was removed
 *         from HSM Asset Store, it wasn't there
 *         anyway, or the key's persistence required
 *         that it wasn't removed.
 * @retval #KEYSTORE_PSA_STATUS_INVALID_ARGUMENT
 *         The specified key has an invalid location.
 * @retval #KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE
 *         If the KeyStore lock cannot be acquired, the KeyStore
 *         module is in use elsewhere.
 * @retval #KEYSTORE_PSA_STATUS_INVALID_KEY_ID
 *         The key identifier does not exist.
 * @retval #KEYSTORE_PSA_STATUS_HARDWARE_FAILURE
 *         Failure in token submission process.
 * @retval #KEYSTORE_PSA_STATUS_CORRUPTION_DETECTED
 *         The result token yielded an error.
 */
int_fast16_t KeyStore_PSA_assetPostProcessing(KeyStore_PSA_KeyFileId key);
/*! @endcond */
#endif

/**
 * @brief Initialize the Key Store.
 *
 * Applications must call this function before calling any other
 * function in this module. This function will initialize key
 * slot memory and load the key IDs of any preprovisioned keys.
 *
 * @retval #KEYSTORE_PSA_STATUS_SUCCESS
 *         Success.
 * @retval #KEYSTORE_PSA_STATUS_GENERIC_ERROR
 *         tfm_its_init() failed
 * @retval #KEYSTORE_PSA_STATUS_DOES_NOT_EXIST
 *         KeyStore_PSA_getPreProvisionedKeyIDs() failed
 *
 */
int_fast16_t KeyStore_PSA_init(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_cryptoutils_cryptokey_CryptoKeyKeyStore_PSA_helpers__include */
