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

#include "mbedtls/ccm.h"
#include "mbedtls/error.h"
#include "ccm_alt.h"

#if defined(MBEDTLS_CCM_ALT)

#include <string.h>

#include <ti/drivers/AESCCM.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>

static const AESCCM_HWAttrs defaultAesCcmHwAttrs = {0};
static bool ccmInitialized                       = false;

/*
 * Initialize context
 */
void mbedtls_ccm_init(mbedtls_ccm_context *ctx)
{
    memset(ctx, 0, sizeof(mbedtls_ccm_context));

    if (ccmInitialized == false)
    {
        AESCCM_init();
        ccmInitialized = true;
    }

    AESCCM_Params params;
    AESCCM_Params_init(&params);
    params.returnBehavior = AESCCM_RETURN_BEHAVIOR_POLLING;

    ctx->ccmConfig.object  = &ctx->ccmObject;
    ctx->ccmConfig.hwAttrs = &defaultAesCcmHwAttrs;

    ctx->handle = AESCCM_construct(&ctx->ccmConfig, &params);
}

/*
 * Set key
 */
int mbedtls_ccm_setkey(mbedtls_ccm_context *ctx,
                       mbedtls_cipher_id_t cipher,
                       const unsigned char *key,
                       unsigned int keybits)
{
    if (cipher != MBEDTLS_CIPHER_ID_AES)
    {
        return MBEDTLS_ERR_PLATFORM_FEATURE_UNSUPPORTED;
    }

    /* Initialize AES key */
    memcpy(ctx->keyMaterial, key, (keybits >> 3));
    (void)CryptoKeyPlaintext_initKey(&ctx->cryptoKey, (uint8_t *)ctx->keyMaterial, (keybits >> 3));

    return 0;
}

/*
 * Free context
 */
void mbedtls_ccm_free(mbedtls_ccm_context *ctx)
{
    if (ctx->handle != NULL)
    {
        AESCCM_close(ctx->handle);
    }

    memset(ctx, 0, sizeof(mbedtls_ccm_context));
}

int mbedtls_ccm_starts(mbedtls_ccm_context *ctx, int mode, const unsigned char *iv, size_t iv_len)
{
    int_fast16_t status;
    ctx->mode = mode;

    if (ctx->handle == NULL)
    {
        return MBEDTLS_ERR_CIPHER_INVALID_CONTEXT;
    }

    if (mode == MBEDTLS_CCM_ENCRYPT || mode == MBEDTLS_CCM_STAR_ENCRYPT)
    {
        status = AESCCM_setupEncrypt(ctx->handle, &ctx->cryptoKey, 0, 0, 0);
    }
    else if (mode == MBEDTLS_CCM_DECRYPT || mode == MBEDTLS_CCM_STAR_DECRYPT)
    {
        status = AESCCM_setupDecrypt(ctx->handle, &ctx->cryptoKey, 0, 0, 0);
    }

    if (status != AESCCM_STATUS_SUCCESS)
    {
        return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    status = AESCCM_setNonce(ctx->handle, iv, iv_len);
    
    if (status != AESCCM_STATUS_SUCCESS)
    {
        status = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }
    return (int)status;
}

int mbedtls_ccm_set_lengths(mbedtls_ccm_context *ctx, size_t total_ad_len, size_t plaintext_len, size_t tag_len)
{
    int_fast16_t status;

    status = AESCCM_setLengths(ctx->handle, total_ad_len, plaintext_len, tag_len);

    if (status != AESCCM_STATUS_SUCCESS)
    {
        status = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }
    return (int)status;
}

int mbedtls_ccm_update_ad(mbedtls_ccm_context *ctx, const unsigned char *ad, size_t ad_len)
{
    int_fast16_t status;
    AESCCM_SegmentedAADOperation segmentedAADOperation;
    AESCCM_SegmentedAADOperation_init(&segmentedAADOperation);
    segmentedAADOperation.aad       = (uint8_t *)ad;
    segmentedAADOperation.aadLength = ad_len;

    status = AESCCM_addAAD(ctx->handle, &segmentedAADOperation);
    
    if (status != AESCCM_STATUS_SUCCESS)
    {
        status = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    return (int)status;
}

int mbedtls_ccm_update(mbedtls_ccm_context *ctx,
                       const unsigned char *input,
                       size_t input_len,
                       unsigned char *output,
                       size_t output_size,
                       size_t *output_len)
{
    int_fast16_t status;
    AESCCM_SegmentedDataOperation segmentedDataOperation;
    AESCCM_SegmentedDataOperation_init(&segmentedDataOperation);
    segmentedDataOperation.input       = (uint8_t *)input;
    segmentedDataOperation.output      = (uint8_t *)output;
    segmentedDataOperation.inputLength = input_len;

    status = AESCCM_addData(ctx->handle, &segmentedDataOperation);

    return (int)status;
}

int mbedtls_ccm_finish(mbedtls_ccm_context *ctx, unsigned char *tag, size_t tag_len)
{
    int_fast16_t status;
    AESCCM_SegmentedFinalizeOperation segmentedFinalizeOperation;

    AESCCM_SegmentedFinalizeOperation_init(&segmentedFinalizeOperation);
    segmentedFinalizeOperation.inputLength = 0;
    segmentedFinalizeOperation.mac         = tag;
    segmentedFinalizeOperation.macLength   = tag_len;

    if (ctx->mode == MBEDTLS_CCM_ENCRYPT || ctx->mode == MBEDTLS_CCM_STAR_ENCRYPT)
    {
        status = AESCCM_finalizeEncrypt(ctx->handle, &segmentedFinalizeOperation);
        if (status != AESCCM_STATUS_SUCCESS)
        {
            status = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
        }
    }
    else if (ctx->mode == MBEDTLS_CCM_DECRYPT || ctx->mode == MBEDTLS_CCM_STAR_DECRYPT)
    {
        status = AESCCM_finalizeDecrypt(ctx->handle, &segmentedFinalizeOperation);
        if (status != AESCCM_STATUS_SUCCESS)
        {
            status = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
        }
    }
    else
    {
        /* Incorrect input parameter for ctx->mode */
        status = MBEDTLS_ERR_CIPHER_BAD_INPUT_DATA;
    }

    mbedtls_ccm_free(ctx);

    return (int)status;
}

/*
 * Authenticated encryption or decryption
 */
static int ccm_auth_crypt(mbedtls_ccm_context *ctx,
                          int mode,
                          size_t length,
                          const unsigned char *iv,
                          size_t iv_len,
                          const unsigned char *add,
                          size_t add_len,
                          const unsigned char *input,
                          unsigned char *output,
                          unsigned char *tag,
                          size_t tag_len)
{
    int_fast16_t status;
    AESCCM_OneStepOperation operation;

    if (ctx->handle == NULL)
    {
        return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    ctx->mode = mode;

    AESCCM_OneStepOperation_init(&operation);
    operation.key         = &ctx->cryptoKey;
    operation.aad         = (uint8_t *)add;
    operation.aadLength   = add_len;
    operation.input       = (uint8_t *)input;
    operation.output      = (uint8_t *)output;
    operation.inputLength = length;
    operation.nonce       = (uint8_t *)iv;
    operation.nonceLength = iv_len;
    operation.mac         = (uint8_t *)tag;
    operation.macLength   = tag_len;

    if (ctx->mode == MBEDTLS_CCM_ENCRYPT || ctx->mode == MBEDTLS_CCM_STAR_ENCRYPT)
    {
        status = AESCCM_oneStepEncrypt(ctx->handle, &operation);
        if (status != AESCCM_STATUS_SUCCESS)
        {
            status = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
        }
    }
    else if (ctx->mode == MBEDTLS_CCM_DECRYPT || ctx->mode == MBEDTLS_CCM_STAR_DECRYPT)
    {
        status = AESCCM_oneStepDecrypt(ctx->handle, &operation);
        if (status != AESCCM_STATUS_SUCCESS)
        {
            status = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
        }
    }
    else
    {
        /* Invalid  input parameter for ctx->mode */
        status = MBEDTLS_ERR_CIPHER_BAD_INPUT_DATA;
    }

    return (int)status;
}

/*
 * Authenticated encryption
 */
int mbedtls_ccm_star_encrypt_and_tag(mbedtls_ccm_context *ctx,
                                     size_t length,
                                     const unsigned char *iv,
                                     size_t iv_len,
                                     const unsigned char *add,
                                     size_t add_len,
                                     const unsigned char *input,
                                     unsigned char *output,
                                     unsigned char *tag,
                                     size_t tag_len)
{
    return (
        ccm_auth_crypt(ctx, MBEDTLS_CCM_STAR_ENCRYPT, length, iv, iv_len, add, add_len, input, output, tag, tag_len));
}

int mbedtls_ccm_encrypt_and_tag(mbedtls_ccm_context *ctx,
                                size_t length,
                                const unsigned char *iv,
                                size_t iv_len,
                                const unsigned char *add,
                                size_t add_len,
                                const unsigned char *input,
                                unsigned char *output,
                                unsigned char *tag,
                                size_t tag_len)
{
    return (ccm_auth_crypt(ctx, MBEDTLS_CCM_ENCRYPT, length, iv, iv_len, add, add_len, input, output, tag, tag_len));
}

int mbedtls_ccm_star_auth_decrypt(mbedtls_ccm_context *ctx,
                                  size_t length,
                                  const unsigned char *iv,
                                  size_t iv_len,
                                  const unsigned char *add,
                                  size_t add_len,
                                  const unsigned char *input,
                                  unsigned char *output,
                                  const unsigned char *tag,
                                  size_t tag_len)
{
    return ccm_auth_crypt(ctx, MBEDTLS_CCM_STAR_DECRYPT, length, iv, iv_len, add, add_len, input, output, (unsigned char *)tag, tag_len);
}

int mbedtls_ccm_auth_decrypt(mbedtls_ccm_context *ctx,
                             size_t length,
                             const unsigned char *iv,
                             size_t iv_len,
                             const unsigned char *add,
                             size_t add_len,
                             const unsigned char *input,
                             unsigned char *output,
                             const unsigned char *tag,
                             size_t tag_len)
{
    return ccm_auth_crypt(ctx, MBEDTLS_CCM_DECRYPT, length, iv, iv_len, add, add_len, input, output, (unsigned char *)tag, tag_len);
}

#endif /* MBEDTLS_CCM_ALT */
