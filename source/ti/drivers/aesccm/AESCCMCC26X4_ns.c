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

#include <ti/drivers/AESCCM.h>
#include <ti/drivers/aesccm/AESCCMCC26X4_ns.h>
#include <ti/drivers/aesccm/AESCCMCC26X4_s.h>
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

extern AESCCM_s_SecureCallback aesccmSecureCB_ns[];
extern AESCCMCC26X4_ns_Object aesccmObject_ns[];

/*
 *  ======== AESCCM_ns_callbackFxn ========
 */
void AESCCM_ns_callbackFxn(uintptr_t arg)
{
    AESCCM_s_SecureCallback *secureCallbackObject = (AESCCM_s_SecureCallback *)arg;
    uintptr_t index                               = GET_CRYPTO_S_HANDLE_INDEX(secureCallbackObject->handle);

    (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
    CryptoPSACC26X4_releaseLock();

    if (aesccmObject_ns[index].returnBehavior == AESCCM_RETURN_BEHAVIOR_BLOCKING)
    {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&CryptoPSACC26X4_operationSemaphore);
    }
    else if (aesccmObject_ns[index].returnBehavior == AESCCM_RETURN_BEHAVIOR_CALLBACK)
    {
        /* Call the callback function provided by the application. */
        aesccmObject_ns[index].callbackFxn(aesccmSecureCB_ns[index].handle,
                                           aesccmSecureCB_ns[index].returnValue,
                                           aesccmSecureCB_ns[index].operation,
                                           aesccmSecureCB_ns[index].operationType);
    }
}

/*
 *  ======== AESCCM_ns_registerCallback ========
 */
static psa_status_t AESCCM_ns_registerCallback(AESCCM_Handle handle, const AESCCM_Params *params)
{
    AESCCM_s_CallbackMsg callbackMsg;
    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Construct SecureCallback object */
    SecureCallback_construct(&aesccmSecureCB_ns[index].object,
                             AESCCM_ns_callbackFxn,
                             (uintptr_t)&aesccmSecureCB_ns[index]);

    callbackMsg.handle   = handle;
    callbackMsg.callback = &aesccmSecureCB_ns[index];
    invecs[0].base       = &callbackMsg;
    invecs[0].len        = sizeof(callbackMsg);

    /* Setup interface for return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver */
    return CryptoPSACC26X4_call(AESCCM_S_MSG_TYPE_REGISTER_CALLBACK, invecs, outvecs);
}

/*
 *  ======== AESCCM_init ========
 */
void AESCCM_init(void)
{
    if (!isInitialized)
    {
        /* Initialize CryptoPSA semaphores and SecureCallback driver */
        CryptoPSACC26X4_init();

        isInitialized = true;
    }
}

/*
 *  ======== AESCCM_open ========
 */
AESCCM_Handle AESCCM_open(uint_least8_t index, const AESCCM_Params *params)
{
    AESCCM_Handle handle = NULL;
    AESCCM_s_OpenMsg openMsg;
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
        params = (AESCCM_Params *)&AESCCM_defaultParams;
    }

    DebugP_assert(params->returnBehavior == AESCCM_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(AESCCM_S_MSG_TYPE_OPEN, invecs, outvecs);

    if ((handle != NULL) && (params->returnBehavior != AESCCM_RETURN_BEHAVIOR_POLLING))
    {
        if (AESCCM_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        aesccmObject_ns[index].returnBehavior   = params->returnBehavior;
        aesccmObject_ns[index].callbackFxn      = params->callbackFxn;
        aesccmObject_ns[index].semaphoreTimeout = params->returnBehavior == AESCCM_RETURN_BEHAVIOR_BLOCKING
                                                      ? params->timeout
                                                      : SemaphoreP_NO_WAIT;
        /* Set power dependency */
        (void)Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);
    }

    return handle;
}

/*
 *  ======== AESCCM_construct ========
 */
AESCCM_Handle AESCCM_construct(AESCCM_Config *config, const AESCCM_Params *params)
{
    AESCCM_Handle handle = NULL;
    AESCCM_s_ConstructMsg constructMsg;
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
        params = (AESCCM_Params *)&AESCCM_defaultParams;
    }

    DebugP_assert(params->returnBehavior == AESCCM_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(AESCCM_S_MSG_TYPE_CONSTRUCT, invecs, outvecs);

    if ((handle != NULL) && (params->returnBehavior != AESCCM_RETURN_BEHAVIOR_POLLING))
    {
        if (AESCCM_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        aesccmObject_ns[index].returnBehavior   = params->returnBehavior;
        aesccmObject_ns[index].callbackFxn      = params->callbackFxn;
        aesccmObject_ns[index].semaphoreTimeout = params->returnBehavior == AESCCM_RETURN_BEHAVIOR_BLOCKING
                                                      ? params->timeout
                                                      : SemaphoreP_NO_WAIT;

        /* Set power dependency */
        (void)Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);
    }

    return handle;
}

/*
 *  ======== AESCCM_close ========
 */
void AESCCM_close(AESCCM_Handle handle)
{
    AESCCM_s_CloseMsg closeMsg;

    DebugP_assert(handle);

    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Destruct SecureCallback object */
    SecureCallback_destruct(&aesccmSecureCB_ns[index].object);

    /* Setup interface for input parameter */
    closeMsg.handle = handle;
    invecs[0].base  = &closeMsg;
    invecs[0].len   = sizeof(closeMsg);

    /* Setup interface for null return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver */
    (void)CryptoPSACC26X4_call(AESCCM_S_MSG_TYPE_CLOSE, invecs, outvecs);

    /* Release power dependency */
    (void)Power_releaseDependency(PowerCC26XX_PERIPH_CRYPTO);
}

/*
 *  ======== AESCCM_oneStepOperation ========
 */
static int_fast16_t AESCCM_oneStepOperation(AESCCM_Handle handle,
                                            AESCCM_OneStepOperation *operationStruct,
                                            int32_t type)
{
    AESCCM_s_OneStepOperationMsg oneStepMsg;
    int_fast16_t result = AESCCM_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);
    uintptr_t key;

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(aesccmObject_ns[index].semaphoreTimeout) == false)
    {
        return AESCCM_STATUS_RESOURCE_UNAVAILABLE;
    }

    if ((aesccmObject_ns[index].returnBehavior == AESCCM_RETURN_BEHAVIOR_POLLING) &&
        (operationStruct->inputLength <= 256))
    {
        /* Call fast veneer */
        key = HwiP_disable();

        if (type == AESCCM_S_MSG_TYPE_ONE_STEP_ENCRYPT)
        {
            result = AESCCM_s_oneStepEncryptFast(handle, operationStruct);
        }
        else
        {
            result = AESCCM_s_oneStepDecryptFast(handle, operationStruct);
        }

        HwiP_restore(key);
        CryptoPSACC26X4_releaseLock();
    }
    else
    {
        /* Setup interface for input parameters */
        oneStepMsg.handle    = handle;
        oneStepMsg.operation = operationStruct;
        invecs[0].base       = &oneStepMsg;
        invecs[0].len        = sizeof(oneStepMsg);

        /* Setup interface for return value */
        outvecs[0].base = &result;
        outvecs[0].len  = sizeof(result);

        if (aesccmObject_ns[index].returnBehavior != AESCCM_RETURN_BEHAVIOR_POLLING)
        {
            (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
        }

        /*
         * PSA call to secure driver:
         *
         * Return value can be ignored since result (in outvecs) is initialized to AESCCM_STATUS_ERROR
         * and will only be updated if the PSA call is successful.
         */
        (void)CryptoPSACC26X4_call(type, invecs, outvecs);

        if ((aesccmObject_ns[index].returnBehavior != AESCCM_RETURN_BEHAVIOR_POLLING) &&
            (result != AESCCM_STATUS_SUCCESS))
        {
            (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
            CryptoPSACC26X4_releaseLock();
        }
        else if (aesccmObject_ns[index].returnBehavior == AESCCM_RETURN_BEHAVIOR_POLLING)
        {
            CryptoPSACC26X4_releaseLock();
        }
        else if (aesccmObject_ns[index].returnBehavior == AESCCM_RETURN_BEHAVIOR_BLOCKING)
        {
            (void)SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
            result = aesccmSecureCB_ns[index].returnValue;
        }
    }

    return (result);
}

/*
 *  ======== AESCCM_oneStepEncrypt ========
 */
int_fast16_t AESCCM_oneStepEncrypt(AESCCM_Handle handle, AESCCM_OneStepOperation *operationStruct)
{
    return AESCCM_oneStepOperation(handle, operationStruct, AESCCM_S_MSG_TYPE_ONE_STEP_ENCRYPT);
}

/*
 *  ======== AESCCM_oneStepDecrypt ========
 */
int_fast16_t AESCCM_oneStepDecrypt(AESCCM_Handle handle, AESCCM_OneStepOperation *operationStruct)
{
    return AESCCM_oneStepOperation(handle, operationStruct, AESCCM_S_MSG_TYPE_ONE_STEP_DECRYPT);
}

/*
 *  ======== AESCCM_setupOperation ========
 */
static int_fast16_t AESCCM_setupOperation(AESCCM_Handle handle,
                                          const CryptoKey *key,
                                          size_t totalAADLength,
                                          size_t totalPlaintextLength,
                                          size_t macLength,
                                          int32_t type)
{
    AESCCM_s_SetupOperationMsg setupMsg;
    int_fast16_t result = AESCCM_STATUS_ERROR;

    /* Setup interface for input parameters */
    setupMsg.handle          = handle;
    setupMsg.key             = key;
    setupMsg.aadLength       = totalAADLength;
    setupMsg.plaintextLength = totalPlaintextLength;
    setupMsg.macLength       = macLength;
    invecs[0].base           = &setupMsg;
    invecs[0].len            = sizeof(setupMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCCM_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(type, invecs, outvecs);

    return (result);
}

/*
 *  ======== AESCCM_setupEncrypt ========
 */
int_fast16_t AESCCM_setupEncrypt(AESCCM_Handle handle,
                                 const CryptoKey *key,
                                 size_t totalAADLength,
                                 size_t totalPlaintextLength,
                                 size_t macLength)
{
    return AESCCM_setupOperation(handle,
                                 key,
                                 totalAADLength,
                                 totalPlaintextLength,
                                 macLength,
                                 AESCCM_S_MSG_TYPE_SETUP_ENCRYPT);
}

/*
 *  ======== AESCCM_setupDecrypt ========
 */
int_fast16_t AESCCM_setupDecrypt(AESCCM_Handle handle,
                                 const CryptoKey *key,
                                 size_t totalAADLength,
                                 size_t totalPlaintextLength,
                                 size_t macLength)
{
    return AESCCM_setupOperation(handle,
                                 key,
                                 totalAADLength,
                                 totalPlaintextLength,
                                 macLength,
                                 AESCCM_S_MSG_TYPE_SETUP_DECRYPT);
}

/*
 *  ======== AESCCM_addAAD ========
 */
int_fast16_t AESCCM_addAAD(AESCCM_Handle handle, AESCCM_SegmentedAADOperation *operation)
{
    AESCCM_s_AddAADMsg addADDMsg;
    int_fast16_t result = AESCCM_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    DebugP_assert(handle);
    DebugP_assert(operation);

    /* Setup interface for input parameters */
    addADDMsg.handle    = handle;
    addADDMsg.operation = operation;
    invecs[0].base      = &addADDMsg;
    invecs[0].len       = sizeof(addADDMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(aesccmObject_ns[index].semaphoreTimeout) == false)
    {
        return AESCCM_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aesccmObject_ns[index].returnBehavior != AESCCM_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCCM_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCCM_S_MSG_TYPE_ADD_AAD, invecs, outvecs);

    if ((aesccmObject_ns[index].returnBehavior != AESCCM_RETURN_BEHAVIOR_POLLING) && (result != AESCCM_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesccmObject_ns[index].returnBehavior == AESCCM_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesccmObject_ns[index].returnBehavior == AESCCM_RETURN_BEHAVIOR_BLOCKING)
    {
        (void)SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aesccmSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESCCM_addData ========
 */
int_fast16_t AESCCM_addData(AESCCM_Handle handle, AESCCM_SegmentedDataOperation *operation)
{
    AESCCM_s_AddDataMsg addDataMsg;
    int_fast16_t result = AESCCM_STATUS_ERROR;
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
    if (CryptoPSACC26X4_acquireLock(aesccmObject_ns[index].semaphoreTimeout) == false)
    {
        return AESCCM_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aesccmObject_ns[index].returnBehavior != AESCCM_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCCM_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCCM_S_MSG_TYPE_ADD_DATA, invecs, outvecs);

    if ((aesccmObject_ns[index].returnBehavior != AESCCM_RETURN_BEHAVIOR_POLLING) && (result != AESCCM_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesccmObject_ns[index].returnBehavior == AESCCM_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesccmObject_ns[index].returnBehavior == AESCCM_RETURN_BEHAVIOR_BLOCKING)
    {
        (void)SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aesccmSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESCCM_finalizeEncrypt ========
 */
int_fast16_t AESCCM_finalizeEncrypt(AESCCM_Handle handle, AESCCM_SegmentedFinalizeOperation *operation)
{
    AESCCM_s_FinalizeOperationMsg finalizeMsg;
    int_fast16_t result = AESCCM_STATUS_ERROR;
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
    if (CryptoPSACC26X4_acquireLock(aesccmObject_ns[index].semaphoreTimeout) == false)
    {
        return AESCCM_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aesccmObject_ns[index].returnBehavior != AESCCM_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCCM_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCCM_S_MSG_TYPE_FINALIZE_ENCRYPT, invecs, outvecs);

    if ((aesccmObject_ns[index].returnBehavior != AESCCM_RETURN_BEHAVIOR_POLLING) && (result != AESCCM_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesccmObject_ns[index].returnBehavior == AESCCM_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesccmObject_ns[index].returnBehavior == AESCCM_RETURN_BEHAVIOR_BLOCKING)
    {
        (void)SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aesccmSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESCCM_finalizeDecrypt ========
 */
int_fast16_t AESCCM_finalizeDecrypt(AESCCM_Handle handle, AESCCM_SegmentedFinalizeOperation *operation)
{
    AESCCM_s_FinalizeOperationMsg finalizeMsg;
    int_fast16_t result = AESCCM_STATUS_ERROR;
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
    if (CryptoPSACC26X4_acquireLock(aesccmObject_ns[index].semaphoreTimeout) == false)
    {
        return AESCCM_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aesccmObject_ns[index].returnBehavior != AESCCM_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCCM_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCCM_S_MSG_TYPE_FINALIZE_DECRYPT, invecs, outvecs);

    if ((aesccmObject_ns[index].returnBehavior != AESCCM_RETURN_BEHAVIOR_POLLING) && (result != AESCCM_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesccmObject_ns[index].returnBehavior == AESCCM_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesccmObject_ns[index].returnBehavior == AESCCM_RETURN_BEHAVIOR_BLOCKING)
    {
        (void)SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aesccmSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESCCM_cancelOperation ========
 */
int_fast16_t AESCCM_cancelOperation(AESCCM_Handle handle)
{
    AESCCM_s_CancelOperationMsg cancelMsg;
    int_fast16_t result = AESCCM_STATUS_ERROR;

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
     * Return value can be ignored since result (in outvecs) is initialized to AESCCM_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCCM_S_MSG_TYPE_CANCEL_OPERATION, invecs, outvecs);

    return (result);
}

/*
 *  ======== AESCCM_setLengths ========
 */
int_fast16_t AESCCM_setLengths(AESCCM_Handle handle, size_t aadLength, size_t plaintextLength, size_t macLength)
{
    AESCCM_s_SetLengthsMsg setLengthMsg;
    int_fast16_t result = AESCCM_STATUS_ERROR;

    DebugP_assert(handle);

    /* Setup interface for input parameters */
    setLengthMsg.handle          = handle;
    setLengthMsg.aadLength       = aadLength;
    setLengthMsg.plaintextLength = plaintextLength;
    setLengthMsg.macLength       = macLength;
    invecs[0].base               = &setLengthMsg;
    invecs[0].len                = sizeof(setLengthMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCCM_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCCM_S_MSG_TYPE_SET_LENGTHS, invecs, outvecs);

    return (result);
}

/*
 *  ======== AESCCM_setNonce ========
 */
int_fast16_t AESCCM_setNonce(AESCCM_Handle handle, const uint8_t *nonce, size_t nonceLength)
{
    AESCCM_s_SetNonceMsg setNonceMsg;
    int_fast16_t result = AESCCM_STATUS_ERROR;

    DebugP_assert(handle);

    /* Setup interface for input parameters */
    setNonceMsg.handle      = handle;
    setNonceMsg.nonce       = nonce;
    setNonceMsg.nonceLength = nonceLength;
    invecs[0].base          = &setNonceMsg;
    invecs[0].len           = sizeof(setNonceMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCCM_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCCM_S_MSG_TYPE_SET_NONCE, invecs, outvecs);

    return (result);
}

/*
 *  ======== AESCCM_generateNonce ========
 */
int_fast16_t AESCCM_generateNonce(AESCCM_Handle handle, uint8_t *nonce, size_t nonceSize, size_t *nonceLength)
{
    /* This feature is not currently supported */
    return (AESCCM_STATUS_FEATURE_NOT_SUPPORTED);
}
