/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
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

#include <ti/drivers/crypto/CryptoCC26X4_ns.h>
#include <ti/drivers/cryptoutils/ecc/ECCParams.h>
#include <ti/drivers/ECDSA.h>
#include <ti/drivers/ecdsa/ECDSACC26X4_ns.h>
#include <ti/drivers/ecdsa/ECDSACC26X4_s.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26X2.h>
#include <ti/drivers/TRNG.h>

#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include <psa/client.h>

/* Static globals */
static bool isInitialized = false;

static volatile bool ECDSACC26X4_ns_pollingDone;

static struct psa_invec invecs[1];
static struct psa_outvec outvecs[1];

/* Extern globals */
extern const ECDSA_Params ECDSA_defaultParams;
extern ECDSA_s_SecureCallback ecdsaSecureCB_ns[];
extern ECDSACC26X4_ns_Object ecdsaObject_ns[];

/*
 *  ======== ECDSA_ns_callbackFxn ========
 */
void ECDSA_ns_callbackFxn(uintptr_t arg)
{
    ECDSA_s_SecureCallback *secureCallbackObject = (ECDSA_s_SecureCallback *)arg;
    uintptr_t index                              = GET_CRYPTO_S_HANDLE_INDEX(secureCallbackObject->handle);

    (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

    CryptoPSACC26X4_releaseLock();

    if (ecdsaObject_ns[index].returnBehavior == ECDSA_RETURN_BEHAVIOR_POLLING)
    {
        ECDSACC26X4_ns_pollingDone = true;
    }
    else if (ecdsaObject_ns[index].returnBehavior == ECDSA_RETURN_BEHAVIOR_BLOCKING)
    {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&CryptoPSACC26X4_operationSemaphore);
    }
    else
    {
        /* Call the callback function provided by the application. */
        ecdsaObject_ns[index].callbackFxn(ecdsaSecureCB_ns[index].handle,
                                          ecdsaSecureCB_ns[index].returnStatus,
                                          ecdsaSecureCB_ns[index].operation,
                                          ecdsaSecureCB_ns[index].operationType);
    }
}

/*
 *  ======== ECDSA_ns_registerCallback ========
 */
static psa_status_t ECDSA_ns_registerCallback(ECDSA_Handle handle, const ECDSA_Params *params)
{
    ECDSA_s_CallbackMsg callbackMsg;
    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Construct SecureCB object */
    SecureCallback_construct(&ecdsaSecureCB_ns[index].object,
                             ECDSA_ns_callbackFxn,
                             (uintptr_t)&ecdsaSecureCB_ns[index]);

    callbackMsg.handle   = handle;
    callbackMsg.callback = &ecdsaSecureCB_ns[index];
    invecs[0].base       = &callbackMsg;
    invecs[0].len        = sizeof(callbackMsg);

    /* Setup interface for return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver - assumes CryptoPSACC26X4 lock is already acquired */
    return CryptoPSACC26X4_call(ECDSA_S_MSG_TYPE_REGISTER_CALLBACK, invecs, outvecs);
}

/*
 *  ======== ECDSA_ns_waitForResult ========
 */
static void ECDSA_ns_waitForResult(int_fast16_t *result, uint8_t objectIndex)
{
    if (*result != ECDSA_STATUS_SUCCESS)
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (ecdsaObject_ns[objectIndex].returnBehavior == ECDSA_RETURN_BEHAVIOR_POLLING)
    {
        /*
         * Emulate polling mode by spinning on a flag which will be set by
         * the callback function
         */
        while (!ECDSACC26X4_ns_pollingDone) {}
        *result = ecdsaSecureCB_ns[objectIndex].returnStatus;
    }
    else if (ecdsaObject_ns[objectIndex].returnBehavior == ECDSA_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        *result = ecdsaSecureCB_ns[objectIndex].returnStatus;
    }
}

/*
 *  ======== ECDSA_init ========
 */
void ECDSA_init(void)
{
    if (!isInitialized)
    {
        /* Initialize CryptoPSA semaphores and SecureCB driver */
        CryptoPSACC26X4_init();

        /* TRNG_init is required to power up the TRNG */
        TRNG_init();

        isInitialized = true;
    }
}

/*
 *  ======== ECDSA_open ========
 */
ECDSA_Handle ECDSA_open(uint_least8_t index, const ECDSA_Params *params)
{
    ECDSA_Handle handle = NULL;
    ECDSA_s_OpenMsg openMsg;
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
        params = (ECDSA_Params *)&ECDSA_defaultParams;
    }

    DebugP_assert(params->returnBehavior == ECDSA_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(ECDSA_S_MSG_TYPE_OPEN, invecs, outvecs);

    if (handle != NULL)
    {
        /* Register NS callback regardless of return behavior */
        if (ECDSA_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        ecdsaObject_ns[index].returnBehavior   = params->returnBehavior;
        ecdsaObject_ns[index].callbackFxn      = params->callbackFxn;
        ecdsaObject_ns[index].semaphoreTimeout = (params->returnBehavior == ECDSA_RETURN_BEHAVIOR_BLOCKING)
                                                     ? params->timeout
                                                     : SemaphoreP_NO_WAIT;

        /* Set power dependency - i.e. power up and enable clock for PKA */
        (void)Power_setDependency(PowerCC26X2_PERIPH_PKA);
    }

    return handle;
}

/*
 *  ======== ECDSA_construct ========
 */
ECDSA_Handle ECDSA_construct(ECDSA_Config *config, const ECDSA_Params *params)
{
    ECDSA_Handle handle = NULL;
    ECDSA_s_ConstructMsg constructMsg;
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
        params = (ECDSA_Params *)&ECDSA_defaultParams;
    }

    DebugP_assert(params->returnBehavior == ECDSA_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(ECDSA_S_MSG_TYPE_CONSTRUCT, invecs, outvecs);

    if (handle != NULL)
    {
        /* Register NS callback regardless of return behavior */
        if (ECDSA_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        ecdsaObject_ns[index].returnBehavior   = params->returnBehavior;
        ecdsaObject_ns[index].callbackFxn      = params->callbackFxn;
        ecdsaObject_ns[index].semaphoreTimeout = (params->returnBehavior == ECDSA_RETURN_BEHAVIOR_BLOCKING)
                                                     ? params->timeout
                                                     : SemaphoreP_NO_WAIT;

        /* Set power dependency - i.e. power up and enable clock for PKA */
        (void)Power_setDependency(PowerCC26X2_PERIPH_PKA);
    }

    return handle;
}

/*
 *  ======== ECDSA_close ========
 */
void ECDSA_close(ECDSA_Handle handle)
{
    ECDSA_s_CloseMsg closeMsg;

    DebugP_assert(handle);

    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Destruct SecureCallback object */
    SecureCallback_destruct(&ecdsaSecureCB_ns[index].object);

    /* Setup interface for input parameter */
    closeMsg.handle = handle;
    invecs[0].base  = &closeMsg;
    invecs[0].len   = sizeof(closeMsg);

    /* Setup interface for null return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver */
    (void)CryptoPSACC26X4_call(ECDSA_S_MSG_TYPE_CLOSE, invecs, outvecs);

    /* Release power dependency */
    (void)Power_releaseDependency(PowerCC26X2_PERIPH_PKA);
}

/*
 *  ======== ECDSA_sign ========
 */
int_fast16_t ECDSA_sign(ECDSA_Handle handle, ECDSA_OperationSign *operation)
{
    ECDSA_s_SignMsg signMsg;
    int_fast16_t result = ECDSA_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    DebugP_assert(params->returnBehavior == ECDSA_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

    /* Setup interface for input parameters */
    signMsg.handle    = handle;
    signMsg.operation = operation;
    invecs[0].base    = &signMsg;
    invecs[0].len     = sizeof(signMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(ecdsaObject_ns[index].semaphoreTimeout) == false)
    {
        return ECDSA_STATUS_RESOURCE_UNAVAILABLE;
    }

    ECDSACC26X4_ns_pollingDone = false;

    (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to ECDSA_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(ECDSA_S_MSG_TYPE_SIGN, invecs, outvecs);

    ECDSA_ns_waitForResult(&result, index);

    return (result);
}

/*
 *  ======== ECDSA_verify ========
 */
int_fast16_t ECDSA_verify(ECDSA_Handle handle, ECDSA_OperationVerify *operation)
{
    ECDSA_s_VerifyMsg verifyMsg;
    int_fast16_t result;
    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    DebugP_assert(params->returnBehavior == ECDSA_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

    /* Setup interface for input parameters */
    verifyMsg.handle    = handle;
    verifyMsg.operation = operation;
    invecs[0].base      = &verifyMsg;
    invecs[0].len       = sizeof(verifyMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(ecdsaObject_ns[index].semaphoreTimeout) == false)
    {
        return ECDSA_STATUS_RESOURCE_UNAVAILABLE;
    }

    ECDSACC26X4_ns_pollingDone = false;

    (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to ECDSA_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(ECDSA_S_MSG_TYPE_VERIFY, invecs, outvecs);

    ECDSA_ns_waitForResult(&result, index);

    return (result);
}

/*
 *  ======== ECDSA_cancelOperation ========
 */
int_fast16_t ECDSA_cancelOperation(ECDSA_Handle handle)
{
    ECDSA_s_CancelOperationMsg cancelMsg;
    int_fast16_t result = ECDSA_STATUS_ERROR;

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
     * Return value can be ignored since result (in outvecs) is initialized to ECDSA_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(ECDSA_S_MSG_TYPE_CANCEL_OPERATION, invecs, outvecs);

    return (result);
}