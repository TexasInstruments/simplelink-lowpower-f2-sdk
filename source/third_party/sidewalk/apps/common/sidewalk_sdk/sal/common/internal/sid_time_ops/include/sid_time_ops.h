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

#ifndef SID_TIME_OPS_H
#define SID_TIME_OPS_H

#include <sid_time_types.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Utility functions for timespec
 */

/* Add tm1 and tm2, set result in tm1 */
void sid_time_add(struct sid_timespec * tm1, const struct sid_timespec * tm2);

/* Subtract tm2 from tm1, set result in tm1 */
void sid_time_sub(struct sid_timespec * tm1, const struct sid_timespec * tm2);

/* Subtract tm2 from tm1, set result in delta */
void sid_time_delta(struct sid_timespec *delta, const struct sid_timespec *tm1, const struct sid_timespec *tm2);

/* Return tm1 > tm2 */
bool sid_time_gt(const struct sid_timespec * tm1, const struct sid_timespec * tm2);

/* Return tm1 < tm2 */
bool sid_time_lt(const struct sid_timespec * tm1, const struct sid_timespec * tm2);

/* Return tm1 == tm2 */
bool sid_time_eq(const struct sid_timespec * tm1, const struct sid_timespec * tm2);

/* Normalize timespec - move nsec to msec if applicable */
void sid_time_normalize(struct sid_timespec * tm);

/* Return whether tm is infinity */
bool sid_time_is_infinity(const struct sid_timespec * tm);

/* Return whether tm is zero */
bool sid_time_is_zero(const struct sid_timespec * tm);

/* Convert timespec to millisecond representation */
uint32_t sid_timespec_to_ms(const struct sid_timespec * tm);

/* Convert millseconds to timespec representation */
void sid_ms_to_timespec(uint32_t msec, struct sid_timespec *tm);

/* Convert timespec to millisecond representation of size 8 bytes */
uint64_t sid_timespec_to_ms_64(const struct sid_timespec * tm);

/* Convert millseconds of size 8 bytes to timespec representation */
void sid_ms_to_timespec_64(uint64_t msec, struct sid_timespec *tm);

/* Convert microseconds of size 4 bytes to timespec representation*/
void sid_us_to_timespec(uint32_t usec, struct sid_timespec *tm);

/* Convert microseconds of size 8 bytes to timespec representation*/
void sid_us_to_timespec_64(uint64_t usec, struct sid_timespec *tm);

/* Convert timespec to microseconds representation of size 4 bytes*/
uint32_t sid_timespec_to_us(const struct sid_timespec *tm);

/* Convert timespec to microseconds representation of size 8 bytes*/
uint64_t sid_timespec_to_us_64(const struct sid_timespec *tm);

#ifdef __cplusplus
}
#endif

#endif
