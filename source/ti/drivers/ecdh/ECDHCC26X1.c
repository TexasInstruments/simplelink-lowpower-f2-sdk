/*
 * Copyright (c) 2020-2022, Texas Instruments Incorporated
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
#include <string.h>

#include <ti/drivers/ecdh/ECDHCC26X1.h>
#include <ti/drivers/ECDH.h>
#include <ti/drivers/cryptoutils/ecc/ECCInitCC26X1.h>
#include <ti/drivers/cryptoutils/ecc/ECCParams.h>
#include <ti/drivers/cryptoutils/utils/CryptoUtils.h>

#include <ti/drivers/dpl/HwiP.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rom.h)
#include DeviceFamily_constructPath(driverlib/rom_ecc.h)

/* Defines */

/* Octet string format requires an extra byte at the start of the public key */
#define OCTET_STRING_OFFSET 1

/* Param length needs to be equal to the length of the largest curve supported plus length offset bytes */
#define ECC_PARAM_LENGTH_WITH_OFFSET_BYTES (ECC_NISTP256_PARAM_LENGTH_BYTES + ECC_LENGTH_OFFSET_BYTES)

/* Typedefs */
typedef union
{
    uint32_t word[ECC_PARAM_LENGTH_WITH_OFFSET_BYTES / sizeof(uint32_t)];
    uint8_t byte[ECC_PARAM_LENGTH_WITH_OFFSET_BYTES];
} ECC_Param;

/* Externs */
extern uint8_t ECCSW_X25519_commonKey(ECC_State *state,
                                      const uint32_t *ecdhCommonKey_inScal,
                                      const uint32_t *ecdhCommonKey_inX,
                                      uint32_t *ecdhCommonKey_outX);
/*
 *  ======== ECDHCC26X1_getKeyResult ========
 */
static void ECDHCC26X1_getKeyResult(const ECC_Param *x,
                                    const ECC_Param *y,
                                    CryptoKey *key,
                                    const ECCParams_CurveParams *curve,
                                    ECDH_KeyMaterialEndianness keyMaterialEndianness)
{
    /*
     * Reverse and copy the X and Y coordinates back to the CryptoKey buffer.
     * Octet string format requires big-endian formatting of X and Y coordinates.
     * Copy directly for little-endian key material.
     */
    if (keyMaterialEndianness == ECDH_BIG_ENDIAN_KEY)
    {
        CryptoUtils_reverseCopy(&x->word[1], key->u.plaintext.keyMaterial + OCTET_STRING_OFFSET, curve->length);

        if (curve->curveType == ECCParams_CURVE_TYPE_MONTGOMERY)
        {
            /* Y coordinate is not used. Zero it out. */
            memset(key->u.plaintext.keyMaterial + curve->length + OCTET_STRING_OFFSET, 0x00, curve->length);
        }
        else
        {
            CryptoUtils_reverseCopy(&y->word[1],
                                    key->u.plaintext.keyMaterial + curve->length + OCTET_STRING_OFFSET,
                                    curve->length);
        }
        /* Write the octet string format constant to the CryptoKey buffer */
        key->u.plaintext.keyMaterial[0] = 0x04;
    }
    else
    {
        memcpy(key->u.plaintext.keyMaterial, &x->word[1], curve->length);

        if (curve->curveType != ECCParams_CURVE_TYPE_MONTGOMERY)
        {
            memcpy(key->u.plaintext.keyMaterial + curve->length, &y->word[1], curve->length);
        }
    }

    /* Mark the CryptoKey as non-empty */
    key->encoding = CryptoKey_PLAINTEXT;
}

/*
 *  ======== ECDHCC26X1_getKeyResultMontgomery ========
 */
static void ECDHCC26X1_getKeyResultMontgomery(const ECC_Param *x, CryptoKey *key, size_t curveLength)
{
    /* Copy the X coordinate back to the CryptoKey buffer. No Y coordinate.
     * Montgomery X-only public key format is little-endian. */
    memcpy(key->u.plaintext.keyMaterial, &x->word[1], curveLength);

    /* Mark the CryptoKey as non-empty */
    key->encoding = CryptoKey_PLAINTEXT;
}

/*
 *  ======== ECDH_init ========
 */
void ECDH_init(void)
{}

/*
 *  ======== ECDH_Params_init ========
 */
void ECDH_Params_init(ECDH_Params *params)
{
    *params = ECDH_defaultParams;
}

/*
 *  ======== ECDH_construct ========
 */
ECDH_Handle ECDH_construct(ECDH_Config *config, const ECDH_Params *params)
{
    ECDH_Handle handle;
    ECDHCC26X1_Object *object;
    uint_fast8_t key;

    handle = (ECDH_Handle)config;
    object = handle->object;

    /* If params are NULL, use defaults */
    if (params == NULL)
    {
        params = &ECDH_defaultParams;
    }

    /* Since CC26X1 ECC is a pure SW implementation, callback mode is not supported */
    if (params->returnBehavior == ECDH_RETURN_BEHAVIOR_CALLBACK)
    {
        return NULL;
    }

    key = HwiP_disable();

    if (object->isOpen)
    {
        HwiP_restore(key);
        return NULL;
    }

    object->isOpen = true;

    HwiP_restore(key);

    object->returnBehavior = params->returnBehavior;
    object->callbackFxn    = params->callbackFxn;

    return handle;
}

/*
 *  ======== ECDH_close ========
 */
void ECDH_close(ECDH_Handle handle)
{
    ECDHCC26X1_Object *object;

    /* Get the pointer to the object */
    object = handle->object;

    /* Mark the module as available */
    object->isOpen = false;
}

/*
 *  ======== ECDHCC26X1_formatX25519PrivateKey ========
 */
static void ECDHCC26X1_formatX25519PrivateKey(uint32_t *myPrivateKey, ECDH_KeyMaterialEndianness KeyMaterialEndianness)
{
    /*
     * Per RFC7748: For X25519, in order to decode 32 random bytes as an integer scalar,
     * set the three least significant bits of the first byte and the most significant bit
     * of the last to zero, set the second most significant bit of the last byte to 1.
     */

    /* Clear bit 0, 1, and 2 */
    myPrivateKey[0] &= 0xFFFFFFF8UL;
    /* Clear bit 255 */
    myPrivateKey[7] &= 0x7FFFFFFFUL;
    /* Set bit 254 */
    myPrivateKey[7] |= 0x40000000UL;
}

/*
 *  ======== ECDH_generatePublicKey ========
 */
int_fast16_t ECDH_generatePublicKey(ECDH_Handle handle, ECDH_OperationGeneratePublicKey *operation)
{
    ECDHCC26X1_Object *object = handle->object;
    int_fast16_t returnStatus = ECDH_STATUS_ERROR;
    size_t curveLength        = operation->curve->length;
    uint8_t eccStatus;

    /* Allocate local copies of the private and public keys because
     * the ECC in ROM implementation requires the word length to be prepended to
     * every array input.
     */
    ECC_Param privateKeyUnion;
    ECC_Param publicKeyUnionX;
    ECC_Param publicKeyUnionY;

    /* Prepend the word length - always 8 words for both P256 and Curve25519 */
    privateKeyUnion.word[0] = 0x08;
    publicKeyUnionX.word[0] = 0x08;
    publicKeyUnionY.word[0] = 0x08;

    /* Only NISTP256 and Curve25519 curves are supported. Both curves have 32-bytes length. */
    if (curveLength != ECC_NISTP256_PARAM_LENGTH_BYTES)
    {
        return ECDH_STATUS_ERROR;
    }

    /* Validate private key length */
    if (operation->myPrivateKey->u.plaintext.keyLength != curveLength)
    {
        return ECDH_STATUS_INVALID_KEY_SIZE;
    }

    /*
     * Validate public key sizes to ensure X-only public key format if using Montgomery curves
     * with little endian key representation. Other cases use both coordinates with additional
     * octet string offset byte when using big endian representation
     */
    if (operation->keyMaterialEndianness == ECDH_LITTLE_ENDIAN_KEY)
    {
        /* X-only public key for Montgomery curves */
        if (operation->curve->curveType == ECCParams_CURVE_TYPE_MONTGOMERY)
        {
            if (operation->myPublicKey->u.plaintext.keyLength != curveLength)
            {
                return ECDH_STATUS_INVALID_KEY_SIZE;
            }
        }
        else /* Little-endian keys for other curves */
        {
            if (operation->myPublicKey->u.plaintext.keyLength != 2 * curveLength)
            {
                return ECDH_STATUS_INVALID_KEY_SIZE;
            }
        }
    }
    else /* Octet string format for any curve */
    {
        if (operation->myPublicKey->u.plaintext.keyLength != (2 * curveLength + OCTET_STRING_OFFSET))
        {
            return ECDH_STATUS_INVALID_KEY_SIZE;
        }
    }

    /*
     * Since we are receiving the private key in octet string format,
     * we need to convert them to little-endian form for use with the ECC in
     * ROM functions. Length is 32-bytes for both NISTP256 and Curve25519.
     * If the private key is already in little-endian form, skip this conversion
     * and directly copy the key.
     */
    if (operation->keyMaterialEndianness == ECDH_BIG_ENDIAN_KEY)
    {
        CryptoUtils_reverseCopyPad(operation->myPrivateKey->u.plaintext.keyMaterial,
                                   &privateKeyUnion.word[1],
                                   curveLength);
    }
    else
    {
        CryptoUtils_copyPad(operation->myPrivateKey->u.plaintext.keyMaterial, &privateKeyUnion.word[1], curveLength);
    }

    /* Assume Montgomery curve type is Curve25519 since ECC params for all other curves
     * such as NIST are provided as Weierstrass type.
     */
    if (operation->curve->curveType == ECCParams_CURVE_TYPE_MONTGOMERY)
    {
        ECDHCC26X1_formatX25519PrivateKey(&privateKeyUnion.word[1], operation->keyMaterialEndianness);

        ECCInitCC26X1_Curve25519(&(object->eccState), ECDH26X1_ECC_WINDOW_SIZE, object->eccWorkZone);

        eccStatus = ECCSW_X25519_commonKey(&(object->eccState),
                                           privateKeyUnion.word,
                                           ECC_Curve25519_generatorX.word,
                                           publicKeyUnionX.word);
    }
    else /* Weierstrass curve type (assume NIST P256) */
    {
        /* Initialize ECC state for NIST P256 with a window size of 3 */
        ECC_initialize(&(object->eccState), object->eccWorkZone);

        /* Check if private key in [1, n-1] */
        eccStatus = ECC_validatePrivateKey(&(object->eccState), privateKeyUnion.word);

        if (eccStatus == STATUS_PRIVATE_VALID)
        {
            eccStatus = ECC_generateKey(&(object->eccState),
                                        privateKeyUnion.word,
                                        privateKeyUnion.word,
                                        publicKeyUnionX.word,
                                        publicKeyUnionY.word);
        }
    }

    /* Check the ECC in ROM return code and set the driver status accordingly */
    if (eccStatus == STATUS_ECDH_KEYGEN_OK)
    {
        returnStatus = ECDH_STATUS_SUCCESS;

        if ((operation->curve->curveType == ECCParams_CURVE_TYPE_MONTGOMERY) &&
            (operation->keyMaterialEndianness == ECDH_LITTLE_ENDIAN_KEY))
        {
            ECDHCC26X1_getKeyResultMontgomery(&publicKeyUnionX, operation->myPublicKey, curveLength);
        }
        else
        {
            ECDHCC26X1_getKeyResult(&publicKeyUnionX,
                                    &publicKeyUnionY,
                                    operation->myPublicKey,
                                    operation->curve,
                                    operation->keyMaterialEndianness);
        }
    }

    return returnStatus;
}

/*
 *  ======== ECDH_computeSharedSecret ========
 */
int_fast16_t ECDH_computeSharedSecret(ECDH_Handle handle, ECDH_OperationComputeSharedSecret *operation)
{
    ECDHCC26X1_Object *object = handle->object;
    int_fast16_t returnStatus = ECDH_STATUS_ERROR;
    size_t curveLength        = operation->curve->length;
    uint8_t eccStatus;

    /* We need to allocate local copies of the private key, public key, and
     * shared secret because the ECC in ROM implementation requires the word
     * length to be prepended to every array input.
     * The length word is prepended during initialisation here.
     */
    ECC_Param privateKeyUnion;
    ECC_Param publicKeyUnionX;
    ECC_Param publicKeyUnionY;
    ECC_Param sharedSecretUnionX;
    ECC_Param sharedSecretUnionY;

    /* Prepend the word length - always 8 words for both P256 and Curve25519 */
    privateKeyUnion.word[0]    = 0x08;
    publicKeyUnionX.word[0]    = 0x08;
    publicKeyUnionY.word[0]    = 0x08;
    sharedSecretUnionX.word[0] = 0x08;
    sharedSecretUnionY.word[0] = 0x08;

    /* Only NISTP256 and Curve25519 curves are supported. Both curves have 32-bytes length. */
    if (curveLength != ECC_NISTP256_PARAM_LENGTH_BYTES)
    {
        return ECDH_STATUS_ERROR;
    }

    /*
     * Validate public key sizes to ensure X-only public key format if using Montgomery curves
     * with little endian key representation. Other cases use both coordinates with additional
     * octet string offset byte when using big endian representation
     */
    if (operation->keyMaterialEndianness == ECDH_LITTLE_ENDIAN_KEY)
    {
        /* X-only public key for Montgomery curves */
        if (operation->curve->curveType == ECCParams_CURVE_TYPE_MONTGOMERY)
        {
            if ((operation->myPrivateKey->u.plaintext.keyLength != curveLength) ||
                (operation->theirPublicKey->u.plaintext.keyLength != curveLength) ||
                (operation->sharedSecret->u.plaintext.keyLength != curveLength))
            {
                return ECDH_STATUS_INVALID_KEY_SIZE;
            }
        }
        else /* Little-endian keys for other curves */
        {
            if ((operation->myPrivateKey->u.plaintext.keyLength != curveLength) ||
                (operation->theirPublicKey->u.plaintext.keyLength != 2 * curveLength) ||
                (operation->sharedSecret->u.plaintext.keyLength != 2 * curveLength))
            {
                return ECDH_STATUS_INVALID_KEY_SIZE;
            }
        }
    }
    else /* Octet string format for any curve */
    {
        if ((operation->myPrivateKey->u.plaintext.keyLength != curveLength) ||
            (operation->theirPublicKey->u.plaintext.keyLength != (2 * curveLength + OCTET_STRING_OFFSET)) ||
            (operation->theirPublicKey->u.plaintext.keyMaterial[0] != 0x04) ||
            (operation->sharedSecret->u.plaintext.keyLength != (2 * curveLength + OCTET_STRING_OFFSET)))
        {
            return ECDH_STATUS_INVALID_KEY_SIZE;
        }
    }

    /*
     * Since we are receiving the private and public keys in octet string format,
     * we need to convert them to little-endian form for use with the ECC in
     * ROM functions
     * If the private key is already in little-endian form, skip this conversion
     * and directly copy the key.
     */
    if (operation->keyMaterialEndianness == ECDH_BIG_ENDIAN_KEY)
    {
        CryptoUtils_reverseCopyPad(operation->myPrivateKey->u.plaintext.keyMaterial,
                                   &privateKeyUnion.word[1],
                                   curveLength);

        CryptoUtils_reverseCopyPad(operation->theirPublicKey->u.plaintext.keyMaterial + OCTET_STRING_OFFSET,
                                   &publicKeyUnionX.word[1],
                                   curveLength);

        CryptoUtils_reverseCopyPad(operation->theirPublicKey->u.plaintext.keyMaterial + curveLength +
                                       OCTET_STRING_OFFSET,
                                   &publicKeyUnionY.word[1],
                                   curveLength);
    }
    else
    {
        CryptoUtils_copyPad(operation->myPrivateKey->u.plaintext.keyMaterial, &privateKeyUnion.word[1], curveLength);

        CryptoUtils_copyPad(operation->theirPublicKey->u.plaintext.keyMaterial, &publicKeyUnionX.word[1], curveLength);

        CryptoUtils_copyPad(operation->theirPublicKey->u.plaintext.keyMaterial + curveLength,
                            &publicKeyUnionY.word[1],
                            curveLength);
    }

    /* Assume Montgomery curve type is Curve25519 since ECC params for all other curves
     * such as NIST are provided as Weierstrass type.
     */
    if (operation->curve->curveType == ECCParams_CURVE_TYPE_MONTGOMERY)
    {
        ECDHCC26X1_formatX25519PrivateKey(&privateKeyUnion.word[1], operation->keyMaterialEndianness);

        ECCInitCC26X1_Curve25519(&(object->eccState), ECDH26X1_ECC_WINDOW_SIZE, object->eccWorkZone);

        eccStatus = ECCSW_X25519_commonKey(&(object->eccState),
                                           privateKeyUnion.word,
                                           ECC_Curve25519_generatorX.word,
                                           sharedSecretUnionX.word);
    }
    else /* Weierstrass curve type (assume NIST P256) */
    {
        /* Initialize ECC state for NIST P256 with a window size of 3 */
        ECC_initialize(&(object->eccState), object->eccWorkZone);

        eccStatus = ECC_validatePublicKey(&(object->eccState), publicKeyUnionX.word, publicKeyUnionY.word);

        if (eccStatus == STATUS_ECC_POINT_ON_CURVE)
        {
            eccStatus = ECC_ECDH_computeSharedSecret(&(object->eccState),
                                                     privateKeyUnion.word,
                                                     publicKeyUnionX.word,
                                                     publicKeyUnionY.word,
                                                     sharedSecretUnionX.word,
                                                     sharedSecretUnionY.word);
        }
    }

    /* Check the ECC in ROM return code and set the driver status accordingly */
    if (eccStatus == STATUS_ECDH_COMMON_KEY_OK)
    {
        returnStatus = ECDH_STATUS_SUCCESS;

        if ((operation->keyMaterialEndianness == ECDH_LITTLE_ENDIAN_KEY) &&
            (operation->curve->curveType == ECCParams_CURVE_TYPE_MONTGOMERY))
        {
            ECDHCC26X1_getKeyResultMontgomery(&sharedSecretUnionX, operation->sharedSecret, curveLength);
        }
        else
        {
            ECDHCC26X1_getKeyResult(&sharedSecretUnionX,
                                    &sharedSecretUnionY,
                                    operation->sharedSecret,
                                    operation->curve,
                                    operation->keyMaterialEndianness);
        }
    }

    return returnStatus;
}
