/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef CC3XX_HASH_H
#define CC3XX_HASH_H

#include "cc3xx_error.h"
#include "cc3xx_dma.h"

#include <stdint.h>
#include <stddef.h>

#define SHA1_OUTPUT_SIZE (20)
#define SHA224_OUTPUT_SIZE (28)
#define SHA256_OUTPUT_SIZE (32)

typedef enum {
    CC3XX_HASH_ALG_SHA1  =  0b0001U,
    CC3XX_HASH_ALG_SHA224 = 0b1010U,
    CC3XX_HASH_ALG_SHA256 = 0b0010U,
} cc3xx_hash_alg_t;

/**
 * @brief Macro that returns the length of the hash associated to the
 *        algorithm value passed as input
 */
#define CC3XX_HASH_LENGTH(alg)                              \
    (                                                       \
        alg == CC3XX_HASH_ALG_SHA1 ? SHA1_OUTPUT_SIZE :     \
        alg == CC3XX_HASH_ALG_SHA224 ? SHA224_OUTPUT_SIZE : \
        alg == CC3XX_HASH_ALG_SHA256 ? SHA256_OUTPUT_SIZE : \
        0                                                   \
    )

struct cc3xx_hash_state_t {
    cc3xx_hash_alg_t alg;
    uint64_t curr_len;
    uint32_t hash_h[8];
    struct cc3xx_dma_state_t dma_state;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief                        Initialize a hash operation.
 *
 * @param[in]  alg               Which hash algorithm should be initialized.
 *
 * @return                       CC3XX_ERR_SUCCESS on success, another
 *                               cc3xx_err_t on error.
 */
cc3xx_err_t cc3xx_hash_init(cc3xx_hash_alg_t alg);

/**
 * @brief                        Input data into a hash operation.
 *
 * @param[in]  buf               A pointer to the data to be input.
 * @param[in]  length            The size of the data to be input.
 *
 * @return                       CC3XX_ERR_SUCCESS on success, another
 *                               cc3xx_err_t on error.
 */
cc3xx_err_t cc3xx_hash_update(const uint8_t *buf, size_t length);

/**
 * @brief                        Get the current state of the hash operation.
 *                               Allows for restartable hash operations.
 *
 * @param[out] state             The cc3xx_hash_state_t to write the state into.
 */
void cc3xx_hash_get_state(struct cc3xx_hash_state_t *state);

/**
 * @brief                        Set the current state of the hash operation.
 *                               Allows for restartable hash operations.
 *
 * @note                         This function initializes the hardware, there is
 *                               no need to separately call cc3xx_hash_init.
 *
 * @param[in]  state             The cc3xx_hash_state_t to read the state from.
 */
void cc3xx_hash_set_state(const struct cc3xx_hash_state_t *state);

/**
 * @brief                        Finish a hash operation, and output the hash.
 *
 * @param[out]  res              The result of the hash operation.
 * @param[in]   length           The size of the result buffer. Must match the
 *                               hash output size. Checked by assert only.
 */
void cc3xx_hash_finish(uint32_t *res, size_t length);

/**
 * @brief                        Uninitialize the hash engine.
 *
 * @note                         The hash engine is not implicitly uninitialized
 *                               on an error.
 *
 */
void cc3xx_hash_uninit(void);

#ifdef __cplusplus
}
#endif

#endif /* CC3XX_HASH_H */
