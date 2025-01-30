/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */


#include "trng.h"
#include "cc3xx_rng_external_trng.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>

static uint32_t otp_trng_get_random_uint32_t(void)
{
    const volatile uint32_t *rng_ptr = (uint32_t *)0x500A5270;
    return *rng_ptr;
}

int32_t bl1_trng_generate_random(uint8_t *output, size_t output_size)
{
    size_t copy_size;
    uint32_t random_word;

    random_word = otp_trng_get_random_uint32_t();
    copy_size = sizeof(uint32_t) - ((uintptr_t)output % sizeof(uint32_t));
    copy_size = output_size < copy_size ? output_size : copy_size;
    memcpy(output, &random_word, copy_size);
    output += copy_size;
    output_size -= copy_size;

    while(output_size >= sizeof(uint32_t)) {
        *((uint32_t *)output) = otp_trng_get_random_uint32_t();
        output += sizeof(uint32_t);
        output_size -= sizeof(uint32_t);
    }

    random_word = otp_trng_get_random_uint32_t();
    copy_size = output_size % sizeof(uint32_t);
    memcpy(output, &random_word, copy_size);

    return 0;
}

cc3xx_err_t rng_get_random(uint8_t* buf, size_t length)
{
    bl1_trng_generate_random(buf, length);
    return CC3XX_ERR_SUCCESS;
}

unsigned char fih_delay_random_uchar(void)
{
    return (uint8_t)otp_trng_get_random_uint32_t();
}
