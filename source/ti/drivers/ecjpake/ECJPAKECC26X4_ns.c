/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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
#include <ti/drivers/ECJPAKE.h>
#include <ti/drivers/ecjpake/ECJPAKECC26X4_ns.h>
#include <ti/drivers/ecjpake/ECJPAKECC26X4_s.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26X2.h>

#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include <psa/client.h>

/* Static globals */
static bool isInitialized = false;

static volatile bool ECJPAKECC26X4_ns_pollingDone;

static struct psa_invec invecs[1];
static struct psa_outvec outvecs[1];

/* Extern globals */
extern const ECJPAKE_Params ECJPAKE_defaultParams;
extern ECJPAKE_s_SecureCallback ecjpakeSecureCB_ns[];
extern ECJPAKECC26X4_ns_Object ecjpakeObject_ns[];

/* Forward declarations */
static int_fast16_t ECJPAKE_ns_operation(ECJPAKE_Handle handle, ECJPAKE_Operation operation, int32_t msgType);
static psa_status_t ECJPAKE_ns_registerCallback(ECJPAKE_Handle handle, const ECJPAKE_Params *params);

/*
 *  ======== ECJPAKE_ns_callbackFxn ========
 */
void ECJPAKE_ns_callbackFxn(uintptr_t arg)
{
    ECJPAKE_s_SecureCallback *secureCallbackObject = (ECJPAKE_s_SecureCallback *)arg;
    uintptr_t index                                = GET_CRYPTO_S_HANDLE_INDEX(secureCallbackObject->handle);

    (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

    CryptoPSACC26X4_releaseLock();

    if (ecjpakeObject_ns[index].returnBehavior == ECJPAKE_RETURN_BEHAVIOR_POLLING)
    {
        ECJPAKECC26X4_ns_pollingDone = true;
    }
    else if (ecjpakeObject_ns[index].returnBehavior == ECJPAKE_RETURN_BEHAVIOR_BLOCKING)
    {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&CryptoPSACC26X4_operationSemaphore);
    }
    else
    {
        /* Call the callback function provided by the application. */
        ecjpakeObject_ns[index].callbackFxn(ecjpakeSecureCB_ns[index].handle,
                                            ecjpakeSecureCB_ns[index].returnStatus,
                                            ecjpakeSecureCB_ns[index].operation,
                                            ecjpakeSecureCB_ns[index].operationType);
    }
}

/*
 *  ======== ECJPAKE_ns_registerCallback ========
 */
static psa_status_t ECJPAKE_ns_registerCallback(ECJPAKE_Handle handle, const ECJPAKE_Params *params)
{
    ECJPAKE_s_CallbackMsg callbackMsg;
    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Construct SecureCB object */
    SecureCallback_construct(&ecjpakeSecureCB_ns[index].object,
                             ECJPAKE_ns_callbackFxn,
                             (uintptr_t)&ecjpakeSecureCB_ns[index]);

    callbackMsg.handle   = handle;
    callbackMsg.callback = &ecjpakeSecureCB_ns[index];
    invecs[0].base       = &callbackMsg;
    invecs[0].len        = sizeof(callbackMsg);

    /* Setup interface for return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver - assumes CryptoPSACC26X4 lock is already acquired */
    return CryptoPSACC26X4_call(ECJPAKE_S_MSG_TYPE_REGISTER_CALLBACK, invecs, outvecs);
}

/*
 *  ======== ECJPAKE_ns_waitForResult ========
 */
static void ECJPAKE_ns_waitForResult(int_fast16_t *result, uint8_t objectIndex)
{
    if (*result != ECJPAKE_STATUS_SUCCESS)
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (ecjpakeObject_ns[objectIndex].returnBehavior == ECJPAKE_RETURN_BEHAVIOR_POLLING)
    {
        /*
         * Emulate polling mode by spinning on a flag which will be set by
         * the callback function
         */
        while (!ECJPAKECC26X4_ns_pollingDone) {}
        *result = ecjpakeSecureCB_ns[objectIndex].returnStatus;
    }
    else if (ecjpakeObject_ns[objectIndex].returnBehavior == ECJPAKE_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        *result = ecjpakeSecureCB_ns[objectIndex].returnStatus;
    }
}

/*
 *  ======== ECJPAKE_init ========
 */
void ECJPAKE_init(void)
{
    if (!isInitialized)
    {
        /* Initialize CryptoPSA semaphores and SecureCB driver */
        CryptoPSACC26X4_init();

        isInitialized = true;
    }
}

/*
 *  ======== ECJPAKE_open ========
 */
ECJPAKE_Handle ECJPAKE_open(uint_least8_t index, const ECJPAKE_Params *params)
{
    ECJPAKE_Handle handle = NULL;
    ECJPAKE_s_OpenMsg openMsg;
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
        params = (ECJPAKE_Params *)&ECJPAKE_defaultParams;
    }

    DebugP_assert(params->returnBehavior == ECJPAKE_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(ECJPAKE_S_MSG_TYPE_OPEN, invecs, outvecs);

    if (handle != NULL)
    {
        /* Register NS callback regardless of return behavior */
        if (ECJPAKE_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        ecjpakeObject_ns[index].returnBehavior   = params->returnBehavior;
        ecjpakeObject_ns[index].callbackFxn      = params->callbackFxn;
        ecjpakeObject_ns[index].semaphoreTimeout = (params->returnBehavior == ECJPAKE_RETURN_BEHAVIOR_BLOCKING)
                                                       ? params->timeout
                                                       : SemaphoreP_NO_WAIT;

        /* Set power dependency - i.e. power up and enable clock for PKA */
        (void)Power_setDependency(PowerCC26X2_PERIPH_PKA);
    }

    return handle;
}

/*
 *  ======== ECJPAKE_construct ========
 */
ECJPAKE_Handle ECJPAKE_construct(ECJPAKE_Config *config, const ECJPAKE_Params *params)
{
    ECJPAKE_Handle handle = NULL;
    ECJPAKE_s_ConstructMsg constructMsg;
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
        params = (ECJPAKE_Params *)&ECJPAKE_defaultParams;
    }

    DebugP_assert(params->returnBehavior == ECJPAKE_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(ECJPAKE_S_MSG_TYPE_CONSTRUCT, invecs, outvecs);

    if (handle != NULL)
    {
        /* Register NS callback regardless of return behavior */
        if (ECJPAKE_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        ecjpakeObject_ns[index].returnBehavior   = params->returnBehavior;
        ecjpakeObject_ns[index].callbackFxn      = params->callbackFxn;
        ecjpakeObject_ns[index].semaphoreTimeout = (params->returnBehavior == ECJPAKE_RETURN_BEHAVIOR_BLOCKING)
                                                       ? params->timeout
                                                       : SemaphoreP_NO_WAIT;

        /* Set power dependency - i.e. power up and enable clock for PKA */
        (void)Power_setDependency(PowerCC26X2_PERIPH_PKA);
    }

    return handle;
}

/*
 *  ======== ECJPAKE_close ========
 */
void ECJPAKE_close(ECJPAKE_Handle handle)
{
    ECJPAKE_s_CloseMsg closeMsg;

    DebugP_assert(handle);

    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Destruct SecureCallback object */
    SecureCallback_destruct(&ecjpakeSecureCB_ns[index].object);

    /* Setup interface for input parameter */
    closeMsg.handle = handle;
    invecs[0].base  = &closeMsg;
    invecs[0].len   = sizeof(closeMsg);

    /* Setup interface for null return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver */
    (void)CryptoPSACC26X4_call(ECJPAKE_S_MSG_TYPE_CLOSE, invecs, outvecs);

    /* Release power dependency */
    (void)Power_releaseDependency(PowerCC26X2_PERIPH_PKA);
}

/*
 *  ======== ECJPAKE_ns_operation ========
 */
static int_fast16_t ECJPAKE_ns_operation(ECJPAKE_Handle handle, ECJPAKE_Operation operation, int32_t msgType)
{
    ECJPAKE_s_OperationMsg msg;
    int_fast16_t result = ECJPAKE_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    DebugP_assert(handle);
    DebugP_assert(operation);

    /* Setup interface for input parameters */
    msg.handle     = handle;
    msg.operation  = operation;
    invecs[0].base = &msg;
    invecs[0].len  = sizeof(msg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(ecjpakeObject_ns[index].semaphoreTimeout) == false)
    {
        return ECJPAKE_STATUS_RESOURCE_UNAVAILABLE;
    }

    ECJPAKECC26X4_ns_pollingDone = false;

    (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to ECJPAKE_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(msgType, invecs, outvecs);

    ECJPAKE_ns_waitForResult(&result, index);

    return (result);
}

/*
 *  ======== ECJPAKE_roundOneGenerateKeys ========
 */
int_fast16_t ECJPAKE_roundOneGenerateKeys(ECJPAKE_Handle handle, ECJPAKE_OperationRoundOneGenerateKeys *operation)
{
    return ECJPAKE_ns_operation(handle, (ECJPAKE_Operation)operation, ECJPAKE_S_MSG_TYPE_ROUND_ONE_GENERATE_KEYS);
}

/*
 *  ======== ECJPAKE_generateZKP ========
 */
int_fast16_t ECJPAKE_generateZKP(ECJPAKE_Handle handle, ECJPAKE_OperationGenerateZKP *operation)
{
    return ECJPAKE_ns_operation(handle, (ECJPAKE_Operation)operation, ECJPAKE_S_MSG_TYPE_GENERATE_ZKP);
}

/*
 *  ======== ECJPAKE_verifyZKP ========
 */
int_fast16_t ECJPAKE_verifyZKP(ECJPAKE_Handle handle, ECJPAKE_OperationVerifyZKP *operation)
{
    return ECJPAKE_ns_operation(handle, (ECJPAKE_Operation)operation, ECJPAKE_S_MSG_TYPE_VERIFY_ZKP);
}

/*
 *  ======== ECJPAKE_roundTwoGenerateKeys ========
 */
int_fast16_t ECJPAKE_roundTwoGenerateKeys(ECJPAKE_Handle handle, ECJPAKE_OperationRoundTwoGenerateKeys *operation)
{
    return ECJPAKE_ns_operation(handle, (ECJPAKE_Operation)operation, ECJPAKE_S_MSG_TYPE_ROUND_TWO_GENERATE_KEYS);
}

/*
 *  ======== ECJPAKE_computeSharedSecret ========
 */
int_fast16_t ECJPAKE_computeSharedSecret(ECJPAKE_Handle handle, ECJPAKE_OperationComputeSharedSecret *operation)
{
    return ECJPAKE_ns_operation(handle, (ECJPAKE_Operation)operation, ECJPAKE_S_MSG_TYPE_COMPUTE_SHARED_SECRET);
}

/*
 *  ======== ECJPAKE_cancelOperation ========
 */
int_fast16_t ECJPAKE_cancelOperation(ECJPAKE_Handle handle)
{
    ECJPAKE_s_CancelOperationMsg cancelMsg;
    int_fast16_t result = ECJPAKE_STATUS_ERROR;

    /* Setup interface for input parameters */
    cancelMsg.handle = handle;
    invecs[0].base   = &cancelMsg;
    invecs[0].len    = sizeof(cancelMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to ECJPAKE_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(ECJPAKE_S_MSG_TYPE_CANCEL_OPERATION, invecs, outvecs);

    return (result);
}
