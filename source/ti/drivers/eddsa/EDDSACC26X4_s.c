/*
 * Copyright (c) 2024, Texas Instruments Incorporated - http://www.ti.com
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

#include "EDDSACC26X4_s.h"

#include <ti/drivers/EDDSA.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>
#include <ti/drivers/cryptoutils/ecc/ECCParamsCC26X4_s.h>
#include <ti/drivers/eddsa/EDDSACC26X2.h>

#include <ti/drivers/tfm/SecureCallback.h>

#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/interface/include/psa/service.h>
#include <third_party/tfm/secure_fw/spm/core/spm.h>
#include <third_party/tfm/secure_fw/spm/include/utilities.h>
#include <third_party/tfm/platform/ext/target/ti/cc26x4/cmse.h> /* TI CMSE helper functions */

#include "ti_drivers_config.h" /* Sysconfig generated header */

#define EDDSA_SECURE_OPERATION_COUNT EDDSA_SECURE_CALLBACK_COUNT

/*!
 *  @brief  Union containing all supported operation structs.
 */
typedef union
{
    EDDSA_OperationGeneratePublicKey generatePublicKey;
    EDDSA_OperationSign sign;
    EDDSA_OperationVerify verify;
} EDDSA_OperationUnion;

/*
 * Stores the following:
 * - Secure copy of the operation and the original pointer to the non-secure
 *   operation to return when the HWI callback occurs.
 * - Secure copy of the public & private keys and the original pointer to the non-secure
 *   public key to update the key encoding return when the HWI callback occurs.
 */
typedef struct
{
    EDDSA_Operation operation_ns;     /* Pointer to non-secure operation */
    EDDSA_OperationUnion operation_s; /* Secure copy of operation */
    CryptoKey publicKey_s;            /* Secure copy of public key */
    CryptoKey privateKey_s;           /* Secure copy of private key (not used for verify operation) */
    CryptoKey *publicKey_ns;          /* Pointer to non-secure public key */
} EDDSA_s_Operation;

static EDDSA_s_Operation EDDSA_s_operation;

/*
 * EDDSA Secure Dynamic Instance struct.
 */
typedef struct
{
    EDDSA_Config config;
    EDDSACC26X2_Object object;
    EDDSACC26X2_HWAttrs hwAttrs;
} EDDSA_s_DynamicInstance;

/*
 * Used to store a secure copy of the dynamic instance (config plus the
 * object and hwAttrs that it points to) provided by non-secure calls to
 * EDDSA_construct.
 */
static EDDSA_s_DynamicInstance EDDSA_s_dynInstance[CONFIG_EDDSA_S_CONFIG_POOL_SIZE];

/* Stores pointers to non-secure EDDSA_s_SecureCallbacks for each driver instance opened or constructed */
static EDDSA_s_SecureCallback *EDDSA_s_secureCB[EDDSA_SECURE_CALLBACK_COUNT];

/* Static driver instances allocated by SysConfig */
extern const EDDSA_Config EDDSA_config[];

/*
 *  ======== EDDSA_s_getCallbackIndex ========
 *  Returns callback index or -1 if index is not found.
 */
static int8_t EDDSA_s_getCallbackIndex(EDDSA_Handle handle_s)
{
    uint_fast8_t index;
    int8_t retIndex = -1;
    bool indexFound = false;

    /* First, search config statically allocated by SysConfig */
    for (index = 0; index < CONFIG_TI_DRIVERS_EDDSA_COUNT; index++)
    {
        if (handle_s == (EDDSA_Handle)&EDDSA_config[index])
        {
            indexFound = true;
            break;
        }
    }

    /* If not found, search secure config copies */
    if (!indexFound)
    {
        for (index = 0; index < CONFIG_EDDSA_S_CONFIG_POOL_SIZE; index++)
        {
            if (handle_s == &EDDSA_s_dynInstance[index].config)
            {
                index += CONFIG_TI_DRIVERS_EDDSA_COUNT;
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
 *  ======== EDDSA_s_hwiCallback ========
 */
static void EDDSA_s_hwiCallback(EDDSA_Handle handle_s,
                                int_fast16_t returnStatus,
                                EDDSA_Operation operation,
                                EDDSA_OperationType operationType)
{
    EDDSA_s_SecureCallback *eddsaSecureCB_ns;
    int8_t index;

    index = EDDSA_s_getCallbackIndex(handle_s);

    if ((index >= 0) && (index < EDDSA_SECURE_CALLBACK_COUNT))
    {
        eddsaSecureCB_ns = EDDSA_s_secureCB[index];

        if (eddsaSecureCB_ns != NULL)
        {
            /* Store arguments in callback object for use by non-secure handler
             */
            eddsaSecureCB_ns->handle        = (EDDSA_Handle)(CRYPTO_S_HANDLE_ID_EDDSA | index);
            eddsaSecureCB_ns->returnStatus  = returnStatus;
            eddsaSecureCB_ns->operation     = EDDSA_s_operation.operation_ns;
            eddsaSecureCB_ns->operationType = operationType;

            if (operationType == EDDSA_OPERATION_TYPE_GENERATE_PUBLIC_KEY)
            {
                /* Copy updated key encoding to the non-secure public key struct */
                EDDSA_s_operation.publicKey_ns->encoding = operation.generatePublicKey->myPublicKey->encoding;

                if (EDDSA_s_operation.publicKey_ns->encoding == CryptoKey_KEYSTORE)
                {
                    /* Copy the updated keyID to the non-secure public key struct */
                    EDDSA_s_operation.publicKey_ns->u.keyStore.keyID = operation.generatePublicKey->myPublicKey->u
                                                                           .keyStore.keyID;
                }
            }

            /* Trigger the interrupt for the non-secure callback dispatcher */
            SecureCallback_post(&eddsaSecureCB_ns->object);
        }
    }
}

/*
 *  ======== EDDSA_s_copyConfig ========
 */
static inline psa_status_t EDDSA_s_copyConfig(EDDSA_Config **secureConfig,
                                              const EDDSA_Config *config,
                                              EDDSA_Handle *retHandle)
{
    EDDSA_Config *config_s;
    EDDSA_s_DynamicInstance *dynInstance_s;
    uint_fast8_t i;

    for (i = 0; i < CONFIG_EDDSA_S_CONFIG_POOL_SIZE; i++)
    {
        dynInstance_s = &EDDSA_s_dynInstance[i];
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
            EDDSA_s_secureCB[i + CONFIG_TI_DRIVERS_EDDSA_COUNT] = NULL;

            /*
             * Return handle is the CRYPTO_S_HANDLE_ID_EDDSA OR'd with the
             * the config pool array index plus the size of constant config
             * array created by Sysconfig.
             */
            *retHandle = (EDDSA_Handle)(CRYPTO_S_HANDLE_ID_EDDSA | (i + CONFIG_TI_DRIVERS_EDDSA_COUNT));
            return PSA_SUCCESS;
        }
    }

    return PSA_ERROR_INSUFFICIENT_MEMORY;
}

/*
 *  ======== EDDSA_s_releaseConfig ========
 */
static inline void EDDSA_s_releaseConfig(EDDSA_Handle nsHandle)
{
    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_EDDSA)
    {
        /* Extract the secure handle index */
        uintptr_t i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        /* Check whether the handle instance refers to a dynamic instance */
        if ((i >= CONFIG_TI_DRIVERS_EDDSA_COUNT) && (i < EDDSA_SECURE_CALLBACK_COUNT))
        {
            /* Set config's object pointer to NULL to indicate the config is available */
            EDDSA_s_dynInstance[i - CONFIG_TI_DRIVERS_EDDSA_COUNT].config.object = NULL;
        }
    }
}

/*
 *  ======== EDDSA_s_copyGenPublicKeyOperation ========
 */
static inline psa_status_t EDDSA_s_copyGenPublicKeyOperation(EDDSA_OperationGeneratePublicKey *secureOperation,
                                                             CryptoKey *securePublicKey,
                                                             CryptoKey *securePrivateKey,
                                                             const EDDSA_OperationGeneratePublicKey *operation)
{
    const ECCParams_CurveParams *curveParams_s;
    ECCParams_ns_CurveParams curveParams_ns;
    int_fast16_t status;

    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(EDDSA_OperationGeneratePublicKey)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(EDDSA_OperationGeneratePublicKey));

    /* Make a local copy of the param struct to avoid typecast */
    (void)spm_memcpy(&curveParams_ns, (void *)secureOperation->curve, sizeof(curveParams_ns));

    /* Validate curve is Ed25519 */
    if (curveParams_ns.secureCurve != ECCParams_SecureCurve_Ed25519)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Get pointer to secure curve params */
    curveParams_s = ECCParams_s_getCurveParams(secureOperation->curve);
    if (curveParams_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Update the operation struct to point to secure curve params */
    secureOperation->curve = curveParams_s;

    /*
     * Make a secure copy of the private key struct and update the operation
     * struct to point to the secure key copy.
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
 *  ======== EDDSA_s_copySignOperation ========
 */
static inline psa_status_t EDDSA_s_copySignOperation(EDDSA_OperationSign *secureOperation,
                                                     CryptoKey *securePublicKey,
                                                     CryptoKey *securePrivateKey,
                                                     const EDDSA_OperationSign *operation)
{
    const ECCParams_CurveParams *curveParams_s;
    ECCParams_ns_CurveParams curveParams_ns;
    int_fast16_t status;

    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(EDDSA_OperationSign)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(EDDSA_OperationSign));

    /* Make a local copy of the param struct to avoid typecast */
    (void)spm_memcpy(&curveParams_ns, (void *)secureOperation->curve, sizeof(curveParams_ns));

    /* Validate curve is Ed25519 */
    if (curveParams_ns.secureCurve != ECCParams_SecureCurve_Ed25519)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Get pointer to secure curve params */
    curveParams_s = ECCParams_s_getCurveParams(secureOperation->curve);
    if (curveParams_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Update the operation struct to point to secure curve params */
    secureOperation->curve = curveParams_s;

    /* Validate pre-hashed message address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)secureOperation->preHashedMessage,
                                              secureOperation->preHashedMessageLength) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Validate signature R component address range */
    if (cmse_has_unpriv_nonsecure_rw_access((void *)secureOperation->R, curveParams_s->length) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Validate signature S component address range */
    if (cmse_has_unpriv_nonsecure_rw_access((void *)secureOperation->S, curveParams_s->length) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /*
     * Make a secure copy of the public key struct and update the operation
     * struct to point to the secure key copy.
     */
    status = CryptoKey_copySecureInputKey(securePublicKey, &secureOperation->myPublicKey);
    if (status != CryptoKey_STATUS_SUCCESS)
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
 *  ======== EDDSA_s_copyVerifyOperation ========
 */
static inline psa_status_t EDDSA_s_copyVerifyOperation(EDDSA_OperationVerify *secureOperation,
                                                       CryptoKey *securePublicKey,
                                                       const EDDSA_OperationVerify *operation)
{
    const ECCParams_CurveParams *curveParams_s;
    ECCParams_ns_CurveParams curveParams_ns;
    int_fast16_t status;

    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(EDDSA_OperationVerify)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(EDDSA_OperationVerify));

    /* Make a local copy of the param struct to avoid typecast */
    (void)spm_memcpy(&curveParams_ns, (void *)secureOperation->curve, sizeof(curveParams_ns));

    /* Validate curve is Ed25519 */
    if (curveParams_ns.secureCurve != ECCParams_SecureCurve_Ed25519)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Get pointer to secure curve params */
    curveParams_s = ECCParams_s_getCurveParams(secureOperation->curve);
    if (curveParams_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Update the operation struct to point to secure curve params */
    secureOperation->curve = curveParams_s;

    /* Validate pre-hashed message address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)secureOperation->preHashedMessage,
                                              secureOperation->preHashedMessageLength) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Validate signature R component address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)secureOperation->R, curveParams_s->length) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Validate signature S component address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)secureOperation->S, curveParams_s->length) == NULL)
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
 *  ======== EDDSA_s_copyParams ========
 */
static psa_status_t EDDSA_s_copyParams(EDDSA_Params *secureParams, const EDDSA_Params *params)
{
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Validate params address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)params, sizeof(EDDSA_Params)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    (void)spm_memcpy(secureParams, params, sizeof(EDDSA_Params));

    /* Validate the return behavior */
    if ((secureParams->returnBehavior == EDDSA_RETURN_BEHAVIOR_CALLBACK) ||
        (secureParams->returnBehavior == EDDSA_RETURN_BEHAVIOR_BLOCKING) ||
        (secureParams->returnBehavior == EDDSA_RETURN_BEHAVIOR_POLLING))
    {
        /*
         * Overwrite the non-secure client's callback function with our own
         * callback which will populate the secure callback object registered
         * using EDDSA_S_MSG_TYPE_REGISTER_CALLBACK.
         */
        secureParams->callbackFxn = EDDSA_s_hwiCallback;

        /*
         * The underlying EDDSA driver is interrupt-driven regardless of the
         * return behavior specified. Since secure partitions cannot process
         * interrupt signals while a PSA call is running, callback return
         * behavior must be forced in all app-specified return behaviors
         * including polling.
         */
        secureParams->returnBehavior = EDDSA_RETURN_BEHAVIOR_CALLBACK;

        status = PSA_SUCCESS;
    }

    return status;
}

/*
 *  ======== EDDSA_s_getHandle ========
 *  Returns a Secure handle from a handle provided from non-secure client.
 */
static EDDSA_Handle EDDSA_s_getHandle(EDDSA_Handle nsHandle)
{
    EDDSA_Handle secureHandle = NULL;
    uint32_t i;

    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_EDDSA)
    {
        /* Extract the secure handle index */
        i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        if (i < CONFIG_TI_DRIVERS_EDDSA_COUNT)
        {
            secureHandle = (EDDSA_Handle)&EDDSA_config[i];
        }
        else if ((i >= CONFIG_TI_DRIVERS_EDDSA_COUNT) && (i < EDDSA_SECURE_CALLBACK_COUNT))
        {
            secureHandle = &EDDSA_s_dynInstance[i - CONFIG_TI_DRIVERS_EDDSA_COUNT].config;
        }
    }

    return secureHandle;
}

/*
 *  ======== EDDSA_s_registerCallback ========
 */
static inline psa_status_t EDDSA_s_registerCallback(psa_msg_t *msg)
{
    EDDSA_Handle handle_s;
    EDDSA_s_CallbackMsg callbackMsg;
    int8_t callbackIndex;
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Only non-secure callers should be registering callbacks */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id) && (msg->in_size[0] == sizeof(callbackMsg)))
    {
        psa_read(msg->handle, 0, &callbackMsg, sizeof(callbackMsg));

        handle_s = EDDSA_s_getHandle(callbackMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        callbackIndex = EDDSA_s_getCallbackIndex(handle_s);

        /* Validate index and callback object address range */
        if ((callbackIndex >= 0) && (callbackIndex < EDDSA_SECURE_CALLBACK_COUNT) &&
            (cmse_has_unpriv_nonsecure_rw_access(callbackMsg.callback, sizeof(EDDSA_s_SecureCallback)) != NULL))
        {
            /*
             * Store the pointer to EDDSA_s_SecureCallback located in
             * non-secure memory.
             */
            EDDSA_s_secureCB[callbackIndex] = callbackMsg.callback;

            status = PSA_SUCCESS;
        }
    }

    return status;
}

/*
 *  ======== EDDSA_s_construct ========
 */
static inline psa_status_t EDDSA_s_construct(psa_msg_t *msg)
{
    EDDSA_s_ConstructMsg constructMsg;
    EDDSA_Handle handle;
    EDDSA_Params params_s;
    const EDDSA_Params *paramsPtr_s = NULL;
    EDDSA_Config *configPtr_s;
    psa_status_t status    = PSA_ERROR_PROGRAMMER_ERROR;
    EDDSA_Handle retHandle = NULL;

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
            status = EDDSA_s_copyParams(&params_s, constructMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }

            paramsPtr_s = &params_s;
        }

        /* Verify config and copy to secure memory */
        status = EDDSA_s_copyConfig(&configPtr_s, constructMsg.config, &retHandle);
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

    handle = EDDSA_construct(configPtr_s, paramsPtr_s);

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
 *  ======== EDDSA_s_open ========
 */
static inline psa_status_t EDDSA_s_open(psa_msg_t *msg)
{
    EDDSA_s_OpenMsg openMsg;
    EDDSA_Handle handle;
    EDDSA_Params params_s;
    EDDSA_Params *paramsPtr_s = NULL;
    EDDSA_Handle retHandle    = NULL;
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
            status = EDDSA_s_copyParams(&params_s, openMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }
        }

        paramsPtr_s = &params_s;
    }

    handle = EDDSA_open(openMsg.index, paramsPtr_s);

    if (handle != NULL)
    {
        if (TFM_CLIENT_ID_IS_NS(msg->client_id))
        {
            /*
             * Set the secure callback object pointer to NULL. The non-secure
             * client must register a secure callback after constructing or
             * opening a driver instance.
             */
            EDDSA_s_secureCB[openMsg.index] = NULL;

            /*
             * Return CRYPTO_S_HANDLE_ID_EDDSA OR'd with the config array index
             * as the handle instead of a pointer to the config to allow for
             * validation of handles provided by non-secure clients.
             */
            retHandle = (EDDSA_Handle)(CRYPTO_S_HANDLE_ID_EDDSA | openMsg.index);
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
 *  ======== EDDSA_s_close ========
 */
static inline psa_status_t EDDSA_s_close(psa_msg_t *msg)
{
    EDDSA_Handle handle_s;
    EDDSA_s_CloseMsg closeMsg;

    if (msg->in_size[0] != sizeof(closeMsg))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &closeMsg, sizeof(closeMsg));

    /* Non-secure callers have negative client ID */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = EDDSA_s_getHandle(closeMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        EDDSA_close(handle_s);

        /* Release the secure config if it is a dynamic */
        EDDSA_s_releaseConfig(closeMsg.handle);
    }
    else /* Secure client */
    {
        EDDSA_close(closeMsg.handle);
    }

    return PSA_SUCCESS;
}

/*
 *  ======== EDDSA_s_generatePublicKey ========
 */
static inline psa_status_t EDDSA_s_generatePublicKey(psa_msg_t *msg)
{
    EDDSA_s_GenPublicKeyMsg genPubKeyMsg;
    EDDSA_Handle handle_s;
    EDDSA_OperationGeneratePublicKey *operation_s;
    CryptoKey *publicKey_s  = &EDDSA_s_operation.publicKey_s;
    CryptoKey *privateKey_s = &EDDSA_s_operation.privateKey_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(genPubKeyMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &genPubKeyMsg, sizeof(genPubKeyMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = EDDSA_s_getHandle(genPubKeyMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &EDDSA_s_operation.operation_s.generatePublicKey;

        /* Save pointer to non-secure operation struct */
        EDDSA_s_operation.operation_ns = (EDDSA_Operation)genPubKeyMsg.operation;

        /* Save pointer to non-secure public key */
        EDDSA_s_operation.publicKey_ns = genPubKeyMsg.operation->myPublicKey;

        /* Validate and copy operation and key structs */
        status = EDDSA_s_copyGenPublicKeyOperation(operation_s, publicKey_s, privateKey_s, genPubKeyMsg.operation);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = EDDSA_generatePublicKey(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = EDDSA_generatePublicKey(genPubKeyMsg.handle, genPubKeyMsg.operation);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== EDDSA_s_sign ========
 */
static inline psa_status_t EDDSA_s_sign(psa_msg_t *msg)
{
    EDDSA_s_SignMsg signMsg;
    EDDSA_Handle handle_s;
    EDDSA_OperationSign *operation_s;
    CryptoKey *publicKey_s  = &EDDSA_s_operation.publicKey_s;
    CryptoKey *privateKey_s = &EDDSA_s_operation.privateKey_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(signMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &signMsg, sizeof(signMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = EDDSA_s_getHandle(signMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &EDDSA_s_operation.operation_s.sign;

        /* Save pointer to non-secure operation struct */
        EDDSA_s_operation.operation_ns = (EDDSA_Operation)signMsg.operation;

        /* Validate and copy operation and key structs */
        status = EDDSA_s_copySignOperation(operation_s, publicKey_s, privateKey_s, signMsg.operation);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = EDDSA_sign(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = EDDSA_sign(signMsg.handle, signMsg.operation);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== EDDSA_s_verify ========
 */
static inline psa_status_t EDDSA_s_verify(psa_msg_t *msg)
{
    EDDSA_s_VerifyMsg verifyMsg;
    EDDSA_Handle handle_s;
    EDDSA_OperationVerify *operation_s;
    CryptoKey *publicKey_s = &EDDSA_s_operation.publicKey_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(verifyMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &verifyMsg, sizeof(verifyMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = EDDSA_s_getHandle(verifyMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &EDDSA_s_operation.operation_s.verify;

        /* Save pointer to non-secure operation struct */
        EDDSA_s_operation.operation_ns = (EDDSA_Operation)verifyMsg.operation;

        /* Validate and copy operation and key structs */
        status = EDDSA_s_copyVerifyOperation(operation_s, publicKey_s, verifyMsg.operation);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = EDDSA_verify(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = EDDSA_verify(verifyMsg.handle, verifyMsg.operation);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== EDDSA_s_cancelOperation ========
 */
static inline psa_status_t EDDSA_s_cancelOperation(psa_msg_t *msg)
{
    EDDSA_Handle handle_s;
    EDDSA_s_CancelOperationMsg cancelMsg;
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

    handle_s = EDDSA_s_getHandle(cancelMsg.handle);
    if (handle_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    ret = EDDSA_cancelOperation(handle_s);

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== EDDSA_s_handlePsaMsg ========
 */
psa_status_t EDDSA_s_handlePsaMsg(psa_msg_t *msg)
{
    psa_status_t status = PSA_SUCCESS;

    switch (msg->type)
    {
        /*
         * If EDDSA_S_MSG_TYPE_CONSTRUCT is used by other secure partitions,
         * any pointer arguments provided must reference memory accessible by
         * this partition which is dependent on the TF-M isolation level in use.
         */
        case EDDSA_S_MSG_TYPE_CONSTRUCT:
            status = EDDSA_s_construct(msg);
            break;

        case EDDSA_S_MSG_TYPE_OPEN:
            status = EDDSA_s_open(msg);
            break;

        /*
         * EDDSA_S_MSG_TYPE_REGISTER_CALLBACK is designed to be used non-secure
         * callers only. Secure callers must only use polling return behavior.
         */
        case EDDSA_S_MSG_TYPE_REGISTER_CALLBACK:
            status = EDDSA_s_registerCallback(msg);
            break;

        case EDDSA_S_MSG_TYPE_CLOSE:
            status = EDDSA_s_close(msg);
            break;

        case EDDSA_S_MSG_TYPE_GEN_PUBLIC_KEY:
            status = EDDSA_s_generatePublicKey(msg);
            break;

        case EDDSA_S_MSG_TYPE_SIGN:
            status = EDDSA_s_sign(msg);
            break;

        case EDDSA_S_MSG_TYPE_VERIFY:
            status = EDDSA_s_verify(msg);
            break;

        /*
         * EDDSA_S_MSG_TYPE_CANCEL_OPERATION supported for non-secure clients
         * only. Secure callers must only use polling return behavior.
         */
        case EDDSA_S_MSG_TYPE_CANCEL_OPERATION:
            status = EDDSA_s_cancelOperation(msg);
            break;

        default:
            /* Unknown msg type */
            status = PSA_ERROR_PROGRAMMER_ERROR;
            break;
    }

    return status;
}

/*
 *  ======== EDDSA_s_init ========
 */
void EDDSA_s_init(void)
{
    EDDSA_init();
}
