/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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
 * @file osal_common.h
 * @brief Common defines, types, etc for use within the OSAL.
 * @private
 */
#ifndef OSAL_COMMON_H
#define OSAL_COMMON_H

/**
 * @deprecated DO NOT USE THESE.
 * @{
 */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ACE_NULL (0)
#define ACE_NULL_PTR ((void*)0)

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

/** @} */

#define RETURN_IF_NOT_OK(retval)                      \
    do {                                              \
        if ((retval != ACE_STATUS_OK)) return retval; \
    } while (0)

#define CHECK_AND_RETURN(expr, retval) \
    do {                               \
        if (!(expr)) return (retval);  \
    } while (0)

#ifdef ACE_ASSERT_BAD_PARAMS
#define CHECK_PARAMS(expr) \
    do {                   \
        assert((expr));    \
    } while (0)
#else
#define CHECK_PARAMS(expr) CHECK_AND_RETURN((expr), ACE_STATUS_BAD_PARAM)
#endif  // ACE_ASSERT_BAD_PARAMS

#ifdef __cplusplus
}
#endif

#endif /* OSAL_COMMON_H */
