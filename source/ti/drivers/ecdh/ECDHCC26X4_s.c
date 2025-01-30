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

#include "ECDHCC26X4_s.h"

#include <ti/drivers/ECDH.h>
#include <ti/drivers/ecdh/ECDHCC26X2.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>
#include <ti/drivers/cryptoutils/ecc/ECCParamsCC26X4_s.h>

#include <ti/drivers/tfm/SecureCallback.h>

#include <third_party/tfm/secure_fw/spm/core/spm.h>
#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/interface/include/psa/service.h>
#include <third_party/tfm/secure_fw/spm/include/utilities.h>

#include <third_party/tfm/platform/ext/target/ti/cc26x4/cmse.h> /* TI CMSE helper functions */
#include "ti_drivers_config.h"                                  /* Sysconfig generated header */

/*!
 *  @brief  Union containing all supported operation structs.
 */
typedef union
{
    ECDH_OperationGeneratePublicKey generatePublicKey;
    ECDH_OperationComputeSharedSecret computeSharedSecret;
} ECDH_OperationUnion;

/*
 * Stores the following:
 * - Secure copy of the operation and the original pointer to the non-secure
 *   operation to return when the HWI callback occurs.
 * - Secure copy of the result key and the original pointer to the non-secure
 *   result key to update the key encoding return when the HWI callback occurs.
 *
 * Note: Input keys are copied into PKA RAM before starting the ECDH driver
 * FSM so it is not required to keep copies of those keys.
 */
typedef struct
{
    ECDH_OperationUnion operation_s; /* Secure copy of operation */
    ECDH_Operation operation_ns;     /* Pointer to non-secure operation */
    CryptoKey resultKey_s;           /* Secure copy of result key */
    CryptoKey *resultKey_ns;         /* Pointer to non-secure result key */
} ECDH_s_Operation;

static ECDH_s_Operation ECDH_s_operation;

/*
 * ECDH Secure Dynamic Instance struct.
 */
typedef struct
{
    ECDH_Config config;
    ECDHCC26X2_Object object;
    ECDHCC26X2_HWAttrs hwAttrs;
} ECDH_s_DynamicInstance;

/*
 * Used to store a secure copy of the dynamic instance (config plus the
 * object and hwAttrs that it points to) provided by non-secure calls to
 * ECDH_construct.
 */
static ECDH_s_DynamicInstance ECDH_s_dynInstance[CONFIG_ECDH_S_CONFIG_POOL_SIZE];

/* Stores pointers to non-secure ECDH_s_SecureCallbacks for each driver instance opened or constructed */
static ECDH_s_SecureCallback *ECDH_s_secureCB[ECDH_SECURE_CALLBACK_COUNT];

/* Static driver instances allocated by SysConfig */
extern const ECDH_Config ECDH_config[];

/*
 *  ======== ECDH_s_getCallbackIndex ========
 *  Returns callback index or -1 if index is not found.
 */
static int8_t ECDH_s_getCallbackIndex(ECDH_Handle handle_s)
{
    uint_fast8_t index;
    int8_t retIndex = -1;
    bool indexFound = false;

    /* First, search config statically allocated by SysConfig */
    for (index = 0; index < CONFIG_TI_DRIVERS_ECDH_COUNT; index++)
    {
        if (handle_s == (ECDH_Handle)&ECDH_config[index])
        {
            indexFound = true;
            break;
        }
    }

    /* If not found, search secure config copies */
    if (!indexFound)
    {
        for (index = 0; index < CONFIG_ECDH_S_CONFIG_POOL_SIZE; index++)
        {
            if (handle_s == &ECDH_s_dynInstance[index].config)
            {
                index += CONFIG_TI_DRIVERS_ECDH_COUNT;
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
 *  ======== ECDH_s_hwiCallback ========
 */
static void ECDH_s_hwiCallback(ECDH_Handle handle_s,
                               int_fast16_t returnStatus,
                               ECDH_Operation operation,
                               ECDH_OperationType operationType)
{
    int8_t index;
    ECDH_s_SecureCallback *ecdhSecureCB_ns;

    index = ECDH_s_getCallbackIndex(handle_s);

    if ((index >= 0) && (index < ECDH_SECURE_CALLBACK_COUNT))
    {
        ecdhSecureCB_ns = ECDH_s_secureCB[index];

        if (ecdhSecureCB_ns != NULL)
        {
            /* Store arguments in callback object for use by non-secure handler */
            ecdhSecureCB_ns->handle        = (ECDH_Handle)(CRYPTO_S_HANDLE_ID_ECDH | index);
            ecdhSecureCB_ns->returnStatus  = returnStatus;
            ecdhSecureCB_ns->operation     = ECDH_s_operation.operation_ns;
            ecdhSecureCB_ns->operationType = operationType;

            /* Copy updated key encoding to the non-secure key struct */
            if (operationType == ECDH_OPERATION_TYPE_GENERATE_PUBLIC_KEY)
            {
                ECDH_s_operation.resultKey_ns->encoding = operation.generatePublicKey->myPublicKey->encoding;

                if (ECDH_s_operation.resultKey_ns->encoding == CryptoKey_KEYSTORE)
                {
                    /* Copy the updated keyID to the non-secure public key struct */
                    ECDH_s_operation.resultKey_ns->u.keyStore.keyID = operation.generatePublicKey->myPublicKey->u
                                                                          .keyStore.keyID;
                }
            }
            else
            {
                ECDH_s_operation.resultKey_ns->encoding = operation.computeSharedSecret->sharedSecret->encoding;

                if (ECDH_s_operation.resultKey_ns->encoding == CryptoKey_KEYSTORE)
                {
                    /* Copy the updated keyID to the non-secure shared secret struct */
                    ECDH_s_operation.resultKey_ns->u.keyStore.keyID = operation.computeSharedSecret->sharedSecret->u
                                                                          .keyStore.keyID;
                }
            }

            /* Trigger the interrupt for the non-secure callback dispatcher */
            SecureCallback_post(&ecdhSecureCB_ns->object);
        }
    }
}

/*
 *  ======== ECDH_s_copyConfig ========
 */
static inline psa_status_t ECDH_s_copyConfig(ECDH_Config **secureConfig,
                                             const ECDH_Config *config,
                                             ECDH_Handle *retHandle)
{
    ECDH_Config *config_s;
    ECDH_s_DynamicInstance *dynInstance_s;
    uint_fast8_t i;

    for (i = 0; i < CONFIG_ECDH_S_CONFIG_POOL_SIZE; i++)
    {
        dynInstance_s = &ECDH_s_dynInstance[i];
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
            ECDH_s_secureCB[i + CONFIG_TI_DRIVERS_ECDH_COUNT] = NULL;

            /*
             * Return handle is the CRYPTO_S_HANDLE_ID_ECDH OR'd with the
             * the config pool array index plus the size of constant config
             * array created by Sysconfig.
             */
            *retHandle = (ECDH_Handle)(CRYPTO_S_HANDLE_ID_ECDH | (i + CONFIG_TI_DRIVERS_ECDH_COUNT));
            return PSA_SUCCESS;
        }
    }

    return PSA_ERROR_INSUFFICIENT_MEMORY;
}

/*
 *  ======== ECDH_s_releaseConfig ========
 */
static inline void ECDH_s_releaseConfig(ECDH_Handle nsHandle)
{
    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_ECDH)
    {
        /* Extract the secure handle index */
        uintptr_t i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        /* Check whether the handle instance refers to a dynamic instance */
        if ((i >= CONFIG_TI_DRIVERS_ECDH_COUNT) && (i < ECDH_SECURE_CALLBACK_COUNT))
        {
            /* Set config's object pointer to NULL to indicate the config is available */
            ECDH_s_dynInstance[i - CONFIG_TI_DRIVERS_ECDH_COUNT].config.object = NULL;
        }
    }
}

/*
 *  ======== ECDH_s_copyGenPublicKeyOperation ========
 */
static inline psa_status_t ECDH_s_copyGenPublicKeyOperation(ECDH_OperationGeneratePublicKey *secureOperation,
                                                            CryptoKey *securePrivateKey,
                                                            CryptoKey *securePublicKey,
                                                            const ECDH_OperationGeneratePublicKey *operation)
{
    const ECCParams_CurveParams *curveParams_s;
    int_fast16_t status;

    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(ECDH_OperationGeneratePublicKey)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(ECDH_OperationGeneratePublicKey));

    /* Get a pointer to secure curve params */
    curveParams_s = ECCParams_s_getCurveParams(secureOperation->curve);
    if (curveParams_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Update the operation struct to point to secure curve params */
    secureOperation->curve = curveParams_s;

    /*
     * Make a secure copy of the private key struct and update the operation
     * struct to point to the secure key copy. Cast is required since key is
     * const data.
     */
    status = CryptoKey_copySecureInputKey(securePrivateKey, &secureOperation->myPrivateKey);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /*
     * Make a secure copy of the public key struct and update the operation
     * struct to point to the secure key copy.
     */
    status = CryptoKey_copySecureOutputKey(securePublicKey, &secureOperation->myPublicKey);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    return PSA_SUCCESS;
}

/*
 *  ======== ECDH_s_copyComputeSharedSecretOperation ========
 */
static inline psa_status_t ECDH_s_copyComputeSharedSecretOperation(ECDH_OperationComputeSharedSecret *secureOperation,
                                                                   CryptoKey *securePrivateKey,
                                                                   CryptoKey *securePublicKey,
                                                                   CryptoKey *secureSharedSecret,
                                                                   const ECDH_OperationComputeSharedSecret *operation)
{
    const ECCParams_CurveParams *curveParams_s;
    int_fast16_t status;

    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(ECDH_OperationComputeSharedSecret)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(ECDH_OperationComputeSharedSecret));

    /* Get a pointer to secure curve params */
    curveParams_s = ECCParams_s_getCurveParams(secureOperation->curve);
    if (curveParams_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Update the operation struct to point to secure curve params */
    secureOperation->curve = curveParams_s;

    /*
     * Make a secure copy of the private key struct and update the operation
     * struct to point to the secure key copy. Cast is required since key is
     * const data.
     */
    status = CryptoKey_copySecureInputKey(securePrivateKey, &secureOperation->myPrivateKey);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /*
     * Make a secure copy of the public key struct and update the operation
     * struct to point to the secure key copy. Cast is required since key is
     * const data.
     */
    status = CryptoKey_copySecureInputKey(securePublicKey, &secureOperation->theirPublicKey);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /*
     * Make a secure copy of the shared secret key struct and update the operation
     * struct to point to the secure key copy.
     */
    status = CryptoKey_copySecureOutputKey(secureSharedSecret, &secureOperation->sharedSecret);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    return PSA_SUCCESS;
}

/*
 *  ======== ECDH_s_copyParams ========
 */
static psa_status_t ECDH_s_copyParams(ECDH_Params *secureParams, const ECDH_Params *params)
{
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Validate params address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)params, sizeof(ECDH_Params)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    (void)spm_memcpy(secureParams, params, sizeof(ECDH_Params));

    /* Validate the return behavior */
    if ((secureParams->returnBehavior == ECDH_RETURN_BEHAVIOR_CALLBACK) ||
        (secureParams->returnBehavior == ECDH_RETURN_BEHAVIOR_BLOCKING) ||
        (secureParams->returnBehavior == ECDH_RETURN_BEHAVIOR_POLLING))
    {
        /*
         * Overwrite the non-secure client's callback function with our own
         * callback which will populate the secure callback object registered
         * using ECDH_S_MSG_TYPE_REGISTER_CALLBACK.
         */
        secureParams->callbackFxn = ECDH_s_hwiCallback;

        /*
         * The underlying ECDH driver is interrupt-driven regardless of the return
         * behavior specified. Since secure partitions cannot process interrupt
         * signals while a PSA call is running, callback return behavior must be
         * forced for all app-specified return behaviors including polling.
         */
        secureParams->returnBehavior = ECDH_RETURN_BEHAVIOR_CALLBACK;

        status = PSA_SUCCESS;
    }

    return status;
}

/*
 *  ======== ECDH_s_getHandle ========
 *  Returns a Secure handle from a handle provided from non-secure client.
 */
static ECDH_Handle ECDH_s_getHandle(ECDH_Handle nsHandle)
{
    ECDH_Handle secureHandle = NULL;
    uint32_t i;

    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_ECDH)
    {
        /* Extract the secure handle index */
        i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        if (i < CONFIG_TI_DRIVERS_ECDH_COUNT)
        {
            secureHandle = (ECDH_Handle)&ECDH_config[i];
        }
        else if ((i >= CONFIG_TI_DRIVERS_ECDH_COUNT) && (i < ECDH_SECURE_CALLBACK_COUNT))
        {
            secureHandle = &ECDH_s_dynInstance[i - CONFIG_TI_DRIVERS_ECDH_COUNT].config;
        }
    }

    return secureHandle;
}

/*
 *  ======== ECDH_s_registerCallback ========
 */
static inline psa_status_t ECDH_s_registerCallback(psa_msg_t *msg)
{
    ECDH_Handle handle_s;
    ECDH_s_CallbackMsg callbackMsg;
    int8_t callbackIndex;
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Only non-secure callers should be registering callbacks */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id) && (msg->in_size[0] == sizeof(callbackMsg)))
    {
        psa_read(msg->handle, 0, &callbackMsg, sizeof(callbackMsg));

        handle_s = ECDH_s_getHandle(callbackMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        callbackIndex = ECDH_s_getCallbackIndex(handle_s);

        /* Validate index and callback object address range */
        if ((callbackIndex >= 0) && (callbackIndex < ECDH_SECURE_CALLBACK_COUNT) &&
            (cmse_has_unpriv_nonsecure_rw_access(callbackMsg.callback, sizeof(ECDH_s_SecureCallback)) != NULL))
        {
            /*
             * Store the pointer to ECDH_s_SecureCallback located in
             * non-secure memory.
             */
            ECDH_s_secureCB[callbackIndex] = callbackMsg.callback;

            status = PSA_SUCCESS;
        }
    }

    return status;
}

/*
 *  ======== ECDH_s_construct ========
 */
static inline psa_status_t ECDH_s_construct(psa_msg_t *msg)
{
    ECDH_s_ConstructMsg constructMsg;
    ECDH_Handle handle;
    ECDH_Params params_s;
    const ECDH_Params *paramsPtr_s = NULL;
    ECDH_Config *configPtr_s;
    psa_status_t status   = PSA_ERROR_PROGRAMMER_ERROR;
    ECDH_Handle retHandle = NULL;

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
            status = ECDH_s_copyParams(&params_s, constructMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }

            paramsPtr_s = &params_s;
        }

        /* Verify config and copy to secure memory */
        status = ECDH_s_copyConfig(&configPtr_s, constructMsg.config, &retHandle);
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

    handle = ECDH_construct(configPtr_s, paramsPtr_s);

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
 *  ======== ECDH_s_open ========
 */
static inline psa_status_t ECDH_s_open(psa_msg_t *msg)
{
    ECDH_s_OpenMsg openMsg;
    ECDH_Handle handle;
    ECDH_Params params_s;
    ECDH_Params *paramsPtr_s = NULL;
    ECDH_Handle retHandle    = NULL;
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
            status = ECDH_s_copyParams(&params_s, openMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }
        }

        paramsPtr_s = &params_s;
    }

    handle = ECDH_open(openMsg.index, paramsPtr_s);

    if (handle != NULL)
    {
        if (TFM_CLIENT_ID_IS_NS(msg->client_id))
        {
            /*
             * Set the secure callback object pointer to NULL. The non-secure
             * client must register a secure callback after constructing or
             * opening a driver instance.
             */
            ECDH_s_secureCB[openMsg.index] = NULL;

            /*
             * Return CRYPTO_S_HANDLE_ID_ECDH OR'd with the config array index
             * as the handle instead of a pointer to the config to allow for
             * validation of handles provided by non-secure clients.
             */
            retHandle = (ECDH_Handle)(CRYPTO_S_HANDLE_ID_ECDH | openMsg.index);
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
 *  ======== ECDH_s_close ========
 */
static inline psa_status_t ECDH_s_close(psa_msg_t *msg)
{
    ECDH_Handle handle_s;
    ECDH_s_CloseMsg closeMsg;

    if (msg->in_size[0] != sizeof(closeMsg))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &closeMsg, sizeof(closeMsg));

    /* Non-secure callers have negative client ID */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = ECDH_s_getHandle(closeMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        ECDH_close(handle_s);

        /* Release the secure config if it is a dynamic */
        ECDH_s_releaseConfig(closeMsg.handle);
    }
    else /* Secure client */
    {
        ECDH_close(closeMsg.handle);
    }

    return PSA_SUCCESS;
}

/*
 *  ======== ECDH_s_generatePublicKey ========
 */
static inline psa_status_t ECDH_s_generatePublicKey(psa_msg_t *msg)
{
    ECDH_s_GeneratePublicKeyMsg genPubKeyMsg;
    ECDH_Handle handle_s;
    ECDH_OperationGeneratePublicKey *operation_s;
    CryptoKey privateKey_s;
    CryptoKey *publicKey_s = &ECDH_s_operation.resultKey_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(genPubKeyMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &genPubKeyMsg, sizeof(genPubKeyMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = ECDH_s_getHandle(genPubKeyMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &ECDH_s_operation.operation_s.generatePublicKey;

        /* Save pointer to non-secure operation struct */
        ECDH_s_operation.operation_ns = (ECDH_Operation)genPubKeyMsg.operation;

        /* Save pointer to non-secure public key */
        ECDH_s_operation.resultKey_ns = genPubKeyMsg.operation->myPublicKey;

        /* Validate and copy operation and key structs */
        status = ECDH_s_copyGenPublicKeyOperation(operation_s, &privateKey_s, publicKey_s, genPubKeyMsg.operation);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = ECDH_generatePublicKey(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = ECDH_generatePublicKey(genPubKeyMsg.handle, genPubKeyMsg.operation);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== ECDH_s_computeSharedSecret ========
 */
static inline psa_status_t ECDH_s_computeSharedSecret(psa_msg_t *msg)
{
    ECDH_s_ComputeSharedSecretMsg compSharedSecretMsg;
    ECDH_Handle handle_s;
    ECDH_OperationComputeSharedSecret *operation_s;
    CryptoKey privateKey_s;
    CryptoKey publicKey_s;
    CryptoKey *sharedSecret_s = &ECDH_s_operation.resultKey_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(compSharedSecretMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &compSharedSecretMsg, sizeof(compSharedSecretMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = ECDH_s_getHandle(compSharedSecretMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &ECDH_s_operation.operation_s.computeSharedSecret;

        /* Save pointer to non-secure operation struct */
        ECDH_s_operation.operation_ns = (ECDH_Operation)compSharedSecretMsg.operation;

        /* Save pointer to non-secure shared secret key */
        ECDH_s_operation.resultKey_ns = compSharedSecretMsg.operation->sharedSecret;

        /* Validate and copy operation and key structs */
        status = ECDH_s_copyComputeSharedSecretOperation(operation_s,
                                                         &privateKey_s,
                                                         &publicKey_s,
                                                         sharedSecret_s,
                                                         compSharedSecretMsg.operation);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = ECDH_computeSharedSecret(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = ECDH_computeSharedSecret(compSharedSecretMsg.handle, compSharedSecretMsg.operation);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== ECDH_s_cancelOperation ========
 */
static inline psa_status_t ECDH_s_cancelOperation(psa_msg_t *msg)
{
    ECDH_Handle handle_s;
    ECDH_s_CancelOperationMsg cancelMsg;
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

    handle_s = ECDH_s_getHandle(cancelMsg.handle);
    if (handle_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    ret = ECDH_cancelOperation(handle_s);

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== ECDH_s_handlePsaMsg ========
 */
psa_status_t ECDH_s_handlePsaMsg(psa_msg_t *msg)
{
    psa_status_t status = PSA_SUCCESS;

    switch (msg->type)
    {
        /*
         * If ECDH_S_MSG_TYPE_CONSTRUCT is used by other secure partitions,
         * any pointer arguments provided must reference memory accessible by
         * this partition which is dependent on the TF-M isolation level in use.
         */
        case ECDH_S_MSG_TYPE_CONSTRUCT:
            status = ECDH_s_construct(msg);
            break;

        case ECDH_S_MSG_TYPE_OPEN:
            status = ECDH_s_open(msg);
            break;

        /*
         * ECDH_S_MSG_TYPE_REGISTER_CALLBACK is designed to be used non-secure
         * callers only. Secure callers must only use polling return behavior.
         */
        case ECDH_S_MSG_TYPE_REGISTER_CALLBACK:
            status = ECDH_s_registerCallback(msg);
            break;

        case ECDH_S_MSG_TYPE_CLOSE:
            status = ECDH_s_close(msg);
            break;

        case ECDH_S_MSG_TYPE_GENERATE_PUBLIC_KEY:
            status = ECDH_s_generatePublicKey(msg);
            break;

        case ECDH_S_MSG_TYPE_COMPUTE_SHARED_SECRET:
            status = ECDH_s_computeSharedSecret(msg);
            break;

        /*
         * ECDH_S_MSG_TYPE_CANCEL_OPERATION supported for non-secure clients
         * only. Secure callers must only use polling return behavior.
         */
        case ECDH_S_MSG_TYPE_CANCEL_OPERATION:
            status = ECDH_s_cancelOperation(msg);
            break;

        default:
            /* Unknown msg type */
            status = PSA_ERROR_PROGRAMMER_ERROR;
            break;
    }

    return status;
}

/*
 *  ======== ECDH_s_init ========
 */
void ECDH_s_init(void)
{
    ECDH_init();
}
