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

 ******************************************************************************/

#include <mbedtls/error.h>
#include <mbedtls/sha256.h>
#include "sha256_alt.h"

#if defined(MBEDTLS_SHA256_ALT)

#include <string.h>
#include <assert.h>

#include <ti/drivers/SHA2.h>

/*!
 *  @brief Hardware-specific configuration attributes
 *
 *  SimpleLink hardware attributes are used by the SHA2_Config struct.
 */
static const SHA2_HWAttrs defaultSha2HwAttrs = {0};

static bool sha256Initialized = false;

/**
 * \brief          Initialize SHA-256 context
 *
 * \param ctx      SHA-256 context to be initialized
 */
void mbedtls_sha256_init(mbedtls_sha256_context *ctx)
{
    memset(ctx, 0, sizeof(mbedtls_sha256_context));

    if (sha256Initialized == false)
    {
        SHA2_init();
        sha256Initialized = true;
    }
}

/**
 * \brief          Clear SHA-256 context
 *
 * \param ctx      SHA-256 context to be cleared
 */
void mbedtls_sha256_free(mbedtls_sha256_context *ctx)
{
    if (ctx->handle != NULL)
    {
        SHA2_close(ctx->handle);
    }

    memset(ctx, 0, sizeof(mbedtls_sha256_context));
}

/**
 * \brief          SHA-256 context setup
 *
 * \param ctx      context to be initialized
 * \param is224    0 = use SHA256, 1 = use SHA224
 *
 * \retval         0                                  on success
 * \retval         MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED on failure to open driver
 */
int mbedtls_sha256_starts(mbedtls_sha256_context *ctx, int is224)
{
    SHA2_HashType type;

    SHA2_Params sha2Params;

    if (ctx->handle == NULL)
    {
        SHA2_Params_init(&sha2Params);

        sha2Params.returnBehavior = SHA2_RETURN_BEHAVIOR_POLLING;

        ctx->config.object  = &ctx->object;
        ctx->config.hwAttrs = &defaultSha2HwAttrs;

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

    if (is224 == 1)
    {
        type = SHA2_HASH_TYPE_224;
    }
    else
    {
        type = SHA2_HASH_TYPE_256;
    }

    SHA2_setHashType(ctx->handle, type);

    return 0;
}

/**
 * \brief          Clone (the state of) a SHA-256 context
 *
 * \param dst      The destination context
 * \param src      The context to be cloned
 */
void mbedtls_sha256_clone(mbedtls_sha256_context *dst, const mbedtls_sha256_context *src)
{
    mbedtls_sha256_context *ctx = dst;
    SHA2_Params sha2Params;

    SHA2_Params_init(&sha2Params);
    sha2Params.returnBehavior = SHA2_RETURN_BEHAVIOR_POLLING;

    ctx->config.object  = &ctx->object;
    ctx->config.hwAttrs = &defaultSha2HwAttrs;

    ctx->handle = SHA2_construct(&ctx->config, &sha2Params);
    /* clone */
    ctx->object = src->object;
}

/**
 * \brief          SHA-256 final digest
 *
 * \param ctx      SHA-256 context
 * \param output   SHA-224/256 checksum result
 *
 * \retval         0                                  on success
 * \retval         MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED on driver failure
 */
int mbedtls_sha256_finish(mbedtls_sha256_context *ctx, unsigned char *output)
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
 * \brief          SHA-256 process buffer
 *
 * \param ctx      SHA-256 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 *
 * \retval         0                                  on success
 * \retval         MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED on driver failure
 */
int mbedtls_sha256_update(mbedtls_sha256_context *ctx, const unsigned char *input, size_t ilen)
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
 * \brief          SHA-256 start/intermediate blocks
 *
 * \param ctx      SHA-256 context
 * \param data     64-byte input data block
 *
 * \retval         0                                  on success
 * \retval         MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED on driver failure
 */
int mbedtls_internal_sha256_process(mbedtls_sha256_context *ctx, const unsigned char data[64])
{
    int_fast16_t result;

    /* Process data in chunks. The driver buffers incomplete blocks internally. */
    result = SHA2_addData(ctx->handle, data, SHA2_BLOCK_SIZE_BYTES_256);

    if (result != SHA2_STATUS_SUCCESS)
    {
        return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }
    else
    {
        return 0;
    }
}

#endif /* MBEDTLS_SHA256_ALT */
