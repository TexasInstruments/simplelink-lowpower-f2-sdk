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

#include "AESGCMCC26X4_s.h"

#include <ti/drivers/AESGCM.h>
#include <ti/drivers/aesgcm/AESGCMCC26X4.h>
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
    AESGCM_OperationUnion *operation_ns; /* Pointer to non-secure operation */
    AESGCM_OperationUnion operation_s;   /* Secure copy of operation */
} AESGCM_s_Operation;

static AESGCM_s_Operation AESGCM_s_operation;

/*
 * AES GCM Secure Dynamic Instance struct.
 */
typedef struct
{
    AESGCM_Config config;
    AESGCMCC26X4_Object object;
    AESGCMCC26X4_HWAttrs hwAttrs;
} AESGCM_s_DynamicInstance;

/*
 * Used to store a secure copy of the dynamic instance (config plus the
 * object and hwAttrs that it points to) provided by non-secure calls to
 * AESGCM_construct.
 */
static AESGCM_s_DynamicInstance AESGCM_s_dynInstance[CONFIG_AESGCM_S_CONFIG_POOL_SIZE];

/* Stores pointers to non-secure AESGCM_s_SecureCallbacks for each driver instance opened or constructed */
static AESGCM_s_SecureCallback *AESGCM_s_secureCB[AESGCM_SECURE_CALLBACK_COUNT];

/* Static driver instances allocated by SysConfig */
extern const AESGCM_Config AESGCM_config[];

/*
 *  ======== AESGCM_s_getCallbackIndex ========
 *  Returns callback index or -1 if index is not found.
 */
int8_t AESGCM_s_getCallbackIndex(AESGCM_Handle handle_s)
{
    uint_fast8_t index;
    int8_t retIndex = -1;
    bool indexFound = false;

    /* First, search config statically allocated by SysConfig */
    for (index = 0; index < CONFIG_TI_DRIVERS_AESGCM_COUNT; index++)
    {
        if (handle_s == (AESGCM_Handle)&AESGCM_config[index])
        {
            indexFound = true;
            break;
        }
    }

    /* If not found, search secure config copies */
    if (!indexFound)
    {
        for (index = 0; index < CONFIG_AESGCM_S_CONFIG_POOL_SIZE; index++)
        {
            if (handle_s == &AESGCM_s_dynInstance[index].config)
            {
                index += CONFIG_TI_DRIVERS_AESGCM_COUNT;
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
 *  ======== AESGCM_s_hwiCallback ========
 */
static void AESGCM_s_hwiCallback(AESGCM_Handle handle_s,
                                 int_fast16_t returnValue,
                                 AESGCM_OperationUnion *operation,
                                 AESGCM_OperationType operationType)
{
    int8_t index;
    AESGCM_s_SecureCallback *aesgcmSecureCB_ns;

    index = AESGCM_s_getCallbackIndex(handle_s);

    if ((index >= 0) && (index < AESGCM_SECURE_CALLBACK_COUNT))
    {
        aesgcmSecureCB_ns = AESGCM_s_secureCB[index];

        if (aesgcmSecureCB_ns != NULL)
        {
            /* Store arguments in callback object for use by non-secure handler */
            aesgcmSecureCB_ns->handle        = (AESGCM_Handle)(CRYPTO_S_HANDLE_ID_AESGCM | index);
            aesgcmSecureCB_ns->returnValue   = returnValue;
            aesgcmSecureCB_ns->operation     = AESGCM_s_operation.operation_ns;
            aesgcmSecureCB_ns->operationType = operationType;

            /* Trigger the interrupt for the non-secure callback dispatcher */
            SecureCallback_post(&aesgcmSecureCB_ns->object);
        }
    }
}

/*
 *  ======== AESGCM_s_copyConfig ========
 */
static inline psa_status_t AESGCM_s_copyConfig(AESGCM_Config **secureConfig,
                                               const AESGCM_Config *config,
                                               AESGCM_Handle *retHandle)
{
    AESGCM_Config *config_s;
    AESGCM_s_DynamicInstance *dynInstance_s;
    uint_fast8_t i;

    for (i = 0; i < CONFIG_AESGCM_S_CONFIG_POOL_SIZE; i++)
    {
        dynInstance_s = &AESGCM_s_dynInstance[i];
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
            AESGCM_s_secureCB[i + CONFIG_TI_DRIVERS_AESGCM_COUNT] = NULL;

            /*
             * Return handle is the CRYPTO_S_HANDLE_ID_AESGCM OR'd with the
             * the config pool array index plus the size of constant config
             * array created by Sysconfig.
             */
            *retHandle = (AESGCM_Handle)(CRYPTO_S_HANDLE_ID_AESGCM | (i + CONFIG_TI_DRIVERS_AESGCM_COUNT));
            return PSA_SUCCESS;
        }
    }

    return PSA_ERROR_INSUFFICIENT_MEMORY;
}

/*
 *  ======== AESGCM_s_releaseConfig ========
 */
static inline void AESGCM_s_releaseConfig(AESGCM_Handle nsHandle)
{
    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_AESGCM)
    {
        /* Extract the secure handle index */
        uintptr_t i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        /* Check whether the handle instance refers to a dynamic instance */
        if ((i >= CONFIG_TI_DRIVERS_AESGCM_COUNT) && (i < AESGCM_SECURE_CALLBACK_COUNT))
        {
            /* Set config's object pointer to NULL to indicate the config is available */
            AESGCM_s_dynInstance[i - CONFIG_TI_DRIVERS_AESGCM_COUNT].config.object = NULL;
        }
    }
}

/*
 *  ======== AESGCM_s_copyOneStepOperation ========
 */
psa_status_t AESGCM_s_copyOneStepOperation(AESGCM_OneStepOperation *secureOperation,
                                           CryptoKey *secureKey,
                                           const AESGCM_OneStepOperation *operation,
                                           int32_t msgType)
{
    /* Internally generated IV is not supported */
    if (secureOperation->ivInternallyGenerated)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(AESGCM_OneStepOperation)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(AESGCM_OneStepOperation));

    /* Validate crypto key struct address range */
    if (cmse_has_unpriv_nonsecure_read_access(secureOperation->key, sizeof(CryptoKey)) == NULL)
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

    /* Verify IV address range */
    if (cmse_has_unpriv_nonsecure_read_access(secureOperation->iv, secureOperation->ivLength) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /*
     * The combined length of AAD and input data must be non-zero. This validation
     * check will occur within the AES GCM driver.
     */

    /* Verify AAD address range if AAD provided */
    if ((secureOperation->aadLength > 0) &&
        (cmse_has_unpriv_nonsecure_read_access(secureOperation->aad, secureOperation->aadLength) == NULL))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Verify input and output address range if input data provided */
    if (secureOperation->inputLength > 0)
    {
        if (cmse_has_unpriv_nonsecure_read_access(secureOperation->input, secureOperation->inputLength) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        if (cmse_has_unpriv_nonsecure_rw_access(secureOperation->output, secureOperation->inputLength) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }
    }

    /*
     * Verify MAC address range - MAC is an output for encrypt operation, and an
     * input for decrypt operation.
     */
    if (msgType == AESGCM_S_MSG_TYPE_ONE_STEP_ENCRYPT)
    {
        if (cmse_has_unpriv_nonsecure_rw_access(secureOperation->mac, secureOperation->macLength) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }
    }
    else
    {
        if (cmse_has_unpriv_nonsecure_read_access(secureOperation->mac, secureOperation->macLength) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }
    }

    return PSA_SUCCESS;
}

/*
 *  ======== AESGCM_s_copyAddAADOperation ========
 */
static inline psa_status_t AESGCM_s_copyAddAADOperation(AESGCM_SegmentedAADOperation *secureOperation,
                                                        const AESGCM_SegmentedAADOperation *operation)
{
    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(AESGCM_SegmentedAADOperation)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(AESGCM_SegmentedAADOperation));

    /* Verify AAD address range */
    if (cmse_has_unpriv_nonsecure_read_access(secureOperation->aad, secureOperation->aadLength) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    return PSA_SUCCESS;
}

/*
 *  ======== AESGCM_s_copyAddDataOperation ========
 */
static inline psa_status_t AESGCM_s_copyAddDataOperation(AESGCM_SegmentedDataOperation *secureOperation,
                                                         const AESGCM_SegmentedDataOperation *operation)
{
    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(AESGCM_SegmentedDataOperation)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(AESGCM_SegmentedDataOperation));

    /*
     * The input length must be a non-zero. This validation check will occur in
     * within the AES GCM driver.
     */

    /* Verify input address range */
    if (cmse_has_unpriv_nonsecure_read_access(secureOperation->input, secureOperation->inputLength) == NULL)
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
 *  ======== AESGCM_s_copyFinalizeOperation ========
 */
static inline psa_status_t AESGCM_s_copyFinalizeOperation(AESGCM_SegmentedFinalizeOperation *secureOperation,
                                                          const AESGCM_SegmentedFinalizeOperation *operation,
                                                          int32_t msgType)
{
    psa_status_t status = PSA_SUCCESS;

    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(AESGCM_SegmentedFinalizeOperation)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(AESGCM_SegmentedFinalizeOperation));

    /* Operations can be finalized with or without additional data */
    if (secureOperation->inputLength > 0)
    {
        /* Verify input address range */
        if (cmse_has_unpriv_nonsecure_read_access(secureOperation->input, secureOperation->inputLength) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Verify output address range */
        if (cmse_has_unpriv_nonsecure_rw_access(secureOperation->output, secureOperation->inputLength) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }
    }

    /*
     * Verify MAC address range - MAC is an output for encrypt operation, and an
     * input for decrypt operation.
     */
    if (msgType == AESGCM_S_MSG_TYPE_FINALIZE_ENCRYPT)
    {
        if (cmse_has_unpriv_nonsecure_rw_access(secureOperation->mac, secureOperation->macLength) == NULL)
        {
            status = PSA_ERROR_PROGRAMMER_ERROR;
        }
    }
    else
    {
        if (cmse_has_unpriv_nonsecure_read_access(secureOperation->mac, secureOperation->macLength) == NULL)
        {
            status = PSA_ERROR_PROGRAMMER_ERROR;
        }
    }

    return status;
}

/*
 *  ======== AESGCM_s_copyParams ========
 */
static psa_status_t AESGCM_s_copyParams(AESGCM_Params *secureParams, const AESGCM_Params *params)
{
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Validate params address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)params, sizeof(AESGCM_Params)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    (void)spm_memcpy(secureParams, params, sizeof(AESGCM_Params));

    /* Validate the return behavior */
    if ((secureParams->returnBehavior == AESGCM_RETURN_BEHAVIOR_CALLBACK) ||
        (secureParams->returnBehavior == AESGCM_RETURN_BEHAVIOR_BLOCKING) ||
        (secureParams->returnBehavior == AESGCM_RETURN_BEHAVIOR_POLLING))
    {
        if (secureParams->returnBehavior != AESGCM_RETURN_BEHAVIOR_POLLING)
        {
            /*
             * Overwrite the non-secure client's callback function with our own
             * callback which will populate the secure callback object registered
             * using AESGCM_S_MSG_TYPE_REGISTER_CALLBACK.
             */
            secureParams->callbackFxn = AESGCM_s_hwiCallback;

            /* Force to callback return behavior */
            secureParams->returnBehavior = AESGCM_RETURN_BEHAVIOR_CALLBACK;
        }

        status = PSA_SUCCESS;
    }

    return status;
}

/*
 *  ======== AESGCM_s_getHandle ========
 *  Returns a Secure handle from a handle provided from non-secure client.
 */
static AESGCM_Handle AESGCM_s_getHandle(AESGCM_Handle nsHandle)
{
    AESGCM_Handle secureHandle = NULL;
    uint32_t i;

    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_AESGCM)
    {
        /* Extract the secure handle index */
        i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        if (i < CONFIG_TI_DRIVERS_AESGCM_COUNT)
        {
            secureHandle = (AESGCM_Handle)&AESGCM_config[i];
        }
        else if ((i >= CONFIG_TI_DRIVERS_AESGCM_COUNT) && (i < AESGCM_SECURE_CALLBACK_COUNT))
        {
            secureHandle = &AESGCM_s_dynInstance[i - CONFIG_TI_DRIVERS_AESGCM_COUNT].config;
        }
    }

    return secureHandle;
}

/*
 *  ======== AESGCM_s_registerCallback ========
 */
static inline psa_status_t AESGCM_s_registerCallback(psa_msg_t *msg)
{
    AESGCM_Handle handle_s;
    AESGCM_s_CallbackMsg callbackMsg;
    int8_t callbackIndex;
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Only non-secure callers should be registering callbacks */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id) && (msg->in_size[0] == sizeof(callbackMsg)))
    {
        psa_read(msg->handle, 0, &callbackMsg, sizeof(callbackMsg));

        handle_s = AESGCM_s_getHandle(callbackMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        callbackIndex = AESGCM_s_getCallbackIndex(handle_s);

        /* Validate index and callback object address range */
        if ((callbackIndex >= 0) && (callbackIndex < AESGCM_SECURE_CALLBACK_COUNT) &&
            (cmse_has_unpriv_nonsecure_rw_access(callbackMsg.callback, sizeof(AESGCM_s_SecureCallback)) != NULL))
        {
            /*
             * Store the pointer to AESGCM_s_SecureCallback located in
             * non-secure memory.
             */
            AESGCM_s_secureCB[callbackIndex] = callbackMsg.callback;

            status = PSA_SUCCESS;
        }
    }

    return status;
}

/*
 *  ======== AESGCM_s_construct ========
 */
static inline psa_status_t AESGCM_s_construct(psa_msg_t *msg)
{
    AESGCM_s_ConstructMsg constructMsg;
    AESGCM_Handle handle;
    AESGCM_Params params_s;
    const AESGCM_Params *paramsPtr_s = NULL;
    AESGCM_Config *configPtr_s;
    psa_status_t status     = PSA_ERROR_PROGRAMMER_ERROR;
    AESGCM_Handle retHandle = NULL;

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
            status = AESGCM_s_copyParams(&params_s, constructMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }

            paramsPtr_s = &params_s;
        }

        /* Verify config and copy to secure memory */
        status = AESGCM_s_copyConfig(&configPtr_s, constructMsg.config, &retHandle);
        if (status != PSA_SUCCESS)
        {
            return status;
        }
    }
    else
    {
        configPtr_s = constructMsg.config;
        paramsPtr_s = constructMsg.params;
    }

    handle = AESGCM_construct(configPtr_s, paramsPtr_s);

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
 *  ======== AESGCM_s_open ========
 */
static inline psa_status_t AESGCM_s_open(psa_msg_t *msg)
{
    AESGCM_s_OpenMsg openMsg;
    AESGCM_Handle handle;
    AESGCM_Params params_s;
    AESGCM_Params *paramsPtr_s = NULL;
    AESGCM_Handle retHandle    = NULL;
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
            status = AESGCM_s_copyParams(&params_s, openMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }
        }

        paramsPtr_s = &params_s;
    }

    handle = AESGCM_open(openMsg.index, paramsPtr_s);

    if (handle != NULL)
    {
        if (TFM_CLIENT_ID_IS_NS(msg->client_id))
        {
            /*
             * Set the secure callback object pointer to NULL. The non-secure
             * client must register a secure callback after constructing or
             * opening a driver instance.
             */
            AESGCM_s_secureCB[openMsg.index] = NULL;

            /*
             * Return CRYPTO_S_HANDLE_ID_AESGCM OR'd with the config array index
             * as the handle instead of a pointer to the config to allow for
             * validation of handles provided by non-secure clients.
             */
            retHandle = (AESGCM_Handle)(CRYPTO_S_HANDLE_ID_AESGCM | openMsg.index);
        }
        else
        {
            retHandle = handle;
        }
    }

    psa_write(msg->handle, 0, &retHandle, sizeof(retHandle));

    return PSA_SUCCESS;
}

/*
 *  ======== AESGCM_s_close ========
 */
static inline psa_status_t AESGCM_s_close(psa_msg_t *msg)
{
    AESGCM_Handle handle_s;
    AESGCM_s_CloseMsg closeMsg;

    if (msg->in_size[0] != sizeof(closeMsg))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &closeMsg, sizeof(closeMsg));

    /* Non-secure callers have negative client ID */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESGCM_s_getHandle(closeMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        AESGCM_close(handle_s);

        /* Release the secure config if it is a dynamic instance */
        AESGCM_s_releaseConfig(closeMsg.handle);
    }
    else
    {
        AESGCM_close(closeMsg.handle);
    }

    return PSA_SUCCESS;
}

/*
 *  ======== AESGCM_s_oneStepOperation ========
 */
static inline psa_status_t AESGCM_s_oneStepOperation(psa_msg_t *msg, int32_t msgType)
{
    AESGCM_s_OneStepOperationMsg oneStepMsg;
    AESGCM_Handle handle_s;
    AESGCM_OneStepOperation *operation_s;
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
        handle_s = AESGCM_s_getHandle(oneStepMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &AESGCM_s_operation.operation_s.oneStepOperation;

        /* Save pointer to non-secure operation struct */
        AESGCM_s_operation.operation_ns = (AESGCM_OperationUnion *)oneStepMsg.operation;

        /* Validate and copy operation and key structs */
        status = AESGCM_s_copyOneStepOperation(operation_s, &key_s, oneStepMsg.operation, msgType);
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

    if (msgType == AESGCM_S_MSG_TYPE_ONE_STEP_ENCRYPT)
    {
        ret = AESGCM_oneStepEncrypt(handle_s, operation_s);
    }
    else
    {
        ret = AESGCM_oneStepDecrypt(handle_s, operation_s);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== AESGCM_s_setupOperation ========
 */
static inline psa_status_t AESGCM_s_setupOperation(psa_msg_t *msg, int32_t msgType)
{
    AESGCM_s_SetupOperationMsg setupMsg;
    AESGCM_Handle handle_s;
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
        handle_s = AESGCM_s_getHandle(setupMsg.handle);
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

    if (msgType == AESGCM_S_MSG_TYPE_SETUP_ENCRYPT)
    {
        ret = AESGCM_setupEncrypt(handle_s, keyPtr_s, setupMsg.totalAADLength, setupMsg.totalPlaintextLength);
    }
    else
    {
        ret = AESGCM_setupDecrypt(handle_s, keyPtr_s, setupMsg.totalAADLength, setupMsg.totalPlaintextLength);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== AESGCM_s_setLengths ========
 */
static inline psa_status_t AESGCM_s_setLengths(psa_msg_t *msg)
{
    AESGCM_Handle handle_s;
    AESGCM_s_SetLengthsMsg setLengthsMsg;
    int_fast16_t ret;

    if (msg->in_size[0] != sizeof(setLengthsMsg))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &setLengthsMsg, sizeof(setLengthsMsg));

    /* Non-secure callers have negative client ID */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESGCM_s_getHandle(setLengthsMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }
    }
    else /* Secure client */
    {
        handle_s = setLengthsMsg.handle;
    }

    ret = AESGCM_setLengths(handle_s, setLengthsMsg.aadLength, setLengthsMsg.plaintextLength);

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== AESGCM_s_setIV ========
 */
static inline psa_status_t AESGCM_s_setIV(psa_msg_t *msg)
{
    AESGCM_Handle handle_s;
    AESGCM_s_SetIVMsg setIVMsg;
    int_fast16_t ret;

    if ((msg->in_size[0] != sizeof(setIVMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &setIVMsg, sizeof(setIVMsg));

    if ((setIVMsg.iv == NULL) || (setIVMsg.ivLength != AESGCM_IV_LENGTH_BYTES))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESGCM_s_getHandle(setIVMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Verify IV address range */
        if (cmse_has_unpriv_nonsecure_read_access((void *)setIVMsg.iv, setIVMsg.ivLength) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }
    }
    else /* Secure client */
    {
        handle_s = setIVMsg.handle;
    }

    ret = AESGCM_setIV(handle_s, setIVMsg.iv, setIVMsg.ivLength);

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== AESGCM_s_addAAD ========
 */
static inline psa_status_t AESGCM_s_addAAD(psa_msg_t *msg)
{
    AESGCM_Handle handle_s;
    AESGCM_SegmentedAADOperation *operation_s;
    AESGCM_s_AddAADMsg addAADMsg;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(addAADMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &addAADMsg, sizeof(addAADMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESGCM_s_getHandle(addAADMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &AESGCM_s_operation.operation_s.segmentedAADOperation;

        /* Save pointer to non-secure operation struct */
        AESGCM_s_operation.operation_ns = (AESGCM_OperationUnion *)addAADMsg.operation;

        /* Verify and copy operation to secure memory */
        status = AESGCM_s_copyAddAADOperation(operation_s, addAADMsg.operation);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = AESGCM_addAAD(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = AESGCM_addAAD(addAADMsg.handle, addAADMsg.operation);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== AESGCM_s_addData ========
 */
static inline psa_status_t AESGCM_s_addData(psa_msg_t *msg)
{
    AESGCM_Handle handle_s;
    AESGCM_SegmentedDataOperation *operation_s;
    AESGCM_s_AddDataMsg addDataMsg;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(addDataMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &addDataMsg, sizeof(addDataMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESGCM_s_getHandle(addDataMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &AESGCM_s_operation.operation_s.segmentedDataOperation;

        /* Save pointer to non-secure operation struct */
        AESGCM_s_operation.operation_ns = (AESGCM_OperationUnion *)addDataMsg.operation;

        /* Verify and copy operation to secure memory */
        status = AESGCM_s_copyAddDataOperation(operation_s, addDataMsg.operation);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = AESGCM_addData(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = AESGCM_addData(addDataMsg.handle, addDataMsg.operation);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== AESGCM_s_finalizeOperation ========
 */
static inline psa_status_t AESGCM_s_finalizeOperation(psa_msg_t *msg, int32_t msgType)
{
    AESGCM_s_FinalizeOperationMsg finalizeMsg;
    AESGCM_SegmentedFinalizeOperation *operation_s;
    AESGCM_Handle handle_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(finalizeMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &finalizeMsg, sizeof(finalizeMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESGCM_s_getHandle(finalizeMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &AESGCM_s_operation.operation_s.segmentedFinalizeOperation;

        /* Save pointer to non-secure operation struct */
        AESGCM_s_operation.operation_ns = (AESGCM_OperationUnion *)finalizeMsg.operation;

        /* Verify and copy operation to secure memory */
        status = AESGCM_s_copyFinalizeOperation(operation_s, finalizeMsg.operation, msgType);
        if (status != PSA_SUCCESS)
        {
            return status;
        }
    }
    else /* Secure client */
    {
        handle_s    = finalizeMsg.handle;
        operation_s = finalizeMsg.operation;
    }

    if (msgType == AESGCM_S_MSG_TYPE_FINALIZE_ENCRYPT)
    {
        ret = AESGCM_finalizeEncrypt(handle_s, operation_s);
    }
    else
    {
        ret = AESGCM_finalizeDecrypt(handle_s, operation_s);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== AESGCM_s_cancelOperation ========
 */
static inline psa_status_t AESGCM_s_cancelOperation(psa_msg_t *msg)
{
    AESGCM_Handle handle_s;
    AESGCM_s_CancelOperationMsg cancelMsg;
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

    handle_s = AESGCM_s_getHandle(cancelMsg.handle);
    if (handle_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    ret = AESGCM_cancelOperation(handle_s);

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== AESGCM_s_handlePsaMsg ========
 */
psa_status_t AESGCM_s_handlePsaMsg(psa_msg_t *msg)
{
    psa_status_t status = PSA_SUCCESS;

    switch (msg->type)
    {
        /*
         * If AESGCM_S_MSG_TYPE_CONSTRUCT is used by other secure partitions,
         * any pointer arguments provided must reference memory accessible by
         * this partition which is dependent on the TF-M isolation level in use.
         */
        case AESGCM_S_MSG_TYPE_CONSTRUCT:
            status = AESGCM_s_construct(msg);
            break;

        case AESGCM_S_MSG_TYPE_OPEN:
            status = AESGCM_s_open(msg);
            break;

        /*
         * AESGCM_S_MSG_TYPE_REGISTER_CALLBACK is designed to be used non-secure
         * callers only. Secure callers must only use polling return behavior.
         */
        case AESGCM_S_MSG_TYPE_REGISTER_CALLBACK:
            status = AESGCM_s_registerCallback(msg);
            break;

        case AESGCM_S_MSG_TYPE_CLOSE:
            status = AESGCM_s_close(msg);
            break;

        case AESGCM_S_MSG_TYPE_ONE_STEP_DECRYPT: /* Fall through */
        case AESGCM_S_MSG_TYPE_ONE_STEP_ENCRYPT:
            status = AESGCM_s_oneStepOperation(msg, msg->type);
            break;

        case AESGCM_S_MSG_TYPE_SETUP_ENCRYPT: /* Fall through */
        case AESGCM_S_MSG_TYPE_SETUP_DECRYPT:
            status = AESGCM_s_setupOperation(msg, msg->type);
            break;

        case AESGCM_S_MSG_TYPE_SET_LENGTHS:
            status = AESGCM_s_setLengths(msg);
            break;

        case AESGCM_S_MSG_TYPE_SET_IV:
            status = AESGCM_s_setIV(msg);
            break;

        case AESGCM_S_MSG_TYPE_ADD_AAD:
            status = AESGCM_s_addAAD(msg);
            break;

        case AESGCM_S_MSG_TYPE_ADD_DATA:
            status = AESGCM_s_addData(msg);
            break;

        case AESGCM_S_MSG_TYPE_FINALIZE_ENCRYPT: /* Fall through */
        case AESGCM_S_MSG_TYPE_FINALIZE_DECRYPT:
            status = AESGCM_s_finalizeOperation(msg, msg->type);
            break;

        /*
         * AESGCM_S_MSG_TYPE_CANCEL_OPERATION supported for non-secure clients
         * only. Secure callers must only use polling return behavior.
         */
        case AESGCM_S_MSG_TYPE_CANCEL_OPERATION:
            status = AESGCM_s_cancelOperation(msg);
            break;

        default:
            /* Unknown msg type */
            status = PSA_ERROR_PROGRAMMER_ERROR;
            break;
    }

    return status;
}

/*
 *  ======== AESGCM_s_init ========
 */
void AESGCM_s_init(void)
{
    AESGCM_init();
}
