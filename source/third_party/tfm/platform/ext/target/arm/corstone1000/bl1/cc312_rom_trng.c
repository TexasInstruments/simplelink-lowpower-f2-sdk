/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "trng.h"
#include "cc3xx_rng.h"

int32_t bl1_trng_generate_random(uint8_t *output, size_t output_size)
{
    if (output_size == 0) {
        return 0;
    }

    if (output == NULL) {
        return -1;
    }

    return cc3xx_rng_get_random(output, output_size);
}

unsigned char fih_delay_random_uchar(void) {
    uint32_t out = 0;
    cc3xx_rng_get_random_uint(256, &out);
    return (unsigned char)out;
}
