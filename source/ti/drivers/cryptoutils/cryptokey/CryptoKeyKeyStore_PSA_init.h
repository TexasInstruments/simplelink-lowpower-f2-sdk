
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

#ifndef ti_drivers_CryptoKeyKeyStore_PSA_init__include
#define ti_drivers_CryptoKeyKeyStore_PSA_init__include

#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  @brief Initializes a CryptoKey type
 *
 *  @param [in]     keyHandle       Pointer to a CryptoKey which will be initialized
 *                                  to type CryptoKey_KEYSTORE
 *                                  and ready for use
 *  @param [in]     keyID           Key ID of the key in Key Store
 *
 *  @param [in]     keyLength       Length of keying material in bytes
 *  @param [in]     keyAttributes   Pointer to the attributes for KeyStore key,
 *                                  use NULL if CryptoKey structure will not
 *                                  be used to generate keys
 *
 *  @return Returns a status code from CryptoKey.h
 */
int_fast16_t KeyStore_PSA_initKey(CryptoKey *keyHandle,
                                  KeyStore_PSA_KeyFileId keyID,
                                  size_t keyLength,
                                  const void *keyAttributes);

/*!
 *  @brief Initializes a Blank CryptoKey type
 *
 *  @param [in]     keyHandle       Pointer to a CryptoKey which will be initialized
 *                                  to type CryptoKey_BLANK_KEYSTORE
 *                                  and ready for use
 *  @param [in]     keyID           Key ID of the key in Key Store
 *
 *  @param [in]     keyLength       Length of keying material in bytes
 *  @param [in]     keyAttributes   Pointer to the attributes for KeyStore key,
 *                                  use NULL if CryptoKey structure will not
 *                                  be used to generate keys
 *
 *  @return Returns a status code from CryptoKey.h
 */
int_fast16_t KeyStore_PSA_initBlankKey(CryptoKey *keyHandle,
                                       KeyStore_PSA_KeyFileId keyID,
                                       size_t keyLength,
                                       const void *keyAttributes);

/*!
 *  @brief Initializes a CryptoKey type
 *
 *  @param [in]     keyHandle       Pointer to a CryptoKey which will be initialized
 *                                  to type CryptoKey_KEYSTORE_HSM
 *                                  and ready for use
 *  @param [in]     keyID           Key ID of the key in Key Store
 *  @param [in]     keyLength       Length of keying material in bytes
 *  @param [in]     keyAttributes   Pointer to the attributes for KeyStore key,
 *                                  use NULL if CryptoKey structure will not
 *                                  be used to generate keys
 *
 *  @return Returns a status code from CryptoKey.h
 */
int_fast16_t KeyStore_PSA_initKeyHSM(CryptoKey *keyHandle,
                                     KeyStore_PSA_KeyFileId keyID,
                                     size_t keyLength,
                                     const void *keyAttributes);

/*!
 *  @brief Initializes a Blank CryptoKey type
 *
 *  @param [in]     keyHandle       Pointer to a CryptoKey which will be initialized
 *                                  to type CryptoKey_BLANK_KEYSTORE_HSM
 *                                  and ready for use
 *  @param [in]     keyID           Key ID of the key in Key Store
 *  @param [in]     keyLength       Length of keying material in bytes
 *  @param [in]     keyAttributes   Pointer to the attributes for KeyStore key,
 *                                  use NULL if CryptoKey structure will not
 *                                  be used to generate keys
 *
 *  @return Returns a status code from CryptoKey.h
 */
int_fast16_t KeyStore_PSA_initBlankKeyHSM(CryptoKey *keyHandle,
                                          KeyStore_PSA_KeyFileId keyID,
                                          size_t keyLength,
                                          const void *keyAttributes);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_KeyStore_PSA_init__include */