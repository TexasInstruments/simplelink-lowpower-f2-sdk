/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef CC3XX_ENDIAN_HELPERS_H
#define CC3XX_ENDIAN_HELPERS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Conveniently, the compiler is smart enough to make this a single-instruction
 * endianness reversal where the architecture has such an instruction
 */
static inline uint32_t bswap_32(uint32_t in) {
    uint32_t out = 0;

    out |= (in & 0xFF000000) >> 24;
    out |= (in & 0x00FF0000) >> 8;
    out |= (in & 0x0000FF00) << 8;
    out |= (in & 0x000000FF) << 24;

    return out;
}

static inline uint64_t bswap_64(uint64_t in) {
    uint64_t out = 0;

    out |= (in & 0xFF00000000000000) >> 56;
    out |= (in & 0x00FF000000000000) >> 40;
    out |= (in & 0x0000FF0000000000) >> 24;
    out |= (in & 0x000000FF00000000) >> 8;
    out |= (in & 0x00000000FF000000) << 8;
    out |= (in & 0x0000000000FF0000) << 24;
    out |= (in & 0x000000000000FF00) << 40;
    out |= (in & 0x00000000000000FF) << 56;

    return out;
}

#ifdef __cplusplus
}
#endif

#endif /* CC3XX_ENDIAN_HELPERS_H */
