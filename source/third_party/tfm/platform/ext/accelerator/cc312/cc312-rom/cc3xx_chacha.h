/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef CC3XX_CHACHA_H
#define CC3XX_CHACHA_H

#include "cc3xx_error.h"
#include "cc3xx_config.h"
#include "cc3xx_dma.h"
#include "cc3xx_poly1305.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CC3XX_CHACHA_KEY_SIZE 32

typedef enum {
    CC3XX_CHACHA_MODE_CHACHA = 0b0,
    CC3XX_CHACHA_MODE_CHACHA_POLY1305 = 0b1,
} cc3xx_chacha_mode_t;

typedef enum {
    CC3XX_CHACHA_DIRECTION_ENCRYPT = 0b0U,
    CC3XX_CHACHA_DIRECTION_DECRYPT = 0b1U,
} cc3xx_chacha_direction_t;

struct cc3xx_chacha_state_t {
    cc3xx_chacha_direction_t direction;
    cc3xx_chacha_mode_t mode;
    bool iv_is_96_bit;
    uint32_t key[8];

    size_t bytes_since_dma_output_addr_set;
    size_t crypted_len;
    size_t authed_len;

    uint64_t counter;
    uint32_t iv[3];

    struct cc3xx_dma_state_t dma_state;
    struct cc3xx_poly1305_state_t poly_state;
};

/**
 * @brief                        Initialize a CHACHA20 operation.

 * @param[in]  direction         Whether the operation should encrypt or decrypt.
 * @param[in]  mode              Which AES mode should be used.
 * @param[in]  key               Buffer containing the key material. Must be
 *                               CC3XX_CHACHA_KEY_SIZE in size.
 * @param[in]  initial_counter   The initial counter value.
 * @param[in]  iv                The CHACHA IV. May be either 8 or 12 bytes.
 * @param[in]  iv_len            The size of the IV input.
 *
 * @return                       CC3XX_ERR_SUCCESS on success, another
 *                               cc3xx_err_t on error.
 */
cc3xx_err_t cc3xx_chacha20_init(cc3xx_chacha_direction_t direction,
                                cc3xx_chacha_mode_t mode,
                                const uint32_t *key,
                                uint64_t initial_counter,
                                const uint32_t *iv, size_t iv_len);

/**
 * @brief                        Get the current state of the CHACHA operation.
 *                               Allows for restartable CHACHA operations.

 * @param[out] state            The cc3xx_chacha20_state_t to write the state
 *                              into.
 */
void cc3xx_chacha20_get_state(struct cc3xx_chacha_state_t *state);

/**
 * @brief                        Set the current state of the CHACHA operation.
 *                               Allows for restartable CHACHA operations.
 *
 * @note                         This funtion initializes the hardware, there is
 *                               no need to seperately call cc3xx_chacha20_init.

 * @param[in]  state            The cc3xx_chacha20_state_t to read the state
 *                              from.
 *
 * @return                       CC3XX_ERR_SUCCESS on success, another
 *                               cc3xx_err_t on error.
 */
cc3xx_err_t cc3xx_chacha20_set_state(const struct cc3xx_chacha_state_t *state);

/**
 * @brief                        Get the amount of bytes that have been output
 *
 * @return                       Amount of bytes of output that has been written
 *                               (which it not necessarily the same amount of
 *                               input that has been submitted, due to DMA
 *                               buffering)
 */
size_t cc3xx_chacha20_get_current_output_size(void);

/**
 * @brief                        Set the buffer that the CHACHA engine will
 *                               output into.
 *
 * @param[out] out               The buffer to output into.
 * @param[in]  out_len           The size of the buffer to output into. If this
 *                               is smaller than the size of the data passed to
 *                               cc3xx_chacha20_update, that function will fail
 *                               with an error.
 */
void cc3xx_chacha20_set_output_buffer(uint8_t *out, size_t out_len);

/**
 * @brief                        Input data to be encrypted/decrypted into an
 *                               CHACHA operation.

 * @param[in]  in                A pointer to the data to be input.
 * @param[in]  in_len            The size of the data to be input.
 *
 * @return                       CC3XX_ERR_SUCCESS on success, another
 *                               cc3xx_err_t on error.
 */
cc3xx_err_t cc3xx_chacha20_update(const uint8_t* in, size_t in_len);

/**
 * @brief                        Input data to be authenticated, but not
 *                               encrypted or decrypted into a CHACHA operation.
 *
 * @note                         This function is a no-op unless the mode is
 *                               CC3XX_CHACHA_MODE_CHACHA_POLY1305.
 *
 * @note                         This function must not be called after
 *                               cc3xx_chacha20_update has been called, until a
 *                               new operation is started.

 * @param[in]  in                A pointer to the data to be input.
 * @param[in]  in_len            The size of the data to be input.
 */
void cc3xx_chacha20_update_authed_data(const uint8_t* in, size_t in_len);

/**
 * @brief                        Finish a CHACHA operation. Calling this will
 *                               encrypt/decrypt the final data.
 *
 * @param[in,out]  tag           The buffer to write the tag into or read and
 *                               compare the tag from, depending on direction.
 *                               Can be NULL if not using
 *                               CC3XX_CHACHA_MODE_CHACHA_POLY1305.
 *
 * @param[out]     size          The size of the output that has been written.
 *
 * @return                       CC3XX_ERR_SUCCESS on success / tag comparison
 *                               succeeded, another cc3xx_err_t on error.
 */
cc3xx_err_t cc3xx_chacha20_finish(uint32_t *tag, size_t *size);

/**
 * @brief                       Uninitialize the CHACHA engine.
 *
 * @note                        The CHACHA engine is not implicitly
 *                              uninitialized on an error.
 *
 */
void cc3xx_chacha20_uninit(void);

#ifdef __cplusplus
}
#endif

#endif /* CC3XX_CHACHA_H */
