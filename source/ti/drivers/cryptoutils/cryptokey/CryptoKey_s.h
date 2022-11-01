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

#ifndef ti_drivers_cryptoutils_cyptokey_CryptoKey_s__include
#define ti_drivers_cryptoutils_cyptokey_CryptoKey_s__include

#include <stdint.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_s.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(TFM_PSA_API) /* TFM_PSA_API indicates this is a SPE build */
/*!
 *  @brief  Non-secure Key store CryptoKey datastructure.
 *
 * This structure contains all the information necessary to access keying material stored
 * in a dedicated key store or key database with memory access controls.
 */
typedef struct
{
    uint32_t keyLength;
    uint32_t keyID;
    #if (ENABLE_KEY_STORAGE == 1) || (SPE_ENABLED == 1)
    struct psa_client_key_attributes_s nsAttributes;
    #endif
} CryptoKey_ns_KeyStore;

/*!
 * @brief  Non-secure CryptoKey datastructure.
 *
 * This structure contains a CryptoKey_Encoding and one of
 * - CryptoKey_Plaintext
 * - CryptoKey_KeyStore
 */
typedef struct
{
    CryptoKey_Encoding encoding;
    union
    {
        CryptoKey_Plaintext plaintext;
        CryptoKey_ns_KeyStore nsKeyStore;
    } u;
} CryptoKey_ns;
#endif /* defined(TFM_PSA_API) */

/*!
 * @brief Copy the KeyStore CryptoKey from client side
 *
 * @note The key attributes contain different struct members depending on the context of build (Secure-only, Non-secure,
 *       or Secure). This function creates a secure copy of the CryptoKey structure, which contains the key attributes,
 *       from non-secure client to the secure CryptoKey structure.
 */
void CryptoKey_s_copyCryptoKeyFromClient(CryptoKey *dstRandomKey, CryptoKey *srcRandomKey, int32_t clientId);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_cryptoutils_cyptokey_CryptoKey_s__include */