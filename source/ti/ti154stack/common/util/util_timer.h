/******************************************************************************

 @file timer.h

 @brief This file contains function declarations common to timer/clock
        functions.

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
#ifndef UtilTimer_H
#define UtilTimer_H

/******************************************************************************
 Includes
 *****************************************************************************/
#include <ti/drivers/dpl/ClockP.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 \defgroup Timer Functions
 <BR>
 Timer/Clock functions.
 <BR>
 */

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/*!
 * \ingroup TimerClock
 * @{
 */

/******************************************************************************
 Function Prototypes
 *****************************************************************************/

/*!
 * @brief   Initialize a TIRTOS Clock instance.
 *
 * @param   pClock        - pointer to clock instance structure.
 * @param   clockCB       - callback function upon clock expiration.
 * @param   clockDuration - longevity of clock timer in milliseconds
 * @param   clockPeriod   - duration of a periodic clock, used continuously
 *                          after clockDuration expires.
 * @param   startFlag     - TRUE to start immediately, FALSE to wait.
 * @param   arg           - argument passed to callback function.
 *
 * @return  Clock_Handle  - a handle to the clock instance.
 */
extern ClockP_Handle UtilTimer_construct(ClockP_Struct *pClock,
                                         ClockP_Fxn clockCB,
                                        uint32_t clockDuration,
                                        uint32_t clockPeriod,
                                        uint8_t startFlag,
                                        uintptr_t arg);

/*!
 * @brief   Destruct the clock
 *
 * @param   pClock - clock struct
 */
extern void UtilTimer_destruct(ClockP_Struct *pClock);

/*!
 * @brief   Start a Timer/Clock.
 *
 * @param   pClock - pointer to clock struct
 */
extern void UtilTimer_start(ClockP_Struct *pClock);

/*!
 * @brief   Determine if a clock is currently active.
 *
 * @param   pClock - pointer to clock struct
 *
 * @return  TRUE or FALSE
 */
extern bool UtilTimer_isActive(ClockP_Struct *pClock);

/*!
 * @brief   Stop a Timer/Clock.
 *
 * @param   pClock - pointer to clock struct
 */
extern void UtilTimer_stop(ClockP_Struct *pClock);

/*!
 * @brief   Set a Timer/Clock timeout.
 *
 * @param   timeOut - Timeout value in milliseconds
 */
extern void UtilTimer_setTimeout(ClockP_Handle handle, uint32_t timeout);

/*!
 * @brief   Get a Timer/Clock timeout.
 *
 * @param   handle - clock handle
 *
 * @return   timeOut - Timeout value in milliseconds
 */
extern uint32_t UtilTimer_getTimeout(ClockP_Handle handle);

/*!
 * @brief   Set a Timer/Clock callback function and argument.
 *
 * @param   handle - clock handle
 * @param   fxn - clock callback function
 * @param   arg - clock callback function argument
 *
 */
extern void UtilTimer_setFunc(ClockP_Handle handle, ClockP_Fxn fxn, uintptr_t arg);

/*! @} end group TimerClock */

#ifdef __cplusplus
}
#endif

#endif /* UtilTimer_H */
