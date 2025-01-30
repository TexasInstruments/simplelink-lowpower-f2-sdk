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

#include <ti/drivers/AESECB.h>
#include <ti/drivers/aesecb/AESECBCC26X4_ns.h>
#include <ti/drivers/aesecb/AESECBCC26X4_s.h>
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

extern AESECB_s_SecureCallback aesecbSecureCB_ns[];
extern AESECBCC26X4_ns_Object aesecbObject_ns[];

/*
 *  ======== AESECB_ns_callbackFxn ========
 */
void AESECB_ns_callbackFxn(uintptr_t arg)
{
    AESECB_s_SecureCallback *secureCallbackObject = (AESECB_s_SecureCallback *)arg;
    uintptr_t index                               = GET_CRYPTO_S_HANDLE_INDEX(secureCallbackObject->handle);

    (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
    CryptoPSACC26X4_releaseLock();

    if (aesecbObject_ns[index].returnBehavior == AESECB_RETURN_BEHAVIOR_BLOCKING)
    {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&CryptoPSACC26X4_operationSemaphore);
    }
    else if (aesecbObject_ns[index].returnBehavior == AESECB_RETURN_BEHAVIOR_CALLBACK)
    {
        /* Call the callback function provided by the application. */
        aesecbObject_ns[index].callbackFxn(aesecbSecureCB_ns[index].handle,
                                           aesecbSecureCB_ns[index].returnValue,
                                           aesecbSecureCB_ns[index].operation,
                                           aesecbSecureCB_ns[index].operationType);
    }
}

/*
 *  ======== AESECB_ns_registerCallback ========
 */
static psa_status_t AESECB_ns_registerCallback(AESECB_Handle handle, const AESECB_Params *params)
{
    AESECB_s_CallbackMsg callbackMsg;
    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Construct SecureCB object */
    SecureCallback_construct(&aesecbSecureCB_ns[index].object,
                             AESECB_ns_callbackFxn,
                             (uintptr_t)&aesecbSecureCB_ns[index]);

    callbackMsg.handle   = handle;
    callbackMsg.callback = &aesecbSecureCB_ns[index];
    invecs[0].base       = &callbackMsg;
    invecs[0].len        = sizeof(callbackMsg);

    /* Setup interface for return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver */
    return CryptoPSACC26X4_call(AESECB_S_MSG_TYPE_REGISTER_CALLBACK, invecs, outvecs);
}

/*
 *  ======== AESECB_init ========
 */
void AESECB_init(void)
{
    if (!isInitialized)
    {
        /* Initialize CryptoPSA semaphores and SecureCB driver */
        CryptoPSACC26X4_init();

        isInitialized = true;
    }
}

/*
 *  ======== AESECB_open ========
 */
AESECB_Handle AESECB_open(uint_least8_t index, const AESECB_Params *params)
{
    AESECB_Handle handle = NULL;
    AESECB_s_OpenMsg openMsg;
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
        params = (AESECB_Params *)&AESECB_defaultParams;
    }

    DebugP_assert(params->returnBehavior == AESECB_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(AESECB_S_MSG_TYPE_OPEN, invecs, outvecs);

    if ((handle != NULL) && (params->returnBehavior != AESECB_RETURN_BEHAVIOR_POLLING))
    {
        if (AESECB_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        aesecbObject_ns[index].returnBehavior   = params->returnBehavior;
        aesecbObject_ns[index].callbackFxn      = params->callbackFxn;
        aesecbObject_ns[index].semaphoreTimeout = params->returnBehavior == AESECB_RETURN_BEHAVIOR_BLOCKING
                                                      ? params->timeout
                                                      : SemaphoreP_NO_WAIT;

        /* Set power dependency */
        (void)Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);
    }

    return handle;
}

/*
 *  ======== AESECB_construct ========
 */
AESECB_Handle AESECB_construct(AESECB_Config *config, const AESECB_Params *params)
{
    AESECB_Handle handle = NULL;
    AESECB_s_ConstructMsg constructMsg;
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
        params = (AESECB_Params *)&AESECB_defaultParams;
    }

    DebugP_assert(params->returnBehavior == AESECB_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(AESECB_S_MSG_TYPE_CONSTRUCT, invecs, outvecs);

    if ((handle != NULL) && (params->returnBehavior != AESECB_RETURN_BEHAVIOR_POLLING))
    {
        if (AESECB_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        aesecbObject_ns[index].returnBehavior   = params->returnBehavior;
        aesecbObject_ns[index].callbackFxn      = params->callbackFxn;
        aesecbObject_ns[index].semaphoreTimeout = params->returnBehavior == AESECB_RETURN_BEHAVIOR_BLOCKING
                                                      ? params->timeout
                                                      : SemaphoreP_NO_WAIT;

        /* Set power dependency */
        (void)Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);
    }

    return handle;
}

/*
 *  ======== AESECB_close ========
 */
void AESECB_close(AESECB_Handle handle)
{
    AESECB_s_CloseMsg closeMsg;

    DebugP_assert(handle);

    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Destruct SecureCallback object */
    SecureCallback_destruct(&aesecbSecureCB_ns[index].object);

    /* Setup interface for input parameter */
    closeMsg.handle = handle;
    invecs[0].base  = &closeMsg;
    invecs[0].len   = sizeof(closeMsg);

    /* Setup interface for null return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver */
    (void)CryptoPSACC26X4_call(AESECB_S_MSG_TYPE_CLOSE, invecs, outvecs);

    /* Release power dependency */
    (void)Power_releaseDependency(PowerCC26XX_PERIPH_CRYPTO);
}

/*
 *  ======== AESECB_oneStepOperation ========
 */
static int_fast16_t AESECB_oneStepOperation(AESECB_Handle handle, AESECB_Operation *operation, int32_t type)
{
    AESECB_s_OneStepOperationMsg oneStepMsg;
    int_fast16_t result = AESECB_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Setup interface for input parameters */
    oneStepMsg.handle    = handle;
    oneStepMsg.operation = operation;
    invecs[0].base       = &oneStepMsg;
    invecs[0].len        = sizeof(oneStepMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(aesecbObject_ns[index].semaphoreTimeout) == false)
    {
        return AESECB_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aesecbObject_ns[index].returnBehavior != AESECB_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESECB_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(type, invecs, outvecs);

    if ((aesecbObject_ns[index].returnBehavior != AESECB_RETURN_BEHAVIOR_POLLING) && (result != AESECB_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesecbObject_ns[index].returnBehavior == AESECB_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesecbObject_ns[index].returnBehavior == AESECB_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aesecbSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESECB_oneStepEncrypt ========
 */
int_fast16_t AESECB_oneStepEncrypt(AESECB_Handle handle, AESECB_Operation *operation)
{
    return AESECB_oneStepOperation(handle, operation, AESECB_S_MSG_TYPE_ONE_STEP_ENCRYPT);
}

/*
 *  ======== AESECB_oneStepDecrypt ========
 */
int_fast16_t AESECB_oneStepDecrypt(AESECB_Handle handle, AESECB_Operation *operation)
{
    return AESECB_oneStepOperation(handle, operation, AESECB_S_MSG_TYPE_ONE_STEP_DECRYPT);
}

/*
 *  ======== AESECB_setupOperation ========
 */
static int_fast16_t AESECB_setupOperation(AESECB_Handle handle, const CryptoKey *key, int32_t type)
{
    AESECB_s_SetupOperationMsg setupMsg;
    int_fast16_t result = AESECB_STATUS_ERROR;

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
     * Return value can be ignored since result (in outvecs) is initialized to AESECB_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(type, invecs, outvecs);

    return (result);
}

/*
 *  ======== AESECB_setupEncrypt ========
 */
int_fast16_t AESECB_setupEncrypt(AESECB_Handle handle, const CryptoKey *key)
{
    return AESECB_setupOperation(handle, key, AESECB_S_MSG_TYPE_SETUP_ENCRYPT);
}

/*
 *  ======== AESECB_setupDecrypt ========
 */
int_fast16_t AESECB_setupDecrypt(AESECB_Handle handle, const CryptoKey *key)
{
    return AESECB_setupOperation(handle, key, AESECB_S_MSG_TYPE_SETUP_DECRYPT);
}

/*
 *  ======== AESECB_addData ========
 */
int_fast16_t AESECB_addData(AESECB_Handle handle, AESECB_Operation *operation)
{
    AESECB_s_AddDataMsg addDataMsg;
    int_fast16_t result = AESECB_STATUS_ERROR;
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
    if (CryptoPSACC26X4_acquireLock(aesecbObject_ns[index].semaphoreTimeout) == false)
    {
        return AESECB_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aesecbObject_ns[index].returnBehavior != AESECB_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESECB_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESECB_S_MSG_TYPE_ADD_DATA, invecs, outvecs);

    if ((aesecbObject_ns[index].returnBehavior != AESECB_RETURN_BEHAVIOR_POLLING) && (result != AESECB_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesecbObject_ns[index].returnBehavior == AESECB_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesecbObject_ns[index].returnBehavior == AESECB_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aesecbSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESECB_finalize ========
 */
int_fast16_t AESECB_finalize(AESECB_Handle handle, AESECB_Operation *operation)
{
    AESECB_s_FinalizeMsg finalizeMsg;
    int_fast16_t result = AESECB_STATUS_ERROR;
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
    if (CryptoPSACC26X4_acquireLock(aesecbObject_ns[index].semaphoreTimeout) == false)
    {
        return AESECB_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aesecbObject_ns[index].returnBehavior != AESECB_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESECB_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESECB_S_MSG_TYPE_FINALIZE, invecs, outvecs);

    if ((aesecbObject_ns[index].returnBehavior != AESECB_RETURN_BEHAVIOR_POLLING) && (result != AESECB_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesecbObject_ns[index].returnBehavior == AESECB_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesecbObject_ns[index].returnBehavior == AESECB_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aesecbSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESECB_cancelOperation ========
 */
int_fast16_t AESECB_cancelOperation(AESECB_Handle handle)
{
    AESECB_s_CancelOperationMsg cancelMsg;
    int_fast16_t result = AESECB_STATUS_ERROR;

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
     * Return value can be ignored since result (in outvecs) is initialized to AESECB_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESECB_S_MSG_TYPE_CANCEL_OPERATION, invecs, outvecs);

    return (result);
}
