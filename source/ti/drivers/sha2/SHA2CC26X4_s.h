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

#ifndef ti_drivers_sha2_SHA2CC26X4_s__include
#define ti_drivers_sha2_SHA2CC26X4_s__include

#include <stdint.h>

#include <ti/drivers/crypto/CryptoCC26X4_s.h>
#include <ti/drivers/SHA2.h>

#include <ti/drivers/tfm/SecureCallback.h>

#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/interface/include/psa/service.h>

#if defined(TFM_BUILD)
    #include "ti_drivers_config.h" /* Sysconfig generated header */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SHA2 secure message types
 */
#define SHA2_S_MSG_TYPE_CONSTRUCT         SHA2_S_MSG_TYPE(0U)
#define SHA2_S_MSG_TYPE_OPEN              SHA2_S_MSG_TYPE(1U)
#define SHA2_S_MSG_TYPE_REGISTER_CALLBACK SHA2_S_MSG_TYPE(2U)
#define SHA2_S_MSG_TYPE_CLOSE             SHA2_S_MSG_TYPE(3U)
#define SHA2_S_MSG_TYPE_HASH_DATA         SHA2_S_MSG_TYPE(4U)
#define SHA2_S_MSG_TYPE_HMAC              SHA2_S_MSG_TYPE(5U)
#define SHA2_S_MSG_TYPE_SET_HASH_TYPE     SHA2_S_MSG_TYPE(6U)
#define SHA2_S_MSG_TYPE_SETUP_HMAC        SHA2_S_MSG_TYPE(7U)
#define SHA2_S_MSG_TYPE_ADD_DATA          SHA2_S_MSG_TYPE(8U)
#define SHA2_S_MSG_TYPE_FINALIZE          SHA2_S_MSG_TYPE(9U)
#define SHA2_S_MSG_TYPE_FINALIZE_HMAC     SHA2_S_MSG_TYPE(10U)
#define SHA2_S_MSG_TYPE_CANCEL_OPERATION  SHA2_S_MSG_TYPE(11U)
#define SHA2_S_MSG_TYPE_RESET             SHA2_S_MSG_TYPE(12U)

/*
 * Config pool size determines how many dynamic driver instances can be created
 * by the non-secure client using SHA2_construct().
 */
#ifndef CONFIG_SHA2_S_CONFIG_POOL_SIZE
    #define CONFIG_SHA2_S_CONFIG_POOL_SIZE 2 /* One instance used for EdDSA */
#endif

#define SHA2_SECURE_CALLBACK_COUNT (CONFIG_TI_DRIVERS_SHA2_COUNT + CONFIG_SHA2_S_CONFIG_POOL_SIZE)

/*
 * ========= SHA2 Secure Callback struct =========
 * Non-secure clients must register their callback after opening or
 * constructing a driver instance with blocking or callback return behavior.
 */
typedef struct
{
    SecureCallback_Object object;
    /* SHA2 callback fxn parameters */
    SHA2_Handle handle;
    int_fast16_t returnStatus;
} SHA2_s_SecureCallback;

/*
 * ========= SHA2 Secure Message Structs =========
 * These secure message structs correspond to the secure message types defined
 * above. Together, they are used by non-secure client to make PSA calls to the
 * SHA2 secure service. There is a single input vector for the PSA call
 * which is a pointer to secure message struct. If the underlying function
 * has a return value, there is a single output vector which is a pointer to
 * storage for the return value.
 */
typedef struct
{
    SHA2_Config *config;
    const SHA2_Params *params;
} SHA2_s_ConstructMsg;

typedef struct
{
    uint_least8_t index;
    const SHA2_Params *params;
} SHA2_s_OpenMsg;

typedef struct
{
    SHA2_Handle handle;
    SHA2_s_SecureCallback *callback;
} SHA2_s_CallbackMsg;

typedef struct
{
    SHA2_Handle handle;
} SHA2_s_CloseMsg;

typedef struct
{
    SHA2_Handle handle;
    const void *data;
    size_t dataLength;
    void *digest;
} SHA2_s_HashDataMsg;

typedef struct
{
    SHA2_Handle handle;
    const CryptoKey *key;
    const void *data;
    size_t dataLength;
    void *hmac;
} SHA2_s_HmacMsg;

typedef struct
{
    SHA2_Handle handle;
    SHA2_HashType type;
} SHA2_s_SetHashTypeMsg;

typedef struct
{
    SHA2_Handle handle;
    const CryptoKey *key;
} SHA2_s_SetupHmacMsg;

typedef struct
{
    SHA2_Handle handle;
    const void *data;
    size_t length;
} SHA2_s_AddDataMsg;

/* For SHA2_S_MSG_TYPE_FINALIZE or SHA2_S_MSG_TYPE_FINALIZE_HMAC */
typedef struct
{
    SHA2_Handle handle;
    void *digestOrHmac;
} SHA2_s_FinalizeMsg;

typedef struct
{
    SHA2_Handle handle;
} SHA2_s_CancelOperationMsg;

typedef struct
{
    SHA2_Handle handle;
} SHA2_s_ResetMsg;

/*!
 *  @brief  Handles PSA messages for SHA2 secure driver
 *
 *  @note   This function should be called by secure partition thread only.
 *
 *  @param [in]  msg     pointer to PSA message
 *
 *  @retval PSA_SUCCESS if successful.
 *  @retval PSA_ERROR_PROGRAMMER_ERROR if any args point to secure addresses.
 */
psa_status_t SHA2_s_handlePsaMsg(psa_msg_t *msg);

/*!
 *  @brief  Initializes the SHA2 secure driver.
 *
 *  @note   This function should be called by secure partition thread only.
 */
void SHA2_s_init(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_sha2_SHA2CC26X4_s__include */
