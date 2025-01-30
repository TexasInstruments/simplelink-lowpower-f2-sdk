
/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef CC3XX_KDF_H
#define CC3XX_KDF_H

#include "cc3xx_error.h"
#include "cc3xx_aes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 */
/**
 * @brief                        This is a counter-mode KDF complying with NIST
 *                               SP800-108 where the PRF is a the AES cipher
 *                               using the CMAC mode of operation as per NIST
 *                               SP800-38B.
 *
 * @note                         This funtion only outputs keys whose size is a
 *                               multiple of the output size of the AES-CMAC
 *                               operation (16 bytes).
 *
 * @param[in]  key_id            Which user/hardware key should be used.
 * @param[in]  key               If key_id is set to CC3XX_AES_KEY_ID_USER_KEY,
 *                               this buffer contains the key material.
 * @param[in]  key_size          The size of the key being used.
 * @param[in]  label             The label to input into the derivation
 *                               operation.
 * @param[in]  label_length      The length of the label.
 * @param[in]  context           The context to input into the derivation
 *                               operation.
 * @param[in]  context_length    The length of the context.
 * @param[out] output_key        The buffer to output the key into.
 * @param[in] out_length         The size of the key to derive.
 *
 * @return                       CC3XX_ERR_SUCCESS on success, another
 *                               cc3xx_err_t on error.
 */
cc3xx_err_t cc3xx_kdf_cmac(cc3xx_aes_key_id_t key_id, const uint32_t *key,
                           cc3xx_aes_keysize_t key_size,
                           const uint8_t *label, size_t label_length,
                           const uint8_t *context, size_t context_length,
                           uint32_t *output_key, size_t out_length);

#ifdef __cplusplus
}
#endif

#endif /* CC3XX_KDF_H */
