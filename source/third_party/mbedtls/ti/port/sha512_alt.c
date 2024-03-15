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

 ******************************************************************************/

#include <mbedtls/error.h>
#include <mbedtls/sha512.h>
#include "sha512_alt.h"

#if defined(MBEDTLS_SHA512_ALT)

#include <string.h>
#include <assert.h>

#include <ti/drivers/SHA2.h>

/*!
 *  @brief Hardware-specific configuration attributes
 *
 *  SimpleLink hardware attributes are used by the SHA2_Config struct.
 */
static const SHA2_HWAttrs defaultSha512HwAttrs = {0};

static bool sha512Initialized = false;

/**
 * \brief          Initialize SHA-512 context
 *
 * \param ctx      SHA-512 context to be initialized
 */
void mbedtls_sha512_init(mbedtls_sha512_context *ctx)
{
    memset(ctx, 0, sizeof(mbedtls_sha512_context));

    if (sha512Initialized == false)
    {
        SHA2_init();
        sha512Initialized = true;
    }
}

/**
 * \brief          Clear SHA-512 context
 *
 * \param ctx      SHA-512 context to be cleared
 */
void mbedtls_sha512_free(mbedtls_sha512_context *ctx)
{
    if (ctx->handle != NULL)
    {
        SHA2_close(ctx->handle);
    }

    memset(ctx, 0, sizeof(mbedtls_sha512_context));
}

/**
 * \brief          SHA-512 context setup
 *
 * \param ctx      context to be initialized
 * \param is224    0 = use SHA512, 1 = use SHA224
 *
 * \retval         0 on success
 * \retval         MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED on failure to open driver
 */
int mbedtls_sha512_starts(mbedtls_sha512_context *ctx, int is384)
{
    SHA2_HashType type;
    SHA2_Params sha2Params;

    if (ctx->handle == NULL)
    {
        SHA2_Params_init(&sha2Params);

        sha2Params.returnBehavior = SHA2_RETURN_BEHAVIOR_POLLING;

        ctx->config.object  = &ctx->object;
        ctx->config.hwAttrs = &defaultSha512HwAttrs;

        ctx->handle = SHA2_construct(&ctx->config, &sha2Params);

        if (NULL == ctx->handle)
        {
            return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
        }
    }
    else
    {
        SHA2_reset(ctx->handle);
    }

    if (is384)
    {
        type = SHA2_HASH_TYPE_384;
    }
    else
    {
        type = SHA2_HASH_TYPE_512;
    }

    SHA2_setHashType(ctx->handle, type);

    return 0;
}

/**
 * \brief          Clone (the state of) a SHA-512 context
 *
 * \param dst      The destination context
 * \param src      The context to be cloned
 */
void mbedtls_sha512_clone(mbedtls_sha512_context *dst, const mbedtls_sha512_context *src)
{
    mbedtls_sha512_context *ctx = dst;
    SHA2_Params sha2Params;

    SHA2_Params_init(&sha2Params);
    sha2Params.returnBehavior = SHA2_RETURN_BEHAVIOR_POLLING;

    ctx->config.object  = &ctx->object;
    ctx->config.hwAttrs = &defaultSha512HwAttrs;

    ctx->handle = SHA2_construct(&ctx->config, &sha2Params);
    /* clone */
    ctx->object = src->object;
}

/**
 * \brief          SHA-512 final digest
 *
 * \param ctx      SHA-512 context
 * \param output   SHA-224/512 checksum result
 *
 * \retval         0 on success
 * \retval         MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED on driver failure
 */
int mbedtls_sha512_finish(mbedtls_sha512_context *ctx, unsigned char *output)
{
    int_fast16_t result;

    result = SHA2_finalize(ctx->handle, output);

    if (SHA2_STATUS_SUCCESS != result)
    {
        return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }
    else
    {
        return 0;
    }
}

/**
 * \brief          SHA-512 process buffer
 *
 * \param ctx      SHA-512 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 *
 * \retval         0 on success
 * \retval         MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED on driver failure
 */
int mbedtls_sha512_update(mbedtls_sha512_context *ctx, const unsigned char *input, size_t ilen)
{
    int_fast16_t result;

    /* Process data in chunks. The driver buffers incomplete blocks internally. */
    result = SHA2_addData(ctx->handle, input, ilen);

    if (SHA2_STATUS_SUCCESS != result)
    {
        return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }
    else
    {
        return 0;
    }
}

/**
 * \brief          SHA-512 start/intermediate blocks
 *
 * \param ctx      SHA-512 context
 * \param data     128-byte input data block
 *
 * \retval         0 on success
 * \retval         MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED on driver failure
 */
int mbedtls_internal_sha512_process(mbedtls_sha512_context *ctx, const unsigned char data[128])
{
    int_fast16_t result;

    /* Process data in chunks. The driver buffers incomplete blocks internally. */
    result = SHA2_addData(ctx->handle, data, SHA2_BLOCK_SIZE_BYTES_512);

    if (SHA2_STATUS_SUCCESS != result)
    {
        return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }
    else
    {
        return 0;
    }
}

#endif /* MBEDTLS_SHA512_ALT */
