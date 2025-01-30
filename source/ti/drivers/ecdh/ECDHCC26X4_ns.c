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
#include <ti/drivers/cryptoutils/ecc/ECCParams.h>
#include <ti/drivers/ECDH.h>
#include <ti/drivers/ecdh/ECDHCC26X4_ns.h>
#include <ti/drivers/ecdh/ECDHCC26X4_s.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26X2.h>

#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include <psa/client.h>

/* Static globals */
static bool isInitialized = false;

static volatile bool ECDHCC26X4_ns_pollingDone;

static struct psa_invec invecs[1];
static struct psa_outvec outvecs[1];

extern ECDH_s_SecureCallback ecdhSecureCB_ns[];
extern ECDHCC26X4_ns_Object ecdhObject_ns[];

/*
 *  ======== ECDH_ns_callbackFxn ========
 */
void ECDH_ns_callbackFxn(uintptr_t arg)
{
    ECDH_s_SecureCallback *secureCallbackObject = (ECDH_s_SecureCallback *)arg;
    uintptr_t index                             = GET_CRYPTO_S_HANDLE_INDEX(secureCallbackObject->handle);

    (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

    CryptoPSACC26X4_releaseLock();

    if (ecdhObject_ns[index].returnBehavior == ECDH_RETURN_BEHAVIOR_POLLING)
    {
        ECDHCC26X4_ns_pollingDone = true;
    }
    else if (ecdhObject_ns[index].returnBehavior == ECDH_RETURN_BEHAVIOR_BLOCKING)
    {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&CryptoPSACC26X4_operationSemaphore);
    }
    else
    {
        /* Call the callback function provided by the application. */
        ecdhObject_ns[index].callbackFxn(ecdhSecureCB_ns[index].handle,
                                         ecdhSecureCB_ns[index].returnStatus,
                                         ecdhSecureCB_ns[index].operation,
                                         ecdhSecureCB_ns[index].operationType);
    }
}

/*
 *  ======== ECDH_ns_registerCallback ========
 */
static psa_status_t ECDH_ns_registerCallback(ECDH_Handle handle, const ECDH_Params *params)
{
    ECDH_s_CallbackMsg callbackMsg;
    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Construct SecureCB object */
    SecureCallback_construct(&ecdhSecureCB_ns[index].object, ECDH_ns_callbackFxn, (uintptr_t)&ecdhSecureCB_ns[index]);

    callbackMsg.handle   = handle;
    callbackMsg.callback = &ecdhSecureCB_ns[index];
    invecs[0].base       = &callbackMsg;
    invecs[0].len        = sizeof(callbackMsg);

    /* Setup interface for return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver - assumes CryptoPSACC26X4 lock is already acquired */
    return CryptoPSACC26X4_call(ECDH_S_MSG_TYPE_REGISTER_CALLBACK, invecs, outvecs);
}

/*
 *  ======== ECDH_ns_waitForResult ========
 */
static void ECDH_ns_waitForResult(int_fast16_t *result, uint8_t objectIndex)
{
    if (*result != ECDH_STATUS_SUCCESS)
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (ecdhObject_ns[objectIndex].returnBehavior == ECDH_RETURN_BEHAVIOR_POLLING)
    {
        /*
         * Emulate polling mode by spinning on a flag which will be set by
         * the callback function
         */
        while (!ECDHCC26X4_ns_pollingDone) {}
        *result = ecdhSecureCB_ns[objectIndex].returnStatus;
    }
    else if (ecdhObject_ns[objectIndex].returnBehavior == ECDH_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        *result = ecdhSecureCB_ns[objectIndex].returnStatus;
    }
}

/*
 *  ======== ECDH_Params_init ========
 */
void ECDH_Params_init(ECDH_Params *params)
{
    *params = ECDH_defaultParams;
}

/*
 *  ======== ECDH_init ========
 */
void ECDH_init(void)
{
    if (!isInitialized)
    {
        /* Initialize CryptoPSA semaphores and SecureCB driver */
        CryptoPSACC26X4_init();

        isInitialized = true;
    }
}

/*
 *  ======== ECDH_open ========
 */
ECDH_Handle ECDH_open(uint_least8_t index, const ECDH_Params *params)
{
    ECDH_Handle handle = NULL;
    ECDH_s_OpenMsg openMsg;
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
        params = (ECDH_Params *)&ECDH_defaultParams;
    }

    DebugP_assert(params->returnBehavior == ECDH_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(ECDH_S_MSG_TYPE_OPEN, invecs, outvecs);

    if (handle != NULL)
    {
        /* Register NS callback regardless of return behavior */
        if (ECDH_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        ecdhObject_ns[index].returnBehavior   = params->returnBehavior;
        ecdhObject_ns[index].callbackFxn      = params->callbackFxn;
        ecdhObject_ns[index].semaphoreTimeout = (params->returnBehavior == ECDH_RETURN_BEHAVIOR_BLOCKING)
                                                    ? params->timeout
                                                    : SemaphoreP_NO_WAIT;

        /* Set power dependency - i.e. power up and enable clock for PKA */
        (void)Power_setDependency(PowerCC26X2_PERIPH_PKA);
    }

    return handle;
}

/*
 *  ======== ECDH_construct ========
 */
ECDH_Handle ECDH_construct(ECDH_Config *config, const ECDH_Params *params)
{
    ECDH_Handle handle = NULL;
    ECDH_s_ConstructMsg constructMsg;
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
        params = (ECDH_Params *)&ECDH_defaultParams;
    }

    DebugP_assert(params->returnBehavior == ECDH_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(ECDH_S_MSG_TYPE_CONSTRUCT, invecs, outvecs);

    if (handle != NULL)
    {
        /* Register NS callback regardless of return behavior */
        if (ECDH_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        ecdhObject_ns[index].returnBehavior   = params->returnBehavior;
        ecdhObject_ns[index].callbackFxn      = params->callbackFxn;
        ecdhObject_ns[index].semaphoreTimeout = (params->returnBehavior == ECDH_RETURN_BEHAVIOR_BLOCKING)
                                                    ? params->timeout
                                                    : SemaphoreP_NO_WAIT;

        /* Set power dependency - i.e. power up and enable clock for PKA */
        (void)Power_setDependency(PowerCC26X2_PERIPH_PKA);
    }

    return handle;
}

/*
 *  ======== ECDH_close ========
 */
void ECDH_close(ECDH_Handle handle)
{
    ECDH_s_CloseMsg closeMsg;

    DebugP_assert(handle);

    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Destruct SecureCallback object */
    SecureCallback_destruct(&ecdhSecureCB_ns[index].object);

    /* Setup interface for input parameter */
    closeMsg.handle = handle;
    invecs[0].base  = &closeMsg;
    invecs[0].len   = sizeof(closeMsg);

    /* Setup interface for null return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver */
    (void)CryptoPSACC26X4_call(ECDH_S_MSG_TYPE_CLOSE, invecs, outvecs);

    /* Release power dependency */
    (void)Power_releaseDependency(PowerCC26X2_PERIPH_PKA);
}

/*
 *  ======== ECDH_generatePublicKey ========
 */
int_fast16_t ECDH_generatePublicKey(ECDH_Handle handle, ECDH_OperationGeneratePublicKey *operation)
{
    ECDH_s_GeneratePublicKeyMsg generateMsg;
    int_fast16_t result = ECDH_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    DebugP_assert(handle);
    DebugP_assert(operation);

    /* Setup interface for input parameters */
    generateMsg.handle    = handle;
    generateMsg.operation = operation;
    invecs[0].base        = &generateMsg;
    invecs[0].len         = sizeof(generateMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(ecdhObject_ns[index].semaphoreTimeout) == false)
    {
        return ECDH_STATUS_RESOURCE_UNAVAILABLE;
    }

    ECDHCC26X4_ns_pollingDone = false;

    (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to ECDH_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(ECDH_S_MSG_TYPE_GENERATE_PUBLIC_KEY, invecs, outvecs);

    ECDH_ns_waitForResult(&result, index);

    return (result);
}

/*
 *  ======== ECDH_computeSharedSecret ========
 */
int_fast16_t ECDH_computeSharedSecret(ECDH_Handle handle, ECDH_OperationComputeSharedSecret *operation)
{
    ECDH_s_ComputeSharedSecretMsg computeMsg;
    int_fast16_t result = ECDH_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    DebugP_assert(handle);
    DebugP_assert(operation);

    /* Setup interface for input parameters */
    computeMsg.handle    = handle;
    computeMsg.operation = operation;
    invecs[0].base       = &computeMsg;
    invecs[0].len        = sizeof(computeMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(ecdhObject_ns[index].semaphoreTimeout) == false)
    {
        return ECDH_STATUS_RESOURCE_UNAVAILABLE;
    }

    ECDHCC26X4_ns_pollingDone = false;

    (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to ECDH_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(ECDH_S_MSG_TYPE_COMPUTE_SHARED_SECRET, invecs, outvecs);

    ECDH_ns_waitForResult(&result, index);

    return (result);
}

/*
 *  ======== ECDH_cancelOperation ========
 */
int_fast16_t ECDH_cancelOperation(ECDH_Handle handle)
{
    ECDH_s_CancelOperationMsg cancelMsg;
    int_fast16_t result = ECDH_STATUS_ERROR;

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
     * Return value can be ignored since result (in outvecs) is initialized to ECDH_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(ECDH_S_MSG_TYPE_CANCEL_OPERATION, invecs, outvecs);

    return (result);
}
