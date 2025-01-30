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

#include "AESCMACCC26X4_s.h"

#include <ti/drivers/AESCMAC.h>
#include <ti/drivers/aescmac/AESCMACCC26XX.h>
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
    AESCMAC_Operation *operation_ns; /* Pointer to non-secure operation */
    AESCMAC_Operation operation_s;   /* Secure copy of operation */
} AESCMAC_s_Operation;

static AESCMAC_s_Operation AESCMAC_s_operation;

/*
 * AES CMAC Secure Dynamic Instance struct.
 */
typedef struct
{
    AESCMAC_Config config;
    AESCMACCC26XX_Object object;
    AESCMACCC26XX_HWAttrs hwAttrs;
} AESCMAC_s_DynamicInstance;

/*
 * Used to store a secure copy of the dynamic instance (config plus the
 * object and hwAttrs that it points to) provided by non-secure calls to
 * AESCMAC_construct.
 */
static AESCMAC_s_DynamicInstance AESCMAC_s_dynInstance[CONFIG_AESCMAC_S_CONFIG_POOL_SIZE];

/* Stores pointers to non-secure AESCMAC_s_SecureCallbacks for each driver instance opened or constructed */
static AESCMAC_s_SecureCallback *AESCMAC_s_secureCB[AESCMAC_SECURE_CALLBACK_COUNT];

/* Static driver instances allocated by SysConfig */
extern const AESCMAC_Config AESCMAC_config[];

/*
 *  ======== AESCMAC_s_getCallbackIndex ========
 *  Returns callback index or -1 if index is not found.
 */
static int8_t AESCMAC_s_getCallbackIndex(AESCMAC_Handle handle_s)
{
    uint_fast8_t index;
    int8_t retIndex = -1;
    bool indexFound = false;

    /* First, search config statically allocated by SysConfig */
    for (index = 0; index < CONFIG_TI_DRIVERS_AESCMAC_COUNT; index++)
    {
        if (handle_s == (AESCMAC_Handle)&AESCMAC_config[index])
        {
            indexFound = true;
            break;
        }
    }

    /* If not found, search secure config copies */
    if (!indexFound)
    {
        for (index = 0; index < CONFIG_AESCMAC_S_CONFIG_POOL_SIZE; index++)
        {
            if (handle_s == &AESCMAC_s_dynInstance[index].config)
            {
                index += CONFIG_TI_DRIVERS_AESCMAC_COUNT;
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
 *  ======== AESCMAC_s_hwiCallback ========
 */
static void AESCMAC_s_hwiCallback(AESCMAC_Handle handle_s,
                                  int_fast16_t returnValue,
                                  AESCMAC_Operation *operation,
                                  AESCMAC_OperationType operationType)
{
    int8_t index;
    AESCMAC_s_SecureCallback *aescmacSecureCB_ns;

    index = AESCMAC_s_getCallbackIndex(handle_s);

    if ((index >= 0) && (index < AESCMAC_SECURE_CALLBACK_COUNT))
    {
        aescmacSecureCB_ns = AESCMAC_s_secureCB[index];

        if (aescmacSecureCB_ns != NULL)
        {
            /* Store arguments in callback object for use by non-secure handler */
            aescmacSecureCB_ns->handle        = (AESCMAC_Handle)(CRYPTO_S_HANDLE_ID_AESCMAC | index);
            aescmacSecureCB_ns->returnValue   = returnValue;
            aescmacSecureCB_ns->operation     = AESCMAC_s_operation.operation_ns;
            aescmacSecureCB_ns->operationType = operationType;

            /* Trigger the interrupt for the non-secure callback dispatcher */
            SecureCallback_post(&aescmacSecureCB_ns->object);
        }
    }
}

/*
 *  ======== AESCMAC_s_copyConfig ========
 */
static inline psa_status_t AESCMAC_s_copyConfig(AESCMAC_Config **secureConfig,
                                                const AESCMAC_Config *config,
                                                AESCMAC_Handle *retHandle)
{
    AESCMAC_Config *config_s;
    AESCMAC_s_DynamicInstance *dynInstance_s;
    uint_fast8_t i;

    for (i = 0; i < CONFIG_AESCMAC_S_CONFIG_POOL_SIZE; i++)
    {
        dynInstance_s = &AESCMAC_s_dynInstance[i];
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
            AESCMAC_s_secureCB[i + CONFIG_TI_DRIVERS_AESCMAC_COUNT] = NULL;

            /*
             * Return handle is the CRYPTO_S_HANDLE_ID_AESCMAC OR'd with the
             * the config pool array index plus the size of constant config
             * array created by Sysconfig.
             */
            *retHandle = (AESCMAC_Handle)(CRYPTO_S_HANDLE_ID_AESCMAC | (i + CONFIG_TI_DRIVERS_AESCMAC_COUNT));
            return PSA_SUCCESS;
        }
    }

    return PSA_ERROR_INSUFFICIENT_MEMORY;
}
/*
 *  ======== AESCMAC_s_releaseConfig ========
 */
static inline void AESCMAC_s_releaseConfig(AESCMAC_Handle nsHandle)
{
    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_AESCMAC)
    {
        /* Extract the secure handle index */
        uintptr_t i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        /* Check whether the handle instance refers to a dynamic instance */
        if ((i >= CONFIG_TI_DRIVERS_AESCMAC_COUNT) && (i < AESCMAC_SECURE_CALLBACK_COUNT))
        {
            /* Set config's object pointer to NULL to indicate the config is available */
            AESCMAC_s_dynInstance[i - CONFIG_TI_DRIVERS_AESCMAC_COUNT].config.object = NULL;
        }
    }
}

/*
 *  ======== AESCMAC_s_copyOperation ========
 */
psa_status_t AESCMAC_s_copyOperation(AESCMAC_Operation *secureOperation,
                                     const AESCMAC_Operation *operation,
                                     AESCMAC_OperationType operationType)
{
    psa_status_t status = PSA_SUCCESS;

    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(AESCMAC_Operation)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(AESCMAC_Operation));

    /* Verify input address range */
    if (cmse_has_unpriv_nonsecure_read_access(secureOperation->input, secureOperation->inputLength) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /*
     * Verify MAC address range - MAC is an output for sign operation, and an
     * input for verify operation. MAC pointer is only used for one-step and
     * finalize operations.
     */
    if ((operationType & AESCMAC_OP_CODE_SEGMENTED) == 0)
    {
        if ((operationType & AESCMAC_OP_FLAG_SIGN) == 0)
        {
            /* Verify operation */
            if (cmse_has_unpriv_nonsecure_read_access(secureOperation->mac, secureOperation->macLength) == NULL)
            {
                status = PSA_ERROR_PROGRAMMER_ERROR;
            }
        }
        else
        {
            /* Sign operation */
            if (cmse_has_unpriv_nonsecure_rw_access(secureOperation->mac, secureOperation->macLength) == NULL)
            {
                status = PSA_ERROR_PROGRAMMER_ERROR;
            }
        }
    }

    return status;
}

/*
 *  ======== AESCMAC_s_copyParams ========
 */
static psa_status_t AESCMAC_s_copyParams(AESCMAC_Params *secureParams, const AESCMAC_Params *params)
{
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Validate params address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)params, sizeof(AESCMAC_Params)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    (void)spm_memcpy(secureParams, params, sizeof(AESCMAC_Params));

    /* Validate the operational mode and return behavior */
    if (((secureParams->operationalMode == AESCMAC_OPMODE_CMAC) ||
         (secureParams->operationalMode == AESCMAC_OPMODE_CBCMAC)) &&
        ((secureParams->returnBehavior == AESCMAC_RETURN_BEHAVIOR_CALLBACK) ||
         (secureParams->returnBehavior == AESCMAC_RETURN_BEHAVIOR_BLOCKING) ||
         (secureParams->returnBehavior == AESCMAC_RETURN_BEHAVIOR_POLLING)))
    {
        if (secureParams->returnBehavior != AESCMAC_RETURN_BEHAVIOR_POLLING)
        {
            /*
             * Overwrite the non-secure client's callback function with our own
             * callback which will populate the secure callback object registered
             * using AESCMAC_S_MSG_TYPE_REGISTER_CALLBACK.
             */
            secureParams->callbackFxn = AESCMAC_s_hwiCallback;

            /* Force to callback return behavior */
            secureParams->returnBehavior = AESCMAC_RETURN_BEHAVIOR_CALLBACK;
        }

        status = PSA_SUCCESS;
    }

    return status;
}

/*
 *  ======== AESCMAC_s_getHandle ========
 *  Returns a Secure handle from a handle provided from non-secure client.
 */
static AESCMAC_Handle AESCMAC_s_getHandle(AESCMAC_Handle nsHandle)
{
    AESCMAC_Handle secureHandle = NULL;
    uint32_t i;

    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_AESCMAC)
    {
        /* Extract the secure handle index */
        i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        if (i < CONFIG_TI_DRIVERS_AESCMAC_COUNT)
        {
            secureHandle = (AESCMAC_Handle)&AESCMAC_config[i];
        }
        else if ((i >= CONFIG_TI_DRIVERS_AESCMAC_COUNT) && (i < AESCMAC_SECURE_CALLBACK_COUNT))
        {
            secureHandle = &AESCMAC_s_dynInstance[i - CONFIG_TI_DRIVERS_AESCMAC_COUNT].config;
        }
    }

    return secureHandle;
}

/*
 *  ======== AESCMAC_s_registerCallback ========
 */
static inline psa_status_t AESCMAC_s_registerCallback(psa_msg_t *msg)
{
    AESCMAC_Handle handle_s;
    AESCMAC_s_CallbackMsg callbackMsg;
    int8_t callbackIndex;
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Only non-secure callers should be registering callbacks */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id) && (msg->in_size[0] == sizeof(callbackMsg)))
    {
        psa_read(msg->handle, 0, &callbackMsg, sizeof(callbackMsg));

        handle_s = AESCMAC_s_getHandle(callbackMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        callbackIndex = AESCMAC_s_getCallbackIndex(handle_s);

        /* Validate index and callback object address range */
        if ((callbackIndex >= 0) && (callbackIndex < AESCMAC_SECURE_CALLBACK_COUNT) &&
            (cmse_has_unpriv_nonsecure_rw_access(callbackMsg.callback, sizeof(AESCMAC_s_SecureCallback)) != NULL))
        {
            /*
             * Store the pointer to AESCMAC_s_SecureCallback located in
             * non-secure memory.
             */
            AESCMAC_s_secureCB[callbackIndex] = callbackMsg.callback;

            status = PSA_SUCCESS;
        }
    }

    return status;
}

/*
 *  ======== AESCMAC_s_construct ========
 */
static inline psa_status_t AESCMAC_s_construct(psa_msg_t *msg)
{
    AESCMAC_s_ConstructMsg constructMsg;
    AESCMAC_Handle handle;
    AESCMAC_Params params_s;
    const AESCMAC_Params *paramsPtr_s = NULL;
    AESCMAC_Config *configPtr_s;
    psa_status_t status      = PSA_ERROR_PROGRAMMER_ERROR;
    AESCMAC_Handle retHandle = NULL;

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
            status = AESCMAC_s_copyParams(&params_s, constructMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }

            paramsPtr_s = &params_s;
        }

        /* Verify config and copy to secure memory */
        status = AESCMAC_s_copyConfig(&configPtr_s, constructMsg.config, &retHandle);
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

    handle = AESCMAC_construct(configPtr_s, paramsPtr_s);

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
 *  ======== AESCMAC_s_open ========
 */
static inline psa_status_t AESCMAC_s_open(psa_msg_t *msg)
{
    AESCMAC_s_OpenMsg openMsg;
    AESCMAC_Handle handle;
    AESCMAC_Params params_s;
    AESCMAC_Params *paramsPtr_s = NULL;
    AESCMAC_Handle retHandle    = NULL;
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
            status = AESCMAC_s_copyParams(&params_s, openMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }
        }

        paramsPtr_s = &params_s;
    }

    handle = AESCMAC_open(openMsg.index, paramsPtr_s);

    if (handle != NULL)
    {
        if (TFM_CLIENT_ID_IS_NS(msg->client_id))
        {
            /*
             * Set the secure callback object pointer to NULL. The non-secure
             * client must register a secure callback after constructing or
             * opening a driver instance.
             */
            AESCMAC_s_secureCB[openMsg.index] = NULL;

            /*
             * Return CRYPTO_S_HANDLE_ID_AESCMAC OR'd with the config array index
             * as the handle instead of a pointer to the config to allow for
             * validation of handles provided by non-secure clients.
             */
            retHandle = (AESCMAC_Handle)(CRYPTO_S_HANDLE_ID_AESCMAC | openMsg.index);
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
 *  ======== AESCMAC_s_close ========
 */
static inline psa_status_t AESCMAC_s_close(psa_msg_t *msg)
{
    AESCMAC_Handle handle_s;
    AESCMAC_s_CloseMsg closeMsg;

    if (msg->in_size[0] != sizeof(closeMsg))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &closeMsg, sizeof(closeMsg));

    /* Non-secure callers have negative client ID */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESCMAC_s_getHandle(closeMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        AESCMAC_close(handle_s);

        /* Release the secure config if it is a dynamic instance */
        AESCMAC_s_releaseConfig(closeMsg.handle);
    }
    else /* Secure client */
    {
        AESCMAC_close(closeMsg.handle);
    }

    return PSA_SUCCESS;
}

/*
 *  ======== AESCMAC_s_oneStepOperation ========
 */
static inline psa_status_t AESCMAC_s_oneStepOperation(psa_msg_t *msg, int32_t msgType)
{
    AESCMAC_s_OneStepOperationMsg oneStepMsg;
    AESCMAC_Handle handle_s;
    AESCMAC_Operation *operation_s;
    AESCMACCC26XX_Object *object;
    CryptoKey key_s;
    CryptoKey *keyPtr_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(oneStepMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &oneStepMsg, sizeof(oneStepMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESCMAC_s_getHandle(oneStepMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        object = handle_s->object;

        operation_s = &AESCMAC_s_operation.operation_s;

        /* Save pointer to non-secure operation struct */
        AESCMAC_s_operation.operation_ns = oneStepMsg.operation;

        /* Verify and copy operation to secure memory */
        status = AESCMAC_s_copyOperation(operation_s, oneStepMsg.operation, object->operationType);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        /* Validate crypto key struct address range */
        if (cmse_has_unpriv_nonsecure_read_access((void *)oneStepMsg.key, sizeof(CryptoKey)) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Copy key to secure memory */
        (void)spm_memcpy(&key_s, oneStepMsg.key, sizeof(CryptoKey));

        if (CryptoKey_verifySecureInputKey(&key_s) != CryptoKey_STATUS_SUCCESS)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        keyPtr_s = &key_s;
    }
    else /* Secure client */
    {
        handle_s    = oneStepMsg.handle;
        operation_s = oneStepMsg.operation;
        keyPtr_s    = oneStepMsg.key;
    }

    if (msgType == AESCMAC_S_MSG_TYPE_ONE_STEP_SIGN)
    {
        ret = AESCMAC_oneStepSign(handle_s, operation_s, keyPtr_s);
    }
    else
    {
        ret = AESCMAC_oneStepVerify(handle_s, operation_s, keyPtr_s);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== AESCMAC_s_setupOperation ========
 */
static inline psa_status_t AESCMAC_s_setupOperation(psa_msg_t *msg, int32_t msgType)
{
    AESCMAC_s_SetupOperationMsg setupMsg;
    AESCMAC_Handle handle_s;
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
        handle_s = AESCMAC_s_getHandle(setupMsg.handle);
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

    if (msgType == AESCMAC_S_MSG_TYPE_SETUP_SIGN)
    {
        ret = AESCMAC_setupSign(handle_s, keyPtr_s);
    }
    else
    {
        ret = AESCMAC_setupVerify(handle_s, keyPtr_s);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== AESCMAC_s_addData ========
 */
static inline psa_status_t AESCMAC_s_addData(psa_msg_t *msg)
{
    AESCMAC_Handle handle_s;
    AESCMAC_Operation *operation_s;
    AESCMAC_s_AddDataMsg addDataMsg;
    AESCMACCC26XX_Object *object;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(addDataMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &addDataMsg, sizeof(addDataMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESCMAC_s_getHandle(addDataMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        object = handle_s->object;

        operation_s = &AESCMAC_s_operation.operation_s;

        /* Save pointer to non-secure operation struct */
        AESCMAC_s_operation.operation_ns = addDataMsg.operation;

        /* Verify and copy operation to secure memory */
        status = AESCMAC_s_copyOperation(operation_s, addDataMsg.operation, object->operationType);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = AESCMAC_addData(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = AESCMAC_addData(addDataMsg.handle, addDataMsg.operation);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== AESCMAC_s_finalizeOperation ========
 */
static inline psa_status_t AESCMAC_s_finalizeOperation(psa_msg_t *msg)
{
    AESCMAC_s_FinalizeMsg finalizeMsg;
    AESCMAC_Operation *operation_s;
    AESCMAC_Handle handle_s;
    AESCMACCC26XX_Object *object;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(finalizeMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &finalizeMsg, sizeof(finalizeMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESCMAC_s_getHandle(finalizeMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        object      = handle_s->object;
        operation_s = &AESCMAC_s_operation.operation_s;

        /* Save pointer to non-secure operation struct */
        AESCMAC_s_operation.operation_ns = finalizeMsg.operation;

        /* Verify and copy operation to secure memory */
        status = AESCMAC_s_copyOperation(operation_s, finalizeMsg.operation, object->operationType);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = AESCMAC_finalize(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = AESCMAC_finalize(finalizeMsg.handle, finalizeMsg.operation);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== AESCMAC_s_cancelOperation ========
 */
static inline psa_status_t AESCMAC_s_cancelOperation(psa_msg_t *msg)
{
    AESCMAC_Handle handle_s;
    AESCMAC_s_CancelOperationMsg cancelMsg;
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

    handle_s = AESCMAC_s_getHandle(cancelMsg.handle);
    if (handle_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    ret = AESCMAC_cancelOperation(handle_s);

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== AESCMAC_s_handlePsaMsg ========
 */
psa_status_t AESCMAC_s_handlePsaMsg(psa_msg_t *msg)
{
    psa_status_t status = PSA_SUCCESS;

    switch (msg->type)
    {
        /*
         * If AESCMAC_S_MSG_TYPE_CONSTRUCT is used by other secure partitions,
         * any pointer arguments provided must reference memory accessible by
         * this partition which is dependent on the TF-M isolation level in use.
         */
        case AESCMAC_S_MSG_TYPE_CONSTRUCT:
            status = AESCMAC_s_construct(msg);
            break;

        case AESCMAC_S_MSG_TYPE_OPEN:
            status = AESCMAC_s_open(msg);
            break;

        /*
         * AESCMAC_S_MSG_TYPE_REGISTER_CALLBACK is designed to be used non-secure
         * callers only. Secure callers must only use polling return behavior.
         */
        case AESCMAC_S_MSG_TYPE_REGISTER_CALLBACK:
            status = AESCMAC_s_registerCallback(msg);
            break;

        case AESCMAC_S_MSG_TYPE_CLOSE:
            status = AESCMAC_s_close(msg);
            break;

        case AESCMAC_S_MSG_TYPE_ONE_STEP_SIGN: /* Fall through */
        case AESCMAC_S_MSG_TYPE_ONE_STEP_VERIFY:
            status = AESCMAC_s_oneStepOperation(msg, msg->type);
            break;

        case AESCMAC_S_MSG_TYPE_SETUP_SIGN: /* Fall through */
        case AESCMAC_S_MSG_TYPE_SETUP_VERIFY:
            status = AESCMAC_s_setupOperation(msg, msg->type);
            break;

        case AESCMAC_S_MSG_TYPE_ADD_DATA:
            status = AESCMAC_s_addData(msg);
            break;

        case AESCMAC_S_MSG_TYPE_FINALIZE:
            status = AESCMAC_s_finalizeOperation(msg);
            break;

        /*
         * AESCMAC_S_MSG_TYPE_CANCEL_OPERATION supported for non-secure clients
         * only. Secure callers must only use polling return behavior.
         */
        case AESCMAC_S_MSG_TYPE_CANCEL_OPERATION:
            status = AESCMAC_s_cancelOperation(msg);
            break;

        default:
            /* Unknown msg type */
            status = PSA_ERROR_PROGRAMMER_ERROR;
            break;
    }

    return status;
}

/*
 *  ======== AESCMAC_s_init ========
 */
void AESCMAC_s_init(void)
{
    AESCMAC_init();
}
