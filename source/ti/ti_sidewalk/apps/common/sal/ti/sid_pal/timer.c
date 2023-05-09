/******************************************************************************

 @file  timer.c

 @brief Describe the purpose and contents of the file.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************

 Copyright (c) 2023, Texas Instruments Incorporated

 All rights reserved not granted herein.
 Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free,
 non-exclusive license under copyrights and patents it now or hereafter
 owns or controls to make, have made, use, import, offer to sell and sell
 ("Utilize") this software subject to the terms herein. With respect to the
 foregoing patent license, such license is granted solely to the extent that
 any such patent is necessary to Utilize the software alone. The patent
 license shall not apply to any combinations which include this software,
 other than combinations with devices manufactured by or for TI ("TI
 Devices"). No hardware patent is licensed hereunder.

 Redistributions must preserve existing copyright notices and reproduce
 this license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution.

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

   * No reverse engineering, decompilation, or disassembly of this software
     is permitted with respect to any software provided in binary form.
   * Any redistribution and use are licensed by TI for use only with TI Devices.
   * Nothing shall obligate TI to provide you with source code for the software
     licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution
 of the source code are permitted provided that the following conditions are
 met:

   * Any redistribution and use of the source code, including any resulting
     derivative works, are licensed by TI for use only with TI Devices.
   * Any redistribution and use of any object code compiled from the source
     code and any resulting derivative works, are licensed by TI for use
     only with TI Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this
 software without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************


 *****************************************************************************/

/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <sid_time_types.h>
#include <sid_pal_timer_ifc.h>
#include <sid_pal_assert_ifc.h>
#include <ti/drivers/dpl/ClockP.h>
#include <TimerP.h>

/* 1 timer_count has been shifted to the left by 17 */
#define PAL_TIMER_COUNTS_ON_PAST                (((200 << 15) / 1000000) << 17) // 200 usec
#define PAL_TIMER_TIME_TO_COUNT(SEC,NSEC)       ((SEC << 32)|(((NSEC << 15) / SID_TIME_NSEC_PER_SEC) << 17))
#define PAL_TIMER_PERIOD64                      ((0x100000000UL * ClockP_getSystemTickPeriod()) / 1000000U)

sid_error_t sid_pal_timer_init(sid_pal_timer_t * timer, sid_pal_timer_cb_t event_callback, void * event_callback_arg)
{
    ClockP_Params clk_params;

    if (!timer || !event_callback) {
        return SID_ERROR_INVALID_ARGS;
    }

    if (timer->handle) {
        return SID_ERROR_NONE;
    }

    timer->callback = event_callback;
    timer->callback_arg = event_callback_arg;
    timer->alarm        = SID_TIME_INFINITY;
    timer->period       = SID_TIME_INFINITY;

    // timer parameters initialization
    ClockP_Params_init(&clk_params);
    clk_params.startFlag = false;
    clk_params.period = 0;
    clk_params.arg = (uint32_t)timer;

    // create timer structure and handle
    timer->handle = ClockP_create((ClockP_Fxn)sid_pal_timer_event_callback, 0, &clk_params);

    // handle is not created when heap memory is not sufficient
    if (!timer->handle) {
        return SID_ERROR_OUT_OF_RESOURCES;
    }

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_timer_deinit(sid_pal_timer_t * timer)
{
    if (!timer || !timer->handle) {
        return SID_ERROR_INVALID_ARGS;
    }

    timer->callback = NULL;
    timer->callback_arg = NULL;

    ClockP_delete(timer->handle);

    timer->handle = NULL;

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_timer_arm(sid_pal_timer_t * timer, sid_pal_timer_prio_class_t type,
        const struct sid_timespec * when, const struct sid_timespec * period)
{
    (void) type;

    uint32_t tick_timeout;
    uint32_t tick_period;
    uint64_t count_period;
    uint64_t count_now;
    uint64_t count_when;
    uint64_t count_timeout;

    if (!timer || !timer->handle || !when) {
        return SID_ERROR_INVALID_ARGS;
    }

    if (sid_pal_timer_is_armed(timer)) {
        return SID_ERROR_INVALID_ARGS;
    }

    // if period is not specified, the timer expires only one shot at when
    if (!period) {
        period = &SID_TIME_ZERO;
    }

    // store the time out and period in timer storage
    timer->alarm    = *when;
    timer->period   = *period;

    // workaround for ClockP_setPeriod
    count_period = PAL_TIMER_TIME_TO_COUNT((uint64_t)period->tv_sec, (uint64_t)period->tv_nsec);
    tick_period = count_period / PAL_TIMER_PERIOD64;
    ClockP_setPeriod(timer->handle, tick_period);

    count_now = TimerP_getCount64(0);
    count_when = PAL_TIMER_TIME_TO_COUNT((uint64_t)when->tv_sec, (uint64_t)when->tv_nsec);

    if (count_when <= count_now) {
        count_when = count_now + PAL_TIMER_COUNTS_ON_PAST;
    }
    count_timeout = count_when - count_now;

    tick_timeout = count_timeout / PAL_TIMER_PERIOD64;

    // set timeout
    ClockP_setTimeout(timer->handle, tick_timeout);

    ClockP_start(timer->handle);

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_timer_cancel(sid_pal_timer_t * timer)
{
    if (!timer || !timer->handle) {
        return SID_ERROR_INVALID_ARGS;
    }

    if (sid_pal_timer_is_armed(timer)) {
        ClockP_stop(timer->handle);
    }

    return SID_ERROR_NONE;
}

bool sid_pal_timer_is_armed(const sid_pal_timer_t * timer)
{
    if (!timer || !timer->handle) {
        return false;
    }


    return ClockP_isActive(timer->handle);
}

sid_error_t sid_pal_timer_facility_init(void * arg)
{
    return SID_ERROR_NONE;
}

void sid_pal_timer_event_callback(void * arg, const struct sid_timespec * now)
{
    sid_pal_timer_t * timer = (sid_pal_timer_t *)arg;

    timer->callback(timer->callback_arg, timer);
}
