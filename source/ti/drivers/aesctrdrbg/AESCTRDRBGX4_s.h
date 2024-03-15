/*
 * Copyright (c) 2022-2023, Texas Instruments Incorporated - http://www.ti.com
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

#ifndef ti_drivers_aesctrdrbg_AESCTRDRBGX4_s__include
#define ti_drivers_aesctrdrbg_AESCTRDRBGX4_s__include

#include <stdint.h>

#include <ti/drivers/crypto/CryptoCC26X4_s.h>
#include <ti/drivers/AESCTRDRBG.h>

#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/interface/include/psa/service.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * AES CTR DRBG secure message types
 *
 * The underlying AES CTR DRBG driver does not support callback return behavior,
 * therefore this secure service can only support polling mode. If the non-secure
 * client specifies blocking mode, polling mode will be automatically substituted.
 */
#define AESCTRDRBG_S_MSG_TYPE_CONSTRUCT        AESCTRDRBG_S_MSG_TYPE(0U)
#define AESCTRDRBG_S_MSG_TYPE_OPEN             AESCTRDRBG_S_MSG_TYPE(1U)
#define AESCTRDRBG_S_MSG_TYPE_CLOSE            AESCTRDRBG_S_MSG_TYPE(2U)
#define AESCTRDRBG_S_MSG_TYPE_RESEED           AESCTRDRBG_S_MSG_TYPE(3U)
#define AESCTRDRBG_S_MSG_TYPE_GET_RANDOM_BYTES AESCTRDRBG_S_MSG_TYPE(4U)
#define AESCTRDRBG_S_MSG_TYPE_GENERATE_KEY     AESCTRDRBG_S_MSG_TYPE(5U)

/*
 * Config pool size determines how many dynamic driver instances can be created
 * by the non-secure client using AESCTRDRBG_construct().
 */
#ifndef CONFIG_AESCTRDRBG_S_CONFIG_POOL_SIZE
    #define CONFIG_AESCTRDRBG_S_CONFIG_POOL_SIZE 1
#endif

/*
 * ========= AES CTR DRBG Secure Message Structs =========
 * These secure message structs correspond to the secure message types defined
 * above. Together, they are used by non-secure client to make PSA calls to the
 * AES CTR DRBG secure service. There is a single input vector for the PSA call
 * which is a pointer to secure message struct. If the underlying function
 * has a return value, there is a single output vector which is a pointer to
 * storage for the return value.
 */
typedef struct
{
    AESCTRDRBG_Config *config;
    const AESCTRDRBG_Params *params;
} AESCTRDRBG_s_ConstructMsg;

typedef struct
{
    uint_least8_t index;
    const AESCTRDRBG_Params *params;
} AESCTRDRBG_s_OpenMsg;

typedef struct
{
    AESCTRDRBG_Handle handle;
} AESCTRDRBG_s_CloseMsg;

typedef struct
{
    AESCTRDRBG_Handle handle;
    const void *seed;
    const void *additionalData;
    size_t additionalDataLength;
} AESCTRDRBG_s_ReseedMsg;

typedef struct
{
    AESCTRDRBG_Handle handle;
    void *randomBytes;
    size_t randomBytesSize;
} AESCTRDRBG_s_GetRandomBytesMsg;

typedef struct
{
    AESCTRDRBG_Handle handle;
    CryptoKey *randomKey;
} AESCTRDRBG_s_GenerateKeyMsg;

/*!
 *  @brief  Handles PSA messages for AES CTR DRBG secure driver
 *
 *  @note   This function should be called by secure partition thread only.
 *
 *  @param [in]  msg     pointer to PSA message
 *
 *  @retval PSA_SUCCESS if successful.
 *  @retval PSA_ERROR_PROGRAMMER_ERROR if any args point to secure addresses.
 */
psa_status_t AESCTRDRBG_s_handlePsaMsg(psa_msg_t *msg);

/*!
 *  @brief  Initializes the AES CTR DRBG secure driver.
 *
 *  @note   This function should be called by secure partition thread only.
 */
void AESCTRDRBG_s_init(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_aesctrdrbg_AESCTRDRBGX4_s__include */
