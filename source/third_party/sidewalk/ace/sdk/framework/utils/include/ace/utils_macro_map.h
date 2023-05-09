/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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
 *
 * PORTIONS OF THIS FILE ARE BASED ON UTIL.H AVAILABLE AT: https://git.io/JvWFL
 * THE ORIGINAL FILE HEADER IS RETAINED BELOW.
 */
/*
 * Copyright (c) 2011-2014, Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file       utils_macro_map.h
 * @brief      Utility macro for providing map-like functionality during
 *             preprocessing.
 * @note       This gets it's own file, as there's a lot of helper macros needed
 *             to implement it, and so we don't want to clutter the main header.
 */
#ifndef UTILS_MACRO_MAP_H
#define UTILS_MACRO_MAP_H

/**
 * @addtogroup ACE_FRAMEWORK_UTILS
 * @{
 */

/**
 * @brief      Applies the given macro to each argument in the argument list.
 *
 * @param      ...   Macro name to be applied to the arguments, followed by
 *                   a list of arguments. The macro should have the following
 *                   form: `MACRO(arg)`.
 */
#define ACE_MACRO_MAP(...) MACRO_MAP_(__VA_ARGS__)

/** @} */

/*
 * Helper macros, used to implement the public ones below.
 */

#define MACRO_MAP_(...) \
    MACRO_MAP_N(NUM_VA_ARGS_LESS_1(__VA_ARGS__), __VA_ARGS__)
#define MACRO_MAP_N(N, ...) MACRO_MAP_N_(N, __VA_ARGS__)
#define MACRO_MAP_N_(N, ...) UTIL_CAT(MACRO_MAP_, N)(__VA_ARGS__, )

#define NUM_VA_ARGS_LESS_1_IMPL(                                               \
    _ignored, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
    _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, \
    _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, \
    _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, \
    _60, _61, _62, N, ...)                                                     \
    N

#define MACRO_MAP_0(...)
#define MACRO_MAP_1(m, a, ...) m(a)
#define MACRO_MAP_2(m, a, ...) m(a) MACRO_MAP_1(m, __VA_ARGS__, )
#define MACRO_MAP_3(m, a, ...) m(a) MACRO_MAP_2(m, __VA_ARGS__, )
#define MACRO_MAP_4(m, a, ...) m(a) MACRO_MAP_3(m, __VA_ARGS__, )
#define MACRO_MAP_5(m, a, ...) m(a) MACRO_MAP_4(m, __VA_ARGS__, )
#define MACRO_MAP_6(m, a, ...) m(a) MACRO_MAP_5(m, __VA_ARGS__, )
#define MACRO_MAP_7(m, a, ...) m(a) MACRO_MAP_6(m, __VA_ARGS__, )
#define MACRO_MAP_8(m, a, ...) m(a) MACRO_MAP_7(m, __VA_ARGS__, )
#define MACRO_MAP_9(m, a, ...) m(a) MACRO_MAP_8(m, __VA_ARGS__, )
#define MACRO_MAP_10(m, a, ...) m(a) MACRO_MAP_9(m, __VA_ARGS__, )
#define MACRO_MAP_11(m, a, ...) m(a) MACRO_MAP_10(m, __VA_ARGS__, )
#define MACRO_MAP_12(m, a, ...) m(a) MACRO_MAP_11(m, __VA_ARGS__, )
#define MACRO_MAP_13(m, a, ...) m(a) MACRO_MAP_12(m, __VA_ARGS__, )
#define MACRO_MAP_14(m, a, ...) m(a) MACRO_MAP_13(m, __VA_ARGS__, )
#define MACRO_MAP_15(m, a, ...) m(a) MACRO_MAP_14(m, __VA_ARGS__, )

#define NUM_VA_ARGS_LESS_1(...)                                              \
    NUM_VA_ARGS_LESS_1_IMPL(                                                 \
        __VA_ARGS__, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
        49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,  \
        32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,  \
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, ~)

#define UTIL_CAT(a, ...) UTIL_PRIMITIVE_CAT(a, __VA_ARGS__)
#define UTIL_PRIMITIVE_CAT(a, ...) a##__VA_ARGS__

#endif  // UTILS_MACRO_MAP_H
