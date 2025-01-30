/*
 * Copyright (c) 2016-2024 Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== clock.c ========
 */

#include <stdint.h>
#include <time.h>
#include <errno.h>

/* FreeRTOS header files */
#include <FreeRTOS.h>
#include <task.h>
#include <portmacro.h>

#include <ti/drivers/dpl/HwiP.h>

#include "time_defines.h"

int _clock_abstime2ticks(clockid_t clockId, const struct timespec *abstime, TickType_t *ticks);

static void _clock_gettimeMono(struct timespec *ts);

/* Seconds, nanoseconds passed to clock_settime() */
static struct timespec setTS = {0, 0};

/* The CLOCK_MONOTONIC time at the time clock_settime() was called */
static struct timespec refTS = {0, 0};

/*
 *  ======== clock_gettime ========
 */
int clock_gettime(clockid_t clockId, struct timespec *ts)
{
    /* do this first to minimize the latency of latching the current time */
    _clock_gettimeMono(ts);

    if ((clockId != CLOCK_MONOTONIC) && (clockId != CLOCK_REALTIME))
    {
        errno = EINVAL;
        return (-1);
    }

    if (clockId == CLOCK_MONOTONIC)
    {
        return (0);
    }

    /* CLOCK_REALTIME */
    if (ts->tv_nsec < refTS.tv_nsec)
    {
        ts->tv_nsec += NSEC_PER_SEC;
        ts->tv_sec--;
    }

    ts->tv_sec  = ts->tv_sec - refTS.tv_sec + setTS.tv_sec;
    ts->tv_nsec = ts->tv_nsec - refTS.tv_nsec + setTS.tv_nsec;

    if (ts->tv_nsec >= NSEC_PER_SEC)
    {
        ts->tv_sec++;
        ts->tv_nsec -= NSEC_PER_SEC;
    }

    return (0);
}

/*
 *  ======== clock_nanosleep ========
 */
int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec *rqtp, struct timespec *rmtp)
{
    TickType_t xDelay;

    if ((clock_id != CLOCK_MONOTONIC) && (clock_id != CLOCK_REALTIME))
    {
        return (EINVAL);
    }

    /* Ignore rmtp (remaining time, if woken by a signal) */
    if ((rmtp != NULL) && (flags & TIMER_ABSTIME) == 0)
    {
        rmtp->tv_sec = rmtp->tv_nsec = 0;
    }

    if (flags & TIMER_ABSTIME)
    {
        if (_clock_abstime2ticks(clock_id, rqtp, &xDelay) != 0)
        {
            return (EINVAL);
        }

        if (xDelay == 0)
        {
            return (0);
        }
    }
    else
    {
        /* max interval, needs to be fixed: TIRTOS-1314 */
        if (rqtp->tv_sec >= FREERTOS_MAX_SECONDS)
        {
            return (EINVAL);
        }

        if ((rqtp->tv_sec == 0) && (rqtp->tv_nsec == 0))
        {
            return (0);
        }

        xDelay = rqtp->tv_sec * configTICK_RATE_HZ;

        /*
         *  Take the ceiling.  The remaining nanoseconds will count as an
         *  entire tick.
         */
        xDelay += (rqtp->tv_nsec + (NSEC_PER_SEC / configTICK_RATE_HZ) - 1) / (NSEC_PER_SEC / configTICK_RATE_HZ);

        /*
         *  Add one tick to ensure the timeout is not less than the
         *  amount of time requested.  The clock may be about to tick,
         *  and that counts as one tick even though the amount of time
         *  until this tick is less than the tick period.
         */
        xDelay++;
    }

    vTaskDelay(xDelay);

    return (0);
}

/*
 *  ======== clock_settime ========
 */
int clock_settime(clockid_t clock_id, const struct timespec *ts)
{
    uintptr_t key;

    if (clock_id != CLOCK_REALTIME)
    {
        errno = EINVAL;
        return (-1);
    }

    /* Number of nanoseconds in a timespec struct should always be in the range [0,1000000000) */
    if ((ts->tv_nsec < 0) || (ts->tv_nsec >= NSEC_PER_SEC))
    {
        errno = EINVAL;
        return (-1);
    }

    key = HwiP_disable();

    _clock_gettimeMono(&refTS);
    setTS = *ts;

    HwiP_restore(key);

    return (0);
}

/*
 *  ======== time ========
 */
#if defined(__clang__) && defined(__TI_TIME_USES_64) && __TI_TIME_USES_64
_CODE_ACCESS __time64_t __time64(__time64_t *tout)
#else
time_t time(time_t *tout)
#endif
{
    struct timespec ts;

    /* get current time (relative to POSIX Epoch) */
    clock_gettime(CLOCK_REALTIME, &ts);

#if defined(__TI_COMPILER_VERSION__)
    /* TI time() function must return seconds since 1900 (TI Epoch) */
    ts.tv_sec += TI_EPOCH_OFFSET;
#endif

    if (tout)
    {
        *tout = (time_t)(ts.tv_sec);
    }

    return ((time_t)(ts.tv_sec));
}

/*
 *************************************************************************
 *              internal functions
 *************************************************************************
 */

/*
 *  ======== _clock_abstime2ticks ========
 */
int _clock_abstime2ticks(clockid_t clockId, const struct timespec *abstime, TickType_t *ticks)
{
    struct timespec curtime;
    TickType_t timeout;
    long nsecs  = 0;
    time_t secs = 0;

    /* Number of nanoseconds in a timespec struct should always be in the range [0,1000000000) */
    if ((abstime->tv_nsec < 0) || (NSEC_PER_SEC <= abstime->tv_nsec))
    {
        return (-1);
    }
    clock_gettime(clockId, &curtime);
    secs = abstime->tv_sec - curtime.tv_sec;

    if ((abstime->tv_sec < curtime.tv_sec) || ((secs == 0) && (abstime->tv_nsec <= curtime.tv_nsec)))
    {
        timeout = 0;
    }
    else
    {
        nsecs = abstime->tv_nsec - curtime.tv_nsec;

        if (nsecs < 0)
        {
            nsecs += NSEC_PER_SEC;
            secs--;
        }

        timeout = secs * configTICK_RATE_HZ;
        timeout += (nsecs + (NSEC_PER_SEC / configTICK_RATE_HZ) - 1) / (NSEC_PER_SEC / configTICK_RATE_HZ);

        /*
         *  Add one tick to ensure the timeout is not less than the
         *  amount of time requested.  The clock may be about to tick,
         *  and that counts as one tick even though the amount of time
         *  until this tick is less than the clock period.
         */
        timeout++;
    }

    *ticks = timeout;

    return (0);
}

/*
 *  ======== _clock_timespecToTicks ========
 */
uint64_t _clock_timespecToTicks(const struct timespec *ts)
{
    uint64_t ticks;

    /* Calculate total amount of ticks from both seconds and nanoseconds in the timespec */
    ticks = ts->tv_sec * configTICK_RATE_HZ + ts->tv_nsec * (uint64_t)configTICK_RATE_HZ / NSEC_PER_SEC;

    return ticks;
}

/*
 *  ======== _clock_gettimeMono ========
 */
static void _clock_gettimeMono(struct timespec *ts)
{
    unsigned long secs;
    TickType_t ticks;
    TickType_t remTicks;
    unsigned long remSecs;
    uintptr_t key;
    uint32_t numRollovers;
    TimeOut_t timeout;

    key = HwiP_disable();
    /*
     *  This gets the number of tick count overflows as well as the tick
     *  count.  We use the internal API (new in FreeRTOS 10), since it can be
     *  called from an ISR.  Calling the public API, vTaskSetTimeOutState,
     *  will cause an exception if called from an ISR.
     */
    vTaskInternalSetTimeOutState(&timeout);

    HwiP_restore(key);

    ticks        = timeout.xTimeOnEntering;
    numRollovers = timeout.xOverflowCount;

    /* Number of seconds in tick count */
    secs = ticks / configTICK_RATE_HZ;

    /* Remaining ticks after seconds are subtracted from tick count */
    remTicks = ticks - (secs * configTICK_RATE_HZ);

    /* Add contribution of remaining ticks from tick count rollovers */
    remTicks = remTicks + (FREERTOS_REM_TICKS * numRollovers);

    remSecs  = remTicks / configTICK_RATE_HZ;
    remTicks = remTicks - (remSecs * configTICK_RATE_HZ);

    ts->tv_sec  = (time_t)secs + remSecs + (FREERTOS_MAX_SECONDS * numRollovers);
    ts->tv_nsec = (unsigned long)(remTicks * (NSEC_PER_SEC / configTICK_RATE_HZ));
}
