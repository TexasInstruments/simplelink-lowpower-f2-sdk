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

psa_key_type_t map_PSAKeyType_to_KeyStoreKeyType(psa_key_type_t keyType, size_t keyBits)
{

    psa_key_type_t oldKeyType = keyType;

    if (!PSA_KEY_TYPE_IS_ECC(keyType))
    {
        switch (keyType)
        {
            case PSA_KEY_TYPE_AES:
                oldKeyType = OLD_PSA_KEY_TYPE_AES;
                break;
            case PSA_KEY_TYPE_ARC4:
                oldKeyType = OLD_PSA_KEY_TYPE_ARC4;
                break;
            case PSA_KEY_TYPE_CAMELLIA:
                oldKeyType = OLD_PSA_KEY_TYPE_CAMELLIA;
                break;
            case PSA_KEY_TYPE_CHACHA20:
                oldKeyType = OLD_PSA_KEY_TYPE_CHACHA20;
                break;
            case PSA_KEY_TYPE_DERIVE:
                oldKeyType = OLD_PSA_KEY_TYPE_DERIVE;
                break;
            case PSA_KEY_TYPE_DES:
                oldKeyType = OLD_PSA_KEY_TYPE_DES;
                break;
            case PSA_KEY_TYPE_HMAC:
                oldKeyType = OLD_PSA_KEY_TYPE_HMAC;
                break;
            case PSA_KEY_TYPE_RAW_DATA:
                oldKeyType = OLD_PSA_KEY_TYPE_RAW_DATA;
                break;
            case PSA_KEY_TYPE_RSA_KEY_PAIR:
                oldKeyType = OLD_PSA_KEY_TYPE_RSA_KEY_PAIR;
                break;
            case PSA_KEY_TYPE_RSA_PUBLIC_KEY:
                oldKeyType = OLD_PSA_KEY_TYPE_RSA_PUBLIC_KEY;
                break;
        }
    }
    else
    {

        psa_ecc_family_t curveFamily = PSA_KEY_TYPE_ECC_GET_FAMILY(keyType);
        /*
         * To get the key type, take the bitwise
         * OR of the key base and the curve
         */
        psa_key_type_t base          = PSA_KEY_TYPE_IS_ECC_PUBLIC_KEY(keyType) ? OLD_PSA_KEY_TYPE_ECC_PUBLIC_KEY_BASE
                                                                               : OLD_PSA_KEY_TYPE_ECC_KEY_PAIR_BASE;

        switch (curveFamily)
        {
            case PSA_ECC_FAMILY_BRAINPOOL_P_R1:
                switch (keyBits)
                {
                    case 256:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_BRAINPOOL_P256R1;
                        break;
                    case 384:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_BRAINPOOL_P384R1;
                        break;
                    case 512:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_BRAINPOOL_P512R1;
                        break;
                }
                break;
            case PSA_ECC_FAMILY_SECP_R1:
                switch (keyBits)
                {
                    case 224:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECP224R1;
                        break;
                    case 256:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECP256R1;
                        break;
                    case 384:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECP384R1;
                        break;
                    case 521:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECP521R1;
                        break;
                }
                break;
            case PSA_ECC_FAMILY_SECP_R2:
                switch (keyBits)
                {
                    case 160:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECP160R2;
                        break;
                }
                break;
            case PSA_ECC_FAMILY_SECT_K1:
                switch (keyBits)
                {
                    case 163:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECT163K1;
                        break;
                    case 233:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECT233K1;
                        break;
                    case 239:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECT239K1;
                        break;
                    case 283:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECT283K1;
                        break;
                    case 409:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECT409K1;
                        break;
                    case 571:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECT571K1;
                        break;
                }
                break;
            case PSA_ECC_FAMILY_SECT_R1:
                switch (keyBits)
                {
                    case 163:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECT163R1;
                        break;
                    case 193:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECT193R1;
                        break;
                    case 233:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECT233R1;
                        break;
                    case 283:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECT283R1;
                        break;
                    case 409:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECT409R1;
                        break;
                    case 571:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECT571R1;
                        break;
                }
                break;
            case PSA_ECC_FAMILY_SECT_R2:
                switch (keyBits)
                {
                    case 163:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECT163R2;
                        break;
                    case 193:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_SECT193R2;
                        break;
                }
                break;
            case PSA_ECC_FAMILY_MONTGOMERY:
                switch (keyBits)
                {
                    case 255:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_CURVE25519;
                        break;
                }
                break;
            /* No clear mapping for TWISTED EDWARDS FAMILY
             * It just needs to map to some valid curve. The curve will be determined
             * by the algorithm, not the key type.
             */
            case PSA_ECC_FAMILY_TWISTED_EDWARDS:
                switch (keyBits)
                {
                    case 255:
                        oldKeyType = base | OLD_PSA_ECC_CURVE_CURVE25519;
                        break;
                }
                break;
        }
    }
    return oldKeyType;
}

/****************************************************************/
/* MBEDTLS/MBEDCRYPTO key type value mapping functions*/
/****************************************************************/

psa_key_type_t map_KeyStoreKeyType_to_PSAKeyType(psa_key_type_t keyType)
{

    psa_key_type_t newKeyType = keyType;

    if (!OLD_PSA_KEY_TYPE_IS_ECC(keyType))
    {
        switch (keyType)
        {
            case OLD_PSA_KEY_TYPE_AES:
                newKeyType = PSA_KEY_TYPE_AES;
                break;
            case OLD_PSA_KEY_TYPE_ARC4:
                newKeyType = PSA_KEY_TYPE_ARC4;
                break;
            case OLD_PSA_KEY_TYPE_CAMELLIA:
                newKeyType = PSA_KEY_TYPE_CAMELLIA;
                break;
            case OLD_PSA_KEY_TYPE_CHACHA20:
                newKeyType = PSA_KEY_TYPE_CHACHA20;
                break;
            case OLD_PSA_KEY_TYPE_DERIVE:
                newKeyType = PSA_KEY_TYPE_DERIVE;
                break;
            case OLD_PSA_KEY_TYPE_DES:
                newKeyType = PSA_KEY_TYPE_DES;
                break;
            case OLD_PSA_KEY_TYPE_HMAC:
                newKeyType = PSA_KEY_TYPE_HMAC;
                break;
            case OLD_PSA_KEY_TYPE_RAW_DATA:
                newKeyType = PSA_KEY_TYPE_RAW_DATA;
                break;
            case OLD_PSA_KEY_TYPE_RSA_KEY_PAIR:
                newKeyType = PSA_KEY_TYPE_RSA_KEY_PAIR;
                break;
            case OLD_PSA_KEY_TYPE_RSA_PUBLIC_KEY:
                newKeyType = PSA_KEY_TYPE_RSA_PUBLIC_KEY;
                break;
        }
    }
    else
    {
        /* remove the base that was ORed with curve and match with a curve, if possible */
        psa_key_type_t base = OLD_PSA_KEY_TYPE_IS_ECC_KEY_PAIR(keyType) ? PSA_KEY_TYPE_ECC_KEY_PAIR_BASE
                                                                        : PSA_KEY_TYPE_ECC_PUBLIC_KEY_BASE;
        keyType             = OLD_PSA_KEY_TYPE_GET_CURVE(keyType);
        switch (keyType)
        {
            case OLD_PSA_ECC_CURVE_BRAINPOOL_P256R1:
            case OLD_PSA_ECC_CURVE_BRAINPOOL_P384R1:
            case OLD_PSA_ECC_CURVE_BRAINPOOL_P512R1:
                newKeyType = base | PSA_ECC_FAMILY_BRAINPOOL_P_R1;
                break;
            case OLD_PSA_ECC_CURVE_SECP160K1:
            case OLD_PSA_ECC_CURVE_SECP192K1:
            case OLD_PSA_ECC_CURVE_SECP224K1:
            case OLD_PSA_ECC_CURVE_SECP256K1:
                newKeyType = base | PSA_ECC_FAMILY_SECP_K1;
                break;
            case OLD_PSA_ECC_CURVE_SECP192R1:
            case OLD_PSA_ECC_CURVE_SECP224R1:
            case OLD_PSA_ECC_CURVE_SECP256R1:
            case OLD_PSA_ECC_CURVE_SECP384R1:
            case OLD_PSA_ECC_CURVE_SECP521R1:
                newKeyType = base | PSA_ECC_FAMILY_SECP_R1;
                break;
            case OLD_PSA_ECC_CURVE_SECP160R2:
                newKeyType = base | PSA_ECC_FAMILY_SECP_R2;
                break;
            case OLD_PSA_ECC_CURVE_SECT163K1:
            case OLD_PSA_ECC_CURVE_SECT233K1:
            case OLD_PSA_ECC_CURVE_SECT239K1:
            case OLD_PSA_ECC_CURVE_SECT283K1:
            case OLD_PSA_ECC_CURVE_SECT409K1:
            case OLD_PSA_ECC_CURVE_SECT571K1:
                newKeyType = base | PSA_ECC_FAMILY_SECT_K1;
                break;
            case OLD_PSA_ECC_CURVE_SECT163R1:
            case OLD_PSA_ECC_CURVE_SECT193R1:
            case OLD_PSA_ECC_CURVE_SECT233R1:
            case OLD_PSA_ECC_CURVE_SECT283R1:
            case OLD_PSA_ECC_CURVE_SECT409R1:
            case OLD_PSA_ECC_CURVE_SECT571R1:
                newKeyType = base | PSA_ECC_FAMILY_SECT_R1;
                break;
            case OLD_PSA_ECC_CURVE_SECT163R2:
            case OLD_PSA_ECC_CURVE_SECT193R2:
                newKeyType = base | PSA_ECC_FAMILY_SECT_R2;
                break;
            case OLD_PSA_ECC_CURVE_CURVE25519:
            case OLD_PSA_ECC_CURVE_CURVE448:
                newKeyType = base | PSA_ECC_FAMILY_MONTGOMERY;
                break;
        }
    }
    return newKeyType;
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

psa_algorithm_t map_PSAKeyAlg_to_KeyStoreKeyAlg(psa_algorithm_t alg)
{

    psa_algorithm_t oldAlg = alg;

    if (alg == PSA_ALG_ECDH)
    {
        oldAlg = OLD_PSA_ALG_ECDH;
    }
    else if (alg == PSA_ALG_ECB_NO_PADDING)
    {
        oldAlg = OLD_PSA_ALG_ECB_NO_PADDING;
    }
    else if (alg == PSA_ALG_CCM)
    {
        oldAlg = OLD_PSA_ALG_CCM;
    }
    else if (alg == PSA_ALG_CBC_NO_PADDING)
    {
        oldAlg = OLD_PSA_ALG_CBC_NO_PADDING;
    }
    else if (alg == PSA_ALG_CTR)
    {
        oldAlg = OLD_PSA_ALG_CTR;
    }
    else if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(alg) == PSA_ALG_CCM)
    {
        oldAlg = OLD_PSA_ALG_CCM;
    }
    else if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(alg) == PSA_ALG_GCM)
    {
        oldAlg = OLD_PSA_ALG_GCM;
    }
    else if (alg == PSA_ALG_SHA_224)
    {
        oldAlg = OLD_PSA_ALG_SHA_224;
    }
    else if (alg == PSA_ALG_SHA_256)
    {
        oldAlg = OLD_PSA_ALG_SHA_256;
    }
    else if (alg == PSA_ALG_SHA_384)
    {
        oldAlg = OLD_PSA_ALG_SHA_384;
    }
    else if (alg == PSA_ALG_SHA_512)
    {
        oldAlg = OLD_PSA_ALG_SHA_512;
    }
    else if (PSA_ALG_FULL_LENGTH_MAC(alg) == PSA_ALG_HMAC(PSA_ALG_SHA_224))
    {
        oldAlg = OLD_PSA_ALG_SHA_224;
    }
    else if (PSA_ALG_FULL_LENGTH_MAC(alg) == PSA_ALG_HMAC(PSA_ALG_SHA_256))
    {
        oldAlg = OLD_PSA_ALG_SHA_256;
    }
    else if (PSA_ALG_FULL_LENGTH_MAC(alg) == PSA_ALG_HMAC(PSA_ALG_SHA_384))
    {
        oldAlg = OLD_PSA_ALG_SHA_384;
    }
    else if (PSA_ALG_FULL_LENGTH_MAC(alg) == PSA_ALG_HMAC(PSA_ALG_SHA_512))
    {
        oldAlg = OLD_PSA_ALG_SHA_512;
    }
    else if (PSA_ALG_FULL_LENGTH_MAC(alg) == PSA_ALG_CBC_MAC)
    {
        oldAlg = OLD_PSA_ALG_CBC_MAC;
    }
    else if (PSA_ALG_FULL_LENGTH_MAC(alg) == PSA_ALG_CMAC)
    {
        oldAlg = OLD_PSA_ALG_CMAC;
    }
    else if (alg == PSA_ALG_ED25519PH)
    {
        /* no clear mapping for this algorithm, so it is arbitrarily mapped to ECDH */
        oldAlg = OLD_PSA_ALG_ECDH;
    }

    return oldAlg;
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