/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef INCLUDE_APPV1_WRAPPERS_GENERIC_H_
#define INCLUDE_APPV1_WRAPPERS_GENERIC_H_

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef STR
#define STR(a_) STR_(a_)
#define STR_(a_) #a_
#endif

#ifndef CONCAT
#define CONCAT_(a_, b_) a_##b_
#define CONCAT(a_, b_) CONCAT_(a_, b_)
#define CONCAT3(a_, b_, c_) CONCAT(a_, CONCAT(b_, c_))
#define CONCAT4(a_, b_, c_, d_) CONCAT(a_, CONCAT3(b_, c_, d_))
#define CONCAT5(a_, b_, c_, d_, e_) CONCAT(a_, CONCAT4(b_, c_, d_, e_))
#define CONCAT6(a_, b_, c_, d_, e_, f_) CONCAT(a_, CONCAT5(b_, c_, d_, e_, f_))
#define CONCAT7(a_, b_, c_, d_, e_, f_, h_) CONCAT(a_, CONCAT6(b_, c_, d_, e_, f_, h_))
#endif

#define ARGS_TABLE(a, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, ret_value__, ...) ret_value__
#define ARGS_COUNT(a, ...) ARGS_TABLE(a, ##__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define VA_COUNT_ARGS(a, ...) ARGS_COUNT(a, ##__VA_ARGS__)

static const size_t size = ARGS_COUNT(1);
static const size_t size_array[] = {ARGS_COUNT(1, 2, 3, 4)};

// STATIC_ASSERT(VA_COUNT_ARGS(a) == 0);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1) == 1);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2) == 2);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3) == 3);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3, 4) == 4);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3, 4, 5) == 5);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3, 4, 5, 6) == 6);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3, 4, 5, 6, 7) == 7);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3, 4, 5, 6, 7, 8) == 8);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3, 4, 5, 6, 7, 8, 9) == 9);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10) == 10);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11) == 11);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12) == 12);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13) == 13);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 70) == 14);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 70) == 15);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 70) == 16);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 70) == 17);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 70) == 18);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 70) == 19);
// STATIC_ASSERT(VA_COUNT_ARGS(a, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 70) == 70);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INCLUDE_APPV1_WRAPPERS_GENERIC_H_ */
