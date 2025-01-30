/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef CC3XX_AES_EXTERNAL_KEY_LOADER_H
#define CC3XX_AES_EXTERNAL_KEY_LOADER_H

#include "cc3xx_error.h"
#include "cc3xx_aes.h"

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

cc3xx_err_t set_key(cc3xx_aes_key_id_t key_id, const uint32_t *key,
                    cc3xx_aes_keysize_t key_size, bool is_tun1);

#ifdef __cplusplus
}
#endif

#endif /* CC3XX_AES_EXTERNAL_KEY_LOADER_H */
