/******************************************************************************

 @file  mac_pwr.c

 @brief This module implements high level procedures for power management.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2005-2023, Texas Instruments Incorporated

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

#include "timac_api.h"
#include "mac_main.h"
#include "mac_pwr.h"
#include "mac_pib.h"
#include "mac_low_level.h"
#include "icall_osal_map_direct.h"
#include "hal_mcu.h"
#include "mac_radio.h"
#include "mac_symbol_timer.h"
#include "mac.h"
#include "mac_radio_defs.h"

/******************************************************************************
  ROM jump table will support
  1. TIMAC ROM image build
  2. TIMAC stack build without ROM image
  3. TIMAC stack with calling ROM image
 *****************************************************************************/
#include "rom_jt_154.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_pwr_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

/* RAT minimum remaining time */
#define RAT_MIN_REMAINING_TIME (300 * MAC_RAT_MHZ )

#define FEATURE_POWER_SAVING

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */

macPwr_t macPwr;


/**************************************************************************************************
 * @fn          macPwrReset
 *
 * @brief       This function initializes the data structures for the pwr module.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macPwrReset(void)
{
  macPwr.mode = MAC_PWR_ON;
}

/**************************************************************************************************
 * @fn          macApiPwrOnReq
 *
 * @brief       This function handles an API power on request event.  If macPwr.mode
 *              indicates the radio is already on it calls the MAC callback with MAC_PWR_ON_CNF
 *              immediately.
 *
 * input parameters
 *
 * @param       pEvent - Pointer to event data.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
MAC_INTERNAL_API void macApiPwrOnReq(macEvent_t *pEvent)
{
  (void)pEvent;  // Intentionally unreferenced parameter
}

/**************************************************************************************************
 * @fn          MAC_PwrOffReq
 *
 * @brief       This direct execute function requests the MAC to power off and radio hardware
 *              and go to sleep.  It first checks if the mac is in idle or polling state by calling
 *              macStateIdleOrPolling() and also whether the receiver is off.  It also checks whether
 *              the next MAC timer expiration is greater than a minimum threshold value.  If
 *              all of the above indicates the MAC can be powered down it calls macRadioPwrOff(),
 *              sets macPwr.mode and returns MAC_SUCCESS.  Otherwise it returns MAC_DENIED.
 *              If macPwr.mode indicates the radio is already off MAC_SUCCESS is returned
 *              immediately.
 *
 * input parameters
 *
 * @param       mode - The desired low power mode.
 *
 * output parameters
 *
 * None.
 *
 * @return      The status of the request, as follows:
 *              MAC_SUCCESS  Operation successful; the MAC is powered off.
 *              MAC_DENIED  The MAC was not able to power off.
 **************************************************************************************************
 */
uint8 MAC_PwrOffReq(uint8 mode)
{
  /* Check if we are already powered down */
  if (macPwr.mode == mode)
  {
    return MAC_SUCCESS;
  }

  /* Verify it is ok to sleep */
  if ((macPwr.mode == MAC_PWR_ON) && MAP_macStateIdleOrPolling() && macSleep(mode))
  {
    macPwr.mode = mode;
    return MAC_SUCCESS;
  }

  return MAC_DENIED;
}

/**************************************************************************************************
 * @fn          MAC_PwrOnReq
 *
 * @brief       This function handles an API power on request event.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void MAC_PwrOnReq(void)
{
  if (macPwr.mode != MAC_PWR_ON)
  {
    macSleepWakeUp();
    macPwr.mode = MAC_PWR_ON;
  }  
}

/**************************************************************************************************
 * @fn          MAC_PwrMode
 *
 * @brief       This function returns the current power mode of the MAC.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      The current power mode of the MAC.
 **************************************************************************************************
 */
uint8 MAC_PwrMode(void)
{
  return macPwr.mode;
}

/**************************************************************************************************
 * @fn          macPwrVote
 *
 * @brief       This function votes for power management state based on entire
 *              MAC internal function states.
 *              This function is thread-safe.
 *
 * input parameters
 *
 * @param       pwrUp     - Indicates if radio power-up is needed
 *              bypassRAT - Indicates if RAT disabling and restoration need to be avoided
 *
 * output parameters
 *
 * None.
 *
 * @return      None
 **************************************************************************************************
 */
MAC_INTERNAL_API void macPwrVote(bool pwrUp, bool bypassRAT)
{
}


/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */

#if (MAC_PWR_SLEEP_LITE != MAC_SLEEP_STATE_OSC_OFF)
#error "ERROR:  Mismatch in paramter values."
#endif

#if (MAC_PWR_SLEEP_DEEP != MAC_SLEEP_STATE_RADIO_OFF)
#error "ERROR:  Mismatch in paramter values."
#endif


/**************************************************************************************************
*/
