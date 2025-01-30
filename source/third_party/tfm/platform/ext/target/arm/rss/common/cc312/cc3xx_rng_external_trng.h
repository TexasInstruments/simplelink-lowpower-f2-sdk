/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __CC3XX_RNG_EXTERNAL_TRNG_H__
#define __CC3XX_RNG_EXTERNAL_TRNG_H__

#include "cc3xx_error.h"

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

cc3xx_err_t rng_get_random(uint8_t* buf, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* __CC3XX_RNG_EXTERNAL_TRNG_H__ */
