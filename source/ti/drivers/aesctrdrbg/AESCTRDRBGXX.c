/*
 * Copyright (c) 2019-2024, Texas Instruments Incorporated
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

#include <ti/drivers/AESCTRDRBG.h>
#include <ti/drivers/aesctrdrbg/AESCTRDRBGXX.h>
#include <ti/drivers/AESCTR.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
#include <ti/drivers/cryptoutils/utils/CryptoUtils.h>

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
    #include <ti/drivers/cryptoutils/hsm/HSMLPF3.h>
    #include <ti/drivers/cryptoutils/hsm/HSMLPF3Utility.h>
#endif

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/aes.h)

#if (ENABLE_KEY_STORAGE == 1)
    #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA.h>
    #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_helpers.h>
    #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_init.h>
    #if (TFM_ENABLED == 1)
        #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_s.h>
    #endif
#endif

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC23X0) || (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
    #include <ti/drivers/aesctr/AESCTRLPF3.h>
#else
    #include <ti/drivers/aesctr/AESCTRCC26XX.h>
#endif

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
    #define CryptoKeyPlaintext_initKey CryptoKeyPlaintextHSM_initKey
#endif

/* Forward declarations */
static void AESCTRDRBGXX_addBigendCounter(uint8_t *counter, uint32_t increment);
static int_fast16_t AESCTRDRBGXX_updateState(AESCTRDRBG_Handle handle,
                                             const void *additionalData,
                                             size_t additionalDataLength);
static void AESCTRDRBG_uninstantiate(AESCTRDRBG_Handle handle);

/* Static globals */
static bool isInitialized = false;

#define CEIL(x, y) (1 + (((x)-1) / (y)))

/*
 *  ======== AESCTRDRBG_init ========
 */
void AESCTRDRBG_init(void)
{
    AESCTR_init();

    isInitialized = true;
}

/*
 *  ======== AESCTRDRBGXX_updateState ========
 */
static int_fast16_t AESCTRDRBGXX_updateState(AESCTRDRBG_Handle handle,
                                             const void *additionalData,
                                             size_t additionalDataLength)
{
    AESCTRDRBGXX_Object *object;
    AESCTR_Operation operation;
    /*
     * Buffer must be word aligned as some AESCTR implementations require
     * word aligned I/O.
     */
    uint32_t buf32[(AESCTRDRBG_MAX_SEED_LENGTH + 3) / 4] = {0};

    object = handle->object;

    /*
     * We need to increment the counter here since regular AESCTR
     * only increments the counter after encrypting it while
     * AESCTRDRBG increments the counter before encrypting it.
     * We do not need to worry about the counter being 1 over afterwards
     * as we will replace the global counter with part of the
     * encrypted result.
     */
    AESCTRDRBGXX_addBigendCounter(object->counter, 1);

    /*
     * Wrap the memcpy below in a zero-length check. Do not remove it!
     * The explicit check is necessary for klocwork to stop emitting a critical
     * warning. Theoretically, memcpy with a length argument of 0 should do
     * nothing. However klocwork emits a warning that there is an out
     * of bounds array access (underflow) on buf32 if this check is not in place.
     */
    if (additionalDataLength > 0)
    {
        /*
         * Copy over any additional data and operate on buf32 in place. This way
         * we can have the case where additionalDataLength < seedLength. This is
         * useful in AESCTRDRBG_getBytes() to avoid allocating a spare empty
         * buffer.
         */
        memcpy(buf32, additionalData, additionalDataLength);
    }

    operation.key            = &object->key;
    operation.input          = (uint8_t *)buf32;
    operation.output         = (uint8_t *)buf32;
    operation.initialCounter = object->counter;
    operation.inputLength    = object->key.u.plaintext.keyLength + AESCTRDRBG_AES_BLOCK_SIZE_BYTES;

    if (AESCTR_oneStepEncrypt(object->ctrHandle, &operation) != AESCTR_STATUS_SUCCESS)
    {
        return AESCTRDRBG_STATUS_ERROR;
    }

    /* Copy the left most keyLength bytes of the computed result */
    memcpy(object->keyingMaterial, buf32, object->key.u.plaintext.keyLength);

    /*
     * Copy new counter value as the right most 16 bytes of the computed result.
     * The key length is always a word multiple number of bytes so we can divide
     * by word size to determine the buf32 index.
     */
    memcpy(object->counter,
           &buf32[object->key.u.plaintext.keyLength / sizeof(uint32_t)],
           AESCTRDRBG_AES_BLOCK_SIZE_BYTES);

    /* Wipe the stack buffer */
    memset(buf32, 0, object->seedLength);

    return AESCTRDRBG_STATUS_SUCCESS;
}

/*
 *  ======== AESCTRDRBGXX_addBigendCounter ========
 */
static void AESCTRDRBGXX_addBigendCounter(uint8_t *counter, uint32_t increment)
{
    uint64_t *counter64 = (uint64_t *)counter;
    uint64_t prior;

    /* Turn it into a little-endian counter */
    CryptoUtils_reverseBufferBytewise(counter64, AESCTRDRBG_AES_BLOCK_SIZE_BYTES);

    prior = counter64[0];

    /* Increment as a 64-bit number */
    counter64[0] += increment;

    /* Check if we wrapped and need to increment the upper 64 bits */
    if (counter64[0] < prior)
    {
        counter64[1]++;
    }

    /* Turn it back into a big-endian integer */
    CryptoUtils_reverseBufferBytewise(counter64, AESCTRDRBG_AES_BLOCK_SIZE_BYTES);
}

/*
 *  ======== AESCTRDRBG_uninstantiate ========
 *
 *  Per the NIST Recommendation SP 800-90A Rev. 1 for DRBG, uninstantiate is the operation
 *  of clearing the internal state {keyingMaterial, counter, reseedCounter} by writing
 *  all 0's. Once uninstantiated, the DRBG instance shall not be used until it's
 *  instantiated again with a fresh seed.
 *
 *  This implementation also sets the isInstantiated flag to false. This function
 *  should be called if any of the AESCTR operations fail so that the DRBG instance
 *  will never be usable when its internal state is potentially corrupt.
 */
static void AESCTRDRBG_uninstantiate(AESCTRDRBG_Handle handle)
{
    AESCTRDRBGXX_Object *object;

    object = handle->object;

    object->isInstantiated = false;
    memset(object->keyingMaterial, 0, object->key.u.plaintext.keyLength);
    memset(object->counter, 0, AESCTRDRBG_AES_BLOCK_SIZE_BYTES);
    object->reseedCounter = 0;
}

/*
 *  ======== AESCTRDRBG_construct ========
 */
AESCTRDRBG_Handle AESCTRDRBG_construct(AESCTRDRBG_Config *config, const AESCTRDRBG_Params *params)
{
    AESCTRDRBG_Handle handle = (AESCTRDRBG_Handle)config;
    AESCTRDRBGXX_Object *object;
    const AESCTRDRBGXX_HWAttrs *hwAttrs;
    AESCTR_Params ctrParams;
    uintptr_t key;
    int_fast16_t status;

    /* There are no valid default params for this driver */
    if (params == NULL)
    {
        return NULL;
    }

    DebugP_assert(handle);
    object  = handle->object;
    hwAttrs = handle->hwAttrs;

    key = HwiP_disable();

    if (!isInitialized || object->isOpen)
    {
        HwiP_restore(key);
        return NULL;
    }

    object->isOpen = true;

    HwiP_restore(key);

    /*
     * personalizationDataLength must be within
     * [0, AESCTRDRBG_AES_BLOCK_SIZE_BYTES + KeyLength] bytes.
     */
    if (params->personalizationDataLength > params->keyLength + AESCTRDRBG_AES_BLOCK_SIZE_BYTES)
    {
        return NULL;
    }

    /* Open the driver's AESCTR instance */
    AESCTR_Params_init(&ctrParams);
#if (TFM_ENABLED == 0)
    ctrParams.returnBehavior = (AESCTR_ReturnBehavior)(params->returnBehavior);
#else
    /*
     * For the secure-only implementation, AESCTRDRBG supports blocking or
     * polling return behavior.  However, when TF-M is enabled, polling return
     * behavior must be forced since drivers cannot block inside the TF-M.
     */
    ctrParams.returnBehavior = AESCTR_RETURN_BEHAVIOR_POLLING;
#endif

    /* Zero out the AESCTR object to ensure AESCTR_construct() will not fail */
    memset(&object->aesctrObject, 0, sizeof(object->aesctrObject));
    object->ctrConfig.object  = &object->aesctrObject;
    object->ctrConfig.hwAttrs = &hwAttrs->aesctrHWAttrs;

    object->ctrHandle = AESCTR_construct(&object->ctrConfig, &ctrParams);

    if (object->ctrHandle == NULL)
    {
        object->isOpen = false;

        return NULL;
    }

    /* Initialize CryptoKey for later use */
    CryptoKeyPlaintext_initKey(&object->key, object->keyingMaterial, params->keyLength);

    /* Zero-out counter and keyingMaterial */
    memset(object->counter, 0, AESCTRDRBG_AES_BLOCK_SIZE_BYTES);
    memset(object->keyingMaterial, 0, params->keyLength);

    /* Store constants for later */
    object->seedLength     = params->keyLength + AESCTRDRBG_AES_BLOCK_SIZE_BYTES;
    object->reseedInterval = params->reseedInterval;

    /* Ideally this should be set only after instantiation is complete. However
     * since this implementation uses the reseed function, this flag is set here
     * to ensure it doesn't fail with AESCTRDRBG_STATUS_UNINSTANTIATED.
     * Note that if reseed fails due to other reasons, the following call to
     * uninstantiate will clear this flag.
     */
    object->isInstantiated = true;

    /* Reseed the instance to generate the initial (counter, keyingMaterial) pair */
    status = AESCTRDRBG_reseed(handle, params->seed, params->personalizationData, params->personalizationDataLength);

    if (status != AESCTRDRBG_STATUS_SUCCESS)
    {
        AESCTR_close(object->ctrHandle);
        AESCTRDRBG_uninstantiate(handle);
        object->isOpen = false;

        return NULL;
    }

    return handle;
}

/*
 *  ======== AESCTRDRBG_close ========
 */
void AESCTRDRBG_close(AESCTRDRBG_Handle handle)
{
    AESCTRDRBGXX_Object *object;

    DebugP_assert(handle);

    /* Get the pointer to the object and hwAttrs */
    object = handle->object;

    AESCTR_close(object->ctrHandle);

    AESCTRDRBG_uninstantiate(handle);

    /* Mark the module as available */
    object->isOpen = false;
}

/*
 *  ======== AESCTRDRBG_getBytes ========
 */
int_fast16_t AESCTRDRBG_getBytes(AESCTRDRBG_Handle handle, CryptoKey *randomBytes)
{
    return AESCTRDRBG_generateKey(handle, randomBytes);
}

/*
 *  ======== AESCTRDRBG_generateKey ========
 */
int_fast16_t AESCTRDRBG_generateKey(AESCTRDRBG_Handle handle, CryptoKey *randomKey)
{
    int_fast16_t status = AESCTRDRBG_STATUS_ERROR;
#if (ENABLE_KEY_STORAGE == 1)
    int_fast16_t keyStoreStatus = KEYSTORE_PSA_STATUS_GENERIC_ERROR;
    uint8_t KeyStore_keyingMaterial[AESCTRDRBG_MAX_KEYSTORE_KEY_SIZE];
    KeyStore_PSA_KeyAttributes attributes = KEYSTORE_PSA_KEY_ATTRIBUTES_INIT;
    KeyStore_PSA_KeyAttributes *attributesPtr;

    attributesPtr = &attributes;

#endif /* ENABLE_KEY_STORAGE */

    if (randomKey != NULL)
    {
        if (randomKey->encoding == CryptoKey_BLANK_PLAINTEXT)
        {
            status = AESCTRDRBG_getRandomBytes(handle,
                                               randomKey->u.plaintext.keyMaterial,
                                               randomKey->u.plaintext.keyLength);
            if (status == AESCTRDRBG_STATUS_SUCCESS)
            {
                randomKey->encoding = CryptoKey_PLAINTEXT;
            }
        }
#if (ENABLE_KEY_STORAGE == 1)
        else if (randomKey->encoding == CryptoKey_BLANK_KEYSTORE)
        {
            if ((randomKey->u.keyStore.keyLength != 0) &&
                (randomKey->u.keyStore.keyLength <= AESCTRDRBG_MAX_KEYSTORE_KEY_SIZE))
            {
                /* Copy keyAttributes from CryptoKey structure */
    #if (TFM_ENABLED == 0)
                attributesPtr = (KeyStore_PSA_KeyAttributes *)randomKey->u.keyStore.keyAttributes;
    #else
                keyStoreStatus = KeyStore_s_copyKeyAttributesFromClient((struct psa_client_key_attributes_s *)
                                                                            randomKey->u.keyStore.keyAttributes,
                                                                        KEYSTORE_PSA_DEFAULT_OWNER,
                                                                        attributesPtr);
    #endif
                status = AESCTRDRBG_getRandomBytes(handle, KeyStore_keyingMaterial, randomKey->u.keyStore.keyLength);

                if (status == AESCTRDRBG_STATUS_SUCCESS)
                {
                    keyStoreStatus = KeyStore_PSA_importKey(attributesPtr,
                                                            KeyStore_keyingMaterial,
                                                            randomKey->u.keyStore.keyLength,
                                                            &attributesPtr->MBEDTLS_PRIVATE(core).MBEDTLS_PRIVATE(id));
                    if (keyStoreStatus == KEYSTORE_PSA_STATUS_SUCCESS)
                    {
                        if (attributesPtr->MBEDTLS_PRIVATE(core).MBEDTLS_PRIVATE(lifetime) ==
                            KEYSTORE_PSA_KEY_LIFETIME_VOLATILE)
                        {
                            /* Set the keyID of volatile keys provided by KeyStore driver in the cryptokey structure */
                            KeyStore_PSA_initKey(randomKey,
                                                 attributesPtr->MBEDTLS_PRIVATE(core).MBEDTLS_PRIVATE(id),
                                                 randomKey->u.keyStore.keyLength,
                                                 NULL);
                        }
                        else
                        {
                            /* Only update the KeyStore encoding for persistent keys */
                            randomKey->encoding = CryptoKey_KEYSTORE;
                        }
                        status = AESCTRDRBG_STATUS_SUCCESS;
                    }
                    else
                    {
                        status = AESCTRDRBG_STATUS_KEYSTORE_ERROR;
                    }
                }
            }
        }
#endif /* ENABLE_KEY_STORAGE */
    }

    return status;
}

/*
 *  ======== AESCTRDRBG_getRandomBytes ========
 */
int_fast16_t AESCTRDRBG_getRandomBytes(AESCTRDRBG_Handle handle, void *randomBytes, size_t randomBytesSize)
{
    AESCTRDRBGXX_Object *object;
    AESCTR_Operation operation;
    int_fast16_t status;
#if (DeviceFamily_PARENT != DeviceFamily_PARENT_CC27XX)
    bool lockAcquired;
    uint32_t lockAcquireTimeout;
#endif

    object = handle->object;

    if (object->isInstantiated == false)
    {
        return AESCTRDRBG_STATUS_UNINSTANTIATED;
    }

    if (object->reseedCounter >= object->reseedInterval)
    {
        return AESCTRDRBG_STATUS_RESEED_REQUIRED;
    }

    /* For CC27XX devices, AES-CTR DRBG leverages the HSM engine for encryption operations.
     * The underlying SW architecture maintains an internal mutex mechanism to ensure only
     * one driver instance is utilizing the HSM engine at once. Therefore, there is no need
     * to call `AESCTR_acquireLock`, `AESCTR_disableThreadSafety`, and `AESCTR_releaseLock`.
     */
#if (DeviceFamily_PARENT != DeviceFamily_PARENT_CC27XX)
    if (SwiP_inISR() || HwiP_inISR())
    {
        lockAcquireTimeout = SemaphoreP_NO_WAIT;
    }
    else
    {
        lockAcquireTimeout = SemaphoreP_WAIT_FOREVER;
    }

    lockAcquired = AESCTR_acquireLock(object->ctrHandle, lockAcquireTimeout);
    if (!lockAcquired)
    {
        return AESCTRDRBG_STATUS_RESOURCE_UNAVAILABLE;
    }

    AESCTR_disableThreadSafety(object->ctrHandle);
#endif

    /* Set the keying material of the CryptoKey to 0.
     * If we use AESCTR to encrypt a buffer full of zeros,
     * the resultant output will be the bitstream of the
     * encrypted counters. That is what is used as
     * random bits by AESCTRDRBG.
     * Zeroing out the keying material and performing
     * the AESCTR encryption in place saves us from
     * allocating a buffer of the right length full
     * of zeros or repeatedly encrypting a 16-byte
     * buffer full of zeros.
     */
    memset(randomBytes, 0, randomBytesSize);

    /* We need to increment the counter here since regular AESCTR
     * only increments the counter after encrypting it while
     * AESCTRDRBG increments the counter before encrypting it.
     */
    AESCTRDRBGXX_addBigendCounter(object->counter, 1);

    operation.key            = &object->key;
    operation.input          = randomBytes;
    operation.output         = randomBytes;
    operation.initialCounter = object->counter;
    operation.inputLength    = randomBytesSize;

    status = AESCTR_oneStepEncrypt(object->ctrHandle, &operation);

    if (status != AESCTR_STATUS_SUCCESS)
    {
#if (DeviceFamily_PARENT != DeviceFamily_PARENT_CC27XX)
        AESCTR_releaseLock(object->ctrHandle);
#endif

        if (status == AESCTR_STATUS_UNALIGNED_IO_NOT_SUPPORTED)
        {
            status = AESCTRDRBG_STATUS_UNALIGNED_IO_NOT_SUPPORTED;
        }
        else
        {
            AESCTRDRBG_uninstantiate(handle);
            status = AESCTRDRBG_STATUS_UNINSTANTIATED;
        }

        return status;
    }

    /* Add the number of counter blocks we produced to the
     * internal counter. We already incremented by one above
     * so we increment by one less here.
     */
    AESCTRDRBGXX_addBigendCounter(object->counter, CEIL(randomBytesSize, AESCTRDRBG_AES_BLOCK_SIZE_BYTES) - 1);

    status = AESCTRDRBGXX_updateState(handle, NULL, 0);

#if (DeviceFamily_PARENT != DeviceFamily_PARENT_CC27XX)
    AESCTR_enableThreadSafety(object->ctrHandle);
    AESCTR_releaseLock(object->ctrHandle);
#endif

    if (status != AESCTRDRBG_STATUS_SUCCESS)
    {
        AESCTRDRBG_uninstantiate(handle);
        return AESCTRDRBG_STATUS_UNINSTANTIATED;
    }

    object->reseedCounter += 1;

    return AESCTRDRBG_STATUS_SUCCESS;
}

/*
 *  ======== AESCTRDRBG_reseed ========
 */
int_fast16_t AESCTRDRBG_reseed(AESCTRDRBG_Handle handle,
                               const void *seed,
                               const void *additionalData,
                               size_t additionalDataLength)
{
    AESCTRDRBGXX_Object *object;
    int_fast16_t status;
    uint8_t tmp[AESCTRDRBG_MAX_SEED_LENGTH];
    uint32_t i;
#if (DeviceFamily_PARENT != DeviceFamily_PARENT_CC27XX)
    bool lockAcquired;
    uint32_t lockAcquireTimeout;
#endif

    object = handle->object;

    if (object->isInstantiated == false)
    {
        return AESCTRDRBG_STATUS_UNINSTANTIATED;
    }

    if (additionalDataLength > object->seedLength)
    {
        return AESCTRDRBG_STATUS_ERROR;
    }

    /* For CC27XX devices, AES-CTR DRBG leverages the HSM engine for encryption operations.
     * The underlying SW architecture maintains an internal mutex mechanism to ensure only
     * one driver instance is utilizing the HSM engine at once. Therefore, there is no need
     * to call `AESCTR_acquireLock`, `AESCTR_disableThreadSafety`, and `AESCTR_releaseLock`.
     */
#if (DeviceFamily_PARENT != DeviceFamily_PARENT_CC27XX)
    if (SwiP_inISR() || HwiP_inISR())
    {
        lockAcquireTimeout = SemaphoreP_NO_WAIT;
    }
    else
    {
        lockAcquireTimeout = SemaphoreP_WAIT_FOREVER;
    }

    lockAcquired = AESCTR_acquireLock(object->ctrHandle, lockAcquireTimeout);
    if (!lockAcquired)
    {
        return AESCTRDRBG_STATUS_RESOURCE_UNAVAILABLE;
    }

    AESCTR_disableThreadSafety(object->ctrHandle);
#endif

    /* Set temporary buffer as additionalData padded with zeros */
    memset(tmp, 0, object->seedLength);
    memcpy(tmp, additionalData, additionalDataLength);

    /* XOR-in the seed. It should always be a multiple of 32 bits */
    for (i = 0; i < object->seedLength / sizeof(uint32_t); i++)
    {
        ((uint32_t *)tmp)[i] ^= ((uint32_t *)seed)[i];
    }

    /* Use the combined seed to generate a new (counter, keyingMaterial) pair */
    status = AESCTRDRBGXX_updateState(handle, tmp, object->seedLength);

#if (DeviceFamily_PARENT != DeviceFamily_PARENT_CC27XX)
    AESCTR_enableThreadSafety(object->ctrHandle);
    AESCTR_releaseLock(object->ctrHandle);
#endif

    if (status != AESCTRDRBG_STATUS_SUCCESS)
    {
        AESCTRDRBG_uninstantiate(handle);
        return AESCTRDRBG_STATUS_UNINSTANTIATED;
    }

    object->reseedCounter = 1;

    return AESCTRDRBG_STATUS_SUCCESS;
}

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
/*
 *  ======== AESCTRDRBG_getRandomBytesFromHSM ========
 */
int_fast16_t AESCTRDRBG_getRandomBytesFromHSM(void *randomBytes, size_t randomBytesSize)
{
    int_fast16_t status    = AESCTRDRBG_STATUS_ERROR;
    int_fast16_t hsmRetval = HSMLPF3_STATUS_ERROR;
    int32_t tokenResult    = 0U;

    if ((randomBytesSize > HSM_RAW_RNG_MAX_LENGTH) || (!HSM_IS_SIZE_MULTIPLE_OF_WORD(randomBytesSize)))
    {
        /* Return error. */
        return AESCTRDRBG_STATUS_INPUT_LENGTH_INVALID;
    }

    if (!HSMLPF3_acquireLock(SemaphoreP_NO_WAIT, (uintptr_t)0U))
    {
        return AESCTRDRBG_STATUS_RESOURCE_UNAVAILABLE;
    }

    Power_setConstraint(PowerLPF3_DISALLOW_STANDBY);

    /* Populates the HSMLPF3 commandToken as a RNG get DRBG random number operation */
    HSMLPF3_constructRNGGetRandomNumberPhysicalToken((uintptr_t)randomBytes, randomBytesSize);

    /* Submit token to the HSM IP engine */
    hsmRetval = HSMLPF3_submitToken(HSMLPF3_RETURN_BEHAVIOR_POLLING, NULL, (uintptr_t)0U);

    if (hsmRetval == HSMLPF3_STATUS_SUCCESS)
    {
        /* Handles post command token submission mechanism.
         * Waits for a result token from the HSM IP in polling and blocking modes (and calls the drivers post-processing
         * fxn) and returns immediately when in callback mode.
         */
        hsmRetval = HSMLPF3_waitForResult();

        if (hsmRetval == HSMLPF3_STATUS_SUCCESS)
        {
            tokenResult = HSMLPF3_getResultCode();

            if ((tokenResult & HSMLPF3_RETVAL_MASK) == EIP130TOKEN_RESULT_SUCCESS)
            {
                status = AESCTRDRBG_STATUS_SUCCESS;
            }
        }
    }

    HSMLPF3_releaseLock();

    Power_releaseConstraint(PowerLPF3_DISALLOW_STANDBY);

    return status;
}

/*
 *  ======== AESCTRDRBG_reseedHSMPostProcessing ========
 */
static inline void AESCTRDRBG_reseedHSMPostProcessing(uintptr_t arg0)
{
    HSMLPF3_releaseLock();

    Power_releaseConstraint(PowerLPF3_DISALLOW_STANDBY);
}

/*
 *  ======== AESCTRDRBG_reseedHSM ========
 */
int_fast16_t AESCTRDRBG_reseedHSM(void)
{
    int_fast16_t status    = AESCTRDRBG_STATUS_ERROR;
    int_fast16_t hsmRetval = HSMLPF3_STATUS_ERROR;

    if (!HSMLPF3_acquireLock(SemaphoreP_NO_WAIT, (uintptr_t)0U))
    {
        return AESCTRDRBG_STATUS_RESOURCE_UNAVAILABLE;
    }

    Power_setConstraint(PowerLPF3_DISALLOW_STANDBY);

    /* Populates the HSMLPF3 commandToken as an RNG configure operation */
    HSMLPF3_constructRNGReseedDRBGPhysicalToken();

    /* Submit token to the HSM IP engine */
    hsmRetval = HSMLPF3_submitToken(HSMLPF3_RETURN_BEHAVIOR_CALLBACK,
                                    AESCTRDRBG_reseedHSMPostProcessing,
                                    (uintptr_t)0U);

    if (hsmRetval == HSMLPF3_STATUS_SUCCESS)
    {
        status = AESCTRDRBG_STATUS_SUCCESS;
    }
    else
    {
        HSMLPF3_releaseLock();

        Power_releaseConstraint(PowerLPF3_DISALLOW_STANDBY);
    }

    return status;
}
#endif