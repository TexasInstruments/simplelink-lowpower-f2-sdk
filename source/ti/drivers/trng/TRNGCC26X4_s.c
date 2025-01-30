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
#include <string.h>

#include "TRNGCC26X4_s.h"

#include <ti/drivers/TRNG.h>
#include <ti/drivers/trng/TRNGCC26XX.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#include <ti/drivers/tfm/SecureCallback.h>

#include <third_party/tfm/secure_fw/include/security_defs.h> /* __tz_c_veneer */

#include <third_party/tfm/secure_fw/spm/core/spm.h>
#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/interface/include/psa/service.h>
#include <third_party/tfm/secure_fw/spm/include/utilities.h>

#include <third_party/tfm/platform/ext/target/ti/cc26x4/cmse.h> /* TI CMSE helper functions */
#include "ti_drivers_config.h"                                  /* Sysconfig generated header */

#define TRNG_INSTANCE_COUNT TRNG_SECURE_CALLBACK_COUNT

/*
 * Stores a secure copy of the entropy key struct used when generating a key
 * and the original pointer to the non-secure entropy key struct to return to
 * non-secure client case of callback or blocking return behavior.
 */
typedef struct
{
    CryptoKey entropyKey_s;
    CryptoKey *entropyKey_ns;
} TRNG_s_EntropyKey;

/*
 * The underlying TRNG driver supports preemption of existing operations with
 * blocking or callback return behavior by a polling operation. Therefore,
 * the entropy key must be tracked per driver instance.
 */
static TRNG_s_EntropyKey TRNG_s_entropyKey[TRNG_INSTANCE_COUNT];

/*
 * TRNG Secure Dynamic Instance struct.
 */
typedef struct
{
    TRNG_Config config;
    TRNGCC26XX_Object object;
    TRNGCC26XX_HWAttrs hwAttrs;
} TRNG_s_DynamicInstance;

/*
 * Used to store a secure copy of the dynamic instance (config plus the
 * object and hwAttrs that it points to) provided by non-secure calls to
 * TRNG_construct.
 */
static TRNG_s_DynamicInstance TRNG_s_dynInstance[CONFIG_TRNG_S_CONFIG_POOL_SIZE];

typedef struct
{
    TRNG_s_CryptoKeySecureCallback *cryptoKeyCallback;
    TRNG_s_RandomBytesSecureCallback *randomBytesCallback;
} TRNG_s_SecureCallbacks;

/* Stores pointers to non-secure TRNG_s_SecureCallbacks for each driver instance opened or constructed */
static TRNG_s_SecureCallbacks TRNG_s_secureCB[TRNG_SECURE_CALLBACK_COUNT];

/* Static driver instances allocated by SysConfig */
extern const TRNG_Config TRNG_config[];

/*
 *  ======== TRNG_s_getCallbackIndex ========
 *  Returns callback index or -1 if index is not found.
 */
static int8_t TRNG_s_getCallbackIndex(TRNG_Handle handle_s)
{
    uint_fast8_t index;
    int8_t retIndex = -1;
    bool indexFound = false;

    /* First, search config statically allocated by SysConfig */
    for (index = 0; index < CONFIG_TI_DRIVERS_TRNG_COUNT; index++)
    {
        if (handle_s == (TRNG_Handle)&TRNG_config[index])
        {
            indexFound = true;
            break;
        }
    }

    /* If not found, search secure config copies */
    if (!indexFound)
    {
        for (index = 0; index < CONFIG_TRNG_S_CONFIG_POOL_SIZE; index++)
        {
            if (handle_s == &TRNG_s_dynInstance[index].config)
            {
                index += CONFIG_TI_DRIVERS_TRNG_COUNT;
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
 *  ======== TRNG_s_clearCallbacks ========
 */
static void TRNG_s_clearCallbacks(uint8_t index)
{
    TRNG_s_secureCB[index].cryptoKeyCallback   = NULL;
    TRNG_s_secureCB[index].randomBytesCallback = NULL;
}

/*
 *  ======== TRNG_s_cryptoKeyCallback ========
 */
static void TRNG_s_cryptoKeyCallback(TRNG_Handle handle_s, int_fast16_t returnValue, CryptoKey *entropy)
{
    int8_t index;
    TRNG_s_CryptoKeySecureCallback *trngSecureCB_ns;

    index = TRNG_s_getCallbackIndex(handle_s);

    if ((index >= 0) && (index < TRNG_SECURE_CALLBACK_COUNT))
    {
        trngSecureCB_ns = TRNG_s_secureCB[index].cryptoKeyCallback;

        /* Copy the updated key encoding to the non-secure key struct */
        TRNG_s_entropyKey[index].entropyKey_ns->encoding = entropy->encoding;

        if (trngSecureCB_ns != NULL)
        {
            /* Store arguments in callback object for use by non-secure handler */
            trngSecureCB_ns->handle      = (TRNG_Handle)(CRYPTO_S_HANDLE_ID_TRNG | index);
            trngSecureCB_ns->returnValue = returnValue;
            trngSecureCB_ns->entropy     = TRNG_s_entropyKey[index].entropyKey_ns;

            /* Trigger the interrupt for the non-secure callback dispatcher */
            SecureCallback_post(&trngSecureCB_ns->object);
        }
    }
}

/*
 *  ======== TRNG_s_randomBytesCallback ========
 */
static void TRNG_s_randomBytesCallback(TRNG_Handle handle_s,
                                       int_fast16_t returnValue,
                                       uint8_t *randomBytes,
                                       size_t randomBytesSize)
{
    int8_t index;
    TRNG_s_RandomBytesSecureCallback *trngSecureCB_ns;
    TRNGCC26XX_Object *object = (TRNGCC26XX_Object *)handle_s->object;

    index = TRNG_s_getCallbackIndex(handle_s);

    if ((index >= 0) && (index < TRNG_SECURE_CALLBACK_COUNT))
    {
        trngSecureCB_ns = TRNG_s_secureCB[index].randomBytesCallback;

        if (trngSecureCB_ns != NULL)
        {
            /* Store arguments in callback object for use by non-secure handler */
            trngSecureCB_ns->handle          = (TRNG_Handle)(CRYPTO_S_HANDLE_ID_TRNG | index);
            trngSecureCB_ns->returnValue     = returnValue;
            trngSecureCB_ns->randomBytes     = object->entropyBuffer;
            trngSecureCB_ns->randomBytesSize = object->entropyRequested;

            /* Trigger the interrupt for the non-secure callback dispatcher */
            SecureCallback_post(&trngSecureCB_ns->object);
        }
    }
}

/*
 *  ======== TRNG_s_copyConfig ========
 */
static inline psa_status_t TRNG_s_copyConfig(TRNG_Config **secureConfig,
                                             const TRNG_Config *config,
                                             TRNG_Handle *retHandle)
{
    TRNG_Config *config_s;
    TRNG_s_DynamicInstance *dynInstance_s;
    uint_fast8_t i;

    for (i = 0; i < CONFIG_TRNG_S_CONFIG_POOL_SIZE; i++)
    {
        dynInstance_s = &TRNG_s_dynInstance[i];
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
             * Set the secure callback pointers to NULL. The application
             * must register secure callbacks after constructing or opening
             * a driver instance.
             */
            TRNG_s_clearCallbacks(i + CONFIG_TI_DRIVERS_TRNG_COUNT);

            /*
             * Return handle is the CRYPTO_S_HANDLE_ID_TRNG OR'd with the
             * the config pool array index plus the size of constant config
             * array created by Sysconfig.
             */
            *retHandle = (TRNG_Handle)(CRYPTO_S_HANDLE_ID_TRNG | (i + CONFIG_TI_DRIVERS_TRNG_COUNT));
            return PSA_SUCCESS;
        }
    }

    return PSA_ERROR_INSUFFICIENT_MEMORY;
}

/*
 *  ======== TRNG_s_releaseConfig ========
 */
static inline void TRNG_s_releaseConfig(TRNG_Handle nsHandle)
{
    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_TRNG)
    {
        /* Extract the secure handle index */
        uintptr_t i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        /* Check whether the handle instance refers to a dynamic instance */
        if ((i >= CONFIG_TI_DRIVERS_TRNG_COUNT) && (i < TRNG_SECURE_CALLBACK_COUNT))
        {
            /* Set config's object pointer to NULL to indicate the config is available */
            TRNG_s_dynInstance[i - CONFIG_TI_DRIVERS_TRNG_COUNT].config.object = NULL;
        }
    }
}

/*
 *  ======== TRNG_s_copyParams ========
 */
static psa_status_t TRNG_s_copyParams(TRNG_Params *secureParams, const TRNG_Params *params)
{
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Validate params address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)params, sizeof(TRNG_Params)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    (void)spm_memcpy(secureParams, params, sizeof(TRNG_Params));

    /* Validate the return behavior */
    if ((secureParams->returnBehavior == TRNG_RETURN_BEHAVIOR_CALLBACK) ||
        (secureParams->returnBehavior == TRNG_RETURN_BEHAVIOR_BLOCKING) ||
        (secureParams->returnBehavior == TRNG_RETURN_BEHAVIOR_POLLING))
    {
        if (secureParams->returnBehavior != TRNG_RETURN_BEHAVIOR_POLLING)
        {
            /*
             * Overwrite the non-secure client's callback functions with our own
             * callbacks which will populate the secure callback object registered
             * using TRNG_S_MSG_TYPE_REGISTER_CALLBACKS.
             */
            secureParams->cryptoKeyCallbackFxn   = TRNG_s_cryptoKeyCallback;
            secureParams->randomBytesCallbackFxn = TRNG_s_randomBytesCallback;

            /* Force to callback return behavior */
            secureParams->returnBehavior = TRNG_RETURN_BEHAVIOR_CALLBACK;
        }

        status = PSA_SUCCESS;
    }

    return status;
}

/*
 *  ======== TRNG_s_getHandle ========
 *  Returns a Secure handle from a handle provided from non-secure client.
 */
static TRNG_Handle TRNG_s_getHandle(TRNG_Handle nsHandle)
{
    TRNG_Handle secureHandle = NULL;
    uint32_t i;

    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_TRNG)
    {
        /* Extract the secure handle index */
        i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        if (i < CONFIG_TI_DRIVERS_TRNG_COUNT)
        {
            secureHandle = (TRNG_Handle)&TRNG_config[i];
        }
        else if ((i >= CONFIG_TI_DRIVERS_TRNG_COUNT) && (i < TRNG_SECURE_CALLBACK_COUNT))
        {
            secureHandle = &TRNG_s_dynInstance[i - CONFIG_TI_DRIVERS_TRNG_COUNT].config;
        }
    }

    return secureHandle;
}

/*
 *  ======== TRNG_s_registerCallbacks ========
 */
static inline psa_status_t TRNG_s_registerCallbacks(psa_msg_t *msg)
{
    TRNG_Handle handle_s;
    TRNG_s_CallbackMsg callbackMsg;
    int8_t callbackIndex;
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Only non-secure callers should be registering callbacks */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id) && (msg->in_size[0] == sizeof(callbackMsg)))
    {
        psa_read(msg->handle, 0, &callbackMsg, sizeof(callbackMsg));

        /* At least one callback function must be non-NULL */
        if ((callbackMsg.cryptoKeyCallback == NULL) && (callbackMsg.randomBytesCallback == NULL))
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        handle_s = TRNG_s_getHandle(callbackMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        callbackIndex = TRNG_s_getCallbackIndex(handle_s);

        /* Validate index */
        if ((callbackIndex >= 0) && (callbackIndex < TRNG_SECURE_CALLBACK_COUNT))
        {
            /*
             * Store the pointers to TRNG_s_CryptoKeySecureCallback and
             * TRNG_s_RandomBytesCallback located in non-secure memory.
             * NULL pointers are valid.
             */
            if ((callbackMsg.cryptoKeyCallback == NULL) ||
                (cmse_has_unpriv_nonsecure_rw_access(callbackMsg.cryptoKeyCallback,
                                                     sizeof(TRNG_s_CryptoKeySecureCallback)) != NULL))
            {
                TRNG_s_secureCB[callbackIndex].cryptoKeyCallback = callbackMsg.cryptoKeyCallback;

                if ((callbackMsg.randomBytesCallback == NULL) ||
                    (cmse_has_unpriv_nonsecure_rw_access(callbackMsg.randomBytesCallback,
                                                         sizeof(TRNG_s_RandomBytesSecureCallback)) != NULL))
                {
                    TRNG_s_secureCB[callbackIndex].randomBytesCallback = callbackMsg.randomBytesCallback;
                    status                                             = PSA_SUCCESS;
                }
            }
        }
    }

    return status;
}

/*
 *  ======== TRNG_s_construct ========
 */
static inline psa_status_t TRNG_s_construct(psa_msg_t *msg)
{
    TRNG_s_ConstructMsg constructMsg;
    TRNG_Handle handle;
    TRNG_Params params_s;
    const TRNG_Params *paramsPtr_s = NULL;
    TRNG_Config *configPtr_s;
    psa_status_t status   = PSA_ERROR_PROGRAMMER_ERROR;
    TRNG_Handle retHandle = NULL;

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
            status = TRNG_s_copyParams(&params_s, constructMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }

            paramsPtr_s = &params_s;
        }

        /* Verify config and copy to secure memory */
        status = TRNG_s_copyConfig(&configPtr_s, constructMsg.config, &retHandle);
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

    handle = TRNG_construct(configPtr_s, paramsPtr_s);

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
 *  ======== TRNG_s_open ========
 */
static inline psa_status_t TRNG_s_open(psa_msg_t *msg)
{
    TRNG_s_OpenMsg openMsg;
    TRNG_Handle handle;
    TRNG_Params params_s;
    TRNG_Params *paramsPtr_s = NULL;
    TRNG_Handle retHandle    = NULL;
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
            status = TRNG_s_copyParams(&params_s, openMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }
        }

        paramsPtr_s = &params_s;
    }

    handle = TRNG_open(openMsg.index, paramsPtr_s);

    if (handle != NULL)
    {
        if (TFM_CLIENT_ID_IS_NS(msg->client_id))
        {
            /*
             * Set the secure callback pointers to NULL. The application
             * must register a secure callbacks after constructing or opening
             * a driver instance.
             */
            TRNG_s_clearCallbacks(openMsg.index);

            /*
             * Return CRYPTO_S_HANDLE_ID_TRNG OR'd with the config array index
             * as the handle instead of a pointer to the config to allow for
             * validation of handles provided by non-secure clients.
             */
            retHandle = (TRNG_Handle)(CRYPTO_S_HANDLE_ID_TRNG | openMsg.index);
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
 *  ======== TRNG_s_close ========
 */
static inline psa_status_t TRNG_s_close(psa_msg_t *msg)
{
    TRNG_Handle handle_s;
    TRNG_s_CloseMsg closeMsg;

    if (msg->in_size[0] != sizeof(closeMsg))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &closeMsg, sizeof(closeMsg));

    /* Non-secure callers have negative client ID */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = TRNG_s_getHandle(closeMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        TRNG_close(handle_s);

        /* Release the secure config if it is a dynamic instance */
        TRNG_s_releaseConfig(closeMsg.handle);
    }
    else /* Secure client */
    {
        TRNG_close(closeMsg.handle);
    }

    return PSA_SUCCESS;
}

/*
 *  ======== TRNG_s_generateKey ========
 */
static inline psa_status_t TRNG_s_generateKey(psa_msg_t *msg)
{
    TRNG_s_GenerateKeyMsg genKeyMsg;
    TRNG_Handle handle_s;
    CryptoKey *entropy_s;
    int_fast16_t ret;
    uint8_t idx;

    if ((msg->in_size[0] != sizeof(genKeyMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &genKeyMsg, sizeof(genKeyMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = TRNG_s_getHandle(genKeyMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Handle index is guaranteed to be valid if handle_s is non-NULL */
        idx       = GET_CRYPTO_S_HANDLE_INDEX(genKeyMsg.handle);
        entropy_s = &TRNG_s_entropyKey[idx].entropyKey_s;

        /* Validate crypto key struct address range */
        if (cmse_has_unpriv_nonsecure_read_access(genKeyMsg.entropy, sizeof(CryptoKey)) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Make a secure copy of the key struct */
        (void)spm_memcpy(entropy_s, genKeyMsg.entropy, sizeof(CryptoKey));

        /* Save the non-secure key struct pointer */
        TRNG_s_entropyKey[idx].entropyKey_ns = genKeyMsg.entropy;

        if (CryptoKey_verifySecureOutputKey(entropy_s) != CryptoKey_STATUS_SUCCESS)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        ret = TRNG_generateKey(handle_s, entropy_s);

        /*
         * For polling return behavior, the key encoding will be updated upon
         * successful completion. To avoid overhead of checking for successful
         * return status and polling return behavior, blindly copy the updated
         * encoding to the non-secure key struct.
         */
        TRNG_s_entropyKey[idx].entropyKey_ns->encoding = entropy_s->encoding;
    }
    else /* Secure client */
    {
        ret = TRNG_generateKey(genKeyMsg.handle, genKeyMsg.entropy);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== TRNG_s_getRandomBytes ========
 */
static inline psa_status_t TRNG_s_getRandomBytes(psa_msg_t *msg)
{
    TRNG_s_GetRandomBytesMsg getBytesMsg;
    TRNG_Handle handle_s;
    int_fast16_t ret;

    if ((msg->in_size[0] != sizeof(getBytesMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &getBytesMsg, sizeof(getBytesMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = TRNG_s_getHandle(getBytesMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Validate random bytes buffer address range */
        if (cmse_has_unpriv_nonsecure_rw_access(getBytesMsg.randomBytes, getBytesMsg.randomBytesSize) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        ret = TRNG_getRandomBytes(handle_s, getBytesMsg.randomBytes, getBytesMsg.randomBytesSize);
    }
    else /* Secure client */
    {
        ret = TRNG_getRandomBytes(getBytesMsg.handle, getBytesMsg.randomBytes, getBytesMsg.randomBytesSize);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== TRNG_s_cancelOperation ========
 */
static inline psa_status_t TRNG_s_cancelOperation(psa_msg_t *msg)
{
    TRNG_Handle handle_s;
    TRNG_s_CancelOperationMsg cancelMsg;
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

    handle_s = TRNG_s_getHandle(cancelMsg.handle);
    if (handle_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    ret = TRNG_cancelOperation(handle_s);

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== TRNG_s_handlePsaMsg ========
 */
psa_status_t TRNG_s_handlePsaMsg(psa_msg_t *msg)
{
    psa_status_t status = PSA_SUCCESS;

    switch (msg->type)
    {
        /*
         * If TRNG_S_MSG_TYPE_CONSTRUCT is used by other secure partitions,
         * any pointer arguments provided must reference memory accessible by
         * this partition which is dependent on the TF-M isolation level in use.
         */
        case TRNG_S_MSG_TYPE_CONSTRUCT:
            status = TRNG_s_construct(msg);
            break;

        case TRNG_S_MSG_TYPE_OPEN:
            status = TRNG_s_open(msg);
            break;

        /*
         * TRNG_S_MSG_TYPE_REGISTER_CALLBACKS is designed to be used non-secure
         * callers only. Secure callers must only use polling return behavior.
         */
        case TRNG_S_MSG_TYPE_REGISTER_CALLBACKS:
            status = TRNG_s_registerCallbacks(msg);
            break;

        case TRNG_S_MSG_TYPE_CLOSE:
            status = TRNG_s_close(msg);
            break;

        case TRNG_S_MSG_TYPE_GENERATE_KEY:
            status = TRNG_s_generateKey(msg);
            break;

        case TRNG_S_MSG_TYPE_GET_RANDOM_BYTES:
            status = TRNG_s_getRandomBytes(msg);
            break;

        /*
         * TRNG_S_MSG_TYPE_CANCEL_OPERATION supported for non-secure clients
         * only. Secure callers must only use polling return behavior.
         */
        case TRNG_S_MSG_TYPE_CANCEL_OPERATION:
            status = TRNG_s_cancelOperation(msg);
            break;

        default:
            /* Unknown msg type */
            status = PSA_ERROR_PROGRAMMER_ERROR;
            break;
    }

    return status;
}

/*
 *  ======== TRNG_s_init ========
 */
__tz_c_veneer void TRNG_s_init(void)
{
    TRNG_init();
}

/*
 *  ======== TRNGCC26XX_s_setSamplesPerCycle ========
 */
__tz_c_veneer int_fast16_t TRNGCC26XX_s_setSamplesPerCycle(TRNG_Handle handle, uint32_t samplesPerCycle)
{
    TRNG_Handle handle_s;

    handle_s = TRNG_s_getHandle(handle);
    if (handle_s == NULL)
    {
        return (int_fast16_t)PSA_ERROR_PROGRAMMER_ERROR;
    }

    return TRNGCC26XX_s_setSamplesPerCycle(handle_s, samplesPerCycle);
}
