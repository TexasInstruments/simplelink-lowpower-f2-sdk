/******************************************************************************

 @file  mac_timer.c

 @brief Timer implementation for high level MAC.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2006-2023, Texas Instruments Incorporated

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

/* ------------------------------------------------------------------------------------------------
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */

#ifndef LIBRARY
#include "ti_wisunfan_features.h"
#endif

#include "hal_types.h"
#include "hal_mcu.h"
#include "mac_spec.h"
#include "mac_low_level.h"
#include "mac_radio.h"
#include "mac_assert.h"
#include "mac_timer.h"
#include "mac_symbol_timer.h"
#include "mac_mgmt.h"
#include "hal_defs.h"
#include "mac_pib.h"
#include "mac.h"
#include "mac_radio_defs.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_timer_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

#include "rom_jt_154.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */

/* adjust next timeout to expire this many symbols early */
// TODO: Verify symbol works here - this value may be too small
#define MAC_TIMER_NEXT_TIMEOUT_ADJ    1

#ifndef MAC_TIMER_INTEGRITY_CHECKS
#define MAC_TIMER_INTEGRITY_CHECKS    FALSE
#endif

/* ------------------------------------------------------------------------------------------------
 *                                            Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                         Local Variables
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                         Local Functions
 * ------------------------------------------------------------------------------------------------
 */

void macTimerUpdateSymbolTimer(void);
void macTimerRecalcUnaligned(int32 adjust, macTimer_t *pTimer);
void macTimerAddTimer(macTimer_t *pTimer, macTimerHeader_t *pList);
uint8 macTimerRemoveTimer(macTimer_t *pTimer, macTimerHeader_t *pList);

/**************************************************************************************************
 * @fn          macTimerInit
 *
 * @brief       Initializes the timer system.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTimerInit(void)
{
  timerUnalignedQ.pNext = NULL;
  timerAlignedQ.pNext = NULL;

  /* MAC timer should be reset completly between startup and scan */  
  macTimerRollover    = MAC_SYMBOL_TIMER_DEFAULT_ROLLOVER;
  macTimerNewRollover = MAC_SYMBOL_TIMER_DEFAULT_ROLLOVER;
  macTimerActive      = NULL;
}

/**************************************************************************************************
 * @fn          macTimerUpdateSymbolTimer
 *
 * @brief       Sets macTimerActive and the symbol trigger for the next timer.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macTimerUpdateSymbolTimer(void)
{
  int32 time;

  macTimerActive = NULL;

  /* set active timer to first unaligned timer before the rollover */
  if (timerUnalignedQ.pNext &&
     (timerUnalignedQ.pNext->symbol < macTimerRollover))
  {
    time = macSymbolTimerCount();

    /* if current time is less than next timer expiration it means the timer already expired
     * but could not be processed before realignment or another event cancelled the overflow
     * compare; set the timer to expire now as a workaround
     */
    if (timerUnalignedQ.pNext->symbol < time)
    {
      timerUnalignedQ.pNext->symbol = time;
    }
    macTimerActive = timerUnalignedQ.pNext;
  }

  /* check if the first aligned timer comes sooner */
  /* timer should be active only when timerAlignedQ.pNext->symbol < macTimerRollover */
  if (timerAlignedQ.pNext)
  {
    if (!macTimerActive || (timerAlignedQ.pNext->symbol < macTimerActive->symbol))
    {
      macTimerActive = timerAlignedQ.pNext;
    }
  }

  /* set the trigger */
  if (macTimerActive && macTimerActive->symbol < macTimerRollover)
  {
    macSymbolTimerSetTrigger(macTimerActive->symbol);
  }
  else
  {
    macSymbolTimerSetTriggerRollover();
    /* timer is not active if symbol > macTimerRollover, reset back to NULL */
    macTimerActive = NULL;
  }
}

/**************************************************************************************************
 * @fn          macTimerUpdateSymbolTimer
 *
 * @brief       Sets macTimerActive and the symbol trigger for the next timer.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macTimerGetNextTimer(void)
{
  int32 time;

  macTimerActive = NULL;

  /* set active timer to first unaligned timer before the rollover */
  if (timerUnalignedQ.pNext &&
     (timerUnalignedQ.pNext->symbol < macTimerRollover))
  {
    time = macSymbolTimerCount();

    /* if current time is less than next timer expiration it means the timer already expired
     * but could not be processed before realignment or another event cancelled the overflow
     * compare; set the timer to expire now as a workaround
     */
    if (timerUnalignedQ.pNext->symbol < time)
    {
      timerUnalignedQ.pNext->symbol = time;
    }
    macTimerActive = timerUnalignedQ.pNext;
  }

  /* check if the first aligned timer comes sooner */
  /* timer should be active only when timerAlignedQ.pNext->symbol < macTimerRollover */
  if (timerAlignedQ.pNext)
  {
    if (!macTimerActive || (timerAlignedQ.pNext->symbol < macTimerActive->symbol))
    {
      macTimerActive = timerAlignedQ.pNext;
    }
  }
}
/**************************************************************************************************
 * @fn          macSymbolTimerTriggerCallback
 *
 * @brief       Callback called on the symbol trigger
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macSymbolTimerTriggerCallback(void)
{
  macTimer_t   *pTimer;
  uint8         parameter;
  void          (*pFunc)(uint8);

  if (macTimerActive)
  {
    MAC_ASSERT(!(macTimerActive->symbol > macTimerRollover));

    if (macTimerActive && (uint32) macTimerActive->symbol > macSymbolTimerCount())
    {
      /*If an interrupt happened and there is a valid timer with
        value set to time in the future, timer needs to be set again,
        as interrupt is disabled once this Hwi/Swi is triggered.
        This case happens due to the while loop in the below else condition
        where multiple timers can be removed in a single callback
        (due to the callback execution time and the timers too close to each other)*/
      MAP_macTimerUpdateSymbolTimer();
    }
    else
    {
        while (macTimerActive)
        {
            MAC_ASSERT(macTimerActive->symbol >= 0);

            /* remove active timer and call action */
            pTimer = macTimerActive;
            /* store callback and its parameter to local variables */
            parameter = pTimer->parameter;
            pFunc     = pTimer->pFunc;
            if(macTimerActive->symbol <= macSymbolTimerCount())
            {
                /* macTimer update queue */
                MAP_macTimerUpdActive(pTimer);
                /* call the callback even if it may be out-dated */
                (*pFunc)(parameter);
            }
            else
            {
                /* set the trigger */
                if (macTimerActive && macTimerActive->symbol < macTimerRollover)
                {
                    macSymbolTimerSetTrigger(macTimerActive->symbol);
                }
                else
                {
                    macSymbolTimerSetTriggerRollover();
                    /* timer is not active if symbol > macTimerRollover, reset back to NULL */
                    macTimerActive = NULL;
                }
                break;
            }
        }
    }
  }
}

/**************************************************************************************************
 * @fn          macTimerRecalcUnaligned
 *
 * @brief       Called to adjust the unaligned timers
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macTimerRecalcUnaligned(int32 adjust, macTimer_t *pTimer)
{
  /* add adjustment to aligned timers */
  while(pTimer)
  {
    pTimer->symbol += adjust;
    if ((pTimer == macTimerActive) && (pTimer->symbol < 0))
    {
      /* don't adjust unaligned timer if it is active timer and its symbol count goes nonzero */
      pTimer->symbol = 0;
    }
    MAC_ASSERT(pTimer->symbol >= 0);
    pTimer = pTimer->pNext;
  }
}

/**************************************************************************************************
 * @fn          macSymbolTimerRolloverCallback
 *
 * @brief       Called when the symbol reaches the rollover.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macSymbolTimerRolloverCallback(void)
{
  macTimer_t    *pTimer;

  pTimer = timerAlignedQ.pNext;

  /* adjustment to unaligned timers */
  MAP_macTimerRecalcUnaligned(-(macTimerRollover), timerUnalignedQ.pNext);

  /* adjustment to aligned timers */
  while(pTimer)
  {
    if (pTimer->symbol >= macTimerRollover)
    {
      pTimer->symbol -= macTimerRollover;
    }

    pTimer = pTimer->pNext;
  }

  /* Deal with possible synchronized change in rollover */
  if (macTimerRollover != macTimerNewRollover)
  {
    macTimerRollover = macTimerNewRollover;
    macSymbolTimerSetRollover(macTimerRollover);
  }

  MAP_macTimerUpdateSymbolTimer();
}

/**************************************************************************************************
 * @fn          macTimerCheckUnAlignedQ
 *
 * @brief       Checks if a timer is in the UnAlignedQ
 *
 * @param       pTimer - Pointer to timer.
 *
 * @return      TRUE/FALSE
 **************************************************************************************************
 */
bool macTimerCheckUnAlignedQ(macTimer_t *pTimer)
{
  macTimer_t    * p;
  halIntState_t    halIntState;
  macTimerHeader_t *pList = &timerUnalignedQ;


  HAL_ENTER_CRITICAL_SECTION(halIntState);

  p = pList->pNext;

  /* find position for this timer in sorted list */
  while (p != NULL)
  {
    if (pTimer == p)
    {
      HAL_EXIT_CRITICAL_SECTION(halIntState);
      return true;
    }
    p = p->pNext;
  }

  HAL_EXIT_CRITICAL_SECTION(halIntState);
  return false;
}
/**************************************************************************************************
 * @fn          macTimerAddTimer
 *
 * @brief       Adds a timer to a list
 *
 * @param       pTimer - Pointer to timer to add.
 * @param       pList - Pointer to timer linked list.
 * @param       symbols - timer expiraton.
 *
 * @return      none
 **************************************************************************************************
 */
void macTimerAddTimer(macTimer_t *pTimer, macTimerHeader_t *pList)
{
  macTimer_t    * p;
  macTimer_t    * pPrev;
  halIntState_t    halIntState;
  pTimer->pNext = NULL;

  /* Make sure RAT is alive before calling timer functions
   * within the critical section.
   */
  MAC_PWR_VOTE_NO_SWI(TRUE);

  HAL_ENTER_CRITICAL_SECTION(halIntState);

  DBG_PRINTL1(DBGSYS, "macTimerAddTimer(): pTimer = 0x%08X", (uint32_t) pTimer);
  DBG_PRINTL1(DBGSYS, "macTimerAddTimer(): timeout = %lu", pTimer->symbol);

  /* Check if the timer is already in the list and remove it */
  macTimerRemoveTimer(pTimer, pList);

#if MAC_TIMER_INTEGRITY_CHECKS == TRUE
  {
    MAC_ASSERT(pTimer != NULL); /* null timer */
    p = pList->pNext;
    while (p != NULL)
    {
      MAC_ASSERT(p != pTimer); /* timer in use */
      p = p->pNext;
    }
  }
#endif

  pPrev = (macTimer_t *) pList;
  p = pList->pNext;

  /* find position for this timer in sorted list */
  while (p != NULL)
  {
    if (pTimer->symbol < p->symbol)
    {
      break;
    }

    pPrev = p;
    p = p->pNext;
  }

  /* insert timer into list */
  pPrev->pNext  = pTimer;
  pTimer->pNext = p;

  /* if this is the first timer in list, update symbol timer */
  if (pList->pNext == pTimer)
  {
    MAP_macTimerUpdateSymbolTimer();
  }

  HAL_EXIT_CRITICAL_SECTION(halIntState);
}

/**************************************************************************************************
 * @fn          macTimerAligned
 *
 * @brief       Adds an aligned timer.
 *
 * @param       pTimer - Pointer to timer to add.
 * @param       symbols - timer expiraton.
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTimerAligned(macTimer_t *pTimer, uint32 symbols)
{

  if (symbols <= macSymbolTimerCount())
  {
    pTimer->symbol = symbols + macTimerRollover;
  }
  else
  {
    pTimer->symbol = symbols;
  }

  /* add the timer */
  MAP_macTimerAddTimer(pTimer, &timerAlignedQ);
}

/**************************************************************************************************
 * @fn          macTimer
 *
 * @brief       Adds an unaligned timer.
 *
 * @param       pTimer - Pointer to timer to add.
 * @param       symbols - timer expiraton.
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTimer(macTimer_t *pTimer, uint32 symbols)
{
  /* convert to aligned time */
  pTimer->symbol = symbols + macSymbolTimerCount();

  /* add the timer */
  MAP_macTimerAddTimer(pTimer, &timerUnalignedQ);
}

/**************************************************************************************************
 * @fn          macTimerRemoveTimer
 *
 * @brief       Removes a timer from a list
 *
 * @param       pTimer - Pointer to timer to remove.
 * @param       pList - Pointer to timer linked list.
 *
 * @return      none
 **************************************************************************************************
 */
uint8 macTimerRemoveTimer(macTimer_t *pTimer, macTimerHeader_t *pList)
{
  macTimer_t     * pPrev;
  macTimer_t     * p;

  pPrev = (macTimer_t *) pList;
  p = pList->pNext;

  /* search for and remove the timer */
  while (p != NULL)
  {
    if (p == pTimer)
    {
      pPrev->pNext = p->pNext;
      return TRUE;
    }

    pPrev = p;
    p = p->pNext;
  }

  return FALSE;
}

/**************************************************************************************************
 * @fn          macTimerCancel
 *
 * @brief       Cancels an aligned or unaligned timer
 *
 * @param       pTimer - Pointer to timer to remove.
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTimerCancel(macTimer_t * pTimer)
{
  halIntState_t    halIntState;

  /* Make sure RAT is active before calling timer functions
   * within the critical section
   */

  MAC_PWR_VOTE_NO_SWI(TRUE);

  HAL_ENTER_CRITICAL_SECTION(halIntState);

  DBG_PRINTL1(DBGSYS, "macTimerCancel(): pTimer = 0x%08X", (uint32_t) pTimer);

  /* first check the unaligned timers */
  if (!MAP_macTimerRemoveTimer(pTimer, &timerUnalignedQ))
  {
    /* if not found in unaligned timers, check aligned */
    MAP_macTimerRemoveTimer(pTimer, &timerAlignedQ);
  }

  if (pTimer == macTimerActive)
  {
    /*disable the RAT channel being used */
    macRatDisableChannelB();
    macTimerUpdateSymbolTimer();
  }

  HAL_EXIT_CRITICAL_SECTION(halIntState);
}

/**************************************************************************************************
 * @fn          macTimerUpdActive
 *
 * @brief       Updates the macActiveTimer to the next one from the queueu
 *
 * @param       pTimer - Pointer to timer to remove.
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTimerUpdActive(macTimer_t * pTimer)
{
  halIntState_t    halIntState;

  /* Make sure RAT is active before calling timer functions
   * within the critical section
   */


  HAL_ENTER_CRITICAL_SECTION(halIntState);

  DBG_PRINTL1(DBGSYS, "macTimerCancel(): pTimer = 0x%08X", (uint32_t) pTimer);

  /* first check the unaligned timers */
  if (!MAP_macTimerRemoveTimer(pTimer, &timerUnalignedQ))
  {
    /* if not found in unaligned timers, check aligned */
    MAP_macTimerRemoveTimer(pTimer, &timerAlignedQ);
  }

  if (pTimer == macTimerActive)
  {
      MAP_macTimerGetNextTimer();
  }

  HAL_EXIT_CRITICAL_SECTION(halIntState);
}

/**************************************************************************************************
 * @fn          macTimerGetTime
 *
 * @brief       Gets the current symbol count.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API uint32 macTimerGetTime(void)
{
  return macSymbolTimerCount();
}

/**************************************************************************************************
 * @fn          macTimerStart
 *
 * @brief       Called to set the beacon order and initial beacon time.
 *
 * @param       initTime - Initialize symbol timer count to this time.
 * @param       beaconOrder - Current network beacon order.
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTimerStart(uint32 initTime, uint8 beaconOrder)
{
  macTimerNewRollover = macTimerRollover = macTimerRolloverValue[beaconOrder];
  macSymbolTimerSetCount(initTime);
  macSymbolTimerSetRollover(macTimerRollover);
}

/**************************************************************************************************
 * @fn          macTimerRealign
 *
 * @brief       Called to realign the beacon period
 *
 * @param       pRxBeacon - Pointer to beacon to align with.
 * @param       beaconOrder - Current network beacon order.
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTimerRealign(macRx_t *pRxBeacon, uint8 beaconOrder)
{
  int32 timerAdjust;
  halIntState_t    halIntState;

  /* Make sure RAT is active before calling timer functions
   * within the critical section
   */
  MAC_PWR_VOTE_NO_SWI(TRUE);

  HAL_ENTER_CRITICAL_SECTION(halIntState);

  DBG_PRINT0(DBGSYS, "macTimerRealign()");

  /* Stop any active trigger */
  macSymbolTimerSetTriggerRollover();

  /* realign the symbol timer */
  timerAdjust = macSymbolTimerRealign(pRxBeacon);

  /* set the rollover to the beacon order */
  macTimerNewRollover = macTimerRollover = macTimerRolloverValue[beaconOrder];
  macSymbolTimerSetRollover(macTimerRollover);

  /* adjust unaligned timers */
  MAP_macTimerRecalcUnaligned(timerAdjust, timerUnalignedQ.pNext);

  /* Set the next timer */
  MAP_macTimerUpdateSymbolTimer();

  HAL_EXIT_CRITICAL_SECTION(halIntState);
}

/**************************************************************************************************
 * @fn          macTimerSyncRollover
 *
 * @brief       Called to change the rollover to a new value synchroneously on the next rollover
 *
 * @param       beaconOrder - Current network beacon order.
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTimerSyncRollover(uint8 beaconOrder)
{
  macTimerNewRollover = macTimerRolloverValue[beaconOrder];
}

/**************************************************************************************************
 * @fn          macTimerSetRollover
 *
 * @brief       Called to change the rollover to a new value immediately
 *
 * @param       beaconOrder - Current network beacon order.
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macTimerSetRollover(uint8 beaconOrder)
{
  halIntState_t    halIntState;

  HAL_ENTER_CRITICAL_SECTION(halIntState);

  /* set the rollover to the beacon order */
  macTimerNewRollover = macTimerRollover = macTimerRolloverValue[beaconOrder];

  uint32 curSymbol = RF_getCurrentTime() / MAC_SYMBOL_TO_RAT_RATIO;
  if((macPrevPeriodRatCount == 0) && (curSymbol > macTimerRollover))
  {
      macPrevPeriodRatCount = (curSymbol/macTimerRollover)*macTimerRollover*MAC_SYMBOL_TO_RAT_RATIO;
  }

  macSymbolTimerSetRollover(macTimerRollover);

  HAL_EXIT_CRITICAL_SECTION(halIntState);
}

/**************************************************************************************************
 * @fn          MAC_PwrNextTimeout
 *
 * @brief       This function returns the next MAC timer expiration in 320 usec units.  If no
 *              timer is running it returns zero.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      The next MAC timer expiration or zero.
 **************************************************************************************************
*/
uint32 MAC_PwrNextTimeout(void)
{
  uint32        trigger;
  uint32        count;
  halIntState_t intState;

  if (timerUnalignedQ.pNext != NULL || timerAlignedQ.pNext != NULL)
  {
    HAL_ENTER_CRITICAL_SECTION(intState);

    /* get trigger */
    trigger = macSymbolTimerGetTrigger();

    /* get current symbol count */
    count = macSymbolTimerCount();

    HAL_EXIT_CRITICAL_SECTION(intState);

    /* adjust count, reducing next timeout for MAC power up time */
    count += MAC_TIMER_NEXT_TIMEOUT_ADJ;

    /* count should be less than trigger */
    if (count < trigger)
    {
      /* return time until next expiration */
      return (trigger - count);
    }
    else
    {
      /* If trigger is not less than count, then count was read just as it hit the trigger.
       * We cannot sleep so return impossibly small expiration time that will prevent sleep.
       */
      return 1;
    }

  }
  else
  {
    /* no timer is running */
    return 0;
  }
}
