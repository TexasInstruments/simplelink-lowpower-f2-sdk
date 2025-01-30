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

#include <ti/drivers/AESCBC.h>
#include <ti/drivers/aescbc/AESCBCCC26X4_ns.h>
#include <ti/drivers/aescbc/AESCBCCC26X4_s.h>
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

extern AESCBC_s_SecureCallback aescbcSecureCB_ns[];
extern AESCBCCC26X4_ns_Object aescbcObject_ns[];

/*
 *  ======== AESCBC_ns_callbackFxn ========
 */
void AESCBC_ns_callbackFxn(uintptr_t arg)
{
    AESCBC_s_SecureCallback *secureCallbackObject = (AESCBC_s_SecureCallback *)arg;

    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(secureCallbackObject->handle);

    (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
    CryptoPSACC26X4_releaseLock();

    if (aescbcObject_ns[index].returnBehavior == AESCBC_RETURN_BEHAVIOR_BLOCKING)
    {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&CryptoPSACC26X4_operationSemaphore);
    }
    else if (aescbcObject_ns[index].returnBehavior == AESCBC_RETURN_BEHAVIOR_CALLBACK)
    {
        /* Call the callback function provided by the application */
        aescbcObject_ns[index].callbackFxn(aescbcSecureCB_ns[index].handle,
                                           aescbcSecureCB_ns[index].returnValue,
                                           aescbcSecureCB_ns[index].operation,
                                           aescbcSecureCB_ns[index].operationType);
    }
}

/*
 *  ======== AESCBC_ns_registerCallback ========
 */
static psa_status_t AESCBC_ns_registerCallback(AESCBC_Handle handle, const AESCBC_Params *params)
{
    AESCBC_s_CallbackMsg callbackMsg;
    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Construct SecureCallback object */
    SecureCallback_construct(&aescbcSecureCB_ns[index].object,
                             AESCBC_ns_callbackFxn,
                             (uintptr_t)&aescbcSecureCB_ns[index]);

    callbackMsg.handle   = handle;
    callbackMsg.callback = &aescbcSecureCB_ns[index];
    invecs[0].base       = &callbackMsg;
    invecs[0].len        = sizeof(callbackMsg);

    /* Setup interface for return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver */
    return CryptoPSACC26X4_call(AESCBC_S_MSG_TYPE_REGISTER_CALLBACK, invecs, outvecs);
}

/*
 *  ======== AESCBC_init ========
 */
void AESCBC_init(void)
{
    if (!isInitialized)
    {
        /* Initialize CryptoPSA semaphores and SecureCallback driver */
        CryptoPSACC26X4_init();

        isInitialized = true;
    }
}

/*
 *  ======== AESCBC_open ========
 */
AESCBC_Handle AESCBC_open(uint_least8_t index, const AESCBC_Params *params)
{
    AESCBC_Handle handle = NULL;
    AESCBC_s_OpenMsg openMsg;
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
        params = (AESCBC_Params *)&AESCBC_defaultParams;
    }

    DebugP_assert(params->returnBehavior == AESCBC_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
     * Return value can be ignored since handle (in outvecs) is initialized to NULL and
     * will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCBC_S_MSG_TYPE_OPEN, invecs, outvecs);

    if ((handle != NULL) && (params->returnBehavior != AESCBC_RETURN_BEHAVIOR_POLLING))
    {
        if (AESCBC_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        aescbcObject_ns[index].returnBehavior   = params->returnBehavior;
        aescbcObject_ns[index].callbackFxn      = params->callbackFxn;
        aescbcObject_ns[index].semaphoreTimeout = params->returnBehavior == AESCBC_RETURN_BEHAVIOR_BLOCKING
                                                      ? params->timeout
                                                      : SemaphoreP_NO_WAIT;
        /* Set power dependency */
        (void)Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);
    }

    return handle;
}

/*
 *  ======== AESCBC_construct ========
 */
AESCBC_Handle AESCBC_construct(AESCBC_Config *config, const AESCBC_Params *params)
{
    AESCBC_Handle handle = NULL;
    AESCBC_s_ConstructMsg constructMsg;
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
        params = (AESCBC_Params *)&AESCBC_defaultParams;
    }

    DebugP_assert(params->returnBehavior == AESCBC_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
     * Return value can be ignored since handle (in outvecs) is initialized to NULL and
     * will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCBC_S_MSG_TYPE_CONSTRUCT, invecs, outvecs);

    if ((handle != NULL) && (params->returnBehavior != AESCBC_RETURN_BEHAVIOR_POLLING))
    {
        if (AESCBC_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        aescbcObject_ns[index].returnBehavior   = params->returnBehavior;
        aescbcObject_ns[index].callbackFxn      = params->callbackFxn;
        aescbcObject_ns[index].semaphoreTimeout = params->returnBehavior == AESCBC_RETURN_BEHAVIOR_BLOCKING
                                                      ? params->timeout
                                                      : SemaphoreP_NO_WAIT;

        /* Set power dependency */
        (void)Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);
    }

    return handle;
}

/*
 *  ======== AESCBC_close ========
 */
void AESCBC_close(AESCBC_Handle handle)
{
    AESCBC_s_CloseMsg closeMsg;

    DebugP_assert(handle);

    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Destruct SecureCallback object */
    SecureCallback_destruct(&aescbcSecureCB_ns[index].object);

    /* Setup interface for input parameter */
    closeMsg.handle = handle;
    invecs[0].base  = &closeMsg;
    invecs[0].len   = sizeof(closeMsg);

    /* Setup interface for null return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver */
    (void)CryptoPSACC26X4_call(AESCBC_S_MSG_TYPE_CLOSE, invecs, outvecs);

    /* Release power dependency */
    (void)Power_releaseDependency(PowerCC26XX_PERIPH_CRYPTO);
}

/*
 *  ======== AESCBC_oneStepOperation ========
 */
static int_fast16_t AESCBC_oneStepOperation(AESCBC_Handle handle,
                                            AESCBC_OneStepOperation *operationStruct,
                                            int32_t type)
{
    AESCBC_s_OneStepOperationMsg oneStepMsg;
    int_fast16_t result = AESCBC_STATUS_ERROR;
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
    if (CryptoPSACC26X4_acquireLock(aescbcObject_ns[index].semaphoreTimeout) == false)
    {
        return AESCBC_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aescbcObject_ns[index].returnBehavior != AESCBC_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCBC_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(type, invecs, outvecs);

    if ((aescbcObject_ns[index].returnBehavior != AESCBC_RETURN_BEHAVIOR_POLLING) && (result != AESCBC_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aescbcObject_ns[index].returnBehavior == AESCBC_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aescbcObject_ns[index].returnBehavior == AESCBC_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aescbcSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESCBC_oneStepEncrypt ========
 */
int_fast16_t AESCBC_oneStepEncrypt(AESCBC_Handle handle, AESCBC_OneStepOperation *operationStruct)
{
    return AESCBC_oneStepOperation(handle, operationStruct, AESCBC_S_MSG_TYPE_ONE_STEP_ENCRYPT);
}

/*
 *  ======== AESCBC_oneStepDecrypt ========
 */
int_fast16_t AESCBC_oneStepDecrypt(AESCBC_Handle handle, AESCBC_OneStepOperation *operationStruct)
{
    return AESCBC_oneStepOperation(handle, operationStruct, AESCBC_S_MSG_TYPE_ONE_STEP_DECRYPT);
}

/*
 *  ======== AESCBC_setupOperation ========
 */
static int_fast16_t AESCBC_setupOperation(AESCBC_Handle handle, const CryptoKey *key, int32_t type)
{
    AESCBC_s_SetupOperationMsg setupMsg;
    int_fast16_t result = AESCBC_STATUS_ERROR;

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
     * Return value can be ignored since result (in outvecs) is initialized to AESCBC_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(type, invecs, outvecs);

    return (result);
}

/*
 *  ======== AESCBC_setupEncrypt ========
 */
int_fast16_t AESCBC_setupEncrypt(AESCBC_Handle handle, const CryptoKey *key)
{
    return AESCBC_setupOperation(handle, key, AESCBC_S_MSG_TYPE_SETUP_ENCRYPT);
}

/*
 *  ======== AESCBC_setupDecrypt ========
 */
int_fast16_t AESCBC_setupDecrypt(AESCBC_Handle handle, const CryptoKey *key)
{
    return AESCBC_setupOperation(handle, key, AESCBC_S_MSG_TYPE_SETUP_DECRYPT);
}

/*
 *  ======== AESCBC_setIV ========
 */
int_fast16_t AESCBC_setIV(AESCBC_Handle handle, const uint8_t *iv, size_t ivLength)
{
    AESCBC_s_SetIVMsg setIVMsg;
    int_fast16_t result = AESCBC_STATUS_ERROR;

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
     * Return value can be ignored since result (in outvecs) is initialized to AESCBC_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCBC_S_MSG_TYPE_SET_IV, invecs, outvecs);

    return (result);
}

/*
 *  ======== AESCBC_addData ========
 */
int_fast16_t AESCBC_addData(AESCBC_Handle handle, AESCBC_SegmentedOperation *operation)
{
    AESCBC_s_AddDataMsg addDataMsg;
    int_fast16_t result = AESCBC_STATUS_ERROR;
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
    if (CryptoPSACC26X4_acquireLock(aescbcObject_ns[index].semaphoreTimeout) == false)
    {
        return AESCBC_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aescbcObject_ns[index].returnBehavior != AESCBC_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCBC_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCBC_S_MSG_TYPE_ADD_DATA, invecs, outvecs);

    if ((aescbcObject_ns[index].returnBehavior != AESCBC_RETURN_BEHAVIOR_POLLING) && (result != AESCBC_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aescbcObject_ns[index].returnBehavior == AESCBC_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aescbcObject_ns[index].returnBehavior == AESCBC_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aescbcSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESCBC_finalize ========
 */
int_fast16_t AESCBC_finalize(AESCBC_Handle handle, AESCBC_SegmentedOperation *operation)
{
    AESCBC_s_FinalizeMsg finalizeMsg;
    int_fast16_t result = AESCBC_STATUS_ERROR;
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
    if (CryptoPSACC26X4_acquireLock(aescbcObject_ns[index].semaphoreTimeout) == false)
    {
        return AESCBC_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (aescbcObject_ns[index].returnBehavior != AESCBC_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to AESCBC_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCBC_S_MSG_TYPE_FINALIZE, invecs, outvecs);

    if ((aescbcObject_ns[index].returnBehavior != AESCBC_RETURN_BEHAVIOR_POLLING) && (result != AESCBC_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (aescbcObject_ns[index].returnBehavior == AESCBC_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (aescbcObject_ns[index].returnBehavior == AESCBC_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = aescbcSecureCB_ns[index].returnValue;
    }

    return (result);
}

/*
 *  ======== AESCBC_cancelOperation ========
 */
int_fast16_t AESCBC_cancelOperation(AESCBC_Handle handle)
{
    AESCBC_s_CancelOperationMsg cancelMsg;
    int_fast16_t result = AESCBC_STATUS_ERROR;

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
     * Return value can be ignored since result (in outvecs) is initialized to AESCBC_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(AESCBC_S_MSG_TYPE_CANCEL_OPERATION, invecs, outvecs);

    return (result);
}

/*
 *  ======== AESCBC_generateIV ========
 */
int_fast16_t AESCBC_generateIV(AESCBC_Handle handle, uint8_t *iv, size_t ivSize, size_t *ivLength)
{
    /* This is not currently supported */
    return (AESCBC_STATUS_FEATURE_NOT_SUPPORTED);
}
