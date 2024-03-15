/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
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

/****************************************************************/
/* Error mapping functions*/
/****************************************************************/

#include <psa/crypto.h>
#include <psa/crypto_compat.h>
#include <cc13x4_cc26x4/crypto_helper.h>

#include <ti/drivers/SHA2.h>
#include <ti/drivers/AESCommon.h>
#include <ti/drivers/ECDSA.h>
#include <ti/drivers/EDDSA.h>
#include <ti/drivers/ECDH.h>
#include <ti/drivers/TRNG.h>

psa_status_t map_SHA2Error_to_PSA_error(int_fast16_t status)
{
    psa_status_t psaStatus;

    switch (status)
    {
        case SHA2_STATUS_RESOURCE_UNAVAILABLE:
            psaStatus = PSA_ERROR_HARDWARE_FAILURE;
            break;

        case SHA2_STATUS_CANCELED:
            psaStatus = PSA_ERROR_BAD_STATE;
            break;

        case SHA2_STATUS_KEYSTORE_ERROR:
            psaStatus = PSA_ERROR_INVALID_HANDLE;
            break;

        case SHA2_STATUS_UNSUPPORTED:
            psaStatus = PSA_ERROR_NOT_SUPPORTED;
            break;

        case SHA2_STATUS_ERROR:
        default:
            psaStatus = PSA_ERROR_GENERIC_ERROR;
            break;
    }
    return psaStatus;
}

psa_status_t map_ECDSAError_to_PSA_error(int_fast16_t status)
{
/* no s/ns support for ECDSA and EDDSA */
#if TFM_ENABLED == 0

    psa_status_t psaStatus;

    switch (status)
    {
        case ECDSA_STATUS_RESOURCE_UNAVAILABLE:
            psaStatus = PSA_ERROR_HARDWARE_FAILURE;
            break;

        case ECDSA_STATUS_ERROR:
        case ECDSA_STATUS_R_LARGER_THAN_ORDER:
        case ECDSA_STATUS_S_LARGER_THAN_ORDER:
            psaStatus = PSA_ERROR_INVALID_SIGNATURE;
            break;

        case ECDSA_STATUS_CANCELED:
            psaStatus = PSA_ERROR_BAD_STATE;
            break;

        case ECDSA_STATUS_INVALID_KEY_SIZE:
        case ECDSA_STATUS_POINT_AT_INFINITY:
        case ECDSA_STATUS_PUBLIC_KEY_NOT_ON_CURVE:
        case ECDSA_STATUS_PUBLIC_KEY_LARGER_THAN_PRIME:
            psaStatus = PSA_ERROR_INVALID_HANDLE;
            break;

        default:
            psaStatus = PSA_ERROR_GENERIC_ERROR;
            break;
    }
    return psaStatus;
#else
    return PSA_ERROR_NOT_SUPPORTED;
#endif
}

psa_status_t map_EDDSAError_to_PSA_error(int_fast16_t status)
{
/* no s/ns support for ECDSA and EDDSA */
#if TFM_ENABLED == 0

    psa_status_t psaStatus;

    switch (status)
    {
        case EDDSA_STATUS_ERROR:
        case EDDSA_STATUS_INVALID_R_SIZE:
        case EDDSA_STATUS_INVALID_S_SIZE:
        case EDDSA_STATUS_S_LARGER_THAN_ORDER:
            psaStatus = PSA_ERROR_INVALID_SIGNATURE;
            break;

        case EDDSA_STATUS_HASH_UNAVAILABLE:
        case EDDSA_STATUS_PKA_UNAVAILABLE:
        case EDDSA_STATUS_SHA2_HASH_FAILURE:
            psaStatus = PSA_ERROR_HARDWARE_FAILURE;
            break;

        case EDDSA_STATUS_PUBLIC_KEY_NOT_ON_CURVE:
        case EDDSA_STATUS_INVALID_PRIVATE_KEY_SIZE:
        case EDDSA_STATUS_INVALID_PUBLIC_KEY_SIZE:
        case EDDSA_STATUS_KEY_ID_PARAM_MISMATCH:
        case EDDSA_STATUS_KEYSTORE_GENERIC_FAILURE:
        case EDDSA_STATUS_KEYSTORE_INVALID_ID:
            psaStatus = PSA_ERROR_INVALID_HANDLE;
            break;

        case EDDSA_STATUS_CANCELED:
            psaStatus = PSA_ERROR_BAD_STATE;
            break;

        default:
            psaStatus = PSA_ERROR_GENERIC_ERROR;
            break;
    }
    return psaStatus;
#else
    return PSA_ERROR_NOT_SUPPORTED;
#endif
}

psa_status_t map_ECDHError_to_PSA_error(int_fast16_t status)
{

    psa_status_t psaStatus;

    switch (status)
    {
        case ECDH_STATUS_RESOURCE_UNAVAILABLE:
            psaStatus = PSA_ERROR_HARDWARE_FAILURE;
            break;

        case ECDH_STATUS_INVALID_KEY_SIZE:
        case ECDH_STATUS_KEYSTORE_ERROR:
            psaStatus = PSA_ERROR_INVALID_HANDLE;
            break;

        case ECDH_STATUS_CANCELED:
            psaStatus = PSA_ERROR_BAD_STATE;
            break;

        case ECDH_STATUS_POINT_AT_INFINITY:
        case ECDH_STATUS_PRIVATE_KEY_LARGER_EQUAL_ORDER:
        case ECDH_STATUS_PRIVATE_KEY_ZERO:
        case ECDH_STATUS_PUBLIC_KEY_LARGER_THAN_PRIME:
        case ECDH_STATUS_PUBLIC_KEY_NOT_ON_CURVE:
            psaStatus = PSA_ERROR_INVALID_HANDLE;
            break;

        case ECDH_STATUS_ERROR:
        default:
            psaStatus = PSA_ERROR_GENERIC_ERROR;
            break;
    }
    return psaStatus;
}

psa_status_t map_TRNGError_to_PSA_error(int_fast16_t status)
{

    psa_status_t psaStatus;

    switch (status)
    {
        case TRNG_STATUS_RESOURCE_UNAVAILABLE:
            psaStatus = PSA_ERROR_HARDWARE_FAILURE;
            break;

        case TRNG_STATUS_INVALID_INPUTS:
            psaStatus = PSA_ERROR_INVALID_ARGUMENT;
            break;

        case TRNG_STATUS_CANCELED:
            psaStatus = PSA_ERROR_BAD_STATE;
            break;

        case TRNG_STATUS_KEYSTORE_ERROR:
            psaStatus = PSA_ERROR_INVALID_HANDLE;
            break;

        case TRNG_STATUS_ERROR:
        default:
            psaStatus = PSA_ERROR_GENERIC_ERROR;
            break;
    }

    return psaStatus;
}

psa_status_t map_AESError_to_PSA_error(int_fast16_t status)
{

    psa_status_t psaStatus;

    switch (status)
    {

        case AES_STATUS_RESOURCE_UNAVAILABLE:
            psaStatus = PSA_ERROR_HARDWARE_FAILURE;
            break;

        case AES_STATUS_CANCELED:
            psaStatus = PSA_ERROR_BAD_STATE;
            break;

        case AES_STATUS_FEATURE_NOT_SUPPORTED:
            psaStatus = PSA_ERROR_NOT_SUPPORTED;
            break;

        case AES_STATUS_KEYSTORE_GENERIC_ERROR:
        case AES_STATUS_KEYSTORE_INVALID_ID:
            psaStatus = PSA_ERROR_INVALID_HANDLE;
            break;

        case AES_STATUS_MAC_INVALID:
            psaStatus = PSA_ERROR_INVALID_SIGNATURE;
            break;

        case AES_STATUS_ERROR:
        default:
            psaStatus = PSA_ERROR_GENERIC_ERROR;
            break;
    }

    return psaStatus;
}

const ECCParams_CurveParams *map_keyTypeToECCParams(psa_key_type_t keyType, size_t keyBits)
{
    psa_ecc_family_t curveFamily             = PSA_KEY_TYPE_ECC_GET_FAMILY(keyType);
    const ECCParams_CurveParams *curveParams = NULL;

    switch (curveFamily)
    {
        case PSA_ECC_FAMILY_BRAINPOOL_P_R1:
            switch (keyBits)
            {
                case 256:
                    curveParams = &ECCParams_BrainpoolP256R1;
                    break;
                case 384:
                    curveParams = &ECCParams_BrainpoolP384R1;
                    break;
                case 512:
                    curveParams = &ECCParams_BrainpoolP512R1;
                    break;
            }
            break;
        case PSA_ECC_FAMILY_SECP_R1:
            switch (keyBits)
            {
                case 224:
                    curveParams = &ECCParams_NISTP224;
                    break;
                case 256:
                    curveParams = &ECCParams_NISTP256;
                    break;
                case 384:
                    curveParams = &ECCParams_NISTP384;
                    break;
                case 521:
                    curveParams = &ECCParams_NISTP521;
                    break;
            }
            break;
        case PSA_ECC_FAMILY_MONTGOMERY:
            switch (keyBits)
            {
                case 255:
                    curveParams = &ECCParams_Curve25519;
                    break;
            }
            break;
        case PSA_ECC_FAMILY_TWISTED_EDWARDS:
            switch (keyBits)
            {
                case 255:
                    curveParams = &ECCParams_Ed25519;
                    break;
            }
            break;
    }
    return curveParams;
}

SHA2_HashType map_hashAlgToHashType(psa_algorithm_t alg)
{
    SHA2_HashType hashType = SHA2_HASH_TYPE_224;

    switch (alg)
    {
        case PSA_ALG_SHA_224:
            hashType = SHA2_HASH_TYPE_224;
            break;

        case PSA_ALG_SHA_256:
            hashType = SHA2_HASH_TYPE_256;
            break;

        case PSA_ALG_SHA_384:
            hashType = SHA2_HASH_TYPE_384;
            break;

        case PSA_ALG_SHA_512:
            hashType = SHA2_HASH_TYPE_512;
            break;
    }

    return hashType;
}

psa_key_file_id_t toKeyStoreKeyID(psa_key_id_t keyID)
{
#if (TFM_ENABLED == 0)
    psa_key_file_id_t keystoreKeyID = {.key_id = keyID, .owner = -1};
    return keystoreKeyID;
#else
    return keyID;
#endif
}

psa_key_id_t toKeyID(psa_key_file_id_t keystoreKeyID)
{
#if (TFM_ENABLED == 0)
    return keystoreKeyID.key_id;
#else
    return keystoreKeyID;
#endif
}