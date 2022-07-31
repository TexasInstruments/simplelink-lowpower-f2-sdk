/*
 * Copyright (c) 2021, Texas Instruments Incorporated
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
/*
 *  ======== RNGCC26XX.c ========
 */

#include <string.h>

#include <ti/drivers/RNG.h>
#include <ti/drivers/rng/RNGCC26XX.h>
#include <ti/drivers/TRNG.h>
#include <ti/drivers/trng/TRNGCC26XX.h>
#include <ti/drivers/cryptoutils/utils/CryptoUtils.h>
#include <ti/drivers/dpl/SemaphoreP.h>
#include <ti/drivers/dpl/HwiP.h>

extern uint8_t RNG_instancePool[];

extern const TRNGCC26XX_HWAttrs RNGCC26XX_trngHWAttrs;

typedef struct RNGCC26XX_OperationParameters_ RNGCC26XX_OperationParameters;

typedef bool (*RNGCC26XX_validator)(RNGCC26XX_OperationParameters *opParams);

/*
 * These values are needed when operating in callback mode.
 */
struct RNGCC26XX_OperationParameters_
{
    RNG_Handle rngHandle;
    uint8_t *output;
    size_t outputBitLength;
    CryptoKey *destinationKey;
    CryptoUtils_Endianess endianess;
    RNGCC26XX_validator validator;
    const uint8_t *lowerLimit;
    const uint8_t *upperLimit;
};

typedef struct
{
    /* No data in the structure should be read or written without first taking this semaphore. */
    SemaphoreP_Struct accessSemaphore;
    size_t poolLevel;
    RNGCC26XX_OperationParameters operationalParameters;
} RNGCC26XX_Instance;

RNGCC26XX_Instance RNG_instanceData;

static bool RNGCC26XX_isInitialized = false;

/*** Prototypes ***/
static int_fast16_t RNGCC26XX_translateTRNGStatus(int_fast16_t trngStatus);
static int_fast16_t RNGCC26XX_generateEntropy(TRNG_Handle trngHandle, uint8_t *byteDest, size_t byteSize);
static void RNGCC26XX_invokeCallback(RNG_Handle handle,
                                     uint8_t *randomBits,
                                     size_t bitLength,
                                     CryptoKey *generatedKey,
                                     int_fast16_t returnValue);
static void RNGCC26XX_trngCallbackFxn(TRNG_Handle trngHandle,
                                      int_fast16_t returnValue,
                                      uint8_t *trngOutputBytes,
                                      size_t trngOutputSize);
static size_t RNGCC26XX_getEntropyFromPool(void *dest, size_t byteSize);
static bool RNGCC26XX_checkRange(RNGCC26XX_OperationParameters *opParams);
static int_fast16_t RNGCC26XX_getValidatedNumber(RNG_Handle handle,
                                                 void *randomNumber,
                                                 size_t randomNumberBitLength,
                                                 CryptoKey *key,
                                                 CryptoUtils_Endianess endianess,
                                                 RNGCC26XX_validator validator,
                                                 const void *lowerLimit,
                                                 const void *upperLimit);

static int_fast16_t RNGCC26XX_translateTRNGStatus(int_fast16_t trngStatus)
{
    int_fast16_t returnValue;

    switch (trngStatus)
    {
        case TRNG_STATUS_SUCCESS:
            returnValue = RNG_STATUS_SUCCESS;
            break;
        case TRNG_STATUS_ERROR:
            returnValue = RNG_STATUS_ERROR;
            break;
        case TRNG_STATUS_RESOURCE_UNAVAILABLE:
            returnValue = RNG_STATUS_RESOURCE_UNAVAILABLE;
            break;
        case TRNG_STATUS_CANCELED:
            returnValue = RNG_STATUS_CANCELED;
            break;
        default:
            returnValue = RNG_STATUS_ERROR;
            break;
    }

    return returnValue;
}

static int_fast16_t RNGCC26XX_generateEntropy(TRNG_Handle trngHandle, uint8_t *byteDest, size_t byteSize)
{
    int_fast16_t returnValue;
    int_fast16_t trngResult;

    trngResult = TRNG_getRandomBytes(trngHandle, byteDest, byteSize);

    returnValue = RNGCC26XX_translateTRNGStatus(trngResult);

    return returnValue;
}

static void RNGCC26XX_invokeCallback(RNG_Handle handle,
                                     uint8_t *randomBits,
                                     size_t bitLength,
                                     CryptoKey *generatedKey,
                                     int_fast16_t returnValue)
{
    RNG_Params *params = &((RNGCC26XX_Object *)handle->object)->rngParams;

    if ((generatedKey != NULL) && (params->cryptoKeyCallbackFxn != NULL))
    {
        params->cryptoKeyCallbackFxn(handle, returnValue, generatedKey);
    }
    else
    {
        if (params->randomBitsCallbackFxn != NULL)
        {
            params->randomBitsCallbackFxn(handle, returnValue, randomBits, bitLength);
        }
    }
}

static void RNGCC26XX_trngCallbackFxn(TRNG_Handle trngHandle,
                                      int_fast16_t returnValue,
                                      uint8_t *trngOutputBytes,
                                      size_t trngOutputSize)
{
    RNGCC26XX_OperationParameters *opParams = &RNG_instanceData.operationalParameters;
    int_fast16_t rngStatus;
    uint8_t bitMask;
    size_t totalByteSize;
    bool isValid = true;

    rngStatus = RNGCC26XX_translateTRNGStatus(returnValue);

    if (rngStatus == RNG_STATUS_SUCCESS)
    {

        totalByteSize = (opParams->outputBitLength + 7u) >> 3u;

        /* Mask out any extra bits copied */
        bitMask = (2u << (((opParams->outputBitLength + 7u) % 8u))) - 1u;
        if (opParams->endianess == CryptoUtils_ENDIANESS_BIG)
        {
            opParams->output[0] &= bitMask;
        }
        else
        {
            opParams->output[totalByteSize - 1u] &= bitMask;
        }

        /* Check resulting value against limits (if any). If lowerLimit is set, the upperLimit must also be set. */
        if (opParams->validator != NULL)
        {
            isValid = opParams->validator(opParams);

            if (!isValid)
            {
                rngStatus = RNGCC26XX_generateEntropy(trngHandle, opParams->output, totalByteSize);
            }
        }
    }

    if ((rngStatus != RNG_STATUS_SUCCESS) || isValid)
    {
        RNGCC26XX_invokeCallback(opParams->rngHandle,
                                 opParams->output,
                                 opParams->outputBitLength,
                                 opParams->destinationKey,
                                 rngStatus);
        SemaphoreP_post(&RNG_instanceData.accessSemaphore);
    }
}

/*
 * Precondition: RNG_instanceData.accessSemaphore has been taken.
 *
 * Returns number of _bytes_ remaining to fulfill the request (rounded up from number of bits remaining.)
 * These will have to be generated since a non-zero return value indicates the pool is now empty.
 */
static size_t RNGCC26XX_getEntropyFromPool(void *dest, size_t byteSize)
{

    uint8_t *byteDest = (uint8_t *)dest;

    size_t bytesRemaining = byteSize;
    size_t bytesToCopy;

    /* Get entropy from pool first */
    if ((bytesRemaining > 0u) && (RNG_instanceData.poolLevel > 0u))
    {
        bytesToCopy = (bytesRemaining > RNG_instanceData.poolLevel) ? RNG_instanceData.poolLevel : bytesRemaining;

        (void)memcpy(byteDest, &RNG_instancePool[RNG_instanceData.poolLevel - bytesToCopy], bytesToCopy);
        CryptoUtils_memset(&RNG_instancePool[RNG_instanceData.poolLevel - bytesToCopy],
                           RNG_poolByteSize,
                           0,
                           bytesToCopy);
        RNG_instanceData.poolLevel -= bytesToCopy;

        bytesRemaining -= bytesToCopy;
    }

    return bytesRemaining;
}

static bool RNGCC26XX_checkRange(RNGCC26XX_OperationParameters *opParams)
{
    return CryptoUtils_isNumberInRange(opParams->output,
                                       opParams->outputBitLength,
                                       opParams->endianess,
                                       opParams->lowerLimit,
                                       opParams->upperLimit);
}

static int_fast16_t RNGCC26XX_getValidatedNumber(RNG_Handle handle,
                                                 void *randomNumber,
                                                 size_t randomNumberBitLength,
                                                 CryptoKey *key,
                                                 CryptoUtils_Endianess endianess,
                                                 RNGCC26XX_validator validator,
                                                 const void *lowerLimit,
                                                 const void *upperLimit)
{
    int_fast16_t returnValue = RNG_STATUS_SUCCESS;
    RNGCC26XX_Object *object;
    size_t bytesToGenerate;
    size_t byteSize;
    uint8_t *byteDestination;
    uint8_t bitMask;
    bool isValid              = false;
    bool trngCallbackExpected = false;

    if ((handle == NULL) || (randomNumber == NULL) || (randomNumberBitLength >= RNG_MAX_BIT_LENGTH))
    {
        returnValue = RNG_STATUS_INVALID_INPUTS;
    }

    if (returnValue == RNG_STATUS_SUCCESS)
    {
        object = (RNGCC26XX_Object *)handle->object;

        if (SemaphoreP_pend(&RNG_instanceData.accessSemaphore, object->rngParams.timeout) != SemaphoreP_OK)
        {
            returnValue = RNG_STATUS_RESOURCE_UNAVAILABLE;
        }
    }

    if (returnValue == RNG_STATUS_SUCCESS)
    {
        RNG_instanceData.operationalParameters.rngHandle       = handle;
        RNG_instanceData.operationalParameters.outputBitLength = randomNumberBitLength;
        RNG_instanceData.operationalParameters.output          = randomNumber;
        RNG_instanceData.operationalParameters.destinationKey  = key;
        RNG_instanceData.operationalParameters.endianess       = endianess;
        RNG_instanceData.operationalParameters.validator       = validator;
        RNG_instanceData.operationalParameters.lowerLimit      = lowerLimit;
        RNG_instanceData.operationalParameters.upperLimit      = upperLimit;

        byteSize        = (randomNumberBitLength + 7u) >> 3u;
        byteDestination = (uint8_t *)randomNumber;
        bitMask         = (2u << (((randomNumberBitLength + 7u) % 8u))) - 1u;
    }

    while ((returnValue == RNG_STATUS_SUCCESS) && !isValid && !trngCallbackExpected)
    {

        bytesToGenerate = RNGCC26XX_getEntropyFromPool(byteDestination, byteSize);

        if (bytesToGenerate > 0u)
        {

            if (object->rngParams.returnBehavior == RNG_RETURN_BEHAVIOR_CALLBACK)
            {
                trngCallbackExpected = true;
            }

            returnValue = RNGCC26XX_generateEntropy(&object->trngConfig,
                                                    &byteDestination[byteSize - bytesToGenerate],
                                                    bytesToGenerate);

            if (returnValue != RNG_STATUS_SUCCESS)
            {
                /*
                 * Regardless of return behavior, if there was an error,
                 * it is expected the TRNG callback will not execute.
                 */
                trngCallbackExpected = false;
            }
        }

        if (!trngCallbackExpected)
        {
            /* All bytes needed have been obtained, Mask off extra bits in MSB */
            if (endianess == CryptoUtils_ENDIANESS_BIG)
            {
                byteDestination[0] &= bitMask;
            }
            else
            {
                byteDestination[byteSize - 1u] &= bitMask;
            }

            if (validator != NULL)
            {
                isValid = validator(&RNG_instanceData.operationalParameters);
            }
            else
            {
                isValid = true;
            }
        }
    }

    if (!trngCallbackExpected)
    {
        if (returnValue == RNG_STATUS_SUCCESS)
        {
            if (object->rngParams.returnBehavior == RNG_RETURN_BEHAVIOR_CALLBACK)
            {
                RNGCC26XX_invokeCallback(handle, randomNumber, randomNumberBitLength, key, returnValue);
            }
        }

        SemaphoreP_post(&RNG_instanceData.accessSemaphore);
    }

    return returnValue;
}

void RNG_Params_init(RNG_Params *params)
{
    *params = RNG_defaultParams;
}

int_fast16_t RNG_init(void)
{
    int_fast16_t returnValue = RNG_STATUS_SUCCESS;

    if (RNGCC26XX_isInitialized == false)
    {
        RNG_instanceData.poolLevel = 0u;
        if (SemaphoreP_constructBinary(&RNG_instanceData.accessSemaphore, 1) == NULL)
        {
            returnValue = RNG_STATUS_ERROR;
        }
        else
        {
            TRNG_init();
            RNGCC26XX_isInitialized = true;
        }
    }

    return returnValue;
}

RNG_Handle RNG_construct(const RNG_Config *config, const RNG_Params *params)
{
    const RNG_Config *handle;
    RNGCC26XX_Object *object;
    TRNG_Params trngParams;

    handle = (const RNG_Config *)config;
    object = (RNGCC26XX_Object *)handle->object;

    TRNG_Params_init(&trngParams);

    if (params == NULL)
    {
        trngParams.returnBehavior = (TRNG_ReturnBehavior)RNG_defaultParams.returnBehavior;
    }
    else
    {
        trngParams.returnBehavior = (TRNG_ReturnBehavior)params->returnBehavior;
    }

    trngParams.randomBytesCallbackFxn = RNGCC26XX_trngCallbackFxn;

    object->trngConfig.object  = &object->trngObject;
    object->trngConfig.hwAttrs = &RNGCC26XX_trngHWAttrs;

    if (TRNG_construct(&object->trngConfig, &trngParams) != NULL)
    {
        /* If params are NULL, use defaults */
        if (params == NULL)
        {
            object->rngParams = RNG_defaultParams;
        }
        else
        {
            object->rngParams = *params;
        }
    }
    else
    {
        handle = NULL;
    }

    return (RNG_Handle)handle;
}

void RNG_close(RNG_Handle handle)
{
    RNGCC26XX_Object *object;

    if (handle != NULL)
    {
        object = (RNGCC26XX_Object *)handle->object;

        TRNG_close(&object->trngConfig);
    }
}

int_fast16_t RNG_getRandomBits(RNG_Handle handle, void *randomBits, size_t randomBitsLength)
{

    return RNGCC26XX_getValidatedNumber(handle,
                                        randomBits,
                                        randomBitsLength,
                                        NULL,
                                        CryptoUtils_ENDIANESS_LITTLE,
                                        NULL,
                                        NULL,
                                        NULL);
}

int_fast16_t RNG_getLERandomNumberInRange(RNG_Handle handle,
                                          const void *lowerLimit,
                                          const void *upperLimit,
                                          void *randomNumber,
                                          size_t randomNumberBitLength)
{

    return RNGCC26XX_getValidatedNumber(handle,
                                        randomNumber,
                                        randomNumberBitLength,
                                        NULL,
                                        CryptoUtils_ENDIANESS_LITTLE,
                                        &RNGCC26XX_checkRange,
                                        lowerLimit,
                                        upperLimit);
}

int_fast16_t RNG_getBERandomNumberInRange(RNG_Handle handle,
                                          const void *lowerLimit,
                                          const void *upperLimit,
                                          void *randomNumber,
                                          size_t randomNumberBitLength)
{

    return RNGCC26XX_getValidatedNumber(handle,
                                        randomNumber,
                                        randomNumberBitLength,
                                        NULL,
                                        CryptoUtils_ENDIANESS_BIG,
                                        &RNGCC26XX_checkRange,
                                        lowerLimit,
                                        upperLimit);
}

int_fast16_t RNG_generateKey(RNG_Handle handle, CryptoKey *key)
{
    int_fast16_t returnValue = RNG_STATUS_SUCCESS;
    uint8_t *randomBits;
    size_t randomBitsLength;

    if (key->encoding != CryptoKey_BLANK_PLAINTEXT)
    {
        returnValue = RNG_STATUS_INVALID_INPUTS;
    }

    if (key->u.plaintext.keyLength > (RNG_MAX_BIT_LENGTH >> 3u))
    {
        returnValue = RNG_STATUS_INVALID_INPUTS;
    }

    if (returnValue == RNG_STATUS_SUCCESS)
    {
        randomBits       = key->u.plaintext.keyMaterial;
        randomBitsLength = key->u.plaintext.keyLength << 3u; /* Bytes to bits */

        returnValue = RNGCC26XX_getValidatedNumber(handle,
                                                   randomBits,
                                                   randomBitsLength,
                                                   key,
                                                   CryptoUtils_ENDIANESS_LITTLE,
                                                   NULL,
                                                   NULL,
                                                   NULL);
    }

    return returnValue;
}

int_fast16_t RNG_generateLEKeyInRange(RNG_Handle handle,
                                      const void *lowerLimit,
                                      const void *upperLimit,
                                      CryptoKey *key,
                                      size_t randomNumberBitLength)
{
    int_fast16_t returnValue;
    uint8_t *randomBits;

    if (key->encoding != CryptoKey_BLANK_PLAINTEXT)
    {
        returnValue = RNG_STATUS_INVALID_INPUTS;
    }
    else
    {

        randomBits = key->u.plaintext.keyMaterial;

        returnValue = RNGCC26XX_getValidatedNumber(handle,
                                                   randomBits,
                                                   randomNumberBitLength,
                                                   key,
                                                   CryptoUtils_ENDIANESS_LITTLE,
                                                   &RNGCC26XX_checkRange,
                                                   lowerLimit,
                                                   upperLimit);
    }

    return returnValue;
}

int_fast16_t RNG_generateBEKeyInRange(RNG_Handle handle,
                                      const void *lowerLimit,
                                      const void *upperLimit,
                                      CryptoKey *key,
                                      size_t randomNumberBitLength)
{
    int_fast16_t returnValue;
    uint8_t *randomBits;

    if (key->encoding != CryptoKey_BLANK_PLAINTEXT)
    {
        returnValue = RNG_STATUS_INVALID_INPUTS;
    }
    else
    {
        randomBits = key->u.plaintext.keyMaterial;

        returnValue = RNGCC26XX_getValidatedNumber(handle,
                                                   randomBits,
                                                   randomNumberBitLength,
                                                   key,
                                                   CryptoUtils_ENDIANESS_BIG,
                                                   &RNGCC26XX_checkRange,
                                                   lowerLimit,
                                                   upperLimit);
    }

    return returnValue;
}

int_fast16_t RNG_fillPoolIfLessThan(size_t bytes)
{
    int_fast16_t returnValue = RNG_STATUS_SUCCESS;
    size_t bytesNeeded;
    TRNG_Params trngParams;
    TRNGCC26XX_Object trngObject;
    TRNG_Config trngConfig;
    TRNG_Handle trngHandle;

    TRNG_init();

    TRNG_Params_init(&trngParams);
    trngConfig.object  = &trngObject;
    trngConfig.hwAttrs = &RNGCC26XX_trngHWAttrs;
    trngObject.isOpen  = false;

    trngHandle = TRNG_construct(&trngConfig, &trngParams);

    if (trngHandle == NULL)
    {
        returnValue = RNG_STATUS_ERROR;
    }
    else if (SemaphoreP_pend(&RNG_instanceData.accessSemaphore, SemaphoreP_WAIT_FOREVER) != SemaphoreP_OK)
    {
        returnValue = RNG_STATUS_RESOURCE_UNAVAILABLE;
        TRNG_close(trngHandle);
    }
    else
    {
        if (RNG_instanceData.poolLevel < bytes)
        {
            bytesNeeded = RNG_poolByteSize - RNG_instanceData.poolLevel;

            if (bytesNeeded != 0)
            {
                returnValue = RNGCC26XX_generateEntropy(trngHandle,
                                                        &RNG_instancePool[RNG_instanceData.poolLevel],
                                                        bytesNeeded);

                if (returnValue == RNG_STATUS_SUCCESS)
                {
                    RNG_instanceData.poolLevel = RNG_poolByteSize;
                }
            }
        }

        SemaphoreP_post(&RNG_instanceData.accessSemaphore);

        TRNG_close(trngHandle);
    }

    return returnValue;
}

int_fast16_t RNG_cancelOperation(RNG_Handle handle)
{
    RNGCC26XX_Object *object;
    int_fast16_t returnValue = RNG_STATUS_INVALID_INPUTS;

    if (handle != NULL)
    {
        object      = (RNGCC26XX_Object *)handle->object;
        returnValue = TRNG_cancelOperation(&object->trngConfig);
    }

    return returnValue;
}
