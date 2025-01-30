/******************************************************************************

 @file osal2tirtos.c

 @brief converts ICAll and OSAL API's to native TIRTOS/POSIX calls

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

/***** Includes *****/
#include "osal_port_timers.h"

#include "stdlib.h"
#include "stdbool.h"

#include <ti/drivers/dpl/ClockP.h>

/***** Defines *****/

/***** Typedefs *****/

typedef struct
{
    ClockP_Handle clockHandle;
    uint8_t taskId;
    uint32_t eventId;
    bool reload;
    void* pNext;
} TimerEntry_t;

/***** Variable declarations *****/
static uint8_t stackTaskID;
static uint32_t stackEventID;

/***** Private variables *****/
static TimerEntry_t* pTimerEntries = NULL;
static TimerEntry_t* pDeleteTimerEntries = NULL;

/***** Private function definitions *****/
static void timerCb(uintptr_t arg);
static uint8_t createTimerEntry(uint8_t taskId, uint32_t eventId, uint32_t timeout, bool reload);
static TimerEntry_t* getTimerEntry(uint8_t taskId, uint32_t eventId);

/***** Public function definitions *****/

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
uint8_t OsalPortTimers_startTimer(uint8_t taskId, uint32_t eventId, uint32_t timeout)
{
    uint8_t status;

    status = createTimerEntry(taskId, eventId, timeout, false);

    return status;
}

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
uint8_t OsalPortTimers_startReloadTimer(uint8_t taskId, uint32_t eventId, uint32_t timeout)
{
    uint8_t status;

    status = createTimerEntry(taskId, eventId, timeout, true);

    return status;
}
  
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
uint8_t OsalPortTimers_stopTimer(uint8_t taskId, uint32_t eventId)
{
    TimerEntry_t* pTimerEntry;

    uintptr_t key;

    //Enter Critial Section
    key = OsalPort_enterCS();

    pTimerEntry = getTimerEntry(taskId, eventId);

    //Stop clock, remove from list and free memory
    if(pTimerEntry != NULL)
    {
        ClockP_stop(pTimerEntry->clockHandle);
        ClockP_delete(pTimerEntry->clockHandle);

        //is it first in list
        if(pTimerEntry == pTimerEntries)
        {
            pTimerEntries = pTimerEntry->pNext;
            OsalPort_free(pTimerEntry);
        }
        else
        {
            //find prev entry
            TimerEntry_t* pTimerEntryPrev = pTimerEntries;
            while(pTimerEntryPrev->pNext != pTimerEntry)
            {
                pTimerEntryPrev = pTimerEntryPrev->pNext;
            }
            pTimerEntryPrev->pNext = pTimerEntry->pNext;
            OsalPort_free(pTimerEntry);
        }
    }
    else
    {
        //Leave Critical Section
        OsalPort_leaveCS(key);

        return OsalPort_INVALIDPARAMETER;
    }

    //Leave Critical Section
    OsalPort_leaveCS(key);

    return OsalPort_SUCCESS;
}

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
uint32_t OsalPortTimers_getTimerTimeout(uint8_t taskId, uint32_t eventId)
{
    TimerEntry_t* pTimerEntry;
    uint32_t timeoutTicks;
    uint32_t timeout = 0; /* timeout in ms */
    uintptr_t key;

    //Enter Critial Section
    key = OsalPort_enterCS();

    pTimerEntry = getTimerEntry(taskId, eventId);

    if(pTimerEntry != NULL)
    {
        timeoutTicks = ClockP_getTimeout(pTimerEntry->clockHandle);
        timeout = timeoutTicks / (1000 / ClockP_getSystemTickPeriod());
    }

    //Leave Critical Section
    OsalPort_leaveCS(key);

    return timeout;
}

/*********************************************************************
 * @fn      OsalPortTimers_cleanUpTimers
 *
 * @brief Clean up inactive Osal Port Timers outside of SWI context
 *
 *
 * @return  Timer entry
 */
void OsalPortTimers_cleanUpTimers(void)
{
  uintptr_t key = OsalPort_enterCS();

  if (pDeleteTimerEntries != NULL) {
    TimerEntry_t *current = pDeleteTimerEntries;
    TimerEntry_t *next;

    // free the entire pDeleteTimerEntries list
    while (current != NULL)
    {
        next = current->pNext;
        ClockP_delete(current->clockHandle);
        OsalPort_free(current);
        current = next;
    }
    pDeleteTimerEntries = NULL;
  }

  OsalPort_leaveCS(key);
}

/*********************************************************************
 * @fn      OsalPortTimers_registerCleanupEvent
 *
 * @brief Set stack task and event ID for timer cleanup event
 *
 * @param   uint8_t    taskId - stack Task ID
 *
 * @return  void
 */
void OsalPortTimers_registerCleanupEvent(uint8_t taskID, uint32_t eventID)
{
  stackTaskID = taskID;
  stackEventID = eventID;
}

/***** Private function definitions *****/

/*********************************************************************
 * @fn      timerCb
 *
 * @brief
 *
 *    This function is the timer callback function send to send events
 *    when the timeout happens.
 *
 *
 * @param   void*    arg - pointer to the Timer Entry that has expired
 *
 * @return  none
 */
static void timerCb(uintptr_t arg)
{
    TimerEntry_t* pTimerEntry = (TimerEntry_t*) arg;

    /* Set event */
    OsalPort_setEvent( pTimerEntry->taskId, pTimerEntry->eventId );

    /* if it is not a reload timer then free the entry */
    if(!pTimerEntry->reload)
    {
        ClockP_stop(pTimerEntry->clockHandle);

        // find the element in the timer list and
        // extract it by updating prev and next ptrs
        if(pTimerEntry == pTimerEntries)
        {
            pTimerEntries = pTimerEntry->pNext;
        }
        else
        {
            TimerEntry_t* pPrevTimerEntry = pTimerEntries;
            while(pPrevTimerEntry->pNext != pTimerEntry)
            {
                pPrevTimerEntry = pPrevTimerEntry->pNext;
            }
            pPrevTimerEntry->pNext = pTimerEntry->pNext;
        }

        // add the element in the removal list, to be freed
        // outside of SWI context
        TimerEntry_t *pDeleteTimerEntry = pDeleteTimerEntries;
        pTimerEntry->pNext = NULL; // remove node from linked list above
        if(pDeleteTimerEntry == NULL)
        {
            pDeleteTimerEntries = pTimerEntry;
        }
        else
        {
            while(pDeleteTimerEntry->pNext != NULL)
            {
                pDeleteTimerEntry = pDeleteTimerEntry->pNext;
            }
            pDeleteTimerEntry->pNext = pTimerEntry;
        }
        
        OsalPort_setEvent( stackTaskID, stackEventID );
    }
}

/*********************************************************************
 * @fn      createTimerEntry
 *
 * @brief
 *
 *    This function is used to create a TIRTOS timer an timer entry.
 *
 * @param   uint8_t    taskId - task ID to post event to when timer expires
 * @param   uint32_t   eventId - event to post
 * @param   uint32_t   timeout - timeout in ms
 * @param   uint32_t   reload  - if timer is reload timer
 *
 * @return  OsalPort status code
 */
static uint8_t createTimerEntry(uint8_t taskId, uint32_t eventId, uint32_t timeout, bool reload)
{
    ClockP_Params clkParams;
    TimerEntry_t* pNewTimerEntry;
    uint8_t status = OsalPort_NO_TIMER_AVAIL;
    uint32_t timeoutTicks = (timeout * (1000 / ClockP_getSystemTickPeriod()));
    uintptr_t key;

    //Enter Critial Section
    key = OsalPort_enterCS();

    //check for existing timer
    pNewTimerEntry = getTimerEntry(taskId, eventId);
    
    if(pNewTimerEntry)
    {
        //reset the time out
        ClockP_stop(pNewTimerEntry->clockHandle);
        ClockP_setTimeout(pNewTimerEntry->clockHandle, timeoutTicks);
        ClockP_start(pNewTimerEntry->clockHandle);
    }
    else
    {
        pNewTimerEntry = OsalPort_malloc(sizeof(TimerEntry_t));

        if(pNewTimerEntry != NULL)
        {
            ClockP_Params_init(&clkParams);
            if(reload)
            {
                clkParams.period = timeoutTicks;
            }
            else
            {
                clkParams.period = 0;
            }

            clkParams.startFlag = true;
            clkParams.arg = (uintptr_t) pNewTimerEntry;

            pNewTimerEntry->clockHandle = ClockP_create(timerCb, timeoutTicks, &clkParams);

            if(pNewTimerEntry->clockHandle != NULL)
            {
                pNewTimerEntry->taskId = taskId;
                pNewTimerEntry->eventId = eventId;
                pNewTimerEntry->reload = reload;
                pNewTimerEntry->pNext = NULL;

                if(pTimerEntries == NULL)
                {
                    pTimerEntries = pNewTimerEntry;
                }
                else
                {
                    TimerEntry_t* pLastTimerEntry = pTimerEntries;
                    while(pLastTimerEntry->pNext != NULL)
                    {
                        pLastTimerEntry = pLastTimerEntry->pNext;
                    }
                    pLastTimerEntry->pNext = pNewTimerEntry;
                }

                ClockP_setTimeout(pNewTimerEntry->clockHandle, timeoutTicks);
                ClockP_start(pNewTimerEntry->clockHandle);

                status = OsalPort_SUCCESS;
            }
            else
            {
                OsalPort_free(pNewTimerEntry);
            }
        }
    }

    //Leave Critical Section
    OsalPort_leaveCS(key);

    return status;
}


/*********************************************************************
 * @fn      getTimerEntry
 *
 * @brief
 *
 *    This function is used to create a TIRTOS timer an timer entry.
 *
 * @param   uint8_t    taskId - task ID to post event to when timer expires
 * @param   uint32_t   eventId - event to post
 *
 * @return  Timer entry
 */
static TimerEntry_t* getTimerEntry(uint8_t taskId, uint32_t eventId)
{
    TimerEntry_t* pTimerEntry;

    pTimerEntry = pTimerEntries;

    /* iterate through entries and find one that matches taskId and eventId */
    while( (pTimerEntry != NULL) &&
           !((pTimerEntry->taskId == taskId) &&
             (pTimerEntry->eventId == eventId)) )
    {
        pTimerEntry = pTimerEntry->pNext;
    }

    return pTimerEntry;
}
