/*
 * Copyright (c) 2024, Texas Instruments Incorporated
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

#ifndef TI_SL_TRANSPARENT_DRIVER_ENTRYPOINTS_H
#define TI_SL_TRANSPARENT_DRIVER_ENTRYPOINTS_H

#include <stdint.h>
#include <stddef.h>
#include <psa/crypto_types.h>

#if defined(ENABLE_TI_SL_MBEDTLS_PSA_DRIVER)
    #ifndef PSA_CRYPTO_ACCELERATOR_DRIVER_PRESENT
        #define PSA_CRYPTO_ACCELERATOR_DRIVER_PRESENT
    #endif /* PSA_CRYPTO_ACCELERATOR_DRIVER_PRESENT */
#endif     /* ENABLE_TI_SL_MBEDTLS_PSA_DRIVER */

/* Key Management functions */
/** Import key.
 *
 * \param[in]  attributes           The attributes of the key to use for the
 *                                  operation.
 * \param[in]  data                 The raw key material. For private keys
 *                                  this must be a big-endian integer of 32
 *                                  bytes; for public key this must be an
 *                                  uncompressed ECPoint (65 bytes).
 * \param[in]  data_length          The size of the raw key material.
 * \param[out] key_buffer           The buffer to contain the key data in
 *                                  output format upon successful return.
 * \param[in]  key_buffer_size      Size of the \p key_buffer buffer in bytes.
 * \param[out] key_buffer_length    The length of the data written in \p
 *                                  key_buffer in bytes.
 * \param[out] bits                 The bitsize of the key.
 *
 * \retval  #PSA_SUCCESS
 *          Success. Key stored in \p key_buffer.
 * \retval  #PSA_ERROR_NOT_SUPPORTED
 *          The input is not supported by this driver (not SECP256R1).
 * \retval  #PSA_ERROR_BUFFER_TOO_SMALL
 *          \p key_buffer_size is too small.
 */
psa_status_t ti_sl_transparent_import_key(const psa_key_attributes_t *attributes,
                                          const uint8_t *data,
                                          size_t data_length,
                                          uint8_t *key_buffer,
                                          size_t key_buffer_size,
                                          size_t *key_buffer_length,
                                          size_t *bits);

/** Export public key.
 * This function directly exports the public key from key storage.
 *
 * \param[in]  attributes           The attributes of the key to use for the
 *                                  operation.
 * \param[in]  key_buffer           The public key in the export format.
 * \param[in]  key_buffer_size      The size of the public key in bytes.
 * \param[out] data                 The buffer to contain the public key in
 *                                  the export format upon successful return.
 * \param[in]  data_size            The size of the \p data buffer in bytes.
 * \param[out] data_length          The length written to \p data in bytes.
 *
 * \retval  #PSA_SUCCESS
 *          Success. Public key retrievd from key storage.
 * \retval  #PSA_ERROR_NOT_SUPPORTED
 *          The input is not supported by this driver (not SECP256R1).
 * \retval  #PSA_ERROR_BUFFER_TOO_SMALL
 *          \p key_buffer_size is too small.
 */
psa_status_t ti_sl_transparent_export_public_key(const psa_key_attributes_t *attributes,
                                                 const uint8_t *key_buffer,
                                                 size_t key_buffer_size,
                                                 uint8_t *data,
                                                 size_t data_size,
                                                 size_t *data_length);

#endif /* TI_SL_TRANSPARENT_DRIVER_ENTRYPOINTS_H */
