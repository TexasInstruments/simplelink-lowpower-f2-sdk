/******************************************************************************

 @file  mac_rx_onoff.c

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
#include "hal_defs.h"
#include "hal_types.h"

#include "mac.h"
#include "mac_pib.h"

/* exported low-level */
#include "mac_low_level.h"

/* low-level specific */
#include "mac_rx_onoff.h"
#include "mac_rx.h"
#include "mac_tx.h"
#include "mac_radio.h"

/* target specific */
#include "mac_radio_defs.h"

/* debug */
#include "mac_assert.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_rx_onoff_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

#include "rom_jt_154.h"

/* ------------------------------------------------------------------------------------------------
 *                                         Global Variables
 * ------------------------------------------------------------------------------------------------
 */
volatile uint8 macRxEnableFlags;


/**************************************************************************************************
 * @fn          macRxOnOffInit
 *
 * @brief       Initialize variables for rx on/off module.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxOnOffInit(void)
{
  macRxEnableFlags = 0;

  /* Power management state may change. Hence, vote. */
  MAC_PWR_VOTE_NO_SWI(FALSE);
}


/**************************************************************************************************
 * @fn          macRxEnable
 *
 * @brief       Set enable flags and then turn on receiver.
 *
 * @param       flags - byte containing rx enable flags to set
 *
 * @return      TRUE if successful, FALSE otherwise
 **************************************************************************************************
 */
MAC_INTERNAL_API bool macRxEnable(uint8 flags)
{
  MAC_ASSERT(flags != 0); /* rx flags not affected */
  
  /* set enable flags and then turn on receiver */
  macRxEnableFlags |= flags;
  DBG_PRINT2(DBGSYS, "macRxEnable(0x%02X), macRxEnableFlags=0x%02X", flags, macRxEnableFlags);

  return macRxOn();
}


/**************************************************************************************************
 * @fn          macRxSoftEnable
 *
 * @brief       Set enable flags but don't turn on the receiver.  Useful to leave the receiver
 *              on after a transmit, but without turning it on immediately.
 *
 * @param       flags - byte containing rx enable flags to set
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxSoftEnable(uint8 flags)
{
  halIntState_t  s;

  MAC_ASSERT(flags != 0); /* rx flags not affected */

  /* set the enable flags but do not turn on the receiver */
  HAL_ENTER_CRITICAL_SECTION(s);
  macRxEnableFlags |= flags;
  DBG_PRINT2(DBGSYS, "macRxSoftEnable(0x%02X), macRxEnableFlags=0x%02X", flags, macRxEnableFlags);
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macRxDisable
 *
 * @brief       Clear indicated rx enable flags.  If all flags are clear, turn off receiver
 *              unless there is an active receive or transmit.
 *
 * @param       flags - byte containg rx enable flags to clear
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxDisable(uint8 flags)
{
  halIntState_t  s;

  MAC_ASSERT(flags != 0); /* rx flags not affected */
  
  /* clear the indicated flags */
  HAL_ENTER_CRITICAL_SECTION(s);
  macRxEnableFlags &= (flags ^ 0xFF);
  DBG_PRINT2(DBGSYS, "macRxDisable(0x%02X), macRxEnableFlags=0x%02X", flags, macRxEnableFlags);
  HAL_EXIT_CRITICAL_SECTION(s);

  /* turn off the radio if it is allowed */
  macRxOffRequest();

  /* Power management state may change. Hence, vote. */
  MAC_PWR_VOTE_NO_SWI(FALSE);
}

/**************************************************************************************************
 * @fn          macRxSoftDisable
 *
 * @brief       Clear given RX enable flags but don't turn off the receiver.
 *              Useful to turn the radio off when macRxOffRequest() is called later.
 *
 * @param       flags - byte containing rx enable flags to clear
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxSoftDisable(uint8 flags)
{
  halIntState_t  s;

  MAC_ASSERT(flags != 0); /* rx flags not affected */
  
  /* clear the indicated flags */
  HAL_ENTER_CRITICAL_SECTION(s);
  macRxEnableFlags &= (flags ^ 0xFF);
  DBG_PRINT2(DBGSYS, "macRxSoftDisable(0x%02X), macRxEnableFlags=0x%02X", flags, macRxEnableFlags);
  HAL_EXIT_CRITICAL_SECTION(s);
}

/**************************************************************************************************
 * @fn          macRxHardDisable
 *
 * @brief       Clear all enable flags and turn off receiver.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxHardDisable(void)
{
  DBG_PRINT0(DBGSYS, "macRxHardDisable()");
  macRxEnableFlags = 0;
  MAC_DEBUG_TURN_OFF_RX_LED();
  macRxOff();

  /* force off and clean up */
  macRxHaltCleanup();

  /* Power management state may change. Hence, vote. */
  MAC_PWR_VOTE_NO_SWI(FALSE);
}

/**************************************************************************************************
 * @fn          macRxOnRequest
 *
 * @brief       Turn on the receiver if any rx enable flag is set.
 *
 * @param       none
 *
 * @return      TRUE if successful, FALSE otherwise
 **************************************************************************************************
 */
MAC_INTERNAL_API bool macRxOnRequest(void)
{
  bool  rxOnSuccess = FALSE;

  DBG_PRINT1(DBGSYS, "macRxOnRequest(): macRxEnableFlags=0x%02X", macRxEnableFlags);

  if (macRxEnableFlags)
  {
    rxOnSuccess = macRxOn();
  }

  return rxOnSuccess;
}


/**************************************************************************************************
 * @fn          macRxOffRequest
 *
 * @brief       Turn off receiver if permitted.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxOffRequest(void)
{
  halIntState_t  s;
  uint8 doneFlag = 0;

  DBG_PRINT1(DBGSYS, "macRxOffRequest(): macRxEnableFlags=0x%02X", macRxEnableFlags);
  HAL_ENTER_CRITICAL_SECTION(s);
  if (!macRxEnableFlags)
  {
    doneFlag = !MAC_RX_IS_PHYSICALLY_ACTIVE() && !MAC_TX_IS_PHYSICALLY_ACTIVE();
    if (doneFlag)
    {
      macRxOff();
    }
  }
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macRxOn
 *
 * @brief       Turn on the receiver if it's not already on.
 *
 * @param       none
 *
 * @return      TRUE if successful, FALSE otherwise
 **************************************************************************************************
 */
MAC_INTERNAL_API bool macRxOn(void)
{
  halIntState_t  s;
  bool rxOnSuccess;
  uint8_t macRxCmdActiveOrg;

  HAL_ENTER_CRITICAL_SECTION(s);
  macRxCmdActiveOrg = macRxCmdActive;

  if (macRxCmdActive != TRUE)
  {
    macRxCmdActive = TRUE;
    HAL_EXIT_CRITICAL_SECTION(s);
    
    /* Power management state may change. Hence, vote. */
    MAC_PWR_VOTE_NO_SWI(TRUE);
  }
  else
  {
    HAL_EXIT_CRITICAL_SECTION(s);
  }

  rxOnSuccess = MAC_RADIO_RX_ON();

  HAL_ENTER_CRITICAL_SECTION(s);
  if (rxOnSuccess != TRUE)
  {
    DBG_PRINT0(DBGSYS, "MAC_RADIO_RX_ON() failure");
    MAC_DEBUG_TURN_OFF_RX_LED();
    
    macRxCmdActive = macRxCmdActiveOrg;
    HAL_EXIT_CRITICAL_SECTION(s);
  }
  else
  {
    HAL_EXIT_CRITICAL_SECTION(s);
    DBG_PRINT0(DBGSYS, "MAC_RADIO_RX_ON()");
    MAC_DEBUG_TURN_ON_RX_LED();
  }
  
  /* Power management state may change. Hence, vote.
   * Note that even if radio is active for RX,
   * CM3 can go to sleep if there is nothing to do but
   * waiting for the RX callback.
   */
  MAC_PWR_VOTE_NO_SWI(FALSE);

  return rxOnSuccess;
}


/**************************************************************************************************
 * @fn          macRxOff
 *
 * @brief       Turn off the receiver if it's not already off.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macRxOff(void)
{
    if (MAC_FH_ENABLED)
    {
        // In FH mode, abort RX instead of gracefully stopping the Command
        MAC_RADIO_FH_RXTX_OFF();
    }
    else
    {
        MAC_RADIO_RXTX_OFF();
    }

    if(numRxPostCmd)
        numRxPostCmd--;
    MAC_DEBUG_TURN_OFF_RX_LED();
    DBG_PRINT0(DBGSYS, "MAC_RADIO_RXTX_OFF()");
}


/**************************************************************************************************
*/
