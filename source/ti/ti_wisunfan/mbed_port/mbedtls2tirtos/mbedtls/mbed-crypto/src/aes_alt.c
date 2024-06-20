/******************************************************************************

 @file aes_alt.c

 @brief AES implientation for TI chip

 Group: CMCU, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2017-2019, Texas Instruments Incorporated
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

 ******************************************************************************
 
 
 *****************************************************************************/

#include "mbedtls/aes.h"
#include "aes_alt.h"

#if defined(MBEDTLS_AES_ALT)

#include <string.h>
#include <assert.h>

#include <ti/devices/DeviceFamily.h>
#include <ti/drivers/AESECB.h>
#include <ti/drivers/AESCBC.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>

#include "mbedtls/platform_util.h"


/* Parameter validation macros based on platform_util.h */
#define AES_VALIDATE_RET( cond )    \
    MBEDTLS_INTERNAL_VALIDATE_RET( cond, MBEDTLS_ERR_AES_BAD_INPUT_DATA )
#define AES_VALIDATE( cond )        \
    MBEDTLS_INTERNAL_VALIDATE( cond )

/**
 * number of active contexts, used for power on/off of the crypto core
 */
static unsigned int ref_num = 0;

static AESECB_Handle AESECB_handle = NULL;

static AESCBC_Handle AESCBC_handle = NULL;


/**
 * @brief Initialize AES context
 *
 * @param [in,out] ctx AES context to be initialized
 */
void mbedtls_aes_init(mbedtls_aes_context *ctx)
{
    (void)(ctx);

    AESECB_Params AESECBParams;
    AESCBC_Params AESCBCParams;


    if (ref_num++ == 0)
    {
        AESECB_Params_init(&AESECBParams);
        AESECBParams.returnBehavior = AESECB_RETURN_BEHAVIOR_POLLING;
        AESECB_handle = AESECB_open(0 , &AESECBParams);
        //assert(AESECB_handle != 0);

        AESCBC_Params_init(&AESCBCParams);
        AESCBCParams.returnBehavior = AESCBC_RETURN_BEHAVIOR_POLLING;
        AESCBC_handle = AESCBC_open(0 , &AESCBCParams);
    }
}

/**
 * @brief          Clear AES context
 *
 * \param ctx      AES context to be cleared
 */
void mbedtls_aes_free(mbedtls_aes_context *ctx)
{
    if (--ref_num == 0)
    {
        AESECB_close(AESECB_handle);
        AESECB_handle = NULL;

        AESCBC_close(AESCBC_handle);
        AESCBC_handle = NULL;
    }

    memset((void *)ctx, 0x00, sizeof(ctx));
}

/**
 * \brief          AES key schedule (encryption)
 *
 * \param ctx      AES context to be initialized
 * \param key      encryption key
 * \param keybits  must be 128, 192 or 256
 *
 * \return         0 if successful, or MBEDTLS_ERR_AES_INVALID_KEY_LENGTH
 */
int mbedtls_aes_setkey_enc(mbedtls_aes_context *ctx, const unsigned char *key, unsigned int keybits)
{
    int_fast16_t statusCrypto = 0;

    /* Initialize AES key */
    memcpy(ctx->keyMaterial, key, (keybits >> 3));
    statusCrypto = CryptoKeyPlaintext_initKey(&ctx->cryptoKey, (uint8_t*) ctx->keyMaterial, (keybits >> 3));
    //assert(statusCrypto == 0);

    return (int)statusCrypto;
}

/**
 * \brief          AES key schedule (decryption)
 *
 * \param ctx      AES context to be initialized
 * \param key      decryption key
 * \param keybits  must be 128, 192 or 256
 *
 * \return         0 if successful, or MBEDTLS_ERR_AES_INVALID_KEY_LENGTH
 */
int mbedtls_aes_setkey_dec(mbedtls_aes_context *ctx, const unsigned char *key, unsigned int keybits)
{
    int_fast16_t statusCrypto;

    /* Initialize AES key */
    statusCrypto = CryptoKeyPlaintext_initKey(&ctx->cryptoKey, (uint8_t*) key, (keybits >> 3));
    //assert(statusCrypto == 0);

    return (int)statusCrypto;
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
    int statusCrypto;
    AESECB_Operation operationOneStepXcrypt;

    /* run it through the authentication + encryption, pass the ccmLVal = 2 */
    AESECB_Operation_init(&operationOneStepXcrypt);

    operationOneStepXcrypt.key = &ctx->cryptoKey;
    operationOneStepXcrypt.inputLength = 16;
    operationOneStepXcrypt.input = (uint8_t *)input;
    operationOneStepXcrypt.output = (uint8_t *)output;

    if(mode == MBEDTLS_AES_ENCRYPT)
    {
        statusCrypto = AESECB_oneStepEncrypt(AESECB_handle, &operationOneStepXcrypt);
    }
    else
    {
        statusCrypto = AESECB_oneStepDecrypt(AESECB_handle, &operationOneStepXcrypt);
    }


    //assert(statusCrypto == 0);

    return statusCrypto;
}

int mbedtls_aes_crypt_cbc( mbedtls_aes_context *ctx,
                    int mode,
                    size_t length,
                    unsigned char iv[16],
                    const unsigned char *input,
                    unsigned char *output )
{
    int statusCrypto;
    AESCBC_Operation operationOneStepXcrypt;

    AES_VALIDATE_RET( ctx != NULL );
    AES_VALIDATE_RET( mode == MBEDTLS_AES_ENCRYPT ||
                      mode == MBEDTLS_AES_DECRYPT );
    AES_VALIDATE_RET( iv != NULL );
    AES_VALIDATE_RET( input != NULL );
    AES_VALIDATE_RET( output != NULL );

    if( length % 16 )
        return( MBEDTLS_ERR_AES_INVALID_INPUT_LENGTH );

    AESCBC_Operation_init(&operationOneStepXcrypt);

    operationOneStepXcrypt.key               = &ctx->cryptoKey;
    operationOneStepXcrypt.input             =(uint8_t *)input;
    operationOneStepXcrypt.output            =(uint8_t *)output;
    operationOneStepXcrypt.inputLength       = length;
    operationOneStepXcrypt.iv                = (uint8_t *)iv;

    if(mode == MBEDTLS_AES_ENCRYPT)
    {
        statusCrypto = AESCBC_oneStepEncrypt(AESCBC_handle, &operationOneStepXcrypt);
    }
    else
    {
        statusCrypto = AESCBC_oneStepDecrypt(AESCBC_handle, &operationOneStepXcrypt);
    }

    return (statusCrypto);
}

#endif //MBEDTLS_AES_ALT

