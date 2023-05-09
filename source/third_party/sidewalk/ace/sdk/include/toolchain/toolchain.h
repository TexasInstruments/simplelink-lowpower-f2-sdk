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
/**
 * @file toolchain.h
 */
#ifndef TOOLCHAIN_H
#define TOOLCHAIN_H

#include <stdbool.h>

// clang-format off

/*
 * These let us probe toolchain capabilities at compile-time. Clang and newer
 * versions of GCC can use __has_builtin() and __has_attribute(). Other/older
 * toolchains will need to explicitly define their capabilities (e.g.
 * HAS_BUILTIN_____builtin_expect) based on the detected version.
 */
#if defined(__has_builtin)
    #define HAS_BUILTIN(func)  __has_builtin(func)
#else
    #define HAS_BUILTIN(func)   HAS_BUILTIN_##func
#endif

#if defined(__has_attribute)
    #define HAS_ATTRIBUTE(attr) __has_attribute(attr)
#else
    #define HAS_ATTRIBUTE(attr) HAS_ATTRIBUTE_##attr
#endif

/*
 * Define the capabilities of the current toolchain (e.g. built-in's),
 * and allow overriding of macros if needed.
 */
#if defined(__clang__)
    #include <toolchain/toolchain_clang.h>
// Do GCC last, as others (e.g. clang) also define __GNUC__.
#elif defined(__GNUC__)
    #include <toolchain/toolchain_gcc.h>
// If we can't detect the toolchain, the platform can provide a header at
// build-time via the PLATFORM_CFLAGS environment variable.
#else
    #include <toolchain/platform.h>
#endif

/*
 * Provide default implementations for each of the toolchain-dependent macros.
 *
 * For unsupported features, where sensible,  these can compile into nop's.
 * Otherwise, a build-time error must be emitted (i.e. #error).
 */

/**
 * @def   packed__
 * @brief Use the minimum required memory be used to represent the data type.
 */
#if !defined(packed__)
    #if HAS_ATTRIBUTE(__packed__)
        #define packed__ __attribute__((__packed__))
    #else
        // We can't silently turn this into a no-op, as we may be depending on
        // e.g. particular packed struct sizes.
        #error "No support for __packed__ attribute. Redefine packed__ if workaround available."
    #endif
#endif

/**
 * @def   aligned__(n)
 * @brief The minimum required alingment (in bytes) for the type/var.
 */
#if !defined(aligned__)
    #if HAS_ATTRIBUTE(__aligned__)
        #define aligned__(n) __attribute__((__aligned__(n)))
    #else
        // We can't silently turn this into a no-op, as it may cause alignment
        // issues on certain architectures.
        #error "No support for __aligned__ attribute. Redefine aligned__ if workaround available."
    #endif
#endif

/**
 * @def   deprecated__
 * @brief Indicate that the type/func/var is deprecated.
 * @note  May result in a warning if it is used anywhere in source code.
 */
#if !defined(deprecated__)
    #if HAS_ATTRIBUTE(deprecated)
        #define deprecated__ __attribute__((deprecated))
    #else
        #define deprecated__
    #endif
#endif

/**
 * @def   likely__(expr)
 * @brief Optimization hint that the provided expression is likely to occur.
 */
#if !defined(likely__)
    #if HAS_BUILTIN(__builtin_expect)
        #define likely__(expr) __builtin_expect((bool)!!(expr), true)
    #else
        #define likely__(expr) (expr)
    #endif
#endif

/**
 * @def   unlikely__(expr)
 * @brief Optimization hint that the provided expression is unlikely to occur.
 */
#if !defined(unlikely__)
    #if HAS_BUILTIN(__builtin_expect)
        #define unlikely__(expr) __builtin_expect((bool)!!(expr), false)
    #else
        #define unlikely__(expr) (expr)
    #endif
#endif

/**
 * @def   public_api__
 * @brief Indicate that the func should be publicly exported.
 */
#if !defined (public_api__)
    #if HAS_ATTRIBUTE(visibility)
        #define public_api__ __attribute__((__visibility__("default")))
    #else
        #define public_api__
    #endif
#endif

/**
 * @def private_api__
 * @brief Indicate that the func should not be publicly exported.
 */
#if !defined (private_api__)
    #if HAS_ATTRIBUTE(visibility)
        #define private_api__ __attribute__((__visibility__("hidden")))
    #else
        #define private_api__
    #endif
#endif

// clang-format on

#endif  // TOOLCHAIN_H
