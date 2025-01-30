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

/*!*****************************************************************************
 *  @file       CryptoKeyKeyStore_PSA_s.h
 *  @brief      Secure Crypto Service
 *
 *  @anchor ti_drivers_cryptoutils_cryptokey_CryptoKeyKeyStore_PSA_s_Overview
 *  # Overview
 *      The Secure KeyStore driver is used to access all KeyStore functions
 *      when using the TF-M.
 *
 *******************************************************************************
 */
#ifndef ti_drivers_cryptoutils_cryptokey_CryptoKeyKeyStore_PSA_s__include
#define ti_drivers_cryptoutils_cryptokey_CryptoKeyKeyStore_PSA_s__include

#include <stdint.h>

#include <ti/drivers/crypto/CryptoCC26X4_s.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA.h>

#include <third_party/tfm/interface/include/psa/crypto.h>
#include <third_party/tfm/interface/include/psa/service.h>

/* For client side key attribute structure */
#include <third_party/tfm/interface/include/psa/crypto_client_struct.h>

/*
 * Crypto Key Store secure message types
 */
#define KEYSTORE_PSA_S_MSG_TYPE_GET_KEY              KEYSTORE_PSA_S_MSG_TYPE(0U)
#define KEYSTORE_PSA_S_MSG_TYPE_GENERATE_KEY         KEYSTORE_PSA_S_MSG_TYPE(1U)
#define KEYSTORE_PSA_S_MSG_TYPE_EXPORT_PUBLIC_KEY    KEYSTORE_PSA_S_MSG_TYPE(2U)
#define KEYSTORE_PSA_S_MSG_TYPE_EXPORT_KEY           KEYSTORE_PSA_S_MSG_TYPE(3U)
#define KEYSTORE_PSA_S_MSG_TYPE_DESTROY_KEY          KEYSTORE_PSA_S_MSG_TYPE(4U)
#define KEYSTORE_PSA_S_MSG_TYPE_IMPORT_KEY           KEYSTORE_PSA_S_MSG_TYPE(5U)
#define KEYSTORE_PSA_S_MSG_TYPE_PURGE_KEY            KEYSTORE_PSA_S_MSG_TYPE(6U)
#define KEYSTORE_PSA_S_MSG_TYPE_GET_KEY_ATTRIBUTES   KEYSTORE_PSA_S_MSG_TYPE(7U)
#define KEYSTORE_PSA_S_MSG_TYPE_RESET_KEY_ATTRIBUTES KEYSTORE_PSA_S_MSG_TYPE(8U)

/*
 * ============ KeyStore driver Secure Message Structs =========
 * These secure message structs correspond to the secure message types defined
 * above. Together, they are used by non-secure client to make PSA calls to the
 * KeyStore secure service. There is a single input vector for the PSA call
 * which is a pointer to secure message struct. If the underlying function
 * has a return value, there is a single output vector which is a pointer to
 * storage for the return value.
 */
typedef struct
{
    uint32_t key;
    uint8_t *data;
    size_t dataSize;
    size_t *dataLength;
    KeyStore_PSA_Algorithm alg;
    KeyStore_PSA_KeyUsage usage;
} KeyStore_s_GetKeyMsg;

typedef struct
{
    struct psa_client_key_attributes_s *attributes;
    KeyStore_PSA_KeyFileId *key;
} KeyStore_s_GenerateKeyMsg;

/* Msg for KeyStore_PSA_exportKey() and KeyStore_PSA_exportPublicKey() */
typedef struct
{
    uint32_t key;
    uint8_t *data;
    size_t dataSize;
    size_t *dataLength;
} KeyStore_s_ExportMsg;

typedef struct
{
    struct psa_client_key_attributes_s *attributes;
    uint8_t *data;
    size_t dataLength;
    uint32_t *key;
} KeyStore_s_ImportKeyMsg;

/* Msg for KeyStore_PSA_destroyKey() and KeyStore_PSA_purgeKey() */
typedef struct
{
    uint32_t key;
} KeyStore_s_DestroyPurgeKeyMsg;

typedef struct
{
    uint32_t key;
    struct psa_client_key_attributes_s *attributes;
} KeyStore_s_GetKeyAttributesMsg;

typedef struct
{
    struct psa_client_key_attributes_s *attributes;
} KeyStore_s_ResetKeyAttributesMsg;

/*!
 *  @brief  Handles PSA messages for KeyStore secure driver
 *
 *  @note   This function should be called by secure partition thread only.
 *
 *  @param [in]  msg     pointer to PSA message
 *
 *  @retval PSA_SUCCESS if successful.
 *  @retval PSA_ERROR_PROGRAMMER_ERROR if any args point to secure addresses.
 */
psa_status_t KeyStore_s_handlePsaMsg(psa_msg_t *msg);

/*!
 *  @brief  Initializes the KeyStore secure driver.
 *
 *  @note   This function should be called by secure partition thread only.
 */
void KeyStore_s_init(void);

/**
 * @brief Gets key attributes from client key attributes.
 *        Follows tfm_crypto_key_attributes_from_client()
 *
 * @param[in]  clientKeyAttr  Client key attributes, address location must be verified to be in non-secure memory by
 *                            calling functions
 * @param[in]  clientId       Partition ID of the calling client
 * @param[out] keyAttributes  Key attributes, no address verification necessary as this is always in secure side
 *
 * @return Always return #KEYSTORE_PSA_STATUS_SUCCESS
 */
psa_status_t KeyStore_s_copyKeyAttributesFromClient(struct psa_client_key_attributes_s *clientKeyAttr,
                                                    int32_t clientId,
                                                    psa_key_attributes_t *keyAttributes);

/**
 * @brief Copies client key ID from non-secure side to secure side
 *
 * @param[out]  keyID         Key ID, no address verification necessary as this is always in secure side
 * @param[in]   clientKeyID   Client key ID, address location must be verified to be in non-secure memory by
 *                            calling functions
 * @param[in]   clientId      Client ID, partition ID from PSA call
 */
void KeyStore_s_copyKeyIDFromClient(KeyStore_PSA_KeyFileId *keyID, int32_t clientId, uint32_t *clientKeyID);
#endif /* ti_drivers_cryptoutils_cryptokey_CryptoKeyKeyStore_PSA_s__include */
