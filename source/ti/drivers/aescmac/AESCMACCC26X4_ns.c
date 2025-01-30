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

#include <ti/drivers/AESCMAC.h>
#include <ti/drivers/aescmac/AESCMACCC26X4_ns.h>
#include <ti/drivers/aescmac/AESCMACCC26X4_s.h>
#include <ti/drivers/crypto/CryptoCC26X4_ns.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include <psa/client.h>

/* Static globals */
static bool isInitialized = false;

static struct psa_invec invecs[1];
static struct psa_outvec outvecs[1];

extern AESCMAC_s_SecureCallback aescmacSecureCB_ns[];
extern AESCMACCC26X4_ns_Object aescmacObject_ns[];

/*
 *  ======== AESCMAC_ns_callbackFxn ========
 */
void AESCMAC_ns_callbackFxn(uintptr_t arg)
{
    AESCMAC_s_SecureCallback *secureCallbackObject = (AESCMAC_s_SecureCallback *)arg;
    uintptr_t index                                = GET_CRYPTO_S_HANDLE_INDEX(secureCallbackObject->handle);

    (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
    CryptoPSACC26X4_releaseLock();

    if (aescmacObject_ns[index].returnBehavior == AESCMAC_RETURN_BEHAVIOR_BLOCKING)
    {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&CryptoPSACC26X4_operationSemaphore);
    }
    else if (aescmacObject_ns[index].returnBehavior == AESCMAC_RETURN_BEHAVIOR_CALLBACK)
    {
        /* Call the callback function provided by the application. */
        aescmacObject_ns[index].callbackFxn(aescmacSecureCB_ns[index].handle,
                                            aescmacSecureCB_ns[index].returnValue,
                                            aescmacSecureCB_ns[index].operation,
                                            aescmacSecureCB_ns[index].operationType);
    }
}

/*
 *  ======== AESCMAC_ns_registerCallback ========
 */
static psa_status_t AESCMAC_ns_registerCallback(AESCMAC_Handle handle, const AESCMAC_Params *params)
{
    AESCMAC_s_CallbackMsg callbackMsg;
    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Construct SecureCallback object */
    SecureCallback_construct(&aescmacSecureCB_ns[index].object,
                             AESCMAC_ns_callbackFxn,
                             (uintptr_t)&aescmacSecureCB_ns[index]);

    callbackMsg.handle   = handle;
    callbackMsg.callback = &aescmacSecureCB_ns[index];
    invecs[0].base       = &callbackMsg;
    invecs[0].len        = sizeof(callbackMsg);

    /* Setup interface for return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver */
    return CryptoPSACC26X4_call(AESCMAC_S_MSG_TYPE_REGISTER_CALLBACK, invecs, outvecs);
}

/*
 *  ======== AESCMAC_init ========
 */
void AESCMAC_init(void)
{
    if (!isInitialized)
    {
        /* Initialize CryptoPSA semaphores and SecureCallback driver */
        CryptoPSACC26X4_init();

        isInitialized = true;
    }
}

/*
 *  ======== AESCMAC_open ========
 */
AESCMAC_Handle AESCMAC_open(uint_least8_t index, const AESCMAC_Params *params)
{
    AESCMAC_Handle handle = NULL;
    AESCMAC_s_OpenMsg openMsg;
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
        params = (AESCMAC_Params *)&AESCMAC_defaultParams;
    }

    DebugP_assert(params->returnBehavior == AESCMAC_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(AESCMAC_S_MSG_TYPE_OPEN, invecs, outvecs);

    if ((handle != NULL) && (params->returnBehavior != AESCMAC_RETURN_BEHAVIOR_POLLING))
    {
        if (AESCMAC_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        aescmacObject_ns[index].returnBehavior   = params->returnBehavior;
        aescmacObject_ns[index].callbackFxn      = params->callbackFxn;
        aescmacObject_ns[index].semaphoreTimeout = params->returnBehavior == AESCMAC_RETURN_BEHAVIOR_BLOCKING
                                                       ? params->timeout
                                                       : SemaphoreP_NO_WAIT;

        /* Set power dependency */
        (void)Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);
    }

    return handle;
}

/*
 *  ======== AESCMAC_construct ========
 */
AESCMAC_Handle AESCMAC_construct(AESCMAC_Config *config, const AESCMAC_Params *params)
{
    AESCMAC_Handle handle = NULL;
    AESCMAC_s_ConstructMsg constructMsg;
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
        params = (AESCMAC_Params *)&AESCMAC_defaultParams;
    }

    DebugP_assert(params->returnBehavior == AESCMAC_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(AESCMAC_S_MSG_TYPE_CONSTRUCT, invecs, outvecs);

    if ((handle != NULL) && (params->returnBehavior != AESCMAC_RETURN_BEHAVIOR_POLLING))
    {
        if (AESCMAC_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        aescmacObject_ns[index].returnBehavior   = params->returnBehavior;
        aescmacObject_ns[index].callbackFxn      = params->callbackFxn;
        aescmacObject_ns[index].semaphoreTimeout = params->returnBehavior == AESCMAC_RETURN_BEHAVIOR_BLOCKING
                                                       ? params->timeout
                                                       : SemaphoreP_NO_WAIT;

        /* Set power dependency */
        (void)Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);
    }

    return handle;
}

/*
 *  ======== AESCMAC_close ========
 */
void AESCMAC_close(AESCMAC_Handle handle)
{
    AESCMAC_s_CloseMsg closeMsg;

    DebugP_assert(handle);

    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Destruct SecureCallback object */
    SecureCallback_destruct(&aescmacSecureCB_ns[index].object);

    /* Setup interface for input parameter */
    closeMsg.handle = handle;
    invecs[0].base  = &closeMsg;
    invecs[0].len   = sizeof(closeMsg);

    /* Setup interface for null return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver */
    (void)CryptoPSACC26X4_call(AESCMAC_S_MSG_TYPE_CLOSE, invecs, outvecs);

    /* Release power dependency */
    (void)Power_releaseDependency(PowerCC26XX_PERIPH_CRYPTO);
}

/*
 *  ======== AESCMAC_setupSign ========
 */
int_fast16_t AESCMAC_setupSign(AESCMAC_Handle handle, const CryptoKey *key)
{
    AESCMAC_s_SetupOperationMsg setupMsg;
    int_fast16_t result = AESCMAC_STATUS_ERROR;

    DebugP_assert(handle);

    /* Setup interface for input parameters */
    setupMsg.handle = handle;
    setupMsg.key    = key;
    invecs[0].base  = &setupMsg;
    invecs[0].len   = sizeof(setupMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCMAC_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCMAC_S_MSG_TYPE_SETUP_SIGN, invecs, outvecs);

    return (result);
}

/*
 *  ======== AESCMAC_setupVerify ========
 */
int_fast16_t AESCMAC_setupVerify(AESCMAC_Handle handle, const CryptoKey *key)
{
    AESCMAC_s_SetupOperationMsg setupMsg;
    int_fast16_t result = AESCMAC_STATUS_ERROR;

    DebugP_assert(handle);

    /* Setup interface for input parameters */
    setupMsg.handle = handle;
    setupMsg.key    = key;
    invecs[0].base  = &setupMsg;
    invecs[0].len   = sizeof(setupMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCMAC_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCMAC_S_MSG_TYPE_SETUP_VERIFY, invecs, outvecs);

    return (result);
}

/*
 *  ======== AESCMAC_addData ========
 */
int_fast16_t AESCMAC_addData(AESCMAC_Handle handle, AESCMAC_Operation *operation)
{
    AESCMAC_s_AddDataMsg addDataMsg;
    int_fast16_t result = AESCMAC_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    DebugP_assert(handle);
    DebugP_assert(operation);

    /* Setup interface for input parameters */
    addDataMsg.handle    = handle;
    addDataMsg.operation = operation;
    invecs[0].base       = &addDataMsg;
    invecs[0].len        = sizeof(addDataMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(aescmacObject_ns[index].semaphoreTimeout) == false)
    {
        return AESCMAC_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aescmacObject_ns[index].returnBehavior != AESCMAC_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCMAC_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCMAC_S_MSG_TYPE_ADD_DATA, invecs, outvecs);

    if ((aescmacObject_ns[index].returnBehavior != AESCMAC_RETURN_BEHAVIOR_POLLING) &&
        (result != AESCMAC_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aescmacObject_ns[index].returnBehavior == AESCMAC_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aescmacObject_ns[index].returnBehavior == AESCMAC_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aescmacSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESCMAC_finalize ========
 */
int_fast16_t AESCMAC_finalize(AESCMAC_Handle handle, AESCMAC_Operation *operation)
{
    AESCMAC_s_FinalizeMsg finalizeMsg;
    int_fast16_t result = AESCMAC_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    DebugP_assert(handle);
    DebugP_assert(operation);

    /* Setup interface for input parameters */
    finalizeMsg.handle    = handle;
    finalizeMsg.operation = operation;
    invecs[0].base        = &finalizeMsg;
    invecs[0].len         = sizeof(finalizeMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(aescmacObject_ns[index].semaphoreTimeout) == false)
    {
        return AESCMAC_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aescmacObject_ns[index].returnBehavior != AESCMAC_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCMAC_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCMAC_S_MSG_TYPE_FINALIZE, invecs, outvecs);

    if ((aescmacObject_ns[index].returnBehavior != AESCMAC_RETURN_BEHAVIOR_POLLING) &&
        (result != AESCMAC_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aescmacObject_ns[index].returnBehavior == AESCMAC_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aescmacObject_ns[index].returnBehavior == AESCMAC_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aescmacSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESCMAC_oneStepSign ========
 */
int_fast16_t AESCMAC_oneStepSign(AESCMAC_Handle handle, AESCMAC_Operation *operation, CryptoKey *key)
{
    AESCMAC_s_OneStepOperationMsg oneStepMsg;
    int_fast16_t result = AESCMAC_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Setup interface for input parameters */
    oneStepMsg.handle    = handle;
    oneStepMsg.operation = operation;
    oneStepMsg.key       = key;
    invecs[0].base       = &oneStepMsg;
    invecs[0].len        = sizeof(oneStepMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(aescmacObject_ns[index].semaphoreTimeout) == false)
    {
        return AESCMAC_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aescmacObject_ns[index].returnBehavior != AESCMAC_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCMAC_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCMAC_S_MSG_TYPE_ONE_STEP_SIGN, invecs, outvecs);

    if ((aescmacObject_ns[index].returnBehavior != AESCMAC_RETURN_BEHAVIOR_POLLING) &&
        (result != AESCMAC_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aescmacObject_ns[index].returnBehavior == AESCMAC_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aescmacObject_ns[index].returnBehavior == AESCMAC_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aescmacSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESCMAC_oneStepVerify ========
 */
int_fast16_t AESCMAC_oneStepVerify(AESCMAC_Handle handle, AESCMAC_Operation *operation, CryptoKey *key)
{
    AESCMAC_s_OneStepOperationMsg oneStepMsg;
    int_fast16_t result = AESCMAC_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Setup interface for input parameters */
    oneStepMsg.handle    = handle;
    oneStepMsg.operation = operation;
    oneStepMsg.key       = key;
    invecs[0].base       = &oneStepMsg;
    invecs[0].len        = sizeof(oneStepMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(aescmacObject_ns[index].semaphoreTimeout) == false)
    {
        return AESCMAC_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aescmacObject_ns[index].returnBehavior != AESCMAC_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCMAC_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCMAC_S_MSG_TYPE_ONE_STEP_VERIFY, invecs, outvecs);

    if ((aescmacObject_ns[index].returnBehavior != AESCMAC_RETURN_BEHAVIOR_POLLING) &&
        (result != AESCMAC_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aescmacObject_ns[index].returnBehavior == AESCMAC_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aescmacObject_ns[index].returnBehavior == AESCMAC_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aescmacSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESCMAC_cancelOperation ========
 */
int_fast16_t AESCMAC_cancelOperation(AESCMAC_Handle handle)
{
    AESCMAC_s_CancelOperationMsg cancelMsg;
    int_fast16_t result = AESCMAC_STATUS_ERROR;

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
     * Return value can be ignored since result (in outvecs) is initialized to AESCMAC_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCMAC_S_MSG_TYPE_CANCEL_OPERATION, invecs, outvecs);

    return (result);
}
