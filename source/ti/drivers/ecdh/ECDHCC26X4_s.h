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

#ifndef ti_drivers_ecdh_ECDHCC26X4_s__include
#define ti_drivers_ecdh_ECDHCC26X4_s__include

#include <stdint.h>

#include <ti/drivers/crypto/CryptoCC26X4_s.h>
#include <ti/drivers/ECDH.h>

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
 * ECDH secure message types
 */
#define ECDH_S_MSG_TYPE_CONSTRUCT             ECDH_S_MSG_TYPE(0U)
#define ECDH_S_MSG_TYPE_OPEN                  ECDH_S_MSG_TYPE(1U)
#define ECDH_S_MSG_TYPE_REGISTER_CALLBACK     ECDH_S_MSG_TYPE(2U)
#define ECDH_S_MSG_TYPE_CLOSE                 ECDH_S_MSG_TYPE(3U)
#define ECDH_S_MSG_TYPE_GENERATE_PUBLIC_KEY   ECDH_S_MSG_TYPE(4U)
#define ECDH_S_MSG_TYPE_COMPUTE_SHARED_SECRET ECDH_S_MSG_TYPE(5U)
#define ECDH_S_MSG_TYPE_CANCEL_OPERATION      ECDH_S_MSG_TYPE(6U)

/*
 * Config pool size determines how many dynamic driver instances can be created
 * by the non-secure client using ECDH_construct().
 */
#ifndef CONFIG_ECDH_S_CONFIG_POOL_SIZE
    #define CONFIG_ECDH_S_CONFIG_POOL_SIZE 1
#endif

#define ECDH_SECURE_CALLBACK_COUNT (CONFIG_TI_DRIVERS_ECDH_COUNT + CONFIG_ECDH_S_CONFIG_POOL_SIZE)

/*
 * ========= ECDH Secure Callback struct =========
 * Non-secure clients must register their callback after opening or
 * constructing a driver instance with blocking or callback return behavior.
 */
typedef struct
{
    SecureCallback_Object object;
    /* ECDH callback fxn parameters */
    ECDH_Handle handle;
    int_fast16_t returnStatus;
    ECDH_Operation operation;
    ECDH_OperationType operationType;
} ECDH_s_SecureCallback;

/*
 * ========= ECDH Secure Message Structs =========
 * These secure message structs correspond to the secure message types defined
 * above. Together, they are used by non-secure client to make PSA calls to the
 * ECDH secure service. There is a single input vector for the PSA call
 * which is a pointer to secure message struct. If the underlying function
 * has a return value, there is a single output vector which is a pointer to
 * storage for the return value.
 */
typedef struct
{
    ECDH_Config *config;
    const ECDH_Params *params;
} ECDH_s_ConstructMsg;

typedef struct
{
    uint_least8_t index;
    const ECDH_Params *params;
} ECDH_s_OpenMsg;

typedef struct
{
    ECDH_Handle handle;
    ECDH_s_SecureCallback *callback;
} ECDH_s_CallbackMsg;

typedef struct
{
    ECDH_Handle handle;
} ECDH_s_CloseMsg;

typedef struct
{
    ECDH_Handle handle;
    ECDH_OperationGeneratePublicKey *operation;
} ECDH_s_GeneratePublicKeyMsg;

typedef struct
{
    ECDH_Handle handle;
    ECDH_OperationComputeSharedSecret *operation;
} ECDH_s_ComputeSharedSecretMsg;

typedef struct
{
    ECDH_Handle handle;
} ECDH_s_CancelOperationMsg;

/*!
 *  @brief  Handles PSA messages for ECDH secure driver
 *
 *  @note   This function should be called by secure partition thread only.
 *
 *  @param [in]  msg     pointer to PSA message
 *
 *  @retval PSA_SUCCESS if successful.
 *  @retval PSA_ERROR_PROGRAMMER_ERROR if any args point to secure addresses.
 */
psa_status_t ECDH_s_handlePsaMsg(psa_msg_t *msg);

/*!
 *  @brief  Initializes the ECDH secure driver.
 *
 *  @note   This function should be called by secure partition thread only.
 */
void ECDH_s_init(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_ecdh_ECDHCC26X4_s__include */
