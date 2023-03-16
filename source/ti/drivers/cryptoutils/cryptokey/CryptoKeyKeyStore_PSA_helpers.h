/*
 * Copyright (c) 2022-2023, Texas Instruments Incorporated
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

#include <third_party/mbedcrypto/library/psa_crypto_core.h>
#include <third_party/mbedcrypto/library/psa_crypto_slot_management.h>
#include <third_party/mbedcrypto/library/psa_crypto_storage.h>

#ifdef __cplusplus
extern "C" {
#endif

/** KeyStore driver semaphore used to synchronize accesses to the keyStore
 *
 * isAcquired: used by openKey() and purgeKey() to check if the KeyStore semaphore is acquired by
 * other KeyStore functions before opening and closing key handles passed to mbedcrypto functions.
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
 * @retval KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE
 * @retval #KEYSTORE_PSA_STATUS_INVALID_KEY_ID
 *         The key identifier does not exist.
 * @retval #KEYSTORE_PSA_STATUS_NOT_PERMITTED
 *         The key does not have matching @p alg and @p usage
 * @retval #KEYSTORE_PSA_STATUS_BAD_STATE
 *         The library has not been previously initialized by
 * KeyStore_PSA_init(). It is implementation-dependent whether a failure to
 * initialize results in this error code.
 */
int_fast16_t KeyStore_PSA_getKey(KeyStore_PSA_KeyFileId key,
                                 uint8_t *data,
                                 size_t dataSize,
                                 size_t *dataLength,
                                 KeyStore_PSA_Algorithm alg,
                                 KeyStore_PSA_KeyUsage usage);

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
