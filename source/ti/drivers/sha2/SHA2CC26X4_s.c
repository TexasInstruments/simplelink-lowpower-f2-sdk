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

#include "SHA2CC26X4_s.h"

#include <ti/drivers/SHA2.h>
#include <ti/drivers/sha2/SHA2CC26X2.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#include <ti/drivers/tfm/SecureCallback.h>

#include <third_party/tfm/secure_fw/spm/core/spm.h>
#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/interface/include/psa/service.h>
#include <third_party/tfm/secure_fw/spm/include/utilities.h>

#include <third_party/tfm/platform/ext/target/ti/cc26x4/cmse.h> /* TI CMSE helper functions */
#include "ti_drivers_config.h"                                  /* Sysconfig generated header */

/*
 * SHA2 Secure Dynamic Instance struct.
 */
typedef struct
{
    SHA2_Config config;
    SHA2CC26X2_Object object;
    SHA2CC26X2_HWAttrs hwAttrs;
} SHA2_s_DynamicInstance;

/*
 * Used to store a secure copy of the dynamic instance (config plus the
 * object and hwAttrs that it points to) provided by non-secure calls to
 * SHA2_construct.
 */
static SHA2_s_DynamicInstance SHA2_s_dynInstance[CONFIG_SHA2_S_CONFIG_POOL_SIZE];

/* Stores pointers to non-secure SHA2_s_SecureCallbacks for each driver instance opened or constructed */
static SHA2_s_SecureCallback *SHA2_s_secureCB[SHA2_SECURE_CALLBACK_COUNT];

/* Static driver instances allocated by SysConfig */
extern const SHA2_Config SHA2_config[];

/* Digest size table defined in SHA2CC26X2.c */
extern const uint8_t digestSizeTable[];

/*
 *  ======== SHA2_s_getHashLength ========
 *  Returns the length of the hash or zero if hash type is invalid.
 */
static uint8_t SHA2_s_getHashLength(SHA2CC26X2_Object *object)
{
    uint8_t hashLen = 0;

    if (object->hashType <= SHA2_HASH_TYPE_512)
    {
        hashLen = digestSizeTable[object->hashType];
    }

    return hashLen;
}

/*
 *  ======== SHA2_s_getCallbackIndex ========
 *  Returns callback index or -1 if index is not found.
 */
static int8_t SHA2_s_getCallbackIndex(SHA2_Handle handle_s)
{
    uint_fast8_t index;
    int8_t retIndex = -1;
    bool indexFound = false;

    /* First, search config statically allocated by SysConfig */
    for (index = 0; index < CONFIG_TI_DRIVERS_SHA2_COUNT; index++)
    {
        if (handle_s == (SHA2_Handle)&SHA2_config[index])
        {
            indexFound = true;
            break;
        }
    }

    /* If not found, search secure config copies */
    if (!indexFound)
    {
        for (index = 0; index < CONFIG_SHA2_S_CONFIG_POOL_SIZE; index++)
        {
            if (handle_s == &SHA2_s_dynInstance[index].config)
            {
                index += CONFIG_TI_DRIVERS_SHA2_COUNT;
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
 *  ======== SHA2_s_hwiCallback ========
 */
static void SHA2_s_hwiCallback(SHA2_Handle handle_s, int_fast16_t returnStatus)
{
    int8_t index;
    SHA2_s_SecureCallback *sha2SecureCB_ns;

    index = SHA2_s_getCallbackIndex(handle_s);

    if ((index >= 0) && (index < SHA2_SECURE_CALLBACK_COUNT))
    {
        sha2SecureCB_ns = SHA2_s_secureCB[index];

        if (sha2SecureCB_ns != NULL)
        {
            /* Store arguments in callback object for use by non-secure handler */
            sha2SecureCB_ns->handle       = (SHA2_Handle)(CRYPTO_S_HANDLE_ID_SHA2 | index);
            sha2SecureCB_ns->returnStatus = returnStatus;

            /* Trigger the interrupt for the non-secure callback dispatcher */
            SecureCallback_post(&sha2SecureCB_ns->object);
        }
    }
}

/*
 *  ======== SHA2_s_copyConfig ========
 */
static inline psa_status_t SHA2_s_copyConfig(SHA2_Config **secureConfig,
                                             const SHA2_Config *config,
                                             SHA2_Handle *retHandle)
{
    SHA2_Config *config_s;
    SHA2_s_DynamicInstance *dynInstance_s;
    uint_fast8_t i;

    for (i = 0; i < CONFIG_SHA2_S_CONFIG_POOL_SIZE; i++)
    {
        dynInstance_s = &SHA2_s_dynInstance[i];
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
            SHA2_s_secureCB[i + CONFIG_TI_DRIVERS_SHA2_COUNT] = NULL;

            /*
             * Return handle is the CRYPTO_S_HANDLE_ID_SHA2 OR'd with the
             * the config pool array index plus the size of constant config
             * array created by Sysconfig.
             */
            *retHandle = (SHA2_Handle)(CRYPTO_S_HANDLE_ID_SHA2 | (i + CONFIG_TI_DRIVERS_SHA2_COUNT));
            return PSA_SUCCESS;
        }
    }

    return PSA_ERROR_INSUFFICIENT_MEMORY;
}

/*
 *  ======== SHA2_s_releaseConfig ========
 */
static inline void SHA2_s_releaseConfig(SHA2_Handle nsHandle)
{
    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_SHA2)
    {
        /* Extract the secure handle index */
        uintptr_t i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        /* Check whether the handle instance refers to a dynamic instance */
        if ((i >= CONFIG_TI_DRIVERS_SHA2_COUNT) && (i < SHA2_SECURE_CALLBACK_COUNT))
        {
            /* Set config's object pointer to NULL to indicate the config is available */
            SHA2_s_dynInstance[i - CONFIG_TI_DRIVERS_SHA2_COUNT].config.object = NULL;
        }
    }
}

/*
 *  ======== SHA2_s_copyParams ========
 */
static psa_status_t SHA2_s_copyParams(SHA2_Params *secureParams, const SHA2_Params *params)
{
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Validate params address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)params, sizeof(SHA2_Params)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    (void)spm_memcpy(secureParams, params, sizeof(SHA2_Params));

    /* Validate the return behavior */
    if ((secureParams->returnBehavior == SHA2_RETURN_BEHAVIOR_CALLBACK) ||
        (secureParams->returnBehavior == SHA2_RETURN_BEHAVIOR_BLOCKING) ||
        (secureParams->returnBehavior == SHA2_RETURN_BEHAVIOR_POLLING))
    {
        if (secureParams->returnBehavior != SHA2_RETURN_BEHAVIOR_POLLING)
        {
            /*
             * Overwrite the non-secure client's callback function with our own
             * callback which will populate the secure callback object registered
             * using SHA2_S_MSG_TYPE_REGISTER_CALLBACK.
             */
            secureParams->callbackFxn = SHA2_s_hwiCallback;

            /* Force to callback return behavior */
            secureParams->returnBehavior = SHA2_RETURN_BEHAVIOR_CALLBACK;
        }

        status = PSA_SUCCESS;
    }

    return status;
}

/*
 *  ======== SHA2_s_getHandle ========
 *  Returns a Secure handle from a handle provided from non-secure client.
 */
static SHA2_Handle SHA2_s_getHandle(SHA2_Handle nsHandle)
{
    SHA2_Handle secureHandle = NULL;
    uint32_t i;

    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_SHA2)
    {
        /* Extract the secure handle index */
        i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        if (i < CONFIG_TI_DRIVERS_SHA2_COUNT)
        {
            secureHandle = (SHA2_Handle)&SHA2_config[i];
        }
        else if ((i >= CONFIG_TI_DRIVERS_SHA2_COUNT) && (i < SHA2_SECURE_CALLBACK_COUNT))
        {
            secureHandle = &SHA2_s_dynInstance[i - CONFIG_TI_DRIVERS_SHA2_COUNT].config;
        }
    }

    return secureHandle;
}

/*
 *  ======== SHA2_s_registerCallback ========
 */
static inline psa_status_t SHA2_s_registerCallback(psa_msg_t *msg)
{
    SHA2_Handle handle_s;
    SHA2_s_CallbackMsg callbackMsg;
    int8_t callbackIndex;
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Only non-secure callers should be registering callbacks */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id) && (msg->in_size[0] == sizeof(callbackMsg)))
    {
        psa_read(msg->handle, 0, &callbackMsg, sizeof(callbackMsg));

        handle_s = SHA2_s_getHandle(callbackMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        callbackIndex = SHA2_s_getCallbackIndex(handle_s);

        /* Validate index and callback object address range */
        if ((callbackIndex >= 0) && (callbackIndex < SHA2_SECURE_CALLBACK_COUNT) &&
            (cmse_has_unpriv_nonsecure_rw_access(callbackMsg.callback, sizeof(SHA2_s_SecureCallback)) != NULL))
        {
            /*
             * Store the pointer to SHA2_s_SecureCallback located in
             * non-secure memory.
             */
            SHA2_s_secureCB[callbackIndex] = callbackMsg.callback;

            status = PSA_SUCCESS;
        }
    }

    return status;
}

/*
 *  ======== SHA2_s_construct ========
 */
static inline psa_status_t SHA2_s_construct(psa_msg_t *msg)
{
    SHA2_s_ConstructMsg constructMsg;
    SHA2_Handle handle;
    SHA2_Params params_s;
    const SHA2_Params *paramsPtr_s = NULL;
    SHA2_Config *configPtr_s;
    psa_status_t status   = PSA_ERROR_PROGRAMMER_ERROR;
    SHA2_Handle retHandle = NULL;

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
            status = SHA2_s_copyParams(&params_s, constructMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }

            paramsPtr_s = &params_s;
        }

        /* Verify config and copy to secure memory */
        status = SHA2_s_copyConfig(&configPtr_s, constructMsg.config, &retHandle);
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

    handle = SHA2_construct(configPtr_s, paramsPtr_s);

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
 *  ======== SHA2_s_open ========
 */
static inline psa_status_t SHA2_s_open(psa_msg_t *msg)
{
    SHA2_s_OpenMsg openMsg;
    SHA2_Handle handle;
    SHA2_Params params_s;
    SHA2_Params *paramsPtr_s = NULL;
    SHA2_Handle retHandle    = NULL;
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
            status = SHA2_s_copyParams(&params_s, openMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }
        }

        paramsPtr_s = &params_s;
    }

    handle = SHA2_open(openMsg.index, paramsPtr_s);

    if (handle != NULL)
    {
        if (TFM_CLIENT_ID_IS_NS(msg->client_id))
        {
            /*
             * Set the secure callback object pointer to NULL. The non-secure
             * client must register a secure callback after constructing or
             * opening a driver instance.
             */
            SHA2_s_secureCB[openMsg.index] = NULL;

            /*
             * Return CRYPTO_S_HANDLE_ID_SHA2 OR'd with the config array index
             * as the handle instead of a pointer to the config to allow for
             * validation of handles provided by non-secure clients.
             */
            retHandle = (SHA2_Handle)(CRYPTO_S_HANDLE_ID_SHA2 | openMsg.index);
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
 *  ======== SHA2_s_close ========
 */
static inline psa_status_t SHA2_s_close(psa_msg_t *msg)
{
    SHA2_Handle handle_s;
    SHA2_s_CloseMsg closeMsg;

    if (msg->in_size[0] != sizeof(closeMsg))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &closeMsg, sizeof(closeMsg));

    /* Non-secure callers have negative client ID */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = SHA2_s_getHandle(closeMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        SHA2_close(handle_s);

        /* Release the secure config if it is a dynamic instance */
        SHA2_s_releaseConfig(closeMsg.handle);
    }
    else /* Secure client */
    {
        SHA2_close(closeMsg.handle);
    }

    return PSA_SUCCESS;
}

/*
 *  ======== SHA2_s_hashData ========
 */
static inline psa_status_t SHA2_s_hashData(psa_msg_t *msg)
{
    SHA2_s_HashDataMsg hashMsg;
    SHA2_Handle handle_s;
    SHA2CC26X2_Object *object;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;
    uint8_t hashLen;

    if ((msg->in_size[0] != sizeof(hashMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &hashMsg, sizeof(hashMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = SHA2_s_getHandle(hashMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        object  = handle_s->object;
        hashLen = SHA2_s_getHashLength(object);
        if (hashLen == 0)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Validate input data address range */
        if (cmse_has_unpriv_nonsecure_read_access((void *)hashMsg.data, hashMsg.dataLength) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Validate output digest address range */
        if (cmse_has_unpriv_nonsecure_rw_access((void *)hashMsg.digest, hashLen) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        ret = SHA2_hashData(handle_s, hashMsg.data, hashMsg.dataLength, hashMsg.digest);
    }
    else /* Secure client */
    {
        ret = SHA2_hashData(hashMsg.handle, hashMsg.data, hashMsg.dataLength, hashMsg.digest);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== SHA2_s_hmac ========
 */
static inline psa_status_t SHA2_s_hmac(psa_msg_t *msg)
{
    /*
     * Secure key may be stored on the stack because SHA2_hmac() processes
     * the key in polling mode regardless of the return behavior specified.
     */
    CryptoKey key_s;
    SHA2_s_HmacMsg hmacMsg;
    SHA2_Handle handle_s;
    SHA2CC26X2_Object *object;
    int_fast16_t ret;
    uint8_t hashLen;

    if ((msg->in_size[0] != sizeof(hmacMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &hmacMsg, sizeof(hmacMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = SHA2_s_getHandle(hmacMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        object  = handle_s->object;
        hashLen = SHA2_s_getHashLength(object);
        if (hashLen == 0)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        ret = CryptoKey_copySecureInputKey(&key_s, &hmacMsg.key);
        if (ret != CryptoKey_STATUS_SUCCESS)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Validate input data address range */
        if (cmse_has_unpriv_nonsecure_read_access((void *)hmacMsg.data, hmacMsg.dataLength) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Validate output HMAC address range */
        if (cmse_has_unpriv_nonsecure_rw_access(hmacMsg.hmac, hashLen) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        ret = SHA2_hmac(handle_s, &key_s, hmacMsg.data, hmacMsg.dataLength, hmacMsg.hmac);
    }
    else /* Secure client */
    {
        ret = SHA2_hmac(hmacMsg.handle, hmacMsg.key, hmacMsg.data, hmacMsg.dataLength, hmacMsg.hmac);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== SHA2_s_setHashType ========
 */
static inline psa_status_t SHA2_s_setHashType(psa_msg_t *msg)
{
    SHA2_Handle handle_s;
    SHA2_s_SetHashTypeMsg setTypeMsg;
    int_fast16_t ret;

    if ((msg->in_size[0] != sizeof(setTypeMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &setTypeMsg, sizeof(setTypeMsg));

    /* Verify hash type is valid */
    if ((setTypeMsg.type < SHA2_HASH_TYPE_224) || (setTypeMsg.type > SHA2_HASH_TYPE_512))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = SHA2_s_getHandle(setTypeMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        ret = SHA2_setHashType(handle_s, setTypeMsg.type);
    }
    else /* Secure client */
    {
        ret = SHA2_setHashType(setTypeMsg.handle, setTypeMsg.type);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== SHA2_s_setupHmac ========
 */
static inline psa_status_t SHA2_s_setupHmac(psa_msg_t *msg)
{
    /*
     * Secure key may be stored on the stack because SHA2_hmac() processes
     * the key in polling mode regardless of the return behavior specified.
     */
    CryptoKey key_s;
    SHA2_Handle handle_s;
    SHA2_s_SetupHmacMsg setupMsg;
    int_fast16_t ret;

    if ((msg->in_size[0] != sizeof(setupMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &setupMsg, sizeof(setupMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = SHA2_s_getHandle(setupMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        ret = CryptoKey_copySecureInputKey(&key_s, &setupMsg.key);
        if (ret != CryptoKey_STATUS_SUCCESS)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        ret = SHA2_setupHmac(handle_s, &key_s);
    }
    else /* Secure client */
    {
        ret = SHA2_setupHmac(setupMsg.handle, setupMsg.key);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== SHA2_s_addData ========
 */
static inline psa_status_t SHA2_s_addData(psa_msg_t *msg)
{
    SHA2_Handle handle_s;
    SHA2_s_AddDataMsg addDataMsg;
    int_fast16_t ret;

    if ((msg->in_size[0] != sizeof(addDataMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &addDataMsg, sizeof(addDataMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = SHA2_s_getHandle(addDataMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Verify data address range */
        if (cmse_has_unpriv_nonsecure_read_access((void *)addDataMsg.data, addDataMsg.length) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        ret = SHA2_addData(handle_s, addDataMsg.data, addDataMsg.length);
    }
    else /* Secure client */
    {
        ret = SHA2_addData(addDataMsg.handle, addDataMsg.data, addDataMsg.length);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== SHA2_s_finalize ========
 */
static inline psa_status_t SHA2_s_finalize(psa_msg_t *msg, int32_t msgType)
{
    SHA2_Handle handle_s;
    SHA2_s_FinalizeMsg finalizeMsg;
    SHA2CC26X2_Object *object;
    int_fast16_t ret;
    uint8_t hashLen;

    if ((msg->in_size[0] != sizeof(finalizeMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &finalizeMsg, sizeof(finalizeMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = SHA2_s_getHandle(finalizeMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        object  = handle_s->object;
        hashLen = SHA2_s_getHashLength(object);
        if (hashLen == 0)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Verify digest output address range */
        if (cmse_has_unpriv_nonsecure_rw_access(finalizeMsg.digestOrHmac, hashLen) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }
    }
    else /* Secure client */
    {
        handle_s = finalizeMsg.handle;
    }

    if (msgType == SHA2_S_MSG_TYPE_FINALIZE)
    {
        ret = SHA2_finalize(handle_s, finalizeMsg.digestOrHmac);
    }
    else /* SHA2_S_MSG_TYPE_FINALIZE_HMAC */
    {
        ret = SHA2_finalizeHmac(handle_s, finalizeMsg.digestOrHmac);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== SHA2_s_cancelOperation ========
 */
static inline psa_status_t SHA2_s_cancelOperation(psa_msg_t *msg)
{
    SHA2_Handle handle_s;
    SHA2_s_CancelOperationMsg cancelMsg;
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

    handle_s = SHA2_s_getHandle(cancelMsg.handle);
    if (handle_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    ret = SHA2_cancelOperation(handle_s);

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== SHA2_s_reset ========
 */
static inline psa_status_t SHA2_s_reset(psa_msg_t *msg)
{
    SHA2_Handle handle_s;
    SHA2_s_ResetMsg resetMsg;

    if (msg->in_size[0] != sizeof(resetMsg))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &resetMsg, sizeof(resetMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = SHA2_s_getHandle(resetMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        SHA2_reset(handle_s);
    }
    else
    {
        SHA2_reset(resetMsg.handle);
    }

    return PSA_SUCCESS;
}

/*
 *  ======== SHA2_s_handlePsaMsg ========
 */
psa_status_t SHA2_s_handlePsaMsg(psa_msg_t *msg)
{
    psa_status_t status = PSA_SUCCESS;

    switch (msg->type)
    {
        /*
         * If SHA2_S_MSG_TYPE_CONSTRUCT is used by other secure partitions,
         * any pointer arguments provided must reference memory accessible by
         * this partition which is dependent on the TF-M isolation level in use.
         */
        case SHA2_S_MSG_TYPE_CONSTRUCT:
            status = SHA2_s_construct(msg);
            break;

        case SHA2_S_MSG_TYPE_OPEN:
            status = SHA2_s_open(msg);
            break;

        /*
         * SHA2_S_MSG_TYPE_REGISTER_CALLBACK is designed to be used non-secure
         * callers only. Secure callers must only use polling return behavior.
         */
        case SHA2_S_MSG_TYPE_REGISTER_CALLBACK:
            status = SHA2_s_registerCallback(msg);
            break;

        case SHA2_S_MSG_TYPE_CLOSE:
            status = SHA2_s_close(msg);
            break;

        case SHA2_S_MSG_TYPE_HASH_DATA:
            status = SHA2_s_hashData(msg);
            break;

        case SHA2_S_MSG_TYPE_HMAC:
            status = SHA2_s_hmac(msg);
            break;

        case SHA2_S_MSG_TYPE_SET_HASH_TYPE:
            status = SHA2_s_setHashType(msg);
            break;

        case SHA2_S_MSG_TYPE_SETUP_HMAC:
            status = SHA2_s_setupHmac(msg);
            break;

        case SHA2_S_MSG_TYPE_ADD_DATA:
            status = SHA2_s_addData(msg);
            break;

        case SHA2_S_MSG_TYPE_FINALIZE: /* Fall through */
        case SHA2_S_MSG_TYPE_FINALIZE_HMAC:
            status = SHA2_s_finalize(msg, msg->type);
            break;

        /*
         * SHA2_S_MSG_TYPE_CANCEL_OPERATION supported for non-secure clients
         * only. Secure callers must only use polling return behavior.
         */
        case SHA2_S_MSG_TYPE_CANCEL_OPERATION:
            status = SHA2_s_cancelOperation(msg);
            break;

        case SHA2_S_MSG_TYPE_RESET:
            status = SHA2_s_reset(msg);
            break;

        default:
            /* Unknown msg type */
            status = PSA_ERROR_PROGRAMMER_ERROR;
            break;
    }

    return status;
}

/*
 *  ======== SHA2_s_init ========
 */
void SHA2_s_init(void)
{
    SHA2_init();
}
