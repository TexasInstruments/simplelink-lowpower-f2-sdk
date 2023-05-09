/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_RNG_H
#define SID_RNG_H

#include <sid_error.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initializes the random number generator. This API
 * must be called before sid_rng_get() and sid_rng_get_range()
 *
 * @retval  SID_ERROR_NONE
 */
sid_error_t sid_rng_init(void);

/**
 * Get a random number
 *
 * IMPORTANT: this API may be called within ISR context
 *
 * @param[in] range of random number returned
 *
 * @retval none
 */
uint32_t sid_rng_get(void);

/**
 * Get a random number between 0 and range - 1
 *
 * IMPORTANT: this API may be called within ISR context
 *
 * @param[in] range of random number returned
 *
 * @retval none
 */
uint16_t sid_rng_get_range(uint16_t range);

/**
 * Get a random number between 0 and range - 1 using crypto RNG
 *
 * IMPORTANT: this API must not be called within ISR context
 *
 * @param[in] range of random number returned.
 *            The range must never be 0 or the output will not be random.
 *
 * @retval random number
 *         If the range is 0, retval will be 0.
 */
uint32_t sid_rng_get_range_crypto_safe(uint32_t range);

#ifdef __cplusplus
}
#endif

#endif
