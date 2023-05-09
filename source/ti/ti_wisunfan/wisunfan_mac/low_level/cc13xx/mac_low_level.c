/******************************************************************************

 @file  mac_low_level.c

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

/* PRCM */
#include "hw_prcm.h"

/* driverlib */
#include "hw_types.h"
#include "interrupt.h"

/* hal */
#include "hal_defs.h"
#include "hal_types.h"
#include "hal_mcu.h"

#include "saddr.h"

/* mailbox */
#include "mac.h"

/* exported low-level */
#include "mac_low_level.h"

/* low-level specific */
#include "mac_radio.h"
#include "mac_rx.h"
#include "mac_tx.h"
#include "mac_rx_onoff.h"
#include "mac_symbol_timer.h"

/* high-level specific */
#include "mac_main.h"
#include "mac_pib.h"

/* target specific */
#include "mac_radio_defs.h"

#ifndef OSAL_PORT2TIRTOS
#include <icall.h>
#include <icall_cc26xx_defs.h>
#include <hw_rfc_pwr.h>
#else
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <hw_rfc_pwr.h>
#endif /* OSAL_PORT2TIRTOS */

/* debug */
#include "mac_assert.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE mac_low_level_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

#include "rf_mac_api.h"
#include "mac_settings.h"

#include "rom_jt_154.h"


/**************************************************************************************************
 * @fn          macLowLevelInit
 *
 * @brief       Initialize low-level MAC.  Called only once on system power-up.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macLowLevelInit(void)
{
  /* Radio hardware initialziation */
  macRadioInit();

  /* Radio software initialziation */
  macRadioSwInit();

  /* At least any one non-dir/imm command should be posted to the RF driver
   * after initialization in order to post a dir/imm command.
   */
  macSetupFsCmd(pMacPib->logicalChannel);
  MAP_macPwrVote(TRUE, FALSE);
    
  /* initialize mcu before anything else */
  MAC_RADIO_MCU_INIT();

  /* Power vote */
  macRxOnOffInit();
  /* Initialize symbol timers */
  macSymbolTimerInit();
}

/**************************************************************************************************
 * @fn          macLowLevelBufferInit
 *
 * @brief       Initialize low-level MAC.  Called only once on system power-up. OSAL must be
 *              initialized before this function is called.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macLowLevelBufferInit(void)
{
  macRxInit();
  macTxInit();
}


/**************************************************************************************************
 * @fn          macLowLevelReset
 *
 * @brief       Reset low-level MAC.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macLowLevelReset(void)
{
}


/**************************************************************************************************
 * @fn          macLowLevelResume
 *
 * @brief       Resume the low-level MAC after a successful return from macLowLevelYield.
 *              Note: assuming interrupts are disabled.
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
MAC_INTERNAL_API void macLowLevelResume(void)
{
#ifdef COMBO_MAC
  macLowLevelBufferInit();
#endif
  macRadioInit();
}


/**************************************************************************************************
 * @fn          macLowLevelYield
 *
 * @brief       Check whether or not the low-level MAC is ready to yield.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      TRUE or FALSE whether the low-level MAC is ready to yield.
 **************************************************************************************************
 */
MAC_INTERNAL_API bool macLowLevelYield(void)
{
  bool rtrn = TRUE;
  halIntState_t  s;
  HAL_ENTER_CRITICAL_SECTION(s);

  // If RX or TX is active or any RX enable flags are set, it's not OK to yield.
  if (macRxActive || macRxOutgoingAckFlag || macTxActive || (macRxEnableFlags & ~MAC_RX_WHEN_IDLE))
  {
    rtrn = FALSE;
  }

  HAL_EXIT_CRITICAL_SECTION(s);
  return rtrn;
}


/**************************************************************************************************
 * @fn          macLowLevelDiags
 *
 * @brief       Increments a specified diagnostic counter (stored in the PIB).
 *
 * @param       pibAttribute - PIB attribute to be incremented.
 *
 * @return      none
 **************************************************************************************************
 */
MAC_INTERNAL_API void macLowLevelDiags( uint8 pibAttribute )
{
#if defined ( FEATURE_SYSTEM_STATS )
  if ( ( pibAttribute >= MAC_DIAGS_RX_CRC_PASS ) &&
       ( pibAttribute <= MAC_DIAGS_TX_SEC_FAIL ) )
  {
    uint32 value;

    /* Update Diagnostics counter */
    MAP_MAC_MlmeGetReq( pibAttribute, &value );
    value++;
    MAP_MAC_MlmeSetReq( pibAttribute, &value );
  }
#endif
}


/**************************************************************************************************
*/
