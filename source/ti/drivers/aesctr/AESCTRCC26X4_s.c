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

#include "AESCTRCC26X4_s.h"

#include <ti/drivers/AESCTR.h>
#include <ti/drivers/aesctr/AESCTRCC26XX.h>
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
    AESCTR_OperationUnion *operation_ns; /* Pointer to non-secure operation */
    AESCTR_OperationUnion operation_s;   /* Secure copy of operation */
} AESCTR_s_Operation;

static AESCTR_s_Operation AESCTR_s_operation;

/*
 * AES CTR Secure Dynamic Instance struct.
 */
typedef struct
{
    AESCTR_Config config;
    AESCTRCC26XX_Object object;
    AESCTRCC26XX_HWAttrs hwAttrs;
} AESCTR_s_DynamicInstance;

/*
 * Used to store a secure copy of the dynamic instance (config plus the
 * object and hwAttrs that it points to) provided by non-secure calls to
 * AESCTR_construct.
 */
static AESCTR_s_DynamicInstance AESCTR_s_dynInstance[CONFIG_AESCTR_S_CONFIG_POOL_SIZE];

/* Stores pointers to non-secure AESCTR_s_SecureCallbacks for each driver instance opened or constructed */
static AESCTR_s_SecureCallback *AESCTR_s_secureCB[AESCTR_SECURE_CALLBACK_COUNT];

/* Static driver instances allocated by SysConfig */
extern const AESCTR_Config AESCTR_config[];

/*
 *  ======== AESCTR_s_getCallbackIndex ========
 *  Returns callback index or -1 if index is not found.
 */
static int8_t AESCTR_s_getCallbackIndex(AESCTR_Handle handle_s)
{
    uint_fast8_t index;
    int8_t retIndex = -1;
    bool indexFound = false;

    /* First, search config statically allocated by SysConfig */
    for (index = 0; index < CONFIG_TI_DRIVERS_AESCTR_COUNT; index++)
    {
        if (handle_s == (AESCTR_Handle)&AESCTR_config[index])
        {
            indexFound = true;
            break;
        }
    }

    /* If not found, search secure config copies */
    if (!indexFound)
    {
        for (index = 0; index < CONFIG_AESCTR_S_CONFIG_POOL_SIZE; index++)
        {
            if (handle_s == &AESCTR_s_dynInstance[index].config)
            {
                index += CONFIG_TI_DRIVERS_AESCTR_COUNT;
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
 *  ======== AESCTR_s_hwiCallback ========
 */
static void AESCTR_s_hwiCallback(AESCTR_Handle handle_s,
                                 int_fast16_t returnValue,
                                 AESCTR_OperationUnion *operation,
                                 AESCTR_OperationType operationType)
{
    int8_t index;
    AESCTR_s_SecureCallback *aesctrSecureCB_ns;

    index = AESCTR_s_getCallbackIndex(handle_s);

    if ((index >= 0) && (index < AESCTR_SECURE_CALLBACK_COUNT))
    {
        aesctrSecureCB_ns = AESCTR_s_secureCB[index];

        if (aesctrSecureCB_ns != NULL)
        {
            /* Store arguments in callback object for use by non-secure handler */
            aesctrSecureCB_ns->handle        = (AESCTR_Handle)(CRYPTO_S_HANDLE_ID_AESCTR | index);
            aesctrSecureCB_ns->returnValue   = returnValue;
            aesctrSecureCB_ns->operation     = AESCTR_s_operation.operation_ns;
            aesctrSecureCB_ns->operationType = operationType;

            /* Trigger the interrupt for the non-secure callback dispatcher */
            SecureCallback_post(&aesctrSecureCB_ns->object);
        }
    }
}

/*
 *  ======== AESCTR_s_copyConfig ========
 */
static inline psa_status_t AESCTR_s_copyConfig(AESCTR_Config **secureConfig,
                                               const AESCTR_Config *config,
                                               AESCTR_Handle *retHandle)
{
    AESCTR_Config *config_s;
    AESCTR_s_DynamicInstance *dynInstance_s;
    uint_fast8_t i;

    for (i = 0; i < CONFIG_AESCTR_S_CONFIG_POOL_SIZE; i++)
    {
        dynInstance_s = &AESCTR_s_dynInstance[i];
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
            AESCTR_s_secureCB[i + CONFIG_TI_DRIVERS_AESCTR_COUNT] = NULL;

            /*
             * Return handle is the CRYPTO_S_HANDLE_ID_AESCTR OR'd with the
             * the config pool array index plus the size of constant config
             * array created by Sysconfig.
             */
            *retHandle = (AESCTR_Handle)(CRYPTO_S_HANDLE_ID_AESCTR | (i + CONFIG_TI_DRIVERS_AESCTR_COUNT));
            return PSA_SUCCESS;
        }
    }

    return PSA_ERROR_INSUFFICIENT_MEMORY;
}

/*
 *  ======== AESCTR_s_releaseConfig ========
 */
static inline void AESCTR_s_releaseConfig(AESCTR_Handle nsHandle)
{
    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_AESCTR)
    {
        /* Extract the secure handle index */
        uintptr_t i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        /* Check whether the handle instance refers to a dynamic instance */
        if ((i >= CONFIG_TI_DRIVERS_AESCTR_COUNT) && (i < AESCTR_SECURE_CALLBACK_COUNT))
        {
            /* Set config's object pointer to NULL to indicate the config is available */
            AESCTR_s_dynInstance[i - CONFIG_TI_DRIVERS_AESCTR_COUNT].config.object = NULL;
        }
    }
}

/*
 *  ======== AESCTR_s_copyOneStepOperation ========
 */
static inline psa_status_t AESCTR_s_copyOneStepOperation(AESCTR_OneStepOperation *secureOperation,
                                                         CryptoKey *secureKey,
                                                         const AESCTR_OneStepOperation *operation)
{
    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(AESCTR_OneStepOperation)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(AESCTR_OneStepOperation));

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

    /*
     * Verify initial counter address range if the pointer is not NULL.
     * If the pointer is NULL, zero will be used for the initial counter value.
     */
    if ((secureOperation->initialCounter != NULL) &&
        (cmse_has_unpriv_nonsecure_read_access((void *)secureOperation->initialCounter, AES_BLOCK_SIZE) == NULL))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    return PSA_SUCCESS;
}

/*
 *  ======== AESCTR_s_copySegmentedOperation ========
 */
static psa_status_t AESCTR_s_copySegmentedOperation(AESCTR_SegmentedOperation *secureOperation,
                                                    const AESCTR_SegmentedOperation *operation)
{
    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(AESCTR_SegmentedOperation)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(AESCTR_SegmentedOperation));

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

    return PSA_SUCCESS;
}

/*
 *  ======== AESCTR_s_copyParams ========
 */
static psa_status_t AESCTR_s_copyParams(AESCTR_Params *secureParams, const AESCTR_Params *params)
{
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Validate params address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)params, sizeof(AESCTR_Params)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    (void)spm_memcpy(secureParams, params, sizeof(AESCTR_Params));

    /* Validate the return behavior */
    if ((secureParams->returnBehavior == AESCTR_RETURN_BEHAVIOR_CALLBACK) ||
        (secureParams->returnBehavior == AESCTR_RETURN_BEHAVIOR_BLOCKING) ||
        (secureParams->returnBehavior == AESCTR_RETURN_BEHAVIOR_POLLING))
    {
        if (secureParams->returnBehavior != AESCTR_RETURN_BEHAVIOR_POLLING)
        {
            /*
             * Overwrite the non-secure client's callback function with our own
             * callback which will populate the secure callback object registered
             * using AESCTR_S_MSG_TYPE_REGISTER_CALLBACK.
             */
            secureParams->callbackFxn = AESCTR_s_hwiCallback;

            /* Force to callback return behavior */
            secureParams->returnBehavior = AESCTR_RETURN_BEHAVIOR_CALLBACK;
        }

        status = PSA_SUCCESS;
    }

    return status;
}

/*
 *  ======== AESCTR_s_getHandle ========
 *  Returns a Secure handle from a handle provided from non-secure client.
 */
static AESCTR_Handle AESCTR_s_getHandle(AESCTR_Handle nsHandle)
{
    AESCTR_Handle secureHandle = NULL;
    uint32_t i;

    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_AESCTR)
    {
        /* Extract the secure handle index */
        i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        if (i < CONFIG_TI_DRIVERS_AESCTR_COUNT)
        {
            secureHandle = (AESCTR_Handle)&AESCTR_config[i];
        }
        else if ((i >= CONFIG_TI_DRIVERS_AESCTR_COUNT) && (i < AESCTR_SECURE_CALLBACK_COUNT))
        {
            secureHandle = &AESCTR_s_dynInstance[i - CONFIG_TI_DRIVERS_AESCTR_COUNT].config;
        }
    }

    return secureHandle;
}

/*
 *  ======== AESCTR_s_registerCallback ========
 */
static inline psa_status_t AESCTR_s_registerCallback(psa_msg_t *msg)
{
    AESCTR_Handle handle_s;
    AESCTR_s_CallbackMsg callbackMsg;
    int8_t callbackIndex;
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Only non-secure callers should be registering callbacks */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id) && (msg->in_size[0] == sizeof(callbackMsg)))
    {
        psa_read(msg->handle, 0, &callbackMsg, sizeof(callbackMsg));

        handle_s = AESCTR_s_getHandle(callbackMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        callbackIndex = AESCTR_s_getCallbackIndex(handle_s);

        /* Validate index and callback object address range */
        if ((callbackIndex >= 0) && (callbackIndex < AESCTR_SECURE_CALLBACK_COUNT) &&
            (cmse_has_unpriv_nonsecure_rw_access(callbackMsg.callback, sizeof(AESCTR_s_SecureCallback)) != NULL))
        {
            /*
             * Store the pointer to AESCTR_s_SecureCallback located in
             * non-secure memory.
             */
            AESCTR_s_secureCB[callbackIndex] = callbackMsg.callback;

            status = PSA_SUCCESS;
        }
    }

    return status;
}

/*
 *  ======== AESCTR_s_construct ========
 */
static inline psa_status_t AESCTR_s_construct(psa_msg_t *msg)
{
    AESCTR_s_ConstructMsg constructMsg;
    AESCTR_Handle handle;
    AESCTR_Params params_s;
    const AESCTR_Params *paramsPtr_s = NULL;
    AESCTR_Config *configPtr_s;
    psa_status_t status     = PSA_ERROR_PROGRAMMER_ERROR;
    AESCTR_Handle retHandle = NULL;

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
            status = AESCTR_s_copyParams(&params_s, constructMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }

            paramsPtr_s = &params_s;
        }

        /* Verify config and copy to secure memory */
        status = AESCTR_s_copyConfig(&configPtr_s, constructMsg.config, &retHandle);
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

    handle = AESCTR_construct(configPtr_s, paramsPtr_s);

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
 *  ======== AESCTR_s_open ========
 */
static inline psa_status_t AESCTR_s_open(psa_msg_t *msg)
{
    AESCTR_s_OpenMsg openMsg;
    AESCTR_Handle handle;
    AESCTR_Params params_s;
    AESCTR_Params *paramsPtr_s = NULL;
    AESCTR_Handle retHandle    = NULL;
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
            status = AESCTR_s_copyParams(&params_s, openMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }
        }

        paramsPtr_s = &params_s;
    }

    handle = AESCTR_open(openMsg.index, paramsPtr_s);

    if (handle != NULL)
    {
        if (TFM_CLIENT_ID_IS_NS(msg->client_id))
        {
            /*
             * Set the secure callback object pointer to NULL. The non-secure
             * client must register a secure callback after constructing or
             * opening a driver instance.
             */
            AESCTR_s_secureCB[openMsg.index] = NULL;

            /*
             * Return CRYPTO_S_HANDLE_ID_AESCTR OR'd with the config array index
             * as the handle instead of a pointer to the config to allow for
             * validation of handles provided by non-secure clients.
             */
            retHandle = (AESCTR_Handle)(CRYPTO_S_HANDLE_ID_AESCTR | openMsg.index);
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
 *  ======== AESCTR_s_close ========
 */
static inline psa_status_t AESCTR_s_close(psa_msg_t *msg)
{
    AESCTR_Handle handle_s;
    AESCTR_s_CloseMsg closeMsg;

    if (msg->in_size[0] != sizeof(closeMsg))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &closeMsg, sizeof(closeMsg));

    /* Non-secure callers have negative client ID */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESCTR_s_getHandle(closeMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        AESCTR_close(handle_s);

        /* Release the secure config if it is a dynamic instance */
        AESCTR_s_releaseConfig(closeMsg.handle);
    }
    else /* Secure client */
    {
        AESCTR_close(closeMsg.handle);
    }

    return PSA_SUCCESS;
}

/*
 *  ======== AESCTR_s_oneStepOperation ========
 */
static inline psa_status_t AESCTR_s_oneStepOperation(psa_msg_t *msg, int32_t msgType)
{
    AESCTR_s_OneStepOperationMsg oneStepMsg;
    AESCTR_Handle handle_s;
    AESCTR_OneStepOperation *operation_s;
    CryptoKey key_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(oneStepMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &oneStepMsg, sizeof(oneStepMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESCTR_s_getHandle(oneStepMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &AESCTR_s_operation.operation_s.oneStepOperation;

        /* Save pointer to non-secure operation struct */
        AESCTR_s_operation.operation_ns = (AESCTR_OperationUnion *)oneStepMsg.operation;

        /* Validate and copy operation */
        status = AESCTR_s_copyOneStepOperation(operation_s, &key_s, oneStepMsg.operation);
        if (status != PSA_SUCCESS)
        {
            return status;
        }
    }
    else /* Secure client */
    {
        handle_s    = oneStepMsg.handle;
        operation_s = oneStepMsg.operation;
    }

    if (msgType == AESCTR_S_MSG_TYPE_ONE_STEP_ENCRYPT)
    {
        ret = AESCTR_oneStepEncrypt(handle_s, operation_s);
    }
    else
    {
        ret = AESCTR_oneStepDecrypt(handle_s, operation_s);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== AESCTR_s_setupOperation ========
 */
static inline psa_status_t AESCTR_s_setupOperation(psa_msg_t *msg, int32_t msgType)
{
    AESCTR_s_SetupOperationMsg setupMsg;
    AESCTR_Handle handle_s;
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
        handle_s = AESCTR_s_getHandle(setupMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /*
         * Verify initial counter address range if the pointer is not NULL.
         * If the pointer is NULL, zero will be used for the initial counter value.
         */
        if ((setupMsg.initialCounter != NULL) &&
            (cmse_has_unpriv_nonsecure_read_access((void *)setupMsg.initialCounter, AES_BLOCK_SIZE) == NULL))
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

    if (msgType == AESCTR_S_MSG_TYPE_SETUP_ENCRYPT)
    {
        ret = AESCTR_setupEncrypt(handle_s, keyPtr_s, setupMsg.initialCounter);
    }
    else
    {
        ret = AESCTR_setupDecrypt(handle_s, keyPtr_s, setupMsg.initialCounter);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== AESCTR_s_addData ========
 */
static inline psa_status_t AESCTR_s_addData(psa_msg_t *msg)
{
    AESCTR_Handle handle_s;
    AESCTR_SegmentedOperation *operation_s;
    AESCTR_s_AddDataMsg addDataMsg;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(addDataMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &addDataMsg, sizeof(addDataMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESCTR_s_getHandle(addDataMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &AESCTR_s_operation.operation_s.segmentedOperation;

        /* Save pointer to non-secure operation struct */
        AESCTR_s_operation.operation_ns = (AESCTR_OperationUnion *)addDataMsg.operation;

        /* Verify and copy operation to secure memory */
        status = AESCTR_s_copySegmentedOperation(operation_s, addDataMsg.operation);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = AESCTR_addData(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = AESCTR_addData(addDataMsg.handle, addDataMsg.operation);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== AESCTR_s_finalizeOperation ========
 */
static inline psa_status_t AESCTR_s_finalizeOperation(psa_msg_t *msg)
{
    AESCTR_s_FinalizeMsg finalizeMsg;
    AESCTR_SegmentedOperation *operation_s;
    AESCTR_Handle handle_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(finalizeMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &finalizeMsg, sizeof(finalizeMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESCTR_s_getHandle(finalizeMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &AESCTR_s_operation.operation_s.segmentedOperation;

        /* Save pointer to non-secure operation struct */
        AESCTR_s_operation.operation_ns = (AESCTR_OperationUnion *)finalizeMsg.operation;

        /* Verify and copy operation to secure memory */
        status = AESCTR_s_copySegmentedOperation(operation_s, finalizeMsg.operation);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = AESCTR_finalize(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = AESCTR_finalize(finalizeMsg.handle, finalizeMsg.operation);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== AESCTR_s_cancelOperation ========
 */
static inline psa_status_t AESCTR_s_cancelOperation(psa_msg_t *msg)
{
    AESCTR_Handle handle_s;
    AESCTR_s_CancelOperationMsg cancelMsg;
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

    handle_s = AESCTR_s_getHandle(cancelMsg.handle);
    if (handle_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    ret = AESCTR_cancelOperation(handle_s);

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== AESCTR_s_handlePsaMsg ========
 */
psa_status_t AESCTR_s_handlePsaMsg(psa_msg_t *msg)
{
    psa_status_t status = PSA_SUCCESS;

    switch (msg->type)
    {
        /*
         * If AESCTR_S_MSG_TYPE_CONSTRUCT is used by other secure partitions,
         * any pointer arguments provided must reference memory accessible by
         * this partition which is dependent on the TF-M isolation level in use.
         */
        case AESCTR_S_MSG_TYPE_CONSTRUCT:
            status = AESCTR_s_construct(msg);
            break;

        case AESCTR_S_MSG_TYPE_OPEN:
            status = AESCTR_s_open(msg);
            break;

        /*
         * AESCTR_S_MSG_TYPE_REGISTER_CALLBACK is designed to be used non-secure
         * callers only. Secure callers must only use polling return behavior.
         */
        case AESCTR_S_MSG_TYPE_REGISTER_CALLBACK:
            status = AESCTR_s_registerCallback(msg);
            break;

        case AESCTR_S_MSG_TYPE_CLOSE:
            status = AESCTR_s_close(msg);
            break;

        case AESCTR_S_MSG_TYPE_ONE_STEP_ENCRYPT: /* Fall through */
        case AESCTR_S_MSG_TYPE_ONE_STEP_DECRYPT:
            status = AESCTR_s_oneStepOperation(msg, msg->type);
            break;

        case AESCTR_S_MSG_TYPE_SETUP_ENCRYPT: /* Fall through */
        case AESCTR_S_MSG_TYPE_SETUP_DECRYPT:
            status = AESCTR_s_setupOperation(msg, msg->type);
            break;

        case AESCTR_S_MSG_TYPE_ADD_DATA:
            status = AESCTR_s_addData(msg);
            break;

        case AESCTR_S_MSG_TYPE_FINALIZE:
            status = AESCTR_s_finalizeOperation(msg);
            break;

        /*
         * AESCTR_S_MSG_TYPE_CANCEL_OPERATION supported for non-secure clients
         * only. Secure callers must only use polling return behavior.
         */
        case AESCTR_S_MSG_TYPE_CANCEL_OPERATION:
            status = AESCTR_s_cancelOperation(msg);
            break;

        default:
            /* Unknown msg type */
            status = PSA_ERROR_PROGRAMMER_ERROR;
            break;
    }

    return status;
}

/*
 *  ======== AESCTR_s_init ========
 */
void AESCTR_s_init(void)
{
    AESCTR_init();
}
