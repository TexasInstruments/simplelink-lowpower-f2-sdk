/******************************************************************************
 Copyright (c) 2022-2023, Texas Instruments Incorporated
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

 *****************************************************************************/

/*!
 *  @file       gcm_alt.h
 *
 *  @brief      Hardware accelerated mbedtls aesgcm implementation
 *
 *  # General
 *
 *  # Limitations
 *
 */

#ifndef MBEDTLS_GCM_ALT_H
#define MBEDTLS_GCM_ALT_H

#include "mbedtls/build_info.h"

#if defined(MBEDTLS_GCM_ALT)

#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4)
#include <ti/drivers/aesgcm/AESGCMCC26X4.h>
   typedef AESGCMCC26X4_Object AESGCM_Object;
   typedef AESGCMCC26X4_HWAttrs AESGCM_HWAttrs;
#else
   #error "No valid DeviceFamily found for the AES-GCM alternate implementation!"
#endif

/**
 * \brief    The GCM context-type definition. The GCM context is passed
 *           to the APIs called.
 */
typedef struct
{
    int mode;                /*!<  operation to perform (MBEDTLS_GCM_ENCRYPT or MBEDTLS_GCM_DECRYPT) */
    CryptoKey cryptoKey;     /*!<  input to the crypto driver */
    uint8_t keyMaterial[32]; /*!<  storage for the key */
    AESGCM_Handle handle;    /*!<  Platform AESGCM handle */
    AESGCM_Config gcmConfig; /*!<  structure containing AESGCM driver specific implementation */
    AESGCM_Object gcmObject; /*!<  Pointer to a driver specific data object */
} mbedtls_gcm_context;

#ifdef __cplusplus
}
#endif

/**
 * \brief           This function declares the lengths of the message
 *                  and additional data for a GCM encryption or decryption
 *                  operation.
 *
 *                  This function must be called before calling mbedtls_gcm_update_ad()
 *                  or mbedtls_gcm_update(). This function must be called after mbedtls_gcm_starts().
 *
 * \note            This function is required for this platform.
 *
 * \param ctx       The GCM context. This must be initialized.
 * \param total_ad_len   The total length of additional data in bytes.
 *                       This must be less than `2^16 - 2^8`.
 * \param plaintext_len  The length in bytes of the plaintext to encrypt or
 *                       result of the decryption (thus not encompassing the
 *                       additional data that are not encrypted).
 *
 * \return          \c 0 on success.
 * \return          MBEDTLS_ERR_CCM_BAD_INPUT on failure.
 */
int mbedtls_gcm_set_lengths(mbedtls_gcm_context *ctx, size_t total_ad_len, size_t plaintext_len);

#if defined(MBEDTLS_SELF_TEST)

/**
 * \brief          Alternative GCM checkup routine.
 *
 * \return         \c 0 on success.
 * \return         \c 1 on failure.
 */
int mbedtls_gcm_self_test_alt(int verbose);

#endif /* MBEDTLS_SELF_TEST */

#endif /* MBEDTLS_GCM_ALT */

#endif /* MBEDTLS_GCM_ALT_H */
