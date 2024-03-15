/******************************************************************************
 Copyright (c) 2022-2023, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include "mbedtls/cmac.h"
#include "mbedtls/cipher.h"
#include "mbedtls/error.h"
#include "mbedtls/platform.h"
#include "cmac_alt.h"

#if defined(MBEDTLS_CMAC_ALT)

#include <string.h>

#include <ti/drivers/AESCMAC.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>

static const AESCMAC_HWAttrs defaultAesCmacHwAttrs = {0};
static bool cmacInitialized                        = false;

/*
 * Initialize context
 */
static void cmac_init(mbedtls_cmac_context_t *cmac_ctx)
{
    memset(cmac_ctx, 0, sizeof(mbedtls_cmac_context_t));

    if (cmacInitialized == false)
    {
        AESCMAC_init();
        cmacInitialized = true;
    }

    AESCMAC_Params params;
    AESCMAC_Params_init(&params);
    params.returnBehavior  = AESCMAC_RETURN_BEHAVIOR_POLLING;
    params.operationalMode = AESCMAC_OPMODE_CMAC;

    cmac_ctx->cmacConfig.object  = &cmac_ctx->cmacObject;
    cmac_ctx->cmacConfig.hwAttrs = &defaultAesCmacHwAttrs;

    cmac_ctx->handle = AESCMAC_construct(&cmac_ctx->cmacConfig, &params);
}

/* Initialize key */
static int cmac_setkey(mbedtls_cmac_context_t *cmac_ctx, const unsigned char *key, size_t keybits)
{
    /* Initialize AES key */
    memcpy(cmac_ctx->keyMaterial, key, (keybits >> 3));

    /* CryptoKeyPlaintext_initKey() alwaysb return success */
    (void)CryptoKeyPlaintext_initKey(&cmac_ctx->cryptoKey, (uint8_t *)cmac_ctx->keyMaterial, (keybits >> 3));

    return 0;
}

/* Close driver instance and clear context */
static void cmac_close(mbedtls_cmac_context_t *cmac_ctx)
{
    if (cmac_ctx->handle != NULL)
    {
        AESCMAC_close(cmac_ctx->handle);
    }

    memset(cmac_ctx, 0, sizeof(mbedtls_cmac_context_t));
}

int mbedtls_cipher_cmac_starts(mbedtls_cipher_context_t *ctx, const unsigned char *key, size_t keybits)
{
    mbedtls_cmac_context_t *cmac_ctx;

    if (mbedtls_cipher_get_type(ctx) == MBEDTLS_CIPHER_DES_EDE3_ECB)
    {
        return MBEDTLS_ERR_PLATFORM_FEATURE_UNSUPPORTED;
    }

    /* Allocate and initialise the CMAC context */
    cmac_ctx = mbedtls_calloc(1, sizeof(mbedtls_cmac_context_t));
    if (cmac_ctx == NULL)
    {
        return (MBEDTLS_ERR_CIPHER_ALLOC_FAILED);
    }

    /* Initialize and construct driver instance */
    cmac_init(cmac_ctx);

    /* Initialize key */
    if (cmac_setkey(cmac_ctx, key, keybits) != CryptoKey_STATUS_SUCCESS)
    {
        return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    if (cmac_ctx->handle == NULL)
    {
        return MBEDTLS_ERR_CIPHER_INVALID_CONTEXT;
    }

    if (AESCMAC_setupSign(cmac_ctx->handle, &cmac_ctx->cryptoKey) != AESCMAC_STATUS_SUCCESS)
    {
        return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    ctx->cmac_ctx = cmac_ctx;

    return 0;
}

int mbedtls_cipher_cmac_update(mbedtls_cipher_context_t *ctx, const unsigned char *input, size_t ilen)
{
    mbedtls_cmac_context_t *cmac_ctx;
    int_fast16_t status;

    cmac_ctx = ctx->cmac_ctx;

    if (cmac_ctx->handle == NULL)
    {
        return MBEDTLS_ERR_CIPHER_INVALID_CONTEXT;
    }

    /* Add input data */
    AESCMAC_Operation_init(&cmac_ctx->operation);
    cmac_ctx->operation.input       = (uint8_t *)input;
    cmac_ctx->operation.inputLength = ilen;
    status                          = AESCMAC_addData(cmac_ctx->handle, &cmac_ctx->operation);

    if (status != AESCMAC_STATUS_SUCCESS)
    {
        status = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    return (int)status;
}

int mbedtls_cipher_cmac_finish(mbedtls_cipher_context_t *ctx, unsigned char *output)
{
    int_fast16_t status;
    mbedtls_cmac_context_t *cmac_ctx;

    cmac_ctx = ctx->cmac_ctx;

    if (cmac_ctx->handle == NULL)
    {
        return MBEDTLS_ERR_CIPHER_INVALID_CONTEXT;
    }

    /* Finalize data */
    cmac_ctx->operation.mac       = (uint8_t *)output;
    cmac_ctx->operation.macLength = MBEDTLS_AES_BLOCK_SIZE;

    status = AESCMAC_finalize(cmac_ctx->handle, &cmac_ctx->operation);

    if (status != AESCMAC_STATUS_SUCCESS)
    {
        status = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    cmac_close(cmac_ctx);

    return (int)status;
}

int mbedtls_cipher_cmac_reset(mbedtls_cipher_context_t *ctx)
{
    mbedtls_cmac_context_t *cmac_ctx;

    cmac_ctx = ctx->cmac_ctx;

    /* Cancel operation */
    if (AESCMAC_cancelOperation(cmac_ctx->handle) != AESCMAC_STATUS_SUCCESS)
    {
        return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    /* Clear current operation */
    memset((void *)&(cmac_ctx->operation), 0, sizeof(AESCMAC_Operation));

    return 0;
}

int mbedtls_cipher_cmac(const mbedtls_cipher_info_t *cipher_info,
                        const unsigned char *key,
                        size_t keylen,
                        const unsigned char *input,
                        size_t ilen,
                        unsigned char *output)
{
    int_fast16_t status;
    mbedtls_cmac_context_t *cmac_ctx;

    if (mbedtls_cipher_info_get_type(cipher_info) == MBEDTLS_CIPHER_DES_EDE3_ECB)
    {
        return MBEDTLS_ERR_PLATFORM_FEATURE_UNSUPPORTED;
    }

    /* Allocate and initialise the CMAC context */
    cmac_ctx = mbedtls_calloc(1, sizeof(mbedtls_cmac_context_t));
    if (cmac_ctx == NULL)
    {
        return (MBEDTLS_ERR_CIPHER_ALLOC_FAILED);
    }

    /* Initialize and construct driver instance */
    cmac_init(cmac_ctx);

    /* Initialize key */
    if (cmac_setkey(cmac_ctx, key, keylen) != CryptoKey_STATUS_SUCCESS)
    {
        status = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
        goto exit;
    }

    if (cmac_ctx->handle == NULL)
    {
        status = MBEDTLS_ERR_CIPHER_INVALID_CONTEXT;
        goto exit;
    }

    AESCMAC_Operation operation;
    AESCMAC_Operation_init(&operation);
    operation.input       = (uint8_t *)input;
    operation.inputLength = ilen;
    operation.mac         = (uint8_t *)output;
    operation.macLength   = MBEDTLS_AES_BLOCK_SIZE;

    status = AESCMAC_oneStepSign(cmac_ctx->handle, &operation, &cmac_ctx->cryptoKey);

    if (status != AESCMAC_STATUS_SUCCESS)
    {
        status = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }

exit:
    /* Clear context */
    cmac_close(cmac_ctx);

    return (int)status;
}

#if defined(MBEDTLS_AES_C)
/*
 * Implementation of AES-CMAC-PRF-128 defined in RFC 4615
 */
int mbedtls_aes_cmac_prf_128(const unsigned char *key,
                             size_t key_length,
                             const unsigned char *input,
                             size_t in_len,
                             unsigned char output[16])
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    const mbedtls_cipher_info_t *cipher_info;
    unsigned char zero_key[MBEDTLS_AES_BLOCK_SIZE];
    unsigned char int_key[MBEDTLS_AES_BLOCK_SIZE];

    if (key == NULL || input == NULL || output == NULL)
        return (MBEDTLS_ERR_CIPHER_BAD_INPUT_DATA);

    cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_ECB);
    if (cipher_info == NULL)
    {
        /* Failing at this point must be due to a build issue */
        ret = MBEDTLS_ERR_CIPHER_FEATURE_UNAVAILABLE;
        goto exit;
    }

    if (key_length == MBEDTLS_AES_BLOCK_SIZE)
    {
        /* Use key as is */
        memcpy(int_key, key, MBEDTLS_AES_BLOCK_SIZE);
    }
    else
    {
        memset(zero_key, 0, MBEDTLS_AES_BLOCK_SIZE);

        ret = mbedtls_cipher_cmac(cipher_info, zero_key, 128, key, key_length, int_key);
        if (ret != 0)
            goto exit;
    }

    ret = mbedtls_cipher_cmac(cipher_info, int_key, 128, input, in_len, output);

exit:
    mbedtls_platform_zeroize(int_key, sizeof(int_key));

    return (ret);
}

#endif /* MBEDTLS_AES_C */

#endif /* MBEDTLS_CMAC_ALT */
