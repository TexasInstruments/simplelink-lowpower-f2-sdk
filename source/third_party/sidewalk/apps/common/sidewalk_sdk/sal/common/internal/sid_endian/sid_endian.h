/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_ENDIAN_H
#define SID_ENDIAN_H

/** @file
 *
 * @defgroup sid_endian sid endian macros
 *
 * @details     Provides byte swap (endian) macros.
 */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__LITTLE_ENDIAN__) || (defined(BYTE_ORDER) && BYTE_ORDER == LITTLE_ENDIAN) \
    || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

/**
 * Converts from host byte order (32-bit) to network byte order.
 */
#define sid_htonl(hostlong) __builtin_bswap32(hostlong)

/**
 * Converts from host byte (16-bit) order to network byte order.
 */
#define sid_htons(hostshort) __builtin_bswap16(hostshort)

/**
 * Converts from network (32-bit) order to host byte order.
 */
#define sid_ntohl(netlong) __builtin_bswap32(netlong)

/**
 * Converts from network (16-bit) order to host byte order.
 */
#define sid_ntohs(netshort) __builtin_bswap16(netshort)

#else

#define sid_htonl(hostlong) (hostlong)
#define sid_htons(hostshort) (hostshort)
#define sid_ntohl(netlong) (netlong)
#define sid_ntohs(netshort) (netshort)

#endif

#ifdef __cplusplus
}
#endif

/** @} */

#endif
