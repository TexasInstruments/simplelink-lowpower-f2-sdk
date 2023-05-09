/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_UTILS_H
#define SID_UTILS_H

#include <stdbool.h>

//TODO: Move this once we have a global internal config
#ifndef SID_BUILD_DEBUG
#error SID_BUILD_DEBUG is not defined
#endif

#ifndef SID_BUILD_RELEASE
#error SID_BUILD_RELEASE is not defined
#endif

#if SID_BUILD_DEBUG && SID_BUILD_RELEASE
#error Both SID_BUILD_DEBUG and SID_BUILD_RELEASE cannot be set at the same time
#endif

#if !SID_BUILD_DEBUG && !SID_BUILD_RELEASE
#error Both SID_BUILD_DEBUG and SID_BUILD_RELEASE cannot be unset at the same time
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef countof
#  undef countof
#endif

#ifdef __clang__
#define countof(array_) \
    (sizeof(array_)/sizeof(array_[0]))
#else
#define countof(array_) \
    (1 \
        ? sizeof(array_)/sizeof((array_)[0]) \
        : sizeof(struct { int do_not_use_countof_for_pointers : ((void*)(array_) == (void*)&array_);}) \
        )
#endif

#ifdef containerof
#  undef containerof
#endif

#define containerof(ptr, type, member) \
    ({ \
        const __typeof__(((type *)0)->member) * tmp_member_ = (ptr); \
        ((type *)((uintptr_t)(tmp_member_) - offsetof(type, member))); \
        })

#ifndef STR
#  define STR(a_)                           STR_(a_)
#  define STR_(a_)                          #a_
#endif

#ifndef CONCAT
#  define CONCAT_(a_, b_)                   a_ ## b_
#  define CONCAT(a_, b_)                    CONCAT_(a_,b_)
#endif

#define CONCAT3(a_, b_, c_)                 CONCAT(a_,CONCAT(b_,c_))
#define CONCAT4(a_, b_, c_, d_)             CONCAT(a_,CONCAT3(b_,c_,d_))
#define CONCAT5(a_, b_, c_, d_, e_)         CONCAT(a_,CONCAT4(b_,c_,d_,e_))
#define CONCAT6(a_, b_, c_, d_, e_, f_)     CONCAT(a_,CONCAT5(b_,c_,d_,e_,f_))
#define CONCAT7(a_, b_, c_, d_, e_, f_, h_) CONCAT(a_,CONCAT6(b_,c_,d_,e_,f_,h_))

#ifndef SID_CEIL_DIV
#  define SID_CEIL_DIV(a_,b_) \
    ({ \
        __typeof__(b_) b_tmp_ = (b_); \
        (a_ + b_tmp_ - 1) / b_tmp_; \
        })
#endif

#ifndef ROUND_DIV
#  define ROUND_DIV(a_,b_) \
    ({ \
        __typeof__(b_) b_tmp_ = (b_); \
        (a_ + b_tmp_ / 2) / b_tmp_; \
        })
#endif

#ifndef ABS
#  define ABS(a_) \
    ({ \
        __typeof__(a_) a_tmp_ = a_; \
        (a_tmp_ > 0 ? a_tmp_ : -a_tmp_); \
        })
#endif

#ifndef SWAP
#  define SWAP(a_,b_) \
    ({ \
        __typeof__(a_) a_tmp_ = (a_); \
        (a_) = (b_); \
        (b_) = a_tmp_; \
        })
#endif

#ifndef SID_MIN
#  define SID_MIN(a_,b_)                    ((a_) < (b_) ? (a_) : (b_))
#endif

#ifndef SID_MAX
#  define SID_MAX(a_,b_)                    ((a_) > (b_) ? (a_) : (b_))
#endif

/*
 * See
 * https://elixir.bootlin.com/zephyr/v2.1.0/source/include/sys/util.h#L232
 */
#ifndef IS_ENABLED
#define REPLACE_TRUE_WITH_COMMA1 COMMA_,
#define IS_ENABLED(option_) IS_ENABLED_(option_)
#define IS_ENABLED_(option_) IS_ENABLED__(REPLACE_TRUE_WITH_COMMA##option_)
#define IS_ENABLED__(true_or_false_option_) IS_ENABLED___(true_or_false_option_ true, false)
#define IS_ENABLED___(ignored, option_value, ...) option_value
#endif

#ifndef UNUSED
#  define UNUSED(x_)                         ((void)(x_))
#endif

#ifndef alignof
#  define alignof(x_)                        __alignof(x_)
#endif

#ifdef __cplusplus
}
#endif

#endif
