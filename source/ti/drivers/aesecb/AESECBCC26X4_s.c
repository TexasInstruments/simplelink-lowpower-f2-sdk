/*
 * Copyright (c) 2022-2024, Texas Instruments Incorporated - http://www.ti.com
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

#include <stdbool.h>
#include <stdint.h>

#include "AESECBCC26X4_s.h"

#include <ti/drivers/AESECB.h>
#include <ti/drivers/aesecb/AESECBCC26XX.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#include <ti/drivers/tfm/SecureCallback.h>

#include <third_party/tfm/secure_fw/spm/core/spm.h>
#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/interface/include/psa/service.h>
#include <third_party/tfm/secure_fw/spm/include/utilities.h>

#include <third_party/tfm/platform/ext/target/ti/cc26x4/cmse.h> /* TI CMSE helper functions */
#include "ti_drivers_config.h"                                  /* Sysconfig generated header */

/*
 * Stores a secure copy of the operation and the original pointer to the
 * non-secure operation to return in case of callback return behavior.
 */
typedef struct
{
    AESECB_Operation *operation_ns; /* Pointer to non-secure operation */
    AESECB_Operation operation_s;   /* Secure copy of operation */
} AESECB_s_Operation;

static AESECB_s_Operation AESECB_s_operation;

/*
 * AES ECB Secure Dynamic Instance struct.
 */
typedef struct
{
    AESECB_Config config;
    AESECBCC26XX_Object object;
    AESECBCC26XX_HWAttrs hwAttrs;
} AESECB_s_DynamicInstance;

/*
 * Used to store a secure copy of the dynamic instance (config plus the
 * object and hwAttrs that it points to) provided by non-secure calls to
 * AESECB_construct.
 */
static AESECB_s_DynamicInstance AESECB_s_dynInstance[CONFIG_AESECB_S_CONFIG_POOL_SIZE];

/* Stores pointers to non-secure AESECB_s_SecureCallbacks for each driver instance opened or constructed */
static AESECB_s_SecureCallback *AESECB_s_secureCB[AESECB_SECURE_CALLBACK_COUNT];

/* Static driver instances allocated by SysConfig */
extern const AESECB_Config AESECB_config[];

/*
 *  ======== AESECB_s_getCallbackIndex ========
 *  Returns callback index or -1 if index is not found.
 */
static int8_t AESECB_s_getCallbackIndex(AESECB_Handle handle_s)
{
    uint_fast8_t index;
    int8_t retIndex = -1;
    bool indexFound = false;

    /* First, search config statically allocated by SysConfig */
    for (index = 0; index < CONFIG_TI_DRIVERS_AESECB_COUNT; index++)
    {
        if (handle_s == (AESECB_Handle)&AESECB_config[index])
        {
            indexFound = true;
            break;
        }
    }

    /* If not found, search secure config copies */
    if (!indexFound)
    {
        for (index = 0; index < CONFIG_AESECB_S_CONFIG_POOL_SIZE; index++)
        {
            if (handle_s == &AESECB_s_dynInstance[index].config)
            {
                index += CONFIG_TI_DRIVERS_AESECB_COUNT;
                indexFound = true;
                break;
            }
        }
    }

    if (indexFound)
    {
        retIndex = (int8_t)index;
    }

    return retIndex;
}

/*
 *  ======== AESECB_s_hwiCallback ========
 */
static void AESECB_s_hwiCallback(AESECB_Handle handle_s,
                                 int_fast16_t returnValue,
                                 AESECB_Operation *operation,
                                 AESECB_OperationType operationType)
{
    int8_t index;
    AESECB_s_SecureCallback *aesecbSecureCB_ns;

    index = AESECB_s_getCallbackIndex(handle_s);

    if ((index >= 0) && (index < AESECB_SECURE_CALLBACK_COUNT))
    {
        aesecbSecureCB_ns = AESECB_s_secureCB[index];

        if (aesecbSecureCB_ns != NULL)
        {
            /* Store arguments in callback object for use by non-secure handler */
            aesecbSecureCB_ns->handle        = (AESECB_Handle)(CRYPTO_S_HANDLE_ID_AESECB | index);
            aesecbSecureCB_ns->returnValue   = returnValue;
            aesecbSecureCB_ns->operation     = AESECB_s_operation.operation_ns;
            aesecbSecureCB_ns->operationType = operationType;

            /* Trigger the interrupt for the non-secure callback dispatcher */
            SecureCallback_post(&aesecbSecureCB_ns->object);
        }
    }
}

/*
 *  ======== AESECB_s_copyConfig ========
 */
static inline psa_status_t AESECB_s_copyConfig(AESECB_Config **secureConfig,
                                               const AESECB_Config *config,
                                               AESECB_Handle *retHandle)
{
    AESECB_Config *config_s;
    AESECB_s_DynamicInstance *dynInstance_s;
    uint_fast8_t i;

    for (i = 0; i < CONFIG_AESECB_S_CONFIG_POOL_SIZE; i++)
    {
        dynInstance_s = &AESECB_s_dynInstance[i];
        config_s      = &dynInstance_s->config;

        if (config_s->object == NULL)
        {
            /* Validate config address range */
            if (cmse_has_unpriv_nonsecure_read_access((void *)config, sizeof(dynInstance_s->config)) == NULL)
            {
                return PSA_ERROR_PROGRAMMER_ERROR;
            }

            /* Copy config to secure memory */
            (void)spm_memcpy(config_s, config, sizeof(dynInstance_s->config));

            /* Validate object address range */
            if (cmse_has_unpriv_nonsecure_read_access(config_s->object, sizeof(dynInstance_s->object)) == NULL)
            {
                return PSA_ERROR_PROGRAMMER_ERROR;
            }

            /* Copy object to secure memory and point config to it */
            (void)spm_memcpy(&dynInstance_s->object, config_s->object, sizeof(dynInstance_s->object));
            config_s->object = &dynInstance_s->object;

            /* Validate HW attributes address range */
            if (cmse_has_unpriv_nonsecure_read_access((void *)config_s->hwAttrs, sizeof(dynInstance_s->hwAttrs)) ==
                NULL)
            {
                return PSA_ERROR_PROGRAMMER_ERROR;
            }

            /* Copy HW attributes to secure memory and point config to it */
            (void)spm_memcpy(&dynInstance_s->hwAttrs, config_s->hwAttrs, sizeof(dynInstance_s->hwAttrs));
            config_s->hwAttrs = &dynInstance_s->hwAttrs;

            *secureConfig = config_s;

            /*
             * Set the secure callback object pointer to NULL. The non-secure
             * client must register a secure callback after constructing or
             * opening a driver instance.
             */
            AESECB_s_secureCB[i + CONFIG_TI_DRIVERS_AESECB_COUNT] = NULL;

            /*
             * Return handle is the CRYPTO_S_HANDLE_ID_AESECB OR'd with the
             * the config pool array index plus the size of constant config
             * array created by Sysconfig.
             */
            *retHandle = (AESECB_Handle)(CRYPTO_S_HANDLE_ID_AESECB | (i + CONFIG_TI_DRIVERS_AESECB_COUNT));
            return PSA_SUCCESS;
        }
    }

    return PSA_ERROR_INSUFFICIENT_MEMORY;
}

/*
 *  ======== AESECB_s_releaseConfig ========
 */
static inline void AESECB_s_releaseConfig(AESECB_Handle nsHandle)
{
    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_AESECB)
    {
        /* Extract the secure handle index */
        uintptr_t i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        /* Check whether the handle instance refers to a dynamic instance */
        if ((i >= CONFIG_TI_DRIVERS_AESECB_COUNT) && (i < AESECB_SECURE_CALLBACK_COUNT))
        {
            /* Set config's object pointer to NULL to indicate the config is available */
            AESECB_s_dynInstance[i - CONFIG_TI_DRIVERS_AESECB_COUNT].config.object = NULL;
        }
    }
}

/*
 *  ======== AESECB_s_copyOperation ========
 *  Set secureKey to NULL for segmented operations.
 */
static inline psa_status_t AESECB_s_copyOperation(AESECB_Operation *secureOperation,
                                                  CryptoKey *secureKey,
                                                  const AESECB_Operation *operation)
{
    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(AESECB_Operation)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(AESECB_Operation));

    /*
     * Crypto key member of the AESECB_Operation is used for one-step operations
     * only.
     */
    if (secureKey != NULL)
    {
        /* Validate crypto key struct address range */
        if (cmse_has_unpriv_nonsecure_read_access((void *)secureOperation->key, sizeof(CryptoKey)) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /*
         * Make a secure copy of the key struct and update the operation
         * struct to point to the secure key copy.
         */
        (void)spm_memcpy(secureKey, secureOperation->key, sizeof(CryptoKey));

        if (CryptoKey_verifySecureInputKey(secureKey) != CryptoKey_STATUS_SUCCESS)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        secureOperation->key = secureKey;
    }

    /* Segmented operations may be finalized with or without additional data */
    if (secureOperation->inputLength > 0)
    {
        /* Verify input address range */
        if (cmse_has_unpriv_nonsecure_read_access((void *)secureOperation->input, secureOperation->inputLength) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Verify output address range */
        if (cmse_has_unpriv_nonsecure_rw_access(secureOperation->output, secureOperation->inputLength) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }
    }

    return PSA_SUCCESS;
}

/*
 *  ======== AESECB_s_copyParams ========
 */
static psa_status_t AESECB_s_copyParams(AESECB_Params *secureParams, const AESECB_Params *params)
{
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Validate params address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)params, sizeof(AESECB_Params)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    (void)spm_memcpy(secureParams, params, sizeof(AESECB_Params));

    /* Validate the return behavior */
    if ((secureParams->returnBehavior == AESECB_RETURN_BEHAVIOR_CALLBACK) ||
        (secureParams->returnBehavior == AESECB_RETURN_BEHAVIOR_BLOCKING) ||
        (secureParams->returnBehavior == AESECB_RETURN_BEHAVIOR_POLLING))
    {
        if (secureParams->returnBehavior != AESECB_RETURN_BEHAVIOR_POLLING)
        {
            /*
             * Overwrite the non-secure client's callback function with our own
             * callback which will populate the secure callback object registered
             * using AESECB_S_MSG_TYPE_REGISTER_CALLBACK.
             */
            secureParams->callbackFxn = AESECB_s_hwiCallback;

            /* Force to callback return behavior */
            secureParams->returnBehavior = AESECB_RETURN_BEHAVIOR_CALLBACK;
        }

        status = PSA_SUCCESS;
    }

    return status;
}

/*
 *  ======== AESECB_s_getHandle ========
 *  Returns a Secure handle from a handle provided from non-secure client.
 */
static AESECB_Handle AESECB_s_getHandle(AESECB_Handle nsHandle)
{
    AESECB_Handle secureHandle = NULL;
    uint32_t i;

    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_AESECB)
    {
        /* Extract the secure handle index */
        i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        if (i < CONFIG_TI_DRIVERS_AESECB_COUNT)
        {
            secureHandle = (AESECB_Handle)&AESECB_config[i];
        }
        else if ((i >= CONFIG_TI_DRIVERS_AESECB_COUNT) && (i < AESECB_SECURE_CALLBACK_COUNT))
        {
            secureHandle = &AESECB_s_dynInstance[i - CONFIG_TI_DRIVERS_AESECB_COUNT].config;
        }
    }

    return secureHandle;
}

/*
 *  ======== AESECB_s_registerCallback ========
 */
static inline psa_status_t AESECB_s_registerCallback(psa_msg_t *msg)
{
    AESECB_Handle handle_s;
    AESECB_s_CallbackMsg callbackMsg;
    int8_t callbackIndex;
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Only non-secure callers should be registering callbacks */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id) && (msg->in_size[0] == sizeof(callbackMsg)))
    {
        psa_read(msg->handle, 0, &callbackMsg, sizeof(callbackMsg));

        handle_s = AESECB_s_getHandle(callbackMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        callbackIndex = AESECB_s_getCallbackIndex(handle_s);

        /* Validate index and callback object address range */
        if ((callbackIndex >= 0) && (callbackIndex < AESECB_SECURE_CALLBACK_COUNT) &&
            (cmse_has_unpriv_nonsecure_rw_access(callbackMsg.callback, sizeof(AESECB_s_SecureCallback)) != NULL))
        {
            /*
             * Store the pointer to AESECB_s_SecureCallback located in
             * non-secure memory.
             */
            AESECB_s_secureCB[callbackIndex] = callbackMsg.callback;

            status = PSA_SUCCESS;
        }
    }

    return status;
}

/*
 *  ======== AESECB_s_construct ========
 */
static inline psa_status_t AESECB_s_construct(psa_msg_t *msg)
{
    AESECB_s_ConstructMsg constructMsg;
    AESECB_Handle handle;
    AESECB_Params params_s;
    const AESECB_Params *paramsPtr_s = NULL;
    AESECB_Config *configPtr_s;
    psa_status_t status     = PSA_ERROR_PROGRAMMER_ERROR;
    AESECB_Handle retHandle = NULL;

    if ((msg->in_size[0] != sizeof(constructMsg)) || (msg->out_size[0] != sizeof(handle)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &constructMsg, sizeof(constructMsg));

    /*
     * For non-secure callers, the params and config must be verified and
     * copied to secure memory. Assume secure callers are providing valid
     * inputs located in secure memory which are accessible by this partition
     * for the TF-M isolation level in use. At isolation levels 2 & 3, this
     * means the params and config data may need to be defined within the
     * same partition as the crypto driver.
     *
     * Non-secure callers have negative client ID.
     */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        if (constructMsg.params != NULL)
        {
            /*
             * Copy params to secure memory and substitute our own callback
             * if callback return behavior is specified.
             */
            status = AESECB_s_copyParams(&params_s, constructMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }

            paramsPtr_s = &params_s;
        }

        /* Verify config and copy to secure memory */
        status = AESECB_s_copyConfig(&configPtr_s, constructMsg.config, &retHandle);
        if (status != PSA_SUCCESS)
        {
            return status;
        }
    }
    else /* Secure client */
    {
        configPtr_s = constructMsg.config;
        paramsPtr_s = constructMsg.params;
    }

    handle = AESECB_construct(configPtr_s, paramsPtr_s);

    if (handle == NULL)
    {
        retHandle = NULL;

        if (TFM_CLIENT_ID_IS_NS(msg->client_id))
        {
            /* Set config's object pointer to NULL to indicate the config is available */
            configPtr_s->object = NULL;
        }
    }
    else if (!TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        /*
         * Return the pointer to the secure config struct provided by the
         * secure caller.
         */
        retHandle = handle;
    }

    psa_write(msg->handle, 0, &retHandle, sizeof(retHandle));

    return PSA_SUCCESS;
}

/*
 *  ======== AESECB_s_open ========
 */
static inline psa_status_t AESECB_s_open(psa_msg_t *msg)
{
    AESECB_s_OpenMsg openMsg;
    AESECB_Handle handle;
    AESECB_Params params_s;
    AESECB_Params *paramsPtr_s = NULL;
    AESECB_Handle retHandle    = NULL;
    psa_status_t status;

    if ((msg->in_size[0] != sizeof(openMsg)) || (msg->out_size[0] != sizeof(handle)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &openMsg, sizeof(openMsg));

    if (openMsg.params != NULL)
    {
        /* Non-secure callers have negative client ID */
        if (TFM_CLIENT_ID_IS_NS(msg->client_id))
        {
            status = AESECB_s_copyParams(&params_s, openMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }
        }

        paramsPtr_s = &params_s;
    }

    handle = AESECB_open(openMsg.index, paramsPtr_s);

    if (handle != NULL)
    {
        if (TFM_CLIENT_ID_IS_NS(msg->client_id))
        {
            /*
             * Set the secure callback object pointer to NULL. The non-secure
             * client must register a secure callback after constructing or
             * opening a driver instance.
             */
            AESECB_s_secureCB[openMsg.index] = NULL;

            /*
             * Return CRYPTO_S_HANDLE_ID_AESECB OR'd with the config array index
             * as the handle instead of a pointer to the config to allow for
             * validation of handles provided by non-secure clients.
             */
            retHandle = (AESECB_Handle)(CRYPTO_S_HANDLE_ID_AESECB | openMsg.index);
        }
        else /* Secure client */
        {
            retHandle = handle;
        }
    }

    psa_write(msg->handle, 0, &retHandle, sizeof(retHandle));

    return PSA_SUCCESS;
}

/*
 *  ======== AESECB_s_close ========
 */
static inline psa_status_t AESECB_s_close(psa_msg_t *msg)
{
    AESECB_Handle handle_s;
    AESECB_s_CloseMsg closeMsg;

    if (msg->in_size[0] != sizeof(closeMsg))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &closeMsg, sizeof(closeMsg));

    /* Non-secure callers have negative client ID */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESECB_s_getHandle(closeMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        AESECB_close(handle_s);

        /* Release the secure config if it is a dynamic instance */
        AESECB_s_releaseConfig(closeMsg.handle);
    }
    else /* Secure client */
    {
        AESECB_close(closeMsg.handle);
    }

    return PSA_SUCCESS;
}

/*
 *  ======== AESECB_s_startOperation ========
 */
static inline psa_status_t AESECB_s_startOperation(psa_msg_t *msg, int32_t msgType)
{
    AESECB_s_OperationMsg opMsg;
    AESECB_Handle handle_s;
    AESECB_Operation *operation_s;
    CryptoKey key_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(opMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &opMsg, sizeof(opMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESECB_s_getHandle(opMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &AESECB_s_operation.operation_s;

        /* Save pointer to non-secure operation struct */
        AESECB_s_operation.operation_ns = opMsg.operation;

        /* Validate and copy operation */
        status = AESECB_s_copyOperation(operation_s, &key_s, opMsg.operation);
        if (status != PSA_SUCCESS)
        {
            return status;
        }
    }
    else /* Secure client */
    {
        handle_s    = opMsg.handle;
        operation_s = opMsg.operation;
    }

    if (msgType == AESECB_S_MSG_TYPE_ONE_STEP_ENCRYPT)
    {
        ret = AESECB_oneStepEncrypt(handle_s, operation_s);
    }
    else if (msgType == AESECB_S_MSG_TYPE_ONE_STEP_DECRYPT)
    {
        ret = AESECB_oneStepDecrypt(handle_s, operation_s);
    }
    else if (msgType == AESECB_S_MSG_TYPE_ADD_DATA)
    {
        ret = AESECB_addData(handle_s, operation_s);
    }
    else /* AESECB_S_MSG_TYPE_FINALIZE */
    {
        ret = AESECB_finalize(handle_s, operation_s);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== AESECB_s_setupOperation ========
 */
static inline psa_status_t AESECB_s_setupOperation(psa_msg_t *msg, int32_t msgType)
{
    AESECB_s_SetupOperationMsg setupMsg;
    AESECB_Handle handle_s;
    CryptoKey key_s;
    const CryptoKey *keyPtr_s;
    int_fast16_t ret;

    if ((msg->in_size[0] != sizeof(setupMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &setupMsg, sizeof(setupMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESECB_s_getHandle(setupMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Validate crypto key struct address range */
        if (cmse_has_unpriv_nonsecure_read_access((void *)setupMsg.key, sizeof(CryptoKey)) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Copy key to secure memory */
        (void)spm_memcpy(&key_s, setupMsg.key, sizeof(CryptoKey));

        if (CryptoKey_verifySecureInputKey(&key_s) != CryptoKey_STATUS_SUCCESS)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        keyPtr_s = &key_s;
    }
    else /* Secure client */
    {
        handle_s = setupMsg.handle;
        keyPtr_s = setupMsg.key;
    }

    if (msgType == AESECB_S_MSG_TYPE_SETUP_ENCRYPT)
    {
        ret = AESECB_setupEncrypt(handle_s, keyPtr_s);
    }
    else
    {
        ret = AESECB_setupDecrypt(handle_s, keyPtr_s);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== AESECB_s_cancelOperation ========
 */
static inline psa_status_t AESECB_s_cancelOperation(psa_msg_t *msg)
{
    AESECB_Handle handle_s;
    AESECB_s_CancelOperationMsg cancelMsg;
    int_fast16_t ret;

    /* Cancellation is only supported for non-secure clients */
    if (!TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    if ((msg->in_size[0] != sizeof(cancelMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &cancelMsg, sizeof(cancelMsg));

    handle_s = AESECB_s_getHandle(cancelMsg.handle);
    if (handle_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    ret = AESECB_cancelOperation(handle_s);

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== AESECB_s_handlePsaMsg ========
 */
psa_status_t AESECB_s_handlePsaMsg(psa_msg_t *msg)
{
    psa_status_t status = PSA_SUCCESS;

    switch (msg->type)
    {
        /*
         * If AESECB_S_MSG_TYPE_CONSTRUCT is used by other secure partitions,
         * any pointer arguments provided must reference memory accessible by
         * this partition which is dependent on the TF-M isolation level in use.
         */
        case AESECB_S_MSG_TYPE_CONSTRUCT:
            status = AESECB_s_construct(msg);
            break;

        case AESECB_S_MSG_TYPE_OPEN:
            status = AESECB_s_open(msg);
            break;

        /*
         * AESECB_S_MSG_TYPE_REGISTER_CALLBACK is designed to be used non-secure
         * callers only. Secure callers must only use polling return behavior.
         */
        case AESECB_S_MSG_TYPE_REGISTER_CALLBACK:
            status = AESECB_s_registerCallback(msg);
            break;

        case AESECB_S_MSG_TYPE_CLOSE:
            status = AESECB_s_close(msg);
            break;

        case AESECB_S_MSG_TYPE_ONE_STEP_ENCRYPT: /* Fall through */
        case AESECB_S_MSG_TYPE_ONE_STEP_DECRYPT: /* Fall through */
        case AESECB_S_MSG_TYPE_ADD_DATA:         /* Fall through */
        case AESECB_S_MSG_TYPE_FINALIZE:
            status = AESECB_s_startOperation(msg, msg->type);
            break;

        case AESECB_S_MSG_TYPE_SETUP_ENCRYPT: /* Fall through */
        case AESECB_S_MSG_TYPE_SETUP_DECRYPT:
            status = AESECB_s_setupOperation(msg, msg->type);
            break;

        /*
         * AESECB_S_MSG_TYPE_CANCEL_OPERATION supported for non-secure clients
         * only. Secure callers must only use polling return behavior.
         */
        case AESECB_S_MSG_TYPE_CANCEL_OPERATION:
            status = AESECB_s_cancelOperation(msg);
            break;

        default:
            /* Unknown msg type */
            status = PSA_ERROR_PROGRAMMER_ERROR;
            break;
    }

    return status;
}

/*
 *  ======== AESECB_s_init ========
 */
void AESECB_s_init(void)
{
    AESECB_init();
}
