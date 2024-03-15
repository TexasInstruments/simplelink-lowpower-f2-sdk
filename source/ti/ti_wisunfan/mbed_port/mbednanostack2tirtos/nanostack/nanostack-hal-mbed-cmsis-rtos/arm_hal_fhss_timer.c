/*
 * Copyright (c) 2018-2019, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <ti/drivers/dpl/ClockP.h>

#include "ns_types.h"
#include "fhss_api.h"
#include "fhss_config.h"
#include "ns_trace.h"
#include "platform/arm_hal_interrupt.h"

#define TRACE_GROUP "fhdr"
#ifndef NUMBER_OF_SIMULTANEOUS_TIMEOUTS
#define NUMBER_OF_SIMULTANEOUS_TIMEOUTS  2
#endif //NUMBER_OF_SIMULTANEOUS_TIMEOUTS

/* convert slots to ticks */
#define US_PER_SLOT 50

#define SLOTS_TO_TICKS(slots)    (((slots * US_PER_SLOT + ClockP_getSystemTickPeriod()/2) / ClockP_getSystemTickPeriod()))
#define TICKS_TO_SLOTS(ticks)    (((ticks * ClockP_getSystemTickPeriod() + US_PER_SLOT/2) / US_PER_SLOT))

typedef void (*fhss_timer_callback_t)(const fhss_api_t *fhss_api, uint16_t);

typedef struct {
    fhss_timer_callback_t fhss_timer_callback;
    uint32_t start_time;
    uint32_t stop_time;
    bool active;
    ClockP_Struct hal_timer_struct;
    ClockP_Handle hal_timer;
} fhss_timeout_s;

static fhss_timeout_s fhss_timeout[NUMBER_OF_SIMULTANEOUS_TIMEOUTS] = {0};
static fhss_api_t *fhss_active_handle;
static bool timer_initialized = false;

static uint32_t read_current_time(void)
{
    uint32_t tick_time;
    tick_time = (ClockP_getSystemTicks() * ClockP_getSystemTickPeriod());
    return TICKS_TO_SLOTS(tick_time);
}

static fhss_timeout_s *find_timeout(void (*callback)(const fhss_api_t *api, uint16_t))
{
    for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_TIMEOUTS; i++) {
        if (fhss_timeout[i].fhss_timer_callback == callback) {
            return &fhss_timeout[i];
        }
    }
    return NULL;
}

static fhss_timeout_s *allocate_timeout(void)
{
    for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_TIMEOUTS; i++) {
        if (fhss_timeout[i].fhss_timer_callback == NULL) {
            return &fhss_timeout[i];
        }
    }
    return NULL;
}

static void fhss_timeout_handler(void)
{
    for (int i = 0; i < NUMBER_OF_SIMULTANEOUS_TIMEOUTS; i++) {
        if (fhss_timeout[i].active && ((fhss_timeout[i].stop_time - fhss_timeout[i].start_time) <= (read_current_time() - fhss_timeout[i].start_time))) {
            fhss_timeout[i].active = false;
            fhss_timeout[i].fhss_timer_callback(fhss_active_handle, read_current_time() - fhss_timeout[i].stop_time);
        }
    }
}

static void timer_callback(void)
{
#if 1 //MBED_CONF_NANOSTACK_HAL_CRITICAL_SECTION_USABLE_FROM_INTERRUPT
    fhss_timeout_handler();
#else
    equeue->call(fhss_timeout_handler);
#endif
}

static int platform_fhss_timer_start(uint32_t slots, void (*callback)(const fhss_api_t *api, uint16_t), const fhss_api_t *callback_param)
{
    int ret_val = -1;
    platform_enter_critical();

    if (timer_initialized == false) {
        uint8_t i;
        ClockP_Params clkParams;

        ClockP_Params_init(&clkParams);
        clkParams.period = 0;
        clkParams.startFlag = false;

        /* allocate TIRTOS Clocks */
        for(i = 0; i < NUMBER_OF_SIMULTANEOUS_TIMEOUTS; i++)
        {
            /* Construct a periodic Clock Instance */
            fhss_timeout[i].hal_timer = ClockP_construct(&(fhss_timeout[i].hal_timer_struct), (ClockP_Fxn)timer_callback,
                            0, &clkParams);

        }

        timer_initialized = true;
    }

    fhss_timeout_s *fhss_tim = find_timeout(callback);
    if (!fhss_tim) {
        fhss_tim = allocate_timeout();
    }

    if (!fhss_tim) {
        platform_exit_critical();
        tr_error("Failed to allocate timeout");
        return ret_val;
    }
    fhss_tim->fhss_timer_callback = callback;
    fhss_tim->start_time = read_current_time();
    fhss_tim->stop_time = fhss_tim->start_time + slots;
    fhss_tim->active = true;

    ClockP_setTimeout(fhss_tim->hal_timer, SLOTS_TO_TICKS(slots));
    ClockP_start(fhss_tim->hal_timer);

    fhss_active_handle = (fhss_api_t*)callback_param;
    ret_val = 0;
    platform_exit_critical();
    return ret_val;
}

static int platform_fhss_timer_stop(void (*callback)(const fhss_api_t *api, uint16_t), const fhss_api_t *api)
{
    (void)api;
    platform_enter_critical();
    fhss_timeout_s *fhss_tim = find_timeout(callback);
    if (!fhss_tim) {
        platform_exit_critical();
        return -1;
    }
    ClockP_stop(fhss_tim->hal_timer);
    fhss_tim->active = false;
    platform_exit_critical();
    return 0;
}

static uint32_t platform_fhss_get_remaining_slots(void (*callback)(const fhss_api_t *api, uint16_t), const fhss_api_t *api)
{
    (void)api;
    platform_enter_critical();
    fhss_timeout_s *fhss_tim = find_timeout(callback);
    if (!fhss_tim) {
        platform_exit_critical();
        return 0;
    }
    uint32_t remaining_slots = fhss_tim->stop_time - read_current_time();
    platform_exit_critical();
    return remaining_slots;
}

static uint32_t platform_fhss_timestamp_read(const fhss_api_t *api)
{
    (void)api;
    return read_current_time();
}

fhss_timer_t fhss_functions = {
    .fhss_timer_start = platform_fhss_timer_start,
    .fhss_timer_stop = platform_fhss_timer_stop,
    .fhss_get_remaining_slots = platform_fhss_get_remaining_slots,
    .fhss_get_timestamp = platform_fhss_timestamp_read,
    .fhss_resolution_divider = 1
};

