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
 * @file osal_time.h
 *
 * @brief ACE OSAL Time APIs.
 *
 * @ingroup ACE_OSAL_TIME
 */
#ifndef ACEOSAL_TIME_H
#define ACEOSAL_TIME_H

#include <ace/ace_status.h>
#include <ace/os_specific.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Type representing ace time in seconds + microseconds
 * @ingroup ACE_OSAL_TIME
 */
typedef struct {
    uint32_t tv_sec;  /** Seconds */
    uint32_t tv_usec; /** Microseconds */
} aceTime_time_t;

/**
 * @brief Type representing ace broken-down time (alias C standard library
 *        definition)\n
 *        struct tm {\n
 *          int tm_sec;\n
 *          int tm_min;\n
 *          int tm_hour;\n
 *          int tm_mday;\n
 *          int tm_mon;\n
 *          int tm_year;\n
 *          int tm_wday;\n
 *          int tm_yday;\n
 *          int tm_isdst;\n
 *          long int tm_gmtoff;\n
 *          const char* tm_zone;\n
 *        };
 * @ingroup ACE_OSAL_TIME
 */
typedef struct tm aceTime_tm_t;

/**
 * @brief ACE OSAL Timer type.
 *
 *        It's definition is dependent on the underlying
 *        platform. Users must treat this as an opaque type whose internals
 *        can only be accessed via the OSAL mutex APIs.
 *
 * @ingroup ACE_OSAL_TIME
 */
typedef struct aceTimer_struct aceTimer_t;

/**
 * @brief Type representing timespec in seconds + nanoseconds\n
 *        struct timespec {\n
 *          time_t tv_sec;\n
 *          long tv_nsec;\n
 *        };
 *
 * @ingroup ACE_OSAL_TIME
 */
typedef struct timespec aceTime_timespec_t;

/**
 * @brief Type representing timer callback
 * @ingroup ACE_OSAL_TIME
 */
typedef void (*aceTime_timerCallback_t)(void* ctx);

/**
 * @brief Clock type support
 * @ingroup ACE_OSAL_TIME
 * @note On some platforms monotonic time may be the same as boot time and
 *       diffing monotonic time and boot time as a measure of time spent
 *       in sleep is not supported.
 */
typedef enum aceTime_clockType {
    /** Clock that cannot be set and represents monotonic time since some
        unspecified starting point. */
    ACETIME_CLOCK_MONOTONIC = 0,
    /** Clock that measures real time. Setting this clock requires appropriate
        privileges. */
    ACETIME_CLOCK_REALTIME = 1,
    /**
     * Similar to ACETIME_CLOCK_MONOTONIC, except that it will both continue
     * counting while the system is suspended, and wake the system if it is
     * suspended. On systems without ACE_OSAL_ALARM_TIMER_SUPPORT enabled, this
     * will fallback to ACETIME_CLOCK_MONOTONIC.
     *
     * @note Only intended for use with aceTime_timerNew().
     */
    ACETIME_CLOCK_BOOTTIME_ALARM = 2,
    /** Monotonically increasing clock that must include any time that
        the system is suspended. */
    ACETIME_CLOCK_BOOTTIME = 3
} aceTime_clockType_t;

/**
 * @brief Timer type support
 * @ingroup ACE_OSAL_TIME
 */
typedef enum aceTime_timerType {
    /** Once started, it will execute its callback function once only */
    ACETIME_TIMER_ONESHOT = 0,
    /** Once started, it will re-start itself each time it expires */
    ACETIME_TIMER_PERIODIC = 1,
} aceTime_timerType_t;

/**
 * @brief Get epoch time
 *        Get the system calendar time in terms of seconds and microseconds
 *        since the Epoch,
 *        1970-01-01 00:00:00 +0000 (UTC).
 * @param[out] t storage for holding current epoch time\n
 *             t->tv_sec : second part\n
 *             t->tv_usec : microsecond part\n
 * @return ACE_STATUS_OK (if succeed)\n
 *         ACE_STATUS_GENERAL_ERROR (if get time error)\n
 *         ACE_STATUS_NULL_POINTER (if pointer parameters are equal to NULL)\n
 * @note Once system time is set, @ref aceTime_setClockTime should
 *       be called to ensure exact calendar time.
 *
 * @ingroup ACE_OSAL_TIME
 */
ace_status_t aceTime_time(aceTime_time_t* t);

/**
 * @brief Get the time in nanosecond resolution with clock type
 * @param[in] clock_id clock type ACETIME_CLOCK_MONOTONIC,
 * ACETIME_CLOCK_REALTIME or ACETIME_CLOCK_BOOTTIME.
 * @param[out] t storage for holding current time\n
 *             t->tv_sec : second part\n
 *             t->tv_nsec : nanosecond part\n
 * @return ACE_STATUS_OK (if succeed)\n
 *         ACE_STATUS_GENERAL_ERROR (if get time error)\n
 *         ACE_STATUS_NULL_POINTER (if pointer parameters are equal to NULL)
 * @note Once system time is set, @ref aceTime_setClockTime should
 *       be called to ensure exact REALTIME time.
 *
 * @ingroup ACE_OSAL_TIME
 */
ace_status_t aceTime_clockTime(aceTime_clockType_t clock_id,
                               aceTime_timespec_t* t);

/**
 * @brief Set time in nanosecond resolution
 * @param[out] t time to set since Epoch\n
 *             t->tv_sec : second part\n
 *             t->tv_nsec : nanosecond part\n
 * @return ACE_STATUS_OK (if succeed)\n
 *         ACE_STATUS_GENERAL_ERROR (if set time error)\n
 *         ACE_STATUS_NULL_POINTER (if pointer parameters are equal to NULL)
 * @note May require the right OS permissions for setting system time.
 *
 * @ingroup ACE_OSAL_TIME
 */
ace_status_t aceTime_setClockTime(const aceTime_timespec_t* t);

/**
 * @brief Convert the aceTime_timespec_t struct to a uint64_t value.
 * @param[in]  t storage for holding current time\n
 *             t->tv_sec : second part\n
 *             t->tv_nsec : nanosecond part\n
 * @return Value corresponding to timespec as microseconds since epoch
 *
 * @ingroup ACE_OSAL_TIME
 */
uint64_t aceTime_timespecToMicroSec(aceTime_timespec_t* t);

/**
 * @brief Microsecond resolution diff of two aceTime_timespec_t structs.
 * @param[in]  t1 Pointer to storage for holding aceTime_timespec_t
 * @param[in]  t2 Pointer to storage for holding aceTime_timespec_t
 * @return Time diff between t1 and t2 computed as t2 - t1 in micro
 *         seconds. API is not timestamp rollover aware.
 *
 * @ingroup ACE_OSAL_TIME
 */
int64_t aceTime_timespecDiff(aceTime_timespec_t* t1, aceTime_timespec_t* t2);

/**
 * @brief Convert the system calendar time to broken-down time representation
 *        (sec/min/hour/mday/mon/year).
 * @param[in] t system calendar time
 * @param[out] tm storage to hold the broken-down time
 * @return ACE_STATUS_OK (if succeed)\n
 *         ACE_STATUS_NULL_POINTER (if pointer parameters are equal to NULL)
 *
 * @ingroup ACE_OSAL_TIME
 */
ace_status_t aceTime_localtime(const aceTime_time_t* t, aceTime_tm_t* tm);

/**
 * @brief Formats the broken-down time tm according to the format specification
 *        format and places the result in the character array s of size max.
 * @param[in] max the size of character array s
 * @param[in] format a null-terminated string and may contain special character
 *            sequences called conversion specifications, each of which is
 *            introduced by a '%' character and terminated by some other
 *            character known as a conversion specifier character. All other
 *            character sequences are ordinarycharacter sequences. The format
 *            specifiers is the same as strftime funtion(C standard library).
 * @param[in] tm current time
 * @param[out] s storage to hold the final time string
 * @return the number of bytes placed in the array s, not including the
 *         terminating null byte, provided the string, including the
 *         terminating null byte. Otherwise, it returns 0, and the contents
 *         of the array is undefined.
 *
 * @ingroup ACE_OSAL_TIME
 */
size_t aceTime_strftime(char* s, size_t max, const char* format,
                        const aceTime_tm_t* tm);

/**
 * @brief Initialize a timer. It's the callback function not excute in this
 *        process.
 * @param[out] t storage to save timer handle
 * @param[in] clock_type timer clock type
 * @param[in] timer_type timer type (one-shot or periodic)
 * @param[in] callback timer callback function
 * @param[in] ctx callback ctx. Do not use local varibles
 * @return ACE_STATUS_OK (if succeed)\n
 *         ACE_STATUS_ALREADY_INITIALIZED (if the timer has been initialized)\n
 *         ACE_STATUS_NULL_POINTER (if pointer parameters are equal to NULL)\n
 *         ACE_STATUS_GENERAL_ERROR (if creat timer error)
 *
 * @ingroup ACE_OSAL_TIME
 */
ace_status_t aceTime_timerNew(aceTimer_t* t, aceTime_clockType_t clock_type,
                              aceTime_timerType_t timer_type,
                              aceTime_timerCallback_t callback, void* ctx);

/**
 * @brief Start a timer with milliseconds as interval
 * @param[in] t timer handle
 * @param[in] period_ms timer interval in milliseconds
 * @return ACE_STATUS_OK (if succeed)\n
 *         ACE_STATUS_UNINITIALIZED (if the timer is uninitialized)\n
 *         ACE_STATUS_NULL_POINTER (if pointer parameters are equal to NULL)\n
 *         ACE_STATUS_BAD_PARAM (if parameters are invalid)\n
 *         ACE_STATUS_GENERAL_ERROR (if start timer error)
 * @note If a timer was already started, then it will be restarted.
 *
 * @ingroup ACE_OSAL_TIME
 */
ace_status_t aceTime_timerStart(aceTimer_t* t, uint32_t period_ms);

/**
 * @brief Start a timer with aceTime_timespec_t used as interval and also flags.
 *        This is a more extended timer start API that allows further
 *        control of things if the aceTime_time_start() is too limited.
 * @param[in] t timer handle
 * @param[in] it timer interval specified in seconds and nanoseconds
 * @param[in] flags timer flags (for future extensibility)
 * @return ACE_STATUS_OK (if succeed)\n
 *         ACE_STATUS_UNINITIALIZED (if the timer is uninitialized)\n
 *         ACE_STATUS_NULL_POINTER (if pointer parameters are equal to NULL)\n
 *         ACE_STATUS_BAD_PARAM (if parameters are invalid)\n
 *         ACE_STATUS_PARAM_OUT_OF_RANGE (if timer interval is longer than
 * UINT32_MAX milliseconds)\n
 *         ACE_STATUS_GENERAL_ERROR (if start timer error)
 * @note If a timer was already started, then it will be restarted.
 * @note it must not contain a duration longer than UINT32_MAX milliseconds.
 *
 * @ingroup ACE_OSAL_TIME
 */
ace_status_t aceTime_timerStartEx(aceTimer_t* t, aceTime_timespec_t* it,
                                  int flags);

/**
 * @brief Check if a timer is running
 * @param[in] t timer handle
 * @return true if timer is running, otherwise false
 *
 * @ingroup ACE_OSAL_TIME
 */
bool aceTime_timerIsRunning(aceTimer_t* t);

/**
 * @brief Stop a timer
 * @param[in] t timer handle
 * @return ACE_STATUS_OK (if succeed)\n
 *         ACE_STATUS_UNINITIALIZED (if the timer is uninitialized)\n
 *         ACE_STATUS_NULL_POINTER (if pointer parameters are equal to NULL)\n
 *         ACE_STATUS_BAD_PARAM (if parameters are invalid)\n
 *         ACE_STATUS_GENERAL_ERROR (if stop timer error)
 * @note If a timer is already stopped, then return ACE_STATUS_OK.
 *
 * @ingroup ACE_OSAL_TIME
 */
ace_status_t aceTime_timerStop(aceTimer_t* t);

/**
 * @brief Delete a timer. Do not use the handle again after delete
 * @param[in] t timer handle
 * @return ACE_STATUS_OK (if succeed)\n
 *         ACE_STATUS_UNINITIALIZED (if the timer is uninitialized)\n
 *         ACE_STATUS_NULL_POINTER (if pointer parameters are equal to NULL)\n
 *         ACE_STATUS_BAD_PARAM (if parameters are invalid)\n
 *         ACE_STATUS_GENERAL_ERROR (if delete timer error)
 *
 * @ingroup ACE_OSAL_TIME
 */
ace_status_t aceTime_timerDelete(aceTimer_t* t);

#ifdef __cplusplus
}
#endif
#endif /* ACEOSAL_TIME_H */
