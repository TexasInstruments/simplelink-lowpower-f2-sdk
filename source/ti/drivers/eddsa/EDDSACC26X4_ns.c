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
#include <ti/drivers/EDDSA.h>
#include <ti/drivers/eddsa/EDDSACC26X4_ns.h>
#include <ti/drivers/eddsa/EDDSACC26X4_s.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26X2.h>
#include <ti/drivers/power/PowerCC26XX.h>

#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include <psa/client.h>

/* Static globals */
static bool isInitialized = false;

static volatile bool EDDSACC26X4_ns_pollingDone;

static struct psa_invec invecs[1];
static struct psa_outvec outvecs[1];

/* Extern globals */
extern const EDDSA_Params EDDSA_defaultParams;
extern EDDSA_s_SecureCallback eddsaSecureCB_ns[];
extern EDDSACC26X4_ns_Object eddsaObject_ns[];

/*
 *  ======== EDDSA_ns_callbackFxn ========
 */
void EDDSA_ns_callbackFxn(uintptr_t arg)
{
    EDDSA_s_SecureCallback *secureCallbackObject = (EDDSA_s_SecureCallback *)arg;
    uintptr_t index                              = GET_CRYPTO_S_HANDLE_INDEX(secureCallbackObject->handle);

    (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

    CryptoPSACC26X4_releaseLock();

    if (eddsaObject_ns[index].returnBehavior == EDDSA_RETURN_BEHAVIOR_POLLING)
    {
        EDDSACC26X4_ns_pollingDone = true;
    }
    else if (eddsaObject_ns[index].returnBehavior == EDDSA_RETURN_BEHAVIOR_BLOCKING)
    {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&CryptoPSACC26X4_operationSemaphore);
    }
    else
    {
        /* Call the callback function provided by the application. */
        eddsaObject_ns[index].callbackFxn(eddsaSecureCB_ns[index].handle,
                                          eddsaSecureCB_ns[index].returnStatus,
                                          eddsaSecureCB_ns[index].operation,
                                          eddsaSecureCB_ns[index].operationType);
    }
}

/*
 *  ======== EDDSA_ns_registerCallback ========
 */
static psa_status_t EDDSA_ns_registerCallback(EDDSA_Handle handle, const EDDSA_Params *params)
{
    EDDSA_s_CallbackMsg callbackMsg;
    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Construct SecureCB object */
    SecureCallback_construct(&eddsaSecureCB_ns[index].object,
                             EDDSA_ns_callbackFxn,
                             (uintptr_t)&eddsaSecureCB_ns[index]);

    callbackMsg.handle   = handle;
    callbackMsg.callback = &eddsaSecureCB_ns[index];
    invecs[0].base       = &callbackMsg;
    invecs[0].len        = sizeof(callbackMsg);

    /* Setup interface for return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver - assumes CryptoPSACC26X4 lock is already acquired */
    return CryptoPSACC26X4_call(EDDSA_S_MSG_TYPE_REGISTER_CALLBACK, invecs, outvecs);
}

/*
 *  ======== EDDSA_ns_waitForResult ========
 */
static void EDDSA_ns_waitForResult(int_fast16_t *result, uint8_t objectIndex)
{
    if (*result != EDDSA_STATUS_SUCCESS)
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (eddsaObject_ns[objectIndex].returnBehavior == EDDSA_RETURN_BEHAVIOR_POLLING)
    {
        /*
         * Emulate polling mode by spinning on a flag which will be set by
         * the callback function
         */
        while (!EDDSACC26X4_ns_pollingDone) {}
        *result = eddsaSecureCB_ns[objectIndex].returnStatus;
    }
    else if (eddsaObject_ns[objectIndex].returnBehavior == EDDSA_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        *result = eddsaSecureCB_ns[objectIndex].returnStatus;
    }
}

/*
 *  ======== EDDSA_init ========
 */
void EDDSA_init(void)
{
    if (!isInitialized)
    {
        /* Initialize CryptoPSA semaphores and SecureCB driver */
        CryptoPSACC26X4_init();

        isInitialized = true;
    }
}

/*
 *  ======== EDDSA_open ========
 */
EDDSA_Handle EDDSA_open(uint_least8_t index, const EDDSA_Params *params)
{
    EDDSA_Handle handle = NULL;
    EDDSA_s_OpenMsg openMsg;
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
        params = (EDDSA_Params *)&EDDSA_defaultParams;
    }

    DebugP_assert(params->returnBehavior == EDDSA_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(EDDSA_S_MSG_TYPE_OPEN, invecs, outvecs);

    if (handle != NULL)
    {
        /* Register NS callback regardless of return behavior */
        if (EDDSA_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        eddsaObject_ns[index].returnBehavior   = params->returnBehavior;
        eddsaObject_ns[index].callbackFxn      = params->callbackFxn;
        eddsaObject_ns[index].semaphoreTimeout = (params->returnBehavior == EDDSA_RETURN_BEHAVIOR_BLOCKING)
                                                     ? params->timeout
                                                     : SemaphoreP_NO_WAIT;

        /* Set power dependency - i.e. power up and enable clock for PKA */
        (void)Power_setDependency(PowerCC26X2_PERIPH_PKA);

        /* Set power dependency - i.e. power up and enable clock for Crypto.
         * This is required since EdDSA uses SHA-2.
         */
        (void)Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);
    }

    return handle;
}

/*
 *  ======== EDDSA_construct ========
 */
EDDSA_Handle EDDSA_construct(EDDSA_Config *config, const EDDSA_Params *params)
{
    EDDSA_Handle handle = NULL;
    EDDSA_s_ConstructMsg constructMsg;
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
        params = (EDDSA_Params *)&EDDSA_defaultParams;
    }

    DebugP_assert(params->returnBehavior == EDDSA_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(EDDSA_S_MSG_TYPE_CONSTRUCT, invecs, outvecs);

    if (handle != NULL)
    {
        /* Register NS callback regardless of return behavior */
        if (EDDSA_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        eddsaObject_ns[index].returnBehavior   = params->returnBehavior;
        eddsaObject_ns[index].callbackFxn      = params->callbackFxn;
        eddsaObject_ns[index].semaphoreTimeout = (params->returnBehavior == EDDSA_RETURN_BEHAVIOR_BLOCKING)
                                                     ? params->timeout
                                                     : SemaphoreP_NO_WAIT;

        /* Set power dependency - i.e. power up and enable clock for PKA */
        (void)Power_setDependency(PowerCC26X2_PERIPH_PKA);

        /* Set power dependency - i.e. power up and enable clock for Crypto.
         * This is required since EdDSA uses SHA-2.
         */
        (void)Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);
    }

    return handle;
}

/*
 *  ======== EDDSA_close ========
 */
void EDDSA_close(EDDSA_Handle handle)
{
    EDDSA_s_CloseMsg closeMsg;

    DebugP_assert(handle);

    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Destruct SecureCallback object */
    SecureCallback_destruct(&eddsaSecureCB_ns[index].object);

    /* Setup interface for input parameter */
    closeMsg.handle = handle;
    invecs[0].base  = &closeMsg;
    invecs[0].len   = sizeof(closeMsg);

    /* Setup interface for null return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver */
    (void)CryptoPSACC26X4_call(EDDSA_S_MSG_TYPE_CLOSE, invecs, outvecs);

    /* Release power dependencies */
    (void)Power_releaseDependency(PowerCC26X2_PERIPH_PKA);
    (void)Power_releaseDependency(PowerCC26XX_PERIPH_CRYPTO);
}

/*
 *  ======== EDDSA_generatePublicKey ========
 */
int_fast16_t EDDSA_generatePublicKey(EDDSA_Handle handle, EDDSA_OperationGeneratePublicKey *operation)
{
    EDDSA_s_GenPublicKeyMsg genKeyMsg;
    int_fast16_t result = EDDSA_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    DebugP_assert(params->returnBehavior == EDDSA_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

    /* Setup interface for input parameters */
    genKeyMsg.handle    = handle;
    genKeyMsg.operation = operation;
    invecs[0].base      = &genKeyMsg;
    invecs[0].len       = sizeof(genKeyMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(eddsaObject_ns[index].semaphoreTimeout) == false)
    {
        return EDDSA_STATUS_PKA_UNAVAILABLE;
    }

    EDDSACC26X4_ns_pollingDone = false;

    (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to EDDSA_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(EDDSA_S_MSG_TYPE_GEN_PUBLIC_KEY, invecs, outvecs);

    EDDSA_ns_waitForResult(&result, index);

    return (result);
}

/*
 *  ======== EDDSA_sign ========
 */
int_fast16_t EDDSA_sign(EDDSA_Handle handle, EDDSA_OperationSign *operation)
{
    EDDSA_s_SignMsg signMsg;
    int_fast16_t result = EDDSA_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    DebugP_assert(params->returnBehavior == EDDSA_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

    /* Setup interface for input parameters */
    signMsg.handle    = handle;
    signMsg.operation = operation;
    invecs[0].base    = &signMsg;
    invecs[0].len     = sizeof(signMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(eddsaObject_ns[index].semaphoreTimeout) == false)
    {
        return EDDSA_STATUS_PKA_UNAVAILABLE;
    }

    EDDSACC26X4_ns_pollingDone = false;

    (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to EDDSA_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(EDDSA_S_MSG_TYPE_SIGN, invecs, outvecs);

    EDDSA_ns_waitForResult(&result, index);

    return (result);
}

/*
 *  ======== EDDSA_verify ========
 */
int_fast16_t EDDSA_verify(EDDSA_Handle handle, EDDSA_OperationVerify *operation)
{
    EDDSA_s_VerifyMsg verifyMsg;
    int_fast16_t result;
    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    DebugP_assert(params->returnBehavior == EDDSA_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

    /* Setup interface for input parameters */
    verifyMsg.handle    = handle;
    verifyMsg.operation = operation;
    invecs[0].base      = &verifyMsg;
    invecs[0].len       = sizeof(verifyMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(eddsaObject_ns[index].semaphoreTimeout) == false)
    {
        return EDDSA_STATUS_PKA_UNAVAILABLE;
    }

    EDDSACC26X4_ns_pollingDone = false;

    (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to EDDSA_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(EDDSA_S_MSG_TYPE_VERIFY, invecs, outvecs);

    EDDSA_ns_waitForResult(&result, index);

    return (result);
}

/*
 *  ======== EDDSA_cancelOperation ========
 */
int_fast16_t EDDSA_cancelOperation(EDDSA_Handle handle)
{
    EDDSA_s_CancelOperationMsg cancelMsg;
    int_fast16_t result = EDDSA_STATUS_ERROR;

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
     * Return value can be ignored since result (in outvecs) is initialized to EDDSA_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(EDDSA_S_MSG_TYPE_CANCEL_OPERATION, invecs, outvecs);

    return (result);
}