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

#ifndef SID_PAL_TIMER_IFC_H
#define SID_PAL_TIMER_IFC_H

/** @file
 *
 * @defgroup sid_pal_timers SID Timer API
 * @{
 *
 * @details Interface for timers for sidewalk SDK
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <sid_error.h>
#include <sid_pal_timer_types.h>
#include <sid_time_types.h>

#include <stdbool.h>

typedef enum {
    SID_PAL_TIMER_PRIO_CLASS_PRECISE,  //< Events to be generated with the maximum supported on this platform precision
    SID_PAL_TIMER_PRIO_CLASS_LOWPOWER, //< Events can be delayed for up to 1 second to optimize power consumption
} sid_pal_timer_prio_class_t;

/**
 * Initialize a timer object
 *
 * @param[in]   timer               Timer object to initialize
 * @param[in]   event_callback      Pointer to the callback function the timer event will be delivered to
 * @param[in]   event_callback_arg  Argument to be provided to the @p event_callback during call
 *
 * @retval SID_ERROR_NONE in case of success
 */
sid_error_t sid_pal_timer_init(sid_pal_timer_t * timer, sid_pal_timer_cb_t event_callback, void * event_callback_arg);

/**
 * De-initialize a timer object
 *
 * @param[in]   timer               Timer object to de-initialize
 *
 * @retval SID_ERROR_NONE in case of success
 *
 * Function fully de-initializes the @p timer object. If it is armed, it will be canceled and then de-initialized.
 */
sid_error_t sid_pal_timer_deinit(sid_pal_timer_t * timer);

/**
 * Arm a timer object
 *
 * @param[in]   timer               Timer object to arm
 * @param[in]   type                Priority class specifier for the timer to be armed
 * @param[in]   when                Pointer to struct sid_timespec identifying the time for the first event generation
 * @param[in]   period              Pointer to struct sid_timespec identifying the period between event generation
 *
 * @retval SID_ERROR_NONE in case of success
 *
 * Function will initialize the @p timer object for first shot at time provided in @p when (required). If
 * the @p period is not NULL and is not TIMESPEC_INFINITY, the @p timer object will be armed to repeat events
 * generation periodically with the period according to the time provided in @p period.
 */
sid_error_t sid_pal_timer_arm(sid_pal_timer_t * timer, sid_pal_timer_prio_class_t type,
        const struct sid_timespec * when, const struct sid_timespec * period);

/**
 * Disarm a timer object
 *
 * @param[in]   timer               Timer object to disarm
 *
 * @retval SID_ERROR_NONE in case of success
 *
 * Function will disarm the @p timer object. If it is not armed, function does no operation.
 */
sid_error_t sid_pal_timer_cancel(sid_pal_timer_t * timer);

/**
 * Check a timer object is valid and armed
 *
 * @param[in]   timer               Timer object to check
 *
 * @retval true in case of @p timer object is armed
 * @retval false in case of @p timer object is disarmed, deinitialized or invalid
 *
 */
bool sid_pal_timer_is_armed(const sid_pal_timer_t * timer);

/**
 * Init the timer facility. This function must be called before before sid_pal_timer_init().
 *
 * OPTIONAL This function is typically used to init HW or SW resources needed for the timer.
 * If none are needed by the timer implementation then this function is unnecessary.
 *
 * @retval SID_ERROR_NONE in case of success
 */
sid_error_t sid_pal_timer_facility_init(void * arg);

/**
 * HW event callback
 *
 * OPTIONAL If sid_timer is implemented as a SW timer, this is the callback that can be
 * registered with the HW resource to provide noritification of HW timer expiry.
 */
void sid_pal_timer_event_callback(void * arg, const struct sid_timespec * now);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
