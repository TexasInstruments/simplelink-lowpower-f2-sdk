/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef HALO_LIB_HALO_EXPORT_HALO_UTILS_H
#define HALO_LIB_HALO_EXPORT_HALO_UTILS_H

#ifdef countof
#  undef countof
#endif

#define countof(array_) \
    (1 \
        ? sizeof(array_)/sizeof((array_)[0]) \
        : sizeof(struct { int do_not_use_countof_for_pointers : ((void*)(array_) == (void*)&array_);}) \
        )

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

#ifndef HALO_CEIL_DIV
#  define HALO_CEIL_DIV(a_,b_) \
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

#ifndef UNUSED
#  define UNUSED(x_)                         ((void)(x_))
#endif

#ifndef alignof
#  define alignof(x_)                        __alignof(x_)
#endif

#endif /* !HALO_LIB_HALO_EXPORT_HALO_UTILS_H */
