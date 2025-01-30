/******************************************************************************

 @file util_timer.c

 @brief This file contains Timer/Clock functions commonly used by
        TIMAC applications for CC13xx with TIRTOS

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
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
#include <ti/drivers/dpl/ClockP.h>

#include "util_timer.h"

/******************************************************************************
 Constants and Typedefs
 *****************************************************************************/

/*! Adjustment for the timers */
#define UtilTimer_MS_ADJUSTMENT     100

/******************************************************************************
 Public Functions
 *****************************************************************************/

/*!
 Initialize a TIRTOS Timer/Clock instance.

 Public function defined in mac_util.h
 */
ClockP_Handle UtilTimer_construct(ClockP_Struct *pClock, ClockP_Fxn clockCB,
                                 uint32_t clockDuration, uint32_t clockPeriod,
                                 uint8_t startFlag,
                                 uintptr_t arg)
{
    ClockP_Params clockParams;

    /* Convert clockDuration in milliseconds to ticks. */
    uint32_t clockTicks = clockDuration * (1000 / ClockP_getSystemTickPeriod());

    /* Setup parameters. */
    ClockP_Params_init(&clockParams);

    /* Setup argument. */
    clockParams.arg = arg;

    /* If period is 0, this is a one-shot timer. */
    clockParams.period = clockPeriod * (1000 / ClockP_getSystemTickPeriod());

    /*
     Starts immediately after construction if true, otherwise wait for a
     call to start.
     */
    clockParams.startFlag = startFlag;

    /*/ Initialize clock instance. */
    ClockP_construct(pClock, clockCB, clockTicks, &clockParams);

    return ClockP_handle(pClock);
}

/*!
 Destruct a TIRTOS Timer/Clock instance.
 */
void UtilTimer_destruct(ClockP_Struct *pClock)
{
    ClockP_destruct(pClock);
}

/*!
 Start a timer/clock.

 Public function defined in mac_util.h
 */
void UtilTimer_start(ClockP_Struct *pClock)
{
    ClockP_Handle handle = ClockP_handle(pClock);

    /* Start clock instance */
    ClockP_start(handle);
}

/*!
 Determine if a timer/clock is currently active.

 Public function defined in mac_util.h
 */
bool UtilTimer_isActive(ClockP_Struct *pClock)
{
    ClockP_Handle handle = ClockP_handle(pClock);

    /* Start clock instance */
    return ClockP_isActive(handle);
}

/*!
 Stop a timer/clock.

 Public function defined in mac_util.h
 */
void UtilTimer_stop(ClockP_Struct *pClock)
{
    ClockP_Handle handle = ClockP_handle(pClock);

    /* Start clock instance */
    ClockP_stop(handle);
}

/*!
 * @brief   Set a Timer/Clock timeout.
 *
 * @param   timeOut - Timeout value in milliseconds
 */
void UtilTimer_setTimeout(ClockP_Handle handle, uint32_t timeout)
{
    ClockP_setTimeout(handle, (timeout * UtilTimer_MS_ADJUSTMENT));
}

/*!
 * @brief   Get a Timer/Clock timeout.
 *
 * @param   handle - clock handle
 *
 * @return   timeOut - Timeout value in milliseconds
 */
uint32_t UtilTimer_getTimeout(ClockP_Handle handle)
{
    uint32_t timeout;

    timeout = ClockP_getTimeout(handle);

    return (timeout / UtilTimer_MS_ADJUSTMENT);
}

#ifdef USE_DMM
/*!
 * @brief   Set a Timer/Clock callback function and argument.
 *
 * @param   handle - clock handle
 * @param   fxn - clock callback function
 * @param   arg - clock callback function argument
 *
 */
void UtilTimer_setFunc(ClockP_Handle handle, ClockP_Fxn fxn, uintptr_t arg)
{
    // ClockP_setFunc(handle, fxn, arg);
}
#endif
