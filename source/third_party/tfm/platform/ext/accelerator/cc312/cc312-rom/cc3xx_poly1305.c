/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cc3xx_poly1305.h"

#include <assert.h>
#include <string.h>

static const uint32_t poly_key_r_mask[] = {
    0x0fffffff,
    0x0ffffffc,
    0x0ffffffc,
    0x0ffffffc
};

static const uint32_t poly_prime[] = {
    0xfffffffb,
    0xffffffff,
    0xffffffff,
    0xffffffff,
    0x3
};

static const uint32_t poly_barrett_tag[] = {
    0x00000000,
    0x00000000,
    0x00000080
};

#define POLY1305_PKA_SAVE_REG_AM 3

static struct cc3xx_poly1305_state_t poly_state;

const size_t reg_sizes_list[POLY1305_PKA_SAVE_REG_AM] = {
    sizeof(poly_state.key_r),
    sizeof(poly_state.key_s),
    sizeof(poly_state.accumulator)
};

static void poly1305_init_from_state(void)
{
    cc3xx_pka_set_modulus(poly_prime, sizeof(poly_prime),
                          poly_barrett_tag, sizeof(poly_barrett_tag));

    /* Before we use the bit 129 mask reg for the 129th bit or mask, use it for
     * the key_r mask.
     */
    cc3xx_pka_write_reg(poly_state.mask_reg, poly_key_r_mask,
                        sizeof(poly_key_r_mask));
    cc3xx_pka_and(poly_state.key_r_reg,
                  poly_state.mask_reg, poly_state.key_r_reg);
}


void cc3xx_poly1305_init(uint32_t *poly_key_r, uint32_t *poly_key_s)
{
    cc3xx_pka_init(sizeof(poly_prime));

    poly_state.key_r_reg = cc3xx_pka_allocate_reg();
    poly_state.key_s_reg = cc3xx_pka_allocate_reg();
    poly_state.accumulator_reg = cc3xx_pka_allocate_reg();
    poly_state.data_input_reg = cc3xx_pka_allocate_reg();
    poly_state.mask_reg = cc3xx_pka_allocate_reg();

    cc3xx_pka_write_reg(poly_state.key_r_reg, poly_key_r, POLY1305_KEY_SIZE);
    cc3xx_pka_write_reg(poly_state.key_s_reg, poly_key_s, POLY1305_KEY_SIZE);

    poly1305_init_from_state();
}

static void poly_process_block(const uint32_t *buf, size_t len)
{
    cc3xx_pka_write_reg(poly_state.data_input_reg, buf, len);

    /* Set the 129th bit to 1 */
    cc3xx_pka_set_to_power_of_two(poly_state.mask_reg, len * 8);
    cc3xx_pka_or(poly_state.data_input_reg,
                 poly_state.mask_reg, poly_state.data_input_reg);

    /* Add the new data to the accumulator */
    cc3xx_pka_mod_add(poly_state.accumulator_reg,
                      poly_state.data_input_reg, poly_state.accumulator_reg);
    /* Multiply the accumulator by r */
    cc3xx_pka_mod_mul(poly_state.accumulator_reg,
                      poly_state.key_r_reg, poly_state.accumulator_reg);
}

void cc3xx_poly1305_update(const uint8_t *buf, size_t length)
{
    size_t data_to_process_length;
    uint32_t temp_block[POLY1305_BLOCK_SIZE / sizeof(uint32_t)];
    size_t buffer_size_free =
        sizeof(poly_state.block_buf) - poly_state.block_buf_size_in_use;

    /* If there is data remaining in the buf, first fill and dispatch it */
    if (poly_state.block_buf_size_in_use != 0) {
        data_to_process_length =
            length < buffer_size_free ? length : buffer_size_free;

        memcpy(((uint8_t *)poly_state.block_buf) + poly_state.block_buf_size_in_use,
               buf, data_to_process_length);

        poly_state.block_buf_size_in_use += data_to_process_length;
        buf += data_to_process_length;
        length -= data_to_process_length;

        /* If this fills the buffer, dispatch it now (Unlike the DMA buffering,
         * we don't need to keep a block of data around for finalization).
         */
        if (poly_state.block_buf_size_in_use == POLY1305_BLOCK_SIZE) {
            poly_process_block(poly_state.block_buf, POLY1305_BLOCK_SIZE);
            poly_state.block_buf_size_in_use = 0;
        }
    }

    if (length == 0) {
        return;
    }

    /* Process all remaining full blocks */
    data_to_process_length = (length / POLY1305_BLOCK_SIZE) * POLY1305_BLOCK_SIZE;
    while (data_to_process_length > 0) {
        /* buf is uint8_t*, but PKA requires uint32_t*, so memcpy into the temp
         * buf to fix this.
         */
        memcpy(temp_block, buf, POLY1305_BLOCK_SIZE);
        poly_process_block(temp_block, POLY1305_BLOCK_SIZE);
        data_to_process_length -= POLY1305_BLOCK_SIZE;
        length -= POLY1305_BLOCK_SIZE;
        buf += POLY1305_BLOCK_SIZE;
    }

    /* If any data remains, push it into the block buffer */
    memcpy(poly_state.block_buf, buf, length);
    poly_state.block_buf_size_in_use += length;
}

void cc3xx_poly1305_get_state(struct cc3xx_poly1305_state_t *state)
{
    cc3xx_pka_reg_id_t save_reg_list[POLY1305_PKA_SAVE_REG_AM] = {
        poly_state.key_r_reg,
        poly_state.key_s_reg,
        poly_state.accumulator_reg
    };

    uint32_t *save_reg_ptr_list[POLY1305_PKA_SAVE_REG_AM] = {
        state->key_r,
        state->key_s,
        state->accumulator
    };

    memcpy(state, &poly_state, sizeof(*state));

    cc3xx_pka_get_state(&state->pka_state, POLY1305_PKA_SAVE_REG_AM, save_reg_list,
                        save_reg_ptr_list, reg_sizes_list);
}

void cc3xx_poly1305_set_state(const struct cc3xx_poly1305_state_t *state)
{
    cc3xx_pka_reg_id_t load_reg_list[POLY1305_PKA_SAVE_REG_AM] = {
        poly_state.key_r_reg,
        poly_state.key_s_reg,
        poly_state.accumulator_reg
    };

    const uint32_t *load_reg_ptr_list[POLY1305_PKA_SAVE_REG_AM] = {
        state->key_r,
        state->key_s,
        state->accumulator
    };

    memcpy(&poly_state, state, sizeof(poly_state));

    cc3xx_pka_set_state(&state->pka_state, POLY1305_PKA_SAVE_REG_AM, load_reg_list,
                        load_reg_ptr_list, reg_sizes_list);

    poly1305_init_from_state();
}

void cc3xx_poly1305_finish(uint32_t *tag)
{
    /* Flush the final block */
    if (poly_state.block_buf_size_in_use != 0) {
        poly_process_block(poly_state.block_buf, poly_state.block_buf_size_in_use);
    }

    /* Finally, the tag is a + s */
    cc3xx_pka_mod_add(poly_state.accumulator_reg,
                      poly_state.key_s_reg, poly_state.accumulator_reg);

    /* Read back the first 16 bytes for the accumulator into the tag */
    cc3xx_pka_read_reg(poly_state.accumulator_reg, tag, POLY1305_TAG_LEN);

    cc3xx_poly1305_uninit();
}

void cc3xx_poly1305_uninit(void)
{
    memset(&poly_state, 0, sizeof(poly_state));

    cc3xx_pka_uninit();
}
