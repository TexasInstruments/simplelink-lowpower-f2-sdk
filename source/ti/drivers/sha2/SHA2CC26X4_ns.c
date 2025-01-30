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
#include <ti/drivers/power/PowerCC26X2.h>
#include <ti/drivers/sha2/SHA2CC26X4_ns.h>
#include <ti/drivers/sha2/SHA2CC26X4_s.h>

#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include <psa/client.h>

/* Static globals */
static bool isInitialized = false;

static struct psa_invec invecs[1];
static struct psa_outvec outvecs[1];

extern SHA2_s_SecureCallback sha2SecureCB_ns[];
extern SHA2CC26X4_ns_Object sha2Object_ns[];

/*
 *  ======== SHA2_ns_callbackFxn ========
 */
void SHA2_ns_callbackFxn(uintptr_t arg)
{
    SHA2_s_SecureCallback *secureCallbackObject = (SHA2_s_SecureCallback *)arg;
    uintptr_t index                             = GET_CRYPTO_S_HANDLE_INDEX(secureCallbackObject->handle);

    (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
    CryptoPSACC26X4_releaseLock();

    if (sha2Object_ns[index].returnBehavior == SHA2_RETURN_BEHAVIOR_BLOCKING)
    {
        /* Unblock the pending task to signal that the operation is complete. */
        SemaphoreP_post(&CryptoPSACC26X4_operationSemaphore);
    }
    else if (sha2Object_ns[index].returnBehavior == SHA2_RETURN_BEHAVIOR_CALLBACK)
    {
        /* Call the callback function provided by the application. */
        sha2Object_ns[index].callbackFxn(sha2SecureCB_ns[index].handle, sha2SecureCB_ns[index].returnStatus);
    }
}

/*
 *  ======== SHA2_ns_registerCallback ========
 */
static psa_status_t SHA2_ns_registerCallback(SHA2_Handle handle, const SHA2_Params *params)
{
    SHA2_s_CallbackMsg callbackMsg;
    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Construct SecureCB object */
    SecureCallback_construct(&sha2SecureCB_ns[index].object, SHA2_ns_callbackFxn, (uintptr_t)&sha2SecureCB_ns[index]);

    callbackMsg.handle   = handle;
    callbackMsg.callback = &sha2SecureCB_ns[index];
    invecs[0].base       = &callbackMsg;
    invecs[0].len        = sizeof(callbackMsg);

    /* Setup interface for return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver - assumes CryptoPSACC26X4 lock is already acquired */
    return CryptoPSACC26X4_call(SHA2_S_MSG_TYPE_REGISTER_CALLBACK, invecs, outvecs);
}

/*
 *  ======== SHA2_init ========
 */
void SHA2_init(void)
{
    if (!isInitialized)
    {
        /* Initialize CryptoPSA semaphores and SecureCB driver */
        CryptoPSACC26X4_init();

        isInitialized = true;
    }
}

/*
 *  ======== SHA2_open ========
 */
SHA2_Handle SHA2_open(uint_least8_t index, const SHA2_Params *params)
{
    SHA2_Handle handle = NULL;
    SHA2_s_OpenMsg openMsg;
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
        params = (SHA2_Params *)&SHA2_defaultParams;
    }

    DebugP_assert(params->returnBehavior == SHA2_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(SHA2_S_MSG_TYPE_OPEN, invecs, outvecs);

    if ((handle != NULL) && (params->returnBehavior != SHA2_RETURN_BEHAVIOR_POLLING))
    {
        if (SHA2_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        sha2Object_ns[index].returnBehavior   = params->returnBehavior;
        sha2Object_ns[index].callbackFxn      = params->callbackFxn;
        sha2Object_ns[index].semaphoreTimeout = params->returnBehavior == SHA2_RETURN_BEHAVIOR_BLOCKING
                                                    ? params->timeout
                                                    : SemaphoreP_NO_WAIT;
        /* Set power dependency */
        (void)Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);
    }

    return handle;
}

/*
 *  ======== SHA2_construct ========
 */
SHA2_Handle SHA2_construct(SHA2_Config *config, const SHA2_Params *params)
{
    SHA2_Handle handle = NULL;
    SHA2_s_ConstructMsg constructMsg;
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
        params = (SHA2_Params *)&SHA2_defaultParams;
    }

    DebugP_assert(params->returnBehavior == SHA2_RETURN_BEHAVIOR_CALLBACK ? params->callbackFxn : true);

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
    (void)CryptoPSACC26X4_call(SHA2_S_MSG_TYPE_CONSTRUCT, invecs, outvecs);

    if ((handle != NULL) && (params->returnBehavior != SHA2_RETURN_BEHAVIOR_POLLING))
    {
        if (SHA2_ns_registerCallback(handle, params) != PSA_SUCCESS)
        {
            handle = NULL;
        }
    }

    if (handle != NULL)
    {
        uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

        /* Store NS handle object */
        sha2Object_ns[index].returnBehavior   = params->returnBehavior;
        sha2Object_ns[index].callbackFxn      = params->callbackFxn;
        sha2Object_ns[index].semaphoreTimeout = params->returnBehavior == SHA2_RETURN_BEHAVIOR_BLOCKING
                                                    ? params->timeout
                                                    : SemaphoreP_NO_WAIT;

        /* Set power dependency */
        (void)Power_setDependency(PowerCC26XX_PERIPH_CRYPTO);
    }

    return handle;
}

/*
 *  ======== SHA2_close ========
 */
void SHA2_close(SHA2_Handle handle)
{
    SHA2_s_CloseMsg closeMsg;

    DebugP_assert(handle);

    uintptr_t index = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Destruct SecureCallback object */
    SecureCallback_destruct(&sha2SecureCB_ns[index].object);

    /* Setup interface for input parameter */
    closeMsg.handle = handle;
    invecs[0].base  = &closeMsg;
    invecs[0].len   = sizeof(closeMsg);

    /* Setup interface for null return value */
    outvecs[0].base = NULL;
    outvecs[0].len  = 0;

    /* PSA call to secure driver */
    (void)CryptoPSACC26X4_call(SHA2_S_MSG_TYPE_CLOSE, invecs, outvecs);

    /* Release power dependency */
    (void)Power_releaseDependency(PowerCC26XX_PERIPH_CRYPTO);
}

/*
 *  ======== SHA2_addData ========
 */
int_fast16_t SHA2_addData(SHA2_Handle handle, const void *data, size_t length)
{
    SHA2_s_AddDataMsg addDataMsg;
    int_fast16_t result = SHA2_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Setup interface for input parameters */
    addDataMsg.handle = handle;
    addDataMsg.data   = data;
    addDataMsg.length = length;
    invecs[0].base    = &addDataMsg;
    invecs[0].len     = sizeof(addDataMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(sha2Object_ns[index].semaphoreTimeout) == false)
    {
        return SHA2_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (sha2Object_ns[index].returnBehavior != SHA2_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to SHA2_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(SHA2_S_MSG_TYPE_ADD_DATA, invecs, outvecs);

    if ((sha2Object_ns[index].returnBehavior != SHA2_RETURN_BEHAVIOR_POLLING) && (result != SHA2_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (sha2Object_ns[index].returnBehavior == SHA2_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (sha2Object_ns[index].returnBehavior == SHA2_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = sha2SecureCB_ns[index].returnStatus;
    }

    return (result);
}

/*
 *  ======== SHA2_finalize ========
 */
int_fast16_t SHA2_finalize(SHA2_Handle handle, void *digest)
{
    SHA2_s_FinalizeMsg finalizeMsg;
    int_fast16_t result = SHA2_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Setup interface for input parameters */
    finalizeMsg.handle       = handle;
    finalizeMsg.digestOrHmac = digest;
    invecs[0].base           = &finalizeMsg;
    invecs[0].len            = sizeof(finalizeMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(sha2Object_ns[index].semaphoreTimeout) == false)
    {
        return SHA2_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (sha2Object_ns[index].returnBehavior != SHA2_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to SHA2_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(SHA2_S_MSG_TYPE_FINALIZE, invecs, outvecs);

    if ((sha2Object_ns[index].returnBehavior != SHA2_RETURN_BEHAVIOR_POLLING) && (result != SHA2_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (sha2Object_ns[index].returnBehavior == SHA2_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (sha2Object_ns[index].returnBehavior == SHA2_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = sha2SecureCB_ns[index].returnStatus;
    }

    return (result);
}

/*
 *  ======== SHA2_hashData ========
 */
int_fast16_t SHA2_hashData(SHA2_Handle handle, const void *data, size_t length, void *digest)
{
    SHA2_s_HashDataMsg hashDataMsg;
    int_fast16_t result = SHA2_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Setup interface for input parameters */
    hashDataMsg.handle     = handle;
    hashDataMsg.data       = data;
    hashDataMsg.dataLength = length;
    hashDataMsg.digest     = digest;
    invecs[0].base         = &hashDataMsg;
    invecs[0].len          = sizeof(hashDataMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(sha2Object_ns[index].semaphoreTimeout) == false)
    {
        return SHA2_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (sha2Object_ns[index].returnBehavior != SHA2_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to SHA2_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(SHA2_S_MSG_TYPE_HASH_DATA, invecs, outvecs);

    if ((sha2Object_ns[index].returnBehavior != SHA2_RETURN_BEHAVIOR_POLLING) && (result != SHA2_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (sha2Object_ns[index].returnBehavior == SHA2_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (sha2Object_ns[index].returnBehavior == SHA2_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = sha2SecureCB_ns[index].returnStatus;
    }

    return (result);
}

/*
 *  ======== SHA2_setupHmac ========
 */
int_fast16_t SHA2_setupHmac(SHA2_Handle handle, const CryptoKey *key)
{
    SHA2_s_SetupHmacMsg setupHmacMsg;
    int_fast16_t result = SHA2_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Setup interface for input parameters */
    setupHmacMsg.handle = handle;
    setupHmacMsg.key    = key;
    invecs[0].base      = &setupHmacMsg;
    invecs[0].len       = sizeof(setupHmacMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(sha2Object_ns[index].semaphoreTimeout) == false)
    {
        return SHA2_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (sha2Object_ns[index].returnBehavior != SHA2_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to SHA2_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(SHA2_S_MSG_TYPE_SETUP_HMAC, invecs, outvecs);

    if ((sha2Object_ns[index].returnBehavior != SHA2_RETURN_BEHAVIOR_POLLING) && (result != SHA2_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (sha2Object_ns[index].returnBehavior == SHA2_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (sha2Object_ns[index].returnBehavior == SHA2_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = sha2SecureCB_ns[index].returnStatus;
    }

    return (result);
}

/*
 *  ======== SHA2_finalizeHmac ========
 */
int_fast16_t SHA2_finalizeHmac(SHA2_Handle handle, void *hmac)
{
    SHA2_s_FinalizeMsg finalizeMsg;
    int_fast16_t result = SHA2_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Setup interface for input parameters */
    finalizeMsg.handle       = handle;
    finalizeMsg.digestOrHmac = hmac;
    invecs[0].base           = &finalizeMsg;
    invecs[0].len            = sizeof(finalizeMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(sha2Object_ns[index].semaphoreTimeout) == false)
    {
        return SHA2_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (sha2Object_ns[index].returnBehavior != SHA2_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to SHA2_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(SHA2_S_MSG_TYPE_FINALIZE_HMAC, invecs, outvecs);

    if ((sha2Object_ns[index].returnBehavior != SHA2_RETURN_BEHAVIOR_POLLING) && (result != SHA2_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (sha2Object_ns[index].returnBehavior == SHA2_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (sha2Object_ns[index].returnBehavior == SHA2_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = sha2SecureCB_ns[index].returnStatus;
    }

    return (result);
}

/*
 *  ======== SHA2_hmac ========
 */
int_fast16_t SHA2_hmac(SHA2_Handle handle, const CryptoKey *key, const void *data, size_t size, void *hmac)
{
    SHA2_s_HmacMsg hmacMsg;
    int_fast16_t result = SHA2_STATUS_ERROR;
    uintptr_t index     = GET_CRYPTO_S_HANDLE_INDEX(handle);

    /* Setup interface for input parameters */
    hmacMsg.handle     = handle;
    hmacMsg.key        = key;
    hmacMsg.data       = data;
    hmacMsg.dataLength = size;
    hmacMsg.hmac       = hmac;
    invecs[0].base     = &hmacMsg;
    invecs[0].len      = sizeof(hmacMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* Try to get semaphore for Crypto PSA access */
    if (CryptoPSACC26X4_acquireLock(sha2Object_ns[index].semaphoreTimeout) == false)
    {
        return SHA2_STATUS_RESOURCE_UNAVAILABLE;
    }

    if (sha2Object_ns[index].returnBehavior != SHA2_RETURN_BEHAVIOR_POLLING)
    {
        (void)Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to SHA2_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(SHA2_S_MSG_TYPE_HMAC, invecs, outvecs);

    if ((sha2Object_ns[index].returnBehavior != SHA2_RETURN_BEHAVIOR_POLLING) && (result != SHA2_STATUS_SUCCESS))
    {
        (void)Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
        CryptoPSACC26X4_releaseLock();
    }
    else if (sha2Object_ns[index].returnBehavior == SHA2_RETURN_BEHAVIOR_POLLING)
    {
        CryptoPSACC26X4_releaseLock();
    }
    else if (sha2Object_ns[index].returnBehavior == SHA2_RETURN_BEHAVIOR_BLOCKING)
    {
        SemaphoreP_pend(&CryptoPSACC26X4_operationSemaphore, SemaphoreP_WAIT_FOREVER);
        result = sha2SecureCB_ns[index].returnStatus;
    }

    return (result);
}

/*
 *  ======== SHA2_reset ========
 */
void SHA2_reset(SHA2_Handle handle)
{
    SHA2_s_ResetMsg resetMsg;
    int_fast16_t result = SHA2_STATUS_ERROR;

    /* Setup interface for input parameters */
    resetMsg.handle = handle;
    invecs[0].base  = &resetMsg;
    invecs[0].len   = sizeof(resetMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /* PSA call to secure driver */
    (void)CryptoPSACC26X4_call(SHA2_S_MSG_TYPE_RESET, invecs, outvecs);
}

/*
 *  ======== SHA2_cancelOperation ========
 */
int_fast16_t SHA2_cancelOperation(SHA2_Handle handle)
{
    SHA2_s_CancelOperationMsg cancelMsg;
    int_fast16_t result = SHA2_STATUS_ERROR;

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
     * Return value can be ignored since result (in outvecs) is initialized to SHA2_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(SHA2_S_MSG_TYPE_CANCEL_OPERATION, invecs, outvecs);

    return (result);
}

int_fast16_t SHA2_setHashType(SHA2_Handle handle, SHA2_HashType type)
{
    SHA2_s_SetHashTypeMsg setHashTypeMsg;
    int_fast16_t result = SHA2_STATUS_ERROR;

    /* Setup interface for input parameters */
    setHashTypeMsg.handle = handle;
    setHashTypeMsg.type   = type;
    invecs[0].base        = &setHashTypeMsg;
    invecs[0].len         = sizeof(setHashTypeMsg);

    /* Setup interface for return value */
    outvecs[0].base = &result;
    outvecs[0].len  = sizeof(result);

    /*
     * PSA call to secure driver:
     *
     * Return value can be ignored since result (in outvecs) is initialized to SHA2_STATUS_ERROR
     * and will only be updated if the PSA call is successful.
     */
    (void)CryptoPSACC26X4_call(SHA2_S_MSG_TYPE_SET_HASH_TYPE, invecs, outvecs);

    return (result);
}
