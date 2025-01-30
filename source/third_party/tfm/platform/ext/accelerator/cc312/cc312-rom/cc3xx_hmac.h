/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __CC3XX_HMAC_H__
#define __CC3XX_HMAC_H__

#include <stdint.h>
#include "cc3xx_error.h"
#include "cc3xx_hash.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The size in Bytes, i.e. B, associated to the HMAC block size
 *
 */
#define CC3XX_HMAC_BLOCK_SIZE (64)

/**
 * @brief Contains the state of the HMAC operation
 *
 */
struct cc3xx_hmac_state_t {
    uint8_t key[CC3XX_HMAC_BLOCK_SIZE];
    struct cc3xx_hash_state_t hash; /* Allows to restart low-level hash */
    cc3xx_hash_alg_t alg; /* Based on the hashing algorithm, sizes change */
} __attribute__((aligned(4)));

/**
 * @brief Sets the key for the HMAC operation on the state
 *
 * @param[out] state   A pointer to a state structure
 * @param[in] key      Buffer containing the key
 * @param[in] key_size Size in bytes of the buffer \param key
 * @param[in] alg      Underlying hashing algorithm
 * @return cc3xx_err_t
 */
cc3xx_err_t cc3xx_hmac_set_key(
    struct cc3xx_hmac_state_t *state,
    const uint8_t *key,
    size_t key_size,
    cc3xx_hash_alg_t alg);

/**
 * @brief Update the HMAC operation with a new chunk of data to authenticate
 *
 * @param[in,out] state   A pointer to a state structure
 * @param[in] data        Buffer containing the data to use for the update
 * @param[in] data_length Size in bytes of the buffer \param data
 * @return cc3xx_err_t
 */
cc3xx_err_t cc3xx_hmac_update(
    struct cc3xx_hmac_state_t *state,
    const uint8_t *data,
    size_t data_length);

/**
 * @brief Finalize the HMAC operation by producing the authentication tag
 *
 * @param[in,out] state A pointer to a state structure
 * @param[out] tag      Output buffer
 * @param[in] tag_size  Size in bytes of the buffer \param tag
 * @return cc3xx_err_t
 */
cc3xx_err_t cc3xx_hmac_finish(
    struct cc3xx_hmac_state_t *state,
    uint32_t *tag,
    size_t tag_size);

#ifdef __cplusplus
}
#endif

#endif /* __CC3XX_HMAC_H__ */
