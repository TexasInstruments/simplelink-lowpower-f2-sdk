/******************************************************************************

 @file  crypto.c

 @brief Describe the purpose and contents of the file.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************

 Copyright (c) 2019-2022, Texas Instruments Incorporated

 All rights reserved not granted herein.
 Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free,
 non-exclusive license under copyrights and patents it now or hereafter
 owns or controls to make, have made, use, import, offer to sell and sell
 ("Utilize") this software subject to the terms herein. With respect to the
 foregoing patent license, such license is granted solely to the extent that
 any such patent is necessary to Utilize the software alone. The patent
 license shall not apply to any combinations which include this software,
 other than combinations with devices manufactured by or for TI ("TI
 Devices"). No hardware patent is licensed hereunder.

 Redistributions must preserve existing copyright notices and reproduce
 this license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution.

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

   * No reverse engineering, decompilation, or disassembly of this software
     is permitted with respect to any software provided in binary form.
   * Any redistribution and use are licensed by TI for use only with TI Devices.
   * Nothing shall obligate TI to provide you with source code for the software
     licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution
 of the source code are permitted provided that the following conditions are
 met:

   * Any redistribution and use of the source code, including any resulting
     derivative works, are licensed by TI for use only with TI Devices.
   * Any redistribution and use of any object code compiled from the source
     code and any resulting derivative works, are licensed by TI for use
     only with TI Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this
 software without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************


 *****************************************************************************/

/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <sid_pal_crypto_ifc.h>
#include <sid_pal_assert_ifc.h>

#include <ti/drivers/TRNG.h>
#include <ti/drivers/SHA2.h>

#include <ti/drivers/AESCMAC.h>
#include <ti/drivers/EDDSA.h>
#include <ti/drivers/AESCTR.h>
#include <ti/drivers/AESGCM.h>
#include <ti/drivers/ECDH.h>
#include <ti/drivers/ECDSA.h>
#include <ti/drivers/cryptoutils/ecc/ECCParams.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
#include <ti/drivers/cryptoutils/utils/CryptoUtils.h>

#include "ti_cc13x2_crypto_config.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define CC13X2_EDDSA_MULTIPLE_KEYS      1
#define BIT_SIZE_PER_BYTE               8
#define AES_KEY_SZ                      16

enum {
    CC13X2_TRNG = 0,
    CC13X2_CRYPTO_SHA2,
    CC13X2_CRYPTO_AES_CMAC,
    CC13X2_CRYPTO_AES_CTR,
    CC13X2_CRYPTO_AES_GCM,
    CC13X2_CRYPTO_EDDSA,
    CC13X2_CRYPTO_ECDSA,
    CC13X2_CRYPTO_ECDH,
    CC13X2_CRYPTO_MAX_COUNTS
} cc13x2_crypto_t;

static void *crypto_handle[CC13X2_CRYPTO_MAX_COUNTS];

static bool crypto_init_done = false;

static uint8_t eddsa_public_key[32];

static inline void cc13x2_crypto_prune(uint8_t *prk)
{
    // RFC 7748
    prk[31] &= 0xF8;        // clear bit 0, 1, 2
    prk[0] &= 0x7F;         // clear bit 255
    prk[0] |= 0x40;         // set bit 254
}

static inline void cc13x2_reverse_bytes_in_place(uint8_t *start, size_t length)
{
    uint8_t tmp;
    uint8_t *end = start + length - 1;

    while (start < end) {
        tmp = *start;
        *start++ = *end;
        *end-- = tmp;
    }
}

static inline uint8_t * cc13x2_get_eddsa_public_key(void)
{
    return eddsa_public_key;
}

static inline sid_error_t cc13x2_regen_eddsa_public_key(uint8_t *prk, size_t prk_size, uint8_t *puk, size_t puk_size)
{
    CryptoKey private_key;
    CryptoKey public_key;
    int_fast16_t result;
    EDDSA_OperationGeneratePublicKey eddsaOperationGeneratePublicKey;

    CryptoKeyPlaintext_initKey(&private_key, prk, prk_size);

    do {
        CryptoKeyPlaintext_initBlankKey(&public_key, puk, puk_size);

        EDDSA_OperationGeneratePublicKey_init(&eddsaOperationGeneratePublicKey);
        eddsaOperationGeneratePublicKey.curve                 = (ECCParams_CurveParams *)&ECCParams_Ed25519;
        eddsaOperationGeneratePublicKey.myPrivateKey          = &private_key;
        eddsaOperationGeneratePublicKey.myPublicKey           = &public_key;

        result = EDDSA_generatePublicKey(crypto_handle[CC13X2_CRYPTO_EDDSA], &eddsaOperationGeneratePublicKey);
    } while(result == EDDSA_STATUS_INVALID_PRIVATE_KEY_SIZE || result == EDDSA_STATUS_POINT_AT_INFINITY);


    if (result != EDDSA_STATUS_SUCCESS) {
        return SID_ERROR_GENERIC;
    }

    return SID_ERROR_NONE;
}

static sid_error_t cc13x2_crypto_init(void)
{
    /* CC13X2 TRNG */
    {
        TRNG_init();
        TRNG_Params params;
        TRNG_Params_init(&params);
        params.returnBehavior = TRNG_RETURN_BEHAVIOR_BLOCKING;
        crypto_handle[CC13X2_TRNG] = TRNG_open(CONFIG_CC13X2_PAL_CRYPTO_TRNG_INSTANCE_ID, &params);
        SID_PAL_ASSERT(crypto_handle[CC13X2_TRNG]);
    }

    /* CC13X2_CRYPTO_SHA2 */
    {
        SHA2_init();
        SHA2_Params params;
        SHA2_Params_init(&params);
        params.returnBehavior = SHA2_RETURN_BEHAVIOR_POLLING;
        crypto_handle[CC13X2_CRYPTO_SHA2] = SHA2_open(CONFIG_CC13X2_PAL_CRYPTO_SHA2_INSTANCE_ID, &params);
        SID_PAL_ASSERT(crypto_handle[CC13X2_CRYPTO_SHA2]);
    }

    /* CC13X2_CRYPTO_AES_CMAC */
    {
        AESCMAC_init();
        AESCMAC_Params params;
        AESCMAC_Params_init(&params);
        params.returnBehavior = AESCMAC_RETURN_BEHAVIOR_POLLING;
        crypto_handle[CC13X2_CRYPTO_AES_CMAC] = AESCMAC_open(CONFIG_CC13X2_PAL_CRYPTO_AESCMAC_INSTANCE_ID, &params);
        SID_PAL_ASSERT(crypto_handle[CC13X2_CRYPTO_AES_CMAC]);
    }

    /* CC13X2_CRYPTO_AES_CTR */
    {
        AESCTR_init();
        AESCTR_Params params;
        AESCTR_Params_init(&params);
        params.returnBehavior = AESCTR_RETURN_BEHAVIOR_POLLING;
        crypto_handle[CC13X2_CRYPTO_AES_CTR] = AESCTR_open(CONFIG_CC13X2_PAL_CRYPTO_AESCTR_INSTANCE_ID, &params);
        SID_PAL_ASSERT(crypto_handle[CC13X2_CRYPTO_AES_CTR]);
    }

    /* CC13X2_CRYPTO_AES_GCM */
    {
        AESGCM_init();
        AESGCM_Params params;
        AESGCM_Params_init(&params);
        params.returnBehavior = AESGCM_RETURN_BEHAVIOR_POLLING;
        crypto_handle[CC13X2_CRYPTO_AES_GCM] = AESGCM_open(CONFIG_CC13X2_PAL_CRYPTO_AESGCM_INSTANCE_ID, &params);
        SID_PAL_ASSERT(crypto_handle[CC13X2_CRYPTO_AES_GCM]);
    }

    /* CC13X2_CRYPTO_EDDSA */
    {
        EDDSA_init();
        EDDSA_Params params;
        EDDSA_Params_init(&params);
        params.returnBehavior = EDDSA_RETURN_BEHAVIOR_POLLING;
        crypto_handle[CC13X2_CRYPTO_EDDSA] = EDDSA_open(CONFIG_CC13X2_PAL_CRYPTO_EDDSA_INSTANCE_ID, &params);
        SID_PAL_ASSERT(crypto_handle[CC13X2_CRYPTO_EDDSA]);
    }

    /* CC13X2_CRYPTO_ECDSA */
    {
        ECDSA_init();
        ECDSA_Params params;
        ECDSA_Params_init(&params);
        params.returnBehavior = ECDSA_RETURN_BEHAVIOR_POLLING;
        crypto_handle[CC13X2_CRYPTO_ECDSA] = ECDSA_open(CONFIG_CC13X2_PAL_CRYPTO_ECDSA_INSTANCE_ID, &params);
        SID_PAL_ASSERT(crypto_handle[CC13X2_CRYPTO_ECDSA]);
    }

    /* CC13X2_CRYPTO_ECDH */
    {
        ECDH_init();
        ECDH_Params params;
        ECDH_Params_init(&params);
        params.returnBehavior = ECDH_RETURN_BEHAVIOR_POLLING;
        crypto_handle[CC13X2_CRYPTO_ECDH] = ECDH_open(CONFIG_CC13X2_PAL_CRYPTO_ECDH_INSTANCE_ID, &params);
        SID_PAL_ASSERT(crypto_handle[CC13X2_CRYPTO_ECDH]);
    }

    return SID_ERROR_NONE;
}

static sid_error_t cc13x2_crypto_deinit(void)
{
    TRNG_close(crypto_handle[CC13X2_TRNG]);
    SHA2_close(crypto_handle[CC13X2_CRYPTO_SHA2]);
    AESCMAC_close(crypto_handle[CC13X2_CRYPTO_AES_CMAC]);
    AESCTR_close(crypto_handle[CC13X2_CRYPTO_AES_CTR]);
    AESGCM_close(crypto_handle[CC13X2_CRYPTO_AES_GCM]);
    EDDSA_close(crypto_handle[CC13X2_CRYPTO_EDDSA]);
    ECDSA_close(crypto_handle[CC13X2_CRYPTO_ECDSA]);
    ECDH_close(crypto_handle[CC13X2_CRYPTO_ECDH]);

    return SID_ERROR_NONE;
}

sid_error_t cc13x2_crypto_rand(uint8_t* rand, size_t size)
{
    sid_error_t error = SID_ERROR_NONE;
    CryptoKey entropy_key;
    int_fast16_t result;

    CryptoKeyPlaintext_initBlankKey(&entropy_key, rand, size);

    result = TRNG_generateEntropy(crypto_handle[CC13X2_TRNG], &entropy_key);

    if (result != TRNG_STATUS_SUCCESS) {
        error = SID_ERROR_GENERIC;
    }

    return error;
}

static sid_error_t cc13x2_crypto_hash(sid_pal_hash_params_t* params)
{
    sid_error_t error = SID_ERROR_NONE;
    SHA2_HashType type = SHA2_HASH_TYPE_256;
    int_fast16_t result;

    switch(params->algo) {
    case SID_PAL_HASH_SHA256:
        type = SHA2_HASH_TYPE_256;
        break;
    case SID_PAL_HASH_SHA512:
        type = SHA2_HASH_TYPE_512;
        break;
    default:
        return SID_ERROR_INVALID_ARGS;
        break;
    }

    result = SHA2_setHashType(crypto_handle[CC13X2_CRYPTO_SHA2], type);
    if (result != SHA2_STATUS_SUCCESS) {
        return SID_ERROR_GENERIC;
    }

    result = SHA2_hashData(crypto_handle[CC13X2_CRYPTO_SHA2], params->data, params->data_size, params->digest);
    if (result != SHA2_STATUS_SUCCESS) {
        error = SID_ERROR_GENERIC;
    }

    return error;
}

static sid_error_t cc13x2_crypto_hmac(sid_pal_hmac_params_t* params)
{
    sid_error_t error = SID_ERROR_NONE;
    SHA2_HashType type = SHA2_HASH_TYPE_256;
    CryptoKey crypto_key;
    int_fast16_t result;

    switch(params->algo) {
    case SID_PAL_HASH_SHA256:
        type = SHA2_HASH_TYPE_256;
        break;
    case SID_PAL_HASH_SHA512:
        type = SHA2_HASH_TYPE_512;
        break;
    default:
        return SID_ERROR_INVALID_ARGS;
        break;
    }

    CryptoKeyPlaintext_initKey(&crypto_key, (uint8_t *)params->key, (size_t)params->key_size);

    result = SHA2_setHashType(crypto_handle[CC13X2_CRYPTO_SHA2], type);
    if (result != SHA2_STATUS_SUCCESS) {
        return SID_ERROR_GENERIC;
    }

    result = SHA2_hmac(crypto_handle[CC13X2_CRYPTO_SHA2], &crypto_key, params->data, params->data_size, params->digest);

    if (result != SHA2_STATUS_SUCCESS) {
        error = SID_ERROR_GENERIC;
    }

    return error;
}

static sid_error_t cc13x2_crypto_aes_crypt(sid_pal_aes_params_t *params)
{
    sid_error_t error = SID_ERROR_NONE;
    CryptoKey crypto_key;
    int_fast16_t result = 0;

    switch (params->mode) {
        case SID_PAL_CRYPTO_ENCRYPT:
        case SID_PAL_CRYPTO_DECRYPT:
            if (params->in_size > params->out_size) {
                return SID_ERROR_INVALID_ARGS;
            }
            break;
        case SID_PAL_CRYPTO_MAC_CALCULATE:
            if (params->out_size != AES_KEY_SZ) {
                return SID_ERROR_INVALID_ARGS;
            }
            break;
        default:
            return SID_ERROR_INVALID_ARGS;
    }

    CryptoKeyPlaintext_initKey(&crypto_key, (uint8_t *)params->key, (size_t)params->key_size / BIT_SIZE_PER_BYTE);
    switch(params->algo) {
        case SID_PAL_AES_CMAC_128: {
            AESCMAC_Operation operation;
            AESCMAC_Operation_init(&operation);
            operation.input             = (uint8_t *)params->in;
            operation.inputLength       = params->in_size;
            operation.mac               = params->out;
            operation.macLength         = params->out_size;
            if (params->mode == SID_PAL_CRYPTO_MAC_CALCULATE) {
                result = AESCMAC_oneStepSign(crypto_handle[CC13X2_CRYPTO_AES_CMAC], &operation, &crypto_key);
            }
            if (result != AESCMAC_STATUS_SUCCESS) {
                error = SID_ERROR_GENERIC;
            }
            break;
        }
        case SID_PAL_AES_CTR_128: {
            AESCTR_Operation operation;
            AESCTR_Operation_init(&operation);
            operation.key               = &crypto_key;
            operation.input             = (uint8_t *)params->in;
            operation.output            = params->out;
            operation.inputLength       = params->in_size;
            operation.initialCounter    = params->iv;
            if (params->mode == SID_PAL_CRYPTO_ENCRYPT) {
                result = AESCTR_oneStepEncrypt(crypto_handle[CC13X2_CRYPTO_AES_CTR], &operation);
            }
            else {
                result = AESCTR_oneStepDecrypt(crypto_handle[CC13X2_CRYPTO_AES_CTR], &operation);
            }
            if (result != AESCTR_STATUS_SUCCESS) {
                error = SID_ERROR_GENERIC;
            }
            break;
        default:
            return SID_ERROR_INVALID_ARGS;
            break;
        }
    }

    return error;
}

static sid_error_t cc13x2_crypto_aead_crypt(sid_pal_aead_params_t *params)
{
    sid_error_t error = SID_ERROR_NONE;
    CryptoKey crypto_key;
    int_fast16_t result;

    CryptoKeyPlaintext_initKey(&crypto_key, (uint8_t *)params->key, (size_t)params->key_size / BIT_SIZE_PER_BYTE);

    switch(params->algo) {
        case SID_PAL_AEAD_GCM_128: {
            AESGCM_Operation operation;
            AESGCM_Operation_init(&operation);
            operation.key               = &crypto_key;
            operation.aad               = (uint8_t *)params->aad;
            operation.aadLength         = params->aad_size;
            operation.input             = (uint8_t *)params->in;
            operation.output            = params->out;
            operation.inputLength       = params->in_size;
            operation.iv                = (uint8_t *)params->iv;
            operation.mac               = params->mac;
            operation.macLength         = params->mac_size;
            if (params->mode == SID_PAL_CRYPTO_ENCRYPT) {
                result = AESGCM_oneStepEncrypt(crypto_handle[CC13X2_CRYPTO_AES_GCM], &operation);
            }
            else {
                result = AESGCM_oneStepDecrypt(crypto_handle[CC13X2_CRYPTO_AES_GCM], &operation);
            }
            if (result != AESGCM_STATUS_SUCCESS) {
                error = SID_ERROR_GENERIC;
            }
            break;
        }
        case SID_PAL_AEAD_CCM_128:
            error = SID_ERROR_NOSUPPORT;
            break;
        case SID_PAL_AEAD_CCM_STAR_128:
            error = SID_ERROR_NOSUPPORT;
            break;
        default:
            error = SID_ERROR_INVALID_ARGS;
            break;
    }

    return error;
}

static sid_error_t do_ecdsa_secp256r1(const CryptoKey *key, const sid_pal_dsa_params_t *params)
{
    uint8_t hash_digest[SHA2_DIGEST_LENGTH_BYTES_256];

    sid_pal_hash_params_t hash_params = {
        .algo = SID_PAL_HASH_SHA256,
        .data = params->in,
        .data_size = params->in_size,
        .digest = hash_digest,
        .digest_size = sizeof(hash_digest)
    };

    sid_error_t ret = cc13x2_crypto_hash(&hash_params);
    if (ret != SID_ERROR_NONE) {
        return ret;
    }

    if (params->mode == SID_PAL_CRYPTO_SIGN) {
        ECDSA_OperationSign operationSign;
        ECDSA_OperationSign_init(&operationSign);
        operationSign.curve             = &ECCParams_NISTP256;
        operationSign.myPrivateKey      = key;
        operationSign.hash              = hash_digest;
        operationSign.r                 = params->signature;
        operationSign.s                 = params->signature + (params->sig_size / 2);

        // Generate the signature
        int_fast16_t result = ECDSA_sign(crypto_handle[CC13X2_CRYPTO_ECDSA], &operationSign);
        if (result != ECDSA_STATUS_SUCCESS) {
            return SID_ERROR_GENERIC;
        }
    } else {
        ECDSA_OperationVerify operationVerify;
        ECDSA_OperationVerify_init(&operationVerify);
        operationVerify.curve             = &ECCParams_NISTP256;
        operationVerify.theirPublicKey    = key;
        operationVerify.hash              = hash_digest;
        operationVerify.r                 = params->signature;
        operationVerify.s                 = params->signature + (params->sig_size / 2);

        // Verify the signature
        int_fast16_t result = ECDSA_verify(crypto_handle[CC13X2_CRYPTO_ECDSA], &operationVerify);
        if (result != ECDSA_STATUS_SUCCESS) {
            return SID_ERROR_GENERIC;
        }
    }

    return SID_ERROR_NONE;
}

static sid_error_t cc13x2_crypto_ecc_dsa(sid_pal_dsa_params_t *params)
{
#define EXTRA_BYTE_SIZE     1
#define EXTRA_BYTE          0x04
    sid_error_t error = SID_ERROR_NONE;
    CryptoKey crypto_key;
    CryptoKey my_public_key;
    uint8_t temp_public_key[65];
    uint8_t *puk;
    int_fast16_t result;

    if ((params->algo == SID_PAL_ECDSA_SECP256R1) && (params->mode == SID_PAL_CRYPTO_VERIFY)) {
        memcpy(&temp_public_key[EXTRA_BYTE_SIZE], (uint8_t *)params->key, (size_t)params->key_size);
        temp_public_key[0] = EXTRA_BYTE;
        CryptoKeyPlaintext_initKey(&crypto_key, temp_public_key, (size_t)params->key_size + EXTRA_BYTE_SIZE);
    } else {
        CryptoKeyPlaintext_initKey(&crypto_key, (uint8_t *)params->key, (size_t)params->key_size);
    }

    if ((params->algo == SID_PAL_EDDSA_ED25519) && (params->mode == SID_PAL_CRYPTO_SIGN)) {
#if CC13X2_EDDSA_MULTIPLE_KEYS
        // regenerae the public key
        puk = temp_public_key;
        error = cc13x2_regen_eddsa_public_key((uint8_t *)params->key, params->key_size, puk, params->key_size);

        if (error != SID_ERROR_NONE) {
            return SID_ERROR_GENERIC;
        }
#else
        // get the public key from stored location
        puk = cc13x2_get_eddsa_public_key();
#endif
        CryptoKeyPlaintext_initKey(&my_public_key, puk, (size_t)params->key_size);
    }

    switch(params->algo) {
    case SID_PAL_ECDH_CURVE25519:
        error = SID_ERROR_NOSUPPORT;
        break;
    case SID_PAL_ECDH_SECP256R1:
        error = SID_ERROR_NOSUPPORT;
        break;
    case SID_PAL_EDDSA_ED25519:
        if (params->mode == SID_PAL_CRYPTO_SIGN) {
            EDDSA_OperationSign operationSign;
            EDDSA_OperationSign_init(&operationSign);
            operationSign.curve             = &ECCParams_Ed25519;
            operationSign.myPrivateKey      = &crypto_key;
            operationSign.myPublicKey       = &my_public_key;
            operationSign.preHashedMessage  = params->in;
            operationSign.preHashedMessageLength = params->in_size;
            operationSign.R                 = params->signature;
            operationSign.S                 = params->signature + (params->sig_size / 2);

            // Generate the signature
            result = EDDSA_sign(crypto_handle[CC13X2_CRYPTO_EDDSA], &operationSign);

            if (result != EDDSA_STATUS_SUCCESS) {
                error = SID_ERROR_GENERIC;
            }
        } else {
            EDDSA_OperationVerify operationVerify;
            EDDSA_OperationVerify_init(&operationVerify);
            operationVerify.curve             = &ECCParams_Ed25519;
            operationVerify.theirPublicKey    = &crypto_key;
            operationVerify.preHashedMessage  = params->in;
            operationVerify.preHashedMessageLength = params->in_size;
            operationVerify.R                 = params->signature;
            operationVerify.S                 = params->signature + (params->sig_size / 2);

            // Verify the signature
            result = EDDSA_verify(crypto_handle[CC13X2_CRYPTO_EDDSA], &operationVerify);

            if (result != EDDSA_STATUS_SUCCESS) {
                error = SID_ERROR_GENERIC;
            }
        }
        break;
    case SID_PAL_ECDSA_SECP256R1:
        error = do_ecdsa_secp256r1(&crypto_key, params);
        break;
    default:
        error = SID_ERROR_INVALID_ARGS;
        break;
    }

    return error;
}

static sid_error_t cc13x2_crypto_ecc_ecdh(sid_pal_ecdh_params_t *params)
{
    sid_error_t error = SID_ERROR_NONE;
    CryptoKey my_private_key;
    CryptoKey their_public_key;
    CryptoKey shared_secret;
    uint32_t temp_private_key[17];
    int_fast16_t result;

    switch(params->algo) {
    case SID_PAL_ECDH_CURVE25519: {
        ECDH_OperationComputeSharedSecret operationComputeSharedSecret;
        // Initialize their public CryptoKey and the shared secret CryptoKey
        CryptoKeyPlaintext_initKey(&their_public_key, (uint8_t *)params->puk, params->puk_size);
        /* Note: TI CURVE25519 crypto code expects private keys to be stored in Big-Endian order, see ECDH.h
         * PAL Unit Test code and all other crypto implementations use LE for private keys.
         * Make an endian-swapping copy here so the passed in LE private key is in BE format for the algo.
         */
        CryptoUtils_copyPad(params->prk, temp_private_key, params->prk_size);
        CryptoKeyPlaintext_initKey(&my_private_key, (uint8_t *)temp_private_key, params->prk_size);
        CryptoKeyPlaintext_initBlankKey(&shared_secret, params->shared_secret, params->shared_secret_sz);

        // The ECC_NISTP256 struct is provided in ti/drivers/types/EccParams.h and the corresponding device-specific implementation
        ECDH_OperationComputeSharedSecret_init(&operationComputeSharedSecret);
        operationComputeSharedSecret.curve                      = &ECCParams_Curve25519;
        operationComputeSharedSecret.myPrivateKey               = &my_private_key;
        operationComputeSharedSecret.theirPublicKey             = &their_public_key;
        operationComputeSharedSecret.sharedSecret               = &shared_secret;
        operationComputeSharedSecret.keyMaterialEndianness      = ECDH_LITTLE_ENDIAN_KEY;
        // Compute the shared secret and copy it to sharedSecretKeyingMaterial
        result = ECDH_computeSharedSecret(crypto_handle[CC13X2_CRYPTO_ECDH], &operationComputeSharedSecret);

        if (result != ECDH_STATUS_SUCCESS) {
            error = SID_ERROR_GENERIC;
        }
        break;
    }
    case SID_PAL_ECDH_SECP256R1:
        error = SID_ERROR_NOSUPPORT;
        break;
    case SID_PAL_EDDSA_ED25519:
        error = SID_ERROR_NOSUPPORT;
        break;
    case SID_PAL_ECDSA_SECP256R1:
        error = SID_ERROR_NOSUPPORT;
        break;
    default:
        error = SID_ERROR_INVALID_ARGS;
        break;
    }

    return error;
}

static sid_error_t cc13x2_crypto_ecc_key_gen(sid_pal_ecc_key_gen_params_t *params)
{
    sid_error_t error = SID_ERROR_NONE;
    CryptoKey my_private_key;
    CryptoKey my_public_key;
    ECCParams_CurveParams     *curve = (ECCParams_CurveParams *)&ECCParams_NISTP256;
    ECDH_KeyMaterialEndianness keyMaterialEndianness = ECDH_BIG_ENDIAN_KEY;
    uint8_t temp_public_key[65];
    uint8_t extra_byte_size = 0;
    bool is_prune_needed = false;
    int_fast16_t result;
    ECDH_OperationGeneratePublicKey ecdhOperationGeneratePublicKey;
    EDDSA_OperationGeneratePublicKey eddsaOperationGeneratePublicKey;

    switch(params->algo) {
        case SID_PAL_ECDH_CURVE25519:
            curve = (ECCParams_CurveParams *)&ECCParams_Curve25519;
            keyMaterialEndianness = ECDH_LITTLE_ENDIAN_KEY;
            extra_byte_size = 0;
            is_prune_needed = false;
            break;
        case SID_PAL_ECDH_SECP256R1:
            return SID_ERROR_NOSUPPORT;
            break;
        case SID_PAL_EDDSA_ED25519:
            /* will be supported at Q1 2021 */
            curve = (ECCParams_CurveParams *)&ECCParams_Ed25519;
            extra_byte_size = 0;
            is_prune_needed = true;
            break;
        case SID_PAL_ECDSA_SECP256R1:
            curve = (ECCParams_CurveParams *)&ECCParams_NISTP256;
            keyMaterialEndianness = ECDH_BIG_ENDIAN_KEY;
            extra_byte_size = 1;
            break;
        default:
            return SID_ERROR_INVALID_ARGS;
            break;
    }

    if (params->algo == SID_PAL_EDDSA_ED25519) {
        do {
            CryptoKeyPlaintext_initBlankKey(&my_private_key, params->prk, params->prk_size);
            CryptoKeyPlaintext_initBlankKey(&my_public_key, temp_public_key, params->puk_size + extra_byte_size);

            result = TRNG_generateEntropy(crypto_handle[CC13X2_TRNG], &my_private_key);

            if (result != TRNG_STATUS_SUCCESS) {
              return SID_ERROR_GENERIC;
            }

            if (is_prune_needed) {
                cc13x2_crypto_prune(params->prk);
            }

            EDDSA_OperationGeneratePublicKey_init(&eddsaOperationGeneratePublicKey);
            eddsaOperationGeneratePublicKey.curve                 = curve;
            eddsaOperationGeneratePublicKey.myPrivateKey          = &my_private_key;
            eddsaOperationGeneratePublicKey.myPublicKey           = &my_public_key;

            result = EDDSA_generatePublicKey(crypto_handle[CC13X2_CRYPTO_EDDSA], &eddsaOperationGeneratePublicKey);
        } while(result == EDDSA_STATUS_INVALID_PRIVATE_KEY_SIZE || result == EDDSA_STATUS_POINT_AT_INFINITY);


        if (result != EDDSA_STATUS_SUCCESS) {
            error = SID_ERROR_GENERIC;
        }

        memcpy(eddsa_public_key, &temp_public_key[extra_byte_size], params->puk_size);
    } else {
        do {
            CryptoKeyPlaintext_initBlankKey(&my_private_key, params->prk, params->prk_size);
            CryptoKeyPlaintext_initBlankKey(&my_public_key, temp_public_key, params->puk_size + extra_byte_size);

            result = TRNG_generateEntropy(crypto_handle[CC13X2_TRNG], &my_private_key);

            if (result != TRNG_STATUS_SUCCESS) {
              return SID_ERROR_GENERIC;
            }

            if (is_prune_needed) {
                cc13x2_crypto_prune(params->prk);
            }

            ECDH_OperationGeneratePublicKey_init(&ecdhOperationGeneratePublicKey);
            ecdhOperationGeneratePublicKey.curve                 = curve;
            ecdhOperationGeneratePublicKey.keyMaterialEndianness = keyMaterialEndianness;
            ecdhOperationGeneratePublicKey.myPrivateKey          = &my_private_key;
            ecdhOperationGeneratePublicKey.myPublicKey           = &my_public_key;

            result = ECDH_generatePublicKey(crypto_handle[CC13X2_CRYPTO_ECDH], &ecdhOperationGeneratePublicKey);
        } while(result == ECDH_STATUS_PRIVATE_KEY_LARGER_EQUAL_ORDER || result == ECDH_STATUS_PRIVATE_KEY_ZERO);


        if (result != ECDH_STATUS_SUCCESS) {
            error = SID_ERROR_GENERIC;
        }
    }

    memcpy(params->puk, &temp_public_key[extra_byte_size], params->puk_size);

    return error;
}

sid_error_t sid_pal_crypto_init(void)
{
    if (crypto_init_done) {
        return SID_ERROR_NONE;
    }

    sid_error_t ret = cc13x2_crypto_init();
    if (ret != SID_ERROR_NONE) {
        return ret;
    }

    crypto_init_done = true;

    uint32_t seed;
    ret = sid_pal_crypto_rand((uint8_t*)&seed, sizeof(seed));
    if (ret == SID_ERROR_NONE) {
        srand(seed);
    } else {
        crypto_init_done = false;
    }

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_crypto_deinit(void)
{
    if (!crypto_init_done) {
        return SID_ERROR_UNINITIALIZED;
    }

    cc13x2_crypto_deinit();
    crypto_init_done = false;

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_crypto_rand(uint8_t* rand, size_t size)
{
    if (!crypto_init_done) {
        return SID_ERROR_UNINITIALIZED;
    }

    if (!rand || !size) {
        return SID_ERROR_NULL_POINTER;
    }

    return cc13x2_crypto_rand(rand, size);
}

sid_error_t sid_pal_crypto_hash(sid_pal_hash_params_t* params)
{
    if (!crypto_init_done) {
        return SID_ERROR_UNINITIALIZED;
    }

    if (!params || !params->data || !params->digest) {
        return SID_ERROR_NULL_POINTER;
    }

    if (!params->data_size || !params->digest_size) {
        return SID_ERROR_INVALID_ARGS;
    }

    return cc13x2_crypto_hash(params);
}

sid_error_t sid_pal_crypto_hmac(sid_pal_hmac_params_t* params)
{
    if (!crypto_init_done) {
        return SID_ERROR_UNINITIALIZED;
    }

    if (!params || !params->key || !params->data || !params->digest) {
        return SID_ERROR_NULL_POINTER;
    }

    if (!params->key_size || !params->data_size || !params->digest_size) {
        return SID_ERROR_INVALID_ARGS;
    }

    return cc13x2_crypto_hmac(params);
}

sid_error_t sid_pal_crypto_aes_crypt(sid_pal_aes_params_t* params)
{
    if (!crypto_init_done) {
        return SID_ERROR_UNINITIALIZED;
    }

    if (!params || !params->key || !params->in || !params->out) {
        return SID_ERROR_NULL_POINTER;
    }

    if (params->algo == SID_PAL_AES_CTR_128 && (!params->iv || params->iv_size != AES_KEY_SZ)) {
        return SID_ERROR_NULL_POINTER;
    }

    if (!params->in_size || !params->key_size) {
        return SID_ERROR_INVALID_ARGS;
    }

    return cc13x2_crypto_aes_crypt(params);
}

sid_error_t sid_pal_crypto_aead_crypt(sid_pal_aead_params_t* params)
{
    if (!crypto_init_done) {
        return SID_ERROR_UNINITIALIZED;
    }

    if (!params || !params->key || !params->in || !params->out || !params->iv || !params->aad) {
        return SID_ERROR_NULL_POINTER;
    }

    if (!params->iv_size || !params->aad_size || !params->key_size || !params->in_size) {
        return SID_ERROR_INVALID_ARGS;
    }

    return cc13x2_crypto_aead_crypt(params);
}

sid_error_t sid_pal_crypto_ecc_dsa(sid_pal_dsa_params_t* params)
{
    if (!crypto_init_done) {
        return SID_ERROR_UNINITIALIZED;
    }

    if (!params || !params->key || !params->in || !params->signature) {
        return SID_ERROR_NULL_POINTER;
    }

    if (!params->key_size || !params->in_size) {
        return SID_ERROR_INVALID_ARGS;
    }

    return cc13x2_crypto_ecc_dsa(params);
}

sid_error_t sid_pal_crypto_ecc_ecdh(sid_pal_ecdh_params_t* params)
{
    if (!crypto_init_done) {
        return SID_ERROR_UNINITIALIZED;
    }

    if (!params || !params->prk || !params->puk || !params->shared_secret) {
        return SID_ERROR_NULL_POINTER;
    }

    return cc13x2_crypto_ecc_ecdh(params);
}

sid_error_t sid_pal_crypto_ecc_key_gen(sid_pal_ecc_key_gen_params_t* params)
{
    if (!crypto_init_done) {
        return SID_ERROR_UNINITIALIZED;
    }

    if (!params || !params->prk || !params->puk) {
        return SID_ERROR_NULL_POINTER;
    }

    return cc13x2_crypto_ecc_key_gen(params);
}