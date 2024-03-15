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

#ifndef ti_drivers_aesecb_AESECBCC26X4_s__include
#define ti_drivers_aesecb_AESECBCC26X4_s__include

#include <stdint.h>

#include <ti/drivers/crypto/CryptoCC26X4_s.h>
#include <ti/drivers/AESECB.h>

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
 * AES ECB secure message types
 */
#define AESECB_S_MSG_TYPE_CONSTRUCT         AESECB_S_MSG_TYPE(0U)
#define AESECB_S_MSG_TYPE_OPEN              AESECB_S_MSG_TYPE(1U)
#define AESECB_S_MSG_TYPE_REGISTER_CALLBACK AESECB_S_MSG_TYPE(2U)
#define AESECB_S_MSG_TYPE_CLOSE             AESECB_S_MSG_TYPE(3U)
#define AESECB_S_MSG_TYPE_ONE_STEP_ENCRYPT  AESECB_S_MSG_TYPE(4U)
#define AESECB_S_MSG_TYPE_ONE_STEP_DECRYPT  AESECB_S_MSG_TYPE(5U)
#define AESECB_S_MSG_TYPE_SETUP_ENCRYPT     AESECB_S_MSG_TYPE(6U)
#define AESECB_S_MSG_TYPE_SETUP_DECRYPT     AESECB_S_MSG_TYPE(7U)
#define AESECB_S_MSG_TYPE_ADD_DATA          AESECB_S_MSG_TYPE(8U)
#define AESECB_S_MSG_TYPE_FINALIZE          AESECB_S_MSG_TYPE(9U)
#define AESECB_S_MSG_TYPE_CANCEL_OPERATION  AESECB_S_MSG_TYPE(10U)

/*
 * Config pool size determines how many dynamic driver instances can be created
 * by the non-secure client using AESECB_construct().
 */
#ifndef CONFIG_AESECB_S_CONFIG_POOL_SIZE
    #define CONFIG_AESECB_S_CONFIG_POOL_SIZE 1
#endif

#define AESECB_SECURE_CALLBACK_COUNT (CONFIG_TI_DRIVERS_AESECB_COUNT + CONFIG_AESECB_S_CONFIG_POOL_SIZE)

/*
 * ========= AES ECB Secure Callback struct =========
 * Non-secure clients must register their callback after opening or
 * constructing a driver instance with blocking or callback return behavior.
 */
typedef struct
{
    SecureCallback_Object object;
    /* AES ECB callback fxn parameters */
    AESECB_Handle handle;
    int_fast16_t returnValue;
    AESECB_Operation *operation;
    AESECB_OperationType operationType;
} AESECB_s_SecureCallback;

/*
 * ========= AES ECB Secure Message Structs =========
 * These secure message structs correspond to the secure message types defined
 * above. Together, they are used by non-secure client to make PSA calls to the
 * AES ECB secure service. There is a single input vector for the PSA call
 * which is a pointer to secure message struct. If the underlying function
 * has a return value, there is a single output vector which is a pointer to
 * storage for the return value.
 */
typedef struct
{
    AESECB_Config *config;
    const AESECB_Params *params;
} AESECB_s_ConstructMsg;

typedef struct
{
    uint_least8_t index;
    const AESECB_Params *params;
} AESECB_s_OpenMsg;

typedef struct
{
    AESECB_Handle handle;
    AESECB_s_SecureCallback *callback;
} AESECB_s_CallbackMsg;

typedef struct
{
    AESECB_Handle handle;
} AESECB_s_CloseMsg;

typedef struct
{
    AESECB_Handle handle;
    AESECB_Operation *operation;
} AESECB_s_OperationMsg;

typedef AESECB_s_OperationMsg AESECB_s_OneStepOperationMsg;

typedef struct
{
    AESECB_Handle handle;
    const CryptoKey *key;
} AESECB_s_SetupOperationMsg;

typedef AESECB_s_OperationMsg AESECB_s_AddDataMsg;

typedef AESECB_s_OperationMsg AESECB_s_FinalizeMsg;

typedef struct
{
    AESECB_Handle handle;
} AESECB_s_CancelOperationMsg;

/*!
 *  @brief  Handles PSA messages for AES ECB secure driver
 *
 *  @note   This function should be called by secure partition thread only.
 *
 *  @param [in]  msg     pointer to PSA message
 *
 *  @retval PSA_SUCCESS if successful.
 *  @retval PSA_ERROR_PROGRAMMER_ERROR if any args point to secure addresses.
 */
psa_status_t AESECB_s_handlePsaMsg(psa_msg_t *msg);

/*!
 *  @brief  Initializes the AES ECB secure driver.
 *
 *  @note   This function should be called by secure partition thread only.
 */
void AESECB_s_init(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_aesecb_AESECBCC26X4_s__include */
