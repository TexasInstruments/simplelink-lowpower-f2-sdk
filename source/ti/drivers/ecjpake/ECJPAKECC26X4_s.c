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

#include "ECJPAKECC26X4_s.h"

#include <ti/drivers/ECJPAKE.h>
#include <ti/drivers/ecjpake/ECJPAKECC26X2.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>
#include <ti/drivers/cryptoutils/ecc/ECCParamsCC26X4_s.h>

#include <ti/drivers/tfm/SecureCallback.h>

#include <third_party/tfm/secure_fw/spm/core/spm.h>
#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/interface/include/psa/service.h>
#include <third_party/tfm/secure_fw/spm/include/utilities.h>

#include <third_party/tfm/platform/ext/target/ti/cc26x4/cmse.h> /* TI CMSE helper functions */
#include "ti_drivers_config.h"                                  /* Sysconfig generated header */

#define ECJPAKE_SECURE_OPERATION_COUNT ECJPAKE_SECURE_CALLBACK_COUNT

/*
 * Union containing all supported ECJPAKE operation structs.
 */
typedef union
{
    ECJPAKE_OperationRoundOneGenerateKeys generateRoundOneKeys;
    ECJPAKE_OperationGenerateZKP generateZKP;
    ECJPAKE_OperationVerifyZKP verifyZKP;
    ECJPAKE_OperationRoundTwoGenerateKeys generateRoundTwoKeys;
    ECJPAKE_OperationComputeSharedSecret computeSharedSecret;
} ECJPAKE_OperationUnion;

/*
 * Struct containing secure copy of all crypto keys for round one generation and
 * the original pointers to the non-secure output keys to update the key
 * encoding return when the HWI callback occurs.
 */
typedef struct
{
    CryptoKey myPrivateKey1;
    CryptoKey myPrivateKey2;
    CryptoKey myPublicKey1;
    CryptoKey myPublicKey2;
    CryptoKey myPrivateV1;
    CryptoKey myPrivateV2;
    CryptoKey myPublicV1;
    CryptoKey myPublicV2;

    CryptoKey *myPublicKey1_ns;
    CryptoKey *myPublicKey2_ns;
    CryptoKey *myPublicV1_ns;
    CryptoKey *myPublicV2_ns;
} ECJPAKE_s_RoundOneCryptoKeys;

/*
 * Struct containing secure copy of all crypto keys for ZKP generation.
 */
typedef struct
{
    CryptoKey myPrivateKey;
    CryptoKey myPrivateV;
} ECJPAKE_s_GenerateZKPKeys;

/*
 * Struct containing secure copy of all crypto keys for ZKP verification.
 */
typedef struct
{
    CryptoKey theirGenerator;
    CryptoKey theirPublicKey;
    CryptoKey theirPublicV;
} ECJPAKE_s_VerifyZKPKeys;

/*
 * Struct containing secure copy of all crypto keys for round two generation and
 * the original pointers to the non-secure output keys to update the key
 * encoding return when the HWI callback occurs.
 */
typedef struct
{
    CryptoKey myPrivateKey2;
    CryptoKey myPublicKey1;
    CryptoKey myPublicKey2;
    CryptoKey theirPublicKey1;
    CryptoKey theirPublicKey2;
    CryptoKey preSharedSecret;

    CryptoKey theirNewGenerator;
    CryptoKey myNewGenerator;
    CryptoKey myCombinedPrivateKey;
    CryptoKey myCombinedPublicKey;
    CryptoKey myPrivateV;
    CryptoKey myPublicV;

    CryptoKey *theirNewGenerator_ns;
    CryptoKey *myNewGenerator_ns;
    CryptoKey *myCombinedPrivateKey_ns;
    CryptoKey *myCombinedPublicKey_ns;
    CryptoKey *myPublicV_ns;
} ECJPAKE_s_RoundTwoCryptoKeys;

/*
 * Struct containing secure copy of all crypto keys for shared secret
 * computation and the original pointer to the non-secure output key to update
 * the key encoding return when the HWI callback occurs.
 */
typedef struct
{
    CryptoKey myCombinedPrivateKey;
    CryptoKey theirCombinedPublicKey;
    CryptoKey theirPublicKey2;
    CryptoKey myPrivateKey2;
    CryptoKey sharedSecret;
    CryptoKey *sharedSecret_ns;
} ECJPAKE_s_SharedSecretCryptoKeys;

/*
 * A union of secure crypto keys structs for all supported operations.
 */
typedef union
{
    ECJPAKE_s_RoundOneCryptoKeys roundOneKeys;
    ECJPAKE_s_GenerateZKPKeys generateZKPKeys;
    ECJPAKE_s_VerifyZKPKeys verifyZKPKeys;
    ECJPAKE_s_RoundTwoCryptoKeys roundTwoKeys;
    ECJPAKE_s_SharedSecretCryptoKeys sharedSecretKeys;
} ECJPAKE_s_CryptoKeys;

/*
 * Stores the following:
 * - Secure copy of the operation and the original pointer to the non-secure
 *   operation to return when the HWI callback occurs.
 * - Secure copy of the crypto keys and the original pointer to the non-secure
 *   output keys to update the key encoding return when the HWI callback occurs.
 */
typedef struct
{
    ECJPAKE_Operation operation_ns;     /* Pointer to non-secure operation */
    ECJPAKE_OperationUnion operation_s; /* Secure copy of operation */
    ECJPAKE_s_CryptoKeys keys_s;
} ECJPAKE_s_Operation;

static ECJPAKE_s_Operation ECJPAKE_s_operation;

/*
 * ECJPAKE Secure Dynamic Instance struct.
 */
typedef struct
{
    ECJPAKE_Config config;
    ECJPAKECC26X2_Object object;
    ECJPAKECC26X2_HWAttrs hwAttrs;
} ECJPAKE_s_DynamicInstance;

/*
 * Used to store a secure copy of the dynamic instance (config plus the
 * object and hwAttrs that it points to) provided by non-secure calls to
 * ECJPAKE_construct.
 */
static ECJPAKE_s_DynamicInstance ECJPAKE_s_dynInstance[CONFIG_ECJPAKE_S_CONFIG_POOL_SIZE];

/*
 * Stores pointers to non-secure ECJPAKE_s_SecureCallbacks for each driver
 * instance opened or constructed.
 */
static ECJPAKE_s_SecureCallback *ECJPAKE_s_secureCB[ECJPAKE_SECURE_CALLBACK_COUNT];

/* Static driver instances allocated by SysConfig */
extern const ECJPAKE_Config ECJPAKE_config[];

/*
 *  ======== ECJPAKE_s_getCallbackIndex ========
 *  Returns callback index or -1 if index is not found.
 */
static int8_t ECJPAKE_s_getCallbackIndex(ECJPAKE_Handle handle_s)
{
    uint_fast8_t index;
    int8_t retIndex = -1;
    bool indexFound = false;

    /* First, search config statically allocated by SysConfig */
    for (index = 0; index < CONFIG_TI_DRIVERS_ECJPAKE_COUNT; index++)
    {
        if (handle_s == (ECJPAKE_Handle)&ECJPAKE_config[index])
        {
            indexFound = true;
            break;
        }
    }

    /* If not found, search secure config copies */
    if (!indexFound)
    {
        for (index = 0; index < CONFIG_ECJPAKE_S_CONFIG_POOL_SIZE; index++)
        {
            if (handle_s == &ECJPAKE_s_dynInstance[index].config)
            {
                index += CONFIG_TI_DRIVERS_ECJPAKE_COUNT;
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
 *  ======== ECJPAKE_s_handleRoundOneKeyResult ========
 */
static void ECJPAKE_s_handleRoundOneKeyResult(ECJPAKE_OperationRoundOneGenerateKeys *opKeys_s)
{
    ECJPAKE_s_RoundOneCryptoKeys *roundOneKeys_s = &ECJPAKE_s_operation.keys_s.roundOneKeys;

    /* Copy updated key encodings to non-secure CryptoKey structs */
    roundOneKeys_s->myPublicKey1_ns->encoding = opKeys_s->myPublicKey1->encoding;
    roundOneKeys_s->myPublicKey2_ns->encoding = opKeys_s->myPublicKey2->encoding;
    roundOneKeys_s->myPublicV1_ns->encoding   = opKeys_s->myPublicV1->encoding;
    roundOneKeys_s->myPublicV2_ns->encoding   = opKeys_s->myPublicV2->encoding;

    /* For any keystore CryptoKeys, copy updated KeyIDs to non-secure CryptoKey structs */
    if (roundOneKeys_s->myPublicKey1_ns->encoding == CryptoKey_KEYSTORE)
    {
        roundOneKeys_s->myPublicKey1_ns->u.keyStore.keyID = opKeys_s->myPublicKey1->u.keyStore.keyID;
    }

    if (roundOneKeys_s->myPublicKey2_ns->encoding == CryptoKey_KEYSTORE)
    {
        roundOneKeys_s->myPublicKey2_ns->u.keyStore.keyID = opKeys_s->myPublicKey2->u.keyStore.keyID;
    }

    if (roundOneKeys_s->myPublicV1_ns->encoding == CryptoKey_KEYSTORE)
    {
        roundOneKeys_s->myPublicV1_ns->u.keyStore.keyID = opKeys_s->myPublicV1->u.keyStore.keyID;
    }

    if (roundOneKeys_s->myPublicV2_ns->encoding == CryptoKey_KEYSTORE)
    {
        roundOneKeys_s->myPublicV2_ns->u.keyStore.keyID = opKeys_s->myPublicV2->u.keyStore.keyID;
    }
}

/*
 *  ======== ECJPAKE_s_handleRoundTwoKeyResult ========
 */
static void ECJPAKE_s_handleRoundTwoKeyResult(ECJPAKE_OperationRoundTwoGenerateKeys *opKeys_s)
{
    ECJPAKE_s_RoundTwoCryptoKeys *roundTwoKeys_s = &ECJPAKE_s_operation.keys_s.roundTwoKeys;

    /* Copy updated key encodings to non-secure CryptoKey structs */
    roundTwoKeys_s->theirNewGenerator_ns->encoding    = opKeys_s->theirNewGenerator->encoding;
    roundTwoKeys_s->myNewGenerator_ns->encoding       = opKeys_s->myNewGenerator->encoding;
    roundTwoKeys_s->myCombinedPrivateKey_ns->encoding = opKeys_s->myCombinedPrivateKey->encoding;
    roundTwoKeys_s->myCombinedPublicKey_ns->encoding  = opKeys_s->myCombinedPublicKey->encoding;
    roundTwoKeys_s->myPublicV_ns->encoding            = opKeys_s->myPublicV->encoding;

    /* For any keystore CryptoKeys, copy updated KeyIDs to non-secure CryptoKey structs */
    if (roundTwoKeys_s->theirNewGenerator_ns->encoding == CryptoKey_KEYSTORE)
    {
        roundTwoKeys_s->theirNewGenerator_ns->u.keyStore.keyID = opKeys_s->theirNewGenerator->u.keyStore.keyID;
    }

    if (roundTwoKeys_s->myNewGenerator_ns->encoding == CryptoKey_KEYSTORE)
    {
        roundTwoKeys_s->myNewGenerator_ns->u.keyStore.keyID = opKeys_s->myNewGenerator->u.keyStore.keyID;
    }

    if (roundTwoKeys_s->myCombinedPrivateKey_ns->encoding == CryptoKey_KEYSTORE)
    {
        roundTwoKeys_s->myCombinedPrivateKey_ns->u.keyStore.keyID = opKeys_s->myCombinedPrivateKey->u.keyStore.keyID;
    }

    if (roundTwoKeys_s->myCombinedPublicKey_ns->encoding == CryptoKey_KEYSTORE)
    {
        roundTwoKeys_s->myCombinedPublicKey_ns->u.keyStore.keyID = opKeys_s->myCombinedPublicKey->u.keyStore.keyID;
    }

    if (roundTwoKeys_s->myPublicV_ns->encoding == CryptoKey_KEYSTORE)
    {
        roundTwoKeys_s->myPublicV_ns->u.keyStore.keyID = opKeys_s->myPublicV->u.keyStore.keyID;
    }
}

/*
 *  ======== ECJPAKE_s_hwiCallback ========
 */
static void ECJPAKE_s_hwiCallback(ECJPAKE_Handle handle_s,
                                  int_fast16_t returnStatus,
                                  ECJPAKE_Operation operation,
                                  ECJPAKE_OperationType operationType)
{
    int8_t index;
    ECJPAKE_s_SecureCallback *ecjpakeSecureCB_ns;

    index = ECJPAKE_s_getCallbackIndex(handle_s);

    if ((index >= 0) && (index < ECJPAKE_SECURE_CALLBACK_COUNT))
    {
        ecjpakeSecureCB_ns = ECJPAKE_s_secureCB[index];

        if (ecjpakeSecureCB_ns != NULL)
        {
            /* Store arguments in callback object for use by non-secure handler */
            ecjpakeSecureCB_ns->handle        = (ECJPAKE_Handle)(CRYPTO_S_HANDLE_ID_ECJPAKE | index);
            ecjpakeSecureCB_ns->returnStatus  = returnStatus;
            ecjpakeSecureCB_ns->operation     = ECJPAKE_s_operation.operation_ns;
            ecjpakeSecureCB_ns->operationType = operationType;

            /* Copy updated key encoding to the non-secure key struct */
            if (operationType == ECJPAKE_OPERATION_TYPE_ROUND_ONE_GENERATE_KEYS)
            {
                ECJPAKE_s_handleRoundOneKeyResult(operation.generateRoundOneKeys);
            }
            else if (operationType == ECJPAKE_OPERATION_TYPE_ROUND_TWO_GENERATE_KEYS)
            {
                ECJPAKE_s_handleRoundTwoKeyResult(operation.generateRoundTwoKeys);
            }
            else if (operationType == ECJPAKE_OPERATION_TYPE_COMPUTE_SHARED_SECRET)
            {
                ECJPAKE_s_SharedSecretCryptoKeys *sharedSecretKeys_s = &ECJPAKE_s_operation.keys_s.sharedSecretKeys;

                /* Copy updated key encoding to non-secure CryptoKey struct */
                sharedSecretKeys_s->sharedSecret_ns->encoding = operation.computeSharedSecret->sharedSecret->encoding;

                if (sharedSecretKeys_s->sharedSecret_ns->encoding == CryptoKey_KEYSTORE)
                {
                    /* Copy updated KeyID to non-secure CryptoKey struct */
                    sharedSecretKeys_s->sharedSecret_ns->u.keyStore.keyID = operation.computeSharedSecret->sharedSecret
                                                                                ->u.keyStore.keyID;
                }
            }

            /* Trigger the interrupt for the non-secure callback dispatcher */
            SecureCallback_post(&ecjpakeSecureCB_ns->object);
        }
    }
}

/*
 *  ======== ECJPAKE_s_copyConfig ========
 */
static inline psa_status_t ECJPAKE_s_copyConfig(ECJPAKE_Config **secureConfig,
                                                const ECJPAKE_Config *config,
                                                ECJPAKE_Handle *retHandle)
{
    ECJPAKE_Config *config_s;
    ECJPAKE_s_DynamicInstance *dynInstance_s;
    uint_fast8_t i;

    for (i = 0; i < CONFIG_ECJPAKE_S_CONFIG_POOL_SIZE; i++)
    {
        dynInstance_s = &ECJPAKE_s_dynInstance[i];
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

            /* Set the secure callback object pointer to NULL. The non-secure
             * client must register a secure callback after constructing or
             * opening a driver instance.
             */
            ECJPAKE_s_secureCB[i + CONFIG_TI_DRIVERS_ECJPAKE_COUNT] = NULL;

            /* Return handle is the CRYPTO_S_HANDLE_ID_ECJPAKE OR'd with the
             * the config pool array index plus the size of constant config
             * array created by SysConfig.
             */
            *retHandle = (ECJPAKE_Handle)(CRYPTO_S_HANDLE_ID_ECJPAKE | (i + CONFIG_TI_DRIVERS_ECJPAKE_COUNT));
            return PSA_SUCCESS;
        }
    }

    return PSA_ERROR_INSUFFICIENT_MEMORY;
}

/*
 *  ======== ECJPAKE_s_releaseConfig ========
 */
static inline void ECJPAKE_s_releaseConfig(ECJPAKE_Handle nsHandle)
{
    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_ECJPAKE)
    {
        /* Extract the secure handle index */
        uintptr_t i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        /* Check whether the handle instance refers to a dynamic instance */
        if ((i >= CONFIG_TI_DRIVERS_ECJPAKE_COUNT) && (i < ECJPAKE_SECURE_CALLBACK_COUNT))
        {
            /* Set config's object pointer to NULL to indicate the config is available */
            ECJPAKE_s_dynInstance[i - CONFIG_TI_DRIVERS_ECJPAKE_COUNT].config.object = NULL;
        }
    }
}

/*
 *  ======== ECJPAKE_s_copyRoundOneGenKeyOp ========
 */
static inline psa_status_t ECJPAKE_s_copyRoundOneGenKeyOp(ECJPAKE_OperationRoundOneGenerateKeys *secureOperation,
                                                          ECJPAKE_s_RoundOneCryptoKeys *secureKeys,
                                                          const ECJPAKE_OperationRoundOneGenerateKeys *operation)
{
    const ECCParams_CurveParams *curveParams_s;
    int_fast16_t status;

    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(ECJPAKE_OperationRoundOneGenerateKeys)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(ECJPAKE_OperationRoundOneGenerateKeys));

    /* Get a pointer to secure curve params */
    curveParams_s = ECCParams_s_getCurveParams(secureOperation->curve);
    if (curveParams_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Update the operation struct to point to secure curve params */
    secureOperation->curve = curveParams_s;

    /* Make a secure copy of all key structs and update the operation struct to
     * point to the secure key copy.
     */
    status = CryptoKey_copySecureInputKey(&secureKeys->myPrivateKey1, &secureOperation->myPrivateKey1);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureInputKey(&secureKeys->myPrivateKey2, &secureOperation->myPrivateKey2);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureOutputKey(&secureKeys->myPublicKey1, &secureOperation->myPublicKey1);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureOutputKey(&secureKeys->myPublicKey2, &secureOperation->myPublicKey2);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureInputKey(&secureKeys->myPrivateV1, &secureOperation->myPrivateV1);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureInputKey(&secureKeys->myPrivateV2, &secureOperation->myPrivateV2);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureOutputKey(&secureKeys->myPublicV1, &secureOperation->myPublicV1);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureOutputKey(&secureKeys->myPublicV2, &secureOperation->myPublicV2);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    return PSA_SUCCESS;
}

/*
 *  ======== ECJPAKE_s_copyGenerateOperation ========
 */
static inline psa_status_t ECJPAKE_s_copyGenerateZKPOperation(ECJPAKE_OperationGenerateZKP *secureOperation,
                                                              ECJPAKE_s_GenerateZKPKeys *secureKeys,
                                                              const ECJPAKE_OperationGenerateZKP *operation)
{
    const ECCParams_CurveParams *curveParams_s;
    int_fast16_t status;

    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(ECJPAKE_OperationGenerateZKP)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(ECJPAKE_OperationGenerateZKP));

    /* Get a pointer to secure curve params */
    curveParams_s = ECCParams_s_getCurveParams(secureOperation->curve);
    if (curveParams_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Update the operation struct to point to secure curve params */
    secureOperation->curve = curveParams_s;

    /* Validate input hash address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)secureOperation->hash, secureOperation->curve->length) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Validate output 'r' component address range */
    if (cmse_has_unpriv_nonsecure_read_access(secureOperation->r, secureOperation->curve->length) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of all key structs and update the operation struct to
     * point to the secure key copy.
     */
    status = CryptoKey_copySecureInputKey(&secureKeys->myPrivateKey, &secureOperation->myPrivateKey);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureInputKey(&secureKeys->myPrivateV, &secureOperation->myPrivateV);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    return PSA_SUCCESS;
}

/*
 *  ======== ECJPAKE_s_copyVerifyOperation ========
 */
static inline psa_status_t ECJPAKE_s_copyVerifyZKPOperation(ECJPAKE_OperationVerifyZKP *secureOperation,
                                                            ECJPAKE_s_VerifyZKPKeys *secureKeys,
                                                            const ECJPAKE_OperationVerifyZKP *operation)
{
    const ECCParams_CurveParams *curveParams_s;
    int_fast16_t status;

    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(ECJPAKE_OperationVerifyZKP)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(ECJPAKE_OperationVerifyZKP));

    /* Get a pointer to secure curve params */
    curveParams_s = ECCParams_s_getCurveParams(secureOperation->curve);
    if (curveParams_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Update the operation struct to point to secure curve params */
    secureOperation->curve = curveParams_s;

    /* Validate input hash address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)secureOperation->hash, secureOperation->curve->length) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Validate input 'r' component address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)secureOperation->r, secureOperation->curve->length) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of all key structs and update the operation struct to
     * point to the secure key copy.
     */

    /* theirGenerator may be NULL in which case default generator point from
     * the curve is used and there is no crypto key to copy.
     */
    if (secureOperation->theirGenerator != NULL)
    {
        status = CryptoKey_copySecureInputKey(&secureKeys->theirGenerator, &secureOperation->theirGenerator);
        if (status != PSA_SUCCESS)
        {
            return status;
        }
    }

    status = CryptoKey_copySecureInputKey(&secureKeys->theirPublicKey, &secureOperation->theirPublicKey);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureInputKey(&secureKeys->theirPublicV, &secureOperation->theirPublicV);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    return PSA_SUCCESS;
}

/*
 *  ======== ECJPAKE_s_copyRoundTwoGenKeyOp ========
 */
static inline psa_status_t ECJPAKE_s_copyRoundTwoGenKeyOp(ECJPAKE_OperationRoundTwoGenerateKeys *secureOperation,
                                                          ECJPAKE_s_RoundTwoCryptoKeys *secureKeys,
                                                          const ECJPAKE_OperationRoundTwoGenerateKeys *operation)
{
    const ECCParams_CurveParams *curveParams_s;
    int_fast16_t status;

    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(ECJPAKE_OperationRoundTwoGenerateKeys)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(ECJPAKE_OperationRoundTwoGenerateKeys));

    /* Get a pointer to secure curve params */
    curveParams_s = ECCParams_s_getCurveParams(secureOperation->curve);
    if (curveParams_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Update the operation struct to point to secure curve params */
    secureOperation->curve = curveParams_s;

    /* Make a secure copy of all key structs and update the operation struct to
     * point to the secure key copy.
     */
    status = CryptoKey_copySecureInputKey(&secureKeys->myPrivateKey2, &secureOperation->myPrivateKey2);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureInputKey(&secureKeys->myPublicKey1, &secureOperation->myPublicKey1);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureInputKey(&secureKeys->myPublicKey2, &secureOperation->myPublicKey2);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureInputKey(&secureKeys->theirPublicKey1, &secureOperation->theirPublicKey1);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureInputKey(&secureKeys->theirPublicKey2, &secureOperation->theirPublicKey2);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureInputKey(&secureKeys->preSharedSecret, &secureOperation->preSharedSecret);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureOutputKey(&secureKeys->theirNewGenerator, &secureOperation->theirNewGenerator);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureOutputKey(&secureKeys->myNewGenerator, &secureOperation->myNewGenerator);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureOutputKey(&secureKeys->myCombinedPrivateKey, &secureOperation->myCombinedPrivateKey);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureOutputKey(&secureKeys->myCombinedPublicKey, &secureOperation->myCombinedPublicKey);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureInputKey(&secureKeys->myPrivateV, &secureOperation->myPrivateV);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureOutputKey(&secureKeys->myPublicV, &secureOperation->myPublicV);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    return PSA_SUCCESS;
}

/*
 *  ======== ECJPAKE_s_copySharedSecretOp ========
 */
static inline psa_status_t ECJPAKE_s_copyComputeSharedSecretOp(ECJPAKE_OperationComputeSharedSecret *secureOperation,
                                                               ECJPAKE_s_SharedSecretCryptoKeys *secureKeys,
                                                               const ECJPAKE_OperationComputeSharedSecret *operation)
{
    const ECCParams_CurveParams *curveParams_s;
    int_fast16_t status;

    /* Validate operation struct address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)operation, sizeof(ECJPAKE_OperationComputeSharedSecret)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Make a secure copy of the operation struct */
    (void)spm_memcpy(secureOperation, operation, sizeof(ECJPAKE_OperationComputeSharedSecret));

    /* Get a pointer to secure curve params */
    curveParams_s = ECCParams_s_getCurveParams(secureOperation->curve);
    if (curveParams_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Update the operation struct to point to secure curve params */
    secureOperation->curve = curveParams_s;

    /* Make a secure copy of all key structs and update the operation struct to
     * point to the secure key copy.
     */
    status = CryptoKey_copySecureInputKey(&secureKeys->myCombinedPrivateKey, &secureOperation->myCombinedPrivateKey);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureInputKey(&secureKeys->theirCombinedPublicKey,
                                          &secureOperation->theirCombinedPublicKey);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureInputKey(&secureKeys->theirPublicKey2, &secureOperation->theirPublicKey2);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureInputKey(&secureKeys->myPrivateKey2, &secureOperation->myPrivateKey2);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = CryptoKey_copySecureOutputKey(&secureKeys->sharedSecret, &secureOperation->sharedSecret);
    if (status != CryptoKey_STATUS_SUCCESS)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    return PSA_SUCCESS;
}

/*
 *  ======== ECJPAKE_s_copyParams ========
 */
static psa_status_t ECJPAKE_s_copyParams(ECJPAKE_Params *secureParams, const ECJPAKE_Params *params)
{
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Validate params address range */
    if (cmse_has_unpriv_nonsecure_read_access((void *)params, sizeof(ECJPAKE_Params)) == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    (void)spm_memcpy(secureParams, params, sizeof(ECJPAKE_Params));

    /* Validate the return behavior */
    if ((secureParams->returnBehavior == ECJPAKE_RETURN_BEHAVIOR_CALLBACK) ||
        (secureParams->returnBehavior == ECJPAKE_RETURN_BEHAVIOR_BLOCKING) ||
        (secureParams->returnBehavior == ECJPAKE_RETURN_BEHAVIOR_POLLING))
    {
        /* Overwrite the non-secure client's callback function with our own
         * callback which will populate the secure callback object registered
         * using ECJPAKE_S_MSG_TYPE_REGISTER_CALLBACK.
         */
        secureParams->callbackFxn = ECJPAKE_s_hwiCallback;

        /* The underlying ECJPAKE driver is interrupt-driven regardless of the return
         * behavior specified. Since secure partitions cannot process interrupt
         * signals while a PSA call is running, callback return behavior must be
         * forced for all app-specified return behaviors including polling.
         */
        secureParams->returnBehavior = ECJPAKE_RETURN_BEHAVIOR_CALLBACK;

        status = PSA_SUCCESS;
    }

    return status;
}

/*
 *  ======== ECJPAKE_s_getHandle ========
 *  Returns a Secure handle from a handle provided from non-secure client.
 */
static ECJPAKE_Handle ECJPAKE_s_getHandle(ECJPAKE_Handle nsHandle)
{
    ECJPAKE_Handle secureHandle = NULL;
    uint32_t i;

    if (GET_CRYPTO_S_HANDLE_ID(nsHandle) == CRYPTO_S_HANDLE_ID_ECJPAKE)
    {
        /* Extract the secure handle index */
        i = GET_CRYPTO_S_HANDLE_INDEX(nsHandle);

        if (i < CONFIG_TI_DRIVERS_ECJPAKE_COUNT)
        {
            secureHandle = (ECJPAKE_Handle)&ECJPAKE_config[i];
        }
        else if ((i >= CONFIG_TI_DRIVERS_ECJPAKE_COUNT) && (i < ECJPAKE_SECURE_CALLBACK_COUNT))
        {
            secureHandle = &ECJPAKE_s_dynInstance[i - CONFIG_TI_DRIVERS_ECJPAKE_COUNT].config;
        }
    }

    return secureHandle;
}

/*
 *  ======== ECJPAKE_s_registerCallback ========
 */
static inline psa_status_t ECJPAKE_s_registerCallback(psa_msg_t *msg)
{
    ECJPAKE_Handle handle_s;
    ECJPAKE_s_CallbackMsg callbackMsg;
    int8_t callbackIndex;
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    /* Only non-secure callers should be registering callbacks */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id) && (msg->in_size[0] == sizeof(callbackMsg)))
    {
        psa_read(msg->handle, 0, &callbackMsg, sizeof(callbackMsg));

        handle_s = ECJPAKE_s_getHandle(callbackMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        callbackIndex = ECJPAKE_s_getCallbackIndex(handle_s);

        /* Validate index and callback object address range */
        if ((callbackIndex >= 0) && (callbackIndex < ECJPAKE_SECURE_CALLBACK_COUNT) &&
            (cmse_has_unpriv_nonsecure_rw_access(callbackMsg.callback, sizeof(ECJPAKE_s_SecureCallback)) != NULL))
        {
            /* Store the pointer to ECJPAKE_s_SecureCallback located in
             * non-secure memory.
             */
            ECJPAKE_s_secureCB[callbackIndex] = callbackMsg.callback;

            status = PSA_SUCCESS;
        }
    }

    return status;
}

/*
 *  ======== ECJPAKE_s_construct ========
 */
static inline psa_status_t ECJPAKE_s_construct(psa_msg_t *msg)
{
    ECJPAKE_s_ConstructMsg constructMsg;
    ECJPAKE_Handle handle;
    ECJPAKE_Params params_s;
    const ECJPAKE_Params *paramsPtr_s = NULL;
    ECJPAKE_Config *configPtr_s;
    psa_status_t status      = PSA_ERROR_PROGRAMMER_ERROR;
    ECJPAKE_Handle retHandle = NULL;

    if ((msg->in_size[0] != sizeof(constructMsg)) || (msg->out_size[0] != sizeof(handle)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &constructMsg, sizeof(constructMsg));

    /* For non-secure callers, the params and config must be verified and
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
            /* Copy params to secure memory and substitute our own callback
             * if callback return behavior is specified.
             */
            status = ECJPAKE_s_copyParams(&params_s, constructMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }

            paramsPtr_s = &params_s;
        }

        /* Verify config and copy to secure memory */
        status = ECJPAKE_s_copyConfig(&configPtr_s, constructMsg.config, &retHandle);
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

    handle = ECJPAKE_construct(configPtr_s, paramsPtr_s);

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
        /* Return the pointer to the secure config struct provided by the
         * secure caller.
         */
        retHandle = handle;
    }

    psa_write(msg->handle, 0, &retHandle, sizeof(retHandle));

    return PSA_SUCCESS;
}

/*
 *  ======== ECJPAKE_s_open ========
 */
static inline psa_status_t ECJPAKE_s_open(psa_msg_t *msg)
{
    ECJPAKE_s_OpenMsg openMsg;
    ECJPAKE_Handle handle;
    ECJPAKE_Params params_s;
    ECJPAKE_Params *paramsPtr_s = NULL;
    ECJPAKE_Handle retHandle    = NULL;
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
            status = ECJPAKE_s_copyParams(&params_s, openMsg.params);
            if (status != PSA_SUCCESS)
            {
                return status;
            }
        }

        paramsPtr_s = &params_s;
    }

    handle = ECJPAKE_open(openMsg.index, paramsPtr_s);

    if (handle != NULL)
    {
        if (TFM_CLIENT_ID_IS_NS(msg->client_id))
        {
            /* Set the secure callback object pointer to NULL. The non-secure
             * client must register a secure callback after constructing or
             * opening a driver instance.
             */
            ECJPAKE_s_secureCB[openMsg.index] = NULL;

            /* Return CRYPTO_S_HANDLE_ID_ECJPAKE OR'd with the config array index
             * as the handle instead of a pointer to the config to allow for
             * validation of handles provided by non-secure clients.
             */
            retHandle = (ECJPAKE_Handle)(CRYPTO_S_HANDLE_ID_ECJPAKE | openMsg.index);
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
 *  ======== ECJPAKE_s_close ========
 */
static inline psa_status_t ECJPAKE_s_close(psa_msg_t *msg)
{
    ECJPAKE_Handle handle_s;
    ECJPAKE_s_CloseMsg closeMsg;

    if (msg->in_size[0] != sizeof(closeMsg))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &closeMsg, sizeof(closeMsg));

    /* Non-secure callers have negative client ID */
    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = ECJPAKE_s_getHandle(closeMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        ECJPAKE_close(handle_s);

        /* Release the secure config if it is a dynamic */
        {
            ECJPAKE_s_releaseConfig(closeMsg.handle);
        }
    }
    else /* Secure client */
    {
        ECJPAKE_close(closeMsg.handle);
    }

    return PSA_SUCCESS;
}

/*
 *  ======== ECJPAKE_s_roundOneGenerateKeys ========
 */
static inline psa_status_t ECJPAKE_s_roundOneGenerateKeys(psa_msg_t *msg)
{
    ECJPAKE_s_OperationMsg genKeysMsg;
    ECJPAKE_Handle handle_s;
    ECJPAKE_OperationRoundOneGenerateKeys *operation_s;
    ECJPAKE_OperationRoundOneGenerateKeys *operation_ns;
    ECJPAKE_s_RoundOneCryptoKeys *keys_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(genKeysMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &genKeysMsg, sizeof(genKeysMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = ECJPAKE_s_getHandle(genKeysMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &ECJPAKE_s_operation.operation_s.generateRoundOneKeys;
        keys_s      = &ECJPAKE_s_operation.keys_s.roundOneKeys;

        operation_ns = genKeysMsg.operation.generateRoundOneKeys;

        /* Save pointer to non-secure operation struct */
        ECJPAKE_s_operation.operation_ns = genKeysMsg.operation;

        /* Save pointers to NS output crypto keys */
        keys_s->myPublicKey1_ns = operation_ns->myPublicKey1;
        keys_s->myPublicKey2_ns = operation_ns->myPublicKey2;
        keys_s->myPublicV1_ns   = operation_ns->myPublicV1;
        keys_s->myPublicV2_ns   = operation_ns->myPublicV2;

        /* Validate and copy operation and key structs */
        status = ECJPAKE_s_copyRoundOneGenKeyOp(operation_s, keys_s, operation_ns);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = ECJPAKE_roundOneGenerateKeys(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = ECJPAKE_roundOneGenerateKeys(genKeysMsg.handle, genKeysMsg.operation.generateRoundOneKeys);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== ECJPAKE_s_generateZKP ========
 */
static inline psa_status_t ECJPAKE_s_generateZKP(psa_msg_t *msg)
{
    ECJPAKE_s_OperationMsg genZKPMsg;
    ECJPAKE_Handle handle_s;
    ECJPAKE_OperationGenerateZKP *operation_s;
    ECJPAKE_s_GenerateZKPKeys *keys_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(genZKPMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &genZKPMsg, sizeof(genZKPMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = ECJPAKE_s_getHandle(genZKPMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &ECJPAKE_s_operation.operation_s.generateZKP;
        keys_s      = &ECJPAKE_s_operation.keys_s.generateZKPKeys;

        /* Save pointer to non-secure operation struct */
        ECJPAKE_s_operation.operation_ns = genZKPMsg.operation;

        /* Validate and copy operation and key structs */
        status = ECJPAKE_s_copyGenerateZKPOperation(operation_s, keys_s, genZKPMsg.operation.generateZKP);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = ECJPAKE_generateZKP(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = ECJPAKE_generateZKP(genZKPMsg.handle, genZKPMsg.operation.generateZKP);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== ECJPAKE_s_verifyZKP ========
 */
static inline psa_status_t ECJPAKE_s_verifyZKP(psa_msg_t *msg)
{
    ECJPAKE_s_OperationMsg verifyKeysMsg;
    ECJPAKE_Handle handle_s;
    ECJPAKE_OperationVerifyZKP *operation_s;
    ECJPAKE_s_VerifyZKPKeys *keys_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(verifyKeysMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &verifyKeysMsg, sizeof(verifyKeysMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = ECJPAKE_s_getHandle(verifyKeysMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &ECJPAKE_s_operation.operation_s.verifyZKP;
        keys_s      = &ECJPAKE_s_operation.keys_s.verifyZKPKeys;

        /* Save pointer to non-secure operation struct */
        ECJPAKE_s_operation.operation_ns = verifyKeysMsg.operation;

        /* Validate and copy operation and key structs */
        status = ECJPAKE_s_copyVerifyZKPOperation(operation_s, keys_s, verifyKeysMsg.operation.verifyZKP);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = ECJPAKE_verifyZKP(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = ECJPAKE_verifyZKP(verifyKeysMsg.handle, verifyKeysMsg.operation.verifyZKP);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== ECJPAKE_s_roundTwoGenerateKeys ========
 */
static inline psa_status_t ECJPAKE_s_roundTwoGenerateKeys(psa_msg_t *msg)
{
    ECJPAKE_s_OperationMsg genKeysMsg;
    ECJPAKE_Handle handle_s;
    ECJPAKE_OperationRoundTwoGenerateKeys *operation_s;
    ECJPAKE_OperationRoundTwoGenerateKeys *operation_ns;
    ECJPAKE_s_RoundTwoCryptoKeys *keys_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(genKeysMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &genKeysMsg, sizeof(genKeysMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = ECJPAKE_s_getHandle(genKeysMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &ECJPAKE_s_operation.operation_s.generateRoundTwoKeys;
        keys_s      = &ECJPAKE_s_operation.keys_s.roundTwoKeys;

        operation_ns = genKeysMsg.operation.generateRoundTwoKeys;

        /* Save pointer to non-secure operation struct */
        ECJPAKE_s_operation.operation_ns = genKeysMsg.operation;

        /* Save pointers to NS output crypto keys */
        keys_s->theirNewGenerator_ns    = operation_ns->theirNewGenerator;
        keys_s->myNewGenerator_ns       = operation_ns->myNewGenerator;
        keys_s->myCombinedPrivateKey_ns = operation_ns->myCombinedPrivateKey;
        keys_s->myCombinedPublicKey_ns  = operation_ns->myCombinedPublicKey;
        keys_s->myPublicV_ns            = operation_ns->myPublicV;

        /* Validate and copy operation and key structs */
        status = ECJPAKE_s_copyRoundTwoGenKeyOp(operation_s, keys_s, operation_ns);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = ECJPAKE_roundTwoGenerateKeys(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = ECJPAKE_roundTwoGenerateKeys(genKeysMsg.handle, genKeysMsg.operation.generateRoundTwoKeys);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== ECJPAKE_s_computeSharedSecret ========
 */
static inline psa_status_t ECJPAKE_s_computeSharedSecret(psa_msg_t *msg)
{
    ECJPAKE_s_OperationMsg sharedSecretMsg;
    ECJPAKE_Handle handle_s;
    ECJPAKE_OperationComputeSharedSecret *operation_s;
    ECJPAKE_s_SharedSecretCryptoKeys *keys_s;
    int_fast16_t ret;
    psa_status_t status = PSA_SUCCESS;

    if ((msg->in_size[0] != sizeof(sharedSecretMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &sharedSecretMsg, sizeof(sharedSecretMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        handle_s = ECJPAKE_s_getHandle(sharedSecretMsg.handle);
        if (handle_s == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        operation_s = &ECJPAKE_s_operation.operation_s.computeSharedSecret;
        keys_s      = &ECJPAKE_s_operation.keys_s.sharedSecretKeys;

        /* Save pointer to non-secure operation struct */
        ECJPAKE_s_operation.operation_ns = sharedSecretMsg.operation;

        /* Save pointer to NS output crypto key */
        keys_s->sharedSecret_ns = sharedSecretMsg.operation.computeSharedSecret->sharedSecret;

        /* Validate and copy operation and key structs */
        status = ECJPAKE_s_copyComputeSharedSecretOp(operation_s,
                                                     keys_s,
                                                     sharedSecretMsg.operation.computeSharedSecret);
        if (status != PSA_SUCCESS)
        {
            return status;
        }

        ret = ECJPAKE_computeSharedSecret(handle_s, operation_s);
    }
    else /* Secure client */
    {
        ret = ECJPAKE_computeSharedSecret(sharedSecretMsg.handle, sharedSecretMsg.operation.computeSharedSecret);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return status;
}

/*
 *  ======== ECJPAKE_s_cancelOperation ========
 */
static inline psa_status_t ECJPAKE_s_cancelOperation(psa_msg_t *msg)
{
    ECJPAKE_Handle handle_s;
    ECJPAKE_s_CancelOperationMsg cancelMsg;
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

    handle_s = ECJPAKE_s_getHandle(cancelMsg.handle);
    if (handle_s == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    ret = ECJPAKE_cancelOperation(handle_s);

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== ECJPAKE_s_handlePsaMsg ========
 */
psa_status_t ECJPAKE_s_handlePsaMsg(psa_msg_t *msg)
{
    psa_status_t status = PSA_SUCCESS;

    switch (msg->type)
    {
        /* If ECJPAKE_S_MSG_TYPE_CONSTRUCT is used by other secure partitions,
         * any pointer arguments provided must reference memory accessible by
         * this partition which is dependent on the TF-M isolation level in use.
         */
        case ECJPAKE_S_MSG_TYPE_CONSTRUCT:
            status = ECJPAKE_s_construct(msg);
            break;

        case ECJPAKE_S_MSG_TYPE_OPEN:
            status = ECJPAKE_s_open(msg);
            break;

        /* ECJPAKE_S_MSG_TYPE_REGISTER_CALLBACK is designed to be used non-secure
         * callers only. Secure callers must only use polling return behavior.
         */
        case ECJPAKE_S_MSG_TYPE_REGISTER_CALLBACK:
            status = ECJPAKE_s_registerCallback(msg);
            break;

        case ECJPAKE_S_MSG_TYPE_CLOSE:
            status = ECJPAKE_s_close(msg);
            break;

        case ECJPAKE_S_MSG_TYPE_ROUND_ONE_GENERATE_KEYS:
            status = ECJPAKE_s_roundOneGenerateKeys(msg);
            break;

        case ECJPAKE_S_MSG_TYPE_GENERATE_ZKP:
            status = ECJPAKE_s_generateZKP(msg);
            break;

        case ECJPAKE_S_MSG_TYPE_VERIFY_ZKP:
            status = ECJPAKE_s_verifyZKP(msg);
            break;

        case ECJPAKE_S_MSG_TYPE_ROUND_TWO_GENERATE_KEYS:
            status = ECJPAKE_s_roundTwoGenerateKeys(msg);
            break;

        case ECJPAKE_S_MSG_TYPE_COMPUTE_SHARED_SECRET:
            status = ECJPAKE_s_computeSharedSecret(msg);
            break;

        /* ECJPAKE_S_MSG_TYPE_CANCEL_OPERATION supported for non-secure clients
         * only. Secure callers must only use polling return behavior.
         */
        case ECJPAKE_S_MSG_TYPE_CANCEL_OPERATION:
            status = ECJPAKE_s_cancelOperation(msg);
            break;

        default:
            /* Unknown msg type */
            status = PSA_ERROR_PROGRAMMER_ERROR;
            break;
    }

    return status;
}

/*
 *  ======== ECJPAKE_s_init ========
 */
void ECJPAKE_s_init(void)
{
    ECJPAKE_init();
}
