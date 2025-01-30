/*
 * Copyright (c) 2022-2023, Texas Instruments Incorporated
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

#include <ti/drivers/crypto/CryptoCC26X4_ns.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/TRNG.h>
#include <ti/drivers/trng/TRNGCC26X4_ns.h>
#include <ti/drivers/trng/TRNGCC26X4_s.h>

#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include <psa/client.h>

/* Static globals */
static bool isInitialized = false;

static struct psa_invec invecs[1];
static struct psa_outvec outvecs[1];

extern TRNG_ns_SecureCB trngSecureCB_ns[];
extern TRNGCC26X4_ns_Object trngObject_ns[];

/*
 *  ======== TRNG_ns_cryptoKeyCallbackFxn ========
 */
void TRNG_ns_cryptoKeyCallbackFxn(uintptr_t arg)
{
    TRNG_s_CryptoKeySecureCallback *secureCallbackObject = (TRNG_s_CryptoKeySecureCallback *)arg;
    uintptr_t index                                      = GET_CRYPTO_S_HANDLE_INDEX(secureCallbackObject->handle);

    (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

    if (trngObject_ns[index].returnBehavior == TRNG_RETURN_BEHAVIOR_BLOCKING)
    {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&trngObject_ns[index].operationSemaphore);
    }
    else if (trngObject_ns[index].returnBehavior == TRNG_RETURN_BEHAVIOR_CALLBACK)
    {
        /* Call the callback function provided by the application. */
        trngObject_ns[index].cryptoKeyCallbackFxn(trngSecureCB_ns[index].cryptoKeyCallback.handle,
                                                  trngSecureCB_ns[index].cryptoKeyCallback.returnValue,
                                                  trngSecureCB_ns[index].cryptoKeyCallback.entropy);
    }
}

/*
 *  ======== TRNG_ns_randomBytesCallbackFxn ========
 */
void TRNG_ns_randomBytesCallbackFxn(uintptr_t arg)
{
    TRNG_s_RandomBytesSecureCallback *secureCallbackObject = (TRNG_s_RandomBytesSecureCallback *)arg;
    uintptr_t index                                        = GET_CRYPTO_S_HANDLE_INDEX(secureCallbackObject->handle);

    (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

    if (trngObject_ns[index].returnBehavior == TRNG_RETURN_BEHAVIOR_BLOCKING)
    {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&trngObject_ns[index].operationSemaphore);
    }
    else if (trngObject_ns[index].returnBehavior == TRNG_RETURN_BEHAVIOR_CALLBACK)
    {
        /* Call the callback function provided by the application. */
        trngObject_ns[index].randomBytesCallbackFxn(trngSecureCB_ns[index].randomBytesCallback.handle,
                                                    trngSecureCB_ns[index].randomBytesCallback.returnValue,
                                                    trngSecureCB_ns[index].randomBytesCallback.randomBytes,
                                                    trngSecureCB_ns[index].randomBytesCallback.randomBytesSize);
    }
}

/*
 *  ======== TRNG_ns_registerCallback ========
 */
static psa_status_t TRNG_ns_registerCallback(TRNG_Handle handle, const TRNG_Params *params)
{
    TRNG_s_CallbackMsg callbackMsg;
    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Construct SecureCB objects */
    SecureCallback_construct(&trngSecureCB_ns[index].cryptoKeyCallback.object,
                             TRNG_ns_cryptoKeyCallbackFxn,
                             (uintptr_t)&trngSecureCB_ns[index].cryptoKeyCallback);

    SecureCallback_construct(&trngSecureCB_ns[index].randomBytesCallback.object,
                             TRNG_ns_randomBytesCallbackFxn,
                             (uintptr_t)&trngSecureCB_ns[index].randomBytesCallback);

    callbackMsg.handle              = handle;
    callbackMsg.cryptoKeyCallback   = &trngSecureCB_ns[index].cryptoKeyCallback;
    callbackMsg.randomBytesCallback = &trngSecureCB_ns[index].randomBytesCallback;
    invecs[0].base                  = &callbackMsg;
    invecs[0].len                   = sizeof(callbackMsg);

    /* Setup interface for return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver */
    return CryptoPSACC26X4_call(TRNG_S_MSG_TYPE_REGISTER_CALLBACKS, invecs, outvecs);
}

/*
 *  ======== TRNG_init ========
 */
void TRNG_init(void)
{
    uintptr_t key;

    if (!isInitialized)
    {
        /* Initialize CryptoPSA semaphores and SecureCB driver */
        CryptoPSACC26X4_init();

        /* Enable power and clocks for TRNG here since the entropy pool starts
         * filling as soon as TRNG initialization is done before the TRNG driver
         * instance is opened or constructed.
         */
        (void)Power_setDependency(PowerCC26XX_PERIPH_TRNG);

        key = HwiP_disable();

        /* Call fast veneer */
        TRNG_s_init();

        HwiP_restore(key);

        isInitialized = true;
    }
}

/*
 *  ======== TRNG_open ========
 */
TRNG_Handle TRNG_open(uint_least8_t index, TRNG_Params *params)
{
    TRNG_Handle handle = NULL;
    TRNG_s_OpenMsg openMsg;
    uintptr_t key;

    key = HwiP_disable();

    if (!isInitialized)
    {
        HwiP_restore(key);
        return NULL;
    }

    HwiP_restore(key);

    /*  If params are NULL, use defaults. Secure driver can also set this, but
     *  NS driver needs to store the return behavior.
     */
    if (params == NULL)
    {
        params = (TRNG_Params *)&TRNG_defaultParams;
    }

    DebugP_assert(params->returnBehavior == TRNG_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

    /* Setup interface for input parameters */
    openMsg.index  = index;
    openMsg.params = params;
    invecs[0].base = &openMsg;
    invecs[0].len  = sizeof(openMsg);

    /* Setup interface for return value */
    outvecs[0].base = &handle;
    outvecs[0].len  = sizeof(handle);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since handle (in outvecs) is initialized to NULL
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(TRNG_S_MSG_TYPE_OPEN, invecs, outvecs);

    if ((handle != NULL) && (params->returnBehavior != TRNG_RETURN_BEHAVIOR_POLLING))
    {
        if (TRNG_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        trngObject_ns[index].returnBehavior         = params->returnBehavior;
        trngObject_ns[index].cryptoKeyCallbackFxn   = params->cryptoKeyCallbackFxn;
        trngObject_ns[index].randomBytesCallbackFxn = params->randomBytesCallbackFxn;

        /*
         * params->timeout is not stored since infinite timeout must be used
         * for blocking return behavior
         */

        SemaphoreP_constructBinary(&trngObject_ns[index].operationSemaphore, 0);
    }

    return handle;
}

/*
 *  ======== TRNG_construct ========
 */
TRNG_Handle TRNG_construct(TRNG_Config *config, const TRNG_Params *params)
{
    TRNG_Handle handle = NULL;
    TRNG_s_ConstructMsg constructMsg;
    uintptr_t key;

    key = HwiP_disable();

    if (!isInitialized)
    {
        HwiP_restore(key);
        return NULL;
    }

    HwiP_restore(key);

    /*
     * If params are NULL, use defaults. Secure driver can also set this, but
     * NS driver needs to store the return behavior.
     */
    if (params == NULL)
    {
        params = (TRNG_Params *)&TRNG_defaultParams;
    }

    DebugP_assert(params->returnBehavior == TRNG_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

    /* Setup interface for input parameters */
    constructMsg.config = config;
    constructMsg.params = params;
    invecs[0].base      = &constructMsg;
    invecs[0].len       = sizeof(constructMsg);

    /* Setup interface for return value */
    outvecs[0].base = &handle;
    outvecs[0].len  = sizeof(handle);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since handle (in outvecs) is initialized to NULL
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(TRNG_S_MSG_TYPE_CONSTRUCT, invecs, outvecs);

    if ((handle != NULL) && (params->returnBehavior != TRNG_RETURN_BEHAVIOR_POLLING))
    {
        if (TRNG_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        trngObject_ns[index].returnBehavior         = params->returnBehavior;
        trngObject_ns[index].cryptoKeyCallbackFxn   = params->cryptoKeyCallbackFxn;
        trngObject_ns[index].randomBytesCallbackFxn = params->randomBytesCallbackFxn;
        /*
         * params->timeout is not stored since infinite timeout must be used
         * for blocking return behavior
         */

        SemaphoreP_constructBinary(&trngObject_ns[index].operationSemaphore, 0);
    }

    return handle;
}

/*
 *  ======== TRNG_close ========
 */
void TRNG_close(TRNG_Handle handle)
{
    TRNG_s_CloseMsg closeMsg;
    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    DebugP_assert(handle);

    /* Setup interface for input parameter */
    closeMsg.handle = handle;
    invecs[0].base  = &closeMsg;
    invecs[0].len   = sizeof(closeMsg);

    /* Setup interface for null return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver */
    (void)CryptoPSACC26X4_call(TRNG_S_MSG_TYPE_CLOSE, invecs, outvecs);

    SemaphoreP_destruct(&trngObject_ns[index].operationSemaphore);
}

/*
 *  ======== TRNG_generateEntropy ========
 */
int_fast16_t TRNG_generateEntropy(TRNG_Handle handle, CryptoKey *entropy)
{
    return TRNG_generateKey(handle, entropy);
}

/*
 *  ======== TRNG_generateKey ========
 */
int_fast16_t TRNG_generateKey(TRNG_Handle handle, CryptoKey *entropy)
{
    TRNG_s_GenerateKeyMsg generateKeyMsg;
    int_fast16_t result = TRNG_STATUS_ERROR;
    psa_status_t psaStatus;
    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Setup interface for input parameters */
    generateKeyMsg.handle  = handle;
    generateKeyMsg.entropy = entropy;
    invecs[0].base         = &generateKeyMsg;
    invecs[0].len          = sizeof(generateKeyMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /*
     * The underlying TRNG driver supports queuing of blocking or callback
     * operations and preemption by a polling operations. Therefore, no HW
     * access semaphore acquisition required here.
     */

    if (trngObject_ns[index].returnBehavior != TRNG_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /* PSA call to secure driver */
    psaStatus = CryptoPSACC26X4_call(TRNG_S_MSG_TYPE_GENERATE_KEY, invecs, outvecs);

    if (psaStatus == PSA_ERROR_PROGRAMMER_ERROR)
    {
        result = TRNG_STATUS_INVALID_INPUTS;
    }
    else if (psaStatus != PSA_SUCCESS)
    {
        result = TRNG_STATUS_ERROR;
    }

    if (result != TRNG_STATUS_SUCCESS)
    {
        if (trngObject_ns[index].returnBehavior != TRNG_RETURN_BEHAVIOR_POLLING)
        {
            /* Release power constraint if not successful */
            (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        }
    }
    else if (trngObject_ns[index].returnBehavior == TRNG_RETURN_BEHAVIOR_BLOCKING)
    {
        /*
         * The app-specified timeout cannot be used here since the secure
         * partition treats blocking return behavior in the same manner as
         * callback.
         */
        SemaphoreP_pend(&trngObject_ns[index].operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = trngSecureCB_ns[index].cryptoKeyCallback.returnValue;
    }

    return (result);
}

/*
 *  ======== TRNG_getRandomBytes ========
 */
int_fast16_t TRNG_getRandomBytes(TRNG_Handle handle, void *randomBytes, size_t randomBytesSize)
{
    TRNG_s_GetRandomBytesMsg getRandomMsg;
    int_fast16_t result = TRNG_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);
    psa_status_t psaStatus;

    /* Setup interface for input parameters */
    getRandomMsg.handle          = handle;
    getRandomMsg.randomBytes     = randomBytes;
    getRandomMsg.randomBytesSize = randomBytesSize;
    invecs[0].base               = &getRandomMsg;
    invecs[0].len                = sizeof(getRandomMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /*
     * The underlying TRNG driver supports queuing of blocking or callback
     * operations and preemption by a polling operations. Therefore, no HW
     * access semaphore acquisition required here.
     */

    if (trngObject_ns[index].returnBehavior != TRNG_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /* PSA call to secure driver */
    psaStatus = CryptoPSACC26X4_call(TRNG_S_MSG_TYPE_GET_RANDOM_BYTES, invecs, outvecs);

    if (psaStatus == PSA_ERROR_PROGRAMMER_ERROR)
    {
        result = TRNG_STATUS_INVALID_INPUTS;
    }
    else if (psaStatus != PSA_SUCCESS)
    {
        result = TRNG_STATUS_ERROR;
    }

    if (result != TRNG_STATUS_SUCCESS)
    {
        if (trngObject_ns[index].returnBehavior != TRNG_RETURN_BEHAVIOR_POLLING)
        {
            /* Release power constraint if not successful */
            (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        }
    }
    else if (trngObject_ns[index].returnBehavior == TRNG_RETURN_BEHAVIOR_BLOCKING)
    {
        /*
         * The app-specified timeout cannot be used here since the secure
         * partition treats blocking return behavior in the same manner as
         * callback.
         */
        SemaphoreP_pend(&trngObject_ns[index].operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = trngSecureCB_ns[index].randomBytesCallback.returnValue;
    }

    return (result);
}

/*
 *  ======== TRNG_cancelOperation ========
 */
int_fast16_t TRNG_cancelOperation(TRNG_Handle handle)
{
    TRNG_s_CancelOperationMsg cancelMsg;
    int_fast16_t result = TRNG_STATUS_ERROR;

    /* Setup interface for input parameters */
    cancelMsg.handle = handle;
    invecs[0].base   = &cancelMsg;
    invecs[0].len    = sizeof(cancelMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to TRNG_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(TRNG_S_MSG_TYPE_CANCEL_OPERATION, invecs, outvecs);

    return (result);
}

/*
 *  ======== TRNGCC26XX_setSamplesPerCycle ========
 * samplesPerCycle must be between 2^8 and 2^24 (256 and 16777216)
 */
int_fast16_t TRNGCC26XX_setSamplesPerCycle(TRNG_Handle handle, uint32_t samplesPerCycle)
{
    int_fast16_t result;
    uintptr_t key;

    key    = HwiP_disable();
    /* Call fast veneer */
    result = TRNGCC26XX_s_setSamplesPerCycle(handle, samplesPerCycle);
    HwiP_restore(key);

    return (result);
}
