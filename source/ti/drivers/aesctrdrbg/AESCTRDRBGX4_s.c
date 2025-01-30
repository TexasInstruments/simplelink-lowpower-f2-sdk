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

#include "AESCTRDRBGX4_s.h"

#include <ti/drivers/AESCTRDRBG.h>
#include <ti/drivers/aesctrdrbg/AESCTRDRBGXX.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#include <third_party/tfm/secure_fw/spm/core/spm.h>
#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/interface/include/psa/service.h>
#include <third_party/tfm/secure_fw/spm/include/utilities.h>

#include <third_party/tfm/platform/ext/target/ti/cc26x4/cmse.h> /* TI CMSE helper functions */
#include "ti_drivers_config.h"                                  /* Sysconfig generated header */

/*
 * AES CTR DRBG Secure Dynamic Instance struct.
 */
typedef struct
{
    AESCTRDRBG_Config config;
    AESCTRDRBGXX_Object object;
    AESCTRDRBGXX_HWAttrs hwAttrs;
} AESCTRDRBG_s_DynamicInstance;

/*
 * Used to store a secure copy of the dynamic instance (config plus the
 * object and hwAttrs that it points to) provided by non-secure calls to
 * AESCTRDRBG_construct.
 */
static AESCTRDRBG_s_DynamicInstance AESCTRDRBG_s_dynInstance[CONFIG_AESCTRDRBG_S_CONFIG_POOL_SIZE];

/* Static driver instances allocated by SysConfig */
extern const AESCTRDRBG_Config AESCTRDRBG_config[];

/*
 *  ======== AESCTRDRBG_s_copyConfig ========
 */
static inline psa_status_t AESCTRDRBG_s_copyConfig(AESCTRDRBG_Config **secureConfig,
                                                   const AESCTRDRBG_Config *config,
                                                   AESCTRDRBG_Handle *retHandle)
{
    AESCTRDRBG_Config *config_s;
    AESCTRDRBG_s_DynamicInstance *dynInstance_s;
    uint_fast8_t i;

    for (i = 0; i < CONFIG_AESCTRDRBG_S_CONFIG_POOL_SIZE; i++)
    {
        dynInstance_s = &AESCTRDRBG_s_dynInstance[i];
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
             * Return handle is the CRYPTO_S_HANDLE_ID_AESCTRDRBG OR'd with the
             * the config pool array index plus the size of constant config
             * array created by Sysconfig.
             */
            *retHandle = (AESCTRDRBG_Handle)(CRYPTO_S_HANDLE_ID_AESCTRDRBG | (i + CONFIG_TI_DRIVERS_AESCTRDRBG_COUNT));
            return PSA_SUCCESS;
        }
    }

    return PSA_ERROR_INSUFFICIENT_MEMORY;
}

/*
 *  ======== AESCTRDRBG_s_releaseConfig ========
 */
static inline void AESCTRDRBG_s_releaseConfig(AESCTRDRBG_Handle nsHandle)
{
    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_AESCTRDRBG)
    {
        /* Extract the secure handle index */
        uintptr_t i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        /* Check whether the handle instance refers to a dynamic instance */
        if ((i >= CONFIG_TI_DRIVERS_AESCTRDRBG_COUNT) &&
            (i < (CONFIG_TI_DRIVERS_AESCTRDRBG_COUNT + CONFIG_AESCTRDRBG_S_CONFIG_POOL_SIZE)))
        {
            /* Set config's object pointer to NULL to indicate the config is available */
            AESCTRDRBG_s_dynInstance[i - CONFIG_TI_DRIVERS_AESCTRDRBG_COUNT].config.object = NULL;
        }
    }
}

/*
 *  ======== AESCTRDRBG_s_copyParams ========
 */
static psa_status_t AESCTRDRBG_s_copyParams(AESCTRDRBG_Params *secureParams, const AESCTRDRBG_Params *params)
{
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Validate params address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)params, sizeof(AESCTRDRBG_Params)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    (void)spm_memcpy(secureParams, params, sizeof(AESCTRDRBG_Params));

    /* Validate the return behavior */
    if ((secureParams->returnBehavior == AESCTRDRBG_RETURN_BEHAVIOR_BLOCKING) ||
        (secureParams->returnBehavior == AESCTRDRBG_RETURN_BEHAVIOR_POLLING))
    {
        /* Force to polling return behavior */
        secureParams->returnBehavior = AESCTRDRBG_RETURN_BEHAVIOR_POLLING;

        status = PSA_SUCCESS;
    }

    return status;
}

/*
 *  ======== AESCTRDRBG_s_getHandle ========
 *  Returns a Secure handle from a handle provided from non-secure client.
 */
static AESCTRDRBG_Handle AESCTRDRBG_s_getHandle(AESCTRDRBG_Handle nsHandle)
{
    AESCTRDRBG_Handle secureHandle = NULL;
    uint32_t i;

    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_AESCTRDRBG)
    {
        /* Extract the secure handle index */
        i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        if (i < CONFIG_TI_DRIVERS_AESCTRDRBG_COUNT)
        {
            secureHandle = (AESCTRDRBG_Handle)&AESCTRDRBG_config[i];
        }
        else if ((i >= CONFIG_TI_DRIVERS_AESCTRDRBG_COUNT) &&
                 (i < (CONFIG_TI_DRIVERS_AESCTRDRBG_COUNT + CONFIG_AESCTRDRBG_S_CONFIG_POOL_SIZE)))
        {
            secureHandle = &AESCTRDRBG_s_dynInstance[i - CONFIG_TI_DRIVERS_AESCTRDRBG_COUNT].config;
        }
    }

    return secureHandle;
}

/*
 *  ======== AESCTRDRBG_s_construct ========
 */
static inline psa_status_t AESCTRDRBG_s_construct(psa_msg_t *msg)
{
    AESCTRDRBG_s_ConstructMsg constructMsg;
    AESCTRDRBG_Handle handle;
    AESCTRDRBG_Params params_s;
    const AESCTRDRBG_Params *paramsPtr_s = NULL;
    AESCTRDRBG_Config *configPtr_s;
    psa_status_t status         = PSA_ERROR_PROGRAMMER_ERROR;
    AESCTRDRBG_Handle retHandle = NULL;

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
            status = AESCTRDRBG_s_copyParams(&params_s, constructMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }

            paramsPtr_s = &params_s;
        }

        /* Verify config and copy to secure memory */
        status = AESCTRDRBG_s_copyConfig(&configPtr_s, constructMsg.config, &retHandle);
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

    handle = AESCTRDRBG_construct(configPtr_s, paramsPtr_s);

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
 *  ======== AESCTRDRBG_s_open ========
 */
static inline psa_status_t AESCTRDRBG_s_open(psa_msg_t *msg)
{
    AESCTRDRBG_s_OpenMsg openMsg;
    AESCTRDRBG_Handle handle;
    AESCTRDRBG_Params params_s;
    AESCTRDRBG_Params *paramsPtr_s = NULL;
    AESCTRDRBG_Handle retHandle    = NULL;
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
            status = AESCTRDRBG_s_copyParams(&params_s, openMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }
        }

        paramsPtr_s = &params_s;
    }

    handle = AESCTRDRBG_open(openMsg.index, paramsPtr_s);

    if (handle != NULL)
    {
        if (TFM_CLIENT_ID_IS_NS(msg->client_id))
        {
            /*
             * Return CRYPTO_S_HANDLE_ID_AESCTRDRBG OR'd with the config array index
             * as the handle instead of a pointer to the config to allow for
             * validation of handles provided by non-secure clients.
             */
            retHandle = (AESCTRDRBG_Handle)(CRYPTO_S_HANDLE_ID_AESCTRDRBG | openMsg.index);
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
 *  ======== AESCTRDRBG_s_close ========
 */
static inline psa_status_t AESCTRDRBG_s_close(psa_msg_t *msg)
{
    AESCTRDRBG_Handle handle_s;
    AESCTRDRBG_s_CloseMsg closeMsg;

    if (msg->in_size[0] != sizeof(closeMsg))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &closeMsg, sizeof(closeMsg));

    /* Non-secure callers have negative client ID */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESCTRDRBG_s_getHandle(closeMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        AESCTRDRBG_close(handle_s);

        /* Release the secure config if it is a dynamic instance */
        AESCTRDRBG_s_releaseConfig(closeMsg.handle);
    }
    else /* Secure client */
    {
        AESCTRDRBG_close(closeMsg.handle);
    }

    return PSA_SUCCESS;
}

/*
 *  ======== AESCTRDRBG_s_reseed ========
 */
static inline psa_status_t AESCTRDRBG_s_reseed(psa_msg_t *msg)
{
    AESCTRDRBG_s_ReseedMsg reseedMsg;
    AESCTRDRBG_Handle handle_s;
    AESCTRDRBGXX_Object *object;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(reseedMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &reseedMsg, sizeof(reseedMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESCTRDRBG_s_getHandle(reseedMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        object = handle_s->object;

        /* Validate seed address range */
        if (cmse_has_unpriv_nonsecure_read_access((void *)reseedMsg.seed, object->seedLength) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Validate additional data address range */
        if (cmse_has_unpriv_nonsecure_read_access((void *)reseedMsg.additionalData, reseedMsg.additionalDataLength) ==
            NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }
    }
    else /* Secure client */
    {
        handle_s = reseedMsg.handle;
    }

    ret = AESCTRDRBG_reseed(handle_s, reseedMsg.seed, reseedMsg.additionalData, reseedMsg.additionalDataLength);

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== AESCTRDRBG_s_getRandomBytes ========
 */
static inline psa_status_t AESCTRDRBG_s_getRandomBytes(psa_msg_t *msg)
{
    AESCTRDRBG_s_GetRandomBytesMsg getBytesMsg;
    AESCTRDRBG_Handle handle_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(getBytesMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &getBytesMsg, sizeof(getBytesMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESCTRDRBG_s_getHandle(getBytesMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Validate output address range */
        if (cmse_has_unpriv_nonsecure_read_access((void *)getBytesMsg.randomBytes, getBytesMsg.randomBytesSize) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }
    }
    else /* Secure client */
    {
        handle_s = getBytesMsg.handle;
    }

    ret = AESCTRDRBG_getRandomBytes(handle_s, getBytesMsg.randomBytes, getBytesMsg.randomBytesSize);

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== AESCTRDRBG_s_generateKey ========
 */
static inline psa_status_t AESCTRDRBG_s_generateKey(psa_msg_t *msg)
{
    AESCTRDRBG_s_GenerateKeyMsg genKeyMsg;
    CryptoKey randomKey_s;
    AESCTRDRBG_Handle handle_s;
    int_fast16_t ret    = AESCTRDRBG_STATUS_RESOURCE_UNAVAILABLE;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(genKeyMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &genKeyMsg, sizeof(genKeyMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = AESCTRDRBG_s_getHandle(genKeyMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        /* Copy randomKey to secure memory */
        (void)spm_memcpy(&randomKey_s, genKeyMsg.randomKey, sizeof(CryptoKey));

        if (CryptoKey_verifySecureOutputKey(&randomKey_s) != CryptoKey_STATUS_SUCCESS)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        ret = AESCTRDRBG_generateKey(handle_s, &randomKey_s);

        /* Copy the updated encoding to the non-secure key struct */
        genKeyMsg.randomKey->encoding = randomKey_s.encoding;

        if (randomKey_s.encoding == CryptoKey_KEYSTORE)
        {
            /* Copy the updated keyID to the non-secure key struct for KeyStore keys */
            genKeyMsg.randomKey->u.keyStore.keyID = randomKey_s.u.keyStore.keyID;
        }
    }
    else /* Secure client */
    {
        ret = AESCTRDRBG_generateKey(genKeyMsg.handle, genKeyMsg.randomKey);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== AESCTRDRBG_s_handlePsaMsg ========
 */
psa_status_t AESCTRDRBG_s_handlePsaMsg(psa_msg_t *msg)
{
    psa_status_t status = PSA_SUCCESS;

    switch (msg->type)
    {
        /*
         * If AESCTRDRBG_S_MSG_TYPE_CONSTRUCT is used by other secure partitions,
         * any pointer arguments provided must reference memory accessible by
         * this partition which is dependent on the TF-M isolation level in use.
         */
        case AESCTRDRBG_S_MSG_TYPE_CONSTRUCT:
            status = AESCTRDRBG_s_construct(msg);
            break;

        case AESCTRDRBG_S_MSG_TYPE_OPEN:
            status = AESCTRDRBG_s_open(msg);
            break;

        case AESCTRDRBG_S_MSG_TYPE_CLOSE:
            status = AESCTRDRBG_s_close(msg);
            break;

        case AESCTRDRBG_S_MSG_TYPE_RESEED:
            status = AESCTRDRBG_s_reseed(msg);
            break;

        case AESCTRDRBG_S_MSG_TYPE_GET_RANDOM_BYTES:
            status = AESCTRDRBG_s_getRandomBytes(msg);
            break;

        case AESCTRDRBG_S_MSG_TYPE_GENERATE_KEY:
            status = AESCTRDRBG_s_generateKey(msg);
            break;

        default:
            /* Unknown msg type */
            status = PSA_ERROR_PROGRAMMER_ERROR;
            break;
    }

    return status;
}

/*
 *  ======== AESCTRDRBG_s_init ========
 */
void AESCTRDRBG_s_init(void)
{
    AESCTRDRBG_init();
}
