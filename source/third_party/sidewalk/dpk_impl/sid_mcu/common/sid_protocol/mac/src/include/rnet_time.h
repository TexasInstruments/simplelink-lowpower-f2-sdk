/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef HALO_LIB_RINGNET_WAN_SRC_INCLUDE_RNET_TIME_H_
#define HALO_LIB_RINGNET_WAN_SRC_INCLUDE_RNET_TIME_H_

#include <stdbool.h>

#include <sid_time_types.h>

#include <sid_protocol_defs.h>


typedef struct {
    U32 secs;
    U16 tu;
} time_def;

#define SIZE_OF_TIME_DEF                6
#ifndef TU_WRAP
#define TU_WRAP(val)                    ((val)&0xFFFFFF)
#endif

#define RTC2_FREQUENCY                  32768UL
#define TUS_IN_SEC                      RTC2_FREQUENCY

#define MS_TO_MICRO_SEC(X)              ((X)*1000)
#define MS_TO_NANO_SEC(X)               ((U64)(X)*1000000ULL)

#define TUS_TO_SECS(X)                  ((U32)(X)/TUS_IN_SEC)
#define TUS_TO_MS(X)                    ((U64)(X)*1000UL/TUS_IN_SEC)
#define TUS_TO_MICRO_SEC(X)             ((U64)(X)*1000000ULL/TUS_IN_SEC)
#define TUS_TO_NS(X)                    ((U64)(X)*1000000000ULL/TUS_IN_SEC)

#define MS_TO_TUS(X)                    ((U64)(X)*TUS_IN_SEC/1000UL)
#define MICRO_SEC_TO_TUS(X)             ((U64)(X)*TUS_IN_SEC/1000000ULL)
#define NS_TO_TUS(X)                    ((U64)(X)*TUS_IN_SEC/1000000000ULL)

#define MS_TO_TIMEDEF_INITIALIZER(X)    ((time_def){(X) / 1000, MS_TO_TUS((X) % 1000)})
#define TUS_TO_TIMEDEF_INITIALIZER(X)   ((time_def){(X) / TUS_IN_SEC, (X) % TUS_IN_SEC})

#define TUS_COUNT_HALF_RANGE 8388608UL // Half range of 24 bit tu time for rollover comparison
#define TIMEDEF_HALF_RANGE              70368744177664ULL //Half range of RTC 47bit counter for rollover comparision
#define TIMEDEF_RANGE                   ((((uint64_t)1)<<47) - 1ULL) //Full range of timedef 47bit counter
#define TUS_MS_HALF_RANGE TUS_TO_MS(TUS_COUNT_HALF_RANGE) //Half range of 24 bit tu time in ms for rollover comparison

static const time_def TIME_DEF_INFINITY = {.secs = ~0, .tu = ~0};
static const time_def TIME_DEF_ZERO     = {.secs = 0, .tu = 0};

#define GPS_TO_UNIX_EPOCH_SECS 315964800UL
#define GPS_LEAP_SECONDS 18

/**
 * @brief Get current uptime in tick
 */
uint32_t rnet_get_current_tu(void);

/**
 * @brief Get current uptime in time_def format
 */
void rnet_get_current_timedef(time_def *tdp);

#define RNET_CURRENT_TUS                        rnet_get_current_tu()
#define RNET_GET_TIMEDEF(X)                     rnet_get_current_timedef(X)

/**
 * @brief Convert timespec to ticks
 */
U64 TimeSpecToTicks(const struct sid_timespec *tm1);

/**
 * @brief Convert ticks to timespec
 */
void TicksToTimeSpec(struct sid_timespec *tm, U32 tu);

/**
 * @brief Convert sid_timespec to time_def
 */
void TimeSpecToTimeDef(time_def *tdp, const struct sid_timespec *tsp );

/**
 * @brief Convert time_def to timespec
 */
void TimeDefToTimeSpec(struct sid_timespec *tsp, const time_def *tdp );

/**
 * @brief Convert timespec to time_def
 */
time_def TimeSpecToTimeDefCopy(struct sid_timespec ts);

/**
 * @brief Add time_def. tm1 = tm2 + tm3
 */
void AddTimeDefs(time_def *tm1, const time_def *tm2, const time_def *tm3);

/**
 * @brief Substract time_def. tm1 = tm2 - tm3
 *        tm2 needs to be lager than tm3
 */
void SubTimeDefs(time_def *tm1, const time_def *tm2, const time_def *tm3);

/**
 * @brief Clear time_def.
 *
 */
void ClearTimeDef(time_def *tm1);

/**
 * @brief Compare time_def.
 *
 * @return  true if tm1 >= tm2
 */
bool TimeDefIsGt(const time_def *tm1, const time_def *tm2);

/**
 * @brief Convert time_def to ticks.
 *
 */
U64 TimeDefToTicks(const time_def *tm1);

/**
 * @brief Convert ticks to time_def.
 *
 */
void TicksToTimeDef(time_def *tm, U32 t_tu);

/**
 * @brief Return true if time_def is infinity
 *
 */
bool TimeDefsIsInfinity(const time_def *tm);

/**
 * @brief Compares time defs.
 *
 * @return  true if tm1 > tm2 and between tm1 & tm2 < half range of time def
 */
bool IsLargerTimeDefs(const time_def *tm1, const time_def *tm2);

/**
 * @brief Compares time defs.
 *
 * @return  true if tm1 == tm2
 */
bool IsEqualTimeDefs(const time_def *tm1, const time_def *tm2);

/**
* @brief Function used for comparing timer values with rollover condition
* @param time0 RTC value
* @param time1 RTC value
* @return true if time0 <= time1, else return false
*/
bool rtc_time_compare_func(uint32_t time_0, uint32_t time_1);

#endif /* HALO_LIB_RINGNET_WAN_SRC_INCLUDE_RNET_TIME_H_ */
