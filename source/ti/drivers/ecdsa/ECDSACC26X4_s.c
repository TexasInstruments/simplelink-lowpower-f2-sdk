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

#include "ECDSACC26X4_s.h"

#include <ti/drivers/ECDSA.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>
#include <ti/drivers/cryptoutils/ecc/ECCParamsCC26X4_s.h>
#include <ti/drivers/ecdsa/ECDSACC26X2.h>

#include <ti/drivers/tfm/SecureCallback.h>

#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/interface/include/psa/service.h>
#include <third_party/tfm/secure_fw/spm/core/spm.h>
#include <third_party/tfm/secure_fw/spm/include/utilities.h>
#include <third_party/tfm/platform/ext/target/ti/cc26x4/cmse.h> /* TI CMSE helper functions */

#include "ti_drivers_config.h" /* Sysconfig generated header */

#define ECDSA_SECURE_OPERATION_COUNT ECDSA_SECURE_CALLBACK_COUNT

/*!
 *  @brief  Union containing all supported operation structs.
 */
typedef union
{
    ECDSA_OperationSign sign;
    ECDSA_OperationVerify verify;
} ECDSA_OperationUnion;

/*
 * Stores a secure copy of the operation & key and the original pointer to the
 * non-secure operation to return in case of callback return behavior.
 */
typedef struct
{
    ECDSA_Operation operation_ns;     /* Pointer to non-secure operation */
    ECDSA_OperationUnion operation_s; /* Secure copy of operation */
    CryptoKey key_s;                  /* Secure copy of key */
} ECDSA_s_Operation;

static ECDSA_s_Operation ECDSA_s_operation;

/*
 * ECDSA Secure Dynamic Instance struct.
 */
typedef struct
{
    ECDSA_Config config;
    ECDSACC26X2_Object object;
    ECDSACC26X2_HWAttrs hwAttrs;
} ECDSA_s_DynamicInstance;

/*
 * Used to store a secure copy of the dynamic instance (config plus the
 * object and hwAttrs that it points to) provided by non-secure calls to
 * ECDSA_construct.
 */
static ECDSA_s_DynamicInstance ECDSA_s_dynInstance[CONFIG_ECDSA_S_CONFIG_POOL_SIZE];

/* Stores pointers to non-secure ECDSA_s_SecureCallbacks for each driver
 * instance opened or constructed */
static ECDSA_s_SecureCallback *ECDSA_s_secureCB[ECDSA_SECURE_CALLBACK_COUNT];

/* Static driver instances allocated by SysConfig */
extern const ECDSA_Config ECDSA_config[];

/*
 *  ======== ECDSA_s_getCallbackIndex ========
 *  Returns callback index or -1 if index is not found.
 */
static int8_t ECDSA_s_getCallbackIndex(ECDSA_Handle handle_s)
{
    uint_fast8_t index;
    int8_t retIndex = -1;
    bool indexFound = false;

    /* First, search config statically allocated by SysConfig */
    for (index = 0; index < CONFIG_TI_DRIVERS_ECDSA_COUNT; index++)
    {
        if (handle_s == (ECDSA_Handle)&ECDSA_config[index])
        {
            indexFound = true;
            break;
        }
    }

    /* If not found, search secure config copies */
    if (!indexFound)
    {
        for (index = 0; index < CONFIG_ECDSA_S_CONFIG_POOL_SIZE; index++)
        {
            if (handle_s == &ECDSA_s_dynInstance[index].config)
            {
                index += CONFIG_TI_DRIVERS_ECDSA_COUNT;
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
 *  ======== ECDSA_s_hwiCallback ========
 */
static void ECDSA_s_hwiCallback(ECDSA_Handle handle_s,
                                int_fast16_t returnStatus,
                                ECDSA_Operation operation,
                                ECDSA_OperationType operationType)
{
    int8_t index;
    ECDSA_s_SecureCallback *ecdsaSecureCB_ns;

    index = ECDSA_s_getCallbackIndex(handle_s);

    if ((index >= 0) && (index < ECDSA_SECURE_CALLBACK_COUNT))
    {
        ecdsaSecureCB_ns = ECDSA_s_secureCB[index];

        if (ecdsaSecureCB_ns != NULL)
        {
            /* Store arguments in callback object for use by non-secure handler
             */
            ecdsaSecureCB_ns->handle        = (ECDSA_Handle)(CRYPTO_S_HANDLE_ID_ECDSA | index);
            ecdsaSecureCB_ns->returnStatus  = returnStatus;
            ecdsaSecureCB_ns->operation     = ECDSA_s_operation.operation_ns;
            ecdsaSecureCB_ns->operationType = operationType;

            /* Trigger the interrupt for the non-secure callback dispatcher */
            SecureCallback_post(&ecdsaSecureCB_ns->object);
        }
    }
}

/*
 *  ======== ECDSA_s_copyConfig ========
 */
static inline psa_status_t ECDSA_s_copyConfig(ECDSA_Config **secureConfig,
                                              const ECDSA_Config *config,
                                              ECDSA_Handle *retHandle)
{
    ECDSA_Config *config_s;
    ECDSA_s_DynamicInstance *dynInstance_s;
    uint_fast8_t i;

    for (i = 0; i < CONFIG_ECDSA_S_CONFIG_POOL_SIZE; i++)
    {
        dynInstance_s = &ECDSA_s_dynInstance[i];
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
            ECDSA_s_secureCB[i + CONFIG_TI_DRIVERS_ECDSA_COUNT] = NULL;

            /*
             * Return handle is the CRYPTO_S_HANDLE_ID_ECDSA OR'd with the
             * the config pool array index plus the size of constant config
             * array created by Sysconfig.
             */
            *retHandle = (ECDSA_Handle)(CRYPTO_S_HANDLE_ID_ECDSA | (i + CONFIG_TI_DRIVERS_ECDSA_COUNT));
            return PSA_SUCCESS;
        }
    }

    return PSA_ERROR_INSUFFICIENT_MEMORY;
}

/*
 *  ======== ECDSA_s_releaseConfig ========
 */
static inline void ECDSA_s_releaseConfig(ECDSA_Handle nsHandle)
{
    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_ECDSA)
    {
        /* Extract the secure handle index */
        uintptr_t i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        /* Check whether the handle instance refers to a dynamic instance */
        if ((i >= CONFIG_TI_DRIVERS_ECDSA_COUNT) && (i < ECDSA_SECURE_CALLBACK_COUNT))
        {
            /* Set config's object pointer to NULL to indicate the config is
             * available */
            ECDSA_s_dynInstance[i - CONFIG_TI_DRIVERS_ECDSA_COUNT].config.object = NULL;
        }
    }
}

/*
 *  ======== ECDSA_s_copySignOperation ========
 */
static inline psa_status_t ECDSA_s_copySignOperation(ECDSA_OperationSign *secureOperation,
                                                     CryptoKey *securePrivateKey,
                                                     const ECDSA_OperationSign *operation)
{
    const ECCParams_CurveParams *curveParams_s;
    int_fast16_t status;

    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(ECDSA_OperationSign)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(ECDSA_OperationSign));

    /* Get a pointer to secure curve params */
    curveParams_s = ECCParams_s_getCurveParams(secureOperation->curve);
    if (curveParams_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Update the operation struct to point to secure curve params */
    secureOperation->curve = curveParams_s;

    /* Validate hash address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)secureOperation->hash, curveParams_s->length) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Validate r component address range */
    if (cmse_has_unpriv_nonsecure_rw_access(secureOperation->r, curveParams_s->length) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Validate s component address range */
    if (cmse_has_unpriv_nonsecure_rw_access(secureOperation->s, curveParams_s->length) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /*
     * Make a secure copy of the private key struct and update the operation
     * struct to point to the secure key copy.
     */
    status = CryptoKey_copySecureInputKey(securePrivateKey, &secureOperation->myPrivateKey);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    return PSA_SUCCESS;
}

/*
 *  ======== ECDSA_s_copyVerifyOperation ========
 */
static inline psa_status_t ECDSA_s_copyVerifyOperation(ECDSA_OperationVerify *secureOperation,
                                                       CryptoKey *securePublicKey,
                                                       const ECDSA_OperationVerify *operation)
{
    const ECCParams_CurveParams *curveParams_s;
    int_fast16_t status;

    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(ECDSA_OperationVerify)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(ECDSA_OperationVerify));

    /* Get a pointer to secure curve params */
    curveParams_s = ECCParams_s_getCurveParams(secureOperation->curve);
    if (curveParams_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Update the operation struct to point to secure curve params */
    secureOperation->curve = curveParams_s;

    /* Validate hash address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)secureOperation->hash, curveParams_s->length) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Validate r component address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)secureOperation->r, curveParams_s->length) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Validate s component address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)secureOperation->s, curveParams_s->length) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /*
     * Make a secure copy of the public key struct and update the operation
     * struct to point to the secure key copy.
     */
    status = CryptoKey_copySecureInputKey(securePublicKey, &secureOperation->theirPublicKey);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    return PSA_SUCCESS;
}

/*
 *  ======== ECDSA_s_copyParams ========
 */
static psa_status_t ECDSA_s_copyParams(ECDSA_Params *secureParams, const ECDSA_Params *params)
{
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Validate params address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)params, sizeof(ECDSA_Params)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    (void)spm_memcpy(secureParams, params, sizeof(ECDSA_Params));

    /* Validate the return behavior */
    if ((secureParams->returnBehavior == ECDSA_RETURN_BEHAVIOR_CALLBACK) ||
        (secureParams->returnBehavior == ECDSA_RETURN_BEHAVIOR_BLOCKING) ||
        (secureParams->returnBehavior == ECDSA_RETURN_BEHAVIOR_POLLING))
    {
        /*
         * Overwrite the non-secure client's callback function with our own
         * callback which will populate the secure callback object registered
         * using ECDSA_S_MSG_TYPE_REGISTER_CALLBACK.
         */
        secureParams->callbackFxn = ECDSA_s_hwiCallback;

        /*
         * The underlying ECDSA driver is interrupt-driven regardless of the
         * return behavior specified. Since secure partitions cannot process
         * interrupt signals while a PSA call is running, callback return
         * behavior must be forced in all app-specified return behaviors
         * including polling.
         */
        secureParams->returnBehavior = ECDSA_RETURN_BEHAVIOR_CALLBACK;

        status = PSA_SUCCESS;
    }

    return status;
}

/*
 *  ======== ECDSA_s_getHandle ========
 *  Returns a Secure handle from a handle provided from non-secure client.
 */
static ECDSA_Handle ECDSA_s_getHandle(ECDSA_Handle nsHandle)
{
    ECDSA_Handle secureHandle = NULL;
    uint32_t i;

    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_ECDSA)
    {
        /* Extract the secure handle index */
        i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        if (i < CONFIG_TI_DRIVERS_ECDSA_COUNT)
        {
            secureHandle = (ECDSA_Handle)&ECDSA_config[i];
        }
        else if ((i >= CONFIG_TI_DRIVERS_ECDSA_COUNT) && (i < ECDSA_SECURE_CALLBACK_COUNT))
        {
            secureHandle = &ECDSA_s_dynInstance[i - CONFIG_TI_DRIVERS_ECDSA_COUNT].config;
        }
    }

    return secureHandle;
}

/*
 *  ======== ECDSA_s_registerCallback ========
 */
static inline psa_status_t ECDSA_s_registerCallback(psa_msg_t *msg)
{
    ECDSA_Handle handle_s;
    ECDSA_s_CallbackMsg callbackMsg;
    int8_t callbackIndex;
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Only non-secure callers should be registering callbacks */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id) && (msg->in_size[0] == sizeof(callbackMsg)))
    {
        psa_read(msg->handle, 0, &callbackMsg, sizeof(callbackMsg));

        handle_s = ECDSA_s_getHandle(callbackMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        callbackIndex = ECDSA_s_getCallbackIndex(handle_s);

        /* Validate index and callback object address range */
        if ((callbackIndex >= 0) && (callbackIndex < ECDSA_SECURE_CALLBACK_COUNT) &&
            (cmse_has_unpriv_nonsecure_rw_access(callbackMsg.callback, sizeof(ECDSA_s_SecureCallback)) != NULL))
        {
            /*
             * Store the pointer to ECDSA_s_SecureCallback located in
             * non-secure memory.
             */
            ECDSA_s_secureCB[callbackIndex] = callbackMsg.callback;

            status = PSA_SUCCESS;
        }
    }

    return status;
}

/*
 *  ======== ECDSA_s_construct ========
 */
static inline psa_status_t ECDSA_s_construct(psa_msg_t *msg)
{
    ECDSA_s_ConstructMsg constructMsg;
    ECDSA_Handle handle;
    ECDSA_Params params_s;
    const ECDSA_Params *paramsPtr_s = NULL;
    ECDSA_Config *configPtr_s;
    psa_status_t status    = PSA_ERROR_PROGRAMMER_ERROR;
    ECDSA_Handle retHandle = NULL;

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
             * Copy params to secure memory and force callback mode,
             * substituting our own callback function.
             */
            status = ECDSA_s_copyParams(&params_s, constructMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }

            paramsPtr_s = &params_s;
        }

        /* Verify config and copy to secure memory */
        status = ECDSA_s_copyConfig(&configPtr_s, constructMsg.config, &retHandle);
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

    handle = ECDSA_construct(configPtr_s, paramsPtr_s);

    if (handle == NULL)
    {
        retHandle = NULL;

        if (TFM_CLIENT_ID_IS_NS(msg->client_id))
        {
            /* Set config's object pointer to NULL to indicate the config is
             * available */
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
 *  ======== ECDSA_s_open ========
 */
static inline psa_status_t ECDSA_s_open(psa_msg_t *msg)
{
    ECDSA_s_OpenMsg openMsg;
    ECDSA_Handle handle;
    ECDSA_Params params_s;
    ECDSA_Params *paramsPtr_s = NULL;
    ECDSA_Handle retHandle    = NULL;
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
            /*
             * Copy params to secure memory and force callback mode,
             * substituting our own callback function.
             */
            status = ECDSA_s_copyParams(&params_s, openMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }
        }

        paramsPtr_s = &params_s;
    }

    handle = ECDSA_open(openMsg.index, paramsPtr_s);

    if (handle != NULL)
    {
        if (TFM_CLIENT_ID_IS_NS(msg->client_id))
        {
            /*
             * Set the secure callback object pointer to NULL. The non-secure
             * client must register a secure callback after constructing or
             * opening a driver instance.
             */
            ECDSA_s_secureCB[openMsg.index] = NULL;

            /*
             * Return CRYPTO_S_HANDLE_ID_ECDSA OR'd with the config array index
             * as the handle instead of a pointer to the config to allow for
             * validation of handles provided by non-secure clients.
             */
            retHandle = (ECDSA_Handle)(CRYPTO_S_HANDLE_ID_ECDSA | openMsg.index);
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
 *  ======== ECDSA_s_close ========
 */
static inline psa_status_t ECDSA_s_close(psa_msg_t *msg)
{
    ECDSA_Handle handle_s;
    ECDSA_s_CloseMsg closeMsg;

    if (msg->in_size[0] != sizeof(closeMsg))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &closeMsg, sizeof(closeMsg));

    /* Non-secure callers have negative client ID */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = ECDSA_s_getHandle(closeMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        ECDSA_close(handle_s);

        /* Release the secure config if it is a dynamic */
        {
            ECDSA_s_releaseConfig(closeMsg.handle);
        }
    }
    else /* Secure client */
    {
        ECDSA_close(closeMsg.handle);
    }

    return PSA_SUCCESS;
}

/*
 *  ======== ECDSA_s_sign ========
 */
static inline psa_status_t ECDSA_s_sign(psa_msg_t *msg)
{
    ECDSA_s_SignMsg signMsg;
    ECDSA_Handle handle_s;
    ECDSA_OperationSign *operation_s;
    CryptoKey *privateKey_s = &ECDSA_s_operation.key_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(signMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &signMsg, sizeof(signMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = ECDSA_s_getHandle(signMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &ECDSA_s_operation.operation_s.sign;

        /* Save pointer to non-secure operation struct */
        ECDSA_s_operation.operation_ns = (ECDSA_Operation)signMsg.operation;

        /* Validate and copy operation and key structs */
        status = ECDSA_s_copySignOperation(operation_s, privateKey_s, signMsg.operation);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = ECDSA_sign(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = ECDSA_sign(signMsg.handle, signMsg.operation);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== ECDSA_s_verify ========
 */
static inline psa_status_t ECDSA_s_verify(psa_msg_t *msg)
{
    ECDSA_s_VerifyMsg verifyMsg;
    ECDSA_Handle handle_s;
    ECDSA_OperationVerify *operation_s;
    CryptoKey *publicKey_s = &ECDSA_s_operation.key_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(verifyMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &verifyMsg, sizeof(verifyMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = ECDSA_s_getHandle(verifyMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &ECDSA_s_operation.operation_s.verify;

        /* Save pointer to non-secure operation struct */
        ECDSA_s_operation.operation_ns = (ECDSA_Operation)verifyMsg.operation;

        /* Validate and copy operation and key structs */
        status = ECDSA_s_copyVerifyOperation(operation_s, publicKey_s, verifyMsg.operation);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = ECDSA_verify(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = ECDSA_verify(verifyMsg.handle, verifyMsg.operation);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== ECDSA_s_cancelOperation ========
 */
static inline psa_status_t ECDSA_s_cancelOperation(psa_msg_t *msg)
{
    ECDSA_Handle handle_s;
    ECDSA_s_CancelOperationMsg cancelMsg;
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

    handle_s = ECDSA_s_getHandle(cancelMsg.handle);
    if (handle_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    ret = ECDSA_cancelOperation(handle_s);

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== ECDSA_s_handlePsaMsg ========
 */
psa_status_t ECDSA_s_handlePsaMsg(psa_msg_t *msg)
{
    psa_status_t status = PSA_SUCCESS;

    switch (msg->type)
    {
        /*
         * If ECDSA_S_MSG_TYPE_CONSTRUCT is used by other secure partitions,
         * any pointer arguments provided must reference memory accessible by
         * this partition which is dependent on the TF-M isolation level in use.
         */
        case ECDSA_S_MSG_TYPE_CONSTRUCT:
            status = ECDSA_s_construct(msg);
            break;

        case ECDSA_S_MSG_TYPE_OPEN:
            status = ECDSA_s_open(msg);
            break;

        /*
         * ECDSA_S_MSG_TYPE_REGISTER_CALLBACK is designed to be used non-secure
         * callers only. Secure callers must only use polling return behavior.
         */
        case ECDSA_S_MSG_TYPE_REGISTER_CALLBACK:
            status = ECDSA_s_registerCallback(msg);
            break;

        case ECDSA_S_MSG_TYPE_CLOSE:
            status = ECDSA_s_close(msg);
            break;

        case ECDSA_S_MSG_TYPE_SIGN:
            status = ECDSA_s_sign(msg);
            break;

        case ECDSA_S_MSG_TYPE_VERIFY:
            status = ECDSA_s_verify(msg);
            break;

        /*
         * ECDSA_S_MSG_TYPE_CANCEL_OPERATION supported for non-secure clients
         * only. Secure callers must only use polling return behavior.
         */
        case ECDSA_S_MSG_TYPE_CANCEL_OPERATION:
            status = ECDSA_s_cancelOperation(msg);
            break;

        default:
            /* Unknown msg type */
            status = PSA_ERROR_PROGRAMMER_ERROR;
            break;
    }

    return status;
}

/*
 *  ======== ECDSA_s_init ========
 */
void ECDSA_s_init(void)
{
    ECDSA_init();
}
