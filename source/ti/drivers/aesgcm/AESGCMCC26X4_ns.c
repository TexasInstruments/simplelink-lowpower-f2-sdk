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

#include <ti/drivers/AESGCM.h>
#include <ti/drivers/aesgcm/AESGCMCC26X4_ns.h>
#include <ti/drivers/aesgcm/AESGCMCC26X4_s.h>
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

extern AESGCM_s_SecureCallback aesgcmSecureCB_ns[];
extern AESGCMCC26X4_ns_Object aesgcmObject_ns[];

/*
 *  ======== AESGCM_ns_callbackFxn ========
 */
void AESGCM_ns_callbackFxn(uintptr_t arg)
{
    AESGCM_s_SecureCallback *secureCallbackObject = (AESGCM_s_SecureCallback *)arg;
    uintptr_t index                               = GET_CRYPTO_S_HANDLE_INDEX(secureCallbackObject->handle);

    (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
    CryptoPSACC26X4_releaseLock();

    if (aesgcmObject_ns[index].returnBehavior == AESGCM_RETURN_BEHAVIOR_BLOCKING)
    {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&CryptoPSACC26X4_operationSemaphore);
    }
    else if (aesgcmObject_ns[index].returnBehavior == AESGCM_RETURN_BEHAVIOR_CALLBACK)
    {
        /* Call the callback function provided by the application. */
        aesgcmObject_ns[index].callbackFxn(aesgcmSecureCB_ns[index].handle,
                                           aesgcmSecureCB_ns[index].returnValue,
                                           aesgcmSecureCB_ns[index].operation,
                                           aesgcmSecureCB_ns[index].operationType);
    }
}

/*
 *  ======== AESGCM_ns_registerCallback ========
 */
static psa_status_t AESGCM_ns_registerCallback(AESGCM_Handle handle, const AESGCM_Params *params)
{
    AESGCM_s_CallbackMsg callbackMsg;
    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Construct SecureCB object */
    SecureCallback_construct(&aesgcmSecureCB_ns[index].object,
                             AESGCM_ns_callbackFxn,
                             (uintptr_t)&aesgcmSecureCB_ns[index]);

    callbackMsg.handle   = handle;
    callbackMsg.callback = &aesgcmSecureCB_ns[index];
    invecs[0].base       = &callbackMsg;
    invecs[0].len        = sizeof(callbackMsg);

    /* Setup interface for return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver - assumes CryptoPSACC26X4 lock is already acquired */
    return CryptoPSACC26X4_call(AESGCM_S_MSG_TYPE_REGISTER_CALLBACK, invecs, outvecs);
}

/*
 *  ======== AESGCM_init ========
 */
void AESGCM_init(void)
{
    if (!isInitialized)
    {
        /* Initialize CryptoPSA semaphores and SecureCB driver */
        CryptoPSACC26X4_init();

        isInitialized = true;
    }
}

/*
 *  ======== AESGCM_open ========
 */
AESGCM_Handle AESGCM_open(uint_least8_t index, const AESGCM_Params *params)
{
    AESGCM_Handle handle = NULL;
    AESGCM_s_OpenMsg openMsg;
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
        params = (AESGCM_Params *)&AESGCM_defaultParams;
    }

    DebugP_assert(params->returnBehavior == AESGCM_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(AESGCM_S_MSG_TYPE_OPEN, invecs, outvecs);

    if ((handle != NULL) && (params->returnBehavior != AESGCM_RETURN_BEHAVIOR_POLLING))
    {
        if (AESGCM_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        aesgcmObject_ns[index].returnBehavior   = params->returnBehavior;
        aesgcmObject_ns[index].callbackFxn      = params->callbackFxn;
        aesgcmObject_ns[index].semaphoreTimeout = params->returnBehavior == AESGCM_RETURN_BEHAVIOR_BLOCKING
                                                      ? params->timeout
                                                      : SemaphoreP_NO_WAIT;

        /* Set power dependency */
        (void)Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);
    }

    return handle;
}

/*
 *  ======== AESGCM_construct ========
 */
AESGCM_Handle AESGCM_construct(AESGCM_Config *config, const AESGCM_Params *params)
{
    AESGCM_Handle handle = NULL;
    AESGCM_s_ConstructMsg constructMsg;
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
        params = (AESGCM_Params *)&AESGCM_defaultParams;
    }

    DebugP_assert(params->returnBehavior == AESGCM_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(AESGCM_S_MSG_TYPE_CONSTRUCT, invecs, outvecs);

    if ((handle != NULL) && (params->returnBehavior != AESGCM_RETURN_BEHAVIOR_POLLING))
    {
        if (AESGCM_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        aesgcmObject_ns[index].returnBehavior   = params->returnBehavior;
        aesgcmObject_ns[index].callbackFxn      = params->callbackFxn;
        aesgcmObject_ns[index].semaphoreTimeout = params->returnBehavior == AESGCM_RETURN_BEHAVIOR_BLOCKING
                                                      ? params->timeout
                                                      : SemaphoreP_NO_WAIT;

        /* Set power dependency */
        (void)Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);
    }

    return handle;
}

/*
 *  ======== AESGCM_close ========
 */
void AESGCM_close(AESGCM_Handle handle)
{
    AESGCM_s_CloseMsg closeMsg;

    DebugP_assert(handle);

    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Destruct SecureCallback object */
    SecureCallback_destruct(&aesgcmSecureCB_ns[index].object);

    /* Setup interface for input parameter */
    closeMsg.handle = handle;
    invecs[0].base  = &closeMsg;
    invecs[0].len   = sizeof(closeMsg);

    /* Setup interface for null return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver */
    (void)CryptoPSACC26X4_call(AESGCM_S_MSG_TYPE_CLOSE, invecs, outvecs);

    /* Release power dependency */
    (void)Power_releaseDependency(PowerCC26XX_PERIPH_CRYPTO);
}

/*
 *  ======== AESGCM_oneStepOperation ========
 */
static int_fast16_t AESGCM_oneStepOperation(AESGCM_Handle handle,
                                            AESGCM_OneStepOperation *operationStruct,
                                            int32_t type)
{
    AESGCM_s_OneStepOperationMsg oneStepMsg;
    int_fast16_t result = AESGCM_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Setup interface for input parameters */
    oneStepMsg.handle    = handle;
    oneStepMsg.operation = operationStruct;
    invecs[0].base       = &oneStepMsg;
    invecs[0].len        = sizeof(oneStepMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(aesgcmObject_ns[index].semaphoreTimeout) == false)
    {
        return AESGCM_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aesgcmObject_ns[index].returnBehavior != AESGCM_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESGCM_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(type, invecs, outvecs);

    if ((aesgcmObject_ns[index].returnBehavior != AESGCM_RETURN_BEHAVIOR_POLLING) && (result != AESGCM_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesgcmObject_ns[index].returnBehavior == AESGCM_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesgcmObject_ns[index].returnBehavior == AESGCM_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aesgcmSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESGCM_oneStepEncrypt ========
 */
int_fast16_t AESGCM_oneStepEncrypt(AESGCM_Handle handle, AESGCM_OneStepOperation *operationStruct)
{
    return AESGCM_oneStepOperation(handle, operationStruct, AESGCM_S_MSG_TYPE_ONE_STEP_ENCRYPT);
}

/*
 *  ======== AESGCM_oneStepDecrypt ========
 */
int_fast16_t AESGCM_oneStepDecrypt(AESGCM_Handle handle, AESGCM_OneStepOperation *operationStruct)
{
    return AESGCM_oneStepOperation(handle, operationStruct, AESGCM_S_MSG_TYPE_ONE_STEP_DECRYPT);
}

/*
 *  ======== AESGCM_setupOperation ========
 */
static int_fast16_t AESGCM_setupOperation(AESGCM_Handle handle,
                                          const CryptoKey *key,
                                          size_t totalAADLength,
                                          size_t totalPlaintextLength,
                                          int32_t type)
{
    AESGCM_s_SetupOperationMsg setupMsg;
    int_fast16_t result = AESGCM_STATUS_ERROR;

    /* Setup interface for input parameters */
    setupMsg.handle               = handle;
    setupMsg.key                  = key;
    setupMsg.totalAADLength       = totalAADLength;
    setupMsg.totalPlaintextLength = totalPlaintextLength;
    invecs[0].base                = &setupMsg;
    invecs[0].len                 = sizeof(setupMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESGCM_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(type, invecs, outvecs);

    return (result);
}

/*
 *  ======== AESGCM_setupEncrypt ========
 */
int_fast16_t AESGCM_setupEncrypt(AESGCM_Handle handle,
                                 const CryptoKey *key,
                                 size_t totalAADLength,
                                 size_t totalPlaintextLength)
{
    return AESGCM_setupOperation(handle, key, totalAADLength, totalPlaintextLength, AESGCM_S_MSG_TYPE_SETUP_ENCRYPT);
}

/*
 *  ======== AESGCM_setupDecrypt ========
 */
int_fast16_t AESGCM_setupDecrypt(AESGCM_Handle handle,
                                 const CryptoKey *key,
                                 size_t totalAADLength,
                                 size_t totalPlaintextLength)
{
    return AESGCM_setupOperation(handle, key, totalAADLength, totalPlaintextLength, AESGCM_S_MSG_TYPE_SETUP_DECRYPT);
}

/*
 *  ======== AESGCM_setLengths ========
 */
int_fast16_t AESGCM_setLengths(AESGCM_Handle handle, size_t aadLength, size_t plaintextLength)
{
    AESGCM_s_SetLengthsMsg setLengthMsg;
    int_fast16_t result = AESGCM_STATUS_ERROR;

    DebugP_assert(handle);

    /* Setup interface for input parameters */
    setLengthMsg.handle          = handle;
    setLengthMsg.aadLength       = aadLength;
    setLengthMsg.plaintextLength = plaintextLength;
    invecs[0].base               = &setLengthMsg;
    invecs[0].len                = sizeof(setLengthMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESGCM_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESGCM_S_MSG_TYPE_SET_LENGTHS, invecs, outvecs);

    return (result);
}

/*
 *  ======== AESGCM_setIV ========
 */
int_fast16_t AESGCM_setIV(AESGCM_Handle handle, const uint8_t *iv, size_t ivLength)
{
    AESGCM_s_SetIVMsg setIVMsg;
    int_fast16_t result = AESGCM_STATUS_ERROR;

    DebugP_assert(handle);

    /* Setup interface for input parameters */
    setIVMsg.handle   = handle;
    setIVMsg.iv       = iv;
    setIVMsg.ivLength = ivLength;
    invecs[0].base    = &setIVMsg;
    invecs[0].len     = sizeof(setIVMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESGCM_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESGCM_S_MSG_TYPE_SET_IV, invecs, outvecs);

    return (result);
}

/*
 *  ======== AESGCM_addAAD ========
 */
int_fast16_t AESGCM_addAAD(AESGCM_Handle handle, AESGCM_SegmentedAADOperation *operation)
{
    AESGCM_s_AddAADMsg addADDMsg;
    int_fast16_t result = AESGCM_STATUS_ERROR;
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
    if (CryptoPSACC26X4_acquireLock(aesgcmObject_ns[index].semaphoreTimeout) == false)
    {
        return AESGCM_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aesgcmObject_ns[index].returnBehavior != AESGCM_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESGCM_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESGCM_S_MSG_TYPE_ADD_AAD, invecs, outvecs);

    if ((aesgcmObject_ns[index].returnBehavior != AESGCM_RETURN_BEHAVIOR_POLLING) && (result != AESGCM_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesgcmObject_ns[index].returnBehavior == AESGCM_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesgcmObject_ns[index].returnBehavior == AESGCM_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aesgcmSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESGCM_addData ========
 */
int_fast16_t AESGCM_addData(AESGCM_Handle handle, AESGCM_SegmentedDataOperation *operation)
{
    AESGCM_s_AddDataMsg addDataMsg;
    int_fast16_t result = AESGCM_STATUS_ERROR;
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
    if (CryptoPSACC26X4_acquireLock(aesgcmObject_ns[index].semaphoreTimeout) == false)
    {
        return AESGCM_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aesgcmObject_ns[index].returnBehavior != AESGCM_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESGCM_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESGCM_S_MSG_TYPE_ADD_DATA, invecs, outvecs);

    if ((aesgcmObject_ns[index].returnBehavior != AESGCM_RETURN_BEHAVIOR_POLLING) && (result != AESGCM_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesgcmObject_ns[index].returnBehavior == AESGCM_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesgcmObject_ns[index].returnBehavior == AESGCM_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aesgcmSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESGCM_finalizeOperation ========
 */
static int_fast16_t AESGCM_finalizeOperation(AESGCM_Handle handle,
                                             AESGCM_SegmentedFinalizeOperation *operation,
                                             int32_t type)
{
    AESGCM_s_FinalizeOperationMsg finalizeMsg;
    int_fast16_t result = AESGCM_STATUS_ERROR;
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
    if (CryptoPSACC26X4_acquireLock(aesgcmObject_ns[index].semaphoreTimeout) == false)
    {
        return AESGCM_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aesgcmObject_ns[index].returnBehavior != AESGCM_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESGCM_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(type, invecs, outvecs);

    if ((aesgcmObject_ns[index].returnBehavior != AESGCM_RETURN_BEHAVIOR_POLLING) && (result != AESGCM_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesgcmObject_ns[index].returnBehavior == AESGCM_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aesgcmObject_ns[index].returnBehavior == AESGCM_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aesgcmSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESGCM_finalizeEncrypt ========
 */
int_fast16_t AESGCM_finalizeEncrypt(AESGCM_Handle handle, AESGCM_SegmentedFinalizeOperation *operation)
{
    return AESGCM_finalizeOperation(handle, operation, AESGCM_S_MSG_TYPE_FINALIZE_ENCRYPT);
}

/*
 *  ======== AESGCM_finalizeDecrypt ========
 */
int_fast16_t AESGCM_finalizeDecrypt(AESGCM_Handle handle, AESGCM_SegmentedFinalizeOperation *operation)
{
    {
        return AESGCM_finalizeOperation(handle, operation, AESGCM_S_MSG_TYPE_FINALIZE_DECRYPT);
    }
}

/*
 *  ======== AESGCM_cancelOperation ========
 */
int_fast16_t AESGCM_cancelOperation(AESGCM_Handle handle)
{
    AESGCM_s_CancelOperationMsg cancelMsg;
    int_fast16_t result = AESGCM_STATUS_ERROR;

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
     * Return value can be ignored since result (in outvecs) is initialized to AESGCM_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESGCM_S_MSG_TYPE_CANCEL_OPERATION, invecs, outvecs);

    return (result);
}
