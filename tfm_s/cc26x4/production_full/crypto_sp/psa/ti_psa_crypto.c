/*
 * Copyright (c) 2022 Texas Instruments Incorporated - http://www.ti.com
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

#include <third_party/mbedtls/include/psa/crypto.h>

#include <psa_manifest/pid.h>

#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
#include <ti/drivers/cryptoutils/ecc/ECCParams.h>

#include <ti/drivers/ECDSA.h>
#include <ti/drivers/ecdsa/ECDSACC26X2.h>

#include <ti/drivers/SHA2.h>
#include <ti/drivers/sha2/SHA2CC26X2.h>

#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_helpers.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/pka.h)

#include "ti_drivers_config.h" /* Sysconfig generated header */

typedef struct
{
    bool isInitialized;
    bool isInitFailed;
} psa_global_data_t;

static psa_global_data_t global_data;

ECDSA_Handle ECDSA_handle = NULL;
SHA2_Handle SHA2_handle   = NULL;

/* Back-end Mbed Crypto hash operation struct */
psa_hash_operation_t ti_psa_crypto_hashOperation;

static psa_status_t ecdsa_to_psa_status(int_fast16_t ecdsa_status)
{
    psa_status_t status;

    switch (ecdsa_status)
    {
        case ECDSA_STATUS_SUCCESS:
            status = PSA_SUCCESS;
            break;

        case ECDSA_STATUS_RESOURCE_UNAVAILABLE:
            status = PSA_ERROR_HARDWARE_FAILURE;
            break;

        case ECDSA_STATUS_ERROR:
            status = PSA_ERROR_INVALID_SIGNATURE;
            break;

        case ECDSA_STATUS_CANCELED:
            status = PSA_ERROR_NOT_PERMITTED;
            break;

        case ECDSA_STATUS_INVALID_KEY_SIZE:
            status = PSA_ERROR_INVALID_HANDLE;
            break;

        default:
            status = PSA_ERROR_GENERIC_ERROR;
            break;
    }

    return status;
}

/* Hard-coded for attestation signature using NIST P256 */
psa_status_t ti_psa_sign_hash(psa_key_handle_t handle,
                              psa_algorithm_t alg,
                              const uint8_t *hash,
                              size_t hash_length,
                              uint8_t *signature,
                              size_t signature_size,
                              size_t *signature_length)
{
    CryptoKey myPrivateKey;
    ECDSA_OperationSign operationSign;
    int_fast16_t status;
    KeyStore_PSA_KeyFileId keyID;
    size_t keyLength;
    uint8_t keyMaterial[NISTP256_PARAM_SIZE_BYTES];

    *signature_length = 0;

    /* only support ECDSA for psa_sign_hash */
    if (!PSA_ALG_IS_ECDSA(alg) || PSA_ALG_IS_DETERMINISTIC_ECDSA(alg))
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    if (signature_size < (2 * NISTP256_PARAM_SIZE_BYTES))
    {
        return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    if (hash_length < NISTP256_PARAM_SIZE_BYTES)
    {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    /* Private key for attestation */
    keyID.key_id = KEYSTORE_PSA_PRE_PROVISIONED_ATTESTATION_PRI_KEY_ID;
    keyID.owner  = TFM_SP_INITIAL_ATTESTATION;

    /*
     * ECDSA driver does not support SKS yet so we must retrieve the key here
     * and create a plaintext CryptoKey.
     */
    status = KeyStore_PSA_getKey(keyID,
                                 &keyMaterial[0],
                                 sizeof(keyMaterial),
                                 &keyLength,
                                 PSA_ALG_ECDSA(PSA_ALG_SHA_256),
                                 KEYSTORE_PSA_KEY_USAGE_SIGN_HASH);

    if (status != KEYSTORE_PSA_STATUS_SUCCESS)
    {
        return status;
    }

    CryptoKeyPlaintext_initKey(&myPrivateKey, keyMaterial, keyLength);

    ECDSA_OperationSign_init(&operationSign);

    operationSign.curve = &ECCParams_NISTP256;

    operationSign.myPrivateKey = &myPrivateKey;
    operationSign.hash         = hash;
    operationSign.r            = signature;
    operationSign.s            = signature + NISTP256_PARAM_SIZE_BYTES;

    status = ECDSA_sign(ECDSA_handle, &operationSign);

    if (status == ECDSA_STATUS_SUCCESS)
    {
        *signature_length = (2 * NISTP256_PARAM_SIZE_BYTES);
    }

    return ecdsa_to_psa_status(status);
}

psa_status_t sha2_to_psa_status(int_fast16_t sha2_status)
{
    psa_status_t status;

    switch (sha2_status)
    {
        case SHA2_STATUS_SUCCESS:
            status = PSA_SUCCESS;
            break;

        case SHA2_STATUS_ERROR:
            status = PSA_ERROR_GENERIC_ERROR;
            break;

        case SHA2_STATUS_RESOURCE_UNAVAILABLE:
            status = PSA_ERROR_HARDWARE_FAILURE;
            break;

        case SHA2_STATUS_CANCELED:
            status = PSA_ERROR_NOT_PERMITTED;
            break;

        case SHA2_STATUS_UNSUPPORTED:
            status = PSA_ERROR_NOT_SUPPORTED;
            break;

        default:
            status = PSA_ERROR_GENERIC_ERROR;
            break;
    }

    return status;
}

static psa_status_t ti_psa_hash_abort(psa_hash_operation_t *operation)
{
    if (operation->alg != 0)
    {
        SHA2_reset(SHA2_handle);
        memset(operation, 0, sizeof(psa_hash_operation_t));
    }

    return (PSA_SUCCESS);
}

psa_status_t ti_psa_hash_setup(psa_hash_operation_t *operation, psa_algorithm_t alg)
{
    int_fast16_t status;

    if (!PSA_ALG_IS_HASH(alg))
    {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (operation->alg != 0)
    {
        return PSA_ERROR_BAD_STATE;
    }

    switch (alg)
    {
        case PSA_ALG_SHA_224:
            status = SHA2_setHashType(SHA2_handle, SHA2_HASH_TYPE_224);
            break;

        case PSA_ALG_SHA_256:
            status = SHA2_setHashType(SHA2_handle, SHA2_HASH_TYPE_256);
            break;

        case PSA_ALG_SHA_384:
            status = SHA2_setHashType(SHA2_handle, SHA2_HASH_TYPE_384);
            break;

        case PSA_ALG_SHA_512:
            status = SHA2_setHashType(SHA2_handle, SHA2_HASH_TYPE_512);
            break;

        default:
            status = SHA2_STATUS_UNSUPPORTED;
            break;
    }

    if (status == SHA2_STATUS_SUCCESS)
    {
        operation->alg = alg;
    }
    else
    {
        ti_psa_hash_abort(operation);
    }

    return sha2_to_psa_status(status);
}

#define ATTESTATION_PUBLIC_KEY_SIZE 65

/* Hardcoded to hash the attestation ECDSA P256 public key */
psa_status_t ti_psa_hash_update(psa_hash_operation_t *operation, const uint8_t *input, size_t input_length)
{
    (void)input; /* Unused input variable */
    size_t keyLength;
    int_fast16_t status;
    KeyStore_PSA_KeyFileId keyID;
    uint8_t keyMaterial[ATTESTATION_PUBLIC_KEY_SIZE];

    if (input_length == 0)
    {
        return PSA_SUCCESS;
    }

    /* Only SHA-2 is supported */
    if ((operation->alg != PSA_ALG_SHA_224) && (operation->alg != PSA_ALG_SHA_256) &&
        (operation->alg != PSA_ALG_SHA_384) && (operation->alg != PSA_ALG_SHA_512))
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    /* Public key for attestation */
    keyID.key_id = KEYSTORE_PSA_PRE_PROVISIONED_ATTESTATION_PUB_KEY_ID;
    keyID.owner  = TFM_SP_INITIAL_ATTESTATION;

    status = KeyStore_PSA_exportPublicKey(keyID, &keyMaterial[0], sizeof(keyMaterial), &keyLength);

    if (status != KEYSTORE_PSA_STATUS_SUCCESS)
    {
        return status;
    }

    status = SHA2_addData(SHA2_handle, &keyMaterial[0], keyLength);

    if (status != PSA_SUCCESS)
    {
        (void)ti_psa_hash_abort(operation);
    }

    return sha2_to_psa_status(status);
}

psa_status_t ti_psa_hash_finish(psa_hash_operation_t *operation, uint8_t *hash, size_t hash_size, size_t *hash_length)
{
    int_fast16_t status;
    size_t actual_hash_length = PSA_HASH_SIZE(operation->alg);
    /* Fill the output buffer with something that isn't a valid hash
     * (barring an attack on the hash and deliberately-crafted input),
     * in case the caller doesn't check the return status properly. */
    *hash_length              = actual_hash_length;

    /* If hash_size is 0 then hash may be NULL and then the
     * call to memset would have undefined behavior. */
    if (hash_size != 0)
    {
        memset(hash, '!', hash_size);
    }

    /* If the output length is smaller than the hash, then report user mistake
     for this alg that does not support a shorten hash.*/
    if (hash_size < actual_hash_length)
    {
        return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    /* Only SHA-2 is supported */
    if ((operation->alg != PSA_ALG_SHA_224) && (operation->alg != PSA_ALG_SHA_256) &&
        (operation->alg != PSA_ALG_SHA_384) && (operation->alg != PSA_ALG_SHA_512))
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    status = SHA2_finalize(SHA2_handle, hash);

    if (status != PSA_SUCCESS)
    {
        (void)ti_psa_hash_abort(operation);
    }
    else
    {
        operation->alg = 0;
    }

    return sha2_to_psa_status(status);
}

static psa_status_t ti_psa_driver_init(void)
{
    psa_status_t status = PSA_ERROR_GENERIC_ERROR;

    if (SHA2_handle == NULL)
    {
        SHA2_Params SHA2_params;

        SHA2_init();

        SHA2_Params_init(&SHA2_params);
        SHA2_params.returnBehavior = SHA2_RETURN_BEHAVIOR_POLLING;
        SHA2_handle                = SHA2_open((CONFIG_TI_DRIVERS_SHA2_COUNT - 1), &SHA2_params);
    }

    if (ECDSA_handle == NULL)
    {
        ECDSA_Params ECDSA_params;

        ECDSA_init();

        ECDSA_Params_init(&ECDSA_params);
        ECDSA_params.returnBehavior = ECDSA_RETURN_BEHAVIOR_POLLING;
        ECDSA_handle                = ECDSA_open((CONFIG_TI_DRIVERS_ECDSA_COUNT - 1), &ECDSA_params);
    }

    if ((SHA2_handle != NULL) && (ECDSA_handle != NULL))
    {
        status = PSA_SUCCESS;
    }

    return status;
}

psa_status_t ti_psa_crypto_init(void)
{
    psa_status_t status;

    if (global_data.isInitFailed)
    {
        return PSA_ERROR_INVALID_HANDLE;
    }
    else if (global_data.isInitialized)
    {
        return PSA_SUCCESS;
    }

    status = ti_psa_driver_init();

    if (status == PSA_SUCCESS)
    {
        global_data.isInitialized = true;
    }
    else
    {
        global_data.isInitFailed = true;
    }

    return status;
}
