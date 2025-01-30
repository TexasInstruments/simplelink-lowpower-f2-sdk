/******************************************************************************
 @file  osal_port_timers.h

 @brief This API maps osal_timers used by the Stacks to TIRTOS API's

 Group: WCS, LPC, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2004-2025, Texas Instruments Incorporated
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

#ifndef OsalPortTimers_H
#define OsalPortTimers_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"

#include "osal_port.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/* the unit is chosen such that the 320us tick equivalent can fit in
 * 32 bits.
 */
#define OsalPortTimers_TIMERS_MAX_TIMEOUT 0x28f5c28e /* unit is ms*/

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      OsalPortTimers_registerCleanupEvent
 *
 * @brief Set stack task and event ID for timer cleanup event
 *
 * @param   uint8_t    taskId - stack Task ID
 *
 * @return  void
 */
extern void OsalPortTimers_registerCleanupEvent(uint8_t taskID, uint32_t eventID);

/*********************************************************************
 * @fn      OsalPortTimers_cleanUpTimers
 *
 * @brief Clean up inactive Osal Port Timers outside of SWI context
 *
 *
 * @return  Timer entry
 */
extern void OsalPortTimers_cleanUpTimers(void);

/*********************************************************************
 * @fn      OsalPortTimers_startTimer
 *
 * @brief
 *
 *    This function is used to create and start a TIRTOS timer.
 *
 *
 * @param   uint8_t    taskId - task ID to post event to when timer expires
 * @param   uint32_t   eventId - event to post
 * @param   uint32_t   timeout - timeout in ms
 *
 * @return  OsalPort status code
 */
extern uint8_t OsalPortTimers_startTimer(uint8_t taskId, uint32_t eventId, uint32_t timeout);

/*********************************************************************
 * @fn      OsalPortTimers_startReloadTimer
 *
 * @brief
 *
 *    This function is used to create and start a TIRTOS timer that reloads
 *
 *
 * @param   uint8_t    taskId - task ID to post event to when timer expires
 * @param   uint32_t   eventId - event to post
 * @param   uint32_t   timeout - timeout in ms
 *
 * @return  OsalPort status code
 */
extern uint8_t OsalPortTimers_startReloadTimer(uint8_t taskId, uint32_t eventId, uint32_t timeout);
  
/*********************************************************************
 * @fn      OsalPortTimers_stopTimer
 *
 * @brief
 *
 *    This function is used to create and stop a TIRTOS timer
 *
 *
 * @param   uint8_t    taskId - task ID to post event to when timer expires
 * @param   uint32_t   eventId - event to post
 *
 * @return  OsalPort status code
 */
extern uint8_t OsalPortTimers_stopTimer(uint8_t taskId, uint32_t eventId);

/*********************************************************************
 * @fn      OsalPortTimers_stopTimer
 *
 * @brief
 *
 *    This function is used to create and stop a TIRTOS timer
 *
 *
 * @param   uint8_t    taskId - task ID to post event to when timer expires
 * @param   uint32_t   eventId - event to post
 *
 * @return  OsalPort status code
 */
extern uint32_t OsalPortTimers_getTimerTimeout(uint8_t taskId, uint32_t eventId); 


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* OsalPortTimers_H */
