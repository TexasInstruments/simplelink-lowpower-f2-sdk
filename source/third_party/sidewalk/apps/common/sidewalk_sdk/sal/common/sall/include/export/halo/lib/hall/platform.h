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

#ifndef HALO_HALL_PLATFORM_H_
#define HALO_HALL_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sid_time_types.h>
#include <halo/lib/hall/utils/platform.h>
#include <unistd.h>

#define SALL_MIN(n, m)    (((n) > (m)) ? (m) : (n))

/**
 * Function return system time into value referenced by pointer
 * @param __tp - reference to value to store system time
 * @return 0 - in success, or -1 if fails
 */
int clock__system_gettime(struct sid_timespec* __tp);

#define HALO_TIME_NSEC_IN_SEC 1000000000l
#define HALO_TIME_USEC_IN_SEC 1000000l
#define HALO_TIME_MSEC_IN_SEC 1000

static inline uint32_t timespec_to_ms(struct sid_timespec ts) {
    return ts.tv_sec * HALO_TIME_MSEC_IN_SEC + ts.tv_nsec / HALO_TIME_USEC_IN_SEC;
}

static inline int timespec_compare(const struct sid_timespec* lhs, const struct sid_timespec* rhs) {
    if (lhs->tv_sec != rhs->tv_sec) {
        return lhs->tv_sec < rhs->tv_sec ? -1 : 1;
    }
    return lhs->tv_nsec - rhs->tv_nsec;
}

static inline struct sid_timespec timespec_add_ms(struct sid_timespec ts, const uint32_t ms) {
    ts.tv_nsec += (ms % HALO_TIME_MSEC_IN_SEC) * HALO_TIME_USEC_IN_SEC;
    ts.tv_sec += ms / HALO_TIME_MSEC_IN_SEC + ts.tv_nsec / HALO_TIME_NSEC_IN_SEC;
    ts.tv_nsec = ts.tv_nsec % HALO_TIME_NSEC_IN_SEC;
    return ts;
}

/**
 * to enable ASSERTS in HALL should be defined
 *  #define HALL_USE_ASSERT
 * Automatically defined for Linux x86 builds in CMake file
 * For embedded platforms should be defined separately
 */
#ifdef HALL_USE_ASSERT
#include <sid_pal_assert_ifc.h>
#define HALL_ASSERT(X) SID_PAL_ASSERT((X))
#else
#define HALL_ASSERT(X)
#endif


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HALO_HALL_PLATFORM_H_ */
