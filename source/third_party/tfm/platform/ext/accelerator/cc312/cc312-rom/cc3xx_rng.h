/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef CC3XX_RNG_H
#define CC3XX_RNG_H

#include "cc3xx_error.h"

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief                        Get random bytes from the CC3XX TRNG.
 *
 * @note                         This function may take a variable amount of
 *                               time to execute. This function may take a
 *                               considerable amount of time to execute if the
 *                               current TRNG entropy pool is depleted and more
 *                               entropy needs generating.
 *
 * @param[out] buf               Buffer to fill with random bytes.
 * @param[in] length             Size of the buffer.
 *
 * @return                       CC3XX_ERR_SUCCESS on success, another
 *                               cc3xx_err_t on error.
 */
cc3xx_err_t cc3xx_rng_get_random(uint8_t* buf, size_t length);

/**
 * @brief                        Get a random unsigned integer from the CC3XX
 *                               TRNG. The value is uniformly distributed
 *                               between 0 and bound - 1.
 *
 * @note                         This function may take a variable amount of
 *                               time to execute.
 *
 * @param[in]  bound             A value N such that 0 <= output < N
 * @param[out] uint              A pointer to the uint32_t to output into.
 *
 * @return                       CC3XX_ERR_SUCCESS on success, another
 *                               cc3xx_err_t on error.
 */
cc3xx_err_t cc3xx_rng_get_random_uint(uint32_t bound, uint32_t *uint);

#ifdef __cplusplus
}
#endif

#endif /* CC3XX_RNG_H */
