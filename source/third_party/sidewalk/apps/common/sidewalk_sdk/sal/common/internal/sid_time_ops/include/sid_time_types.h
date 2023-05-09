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

#ifndef SID_TIME_TYPES_H
#define SID_TIME_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SID_TIME_NSEC_PER_SEC   ((uint32_t)1000000000UL)
#define SID_TIME_USEC_PER_SEC   ((uint32_t)1000000UL)
#define SID_TIME_USEC_PER_MSEC  ((uint32_t)1000UL)
#define SID_TIME_MSEC_PER_SEC   ((uint32_t)1000UL)
#define SID_TIME_NSEC_PER_USEC   ((uint32_t)1000UL)
#define SID_TIME_NSEC_PER_MSEC   ((uint32_t)1000000UL)

#define SID_TIME_SEC_PER_MINUTE    ((uint32_t)60UL)
#define SID_TIME_MINUTE_PER_HOUR   ((uint32_t)60UL)
#define SID_TIME_HOUR_PER_DAY      ((uint32_t)24UL)
#define SID_TIME_SEC_PER_DAY       ((uint32_t)SID_TIME_HOUR_PER_DAY * SID_TIME_MINUTE_PER_HOUR * SID_TIME_SEC_PER_MINUTE)

typedef uint32_t sid_time_t;

struct sid_timespec {
    sid_time_t tv_sec; // seconds
    uint32_t tv_nsec; // nanoseconds
};

static const struct sid_timespec SID_TIME_INFINITY = { .tv_sec = UINT32_MAX, .tv_nsec = UINT32_MAX};
static const struct sid_timespec SID_TIME_ZERO     = {};

#ifdef __cplusplus
}
#endif

#endif
