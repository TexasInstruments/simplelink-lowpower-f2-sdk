/*
 * Copyright (c) 2015-2024, Texas Instruments Incorporated
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
 *  ======== ClockP_tirtos.c ========
 */

#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/dpl/HwiP.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Types.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

/* Upper 32 bits of the 64-bit SysTickCount */
static uint32_t upperSystemTicks64      = 0;
static bool isUpperSystemTicks64Updated = false;

/* Callback function to increment 64-bit counter on 32-bit counter overflow */
static void systemTicks64Callback(uintptr_t arg);

/*
 *  ======== ClockP_construct ========
 */
ClockP_Handle ClockP_construct(ClockP_Struct *handle, ClockP_Fxn clockFxn, uint32_t timeout, ClockP_Params *params)
{
    Clock_Params clockParams;
    Clock_Handle clock;

    if (params == NULL)
    {
        Clock_construct((Clock_Struct *)handle, (Clock_FuncPtr)clockFxn, timeout, NULL);
    }
    else
    {
        Clock_Params_init(&clockParams);
        clockParams.arg       = params->arg;
        clockParams.startFlag = params->startFlag;
        clockParams.period    = params->period;
        Clock_construct((Clock_Struct *)handle, (Clock_FuncPtr)clockFxn, timeout, &clockParams);
    }

    clock = Clock_handle((Clock_Struct *)handle);

    return ((ClockP_Handle)clock);
}

/*
 *  ======== ClockP_create ========
 */
ClockP_Handle ClockP_create(ClockP_Fxn clockFxn, uint32_t timeout, ClockP_Params *params)
{
    Clock_Handle handle;
    Clock_Params clockParams;

    /* Use 0 for timeout since we are only doing one-shot clocks */
    if (params == NULL)
    {
        handle = Clock_create((Clock_FuncPtr)clockFxn, timeout, NULL, Error_IGNORE);
    }
    else
    {
        Clock_Params_init(&clockParams);
        clockParams.arg       = params->arg;
        clockParams.startFlag = params->startFlag;
        clockParams.period    = params->period;
        handle                = Clock_create((Clock_FuncPtr)clockFxn, timeout, &clockParams, Error_IGNORE);
    }

    return ((ClockP_Handle)handle);
}

/*
 *  ======== ClockP_delete ========
 */
void ClockP_delete(ClockP_Handle handle)
{
    Clock_Handle clock = (Clock_Handle)handle;

    Clock_delete(&clock);
}

/*
 *  ======== ClockP_destruct ========
 */
void ClockP_destruct(ClockP_Struct *clockP)
{
    Clock_destruct((Clock_Struct *)clockP);
}

/*
 *  ======== ClockP_getCpuFreq ========
 */
void ClockP_getCpuFreq(ClockP_FreqHz *freq)
{
    BIOS_getCpuFreq((Types_FreqHz *)freq);
}

/*
 *  ======== ClockP_getSystemTickPeriod ========
 */
uint32_t ClockP_getSystemTickPeriod(void)
{
    return (Clock_tickPeriod);
}

/*
 *  ======== ClockP_getSystemTicks ========
 */
uint32_t ClockP_getSystemTicks(void)
{
    return (Clock_getTicks());
}

/*
 *  ======== systemTicks64Callback ========
 */
static void systemTicks64Callback(uintptr_t arg)
{
    /* Disable interrupts while updating upper global upperSystemTicks64 */
    uintptr_t key = HwiP_disable();

    if (isUpperSystemTicks64Updated == false)
    {
        /* Callback has occurred shortly after 32 bits overflow. Increment the
         * upper 32 bits and set the updated flag (isUpperSystemTicks64Updated).
         */
        upperSystemTicks64++;
        isUpperSystemTicks64Updated = true;
    }
    else
    {
        /* Callback has occurred shortly after the midway between two 32 bits
         * overflows. Clear the updated flag (isUpperSystemTicks64Updated) so
         * the upperSystemTicks64 will be updated on the next callback.
         */
        isUpperSystemTicks64Updated = false;
    }

    HwiP_restore(key);
}

/*
 *  ======== ClockP_getSystemTicks64 ========
 */
uint64_t ClockP_getSystemTicks64(void)
{
    static ClockP_Struct systemTicks64Clock;
    static bool systemTicks64Initialised = false;
    ClockP_Params params;
    uint32_t lowerSystemTicks64;
    uintptr_t key;
    uint64_t tickValue;

    key = HwiP_disable();

    /* Initialise clock needed to maintain 64-bit SystemTicks when function
     * is called for the first time.
     */
    if (!systemTicks64Initialised)
    {
        ClockP_Params_init(&params);

        /* Start clock immediately when created */
        params.startFlag = true;
        /* Configure the clock to trigger with double the frequency as 32-bit
         * overflow. When the callback function is called just after the 32-bit
         * counter overflows, it will clear the isUpperSystemTicks64Updated
         * flag. When the callback function is called halfway between the
         * previous overflow and the next one, it will clear the
         * updatedUppterSysTick64 flag.
         */
        params.period    = ((uint64_t)UINT32_MAX + 1) / 2;

        uint32_t currentTick = ClockP_getSystemTicks();

        /* The clock must be synchronized with the overflow event.
         * Calculate ticks until next overflow.
         */
        uint32_t delayedStart = ((uint64_t)UINT32_MAX + 1) - currentTick;

        /* Ensure the delayed start is non-zero and less than (or equal to) the
         * period, and set the initial state of isUpperSystemTicks64Updated
         * accordingly.
         */
        if ((delayedStart > params.period) || (delayedStart == 0))
        {
            delayedStart -= params.period;

            /* The first callback will be at the midpoint between the next
             * overflow and the previous overflow. Meaning the first time the
             * callback function is called, the isUpperSystemTicks64Updated flag
             * should be cleared and upperSystemTicks64 should not be
             * incremented. Setting the isUpperSystemTicks64Updated flag will
             * also prevent upperSystemTicks64Temp from being artificially
             * incremented below.
             */
            isUpperSystemTicks64Updated = true;
        }
        else
        {
            /* The first callback will be at the overflow, meaning the first
             * time the callback function is called, upperSystemTicks64 should
             * be incremented and the isUpperSystemTicks64Updated flag should be
             * set.
             */
            isUpperSystemTicks64Updated = false;
        }

        /* Construct and start the clock */
        ClockP_construct(&systemTicks64Clock, systemTicks64Callback, delayedStart, &params);

        systemTicks64Initialised = true;
    }

    lowerSystemTicks64              = ClockP_getSystemTicks();
    uint32_t upperSystemTicks64Temp = upperSystemTicks64;

    /* If the lower 32 bits have recently overflowed, but the upper 32 bits
     * have not yet been incremented (i.e. systemTicks64Callback() has not yet
     * executed) then artificially increment the upper 32 bits here.
     */
    if ((lowerSystemTicks64 < (((uint64_t)UINT32_MAX + 1) / 2)) && (isUpperSystemTicks64Updated == false))
    {
        upperSystemTicks64Temp++;
    }

    /* Return the upper 32 bits + lower 32 bits as is */
    tickValue = ((uint64_t)upperSystemTicks64Temp << 32) | lowerSystemTicks64;

    HwiP_restore(key);

    return tickValue;
}

/*
 *  ======== ClockP_getTimeout ========
 */
uint32_t ClockP_getTimeout(ClockP_Handle handle)
{
    return (Clock_getTimeout((Clock_Handle)handle));
}

/*
 *  ======== ClockP_isActive ========
 */
bool ClockP_isActive(ClockP_Handle handle)
{
    return (Clock_isActive((Clock_Handle)handle));
}

/*
 *  ======== ClockP_Params_init ========
 */
void ClockP_Params_init(ClockP_Params *params)
{
    params->arg       = 0;
    params->startFlag = false;
    params->period    = 0;
}

/*
 *  ======== ClockP_setFunc ========
 */
void ClockP_setFunc(ClockP_Handle handle, ClockP_Fxn clockFxn, uintptr_t arg)
{
    Clock_setFunc((Clock_Handle)handle, (Clock_FuncPtr)clockFxn, arg);
}

/*
 *  ======== ClockP_setTimeout ========
 */
void ClockP_setTimeout(ClockP_Handle handle, uint32_t timeout)
{
    Clock_setTimeout((Clock_Handle)handle, timeout);
}

/*
 *  ======== ClockP_setPeriod ========
 */
void ClockP_setPeriod(ClockP_Handle handle, uint32_t period)
{
    Clock_setPeriod((Clock_Handle)handle, period);
}

/*
 *  ======== ClockP_start ========
 */
void ClockP_start(ClockP_Handle handle)
{
    Clock_start((Clock_Handle)handle);
}

/*
 *  ======== ClockP_stop ========
 */
void ClockP_stop(ClockP_Handle handle)
{
    Clock_stop((Clock_Handle)handle);
}

/*
 *  ======== ClockP_sleep ========
 */
void ClockP_sleep(uint32_t sec)
{
    unsigned long timeout;

    timeout = ((unsigned long)sec * 1000000L) / Clock_tickPeriod;

    Task_sleep((uint32_t)timeout);
}

/*
 *  ======== ClockP_usleep ========
 */
void ClockP_usleep(uint32_t usec)
{
    uint32_t timeout;

    /* Clock_tickPeriod is the Clock period in microsecnds */
    timeout = (uint32_t)((usec + Clock_tickPeriod / 2) / Clock_tickPeriod);

    Task_sleep(timeout);
}

/*
 *  ======== ClockP_staticObjectSize ========
 */
size_t ClockP_staticObjectSize(void)
{
    return (sizeof(Clock_Struct));
}
