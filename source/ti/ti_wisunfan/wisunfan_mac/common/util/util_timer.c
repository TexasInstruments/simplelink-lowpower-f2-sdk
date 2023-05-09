/******************************************************************************

 @file util_timer.c

 @brief This file contains Timer/Clock functions commonly used by
        TIMAC applications for CC13xx with TIRTOS

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2023, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

/******************************************************************************
 Includes
 *****************************************************************************/

#include <string.h>

#include <stdbool.h>
// #include <ti/sysbios/knl/Semaphore.h>
// #include <ti/sysbios/knl/Queue.h>

#include "util_timer.h"

/******************************************************************************
 Constants and Typedefs
 *****************************************************************************/

/*! Adjustment for the timers */
#define TIMER_MS_ADJUSTMENT     100

/* RTOS queue for profile/app messages. */
typedef struct _queueRec_
{
    Queue_Elem _elem;    /* queue element */
    uint8_t *pData;      /* pointer to app data */
} queueRec_t;

/******************************************************************************
 Public Functions
 *****************************************************************************/

/*!
 Initialize a TIRTOS Timer/Clock instance.

 Public function defined in mac_util.h
 */
Clock_Handle Timer_construct(Clock_Struct *pClock, Clock_FuncPtr clockCB,
                                 uint32_t clockDuration, uint32_t clockPeriod,
                                 uint8_t startFlag,
                                 UArg arg)
{
    Clock_Params clockParams;

    /* Convert clockDuration in milliseconds to ticks. */
    uint32_t clockTicks = clockDuration * (1000 / Clock_tickPeriod);

    /* Setup parameters. */
    Clock_Params_init(&clockParams);

    /* Setup argument. */
    clockParams.arg = arg;

    /* If period is 0, this is a one-shot timer. */
    clockParams.period = clockPeriod * (1000 / Clock_tickPeriod);

    /*
     Starts immediately after construction if true, otherwise wait for a
     call to start.
     */
    clockParams.startFlag = startFlag;

    /*/ Initialize clock instance. */
    Clock_construct(pClock, clockCB, clockTicks, &clockParams);

    return Clock_handle(pClock);
}

/*!
 Start a timer/clock.

 Public function defined in mac_util.h
 */
void Timer_start(Clock_Struct *pClock)
{
    Clock_Handle handle = Clock_handle(pClock);

    /* Start clock instance */
    Clock_start(handle);
}

/*!
 Determine if a timer/clock is currently active.

 Public function defined in mac_util.h
 */
bool Timer_isActive(Clock_Struct *pClock)
{
    Clock_Handle handle = Clock_handle(pClock);

    /* Start clock instance */
    return Clock_isActive(handle);
}

/*!
 Stop a timer/clock.

 Public function defined in mac_util.h
 */
void Timer_stop(Clock_Struct *pClock)
{
    Clock_Handle handle = Clock_handle(pClock);

    /* Start clock instance */
    Clock_stop(handle);
}

/*!
 * @brief   Set a Timer/Clock timeout.
 *
 * @param   timeOut - Timeout value in milliseconds
 */
void Timer_setTimeout(Clock_Handle handle, uint32_t timeout)
{
    Clock_setTimeout(handle, (timeout * TIMER_MS_ADJUSTMENT));
}

/*!
 * @brief   Get a Timer/Clock timeout.
 *
 * @param   handle - clock handle
 *
 * @return   timeOut - Timeout value in milliseconds
 */
uint32_t Timer_getTimeout(Clock_Handle handle)
{
    uint32_t timeout;

    timeout = Clock_getTimeout(handle);

    return (timeout / TIMER_MS_ADJUSTMENT);
}

/*!
 * @brief   Set a Timer/Clock callback function and argument.
 *
 * @param   handle - clock handle
 * @param   fxn - clock callback function
 * @param   arg - clock callback function argument
 *
 */
void Timer_setFunc(Clock_Handle handle, Clock_FuncPtr fxn, UArg arg)
{
    Clock_setFunc(handle, fxn, arg);
}
