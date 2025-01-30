/*
 * Copyright (c) 2022-2024, Texas Instruments Incorporated - http://www.ti.com
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

#include <ti/drivers/crypto/CryptoCC26X4_ns.h>

/* CryptoKey headers */
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_s.h>

/* PSA headers from TF-M interface */
#include <third_party/tfm/interface/include/psa/crypto.h>
#include <third_party/tfm/interface/include/psa/client.h>

static struct psa_invec invecs[1];
static struct psa_outvec outvecs[1];

/*
 *  ======== KeyStore_PSA_exportCommon ========
 */
static int_fast16_t KeyStore_PSA_exportCommon(KeyStore_PSA_KeyFileId key,
                                              uint8_t *data,
                                              size_t dataSize,
                                              size_t *dataLength,
                                              int32_t type)
{
    KeyStore_s_ExportMsg exportCommonMsg;
    int_fast16_t ret = KEYSTORE_PSA_STATUS_GENERIC_ERROR;

    exportCommonMsg.key        = key;
    exportCommonMsg.data       = data;
    exportCommonMsg.dataSize   = dataSize;
    exportCommonMsg.dataLength = dataLength;

    invecs[0].base = &exportCommonMsg;
    invecs[0].len  = sizeof(exportCommonMsg);

    outvecs[0].base = &ret;
    outvecs[0].len  = sizeof(ret);

    /*
     * PSA call to secure driver:
     * if statement returns from secure driver can be ignored by the non-secure driver,
     * the secure KeyStore driver returns are handled by the application using outvecs (ret)
     */
    (void)CryptoPSACC26X4_call(type, invecs, outvecs);

    return ret;
}

/*
 *  ======== KeyStore_PSA_exportPublicKey ========
 */
int_fast16_t KeyStore_PSA_exportPublicKey(KeyStore_PSA_KeyFileId key,
                                          uint8_t *data,
                                          size_t dataSize,
                                          size_t *dataLength)
{
    return KeyStore_PSA_exportCommon(key, data, dataSize, dataLength, KEYSTORE_PSA_S_MSG_TYPE_EXPORT_PUBLIC_KEY);
}

/*
 *  ======== KeyStore_PSA_exportKey ========
 */
int_fast16_t KeyStore_PSA_exportKey(KeyStore_PSA_KeyFileId key, uint8_t *data, size_t dataSize, size_t *dataLength)
{
    return KeyStore_PSA_exportCommon(key, data, dataSize, dataLength, KEYSTORE_PSA_S_MSG_TYPE_EXPORT_KEY);
}

/*
 *  ======== KeyStore_PSA_destroyCommon ========
 */
static int_fast16_t KeyStore_PSA_destroyCommon(KeyStore_PSA_KeyFileId key, int32_t type)
{
    KeyStore_s_DestroyPurgeKeyMsg destroyCommonMsg;
    int_fast16_t ret = KEYSTORE_PSA_STATUS_GENERIC_ERROR;

    destroyCommonMsg.key = key;

    invecs[0].base = &destroyCommonMsg;
    invecs[0].len  = sizeof(destroyCommonMsg);

    outvecs[0].base = &ret;
    outvecs[0].len  = sizeof(ret);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since ret (in outvecs) is initialized to KEYSTORE_PSA_STATUS_GENERIC_ERROR and
     * will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(type, invecs, outvecs);

    return ret;
}

/*
 *  ======== KeyStore_PSA_destroyKey ========
 */
int_fast16_t KeyStore_PSA_destroyKey(KeyStore_PSA_KeyFileId key)
{
    return KeyStore_PSA_destroyCommon(key, KEYSTORE_PSA_S_MSG_TYPE_DESTROY_KEY);
}

/*
 *  ======== KeyStore_PSA_importKey ========
 */
int_fast16_t KeyStore_PSA_importKey(KeyStore_PSA_KeyAttributes *attributes,
                                    uint8_t *data,
                                    size_t dataLength,
                                    KeyStore_PSA_KeyFileId *key)
{
    KeyStore_s_ImportKeyMsg importKeyMsg;
    int_fast16_t ret = KEYSTORE_PSA_STATUS_GENERIC_ERROR;

    importKeyMsg.attributes = &attributes->client;
    importKeyMsg.key        = key;
    importKeyMsg.data       = data;
    importKeyMsg.dataLength = dataLength;

    invecs[0].base = &importKeyMsg;
    invecs[0].len  = sizeof(importKeyMsg);

    outvecs[0].base = &ret;
    outvecs[0].len  = sizeof(ret);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since ret (in outvecs) is initialized to KEYSTORE_PSA_STATUS_GENERIC_ERROR and
     * will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(KEYSTORE_PSA_S_MSG_TYPE_IMPORT_KEY, invecs, outvecs);

    return ret;
}

/*
 *  ======== KeyStore_PSA_purgeKey ========
 */
int_fast16_t KeyStore_PSA_purgeKey(KeyStore_PSA_KeyFileId key)
{
    return KeyStore_PSA_destroyCommon(key, KEYSTORE_PSA_S_MSG_TYPE_PURGE_KEY);
}

/*
 *  ======== KeyStore_PSA_getKeyAttributes ========
 */
int_fast16_t KeyStore_PSA_getKeyAttributes(KeyStore_PSA_KeyFileId key, KeyStore_PSA_KeyAttributes *attributes)
{
    KeyStore_s_GetKeyAttributesMsg getKeyAttributesMsg;
    int_fast16_t ret = KEYSTORE_PSA_STATUS_GENERIC_ERROR;

    getKeyAttributesMsg.attributes = &attributes->client;
    getKeyAttributesMsg.key        = key;

    invecs[0].base = &getKeyAttributesMsg;
    invecs[0].len  = sizeof(getKeyAttributesMsg);

    outvecs[0].base = &ret;
    outvecs[0].len  = sizeof(ret);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since ret (in outvecs) is initialized to KEYSTORE_PSA_STATUS_GENERIC_ERROR and
     * will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(KEYSTORE_PSA_S_MSG_TYPE_GET_KEY_ATTRIBUTES, invecs, outvecs);

    return ret;
}

/*
 *  ======== KeyStore_PSA_resetKeyAttributes ========
 */
void KeyStore_PSA_resetKeyAttributes(KeyStore_PSA_KeyAttributes *attributes)
{
    KeyStore_s_ResetKeyAttributesMsg resetKeyAttributeMsg;

    resetKeyAttributeMsg.attributes = &attributes->client;

    invecs[0].base = &resetKeyAttributeMsg;
    invecs[0].len  = sizeof(resetKeyAttributeMsg);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since ret (in outvecs) is initialized to KEYSTORE_PSA_STATUS_GENERIC_ERROR and
     * will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(KEYSTORE_PSA_S_MSG_TYPE_RESET_KEY_ATTRIBUTES, invecs, outvecs);
}
