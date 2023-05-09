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
 */
#ifndef TOOLCHAIN_H
#error "Don't include toolchain_gcc.h directly. Use toolchain/toolchain.h instead."
#endif
#ifndef TOOLCHAIN_GCC_H
#define TOOLCHAIN_GCC_H

// clang-format off

/*
 * Compatability checks.
 *
 * These were tested using godbolt.org, with the oldest version of GCC we
 * checked against being 4.1.2 (Feb 2007). If it works with 4.1.2, we assume it
 * works for everyone.
 */

// This allows for easier version checks (e.g. GCC_VERSION >= 50000 ensures
// we're on at least 5.0.0).
#if !defined(GCC_VERSION)
    #define GCC_VERSION (__GNUC__ * 10000 \
                        + __GNUC_MINOR__ * 100 \
                        + __GNUC_PATCHLEVEL__)
#endif

#if GCC_VERSION <= 50000
    // These all exist in GCC 4.1.2.
    #if !defined(HAS_ATTRIBUTE___packed__)
        #define HAS_ATTRIBUTE___packed__ 1
    #endif
    #if !defined(HAS_ATTRIBUTE___aligned__)
        #define HAS_ATTRIBUTE___aligned__ 1
    #endif
    #if !defined(HAS_ATTRIBUTE_deprecated)
        #define HAS_ATTRIBUTE_deprecated 1
    #endif
    #if !defined(HAS_ATTRIBUTE_visibility)
        #define HAS_ATTRIBUTE_visibility 1
    #endif
#endif

#if !defined(HAS_BUILTIN___builtin_types_compatible_p)
    #define HAS_BUILTIN___builtin_types_compatible_p 1
#endif

#if !defined(HAS_BUILTIN_____builtin_expect)
    #if GCC_VERSION >= 40102
        #define HAS_BUILTIN_____builtin_expect 1
    #endif
#endif

// clang-format on

#endif  // TOOLCHAIN_GCC_H
