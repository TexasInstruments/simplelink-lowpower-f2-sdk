/******************************************************************************

 @file  rfcore_cc13_15_4_patch.h

 @brief This file contains all patches that need to be applied for the
        RF core firmware (i.e. CM0, MCE and RFE)

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2013-2025, Texas Instruments Incorporated

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

#ifndef RFCORE_15_4_PATCH
#define RFCORE_15_4_PATCH

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * INCLUDES
 */

#include "hw_types.h"
#include "hw_rfc_pwr.h"
#include "rfHal.h"


#include "apply_patch.h" 
    
#ifndef NO_SINGLE_ENDED_MODE_SUPPORT
// Patch for MCE
#include "apply_zigbee_xs_is_mce_patch_partial.h"

#endif    

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */
   

/*******************************************************************************
 * @fn          patchRfCore API
 *
 * @brief       This subroutine is used to consolidate and execute 
 *              patching for the RF FW.
 *
 * input parameters
 *
 * @param       singleEndMode - TURE if single ended mode.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void patchRfCore( bool singleEndedMode )
{
  RF_Stat stat;
  
#ifndef NO_SINGLE_ENDED_MODE_SUPPORT
  rfOpCmd_TopsmCopy_t topsmCopyCmd;
  rfOpImmedCmd_RW_RfReg_t writeRfRegCmd;
  uint32_t cpeIrqClrFlags;
#endif  

#ifndef NO_SINGLE_ENDED_MODE_SUPPORT
  if (singleEndedMode) {
    // PG 2 single ended mode
    memset(&topsmCopyCmd, 0, sizeof(topsmCopyCmd));  
    topsmCopyCmd.rfOpCmd.cmdNum    = CMD_TOPSM_COPY;
    topsmCopyCmd.rfOpCmd.status    = RFSTAT_IDLE;
    topsmCopyCmd.rfOpCmd.pNextRfOp = NULL;
    CLR_RFOP_ALT_TRIG_CMD( topsmCopyCmd.rfOpCmd.startTrig );
    SET_RFOP_TRIG_TYPE( topsmCopyCmd.rfOpCmd.startTrig, TRIGTYPE_NOW );
    SET_RFOP_COND_RULE( topsmCopyCmd.rfOpCmd.condition, COND_NEVER );
    topsmCopyCmd.mceBank           = 1;          // Bank containing 15.4 code
    topsmCopyCmd.rfeBank           = -1;         // No RFE patch
    topsmCopyCmd.mceStopAddr       = ZIGBEE_XS_IS_MCE_NUM_COPY;  // Number of instructions to copy
    topsmCopyCmd.rfeStopAddr       = 0;
  
    // Clear interrupt flags
    cpeIrqClrFlags = MB_RFCPEIFG_REG & (MB_COMMAND_DONE_INT | MB_LAST_COMMAND_DONE_INT);
    if (cpeIrqClrFlags) {
      do {
        MB_RFCPEIFG_REG = ~cpeIrqClrFlags;
      } while (MB_RFCPEIFG_REG & cpeIrqClrFlags);
   }
  
   stat = RF_runImmediateCmd(RF_handle, (uint32_t*) &topsmCopyCmd);
  }
#endif // NO_SINGLE_ENDED_MODE_SUPPORT
  
  applyPatch();

#ifndef NO_SINGLE_ENDED_MODE_SUPPORT
  if (singleEndedMode) {
    // Force enable MCE RAM
    stat = RF_runDirectCmd(RF_handle,
      (uint32_t) BUILD_DIRECT_PARAM_EXT_CMD(CMD_FORCE_CLK_ENA,
                                            RFC_PWR_PWMCLKEN_RFERAM));

    // Wait for CMD_TOPSM_COPY to finish
    while( !(MB_RFCPEIFG_REG & (MB_COMMAND_DONE_INT | MB_LAST_COMMAND_DONE_INT)) );
  
    enterZigbeeXsIsMcePatch();

    // Clear interrupt flags
    cpeIrqClrFlags = MB_RFCPEIFG_REG & (MB_COMMAND_DONE_INT | MB_LAST_COMMAND_DONE_INT);
    if (cpeIrqClrFlags) {
      do {
        MB_RFCPEIFG_REG = ~cpeIrqClrFlags;
      } while (MB_RFCPEIFG_REG & cpeIrqClrFlags);
    }
    // Stop force enabling RAMs
    stat = RF_runDirectCmd(RF_handle,
      (uint32_t) BUILD_DIRECT_PARAM_EXT_CMD(CMD_FORCE_CLK_ENA, 0));
  }
#endif // NO_SINGLE_ENDED_MODE_SUPPORT

  return;
}


#ifdef __cplusplus
}
#endif

#endif /* RFCORE_15_4_PATCH */
