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

#ifndef ti_drivers_trng_TRNGCC26X4_s__include
#define ti_drivers_trng_TRNGCC26X4_s__include

#include <stdint.h>

#include <ti/drivers/crypto/CryptoCC26X4_s.h>
#include <ti/drivers/TRNG.h>

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
 * TRNG secure message types
 */
#define TRNG_S_MSG_TYPE_CONSTRUCT          TRNG_S_MSG_TYPE(0U)
#define TRNG_S_MSG_TYPE_OPEN               TRNG_S_MSG_TYPE(1U)
#define TRNG_S_MSG_TYPE_REGISTER_CALLBACKS TRNG_S_MSG_TYPE(2U)
#define TRNG_S_MSG_TYPE_CLOSE              TRNG_S_MSG_TYPE(3U)
#define TRNG_S_MSG_TYPE_GENERATE_KEY       TRNG_S_MSG_TYPE(4U)
#define TRNG_S_MSG_TYPE_GET_RANDOM_BYTES   TRNG_S_MSG_TYPE(5U)
#define TRNG_S_MSG_TYPE_CANCEL_OPERATION   TRNG_S_MSG_TYPE(6U)

/*
 * Config pool size determines how many dynamic driver instances can be created
 * by the non-secure client using TRNG_construct().
 */
#ifndef CONFIG_TRNG_S_CONFIG_POOL_SIZE
    #define CONFIG_TRNG_S_CONFIG_POOL_SIZE 2 /* One instance used for ECDSA */
#endif

#define TRNG_SECURE_CALLBACK_COUNT (CONFIG_TI_DRIVERS_TRNG_COUNT + CONFIG_TRNG_S_CONFIG_POOL_SIZE)

/*
 * ========= TRNG Secure Callback structs =========
 * Non-secure clients must register their callback after opening or
 * constructing a driver instance with blocking or callback return behavior.
 */
typedef struct
{
    SecureCallback_Object object;
    /* TRNG crypto key callback fxn parameters */
    TRNG_Handle handle;
    int_fast16_t returnValue;
    CryptoKey *entropy;
} TRNG_s_CryptoKeySecureCallback;

typedef struct
{
    SecureCallback_Object object;
    /* TRNG random bytes callback fxn parameters */
    TRNG_Handle handle;
    int_fast16_t returnValue;
    uint8_t *randomBytes;
    size_t randomBytesSize;
} TRNG_s_RandomBytesSecureCallback;

/*
 * ========= TRNG Secure Message Structs =========
 * These secure message structs correspond to the secure message types defined
 * above. Together, they are used by non-secure client to make PSA calls to the
 * TRNG secure service. There is a single input vector for the PSA call
 * which is a pointer to secure message struct. If the underlying function
 * has a return value, there is a single output vector which is a pointer to
 * storage for the return value.
 */
typedef struct
{
    TRNG_Config *config;
    const TRNG_Params *params;
} TRNG_s_ConstructMsg;

typedef struct
{
    uint_least8_t index;
    const TRNG_Params *params;
} TRNG_s_OpenMsg;

typedef struct
{
    TRNG_Handle handle;
    TRNG_s_CryptoKeySecureCallback *cryptoKeyCallback;
    TRNG_s_RandomBytesSecureCallback *randomBytesCallback;
} TRNG_s_CallbackMsg;

typedef struct
{
    TRNG_Handle handle;
} TRNG_s_CloseMsg;

typedef struct
{
    TRNG_Handle handle;
    CryptoKey *entropy;
} TRNG_s_GenerateKeyMsg;

typedef struct
{
    TRNG_Handle handle;
    void *randomBytes;
    size_t randomBytesSize;
} TRNG_s_GetRandomBytesMsg;

typedef struct
{
    TRNG_Handle handle;
} TRNG_s_CancelOperationMsg;

/*!
 *  @brief  Handles PSA messages for TRNG secure driver
 *
 *  @note   This function should be called by secure partition thread only.
 *
 *  @param [in]  msg     pointer to PSA message
 *
 *  @retval PSA_SUCCESS if successful.
 *  @retval PSA_ERROR_PROGRAMMER_ERROR if any args point to secure addresses.
 */
psa_status_t TRNG_s_handlePsaMsg(psa_msg_t *msg);

/*!
 *  @brief  Initializes the TRNG secure driver.
 *
 *  @note   This function should be called by the non-secure client and power
 *          to TRNG HW must be enabled first.
 */
void TRNG_s_init(void);

/*!
 *  @brief  Fast veneer to set the number of entropy generation cycles before
 *          the results are returned.
 *
 *  @param  handle           A valid TRNGCC26XX handle returned from TRNGCC26XX_open
 *  @param  samplesPerCycle  Number of 48MHz clock cycles to sample. Must be between 2^8 and 2^24.
 *
 *  @retval PSA_ERROR_PROGRAMMER_ERROR if the handle is invalid.
 *  @retval TRNG_STATUS_SUCCESS if successful.
 */
int_fast16_t TRNGCC26XX_s_setSamplesPerCycle(TRNG_Handle handle, uint32_t samplesPerCycle);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_trng_TRNGCC26X4_s__include */
