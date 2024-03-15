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

#ifndef ti_drivers_aesccm_AESCCMCC26X4_s__include
#define ti_drivers_aesccm_AESCCMCC26X4_s__include

#include <stdint.h>

#include <ti/drivers/crypto/CryptoCC26X4_s.h>
#include <ti/drivers/AESCCM.h>

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
 * AES CCM secure message types
 *
 * For optimal performance, non-secure clients must use the
 * AESCCM_s_oneStepEncryptFast and AESCCM_s_oneStepDecryptFast veneers
 * instead of PSA calls with AESCCM_S_MSG_TYPE_ONE_STEP_ENCRYPT
 * or AESCCM_S_MSG_TYPE_ONE_STEP_DECRYPT.
 */
#define AESCCM_S_MSG_TYPE_CONSTRUCT         AESCCM_S_MSG_TYPE(0U)
#define AESCCM_S_MSG_TYPE_OPEN              AESCCM_S_MSG_TYPE(1U)
#define AESCCM_S_MSG_TYPE_REGISTER_CALLBACK AESCCM_S_MSG_TYPE(2U)
#define AESCCM_S_MSG_TYPE_CLOSE             AESCCM_S_MSG_TYPE(3U)
#define AESCCM_S_MSG_TYPE_ONE_STEP_ENCRYPT  AESCCM_S_MSG_TYPE(4U)
#define AESCCM_S_MSG_TYPE_ONE_STEP_DECRYPT  AESCCM_S_MSG_TYPE(5U)
#define AESCCM_S_MSG_TYPE_SETUP_ENCRYPT     AESCCM_S_MSG_TYPE(6U)
#define AESCCM_S_MSG_TYPE_SETUP_DECRYPT     AESCCM_S_MSG_TYPE(7U)
#define AESCCM_S_MSG_TYPE_SET_LENGTHS       AESCCM_S_MSG_TYPE(8U)
#define AESCCM_S_MSG_TYPE_SET_NONCE         AESCCM_S_MSG_TYPE(9U)
#define AESCCM_S_MSG_TYPE_ADD_AAD           AESCCM_S_MSG_TYPE(10U)
#define AESCCM_S_MSG_TYPE_ADD_DATA          AESCCM_S_MSG_TYPE(11U)
#define AESCCM_S_MSG_TYPE_FINALIZE_ENCRYPT  AESCCM_S_MSG_TYPE(12U)
#define AESCCM_S_MSG_TYPE_FINALIZE_DECRYPT  AESCCM_S_MSG_TYPE(13U)
#define AESCCM_S_MSG_TYPE_CANCEL_OPERATION  AESCCM_S_MSG_TYPE(14U)

/*
 * Config pool size determines how many dynamic driver instances can be created
 * by the non-secure client using AESCCM_construct().
 */
#ifndef CONFIG_AESCCM_S_CONFIG_POOL_SIZE
    #define CONFIG_AESCCM_S_CONFIG_POOL_SIZE 1
#endif

#define AESCCM_SECURE_CALLBACK_COUNT (CONFIG_TI_DRIVERS_AESCCM_COUNT + CONFIG_AESCCM_S_CONFIG_POOL_SIZE)

/*
 * ========= AES CCM Secure Callback struct =========
 * Non-secure clients must register their callback after opening or
 * constructing a driver instance with blocking or callback return behavior.
 */
typedef struct
{
    SecureCallback_Object object;
    /* AES CCM callback fxn parameters */
    AESCCM_Handle handle;
    int_fast16_t returnValue;
    AESCCM_OperationUnion *operation;
    AESCCM_OperationType operationType;
} AESCCM_s_SecureCallback;

/*
 * ========= AES CCM Secure Message Structs =========
 * These secure message structs correspond to the secure message types defined
 * above. Together, they are used by non-secure client to make PSA calls to the
 * AES CCM secure service. There is a single input vector for the PSA call
 * which is a pointer to secure message struct. If the underlying function
 * has a return value, there is a single output vector which is a pointer to
 * storage for the return value.
 */
typedef struct
{
    AESCCM_Config *config;
    const AESCCM_Params *params;
} AESCCM_s_ConstructMsg;

typedef struct
{
    uint_least8_t index;
    const AESCCM_Params *params;
} AESCCM_s_OpenMsg;

typedef struct
{
    AESCCM_Handle handle;
    AESCCM_s_SecureCallback *callback;
} AESCCM_s_CallbackMsg;

typedef struct
{
    AESCCM_Handle handle;
} AESCCM_s_CloseMsg;

typedef struct
{
    AESCCM_Handle handle;
    AESCCM_OneStepOperation *operation;
} AESCCM_s_OneStepOperationMsg;

typedef struct
{
    AESCCM_Handle handle;
    const CryptoKey *key;
    size_t aadLength;
    size_t plaintextLength;
    size_t macLength;
} AESCCM_s_SetupOperationMsg;

typedef struct
{
    AESCCM_Handle handle;
    size_t aadLength;
    size_t plaintextLength;
    size_t macLength;
} AESCCM_s_SetLengthsMsg;

typedef struct
{
    AESCCM_Handle handle;
    const uint8_t *nonce;
    size_t nonceLength;
} AESCCM_s_SetNonceMsg;

typedef struct
{
    AESCCM_Handle handle;
    AESCCM_SegmentedAADOperation *operation;
} AESCCM_s_AddAADMsg;

typedef struct
{
    AESCCM_Handle handle;
    AESCCM_SegmentedDataOperation *operation;
} AESCCM_s_AddDataMsg;

typedef struct
{
    AESCCM_Handle handle;
    AESCCM_SegmentedFinalizeOperation *operation;
} AESCCM_s_FinalizeOperationMsg;

typedef struct
{
    AESCCM_Handle handle;
} AESCCM_s_CancelOperationMsg;

/*!
 *  @brief  Fast veneer to perform a single-step AESCCM encryption &
 *          authentication operation in polling mode without the overhead of
 *          PSA call.
 *
 *  @note   See AESCCM_oneStepEncrypt() for full description, parameters, and
 *          return values. PSA_ERROR_PROGRAMMER_ERROR is an additional return
 *          value.
 *
 *  @retval PSA_ERROR_PROGRAMMER_ERROR if any args point to secure addresses
 *          or handle is invalid.
 */
int_fast16_t AESCCM_s_oneStepEncryptFast(AESCCM_Handle handle, AESCCM_Operation *operation);

/*!
 *  @brief  Fast veneer to perform a single-step AESCCM decryption &
 *          verification operation in polling mode without the overhead of
 *          PSA call.
 *
 *  @note   See AESCCM_oneStepDecrypt() for full description, parameters, and
 *          return values. PSA_ERROR_PROGRAMMER_ERROR is an additional return
 *          value.
 *
 *  @retval PSA_ERROR_PROGRAMMER_ERROR if any args point to secure addresses
 *          or handle is invalid.
 */
int_fast16_t AESCCM_s_oneStepDecryptFast(AESCCM_Handle handle, AESCCM_Operation *operation);

/*!
 *  @brief  Handles PSA messages for AES CCM secure driver
 *
 *  @note   This function should be called by secure partition thread only.
 *
 *  @param [in]  msg     pointer to PSA message
 *
 *  @retval PSA_SUCCESS if successful.
 *  @retval PSA_ERROR_PROGRAMMER_ERROR if any args point to secure addresses.
 */
psa_status_t AESCCM_s_handlePsaMsg(psa_msg_t *msg);

/*!
 *  @brief  Initializes the AES CCM secure driver.
 *
 *  @note   This function should be called by secure partition thread only.
 */
void AESCCM_s_init(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_aesccm_AESCCMCC26X4_s__include */
