/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cc3xx_kdf.h"

#include "cc3xx_rng.h"
#include "cc3xx_stdlib.h"

#include <assert.h>

cc3xx_err_t cc3xx_kdf_cmac(cc3xx_aes_key_id_t key_id, const uint32_t *key,
                           cc3xx_aes_keysize_t key_size,
                           const uint8_t *label, size_t label_length,
                           const uint8_t *context, size_t context_length,
                           uint32_t *output_key, size_t out_length)
{
    uint32_t i_idx = 1;
    uint32_t l_total_length = out_length;
    uint8_t null_byte = 0;
    cc3xx_err_t err;
    uint32_t cmac_buf[AES_TAG_MAX_LEN / sizeof(uint32_t)];
    size_t output_idx;

    /* Check alignment */
    assert(((uintptr_t)output_key & 0b11) == 0);
    assert((out_length / sizeof(cmac_buf)) != 0);

    for(output_idx = 0; output_idx < out_length; output_idx += sizeof(cmac_buf)) {
        err = cc3xx_aes_init(CC3XX_AES_DIRECTION_ENCRYPT, CC3XX_AES_MODE_CMAC,
                             key_id, key, key_size, NULL, 0);
        if (err != CC3XX_ERR_SUCCESS) {
            goto out;
        }

        cc3xx_aes_set_tag_len(AES_TAG_MAX_LEN);

        cc3xx_aes_update_authed_data((uint8_t *)&i_idx, sizeof(i_idx));
        cc3xx_aes_update_authed_data(label, label_length);
        cc3xx_aes_update_authed_data(&null_byte, sizeof(null_byte));
        cc3xx_aes_update_authed_data(context, context_length);
        cc3xx_aes_update_authed_data((uint8_t *)&l_total_length,
                                     sizeof(l_total_length));
        cc3xx_aes_finish(cmac_buf, NULL);

        cc3xx_dpa_hardened_word_copy((void *)output_key + output_idx, cmac_buf,
                                     sizeof(cmac_buf) / sizeof(uint32_t));
        i_idx += 1;
    }

    err = CC3XX_ERR_SUCCESS;
out:
    if (err != CC3XX_ERR_SUCCESS) {
        cc3xx_rng_get_random((uint8_t *)output_key, out_length);
    }

    return err;
}
