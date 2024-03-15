/******************************************************************************
 Copyright (c) 2019-2023, Texas Instruments Incorporated
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

#include "mbedtls/aes.h"
#include "mbedtls/cipher.h"
#include "mbedtls/error.h"
#include "aes_alt.h"

#if defined(MBEDTLS_AES_ALT)

#include <string.h>
#include <assert.h>

#include <ti/drivers/AESECB.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>

static const AESECB_HWAttrs defaultAesEcbHwAttrs = {0};

#if defined(MBEDTLS_CIPHER_MODE_CBC)
#include <ti/drivers/AESCBC.h>
static const AESCBC_HWAttrs defaultAesCbcHwAttrs = {0};
#endif /* MBEDTLS_CIPHER_MODE_CBC */

#if defined(MBEDTLS_CIPHER_MODE_CTR)
#include <ti/drivers/AESCTR.h>
static const AESCTR_HWAttrs defaultAesCtrHwAttrs = {0};
#endif /* MBEDTLS_CIPHER_MODE_CTR */

bool aesInitialized = false;

/**
 * @brief Initialize AES context
 *
 * @param [in,out] ctx AES context to be initialized
 */
void mbedtls_aes_init(mbedtls_aes_context *ctx)
{
    memset(ctx, 0, sizeof(mbedtls_aes_context));

    if (aesInitialized == false)
    {
        AESECB_init();
    #if defined(MBEDTLS_CIPHER_MODE_CBC)
        AESCBC_init();
    #endif
    #if defined(MBEDTLS_CIPHER_MODE_CTR)
        AESCTR_init();
    #endif

        aesInitialized = true;
    }

    AESECB_Params ecbParams;
    AESECB_Params_init(&ecbParams);
    ecbParams.returnBehavior = AESECB_RETURN_BEHAVIOR_POLLING;

    ctx->ecbConfig.object  = &ctx->ecbObject;
    ctx->ecbConfig.hwAttrs = &defaultAesEcbHwAttrs;

    ctx->ecbHandle = AESECB_construct(&ctx->ecbConfig, &ecbParams);

    #if defined(MBEDTLS_CIPHER_MODE_CBC)
    AESCBC_Params cbcParams;
    AESCBC_Params_init(&cbcParams);
    cbcParams.returnBehavior = AESCBC_RETURN_BEHAVIOR_POLLING;

    ctx->cbcConfig.object  = &ctx->cbcObject;
    ctx->cbcConfig.hwAttrs = &defaultAesCbcHwAttrs;

    ctx->cbcHandle = AESCBC_construct(&ctx->cbcConfig, &cbcParams);
    #endif     /* MBEDTLS_CIPHER_MODE_CBC */

    #if defined(MBEDTLS_CIPHER_MODE_CTR)
    AESCTR_Params ctrParams;
    AESCTR_Params_init(&ctrParams);
    ctrParams.returnBehavior = AESCTR_RETURN_BEHAVIOR_POLLING;

    ctx->ctrConfig.object  = &ctx->ctrObject;
    ctx->ctrConfig.hwAttrs = &defaultAesCtrHwAttrs;

    ctx->ctrHandle = AESCTR_construct(&ctx->ctrConfig, &ctrParams);
    #endif     /* MBEDTLS_CIPHER_MODE_CTR */    
}

/**
 * @brief          Clear AES context
 *
 * \param ctx      AES context to be cleared
 */
void mbedtls_aes_free(mbedtls_aes_context *ctx)
{
    if (ctx->ecbHandle != NULL)
    {
        AESECB_close(ctx->ecbHandle);
    }
#if defined(MBEDTLS_CIPHER_MODE_CBC)
    else if (ctx->ecbHandle != NULL)
    {
        AESCBC_close(ctx->cbcHandle);
    }
#endif
#if defined(MBEDTLS_CIPHER_MODE_CTR)
    else if (ctx->ecbHandle != NULL)
    {
        AESCTR_close(ctx->ctrHandle);
    }
#endif    
    memset(ctx, 0, sizeof(mbedtls_aes_context));
}

/**
 * \brief          AES key initialize and set
 *
 */
static int mbedtls_aes_setkey(mbedtls_aes_context *ctx, const unsigned char *key, unsigned int keybits)
{
    /* Initialize AES key */
    memcpy(ctx->keyMaterial, key, (keybits >> 3));

    /* CryptoKeyPlaintext_initKey always returns success */
    (void)CryptoKeyPlaintext_initKey(&ctx->cryptoKey, (uint8_t *)ctx->keyMaterial, (keybits >> 3));

    return 0;
}

/**
 * \brief          AES key schedule (encryption)
 *
 * \param ctx      AES context to be initialized
 * \param key      encryption key
 * \param keybits  must be 128 or 256
 *
 * \return         0 if successful, or MBEDTLS_ERR_AES_INVALID_KEY_LENGTH
 */
int mbedtls_aes_setkey_enc(mbedtls_aes_context *ctx, const unsigned char *key, unsigned int keybits)
{
    return mbedtls_aes_setkey(ctx, key, keybits);
}

/**
 * \brief          AES key schedule (decryption)
 *
 * \param ctx      AES context to be initialized
 * \param key      decryption key
 * \param keybits  must be 128 or 256
 *
 * \return         0 if successful, or MBEDTLS_ERR_AES_INVALID_KEY_LENGTH
 */
int mbedtls_aes_setkey_dec(mbedtls_aes_context *ctx, const unsigned char *key, unsigned int keybits)
{
    return mbedtls_aes_setkey(ctx, key, keybits);
}

/**
 * \brief          AES-ECB block encryption/decryption
 *
 * \param ctx      AES context
 * \param mode     MBEDTLS_AES_ENCRYPT or MBEDTLS_AES_DECRYPT
 * \param input    16-byte input block
 * \param output   16-byte output block
 *
 * \return         0 if successful
 */
int mbedtls_aes_crypt_ecb(mbedtls_aes_context *ctx, int mode, const unsigned char input[16], unsigned char output[16])
{
    int_fast16_t statusCrypto;
    AESECB_Operation operationOneStep;

    if (ctx->ecbHandle == NULL)
    {
        return MBEDTLS_ERR_AES_BAD_INPUT_DATA;
    }

    AESECB_Operation_init(&operationOneStep);

    operationOneStep.key         = &ctx->cryptoKey;
    operationOneStep.inputLength = 16;
    operationOneStep.input       = (uint8_t *)input;
    operationOneStep.output      = (uint8_t *)output;

    if (mode == MBEDTLS_AES_DECRYPT)
    {
        statusCrypto = AESECB_oneStepDecrypt(ctx->ecbHandle, &operationOneStep);
    }
    else
    {
        statusCrypto = AESECB_oneStepEncrypt(ctx->ecbHandle, &operationOneStep);
    }

    if (statusCrypto != AESECB_STATUS_SUCCESS)
    {
        statusCrypto = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    return (int)statusCrypto;
}

#if defined(MBEDTLS_CIPHER_MODE_CBC)
/*
 * AES-CBC buffer encryption/decryption
 */
int mbedtls_aes_crypt_cbc(mbedtls_aes_context *ctx,
                          int mode,
                          size_t length,
                          unsigned char iv[16],
                          const unsigned char *input,
                          unsigned char *output)
{
    int_fast16_t statusCrypto;
    AESCBC_OneStepOperation operationOneStep;
    unsigned char tmp_input[64];

    memcpy(tmp_input, input, 16);

    if (ctx->cbcHandle == NULL)
    {
        return MBEDTLS_ERR_CIPHER_INVALID_CONTEXT;
    }

    AESCBC_OneStepOperation_init(&operationOneStep);

    operationOneStep.key         = &ctx->cryptoKey;
    operationOneStep.inputLength = length;
    operationOneStep.input       = (uint8_t *)tmp_input;
    operationOneStep.output      = (uint8_t *)output;
    operationOneStep.iv          = (uint8_t *)iv;

    if (mode == MBEDTLS_AES_DECRYPT)
    {
        statusCrypto = AESCBC_oneStepDecrypt(ctx->cbcHandle, &operationOneStep);
        /* Get next IV for dec, next IV = prev CT input */
        memcpy(iv, tmp_input, 16);
    }
    else
    {
        statusCrypto = AESCBC_oneStepEncrypt(ctx->cbcHandle, &operationOneStep);
        /* Get next IV for enc, next IV = current CT output */
        memcpy(iv, output, 16);
    }

    input += 16;
    output += 16;

    if (statusCrypto != AESECB_STATUS_SUCCESS)
    {
        statusCrypto = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    return (int)statusCrypto;
}
#endif /* MBEDTLS_CIPHER_MODE_CBC */

#if defined(MBEDTLS_CIPHER_MODE_CFB)
/*
 * AES-CFB128 buffer encryption/decryption
 */
int mbedtls_aes_crypt_cfb128(mbedtls_aes_context *ctx,
                             int mode,
                             size_t length,
                             size_t *iv_off,
                             unsigned char iv[16],
                             const unsigned char *input,
                             unsigned char *output)
{
    int c;
    size_t n = *iv_off;

    if (mode == MBEDTLS_AES_DECRYPT)
    {
        while (length--)
        {
            if (n == 0)
                mbedtls_aes_crypt_ecb(ctx, MBEDTLS_AES_ENCRYPT, iv, iv);

            c         = *input++;
            *output++ = (unsigned char)(c ^ iv[n]);
            iv[n]     = (unsigned char)c;

            n = (n + 1) & 0x0F;
        }
    }
    else
    {
        while (length--)
        {
            if (n == 0)
                mbedtls_aes_crypt_ecb(ctx, MBEDTLS_AES_ENCRYPT, iv, iv);

            iv[n] = *output++ = (unsigned char)(iv[n] ^ *input++);

            n = (n + 1) & 0x0F;
        }
    }

    *iv_off = n;

    return 0;
}

/*
 * AES-CFB8 buffer encryption/decryption
 */
int mbedtls_aes_crypt_cfb8(mbedtls_aes_context *ctx,
                           int mode,
                           size_t length,
                           unsigned char iv[16],
                           const unsigned char *input,
                           unsigned char *output)
{
    unsigned char c;
    unsigned char ov[17];

    while (length--)
    {
        memcpy(ov, iv, 16);
        mbedtls_aes_crypt_ecb(ctx, MBEDTLS_AES_ENCRYPT, iv, iv);

        if (mode == MBEDTLS_AES_DECRYPT)
            ov[16] = *input;

        c = *output++ = (unsigned char)(iv[0] ^ *input++);

        if (mode == MBEDTLS_AES_ENCRYPT)
            ov[16] = c;

        memcpy(iv, ov + 1, 16);
    }

    return (0);
}
#endif /*MBEDTLS_CIPHER_MODE_CFB */

#if defined(MBEDTLS_CIPHER_MODE_OFB)
/*
 * AES-OFB (Output Feedback Mode) buffer encryption/decryption
 */
int mbedtls_aes_crypt_ofb(mbedtls_aes_context *ctx,
                          size_t length,
                          size_t *iv_off,
                          unsigned char iv[16],
                          const unsigned char *input,
                          unsigned char *output)
{
    int ret = 0;
    size_t n;

    if ((ctx != NULL) || (iv_off != NULL) || (iv != NULL) || (input != NULL) || (output != NULL))
    {
        return (MBEDTLS_ERR_AES_BAD_INPUT_DATA);
    }

    n = *iv_off;

    if (n > 15)
        return (MBEDTLS_ERR_AES_BAD_INPUT_DATA);

    while (length--)
    {
        if (n == 0)
        {
            ret = mbedtls_aes_crypt_ecb(ctx, MBEDTLS_AES_ENCRYPT, iv, iv);
            if (ret != 0)
                goto exit;
        }
        *output++ = *input++ ^ iv[n];

        n = (n + 1) & 0x0F;
    }

    *iv_off = n;

exit:
    return (ret);
}
#endif /* MBEDTLS_CIPHER_MODE_OFB */

#if defined(MBEDTLS_CIPHER_MODE_CTR)
/*
 * AES-CTR buffer encryption/decryption
 */
int mbedtls_aes_crypt_ctr(mbedtls_aes_context *ctx,
                          size_t length,
                          size_t *nc_off,
                          unsigned char nonce_counter[16],
                          unsigned char stream_block[16],
                          const unsigned char *input,
                          unsigned char *output)
{
    int_fast16_t statusCrypto;
    AESCTR_Operation operationOneStep;

    if (ctx->ctrHandle == NULL)
    {
        return MBEDTLS_ERR_AES_BAD_INPUT_DATA;
    }

    AESCTR_Operation_init(&operationOneStep);

    operationOneStep.key            = &ctx->cryptoKey;
    operationOneStep.inputLength    = length;
    operationOneStep.input          = (uint8_t *)input;
    operationOneStep.output         = (uint8_t *)output;
    operationOneStep.initialCounter = nonce_counter;

    statusCrypto = AESCTR_oneStepEncrypt(ctx->ctrHandle, &operationOneStep);

    return (int)statusCrypto;
}
#endif /* MBEDTLS_CIPHER_MODE_CTR */

#endif /* MBEDTLS_AES_ALT */
