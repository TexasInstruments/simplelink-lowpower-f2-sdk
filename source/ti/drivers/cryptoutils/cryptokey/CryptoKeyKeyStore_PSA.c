/*
 * Copyright (c) 2022-2024 Texas Instruments Incorporated - http://www.ti.com
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

/* CryptoKey headers */
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_init.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

/*
 *  ======== KeyStore_PSA_initKey ========
 */
int_fast16_t KeyStore_PSA_initKey(CryptoKey *keyHandle,
                                  KeyStore_PSA_KeyFileId keyID,
                                  size_t keyLength,
                                  const void *keyAttributes)
{
    keyHandle->encoding = CryptoKey_KEYSTORE;

    SET_KEY_ID(keyHandle->u.keyStore.keyID, keyID);

    keyHandle->u.keyStore.keyLength = keyLength;

    keyHandle->u.keyStore.keyAttributes = keyAttributes;

    return CryptoKey_STATUS_SUCCESS;
}

/*
 *  ======== KeyStore_PSA_initBlankKey ========
 */
int_fast16_t KeyStore_PSA_initBlankKey(CryptoKey *keyHandle,
                                       KeyStore_PSA_KeyFileId keyID,
                                       size_t keyLength,
                                       const void *keyAttributes)
{
    keyHandle->encoding = CryptoKey_BLANK_KEYSTORE;

    SET_KEY_ID(keyHandle->u.keyStore.keyID, keyID);

    keyHandle->u.keyStore.keyLength = keyLength;

    keyHandle->u.keyStore.keyAttributes = keyAttributes;

    return CryptoKey_STATUS_SUCCESS;
}

/*
 *  ======== KeyStore_PSA_initKeyHSM ========
 */
int_fast16_t KeyStore_PSA_initKeyHSM(CryptoKey *keyHandle,
                                     KeyStore_PSA_KeyFileId keyID,
                                     size_t keyLength,
                                     const void *keyAttributes)
{
    keyHandle->encoding = CryptoKey_KEYSTORE_HSM;

    SET_KEY_ID(keyHandle->u.keyStore.keyID, keyID);

    keyHandle->u.keyStore.keyLength = keyLength;

    keyHandle->u.keyStore.keyAttributes = keyAttributes;

    return CryptoKey_STATUS_SUCCESS;
}

/*
 *  ======== KeyStore_PSA_initBlankKeyHSM ========
 */
int_fast16_t KeyStore_PSA_initBlankKeyHSM(CryptoKey *keyHandle,
                                          KeyStore_PSA_KeyFileId keyID,
                                          size_t keyLength,
                                          const void *keyAttributes)
{
    keyHandle->encoding = CryptoKey_BLANK_KEYSTORE_HSM;

    SET_KEY_ID(keyHandle->u.keyStore.keyID, keyID);

    keyHandle->u.keyStore.keyLength = keyLength;

    keyHandle->u.keyStore.keyAttributes = keyAttributes;

    return CryptoKey_STATUS_SUCCESS;
}

/*
 *  ======== KeyStore_PSA_setKeyId ========
 */
void KeyStore_PSA_setKeyId(KeyStore_PSA_KeyAttributes *attributes, KeyStore_PSA_KeyFileId key)
{
    psa_set_key_id(attributes, key);
}

/*
 *  ======== KeyStore_PSA_setKeyLifetime ========
 */
void KeyStore_PSA_setKeyLifetime(KeyStore_PSA_KeyAttributes *attributes, KeyStore_PSA_KeyLifetime lifetime)
{
    psa_set_key_lifetime(attributes, lifetime);
}

/*
 *  ======== KeyStore_PSA_getKeyId ========
 */
KeyStore_PSA_KeyFileId KeyStore_PSA_getKeyId(KeyStore_PSA_KeyAttributes *attributes)
{
    return psa_get_key_id(attributes);
}

/*
 *  ======== KeyStore_PSA_getKeyLifetime ========
 */
KeyStore_PSA_KeyLifetime KeyStore_PSA_getKeyLifetime(KeyStore_PSA_KeyAttributes *attributes)
{
    return psa_get_key_lifetime(attributes);
}

/*
 *  ======== KeyStore_PSA_setKeyUsageFlags ========
 */
void KeyStore_PSA_setKeyUsageFlags(KeyStore_PSA_KeyAttributes *attributes, KeyStore_PSA_KeyUsage usageFlags)
{
    psa_set_key_usage_flags(attributes, usageFlags);
}

/*
 *  ======== KeyStore_PSA_getKeyUsageFlags ========
 */
KeyStore_PSA_KeyUsage KeyStore_PSA_getKeyUsageFlags(KeyStore_PSA_KeyAttributes *attributes)
{
    return psa_get_key_usage_flags(attributes);
}

/*
 *  ======== KeyStore_PSA_setKeyAlgorithm ========
 */
void KeyStore_PSA_setKeyAlgorithm(KeyStore_PSA_KeyAttributes *attributes, KeyStore_PSA_Algorithm alg)
{
    psa_set_key_algorithm(attributes, alg);
}

/*
 *  ======== KeyStore_PSA_getKeyAlgorithm ========
 */
KeyStore_PSA_Algorithm KeyStore_PSA_getKeyAlgorithm(KeyStore_PSA_KeyAttributes *attributes)
{
    return psa_get_key_algorithm(attributes);
}

/*
 *  ======== KeyStore_PSA_setKeyType ========
 */
void KeyStore_PSA_setKeyType(KeyStore_PSA_KeyAttributes *attributes, KeyStore_PSA_KeyType type)
{
    psa_set_key_type(attributes, type);
}

/*
 *  ======== KeyStore_PSA_setKeyBits ========
 */
void KeyStore_PSA_setKeyBits(KeyStore_PSA_KeyAttributes *attributes, size_t bits)
{
    psa_set_key_bits(attributes, bits);
}

/*
 *  ======== KeyStore_PSA_getKeyType ========
 */
KeyStore_PSA_KeyType KeyStore_PSA_getKeyType(KeyStore_PSA_KeyAttributes *attributes)
{
    return psa_get_key_type(attributes);
}

/*
 *  ======== KeyStore_PSA_getKeyBits ========
 */
size_t KeyStore_PSA_getKeyBits(KeyStore_PSA_KeyAttributes *attributes)
{
    return psa_get_key_bits(attributes);
}
