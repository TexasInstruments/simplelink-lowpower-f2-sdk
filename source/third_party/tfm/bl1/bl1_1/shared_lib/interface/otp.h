/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef BL1_1_OTP_H
#define BL1_1_OTP_H

#include <stdint.h>
#include <stddef.h>

#include "fih.h"
#include "crypto_key_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BL1_2_HASH_SIZE
#define BL1_2_HASH_SIZE 32
#endif /* BL1_2_HASH_SIZE */

#ifndef BL2_HASH_SIZE
#define BL2_HASH_SIZE 32
#endif /* BL1_2_HASH_SIZE */

#ifndef TFM_OTP_NV_COUNTER_NUMBER
#define TFM_OTP_NV_COUNTER_NUMBER 4u
#endif /* !TFM_OTP_NV_COUNTER_NUMBER */

#ifndef TFM_OTP_NV_COUNTER_SIZE_IN_BITS
#define TFM_OTP_NV_COUNTER_SIZE_IN_BITS 32u
#endif /* !TFM_OTP_NV_COUNTER_SIZE_IN_BITS */

/* Use ceiling division so we always have at least the correct amount of bits */
#define TFM_OTP_NV_COUNTER_BYTES ((TFM_OTP_NV_COUNTER_SIZE_IN_BITS + 7) / 8)

enum tfm_bl1_nv_counter_id_t {
    BL1_NV_COUNTER_ID_BL2_IMAGE,
};

/* Load the key with the given ID into the key buf */
fih_int bl1_otp_read_key(enum tfm_bl1_key_id_t key_id, uint8_t *key_buf);

#ifdef __cplusplus
}
#endif

#endif /* BL1_1_OTP_H */
