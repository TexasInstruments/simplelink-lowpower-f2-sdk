/******************************************************************************

 @file  mac_sleep.c

 @brief Describe the purpose and contents of the file.

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
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#ifndef LIBRARY
#include "ti_wisunfan_features.h"
#endif

/* hal */
#include "hal_types.h"

/* high-level */
#include "mac_pib.h"

/* exported low-level */
#include "mac_low_level.h"

/* low-level specific */
#include "mac_sleep.h"
#include "mac_radio.h"
#include "mac_tx.h"
#include "mac_rx.h"
#include "mac_rx_onoff.h"
#include "mac_symbol_timer.h"

/* target specific */
#include "mac_radio_defs.h"

/* debug */
#include "mac_assert.h"


/* ------------------------------------------------------------------------------------------------
 *                                         Global Variables
 * ------------------------------------------------------------------------------------------------
 */
uint8 macSleepState = MAC_SLEEP_STATE_RADIO_OFF;


/**************************************************************************************************
 * @fn          macSleepWakeUp
 *
 * @brief       Wake up the radio from sleep mode.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macSleepWakeUp(void)
{
  /* don't wake up radio if it's already awake */
  if (macSleepState == MAC_SLEEP_STATE_AWAKE)
  {
    return;
  }

  /* wake up MAC timer */
  MAC_RADIO_TIMER_WAKE_UP();

  /* if radio was completely off, restore from that state first */
  if (macSleepState == MAC_SLEEP_STATE_RADIO_OFF)
  {
    /* turn on radio power (turns on oscillator too) */
    MAC_RADIO_TURN_ON_POWER();

    /* power-up initialization of receive logic */
    macRxRadioPowerUpInit();
  }
  else
  {
    MAC_ASSERT(macSleepState == MAC_SLEEP_STATE_OSC_OFF);

    /* turn on the oscillator */
    MAC_RADIO_TURN_ON_OSC();
  }

  /* update sleep state here before requesting to turn on receiver */
  macSleepState = MAC_SLEEP_STATE_AWAKE;

  /* turn on the receiver if enabled */
  macRxOnRequest();
}


/**************************************************************************************************
 * @fn          macSleep
 *
 * @brief       Puts radio into the selected sleep mode.
 *
 * @param       sleepState - selected sleep level, see #defines in .h file
 *
 * @return      TRUE if radio was successfully put into selected sleep mode.
 *              FALSE if it was not safe for radio to go to sleep.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macSleep(uint8 sleepState)
{
  halIntState_t  s;

  /* disable interrupts until macSleepState can be set */
  HAL_ENTER_CRITICAL_SECTION(s);

  /* assert checks */
  MAC_ASSERT(macSleepState == MAC_SLEEP_STATE_AWAKE); /* radio must be awake to put it to sleep */
  MAC_ASSERT(macRxFilter == RX_FILTER_OFF); /* do not sleep when scanning or in promiscuous mode */

  /* if either RX or TX is active or any RX enable flags are set, it's not OK to sleep */
  if (macRxActive || macRxOutgoingAckFlag || macTxActive || macRxEnableFlags ||
      macSymbolTimerImpending)
  {
    HAL_EXIT_CRITICAL_SECTION(s);
    return(FALSE);
  }

  /* turn off the receiver */
  macRxOff();

  /* update sleep state variable */
  macSleepState = sleepState;

  /* macSleepState is now set, re-enable interrupts */
  HAL_EXIT_CRITICAL_SECTION(s);
  
  /* put MAC timer to sleep */
  MAC_RADIO_TIMER_SLEEP();

  /* put radio in selected sleep mode */
  if (sleepState == MAC_SLEEP_STATE_OSC_OFF)
  {
    MAC_RADIO_TURN_OFF_OSC();
  }
  else
  {
    MAC_ASSERT(sleepState == MAC_SLEEP_STATE_RADIO_OFF); /* unknown sleep state */
    MAC_RADIO_TURN_OFF_POWER();
  }

  /* radio successfully entered sleep mode */
  return(TRUE);
}

#if defined USE_ICALL || defined OSAL_PORT2TIRTOS
/**************************************************************************************************
 * @fn          macCanSleep
 *
 * @brief       Checks whether rado can be put into sleep.
 *              This function is not thread-safe.
 *
 * @param       None
 *
 * @return      TRUE if radio can be put into sleep
 *              FALSE, otherwise.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macCanSleep(void)
{
  /* If either RX or TX is active or any RX enable flags are set,
   * it's not OK to sleep */
  if (macRxActive || macRxOutgoingAckFlag || macTxActive || macRxEnableFlags ||
          macRxCmdActive || macTxCmdActive || macFsCmdActive)
  {
    return(FALSE);
  }

  /* Radio may enter sleep mode */
  return(TRUE);
}
#endif /* defined USE_ICALL || defined OSAL_PORT2TIRTOS */



/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */
#if ((MAC_SLEEP_STATE_AWAKE == MAC_SLEEP_STATE_OSC_OFF) ||  \
     (MAC_SLEEP_STATE_AWAKE == MAC_SLEEP_STATE_RADIO_OFF))
#error "ERROR!  Non-unique state values."
#endif


/**************************************************************************************************
*/
