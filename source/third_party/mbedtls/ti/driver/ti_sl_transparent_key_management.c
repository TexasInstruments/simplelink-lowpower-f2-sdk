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

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <mbedtls/private_access.h>
#include <psa/crypto.h>

/*
 *  ======== ti_sl_transparent_import_key ========
 */
psa_status_t ti_sl_transparent_import_key(const psa_key_attributes_t *attributes,
                                          const uint8_t *data,
                                          size_t data_length,
                                          uint8_t *key_buffer,
                                          size_t key_buffer_size,
                                          size_t *key_buffer_length,
                                          size_t *bits)
{
    psa_key_type_t keyType = psa_get_key_type(attributes);

    /* Transparent driver is not involved in validation of symmetric keys. */
    if (!PSA_KEY_TYPE_IS_ECC(keyType))
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    /* SL Driver uses ECC keys in in export format for input
     * Input can be directly copied to the buffer. */
    if (key_buffer_size < data_length)
    {
        return PSA_ERROR_BUFFER_TOO_SMALL;
    }
    memcpy(key_buffer, data, data_length);
    *key_buffer_length = data_length;

    /* Void unused args to prevent compiler warning */
    (void)bits;

    return PSA_SUCCESS;
}

/*
 *  ======== ti_sl_transparent_export_public_key ========
 */
psa_status_t ti_sl_transparent_export_public_key(const psa_key_attributes_t *attributes,
                                                 const uint8_t *key_buffer,
                                                 size_t key_buffer_size,
                                                 uint8_t *data,
                                                 size_t data_size,
                                                 size_t *data_length)
{
    psa_key_type_t keyType = psa_get_key_type(attributes);

    if (!PSA_KEY_TYPE_IS_ECC(keyType))
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    switch (keyType)
    {
        case PSA_WANT_KEY_TYPE_ECC_PUBLIC_KEY:
        case PSA_KEY_TYPE_ECC_KEY_PAIR_BASE:
            if (data_size < key_buffer_size)
            {
                return PSA_ERROR_BUFFER_TOO_SMALL;
            }
            memcpy(data, key_buffer, key_buffer_size);
            *data_length = key_buffer_size;
            return PSA_SUCCESS;
        default:
            return PSA_ERROR_NOT_SUPPORTED;
    }

    return PSA_SUCCESS;
}
