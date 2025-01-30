/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef CC3XX_POLY1305_H
#define CC3XX_POLY1305_H

#include "cc3xx_error.h"
#include "cc3xx_pka.h"

#include <stdint.h>
#include <stddef.h>

#define POLY1305_TAG_LEN 16
#define POLY1305_BLOCK_SIZE 16
#define POLY1305_KEY_SIZE 16
#define POLY1305_PKA_REG_SIZE 20

#ifdef __cplusplus
extern "C" {
#endif

struct cc3xx_poly1305_state_t {
    struct cc3xx_pka_state_t pka_state;

    cc3xx_pka_reg_id_t key_r_reg;
    cc3xx_pka_reg_id_t key_s_reg;
    cc3xx_pka_reg_id_t accumulator_reg;
    cc3xx_pka_reg_id_t data_input_reg;
    cc3xx_pka_reg_id_t mask_reg;

    uint32_t key_r[POLY1305_PKA_REG_SIZE / sizeof(uint32_t)];
    uint32_t key_s[POLY1305_PKA_REG_SIZE / sizeof(uint32_t)];
    uint32_t accumulator[POLY1305_PKA_REG_SIZE / sizeof(uint32_t)];

    uint32_t block_buf[POLY1305_BLOCK_SIZE / sizeof(uint32_t)];
    size_t block_buf_size_in_use;
};

/**
 * @brief                       Initialize the poly1305 operation.
 *
 * @param[in]  poly_key_r       This is the "r" portion of the poly1035 key (the
 *                              first half of the zero block). Must be
 *                              ``POLY1305_KEY_SIZE`` bytes in length.
 *
 * @param[in]  poly_key_s       This is the "s" portion of the poly1035 key (the
 *                              second half of the zero block). Must be
 *                              ``POLY1305_KEY_SIZE`` bytes in length.
 *
 */
void cc3xx_poly1305_init(uint32_t *poly_key_r, uint32_t *poly_key_s);

/**
 * @brief                        Input data into the poly1305 operation.

 * @param[in]  buf               A pointer to the data to be input.
 * @param[in]  length            The size of the data to be input.
 */
void cc3xx_poly1305_update(const uint8_t *buf, size_t length);

/**
 * @brief                        Get the current state of the poly1305 operation.
 *                               Allows for restartable poly1305 operations.

 * @param[out] state            The cc3xx_pooly1035_state_t to write the state
 *                              into.
 */
void cc3xx_poly1305_get_state(struct cc3xx_poly1305_state_t *state);

/**
 * @brief                        Set the current state of the poly1305 operation.
 *                               Allows for restartable poly1305 operations.
 *
 * @note                         This funtion initializes the operation, there
 *                               is no need to seperately call
 *                               cc3xx_poly1305_init.

 * @param[in]  state            The cc3xx_poly1305_state_t to read the state
 *                              from.
 */
void cc3xx_poly1305_set_state(const struct cc3xx_poly1305_state_t *state);

/**
 * @brief                        Finish a poly1305 operation, and write the tag
 *                               into the buffer.
 *
 * @param[out]  tag              The buffer to write the tag into.
 */
void cc3xx_poly1305_finish(uint32_t *tag);

/**
 * @brief                       Uninitialize the poly1305 operation.
 */
void cc3xx_poly1305_uninit(void);

#ifdef __cplusplus
}
#endif

#endif /* CC3XX_POLY1305_H */
