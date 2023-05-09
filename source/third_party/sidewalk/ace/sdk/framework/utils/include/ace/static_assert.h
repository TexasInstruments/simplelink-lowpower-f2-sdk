/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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
 * @file static_assert.h
 *
 * @brief Portable compile time assertion
 *
 * @ingroup ACE_STATIC_ASSERT
 *
 */
#ifndef STATIC_ASSERT_H
#define STATIC_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

#define ACE_STATIC_ASSERT_FALLBACK(expr, msg, token) \
    __Static_assert(expr, msg, token)
#define __Static_assert(x, y, z) ___Static_assert(x, y, z)
#define ___Static_assert(x, y, z) typedef char __assert_##z[(x) ? 1 : -1]

/* Use the fallback by default... */
#define ACE_STATIC_ASSERT_MAPPING(x, y, z) ACE_STATIC_ASSERT_FALLBACK(x, y, z)

/* ... unless we have C11 support, in which case, use _Static_assert(). */
#if defined(__GNUC_PREREQ)
#if ((__GNUC_PREREQ(4, 6)) && (!defined(__cplusplus)))
#include <assert.h>
#undef ACE_STATIC_ASSERT_MAPPING
#define ACE_STATIC_ASSERT_MAPPING(x, y, z) _Static_assert(x, y)
#endif /* ((__GNUC_PREREQ(4, 6)) && (!defined(__cplusplus)))  */
#endif /* defined(__GNUC_PREREQ) */

/**
 *  @brief Compile time assertion.
 *
 *  This macro defines a portable implementation of static or compile time
 *  assertions. Given a compile time constant expression expr, use of this
 *  macro forces a compiler error if that expression evaluates to false at
 *  compile time. On C11 compliant implementations, a descriptive error is
 *  printed along with the string passed in as msg. On other compilers, an
 *  error '`size of array __assert_<t> is negative`' is printed where \<t\>
 *  is the token passed in. Care should be taken to ensure the token is a
 *  valid C identifier.
 *
 *  This macro can be used as follows:
 *  @code
 *  ACE_STATIC_ASSERT(sizeof(int) > sizeof(char), "That's a really small int",
 *                    int_gt_char);
 *  ACE_STATIC_ASSERT(5 * 1 == 5, "Multiplicative identity check fails",
 *                    one_is_identity);
 *  ACE_STATIC_ASSERT(5 * (2 = 3) == ((5 * 2) + (5 * 3)),
 *                    "Distributive property check fails",
 *                    distributivity_holds);
 *  @endcode
 *  @param[in] expr Expression to check
 *  @param[in] msg User friendly message describing the check
 *  @param[in] token Unique token
 *  @ingroup ACE_STATIC_ASSERT
 */
#define ACE_STATIC_ASSERT(expr, msg, token) \
    ACE_STATIC_ASSERT_MAPPING(expr, msg, token)

#ifdef __cplusplus
}
#endif

#endif /* STATIC_ASSERT_H */