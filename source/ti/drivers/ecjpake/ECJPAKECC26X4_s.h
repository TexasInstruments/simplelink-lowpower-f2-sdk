/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
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

#ifndef ti_drivers_ecjpake_ECJPAKECC26X4_s__include
#define ti_drivers_ecjpake_ECJPAKECC26X4_s__include

#include <stdint.h>

#include <ti/drivers/crypto/CryptoCC26X4_s.h>
#include <ti/drivers/ECJPAKE.h>

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
 * ECJPAKE secure message types
 */
#define ECJPAKE_S_MSG_TYPE_CONSTRUCT               ECJPAKE_S_MSG_TYPE(0U)
#define ECJPAKE_S_MSG_TYPE_OPEN                    ECJPAKE_S_MSG_TYPE(1U)
#define ECJPAKE_S_MSG_TYPE_REGISTER_CALLBACK       ECJPAKE_S_MSG_TYPE(2U)
#define ECJPAKE_S_MSG_TYPE_CLOSE                   ECJPAKE_S_MSG_TYPE(3U)
#define ECJPAKE_S_MSG_TYPE_ROUND_ONE_GENERATE_KEYS ECJPAKE_S_MSG_TYPE(4U)
#define ECJPAKE_S_MSG_TYPE_GENERATE_ZKP            ECJPAKE_S_MSG_TYPE(5U)
#define ECJPAKE_S_MSG_TYPE_VERIFY_ZKP              ECJPAKE_S_MSG_TYPE(6U)
#define ECJPAKE_S_MSG_TYPE_ROUND_TWO_GENERATE_KEYS ECJPAKE_S_MSG_TYPE(7U)
#define ECJPAKE_S_MSG_TYPE_COMPUTE_SHARED_SECRET   ECJPAKE_S_MSG_TYPE(8U)
#define ECJPAKE_S_MSG_TYPE_CANCEL_OPERATION        ECJPAKE_S_MSG_TYPE(9U)

/*
 * Config pool size determines how many dynamic driver instances can be created
 * by the non-secure client using ECJPAKE_construct().
 */
#ifndef CONFIG_ECJPAKE_S_CONFIG_POOL_SIZE
    #define CONFIG_ECJPAKE_S_CONFIG_POOL_SIZE 1
#endif

#define ECJPAKE_SECURE_CALLBACK_COUNT (CONFIG_TI_DRIVERS_ECJPAKE_COUNT + CONFIG_ECJPAKE_S_CONFIG_POOL_SIZE)

/*
 * ========= ECJPAKE Secure Callback struct =========
 * Non-secure clients must register their callback after opening or
 * constructing a driver instance with blocking or callback return behavior.
 */
typedef struct
{
    SecureCallback_Object object;
    /* ECJPAKE callback fxn parameters */
    ECJPAKE_Handle handle;
    int_fast16_t returnStatus;
    ECJPAKE_Operation operation;
    ECJPAKE_OperationType operationType;
} ECJPAKE_s_SecureCallback;

/*
 * ========= ECJPAKE Secure Message Structs =========
 * These secure message structs correspond to the secure message types defined
 * above. Together, they are used by non-secure client to make PSA calls to the
 * ECJPAKE secure service. There is a single input vector for the PSA call
 * which is a pointer to secure message struct. If the underlying function
 * has a return value, there is a single output vector which is a pointer to
 * storage for the return value.
 */
typedef struct
{
    ECJPAKE_Config *config;
    const ECJPAKE_Params *params;
} ECJPAKE_s_ConstructMsg;

typedef struct
{
    uint_least8_t index;
    const ECJPAKE_Params *params;
} ECJPAKE_s_OpenMsg;

typedef struct
{
    ECJPAKE_Handle handle;
    ECJPAKE_s_SecureCallback *callback;
} ECJPAKE_s_CallbackMsg;

typedef struct
{
    ECJPAKE_Handle handle;
} ECJPAKE_s_CloseMsg;

/*
 * Message struct for the following message types:
 *  - ECJPAKE_S_MSG_TYPE_ROUND_ONE_GENERATE_KEYS
 *  - ECJPAKE_S_MSG_TYPE_GENERATE_ZKP
 *  - ECJPAKE_S_MSG_TYPE_VERIFY_ZKP
 *  - ECJPAKE_S_MSG_TYPE_ROUND_TWO_GENERATE_KEYS
 *  - ECJPAKE_S_MSG_TYPE_COMPUTE_SHARED_SECRET
 */
typedef struct
{
    ECJPAKE_Handle handle;
    ECJPAKE_Operation operation;
} ECJPAKE_s_OperationMsg;

typedef struct
{
    ECJPAKE_Handle handle;
} ECJPAKE_s_CancelOperationMsg;

/*!
 *  @brief  Handles PSA messages for ECJPAKE secure driver
 *
 *  @note   This function should be called by secure partition thread only.
 *
 *  @param [in]  msg     pointer to PSA message
 *
 *  @retval PSA_SUCCESS if successful.
 *  @retval PSA_ERROR_PROGRAMMER_ERROR if any args point to secure addresses.
 */
psa_status_t ECJPAKE_s_handlePsaMsg(psa_msg_t *msg);

/*!
 *  @brief  Initializes the ECJPAKE secure driver.
 *
 *  @note   This function should be called by secure partition thread only.
 */
void ECJPAKE_s_init(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_ecjpake_ECJPAKECC26X4_s__include */
