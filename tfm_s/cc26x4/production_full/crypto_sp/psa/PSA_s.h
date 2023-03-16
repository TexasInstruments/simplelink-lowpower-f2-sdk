/*
 * Copyright (c) 2022 Texas Instruments Incorporated - http://www.ti.com
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

#ifndef psa_s__include
#define psa_s__include

#include <stdint.h>

#include <ti/drivers/crypto/CryptoCC26X4_s.h>

#include <third_party/tfm/interface/include/psa/crypto.h>
#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/interface/include/psa/service.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PSA_S_MSG_TYPE(index) (CRYPTO_S_MSG_TYPE_INDEX_PSA | ((int32_t)1 << (CRYPTO_S_MSG_TYPE_SHIFT + (index))))

/*
 * PSA secure message types
 */
#define PSA_S_MSG_TYPE_SIGN_HASH   PSA_S_MSG_TYPE(0U)
#define PSA_S_MSG_TYPE_HASH_SETUP  PSA_S_MSG_TYPE(1U)
#define PSA_S_MSG_TYPE_HASH_UPDATE PSA_S_MSG_TYPE(2U)
#define PSA_S_MSG_TYPE_HASH_FINISH PSA_S_MSG_TYPE(3U)

/*
 * ========= PSA Secure Message Structs =========
 * These secure message structs correspond to the secure message types defined
 * above. There is a single input vector for the PSA call which is a pointer to
 * secure message struct. If the underlying function has a return value, there
 * is a single output vector which is a pointer to storage for the return value.
 */
typedef struct
{
    psa_key_handle_t handle;
    psa_algorithm_t alg;
    const uint8_t *hash;
    size_t hash_length;
    uint8_t *signature;
    size_t signature_size;
    size_t *signature_length;
} PSA_s_SignHashMsg;

typedef struct
{
    psa_hash_operation_t *operation;
    psa_algorithm_t alg;
} PSA_s_HashSetupMsg;

typedef struct
{
    psa_hash_operation_t *operation;
    const uint8_t *input;
    size_t input_length;
} PSA_s_HashUpdateMsg;

typedef struct
{
    psa_hash_operation_t *operation;
    uint8_t *hash;
    size_t hash_size;
    size_t *hash_length;
} PSA_s_HashFinishMsg;

/*!
 *  @brief  Handles PSA messages for PSA Crypto driver
 *
 *  @note   This function should be called by secure partition thread only.
 *
 *  @param [in]  msg     pointer to PSA message
 *
 *  @retval PSA_SUCCESS if successful.
 *  @retval PSA_ERROR_PROGRAMMER_ERROR if any args point to secure addresses.
 */
psa_status_t PSA_s_handlePsaMsg(psa_msg_t *msg);

/*!
 *  @brief  Initializes the PSA secure driver.
 *
 *  @note   This function should be called by secure partition thread only.
 */
void PSA_s_init(void);

#ifdef __cplusplus
}
#endif
#endif