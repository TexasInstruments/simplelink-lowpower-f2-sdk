/*
 * Copyright (c) 2022 Texas Instruments Incorporated. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Contains PSA Crypto API client interface exposed to the Secure Processing
 * Environment with only functions necessary to support attestation service.
 *
 */

#include <stdint.h>

#include <psa/ti_psa_tfm.h>
#include <third_party/tfm/interface/include/psa/crypto.h>

#include <psa/PSA_s.h>

#include <third_party/tfm/interface/include/tfm_api.h>
#include <third_party/tfm/interface/include/psa/client.h>
#include <third_party/tfm/interface/include/psa/error.h>

#include <psa_manifest/sid.h>


psa_status_t psa_sign_hash(psa_key_handle_t handle,
                           psa_algorithm_t alg,
                           const uint8_t *hash,
                           size_t hash_length,
                           uint8_t *signature,
                           size_t signature_size,
                           size_t *signature_length)
{
    PSA_s_SignHashMsg signMsg;
    psa_handle_t sp_handle = PSA_NULL_HANDLE;
    psa_status_t status;
    struct psa_invec invecs[1];

    signMsg.handle           = handle;
    signMsg.alg              = alg;
    signMsg.hash             = hash;
    signMsg.hash_length      = hash_length;
    signMsg.signature        = signature;
    signMsg.signature_size   = signature_size;
    signMsg.signature_length = signature_length;
    invecs[0].base           = &signMsg;
    invecs[0].len            = sizeof(signMsg);

    sp_handle = psa_connect(CRYPTO_SP_SERVICE_SID, CRYPTO_SP_SERVICE_VERSION);

    if (!PSA_HANDLE_IS_VALID(sp_handle))
    {
        return PSA_HANDLE_TO_ERROR(sp_handle);
    }

    status = psa_call(sp_handle, PSA_S_MSG_TYPE_SIGN_HASH, invecs, 1, NULL, 0);

    psa_close(sp_handle);

    return status;
}

psa_status_t psa_hash_setup(psa_hash_operation_t *operation, psa_algorithm_t alg)
{
    PSA_s_HashSetupMsg setupMsg;
    psa_handle_t sp_handle = PSA_NULL_HANDLE;
    psa_status_t status;
    struct psa_invec invecs[1];

    setupMsg.operation = operation;
    setupMsg.alg       = alg;
    invecs[0].base     = &setupMsg;
    invecs[0].len      = sizeof(setupMsg);

    sp_handle = psa_connect(CRYPTO_SP_SERVICE_SID, CRYPTO_SP_SERVICE_VERSION);

    if (!PSA_HANDLE_IS_VALID(sp_handle))
    {
        return PSA_HANDLE_TO_ERROR(sp_handle);
    }

    status = psa_call(sp_handle, PSA_S_MSG_TYPE_HASH_SETUP, invecs, 1, NULL, 0);

    psa_close(sp_handle);

    return status;
}

psa_status_t psa_hash_update(psa_hash_operation_t *operation, const uint8_t *input, size_t input_length)
{
    PSA_s_HashUpdateMsg updateMsg;
    psa_handle_t sp_handle = PSA_NULL_HANDLE;
    psa_status_t status;
    struct psa_invec invecs[1];

    updateMsg.operation    = operation;
    updateMsg.input        = input;
    updateMsg.input_length = input_length;
    invecs[0].base         = &updateMsg;
    invecs[0].len          = sizeof(updateMsg);

    sp_handle = psa_connect(CRYPTO_SP_SERVICE_SID, CRYPTO_SP_SERVICE_VERSION);

    if (!PSA_HANDLE_IS_VALID(sp_handle))
    {
        return PSA_HANDLE_TO_ERROR(sp_handle);
    }

    status = psa_call(sp_handle, PSA_S_MSG_TYPE_HASH_UPDATE, invecs, 1, NULL, 0);

    psa_close(sp_handle);

    return status;
}

psa_status_t psa_hash_finish(psa_hash_operation_t *operation, uint8_t *hash, size_t hash_size, size_t *hash_length)
{
    PSA_s_HashFinishMsg finishMsg;
    psa_handle_t sp_handle = PSA_NULL_HANDLE;
    psa_status_t status;
    struct psa_invec invecs[1];

    finishMsg.operation   = operation;
    finishMsg.hash        = hash;
    finishMsg.hash_size   = hash_size;
    finishMsg.hash_length = hash_length;
    invecs[0].base        = &finishMsg;
    invecs[0].len         = sizeof(finishMsg);

    sp_handle = psa_connect(CRYPTO_SP_SERVICE_SID, CRYPTO_SP_SERVICE_VERSION);

    if (!PSA_HANDLE_IS_VALID(sp_handle))
    {
        return PSA_HANDLE_TO_ERROR(sp_handle);
    }

    status = psa_call(sp_handle, PSA_S_MSG_TYPE_HASH_FINISH, invecs, 1, NULL, 0);

    psa_close(sp_handle);

    return status;
}

psa_status_t psa_crypto_init(void)
{
    /* Init is handled by secure partition */
    return PSA_SUCCESS;
}
