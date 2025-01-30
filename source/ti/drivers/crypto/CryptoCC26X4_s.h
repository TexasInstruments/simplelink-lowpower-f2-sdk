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
/*!*****************************************************************************
 *  @file       CryptoCC26X4_s.h
 *  @brief      Secure Crypto Service
 *
 *  @anchor ti_drivers_crypto_CryptoCC26X4_s_Overview
 *  # Overview
 *      The Secure Crypto Service is used to access all cryptographic functions
 *      when using the TF-M.
 *
 *******************************************************************************
 */
#ifndef ti_drivers_crypto_CryptoCC26X4_s__include
#define ti_drivers_crypto_CryptoCC26X4_s__include

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Index values for secure msg types for various crypto drivers */
#define CRYPTO_S_MSG_TYPE_INDEX_AESCBC     ((int32_t)0x11)
#define CRYPTO_S_MSG_TYPE_INDEX_AESCCM     ((int32_t)0x22)
#define CRYPTO_S_MSG_TYPE_INDEX_AESCMAC    ((int32_t)0x33)
#define CRYPTO_S_MSG_TYPE_INDEX_AESCTR     ((int32_t)0x44)
#define CRYPTO_S_MSG_TYPE_INDEX_AESECB     ((int32_t)0x55)
#define CRYPTO_S_MSG_TYPE_INDEX_AESGCM     ((int32_t)0x66)
#define CRYPTO_S_MSG_TYPE_INDEX_AESCTRDRBG ((int32_t)0x77)
#define CRYPTO_S_MSG_TYPE_INDEX_ECDH       ((int32_t)0x88)
#define CRYPTO_S_MSG_TYPE_INDEX_ECDSA      ((int32_t)0x99)
#define CRYPTO_S_MSG_TYPE_INDEX_ECJPAKE    ((int32_t)0xAA)
#define CRYPTO_S_MSG_TYPE_INDEX_EDDSA      ((int32_t)0xBB)
#define CRYPTO_S_MSG_TYPE_INDEX_SHA2       ((int32_t)0xCC)
#define CRYPTO_S_MSG_TYPE_INDEX_TRNG       ((int32_t)0xDD)
#define CRYPTO_S_MSG_TYPE_INDEX_KEYSTORE   ((int32_t)0xEE)
#define CRYPTO_S_MSG_TYPE_INDEX_PSA        ((int32_t)0xFF)

#define CRYPTO_S_MSG_TYPE_FUNCNUM_BITS    4U
#define CRYPTO_S_MSG_TYPE_FUNCNUM_SHIFT   8U
#define CRYPTO_S_MSG_TYPE_INDEX_MASK      0xFF
#define GET_CRYPTO_S_MSG_TYPE_INDEX(type) ((type) & (int32_t)CRYPTO_S_MSG_TYPE_INDEX_MASK)

/* TF-M PSA NS interface internally limits the type to int16_t */
#define TFM_PSA_TYPE_MASK 0x00007FFF

/*
 * Macros used to generate PSA message type values for various crypto driver
 * functions. It duplicates the funcNum in the highest nibble when possible
 * for bit flip resistance. The funcNum value is limited to 15.
 */
#define CRYPTO_S_MSG_TYPE(index, funcNum)                                                           \
    (((index) | ((int32_t)(funcNum) << CRYPTO_S_MSG_TYPE_FUNCNUM_SHIFT) |                           \
      ((int32_t)(funcNum) << (CRYPTO_S_MSG_TYPE_FUNCNUM_SHIFT + CRYPTO_S_MSG_TYPE_FUNCNUM_BITS))) & \
     TFM_PSA_TYPE_MASK)

#define AESCBC_S_MSG_TYPE(funcNum)       CRYPTO_S_MSG_TYPE(CRYPTO_S_MSG_TYPE_INDEX_AESCBC, funcNum)
#define AESCCM_S_MSG_TYPE(funcNum)       CRYPTO_S_MSG_TYPE(CRYPTO_S_MSG_TYPE_INDEX_AESCCM, funcNum)
#define AESCMAC_S_MSG_TYPE(funcNum)      CRYPTO_S_MSG_TYPE(CRYPTO_S_MSG_TYPE_INDEX_AESCMAC, funcNum)
#define AESCTR_S_MSG_TYPE(funcNum)       CRYPTO_S_MSG_TYPE(CRYPTO_S_MSG_TYPE_INDEX_AESCTR, funcNum)
#define AESCTRDRBG_S_MSG_TYPE(funcNum)   CRYPTO_S_MSG_TYPE(CRYPTO_S_MSG_TYPE_INDEX_AESCTRDRBG, funcNum)
#define AESECB_S_MSG_TYPE(funcNum)       CRYPTO_S_MSG_TYPE(CRYPTO_S_MSG_TYPE_INDEX_AESECB, funcNum)
#define AESGCM_S_MSG_TYPE(funcNum)       CRYPTO_S_MSG_TYPE(CRYPTO_S_MSG_TYPE_INDEX_AESGCM, funcNum)
#define ECDH_S_MSG_TYPE(funcNum)         CRYPTO_S_MSG_TYPE(CRYPTO_S_MSG_TYPE_INDEX_ECDH, funcNum)
#define ECDSA_S_MSG_TYPE(funcNum)        CRYPTO_S_MSG_TYPE(CRYPTO_S_MSG_TYPE_INDEX_ECDSA, funcNum)
#define ECJPAKE_S_MSG_TYPE(funcNum)      CRYPTO_S_MSG_TYPE(CRYPTO_S_MSG_TYPE_INDEX_ECJPAKE, funcNum)
#define EDDSA_S_MSG_TYPE(funcNum)        CRYPTO_S_MSG_TYPE(CRYPTO_S_MSG_TYPE_INDEX_EDDSA, funcNum)
#define SHA2_S_MSG_TYPE(funcNum)         CRYPTO_S_MSG_TYPE(CRYPTO_S_MSG_TYPE_INDEX_SHA2, funcNum)
#define TRNG_S_MSG_TYPE(funcNum)         CRYPTO_S_MSG_TYPE(CRYPTO_S_MSG_TYPE_INDEX_TRNG, funcNum)
#define KEYSTORE_PSA_S_MSG_TYPE(funcNum) CRYPTO_S_MSG_TYPE(CRYPTO_S_MSG_TYPE_INDEX_KEYSTORE, funcNum)
#define PSA_S_MSG_TYPE(funcNum)          CRYPTO_S_MSG_TYPE(CRYPTO_S_MSG_TYPE_INDEX_PSA, funcNum)

/*
 * Secure handle ID values which correspond to unmapped memory ranges are used
 * to avoid confusion with non-secure driver handles which point to valid
 * memory locations.
 */
enum
{
    CRYPTO_S_HANDLE_ID_AESCBC     = 0xABCD1000U,
    CRYPTO_S_HANDLE_ID_AESCCM     = 0xABCD2000U,
    CRYPTO_S_HANDLE_ID_AESCMAC    = 0xABCD3000U,
    CRYPTO_S_HANDLE_ID_AESCTR     = 0xABCD4000U,
    CRYPTO_S_HANDLE_ID_AESECB     = 0xABCD5000U,
    CRYPTO_S_HANDLE_ID_AESGCM     = 0xABCD6000U,
    CRYPTO_S_HANDLE_ID_AESCTRDRBG = 0xABCD7000U,
    CRYPTO_S_HANDLE_ID_ECDH       = 0xABCD8000U,
    CRYPTO_S_HANDLE_ID_ECDSA      = 0xABCD9000U,
    CRYPTO_S_HANDLE_ID_ECJPAKE    = 0xABCDA000U,
    CRYPTO_S_HANDLE_ID_EDDSA      = 0xABCDB000U,
    CRYPTO_S_HANDLE_ID_SHA2       = 0xABCDC000U,
    CRYPTO_S_HANDLE_ID_TRNG       = 0xABCDD000U,
};

#define CRYPTO_S_HANDLE_ID_MASK    0xFFFFF000U
#define CRYPTO_S_HANDLE_INDEX_MASK 0x00000FFFU

#define GET_CRYPTO_S_HANDLE_ID(handle)    ((uintptr_t)(handle)&CRYPTO_S_HANDLE_ID_MASK)
#define GET_CRYPTO_S_HANDLE_INDEX(handle) ((uintptr_t)(handle)&CRYPTO_S_HANDLE_INDEX_MASK)

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_crypto_CryptoCC26X4_s__include */
