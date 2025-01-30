/*
 * Copyright (c) 2017-2024, Texas Instruments Incorporated
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
#include <stdbool.h>
#include <string.h>

#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26X2.h>
#include <ti/drivers/ECJPAKE.h>
#include <ti/drivers/ecjpake/ECJPAKECC26X2.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>
#include <ti/drivers/cryptoutils/sharedresources/PKAResourceCC26XX.h>
#include <ti/drivers/cryptoutils/utils/CryptoUtils.h>
#include <ti/drivers/utils/Math.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_pka.h)
#include DeviceFamily_constructPath(inc/hw_pka_ram.h)
#include DeviceFamily_constructPath(driverlib/pka.h)

#if (ENABLE_KEY_STORAGE == 1)
    #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_helpers.h>
    #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_init.h>
    #if (TFM_ENABLED == 1)
        #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_s.h>
    #endif

    /*
     * Max key sizes 521b private keys,
     * (521b) * 2 + 1B for octet offset for public keys
     */
    #define ECJPAKE_MAX_KEYSTORE_PUBLIC_KEY_SIZE  133
    #define ECJPAKE_MAX_KEYSTORE_PRIVATE_KEY_SIZE 66
#endif

#define SCRATCH_BUFFER_0_SIZE 256

/* We need several scratch buffers as the private and public keys are provided
 * in octet string form and thus the components are big-endian. We need to feed
 * the PKA with little-endian integers. That means moving them to some scratch
 * memory and reversing them.
 * We are storing them in bytes 512 to 512 + 3 x SCRATCH_BUFFER_SIZE.
 * Byte 1024 to 1024 + 66 and 1536 to 1536 + 66 are used by the public key
 * validation routine in driverlib.
 */
#define SCRATCH_KEY_OFFSET 512
#define SCRATCH_KEY_SIZE   96

#define SCRATCH_PRIVATE_KEY ((uint32_t *)(PKA_RAM_BASE + SCRATCH_KEY_OFFSET))
#define SCRATCH_PUBLIC_X    ((uint32_t *)(PKA_RAM_BASE + SCRATCH_KEY_OFFSET + 1 * SCRATCH_KEY_SIZE))
#define SCRATCH_PUBLIC_Y    ((uint32_t *)(PKA_RAM_BASE + SCRATCH_KEY_OFFSET + 2 * SCRATCH_KEY_SIZE))

#define SCRATCH_BUFFER_OFFSET 1024
#define SCRATCH_BUFFER_SIZE   256

#define SCRATCH_BUFFER_0 ((uint32_t *)(PKA_RAM_BASE + SCRATCH_BUFFER_OFFSET + 0 * SCRATCH_BUFFER_SIZE))
#define SCRATCH_BUFFER_2 ((uint32_t *)(PKA_RAM_BASE + SCRATCH_BUFFER_OFFSET + 1 * SCRATCH_BUFFER_SIZE))

/* Octet string format requires an extra byte at the start of the public key */
#define OCTET_STRING_OFFSET 1

/* Forward declarations */
static void ECJPAKECC26X2_hwiFxn(uintptr_t arg0);
#if (TFM_ENABLED == 0)
static void ECJPAKECC26X2_internalCallbackFxn(ECJPAKE_Handle handle,
                                              int_fast16_t returnStatus,
                                              ECJPAKE_Operation operation,
                                              ECJPAKE_OperationType operationType);
#endif
static int_fast16_t ECJPAKECC26X2_waitForAccess(ECJPAKE_Handle handle);
static int_fast16_t ECJPAKECC26X2_waitForResult(ECJPAKE_Handle handle);
static int_fast16_t ECJPAKECC26X2_runFSM(ECJPAKE_Handle handle);
static int_fast16_t ECJPAKECC26X2_convertReturnValue(uint32_t pkaResult);

/* Extern globals */
extern const ECJPAKE_Params ECJPAKE_defaultParams;

/* Static globals */
static bool isInitialized = false;

static uint32_t scratchBufferSize = SCRATCH_BUFFER_0_SIZE;

#if (ENABLE_KEY_STORAGE == 1)
uint8_t ECJPAKECC26X2_keyStorePrivateKeyMaterial[ECJPAKE_MAX_KEYSTORE_PRIVATE_KEY_SIZE];
uint8_t ECJPAKECC26X2_keyStorePreSharedSecretMaterial[ECJPAKE_MAX_KEYSTORE_PRIVATE_KEY_SIZE];
uint8_t ECJPAKECC26X2_keyStorePublicKeyMaterial[ECJPAKE_MAX_KEYSTORE_PUBLIC_KEY_SIZE];
uint8_t ECJPAKECC26X2_keyStoreSharedSecretMaterial[ECJPAKE_MAX_KEYSTORE_PUBLIC_KEY_SIZE];

/*
 *  ======== ECJPAKE_isPersistentLifetime ========
 */
bool ECJPAKE_isPersistentLifetime(CryptoKey *key)
{
    KeyStore_PSA_KeyAttributes *attributesPtr;

    #if (TFM_ENABLED == 1)
    KeyStore_PSA_KeyAttributes attributes = KEYSTORE_PSA_KEY_ATTRIBUTES_INIT;
    attributesPtr                         = &attributes;
    (void)KeyStore_s_copyKeyAttributesFromClient((struct psa_client_key_attributes_s *)key->u.keyStore.keyAttributes,
                                                 KEYSTORE_PSA_DEFAULT_OWNER,
                                                 attributesPtr);
    #else
    attributesPtr  = (KeyStore_PSA_KeyAttributes *)key->u.keyStore.keyAttributes;
    #endif

    if (attributesPtr->MBEDTLS_PRIVATE(core).MBEDTLS_PRIVATE(lifetime) == KEYSTORE_PSA_KEY_LIFETIME_PERSISTENT)
    {
        return true;
    }
    else
    {
        return false;
    }
}
#endif

/*
 *  ======== ECJPAKE_getPreSharedSecret ========
 */
int_fast16_t ECJPAKE_getPreSharedSecret(const CryptoKey *preSharedSecret,
                                        uint8_t **preSharedSecretMaterial,
                                        size_t *preSharedSecretLength)
{
    int_fast16_t status;

#if (ENABLE_KEY_STORAGE == 1)
    int_fast16_t keyStoreStatus;
    KeyStore_PSA_KeyFileId keyID;
#endif
    if (preSharedSecret->encoding == CryptoKey_PLAINTEXT)
    {
        *preSharedSecretMaterial = preSharedSecret->u.plaintext.keyMaterial;
        *preSharedSecretLength   = preSharedSecret->u.plaintext.keyLength;
        status                   = ECJPAKE_STATUS_SUCCESS;
    }
#if (ENABLE_KEY_STORAGE == 1)
    else if (preSharedSecret->encoding == CryptoKey_KEYSTORE)
    {
        memset(ECJPAKECC26X2_keyStorePreSharedSecretMaterial, 0, sizeof(ECJPAKECC26X2_keyStorePreSharedSecretMaterial));
        /* Obtain the private key from keystore */
        GET_KEY_ID(keyID, preSharedSecret->u.keyStore.keyID);

        keyStoreStatus = KeyStore_PSA_getKey(keyID,
                                             ECJPAKECC26X2_keyStorePreSharedSecretMaterial,
                                             sizeof(ECJPAKECC26X2_keyStorePreSharedSecretMaterial),
                                             preSharedSecretLength,
                                             KEYSTORE_PSA_ALG_PAKE,
                                             KEYSTORE_PSA_KEY_USAGE_DERIVE);
        if ((keyStoreStatus != KEYSTORE_PSA_STATUS_SUCCESS) ||
            (*preSharedSecretLength != preSharedSecret->u.keyStore.keyLength))
        {
            status = ECJPAKE_STATUS_KEYSTORE_ERROR;
        }
        else
        {
            *preSharedSecretMaterial = ECJPAKECC26X2_keyStorePreSharedSecretMaterial;
            status                   = ECJPAKE_STATUS_SUCCESS;
        }
    }
#endif
    else
    {
        status = ECJPAKE_STATUS_ERROR;
    }

    return status;
}

/*
 *  ======== ECJPAKE_getPrivateKey ========
 */
int_fast16_t ECJPAKE_getPrivateKey(const CryptoKey *privateKey, uint8_t **privateKeyMaterial, size_t *privateKeyLength)
{
    int_fast16_t status;

#if (ENABLE_KEY_STORAGE == 1)
    int_fast16_t keyStoreStatus;
    KeyStore_PSA_KeyFileId keyID;
#endif
    if (privateKey->encoding == CryptoKey_PLAINTEXT)
    {
        *privateKeyMaterial = privateKey->u.plaintext.keyMaterial;
        *privateKeyLength   = privateKey->u.plaintext.keyLength;
        status              = ECJPAKE_STATUS_SUCCESS;
    }
#if (ENABLE_KEY_STORAGE == 1)
    else if (privateKey->encoding == CryptoKey_KEYSTORE)
    {
        memset(ECJPAKECC26X2_keyStorePrivateKeyMaterial, 0, sizeof(ECJPAKECC26X2_keyStorePrivateKeyMaterial));
        /* Obtain the private key from keystore */
        GET_KEY_ID(keyID, privateKey->u.keyStore.keyID);

        keyStoreStatus = KeyStore_PSA_getKey(keyID,
                                             ECJPAKECC26X2_keyStorePrivateKeyMaterial,
                                             sizeof(ECJPAKECC26X2_keyStorePrivateKeyMaterial),
                                             privateKeyLength,
                                             KEYSTORE_PSA_ALG_PAKE,
                                             KEYSTORE_PSA_KEY_USAGE_DERIVE);
        if ((keyStoreStatus != KEYSTORE_PSA_STATUS_SUCCESS) || (*privateKeyLength != privateKey->u.keyStore.keyLength))
        {
            status = ECJPAKE_STATUS_KEYSTORE_ERROR;
        }
        else
        {
            *privateKeyMaterial = ECJPAKECC26X2_keyStorePrivateKeyMaterial;
            status              = ECJPAKE_STATUS_SUCCESS;
        }
    }
#endif
    else
    {
        status = ECJPAKE_STATUS_ERROR;
    }

    return status;
}

/*
 *  ======== ECJPAKE_getPublicKey ========
 */
int_fast16_t ECJPAKE_getPublicKey(const CryptoKey *publicKey, uint8_t **publicKeyMaterial, size_t *publicKeyLength)
{
    int_fast16_t status;

#if (ENABLE_KEY_STORAGE == 1)
    int_fast16_t keyStoreStatus;
    KeyStore_PSA_KeyFileId keyID;
#endif
    if (publicKey->encoding == CryptoKey_PLAINTEXT)
    {
        *publicKeyMaterial = publicKey->u.plaintext.keyMaterial;
        *publicKeyLength   = publicKey->u.plaintext.keyLength;
        status             = ECJPAKE_STATUS_SUCCESS;
    }
#if (ENABLE_KEY_STORAGE == 1)
    else if (publicKey->encoding == CryptoKey_KEYSTORE)
    {
        memset(ECJPAKECC26X2_keyStorePublicKeyMaterial, 0, sizeof(ECJPAKECC26X2_keyStorePublicKeyMaterial));
        /* Obtain the public key from keystore */
        GET_KEY_ID(keyID, publicKey->u.keyStore.keyID);

        keyStoreStatus = KeyStore_PSA_getKey(keyID,
                                             ECJPAKECC26X2_keyStorePublicKeyMaterial,
                                             sizeof(ECJPAKECC26X2_keyStorePublicKeyMaterial),
                                             publicKeyLength,
                                             KEYSTORE_PSA_ALG_PAKE,
                                             KEYSTORE_PSA_KEY_USAGE_DERIVE);
        if (keyStoreStatus != KEYSTORE_PSA_STATUS_SUCCESS || (*publicKeyLength != publicKey->u.keyStore.keyLength))
        {
            status = ECJPAKE_STATUS_KEYSTORE_ERROR;
        }
        else
        {
            *publicKeyMaterial = ECJPAKECC26X2_keyStorePublicKeyMaterial;
            status             = ECJPAKE_STATUS_SUCCESS;
        }
    }
#endif
    else
    {
        status = ECJPAKE_STATUS_ERROR;
    }

    return status;
}

/*
 *  ======== ECJPAKE_getSharedSecretBuffer ========
 */
int_fast16_t ECJPAKE_getSharedSecretBuffer(const CryptoKey *sharedSecret, uint8_t **sharedSecretMaterial)
{
    int_fast16_t status;
    if (sharedSecret->encoding == CryptoKey_BLANK_PLAINTEXT)
    {
        *sharedSecretMaterial = sharedSecret->u.plaintext.keyMaterial;
        status                = ECJPAKE_STATUS_SUCCESS;
    }
#if (ENABLE_KEY_STORAGE == 1)
    else if (sharedSecret->encoding == CryptoKey_BLANK_KEYSTORE)
    {
        *sharedSecretMaterial = ECJPAKECC26X2_keyStoreSharedSecretMaterial;
        status                = ECJPAKE_STATUS_SUCCESS;
    }
#endif
    else
    {
        status = ECJPAKE_STATUS_ERROR;
    }

    return status;
}

/*
 *  ======== ECJPAKECC26X2_generatePublicKeyStart ========
 */
int_fast16_t ECJPAKECC26X2_generatePublicKeyStart(const CryptoKey *privateKey,
                                                  const ECCParams_CurveParams *curve,
                                                  uint32_t *resultAddress)
{
    uint32_t pkaResult;
    uint8_t *privateKeyMaterial;
    size_t privateKeyLength;
    int_fast16_t status;

    status = ECJPAKE_getPrivateKey(privateKey, &privateKeyMaterial, &privateKeyLength);
    if (status != ECJPAKE_STATUS_SUCCESS)
    {
        return status;
    }

    /* Check if Private Key is all zeros */
    if (PKAArrayAllZeros(privateKeyMaterial, curve->length))
    {
        return ECJPAKE_STATUS_INVALID_PRIVATE_KEY;
    }

    /* Since we are receiving the private and public keys in octet string format,
     * we need to convert them to little-endian form for use with the PKA
     */
    CryptoUtils_reverseCopyPad(privateKeyMaterial, SCRATCH_PRIVATE_KEY, curve->length);

    /* Check if Private Key is less than order of the curve */
    PKABigNumCmpStart((uint8_t *)SCRATCH_PRIVATE_KEY, curve->order, curve->length);

    while (PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY) {}

    pkaResult = PKABigNumCmpGetResult();

    if (pkaResult != PKA_STATUS_A_LESS_THAN_B)
    {
        return ECJPAKE_STATUS_INVALID_PRIVATE_KEY;
    }

    PKAEccMultiplyStart((uint8_t *)SCRATCH_PRIVATE_KEY,
                        curve->generatorX,
                        curve->generatorY,
                        curve->prime,
                        curve->a,
                        curve->b,
                        curve->length,
                        resultAddress);

    // If we get to this point, we want to perform another PKA operation
    IntPendClear(INT_PKA_IRQ);
    IntEnable(INT_PKA_IRQ);

    return ECJPAKECC26X2_STATUS_FSM_RUN_PKA_OP;
}

#if (ENABLE_KEY_STORAGE == 1)
/*
 *  ======== ECJPAKECC26X2_importSecureKey ========
 */
static int_fast16_t ECJPAKECC26X2_importSecureKey(CryptoKey *key,
                                                  uint8_t *keyingMaterial,
                                                  ECCParams_CurveType curveType)
{
    KeyStore_PSA_KeyFileId keyID;
    int_fast16_t status         = ECJPAKE_STATUS_KEYSTORE_ERROR;
    int_fast16_t keyStoreResult = KEYSTORE_PSA_STATUS_GENERIC_ERROR;
    KeyStore_PSA_KeyAttributes *attributesPtr;
    #if (TFM_ENABLED == 1)
    KeyStore_PSA_KeyAttributes attributes = KEYSTORE_PSA_KEY_ATTRIBUTES_INIT;
    #endif

    /* Copy key identifier from CryptoKey */
    GET_KEY_ID(keyID, key->u.keyStore.keyID);

    #if (TFM_ENABLED == 0)
    attributesPtr = (KeyStore_PSA_KeyAttributes *)key->u.keyStore.keyAttributes;
    #else
    attributesPtr  = &attributes;
    keyStoreResult = KeyStore_s_copyKeyAttributesFromClient((struct psa_client_key_attributes_s *)
                                                                key->u.keyStore.keyAttributes,
                                                            KEYSTORE_PSA_DEFAULT_OWNER,
                                                            attributesPtr);
    #endif
    keyStoreResult = KeyStore_PSA_importKey(attributesPtr, keyingMaterial, key->u.keyStore.keyLength, &keyID);

    if (keyStoreResult == KEYSTORE_PSA_STATUS_SUCCESS)
    {
        KeyStore_PSA_initKey(key, keyID, key->u.keyStore.keyLength, attributesPtr);
        status = ECJPAKECC26X2_STATUS_FSM_RUN_FSM;
    }
    else
    {
        status = ECJPAKE_STATUS_KEYSTORE_ERROR;
    }

    return status;
}
#endif

/*
 *  ======== ECJPAKECC26X2_getPublicKeyResult ========
 */
uint32_t ECJPAKECC26X2_getPublicKeyResult(CryptoKey *publicKey,
                                          const ECCParams_CurveParams *curve,
                                          uint32_t resultPKAMemAddr)
{
    uint32_t pkaResult;
    uint8_t *keyMaterial;
    int_fast16_t status = ECJPAKE_STATUS_KEYSTORE_ERROR;
#if (ENABLE_KEY_STORAGE == 1)
    uint8_t publicKeyMaterial[ECJPAKE_MAX_KEYSTORE_PUBLIC_KEY_SIZE];
#endif

    if (publicKey->encoding == CryptoKey_BLANK_PLAINTEXT)
    {
        keyMaterial = publicKey->u.plaintext.keyMaterial;
    }
#if (ENABLE_KEY_STORAGE == 1)
    else if (publicKey->encoding == CryptoKey_BLANK_KEYSTORE)
    {
        keyMaterial = publicKeyMaterial;
    }
#endif
    else
    {
        return ECJPAKE_STATUS_ERROR;
    }

    /* Get X and Y coordinates for short Weierstrass curves */
    pkaResult = PKAEccMultiplyGetResult(keyMaterial + OCTET_STRING_OFFSET,
                                        keyMaterial + curve->length + OCTET_STRING_OFFSET,
                                        resultPKAMemAddr,
                                        curve->length);

    /* Set first byte of output public key to 0x04 to indicate x,y
     * big-endian coordinates in octet string format
     */
    keyMaterial[0] = 0x04;

    /* Byte-reverse integer X coordinate for octet string format */
    CryptoUtils_reverseBufferBytewise(keyMaterial + OCTET_STRING_OFFSET, curve->length);

    /* Byte-reverse integer Y coordinate for octet string format */
    CryptoUtils_reverseBufferBytewise(keyMaterial + curve->length + OCTET_STRING_OFFSET, curve->length);

    status = ECJPAKECC26X2_convertReturnValue(pkaResult);

    if (status == ECJPAKECC26X2_STATUS_FSM_RUN_FSM)
    {
        if (publicKey->encoding == CryptoKey_BLANK_PLAINTEXT)
        {
            publicKey->encoding = CryptoKey_PLAINTEXT;
        }
#if (ENABLE_KEY_STORAGE == 1)
        else if (publicKey->encoding == CryptoKey_BLANK_KEYSTORE)
        {
            status = ECJPAKECC26X2_importSecureKey(publicKey, keyMaterial, curve->curveType);
        }
#endif
    }
    return status;
}

#if (TFM_ENABLED == 0)
/*
 *  ======== ECJPAKECC26X2_internalCallbackFxn ========
 */
static void ECJPAKECC26X2_internalCallbackFxn(ECJPAKE_Handle handle,
                                              int_fast16_t returnStatus,
                                              ECJPAKE_Operation operation,
                                              ECJPAKE_OperationType operationType)
{
    ECJPAKECC26X2_Object *object = handle->object;

    /* This function is only ever registered when in ECJPAKE_RETURN_BEHAVIOR_BLOCKING
     * or ECJPAKE_RETURN_BEHAVIOR_POLLING.
     */
    if (object->returnBehavior == ECJPAKE_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_post(&PKAResourceCC26XX_operationSemaphore);
    }
    else
    {
        PKAResourceCC26XX_pollingFlag = 1;
    }
}
#endif

/*
 *  ======== ECJPAKECC26X2_hwiFxn ========
 */
static void ECJPAKECC26X2_hwiFxn(uintptr_t arg0)
{
    ECJPAKECC26X2_Object *object = ((ECJPAKE_Handle)arg0)->object;
    uint32_t key;

    /* Disable interrupt again */
    IntDisable(INT_PKA_IRQ);

    do
    {
        object->operationStatus = ECJPAKECC26X2_runFSM((ECJPAKE_Handle)arg0);
        object->fsmState++;
    } while (object->operationStatus == ECJPAKECC26X2_STATUS_FSM_RUN_FSM);

    /* We need a critical section here in case the operation is canceled
     * asynchronously.
     */
    key = HwiP_disable();

    if (object->operationCanceled)
    {
        /* Set function register to 0. This should stop the current operation */
        HWREG(PKA_BASE + PKA_O_FUNCTION) = 0;

        object->operationStatus = ECJPAKE_STATUS_CANCELED;
    }

    switch (object->operationStatus)
    {
        case ECJPAKECC26X2_STATUS_FSM_RUN_PKA_OP:

            HwiP_restore(key);

            /* Do nothing. The PKA hardware
             * will execute in the background and post
             * this SWI when it is done.
             */
            break;
        case ECJPAKE_STATUS_SUCCESS:
            /* Intentional fall through */
        case ECJPAKE_STATUS_ERROR:
            /* Intentional fall through */
        case ECJPAKE_STATUS_CANCELED:
            /* Intentional fall through */
        case ECJPAKE_STATUS_KEYSTORE_ERROR:
            /* Intentional fall through */
        default:

            /* Mark this operation as complete */
            object->operationInProgress = false;

            /* Clear any pending interrupt in case a transaction kicked off
             * above already finished
             */
            IntDisable(INT_PKA_IRQ);
            IntPendClear(INT_PKA_IRQ);

            /* We can end the critical section since the operation may no
             * longer be canceled
             */
            HwiP_restore(key);

            /* Make sure there is no keying material remaining in PKA RAM */
            PKAClearPkaRam();

            /*  Grant access for other threads to use the crypto module.
             *  The semaphore must be posted before the callbackFxn to allow the chaining
             *  of operations.
             */
            SemaphoreP_post(&PKAResourceCC26XX_accessSemaphore);

            Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

            object->callbackFxn((ECJPAKE_Handle)arg0,
                                object->operationStatus,
                                object->operation,
                                object->operationType);
    }
}

/*
 *  ======== ECJPAKECC26X2_runSignFSM ========
 */
static int_fast16_t ECJPAKECC26X2_runFSM(ECJPAKE_Handle handle)
{
    static uint32_t resultAddress;
    ECJPAKECC26X2_Object *object = handle->object;
    uint32_t pkaResult;

    uint8_t *privateKeyMaterial;
    size_t privateKeyLength;
    uint8_t *publicKeyMaterial;
    size_t publicKeyLength;
    uint8_t *preSharedSecretMaterial;
    size_t preSharedSecretLength;
    uint8_t *sharedSecretMaterial;
    int_fast16_t status;

    switch (object->fsmState)
    {
        /* ====================================================================
         * ROUND ONE KEY GENERATION FSM STATE IMPLEMENTATIONS
         * ====================================================================
         */
        case ECJPAKECC26X2_FSM_ROUND_ONE_GENERATE_MYPUBLICKEY1:

            return ECJPAKECC26X2_generatePublicKeyStart(object->operation.generateRoundOneKeys->myPrivateKey1,
                                                        object->operation.generateRoundOneKeys->curve,
                                                        &resultAddress);

        case ECJPAKECC26X2_FSM_ROUND_ONE_GENERATE_MYPUBLICKEY1_RESULT:
            /* Get X and Y coordinates for short Weierstrass curves */
            return ECJPAKECC26X2_getPublicKeyResult(object->operation.generateRoundOneKeys->myPublicKey1,
                                                    object->operation.generateRoundOneKeys->curve,
                                                    resultAddress);

        case ECJPAKECC26X2_FSM_ROUND_ONE_GENERATE_MYPUBLICKEY2:

            return ECJPAKECC26X2_generatePublicKeyStart(object->operation.generateRoundOneKeys->myPrivateKey2,
                                                        object->operation.generateRoundOneKeys->curve,
                                                        &resultAddress);

        case ECJPAKECC26X2_FSM_ROUND_ONE_GENERATE_MYPUBLICKEY2_RESULT:
            /* Get X and Y coordinates for short Weierstrass curves */
            return ECJPAKECC26X2_getPublicKeyResult(object->operation.generateRoundOneKeys->myPublicKey2,
                                                    object->operation.generateRoundOneKeys->curve,
                                                    resultAddress);

        case ECJPAKECC26X2_FSM_ROUND_ONE_GENERATE_MYPUBLICV1:

            return ECJPAKECC26X2_generatePublicKeyStart(object->operation.generateRoundOneKeys->myPrivateV1,
                                                        object->operation.generateRoundOneKeys->curve,
                                                        &resultAddress);

        case ECJPAKECC26X2_FSM_ROUND_ONE_GENERATE_MYPUBLICV1_RESULT:
            /* Get X and Y coordinates for short Weierstrass curves */
            return ECJPAKECC26X2_getPublicKeyResult(object->operation.generateRoundOneKeys->myPublicV1,
                                                    object->operation.generateRoundOneKeys->curve,
                                                    resultAddress);

        case ECJPAKECC26X2_FSM_ROUND_ONE_GENERATE_MYPUBLICV2:

            return ECJPAKECC26X2_generatePublicKeyStart(object->operation.generateRoundOneKeys->myPrivateV2,
                                                        object->operation.generateRoundOneKeys->curve,
                                                        &resultAddress);

        case ECJPAKECC26X2_FSM_ROUND_ONE_GENERATE_MYPUBLICV2_RESULT:
            /* Get X and Y coordinates for short Weierstrass curves */
            return ECJPAKECC26X2_getPublicKeyResult(object->operation.generateRoundOneKeys->myPublicV2,
                                                    object->operation.generateRoundOneKeys->curve,
                                                    resultAddress);

        /* ====================================================================
         * GENERATE ZKP FSM STATE IMPLEMENTATIONS
         * ====================================================================
         */
        case ECJPAKECC26X2_FSM_GENERATE_ZKP_PRIVATEKEY_X_HASH:

            /* Since we are receiving the private and public keys in octet string format,
             * we need to convert them to little-endian form for use with the PKA
             */
            status = ECJPAKE_getPrivateKey(object->operation.generateZKP->myPrivateKey,
                                           &privateKeyMaterial,
                                           &privateKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            CryptoUtils_reverseCopyPad(privateKeyMaterial,
                                       SCRATCH_PRIVATE_KEY,
                                       object->operation.generateZKP->curve->length);

            CryptoUtils_reverseCopyPad(object->operation.generateZKP->hash,
                                       SCRATCH_BUFFER_0,
                                       object->operation.generateZKP->curve->length);

            PKABigNumMultiplyStart((uint8_t *)SCRATCH_PRIVATE_KEY,
                                   object->operation.generateZKP->curve->length,
                                   (uint8_t *)SCRATCH_BUFFER_0,
                                   object->operation.generateZKP->curve->length,
                                   &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_GENERATE_ZKP_PRIVATEKEY_X_HASH_RESULT:

            pkaResult = PKABigNumMultGetResult((uint8_t *)SCRATCH_BUFFER_0, &scratchBufferSize, resultAddress);

            return ECJPAKECC26X2_convertReturnValue(pkaResult);

        case ECJPAKECC26X2_FSM_GENERATE_ZKP_PRIVATEKEY_X_HASH_MOD_N:

            /* Zero out the buffer up to the curve length. The PKA hardware cannot
             * handle modulus operations where the byte-length of the dividend is smaller
             * than the divisor.
             */
            if (object->operation.generateZKP->curve->length > scratchBufferSize)
            {
                PKAZeroOutArray((uint8_t *)SCRATCH_BUFFER_0 + scratchBufferSize,
                                object->operation.generateZKP->curve->length - scratchBufferSize);
            }

            /* The scratch buffer content has a real length of scratchBufferSize but is
             * zero-extended until curve->length. We cannot start a modulo operation on
             * the PKA where dividend length < divisor length. Hence, the buffer size
             * always needs to be >= curve->length.
             */
            PKABigNumModStart((uint8_t *)SCRATCH_BUFFER_0,
                              Math_MAX(scratchBufferSize, object->operation.generateZKP->curve->length),
                              object->operation.generateZKP->curve->order,
                              object->operation.generateZKP->curve->length,
                              &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_GENERATE_ZKP_PRIVATEKEY_X_HASH_MOD_N_RESULT:

            pkaResult = PKABigNumModGetResult(object->operation.generateZKP->r,
                                              object->operation.generateZKP->curve->length,
                                              resultAddress);

            return ECJPAKECC26X2_convertReturnValue(pkaResult);

        case ECJPAKECC26X2_FSM_GENERATE_ZKP_ADD_V_TO_N:

            /* Since we are receiving the private and public keys in octet string format,
             * we need to convert them to little-endian form for use with the PKA
             */
            status = ECJPAKE_getPrivateKey(object->operation.generateZKP->myPrivateV,
                                           &privateKeyMaterial,
                                           &privateKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            CryptoUtils_reverseCopyPad(privateKeyMaterial,
                                       SCRATCH_PRIVATE_KEY,
                                       object->operation.generateRoundOneKeys->curve->length);

            PKABigNumAddStart((uint8_t *)SCRATCH_PRIVATE_KEY,
                              object->operation.generateZKP->curve->length,
                              object->operation.generateZKP->curve->order,
                              object->operation.generateZKP->curve->length,
                              &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_GENERATE_ZKP_ADD_V_TO_N_RESULT:

            scratchBufferSize = SCRATCH_BUFFER_0_SIZE;

            pkaResult = PKABigNumAddGetResult((uint8_t *)SCRATCH_BUFFER_0, &scratchBufferSize, resultAddress);

            return ECJPAKECC26X2_convertReturnValue(pkaResult);

        case ECJPAKECC26X2_FSM_GENERATE_ZKP_SUBTRACT_RESULTS:

            PKABigNumSubStart((uint8_t *)SCRATCH_BUFFER_0,
                              scratchBufferSize,
                              object->operation.generateZKP->r,
                              object->operation.generateZKP->curve->length,
                              &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_GENERATE_ZKP_SUBTRACT_RESULTS_RESULT:

            scratchBufferSize = SCRATCH_BUFFER_0_SIZE;

            pkaResult = PKABigNumSubGetResult((uint8_t *)SCRATCH_BUFFER_0, &scratchBufferSize, resultAddress);

            return ECJPAKECC26X2_convertReturnValue(pkaResult);

        case ECJPAKECC26X2_FSM_GENERATE_ZKP_SUBTRACT_RESULT_MOD_N:

            /* Zero out the buffer up to the curve length. The PKA hardware cannot
             * handle modulus operations where the byte-length of the dividend is smaller
             * than the divisor.
             */
            if (object->operation.generateZKP->curve->length > scratchBufferSize)
            {
                PKAZeroOutArray((uint8_t *)SCRATCH_BUFFER_0 + scratchBufferSize,
                                object->operation.generateZKP->curve->length - scratchBufferSize);
            }

            /* The scratch buffer content has a real length of scratchBufferSize but is
             * zero-extended until curve->length. We cannot start a modulo operation on
             * the PKA where dividend length < divisor length. Hence, the buffer size
             * always needs to be >= curve->length.
             */
            PKABigNumModStart((uint8_t *)SCRATCH_BUFFER_0,
                              Math_MAX(scratchBufferSize, object->operation.generateZKP->curve->length),
                              object->operation.generateZKP->curve->order,
                              object->operation.generateZKP->curve->length,
                              &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_GENERATE_ZKP_SUBTRACT_RESULT_MOD_N_RESULT:

            pkaResult = PKABigNumModGetResult(object->operation.generateZKP->r,
                                              object->operation.generateZKP->curve->length,
                                              resultAddress);

            CryptoUtils_reverseBufferBytewise(object->operation.generateZKP->r,
                                              object->operation.generateZKP->curve->length);

            return ECJPAKECC26X2_convertReturnValue(pkaResult);

        /* ====================================================================
         * VERIFY ZKP FSM STATE IMPLEMENTATIONS
         * ====================================================================
         */
        case ECJPAKECC26X2_FSM_VERIFY_ZKP_VALIDATE_PUBLIC_KEY:

            status = ECJPAKE_getPublicKey(object->operation.verifyZKP->theirPublicKey,
                                          &publicKeyMaterial,
                                          &publicKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }

            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET,
                                       SCRATCH_PUBLIC_X,
                                       object->operation.verifyZKP->curve->length);

            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET +
                                           object->operation.verifyZKP->curve->length,
                                       SCRATCH_PUBLIC_Y,
                                       object->operation.verifyZKP->curve->length);

            pkaResult = PKAEccVerifyPublicKeyWeierstrassStart((uint8_t *)SCRATCH_PUBLIC_X,
                                                              (uint8_t *)SCRATCH_PUBLIC_Y,
                                                              object->operation.verifyZKP->curve->prime,
                                                              object->operation.verifyZKP->curve->a,
                                                              object->operation.verifyZKP->curve->b,
                                                              object->operation.verifyZKP->curve->order,
                                                              object->operation.verifyZKP->curve->length);

            // Break out early since no PKA operation was started by the verify fxn
            return ECJPAKECC26X2_convertReturnValue(pkaResult);

        case ECJPAKECC26X2_FSM_VERIFY_ZKP_MULT_G_BY_R:

            CryptoUtils_reverseCopyPad(object->operation.verifyZKP->r,
                                       SCRATCH_PRIVATE_KEY,
                                       object->operation.verifyZKP->curve->length);

            /* If the generator was explicitly specified, we need to copy and
             * reverse it.
             */
            if (object->operation.verifyZKP->theirGenerator != NULL)
            {
                status = ECJPAKE_getPublicKey(object->operation.verifyZKP->theirGenerator,
                                              &publicKeyMaterial,
                                              &publicKeyLength);
                if (status != ECJPAKE_STATUS_SUCCESS)
                {
                    return status;
                }
                CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET,
                                           SCRATCH_PUBLIC_X,
                                           object->operation.verifyZKP->curve->length);

                CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET +
                                               object->operation.verifyZKP->curve->length,
                                           SCRATCH_PUBLIC_Y,
                                           object->operation.verifyZKP->curve->length);

                PKAEccMultiplyStart((uint8_t *)SCRATCH_PRIVATE_KEY,
                                    (uint8_t *)SCRATCH_PUBLIC_X,
                                    (uint8_t *)SCRATCH_PUBLIC_Y,
                                    object->operation.verifyZKP->curve->prime,
                                    object->operation.verifyZKP->curve->a,
                                    object->operation.verifyZKP->curve->b,
                                    object->operation.verifyZKP->curve->length,
                                    &resultAddress);
            }
            /* Otherwise, we just use the generator of the curve */
            else
            {
                PKAEccMultiplyStart((uint8_t *)SCRATCH_PRIVATE_KEY,
                                    object->operation.verifyZKP->curve->generatorX,
                                    object->operation.verifyZKP->curve->generatorY,
                                    object->operation.verifyZKP->curve->prime,
                                    object->operation.verifyZKP->curve->a,
                                    object->operation.verifyZKP->curve->b,
                                    object->operation.verifyZKP->curve->length,
                                    &resultAddress);
            }

            break;

        case ECJPAKECC26X2_FSM_VERIFY_ZKP_MULT_G_BY_R_RESULT:

            pkaResult = PKAEccMultiplyGetResult((uint8_t *)SCRATCH_BUFFER_0,
                                                (uint8_t *)SCRATCH_BUFFER_0 +
                                                    object->operation.verifyZKP->curve->length,
                                                resultAddress,
                                                object->operation.verifyZKP->curve->length);

            return ECJPAKECC26X2_convertReturnValue(pkaResult);

        case ECJPAKECC26X2_FSM_VERIFY_ZKP_HASH_MOD_N:

            CryptoUtils_reverseCopyPad(object->operation.verifyZKP->hash,
                                       SCRATCH_PRIVATE_KEY,
                                       object->operation.verifyZKP->curve->length);

            PKABigNumModStart((uint8_t *)SCRATCH_PRIVATE_KEY,
                              object->operation.verifyZKP->curve->length,
                              object->operation.verifyZKP->curve->order,
                              object->operation.verifyZKP->curve->length,
                              &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_VERIFY_ZKP_HASH_MOD_N_RESULT:

            pkaResult = PKABigNumModGetResult((uint8_t *)SCRATCH_BUFFER_2,
                                              object->operation.verifyZKP->curve->length,
                                              resultAddress);

            return ECJPAKECC26X2_convertReturnValue(pkaResult);

        case ECJPAKECC26X2_FSM_VERIFY_ZKP_MULT_X_BY_HASH:

            status = ECJPAKE_getPublicKey(object->operation.verifyZKP->theirPublicKey,
                                          &publicKeyMaterial,
                                          &publicKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET,
                                       SCRATCH_PUBLIC_X,
                                       object->operation.verifyZKP->curve->length);

            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET +
                                           object->operation.verifyZKP->curve->length,
                                       SCRATCH_PUBLIC_Y,
                                       object->operation.verifyZKP->curve->length);

            PKAEccMultiplyStart((uint8_t *)SCRATCH_BUFFER_2,
                                (uint8_t *)SCRATCH_PUBLIC_X,
                                (uint8_t *)SCRATCH_PUBLIC_Y,
                                object->operation.verifyZKP->curve->prime,
                                object->operation.verifyZKP->curve->a,
                                object->operation.verifyZKP->curve->b,
                                object->operation.verifyZKP->curve->length,
                                &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_VERIFY_ZKP_MULT_X_BY_HASH_RESULT:

            pkaResult = PKAEccMultiplyGetResult((uint8_t *)SCRATCH_BUFFER_2,
                                                (uint8_t *)SCRATCH_BUFFER_2 +
                                                    object->operation.verifyZKP->curve->length,
                                                resultAddress,
                                                object->operation.verifyZKP->curve->length);

            return ECJPAKECC26X2_convertReturnValue(pkaResult);

        case ECJPAKECC26X2_FSM_VERIFY_ZKP_ADD_RESULTS:

            PKAEccAddStart((uint8_t *)SCRATCH_BUFFER_0,
                           (uint8_t *)SCRATCH_BUFFER_0 + object->operation.verifyZKP->curve->length,
                           (uint8_t *)SCRATCH_BUFFER_2,
                           (uint8_t *)SCRATCH_BUFFER_2 + object->operation.verifyZKP->curve->length,
                           object->operation.verifyZKP->curve->prime,
                           object->operation.verifyZKP->curve->a,
                           object->operation.verifyZKP->curve->length,
                           &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_VERIFY_ZKP_ADD_RESULTS_RESULT:

            pkaResult = PKAEccAddGetResult((uint8_t *)SCRATCH_BUFFER_0,
                                           (uint8_t *)SCRATCH_BUFFER_0 + object->operation.verifyZKP->curve->length,
                                           resultAddress,
                                           object->operation.verifyZKP->curve->length);

            return ECJPAKECC26X2_convertReturnValue(pkaResult);

        case ECJPAKECC26X2_FSM_VERIFY_ZKP_COMPARE_AGAINST_V:

            status = ECJPAKE_getPublicKey(object->operation.verifyZKP->theirPublicV,
                                          &publicKeyMaterial,
                                          &publicKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET,
                                       SCRATCH_BUFFER_2,
                                       object->operation.verifyZKP->curve->length);

            /* This requires the implicit assumption that we are using a curve with a word-multiple length.
             * The helper function is written with the assumption that the destination is word-aligned.
             * In practice, this only affects the NIST-P521 curve that we do not officially support with
             * ECJPAKE currently.
             */
            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET +
                                           object->operation.verifyZKP->curve->length,
                                       (uint32_t *)((uint8_t *)SCRATCH_BUFFER_2 +
                                                    object->operation.verifyZKP->curve->length),
                                       object->operation.verifyZKP->curve->length);

            if (CryptoUtils_buffersMatchWordAligned(SCRATCH_BUFFER_2,
                                                    SCRATCH_BUFFER_0,
                                                    2 * object->operation.verifyZKP->curve->length))
            {
                return ECJPAKE_STATUS_SUCCESS;
            }
            else
            {
                return ECJPAKE_STATUS_ERROR;
            }

        /* ====================================================================
         * ROUND TWO KEY GENERATION FSM STATE IMPLEMENTATIONS
         * ====================================================================
         */
        case ECJPAKECC26X2_FSM_ROUND_TWO_MULT_MYPRIVATEKEY2_BY_PRESHAREDSECRET:

            /* Since we are receiving the private and public keys in octet string format,
             * we need to convert them to little-endian form for use with the PKA
             */
            status = ECJPAKE_getPrivateKey(object->operation.generateRoundTwoKeys->myPrivateKey2,
                                           &privateKeyMaterial,
                                           &privateKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            CryptoUtils_reverseCopyPad(privateKeyMaterial,
                                       SCRATCH_PRIVATE_KEY,
                                       object->operation.generateRoundTwoKeys->curve->length);

            status = ECJPAKE_getPreSharedSecret(object->operation.generateRoundTwoKeys->preSharedSecret,
                                                &preSharedSecretMaterial,
                                                &preSharedSecretLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            CryptoUtils_reverseCopyPad(preSharedSecretMaterial, SCRATCH_BUFFER_0, preSharedSecretLength);

            PKABigNumMultiplyStart((uint8_t *)SCRATCH_PRIVATE_KEY,
                                   object->operation.generateRoundTwoKeys->curve->length,
                                   (uint8_t *)SCRATCH_BUFFER_0,
                                   preSharedSecretLength,
                                   &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_ROUND_TWO_MULT_MYPRIVATEKEY2_BY_PRESHAREDSECRET_RESULT:

            pkaResult = PKABigNumMultGetResult((uint8_t *)SCRATCH_BUFFER_0, &scratchBufferSize, resultAddress);

            return ECJPAKECC26X2_convertReturnValue(pkaResult);

        case ECJPAKECC26X2_FSM_ROUND_TWO_MYCOMBINEDPRIVATEKEY_MOD_N:

            /* Zero out the buffer up to the curve length. The PKA hardware cannot
             * handle modulus operations where the byte-length of the dividend is smaller
             * than the divisor.
             */
            if (object->operation.generateRoundTwoKeys->curve->length > scratchBufferSize)
            {
                PKAZeroOutArray((uint8_t *)SCRATCH_BUFFER_0 + scratchBufferSize,
                                object->operation.generateRoundTwoKeys->curve->length - scratchBufferSize);
            }

            /* The scratch buffer content has a real length of scratchBufferSize but is
             * zero-extended until curve->length. We cannot start a modulo operation on
             * the PKA where dividend length < divisor length. Hence, the buffer size
             * always needs to be >= curve->length.
             */
            PKABigNumModStart((uint8_t *)SCRATCH_BUFFER_0,
                              Math_MAX(scratchBufferSize, object->operation.generateRoundTwoKeys->curve->length),
                              object->operation.generateRoundTwoKeys->curve->order,
                              object->operation.generateRoundTwoKeys->curve->length,
                              &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_ROUND_TWO_MYCOMBINEDPRIVATEKEY_MOD_N_RESULT:

            if (object->operation.generateRoundTwoKeys->myCombinedPrivateKey->encoding == CryptoKey_BLANK_PLAINTEXT)
            {
                privateKeyMaterial = object->operation.generateRoundTwoKeys->myCombinedPrivateKey->u.plaintext
                                         .keyMaterial;
            }
#if (ENABLE_KEY_STORAGE == 1)
            else if (object->operation.generateRoundTwoKeys->myCombinedPrivateKey->encoding == CryptoKey_BLANK_KEYSTORE)
            {
                memset(ECJPAKECC26X2_keyStorePrivateKeyMaterial, 0, sizeof(ECJPAKECC26X2_keyStorePrivateKeyMaterial));
                privateKeyMaterial = ECJPAKECC26X2_keyStorePrivateKeyMaterial;
            }
#endif
            else
            {
                return ECJPAKE_STATUS_ERROR;
            }
            pkaResult = PKABigNumModGetResult(privateKeyMaterial,
                                              object->operation.generateRoundTwoKeys->curve->length,
                                              resultAddress);

            /* We just wrote back a little-endian integer.
             * We need to turn that output into OS format.
             * For a private key, that just means byte-reversing the array.
             */
            CryptoUtils_reverseBufferBytewise(privateKeyMaterial,
                                              object->operation.generateRoundTwoKeys->curve->length);

            status = ECJPAKECC26X2_convertReturnValue(pkaResult);
            if (status == ECJPAKECC26X2_STATUS_FSM_RUN_FSM)
            {
                if (object->operation.generateRoundTwoKeys->myCombinedPrivateKey->encoding == CryptoKey_BLANK_PLAINTEXT)
                {
                    object->operation.generateRoundTwoKeys->myCombinedPrivateKey->encoding = CryptoKey_PLAINTEXT;
                }
#if (ENABLE_KEY_STORAGE == 1)
                else if (object->operation.generateRoundTwoKeys->myCombinedPrivateKey->encoding ==
                         CryptoKey_BLANK_KEYSTORE)
                {
                    status = ECJPAKECC26X2_importSecureKey(object->operation.generateRoundTwoKeys->myCombinedPrivateKey,
                                                           privateKeyMaterial,
                                                           object->operation.generateRoundTwoKeys->curve->curveType);
                }
#endif
            }
            return status;

        case ECJPAKECC26X2_FSM_ROUND_TWO_ADD_MYPUBLICKEY1_TO_THEIRPUBLICKEY1:

            /* Copy myPublicKey1 into the public key scratch area in little-endian
             * form.
             */
            status = ECJPAKE_getPublicKey(object->operation.generateRoundTwoKeys->myPublicKey1,
                                          &publicKeyMaterial,
                                          &publicKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET,
                                       SCRATCH_PUBLIC_X,
                                       object->operation.generateRoundTwoKeys->curve->length);

            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET +
                                           object->operation.generateRoundTwoKeys->curve->length,
                                       SCRATCH_PUBLIC_Y,
                                       object->operation.generateRoundTwoKeys->curve->length);

            /* Copy theirPublicKey1 into SCRATCH_BUFFER_0 in little-endian form.
             * There is the implicit assumption that the curve length is a
             * word-multiple here.
             */
            status = ECJPAKE_getPublicKey(object->operation.generateRoundTwoKeys->theirPublicKey1,
                                          &publicKeyMaterial,
                                          &publicKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET,
                                       SCRATCH_BUFFER_0,
                                       object->operation.generateRoundTwoKeys->curve->length);

            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET +
                                           object->operation.generateRoundTwoKeys->curve->length,
                                       (uint32_t *)((uint8_t *)SCRATCH_BUFFER_0 +
                                                    object->operation.generateRoundTwoKeys->curve->length),
                                       object->operation.generateRoundTwoKeys->curve->length);

            PKAEccAddStart((uint8_t *)SCRATCH_PUBLIC_X,
                           (uint8_t *)SCRATCH_PUBLIC_Y,
                           (uint8_t *)SCRATCH_BUFFER_0,
                           (uint8_t *)SCRATCH_BUFFER_0 + object->operation.generateRoundTwoKeys->curve->length,
                           object->operation.generateRoundTwoKeys->curve->prime,
                           object->operation.generateRoundTwoKeys->curve->a,
                           object->operation.generateRoundTwoKeys->curve->length,
                           &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_ROUND_TWO_ADD_MYPUBLICKEY1_TO_THEIRPUBLICKEY1_RESULT:

            pkaResult = PKAEccAddGetResult((uint8_t *)SCRATCH_BUFFER_0,
                                           (uint8_t *)SCRATCH_BUFFER_0 +
                                               object->operation.generateRoundTwoKeys->curve->length,
                                           resultAddress,
                                           object->operation.generateRoundTwoKeys->curve->length);

            return ECJPAKECC26X2_convertReturnValue(pkaResult);

        case ECJPAKECC26X2_FSM_ROUND_TWO_ADD_THEIRPUBLICKEY2:

            /* Copy theirPublicKey2 into the public key scratch area in little-endian
             * form. The ECC coordinates resulting from the previous operation
             * should still be in SCRATCH_BUFFER_0.
             */
            status = ECJPAKE_getPublicKey(object->operation.generateRoundTwoKeys->theirPublicKey2,
                                          &publicKeyMaterial,
                                          &publicKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET,
                                       SCRATCH_PUBLIC_X,
                                       object->operation.generateRoundTwoKeys->curve->length);

            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET +
                                           object->operation.generateRoundTwoKeys->curve->length,
                                       SCRATCH_PUBLIC_Y,
                                       object->operation.generateRoundTwoKeys->curve->length);

            PKAEccAddStart((uint8_t *)SCRATCH_BUFFER_0,
                           (uint8_t *)SCRATCH_BUFFER_0 + object->operation.generateRoundTwoKeys->curve->length,
                           (uint8_t *)SCRATCH_PUBLIC_X,
                           (uint8_t *)SCRATCH_PUBLIC_Y,
                           object->operation.generateRoundTwoKeys->curve->prime,
                           object->operation.generateRoundTwoKeys->curve->a,
                           object->operation.generateRoundTwoKeys->curve->length,
                           &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_ROUND_TWO_ADD_THEIRPUBLICKEY2_RESULT:

            return ECJPAKECC26X2_getPublicKeyResult(object->operation.generateRoundTwoKeys->myNewGenerator,
                                                    object->operation.generateRoundTwoKeys->curve,
                                                    resultAddress);

        case ECJPAKECC26X2_FSM_ROUND_TWO_ADD_MYPUBLICKEY2:

            /* Copy myPublicKey2 into the public key scratch area in little-endian
             * form.
             */
            status = ECJPAKE_getPublicKey(object->operation.generateRoundTwoKeys->myPublicKey2,
                                          &publicKeyMaterial,
                                          &publicKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET,
                                       SCRATCH_PUBLIC_X,
                                       object->operation.generateRoundTwoKeys->curve->length);

            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET +
                                           object->operation.generateRoundTwoKeys->curve->length,
                                       SCRATCH_PUBLIC_Y,
                                       object->operation.generateRoundTwoKeys->curve->length);

            PKAEccAddStart((uint8_t *)SCRATCH_BUFFER_0,
                           (uint8_t *)SCRATCH_BUFFER_0 + object->operation.generateRoundTwoKeys->curve->length,
                           (uint8_t *)SCRATCH_PUBLIC_X,
                           (uint8_t *)SCRATCH_PUBLIC_Y,
                           object->operation.generateRoundTwoKeys->curve->prime,
                           object->operation.generateRoundTwoKeys->curve->a,
                           object->operation.generateRoundTwoKeys->curve->length,
                           &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_ROUND_TWO_ADD_MYPUBLICKEY2_RESULT:

            return ECJPAKECC26X2_getPublicKeyResult(object->operation.generateRoundTwoKeys->theirNewGenerator,
                                                    object->operation.generateRoundTwoKeys->curve,
                                                    resultAddress);

        case ECJPAKECC26X2_FSM_ROUND_TWO_MULT_MYCOMBINEDPRIVATEKEY_BY_MYNEWGENERATOR:

            /* Copy myCombinedPrivateKey into the private key scratch area in
             * little-endian form.
             */
            status = ECJPAKE_getPrivateKey(object->operation.generateRoundTwoKeys->myCombinedPrivateKey,
                                           &privateKeyMaterial,
                                           &privateKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            CryptoUtils_reverseCopyPad(privateKeyMaterial,
                                       SCRATCH_PRIVATE_KEY,
                                       object->operation.generateRoundTwoKeys->curve->length);

            /* Copy myNewGenerator into the public key scratch area in little-endian
             * form.
             */
            status = ECJPAKE_getPublicKey(object->operation.generateRoundTwoKeys->myNewGenerator,
                                          &publicKeyMaterial,
                                          &publicKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET,
                                       SCRATCH_PUBLIC_X,
                                       object->operation.generateRoundTwoKeys->curve->length);

            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET +
                                           object->operation.generateRoundTwoKeys->curve->length,
                                       SCRATCH_PUBLIC_Y,
                                       object->operation.generateRoundTwoKeys->curve->length);

            PKAEccMultiplyStart((uint8_t *)SCRATCH_PRIVATE_KEY,
                                (uint8_t *)SCRATCH_PUBLIC_X,
                                (uint8_t *)SCRATCH_PUBLIC_Y,
                                object->operation.generateRoundTwoKeys->curve->prime,
                                object->operation.generateRoundTwoKeys->curve->a,
                                object->operation.generateRoundTwoKeys->curve->b,
                                object->operation.generateRoundTwoKeys->curve->length,
                                &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_ROUND_TWO_MULT_MYCOMBINEDPRIVATEKEY_BY_MYNEWGENERATOR_RESULT:

            return ECJPAKECC26X2_getPublicKeyResult(object->operation.generateRoundTwoKeys->myCombinedPublicKey,
                                                    object->operation.generateRoundTwoKeys->curve,
                                                    resultAddress);

        case ECJPAKECC26X2_FSM_ROUND_TWO_GENERATE_MYPUBLICV:

            status = ECJPAKE_getPrivateKey(object->operation.generateRoundTwoKeys->myPrivateV,
                                           &privateKeyMaterial,
                                           &privateKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }

            if (PKAArrayAllZeros(privateKeyMaterial, object->operation.generateRoundTwoKeys->curve->length))
            {
                return ECJPAKE_STATUS_INVALID_PRIVATE_KEY;
            }

            /* Since we are receiving the private and public keys in octet string format,
             * we need to convert them to little-endian form for use with the PKA
             */
            CryptoUtils_reverseCopyPad(privateKeyMaterial,
                                       SCRATCH_PRIVATE_KEY,
                                       object->operation.generateRoundTwoKeys->curve->length);

            /* Copy myNewGenerator into the public key scratch area in little-endian
             * form.
             */
            status = ECJPAKE_getPublicKey(object->operation.generateRoundTwoKeys->myNewGenerator,
                                          &publicKeyMaterial,
                                          &publicKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET,
                                       SCRATCH_PUBLIC_X,
                                       object->operation.generateRoundTwoKeys->curve->length);

            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET +
                                           object->operation.generateRoundTwoKeys->curve->length,
                                       SCRATCH_PUBLIC_Y,
                                       object->operation.generateRoundTwoKeys->curve->length);

            PKABigNumCmpStart((uint8_t *)SCRATCH_PRIVATE_KEY,
                              object->operation.generateRoundTwoKeys->curve->order,
                              object->operation.generateRoundTwoKeys->curve->length);

            while (PKAGetOpsStatus() == PKA_STATUS_OPERATION_BUSY) {}

            pkaResult = PKABigNumCmpGetResult();

            if (pkaResult != PKA_STATUS_A_LESS_THAN_B)
            {
                return ECJPAKE_STATUS_INVALID_PRIVATE_KEY;
            }

            PKAEccMultiplyStart((uint8_t *)SCRATCH_PRIVATE_KEY,
                                (uint8_t *)SCRATCH_PUBLIC_X,
                                (uint8_t *)SCRATCH_PUBLIC_Y,
                                object->operation.generateRoundTwoKeys->curve->prime,
                                object->operation.generateRoundTwoKeys->curve->a,
                                object->operation.generateRoundTwoKeys->curve->b,
                                object->operation.generateRoundTwoKeys->curve->length,
                                &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_ROUND_TWO_GENERATE_MYPUBLICV_RESULT:

            return ECJPAKECC26X2_getPublicKeyResult(object->operation.generateRoundTwoKeys->myPublicV,
                                                    object->operation.generateRoundTwoKeys->curve,
                                                    resultAddress);

        /* ====================================================================
         * GENERATE SHARED SECRET FSM STATE IMPLEMENTATIONS
         * ====================================================================
         */
        case ECJPAKECC26X2_FSM_GENERATE_SHARED_SECRET_MULT_THEIRPUBLICKEY2_BY_MYCOMBINEDPRIVATEKEY:

            /* Since we are receiving the private and public keys in octet string format,
             * we need to convert them to little-endian form for use with the PKA
             */
            /* Copy myCombinedPrivateKey into the public key scratch area in little-endian
             * form.
             */
            status = ECJPAKE_getPrivateKey(object->operation.computeSharedSecret->myCombinedPrivateKey,
                                           &privateKeyMaterial,
                                           &privateKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            CryptoUtils_reverseCopyPad(privateKeyMaterial,
                                       SCRATCH_PRIVATE_KEY,
                                       object->operation.computeSharedSecret->curve->length);

            status = ECJPAKE_getPublicKey(object->operation.computeSharedSecret->theirPublicKey2,
                                          &publicKeyMaterial,
                                          &publicKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET,
                                       SCRATCH_PUBLIC_X,
                                       object->operation.computeSharedSecret->curve->length);

            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET +
                                           object->operation.computeSharedSecret->curve->length,
                                       SCRATCH_PUBLIC_Y,
                                       object->operation.computeSharedSecret->curve->length);

            PKAEccMultiplyStart((uint8_t *)SCRATCH_PRIVATE_KEY,
                                (uint8_t *)SCRATCH_PUBLIC_X,
                                (uint8_t *)SCRATCH_PUBLIC_Y,
                                object->operation.computeSharedSecret->curve->prime,
                                object->operation.computeSharedSecret->curve->a,
                                object->operation.computeSharedSecret->curve->b,
                                object->operation.computeSharedSecret->curve->length,
                                &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_GENERATE_SHARED_SECRET_MULT_THEIRPUBLICKEY2_BY_MYCOMBINEDPRIVATEKEY_RESULT:

            /* While we are copying back an EC point to sharedSecret, we are
             * only using it as temp storage. As such, we can let it remain
             * in little-endian form.
             */
            status = ECJPAKE_getSharedSecretBuffer(object->operation.computeSharedSecret->sharedSecret,
                                                   &sharedSecretMaterial);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            pkaResult = PKAEccMultiplyGetResult(sharedSecretMaterial,
                                                sharedSecretMaterial +
                                                    object->operation.computeSharedSecret->curve->length,
                                                resultAddress,
                                                object->operation.computeSharedSecret->curve->length);

            return ECJPAKECC26X2_convertReturnValue(pkaResult);

        case ECJPAKECC26X2_FSM_GENERATE_SHARED_SECRET_SUB_YCOORDINATE_FROM_PRIME:

            status = ECJPAKE_getSharedSecretBuffer(object->operation.computeSharedSecret->sharedSecret,
                                                   &sharedSecretMaterial);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            PKABigNumSubStart(object->operation.computeSharedSecret->curve->prime,
                              object->operation.computeSharedSecret->curve->length,
                              sharedSecretMaterial + object->operation.computeSharedSecret->curve->length,
                              object->operation.computeSharedSecret->curve->length,
                              &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_GENERATE_SHARED_SECRET_SUB_YCOORDINATE_FROM_PRIME_RESULT:

            /* Use scratchBufferSize as a dummy length variable since we will not copy the
             * result into the SCRATCH_BUFFER_0
             */
            scratchBufferSize = object->operation.computeSharedSecret->curve->length;

            status = ECJPAKE_getSharedSecretBuffer(object->operation.computeSharedSecret->sharedSecret,
                                                   &sharedSecretMaterial);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            pkaResult = PKABigNumSubGetResult(sharedSecretMaterial +
                                                  object->operation.computeSharedSecret->curve->length,
                                              &scratchBufferSize,
                                              resultAddress);

            /* Zero out the private key buffer up to the curve length. Otherwise, we may
             * have rubbish floating around the buffer instead of being zero sign-extended.
             */
            if (object->operation.computeSharedSecret->curve->length > scratchBufferSize)
            {
                status = ECJPAKE_getSharedSecretBuffer(object->operation.computeSharedSecret->sharedSecret,
                                                       &sharedSecretMaterial);
                if (status != ECJPAKE_STATUS_SUCCESS)
                {
                    return status;
                }
                PKAZeroOutArray(sharedSecretMaterial + object->operation.computeSharedSecret->curve->length +
                                    scratchBufferSize,
                                object->operation.computeSharedSecret->curve->length - scratchBufferSize);
            }

            return ECJPAKECC26X2_convertReturnValue(pkaResult);

        case ECJPAKECC26X2_FSM_GENERATE_SHARED_SECRET_ADD_THEIRCOMBINEDPUBLICKEY:

            /* Copy theirCombinedPublicKey into the public key scratch area in little-endian
             * form.
             */
            status = ECJPAKE_getPublicKey(object->operation.computeSharedSecret->theirCombinedPublicKey,
                                          &publicKeyMaterial,
                                          &publicKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET,
                                       SCRATCH_PUBLIC_X,
                                       object->operation.computeSharedSecret->curve->length);

            CryptoUtils_reverseCopyPad(publicKeyMaterial + OCTET_STRING_OFFSET +
                                           object->operation.computeSharedSecret->curve->length,
                                       SCRATCH_PUBLIC_Y,
                                       object->operation.computeSharedSecret->curve->length);

            status = ECJPAKE_getSharedSecretBuffer(object->operation.computeSharedSecret->sharedSecret,
                                                   &sharedSecretMaterial);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            PKAEccAddStart(sharedSecretMaterial,
                           sharedSecretMaterial + object->operation.computeSharedSecret->curve->length,
                           (uint8_t *)SCRATCH_PUBLIC_X,
                           (uint8_t *)SCRATCH_PUBLIC_Y,
                           object->operation.computeSharedSecret->curve->prime,
                           object->operation.computeSharedSecret->curve->a,
                           object->operation.computeSharedSecret->curve->length,
                           &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_GENERATE_SHARED_SECRET_ADD_THEIRCOMBINEDPUBLICKEY_RESULT:

            status = ECJPAKE_getSharedSecretBuffer(object->operation.computeSharedSecret->sharedSecret,
                                                   &sharedSecretMaterial);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            pkaResult = PKAEccAddGetResult(sharedSecretMaterial,
                                           sharedSecretMaterial + object->operation.computeSharedSecret->curve->length,
                                           resultAddress,
                                           object->operation.computeSharedSecret->curve->length);

            return ECJPAKECC26X2_convertReturnValue(pkaResult);

        case ECJPAKECC26X2_FSM_GENERATE_SHARED_SECRET_MULT_BY_MYPRIVATEKEY2:

            /* Since we are receiving the private and public keys in octet string format,
             * we need to convert them to little-endian form for use with the PKA
             */
            status = ECJPAKE_getPrivateKey(object->operation.computeSharedSecret->myPrivateKey2,
                                           &privateKeyMaterial,
                                           &privateKeyLength);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            CryptoUtils_reverseCopyPad(privateKeyMaterial,
                                       SCRATCH_PRIVATE_KEY,
                                       object->operation.computeSharedSecret->curve->length);

            status = ECJPAKE_getSharedSecretBuffer(object->operation.computeSharedSecret->sharedSecret,
                                                   &sharedSecretMaterial);
            if (status != ECJPAKE_STATUS_SUCCESS)
            {
                return status;
            }
            PKAEccMultiplyStart((uint8_t *)SCRATCH_PRIVATE_KEY,
                                sharedSecretMaterial,
                                sharedSecretMaterial + object->operation.computeSharedSecret->curve->length,
                                object->operation.computeSharedSecret->curve->prime,
                                object->operation.computeSharedSecret->curve->a,
                                object->operation.computeSharedSecret->curve->b,
                                object->operation.computeSharedSecret->curve->length,
                                &resultAddress);

            break;

        case ECJPAKECC26X2_FSM_GENERATE_SHARED_SECRET_MULT_BY_MYPRIVATEKEY2_RESULT:
            return ECJPAKECC26X2_getPublicKeyResult(object->operation.computeSharedSecret->sharedSecret,
                                                    object->operation.computeSharedSecret->curve,
                                                    resultAddress);

        case ECJPAKECC26X2_FSM_ROUND_ONE_GENERATE_RETURN:
            return ECJPAKE_STATUS_SUCCESS;

        case ECJPAKECC26X2_FSM_ROUND_TWO_GENERATE_RETURN:
            return ECJPAKE_STATUS_SUCCESS;

        case ECJPAKECC26X2_FSM_GENERATE_SHARED_SECRET_RETURN:
            return ECJPAKE_STATUS_SUCCESS;

        case ECJPAKECC26X2_FSM_ZKP_GENERATE_RETURN:
            return ECJPAKE_STATUS_SUCCESS;
        default:
            return ECJPAKE_STATUS_ERROR;
    }

    // If we get to this point, we want to perform another PKA operation
    IntPendClear(INT_PKA_IRQ);
    IntEnable(INT_PKA_IRQ);

    return ECJPAKECC26X2_STATUS_FSM_RUN_PKA_OP;
}

/*
 *  ======== ECJPAKECC26X2_convertReturnValue ========
 */
static int_fast16_t ECJPAKECC26X2_convertReturnValue(uint32_t pkaResult)
{
    switch (pkaResult)
    {
        case PKA_STATUS_SUCCESS:
        case PKA_STATUS_A_LESS_THAN_B:
        case PKA_STATUS_EQUAL:
            /* A less than B and equal only come up when checking private
             * key values. They indicate a key within the correct range.
             */
            return ECJPAKECC26X2_STATUS_FSM_RUN_FSM;

        case PKA_STATUS_X_ZERO:
        case PKA_STATUS_Y_ZERO:
        case PKA_STATUS_RESULT_0:
            /* Theoretically, PKA_STATUS_RESULT_0 might be caused by other
             * operations failing but the only one that really should yield
             * 0 is ECC multiplication with invalid inputs that yield the
             * point at infinity.
             */
            return ECJPAKE_STATUS_POINT_AT_INFINITY;

        case PKA_STATUS_X_LARGER_THAN_PRIME:
        case PKA_STATUS_Y_LARGER_THAN_PRIME:
            return ECJPAKE_STATUS_PUBLIC_KEY_LARGER_THAN_PRIME;

        case PKA_STATUS_POINT_NOT_ON_CURVE:
            return ECJPAKE_STATUS_PUBLIC_KEY_NOT_ON_CURVE;

        default:
            return ECJPAKE_STATUS_ERROR;
    }
}

/*
 *  ======== ECJPAKECC26X2_waitForAccess ========
 */
static int_fast16_t ECJPAKECC26X2_waitForAccess(ECJPAKE_Handle handle)
{
    ECJPAKECC26X2_Object *object = handle->object;
    uint32_t timeout;

    /* Set to SemaphoreP_NO_WAIT to start operations from SWI or HWI context */
    timeout = object->returnBehavior == ECJPAKE_RETURN_BEHAVIOR_BLOCKING ? object->semaphoreTimeout
                                                                         : SemaphoreP_NO_WAIT;

    return SemaphoreP_pend(&PKAResourceCC26XX_accessSemaphore, timeout);
}

/*
 *  ======== ECJPAKECC26X2_waitForResult ========
 */
static int_fast16_t ECJPAKECC26X2_waitForResult(ECJPAKE_Handle handle)
{
    ECJPAKECC26X2_Object *object = handle->object;

    object->operationInProgress = true;

    switch (object->returnBehavior)
    {
        case ECJPAKE_RETURN_BEHAVIOR_POLLING:
            while (!PKAResourceCC26XX_pollingFlag) {}
            return object->operationStatus;

        case ECJPAKE_RETURN_BEHAVIOR_BLOCKING:
            SemaphoreP_pend(&PKAResourceCC26XX_operationSemaphore, SemaphoreP_WAIT_FOREVER);
            return object->operationStatus;

        case ECJPAKE_RETURN_BEHAVIOR_CALLBACK:
            return ECJPAKE_STATUS_SUCCESS;

        default:
            return ECJPAKE_STATUS_ERROR;
    }
}

/*
 *  ======== ECJPAKE_init ========
 */
void ECJPAKE_init(void)
{
    PKAResourceCC26XX_constructRTOSObjects();

    isInitialized = true;
}

/*
 *  ======== ECJPAKE_construct ========
 */
ECJPAKE_Handle ECJPAKE_construct(ECJPAKE_Config *config, const ECJPAKE_Params *params)
{
    ECJPAKE_Handle handle;
    ECJPAKECC26X2_Object *object;
    uint_fast8_t key;

    handle = (ECJPAKE_Handle)config;
    object = handle->object;

    /* If params are NULL, use defaults */
    if (params == NULL)
    {
        params = (ECJPAKE_Params *)&ECJPAKE_defaultParams;
    }

    key = HwiP_disable();

    if (!isInitialized || object->isOpen)
    {
        HwiP_restore(key);
        return NULL;
    }

    object->isOpen = true;

    HwiP_restore(key);

    DebugP_assert((params->returnBehavior == ECJPAKE_RETURN_BEHAVIOR_CALLBACK) ? params->callbackFxn : true);

    object->returnBehavior = params->returnBehavior;
#if (TFM_ENABLED == 1)
    /* Always use the secure callback function */
    object->callbackFxn = params->callbackFxn;
#else
    object->callbackFxn      = (params->returnBehavior == ECJPAKE_RETURN_BEHAVIOR_CALLBACK)
                                   ? params->callbackFxn
                                   : ECJPAKECC26X2_internalCallbackFxn;
    object->semaphoreTimeout = params->timeout;
#endif

    /* Set power dependency - i.e. power up and enable clock for PKA (PKAResourceCC26XX) module. */
    Power_setDependency(PowerCC26X2_PERIPH_PKA);

    return handle;
}

/*
 *  ======== ECJPAKE_close ========
 */
void ECJPAKE_close(ECJPAKE_Handle handle)
{
    ECJPAKECC26X2_Object *object;

    DebugP_assert(handle);

    /* Get the pointer to the object */
    object = handle->object;

    /* Release power dependency on PKA Module. */
    Power_releaseDependency(PowerCC26X2_PERIPH_PKA);

    /* Mark the module as available */
    object->isOpen = false;
}

/*
 *  ======== ECJPAKE_roundOneGenerateKeys ========
 */
int_fast16_t ECJPAKE_roundOneGenerateKeys(ECJPAKE_Handle handle, ECJPAKE_OperationRoundOneGenerateKeys *operation)
{
    bool isKeyBlank;
    ECJPAKECC26X2_Object *object         = handle->object;
    ECJPAKECC26X2_HWAttrs const *hwAttrs = handle->hwAttrs;

    (void)CryptoKey_isBlank(operation->myPublicKey1, &isKeyBlank);
    if (!isKeyBlank)
    {
        return ECJPAKE_STATUS_OUTPUT_KEY_NOT_BLANK;
    }

    (void)CryptoKey_isBlank(operation->myPublicKey2, &isKeyBlank);
    if (!isKeyBlank)
    {
        return ECJPAKE_STATUS_OUTPUT_KEY_NOT_BLANK;
    }

    (void)CryptoKey_isBlank(operation->myPublicV1, &isKeyBlank);
    if (!isKeyBlank)
    {
        return ECJPAKE_STATUS_OUTPUT_KEY_NOT_BLANK;
    }

    (void)CryptoKey_isBlank(operation->myPublicV2, &isKeyBlank);
    if (!isKeyBlank)
    {
        return ECJPAKE_STATUS_OUTPUT_KEY_NOT_BLANK;
    }

    if (ECJPAKECC26X2_waitForAccess(handle) != SemaphoreP_OK)
    {
        return ECJPAKE_STATUS_RESOURCE_UNAVAILABLE;
    }

    /* Copy over all parameters we will need access to in the FSM.
     * The FSM runs in SWI context and thus needs to keep track of
     * all of them somehow.
     */
    object->fsmState                       = ECJPAKECC26X2_FSM_ROUND_ONE_GENERATE_MYPUBLICKEY1;
    object->operationStatus                = ECJPAKE_STATUS_ERROR;
    object->operation.generateRoundOneKeys = operation;
    object->operationType                  = ECJPAKE_OPERATION_TYPE_ROUND_ONE_GENERATE_KEYS;
    object->operationCanceled              = false;
    scratchBufferSize                      = SCRATCH_BUFFER_0_SIZE;

    /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
     * drivers and they all need to coexist. Whenever a driver starts an operation, it
     * registers its HWI callback with the OS.
     */
    HwiP_setFunc(&PKAResourceCC26XX_hwi, ECJPAKECC26X2_hwiFxn, (uintptr_t)handle);
    HwiP_setPriority(INT_PKA_IRQ, hwAttrs->intPriority);

    PKAResourceCC26XX_pollingFlag = 0;

    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /* Run the FSM by triggering the interrupt. It is level triggered
     * and the complement of the RUN bit.
     */
    IntEnable(INT_PKA_IRQ);

    return ECJPAKECC26X2_waitForResult(handle);
}

/*
 *  ======== ECJPAKE_generateZKP ========
 */
int_fast16_t ECJPAKE_generateZKP(ECJPAKE_Handle handle, ECJPAKE_OperationGenerateZKP *operation)
{
    ECJPAKECC26X2_Object *object         = handle->object;
    ECJPAKECC26X2_HWAttrs const *hwAttrs = handle->hwAttrs;

    if (ECJPAKECC26X2_waitForAccess(handle) != SemaphoreP_OK)
    {
        return ECJPAKE_STATUS_RESOURCE_UNAVAILABLE;
    }

    /* Copy over all parameters we will need access to in the FSM.
     * The FSM runs in SWI context and thus needs to keep track of
     * all of them somehow.
     */
    object->fsmState              = ECJPAKECC26X2_FSM_GENERATE_ZKP_PRIVATEKEY_X_HASH;
    object->operationStatus       = ECJPAKE_STATUS_ERROR;
    object->operation.generateZKP = operation;
    object->operationType         = ECJPAKE_OPERATION_TYPE_GENERATE_ZKP;
    object->operationCanceled     = false;
    scratchBufferSize             = SCRATCH_BUFFER_0_SIZE;

    /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
     * drivers and they all need to coexist. Whenever a driver starts an operation, it
     * registers its HWI callback with the OS.
     */
    HwiP_setFunc(&PKAResourceCC26XX_hwi, ECJPAKECC26X2_hwiFxn, (uintptr_t)handle);
    HwiP_setPriority(INT_PKA_IRQ, hwAttrs->intPriority);

    PKAResourceCC26XX_pollingFlag = 0;

    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /* Run the FSM by triggering the interrupt. It is level triggered
     * and the complement of the RUN bit.
     */
    IntEnable(INT_PKA_IRQ);

    return ECJPAKECC26X2_waitForResult(handle);
}

/*
 *  ======== ECJPAKE_verifyZKP ========
 */
int_fast16_t ECJPAKE_verifyZKP(ECJPAKE_Handle handle, ECJPAKE_OperationVerifyZKP *operation)
{
    ECJPAKECC26X2_Object *object         = handle->object;
    ECJPAKECC26X2_HWAttrs const *hwAttrs = handle->hwAttrs;

    if (ECJPAKECC26X2_waitForAccess(handle) != SemaphoreP_OK)
    {
        return ECJPAKE_STATUS_RESOURCE_UNAVAILABLE;
    }

    /* Copy over all parameters we will need access to in the FSM.
     * The FSM runs in SWI context and thus needs to keep track of
     * all of them somehow.
     */
    object->fsmState            = ECJPAKECC26X2_FSM_VERIFY_ZKP_VALIDATE_PUBLIC_KEY;
    object->operationStatus     = ECJPAKE_STATUS_ERROR;
    object->operation.verifyZKP = operation;
    object->operationType       = ECJPAKE_OPERATION_TYPE_VERIFY_ZKP;
    object->operationCanceled   = false;
    scratchBufferSize           = SCRATCH_BUFFER_0_SIZE;

    /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
     * drivers and they all need to coexist. Whenever a driver starts an operation, it
     * registers its HWI callback with the OS.
     */
    HwiP_setFunc(&PKAResourceCC26XX_hwi, ECJPAKECC26X2_hwiFxn, (uintptr_t)handle);
    HwiP_setPriority(INT_PKA_IRQ, hwAttrs->intPriority);

    PKAResourceCC26XX_pollingFlag = 0;

    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /* Run the FSM by triggering the interrupt. It is level triggered
     * and the complement of the RUN bit.
     */
    IntEnable(INT_PKA_IRQ);

    return ECJPAKECC26X2_waitForResult(handle);
}

/*
 *  ======== ECJPAKE_roundTwoGenerateKeys ========
 */
int_fast16_t ECJPAKE_roundTwoGenerateKeys(ECJPAKE_Handle handle, ECJPAKE_OperationRoundTwoGenerateKeys *operation)
{
    bool isKeyBlank;
    ECJPAKECC26X2_Object *object         = handle->object;
    ECJPAKECC26X2_HWAttrs const *hwAttrs = handle->hwAttrs;

    /* Ignore return since CryptoKey_isBlank always returns success */
    (void)CryptoKey_isBlank(operation->theirNewGenerator, &isKeyBlank);
    if (!isKeyBlank)
    {
        return ECJPAKE_STATUS_OUTPUT_KEY_NOT_BLANK;
    }

    (void)CryptoKey_isBlank(operation->myNewGenerator, &isKeyBlank);
    if (!isKeyBlank)
    {
        return ECJPAKE_STATUS_OUTPUT_KEY_NOT_BLANK;
    }

    (void)CryptoKey_isBlank(operation->myCombinedPrivateKey, &isKeyBlank);
    if (!isKeyBlank)
    {
        return ECJPAKE_STATUS_OUTPUT_KEY_NOT_BLANK;
    }

    (void)CryptoKey_isBlank(operation->myCombinedPublicKey, &isKeyBlank);
    if (!isKeyBlank)
    {
        return ECJPAKE_STATUS_OUTPUT_KEY_NOT_BLANK;
    }

    (void)CryptoKey_isBlank(operation->myPublicV, &isKeyBlank);
    if (!isKeyBlank)
    {
        return ECJPAKE_STATUS_OUTPUT_KEY_NOT_BLANK;
    }

    if (ECJPAKECC26X2_waitForAccess(handle) != SemaphoreP_OK)
    {
        return ECJPAKE_STATUS_RESOURCE_UNAVAILABLE;
    }

    /* Copy over all parameters we will need access to in the FSM.
     * The FSM runs in SWI context and thus needs to keep track of
     * all of them somehow.
     */
    object->fsmState                       = ECJPAKECC26X2_FSM_ROUND_TWO_MULT_MYPRIVATEKEY2_BY_PRESHAREDSECRET;
    object->operationStatus                = ECJPAKE_STATUS_ERROR;
    object->operation.generateRoundTwoKeys = operation;
    object->operationType                  = ECJPAKE_OPERATION_TYPE_ROUND_TWO_GENERATE_KEYS;
    object->operationCanceled              = false;
    scratchBufferSize                      = SCRATCH_BUFFER_0_SIZE;

    /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
     * drivers and they all need to coexist. Whenever a driver starts an operation, it
     * registers its HWI callback with the OS.
     */
    HwiP_setFunc(&PKAResourceCC26XX_hwi, ECJPAKECC26X2_hwiFxn, (uintptr_t)handle);
    HwiP_setPriority(INT_PKA_IRQ, hwAttrs->intPriority);

    PKAResourceCC26XX_pollingFlag = 0;

    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /* Run the FSM by triggering the interrupt. It is level triggered
     * and the complement of the RUN bit.
     */
    IntEnable(INT_PKA_IRQ);

    return ECJPAKECC26X2_waitForResult(handle);
}

/*
 *  ======== ECJPAKE_computeSharedSecret ========
 */
int_fast16_t ECJPAKE_computeSharedSecret(ECJPAKE_Handle handle, ECJPAKE_OperationComputeSharedSecret *operation)
{
    bool isKeyBlank;
    ECJPAKECC26X2_Object *object         = handle->object;
    ECJPAKECC26X2_HWAttrs const *hwAttrs = handle->hwAttrs;

    /* Ignore return since CryptoKey_isBlank always returns success */
    (void)CryptoKey_isBlank(operation->sharedSecret, &isKeyBlank);
    if (!isKeyBlank)
    {
        return ECJPAKE_STATUS_OUTPUT_KEY_NOT_BLANK;
    }

    if (ECJPAKECC26X2_waitForAccess(handle) != SemaphoreP_OK)
    {
        return ECJPAKE_STATUS_RESOURCE_UNAVAILABLE;
    }

    /* Copy over all parameters we will need access to in the FSM.
     * The FSM runs in SWI context and thus needs to keep track of
     * all of them somehow.
     */
    object->fsmState        = ECJPAKECC26X2_FSM_GENERATE_SHARED_SECRET_MULT_THEIRPUBLICKEY2_BY_MYCOMBINEDPRIVATEKEY;
    object->operationStatus = ECJPAKE_STATUS_ERROR;
    object->operation.computeSharedSecret = operation;
    object->operationType                 = ECJPAKE_OPERATION_TYPE_COMPUTE_SHARED_SECRET;
    object->operationCanceled             = false;
    scratchBufferSize                     = SCRATCH_BUFFER_0_SIZE;

    /* We need to set the HWI function and priority since the same physical interrupt is shared by multiple
     * drivers and they all need to coexist. Whenever a driver starts an operation, it
     * registers its HWI callback with the OS.
     */
    HwiP_setFunc(&PKAResourceCC26XX_hwi, ECJPAKECC26X2_hwiFxn, (uintptr_t)handle);
    HwiP_setPriority(INT_PKA_IRQ, hwAttrs->intPriority);

    PKAResourceCC26XX_pollingFlag = 0;

    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /* Run the FSM by triggering the interrupt. It is level triggered
     * and the complement of the RUN bit.
     */
    IntEnable(INT_PKA_IRQ);

    return ECJPAKECC26X2_waitForResult(handle);
}

/*
 *  ======== ECJPAKE_cancelOperation ========
 */
int_fast16_t ECJPAKE_cancelOperation(ECJPAKE_Handle handle)
{
    ECJPAKECC26X2_Object *object = handle->object;

    if (!object->operationInProgress)
    {
        return ECJPAKE_STATUS_ERROR;
    }

    object->operationCanceled = true;

    /* Post hwi as if operation finished for cleanup */
    IntEnable(INT_PKA_IRQ);
    HwiP_post(INT_PKA_IRQ);

    return ECJPAKE_STATUS_SUCCESS;
}