/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

/*
 * @file utils_str.h
 */
#ifndef UTILS_STR_H
#define UTILS_STR_H

#include <stdint.h>
#include <stdlib.h>

// clang-format off
#define ACE_UTILS_U64TOA_BUFF_SIZE(radix) \
    ((radix == 2)  ? (64 + 1): \
     (radix == 3)  ? (41 + 1): \
     (radix == 4)  ? (32 + 1): \
     (radix == 5)  ? (28 + 1): \
     (radix == 6)  ? (25 + 1): \
     (radix == 7)  ? (23 + 1): \
     (radix == 8)  ? (22 + 1): \
     (radix == 9)  ? (21 + 1): \
     (radix == 10) ? (20 + 1): \
     (radix == 11) ? (19 + 1): \
     (radix == 12) ? (18 + 1): \
     (radix == 13) ? (18 + 1): \
     (radix == 14) ? (17 + 1): \
     (radix == 15) ? (17 + 1): \
      (16 + 1))
// clang-format on

/**
 * @brief      Converts an unsigned 64-bit integer to printable C string using
 *             the specified radix.
 *
 * @param[in]   val       Number to be converted/
 * @param[out]  str       The string result.
 * @param[in]   str_size  The size (in chars) of the buffer pointed to by str.
 * @param[in]   radix     Base of val; must be in the range 2-16.
 */
void aceUtils_u64toa(uint64_t val, char* str, size_t str_size, int radix);

#endif /* UTILS_STR_H */
