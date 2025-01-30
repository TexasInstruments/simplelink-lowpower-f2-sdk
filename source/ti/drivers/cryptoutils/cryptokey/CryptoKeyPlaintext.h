/*
 * Copyright (c) 2017-2024, Texas Instruments Incorporated
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

/** ============================================================================
 *  @file       CryptoKeyPlaintext.h
 *
 *  @warning    This is a beta API. It may change in future releases.
 *
 *  # Overview #
 *  This file contains the APIs to initialize and access plaintext CryptoKeys.
 *  Plaintext CryptoKeys point to keying material stored in flash or RAM and
 *  are not subject to enforced usage restrictions. That only means that calling
 *  a function that requires an asymmetric public key with a symmetric key will
 *  not return an error. It will likely not yield the desired results.
 *
 *  # Usage #
 *
 *  Plaintext keys are the simplest of the CryptoKeys. All they do is store the
 *  length of and a pointer to the keying material. Their use is hence simple as
 *  well. After calling the initialization function, the CryptoKey may be used in
 *  any of the crypto operation APIs that take a CryptoKey as an input.
 *
 *  @code
 *
 *  uint8_t keyingMaterial[16];
 *  CryptoKey cryptoKey;
 *
 *  // Initialise the CryptoKey
 *  CryptoKeyPlaintext_initKey(&cryptoKey, keyingMaterial, sizeof(keyingMaterial));
 *
 *  // Use the CryptoKey in another crypto operation
 *
 *  @endcode
 *
 */

#ifndef ti_drivers_cryptoutils_cryptokey_CryptoKeyPlaintext__include
#define ti_drivers_cryptoutils_cryptokey_CryptoKeyPlaintext__include

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  @brief Marks a CryptoKey as 'blank'.
 *
 *  The CryptoKey will be unlinked from any previously connected keying material
 *
 *  @param [in]     keyHandle   Pointer to a CryptoKey
 *
 *  @return Returns a status code
 */
int_fast16_t CryptoKeyPlaintext_markAsBlank(CryptoKey *keyHandle);

/*!
 *  @brief Initializes a CryptoKey type
 *
 *  @param [in]     keyHandle   Pointer to a CryptoKey which will be initialized
 *                              to type CryptoKey_PLAINTEXT
 *                              and ready for use
 *  @param [in]     key         Pointer to keying material
 *
 *  @param [in]     keyLength   Length of keying material in bytes
 *
 *  @return Returns a status code from CryptoKey.h
 */
int_fast16_t CryptoKeyPlaintext_initKey(CryptoKey *keyHandle, uint8_t *key, size_t keyLength);

/*!
 *  @brief Initializes an empty plaintext CryptoKey type
 *
 *  @param [in]     keyHandle       Pointer to a CryptoKey which will be
 *                                  initialized to type
 *                                  CryptoKey_BLANK_PLAINTEXT
 *
 *  @param [in]     keyLocation     Pointer to location where plaintext keying
 *                                  material can be stored
 *
 *  @param [in]     keyLength       Length of keying material, in bytes
 *
 *  @return Returns a status code from CryptoKey.h
 */
int_fast16_t CryptoKeyPlaintext_initBlankKey(CryptoKey *keyHandle, uint8_t *keyLocation, size_t keyLength);

/*!
 *  @brief Sets the CryptoKey keyMaterial pointer
 *
 *  Updates the key location for a plaintext CryptoKey.
 *  Does not modify data at the pointer location.
 *
 *  @param [out]     keyHandle   Pointer to a plaintext CryptoKey who's key
 *                               data pointer will be modified
 *
 *  @param [in]     location    Pointer to key data location
 *
 *  @return Returns a status code from CryptoKey.h
 */
int_fast16_t CryptoKeyPlaintext_setKeyLocation(CryptoKey *keyHandle, uint8_t *location);

/*!
 * @brief Gets the CryptoKey keyMaterial pointer
 *
 *  @param [in]     keyHandle   Pointer to an initialized plaintext CryptoKey
 *
 *  @param [out]    location    Pointer to key data location
 *
 *  @return Returns a status code from CryptoKey.h
 */
int_fast16_t CryptoKeyPlaintext_getKeyLocation(CryptoKey *keyHandle, uint8_t **location);

/*!
 *  @brief Gets the length of a plaintext key
 *
 *  @param [in]      keyHandle  Pointer to a plaintext CryptoKey
 *
 *  @param [out]     length     Length of the keying material, in bytes
 *
 *  @return Returns a status code from CryptoKey.h
 */
int_fast16_t CryptoKeyPlaintext_getKeyLength(CryptoKey *keyHandle, size_t *length);
/*!
 *  @brief Sets the length of a plaintext key
 *
 *  @param [out]     keyHandle   Pointer to a CryptoKey
 *
 *  @param [in]      length      Length value in bytes to update
 *                               @c keyHandle with
 *
 *  @return Returns a status code from CryptoKey.h
 */
int_fast16_t CryptoKeyPlaintext_setKeyLength(CryptoKey *keyHandle, size_t length);

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX) || (DeviceFamily_PARENT == DeviceFamily_PARENT_CC35XX)
/*!
 *  @brief Initializes a CryptoKey type with HSM
 *
 *  @param [in]     keyHandle   Pointer to a CryptoKey which will be initialized
 *                              to type CryptoKey_PLAINTEXT_HSM
 *                              and ready for use
 *
 *  @param [in]     key         Pointer to keying material
 *
 *  @param [in]     keyLength   Length of keying material in bytes
 *
 *  @return Returns a status code from CryptoKey.h
 */
int_fast16_t CryptoKeyPlaintextHSM_initKey(CryptoKey *keyHandle, uint8_t *key, size_t keyLength);

/*!
 *  @brief Initializes an empty plaintext CryptoKey type with HSM
 *
 *  @param [in]     keyHandle   Pointer to a CryptoKey which will be initialized
 *                              to type CryptoKey_BLANK_PLAINTEXT_HSM
 *                              and ready for use
 *
 *  @param [in]     key         Pointer to keying material
 *
 *  @param [in]     keyLength   Length of keying material in bytes
 *
 *  @return Returns a status code from CryptoKey.h
 */
int_fast16_t CryptoKeyPlaintextHSM_initBlankKey(CryptoKey *keyHandle, uint8_t *key, size_t keyLength);
#endif

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_cryptoutils_cryptokey_CryptoKeyPlaintext__include */
