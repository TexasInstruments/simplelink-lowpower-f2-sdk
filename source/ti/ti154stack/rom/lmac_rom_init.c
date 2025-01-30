/******************************************************************************

 @file  lmac_rom_init.c

 @brief This file contains the externs for ROM API initialization.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2017-2025, Texas Instruments Incorporated

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

/*******************************************************************************
 * INCLUDES
 */

#include "mac.h"
#include "mac_tx.h"
#include "mac_radio.h"
#include "mac_radio_tx.h"
#include "mac_rx_onoff.h"
#include "mac_symbol_timer.h"
#include "mac_low_level.h"

#include "rom_jt_def_154.h"

/*******************************************************************************
 * EXTERNS
 */

extern uint8 rxFhRsl;
#if defined(COMBO_MAC) || defined(FREQ_2_4G)
extern bool macSrcMatchIsEnabled;
#endif
/*******************************************************************************
 * PROTOTYPES
 */

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

// ROM Flash Jump Table

#if defined __TI_COMPILER_VERSION || defined __TI_COMPILER_VERSION__
#pragma DATA_ALIGN(LMAC_ROM_Flash_JT, 4)
#elif defined(__GNUC__) || defined(__clang__)
__attribute__ ((aligned (4)))
#else
#pragma data_alignment=4
#endif

const uint32 LMAC_ROM_Flash_JT[] =
{
    /* global variables */
    (uint32)&macTxSlottedDelay,
    (uint32)&macUnitBackoffPeriod,
    (uint32)&rxFhRsl,
    (uint32)&bInSwi,
    (uint32)&macPhyChannel,
#if defined(COMBO_MAC) || defined(FREQ_2_4G)
    (uint32)&macSrcMatchIsEnabled,
#else
    (uint32)NULL,
#endif
    (uint32)&macRadioYielded,

    /* function pointer */
    (uint32)&macRegisterSfdDetect,                                       // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+0]
    (uint32)&macTxFrame,                                                 // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+1]
    (uint32)&macRadioSetTxPower,                                         // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+2]
    (uint32)&macRxSoftEnable,                                            // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+3]
    (uint32)&macRxSoftDisable,                                           // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+4]
    (uint32)&macRadioSetChannel,                                         // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+5]
    (uint32)&macRadioSetChannelRx,                                       // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+6]

    (uint32)&macRxOffRequest,                                            // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+7]
    (uint32)&macRxDisable,                                               // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+8]
    (uint32)&macRxEnable,                                                // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+9]
    (uint32)&macRxHardDisable,                                           // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+10]
    (uint32)&macRadioSetPanCoordinator,                                  // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+11]
    (uint32)&macRadioRandomByte,                                         // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+12]

    (uint32)&MAC_SrcMatchCheckAllPending,                                // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+13]
    (uint32)&MAC_SrcMatchAckAllPending,                                  // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+14]

    (uint32)&macLowLevelDiags,                                           // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+15]
    (uint32)&macRadioSwInit,                                             // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+16]
    (uint32)&macRadioSetPanID,                                           // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+17]
    (uint32)&macRadioSetShortAddr,                                       // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+18]
    (uint32)&macRadioSetIEEEAddr,                                        // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+19]
    (uint32)&macSymbolTimerBCWakeUp,                                     // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+20]
    (uint32)&macSymbolTimerInit,                                         // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+21]

    (uint32)&MAC_SrcMatchAddEntry,                                       // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+22]
    (uint32)&MAC_SrcMatchDeleteEntry,                                    // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+23]
    (uint32)&macCanSleep,                                                // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+24]
    (uint32)&macGetRadioTxPowerReg,                                      // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+25]
    (uint32)&macLowLevelReset,                                           // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+26]
    (uint32)&macLowLevelResume,                                          // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+27]
#if defined (COMBO_MAC) || (!defined (FREQ_2_4G))
    (uint32)&macMcuBitReverse,                                           // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+28]
#else
    (uint32)NULL,
#endif
    (uint32)&macRadioEnergyDetectStop,                                   // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+29]
    (uint32)&macRadioSetRE,                                              // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+30]
    (uint32)&macRadioStartScan,                                          // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+31]
    (uint32)&macRadioStopScan,                                           // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+32]
#if defined (FEATURE_ENHANCED_ACK) && (defined (COMBO_MAC) || (!defined (FREQ_2_4G)))
    (uint32)&macRadioTxEnhAckCb,                                         // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+33]
#else
    (uint32)NULL,
#endif
    (uint32)&macRatDisableChannelB,                                      // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+34]
    (uint32)&macRxSequenceNum,                                           // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+35]
    (uint32)&macTxFrameRetransmit,                                       // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+36]
    (uint32)&macSymbolBCTimerPowerUp,                                    // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+37]
#if defined( FEATURE_BEACON_MODE )
    (uint32)&macRadioTxBOBoundary,                                       // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+38]
#else
    (uint32)NULL,
#endif

#if defined (FEATURE_MAC_SECURITY)
    (uint32)&macTxDecrypt,                                               // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+39]
#else
    (uint32)NULL,
#endif
    (uint32)&txCsmaDelay,                                                // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+40]
    (uint32)&macGetRadioState,                                           // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+41]
    (uint32)&MAC_SrcMatchEnable,                                         // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+42]
    (uint32)&macLowLevelYield,                                           // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+43]

    //    (uint32)&macRadioTxBackoffBoundary,                                // ROM_LMAC_JT_OFFSET[LMAC_API_BASE_INDEX+2]
    //(uint32)&macRadioPowerDown,
    //(uint32)&macRadioPowerUp,
    //(uint32)&macRadioPowerUpWait,
    //(uint32)&macRadioPowerUpBeaconMode,

    //(uint32)&macSymbolTimerChkPowerDown,
    //(uint32)&macSymbolTimerCount,
    //(uint32)&macSymbolTimerRealign,
    //(uint32)&macSymbolTimerSetCount,
    //(uint32)&macSymbolTimerSetRollover,
    //(uint32)&macSymbolTimerSetTrigger,
    //(uint32)&macSymbolTimerSetTriggerRollover,

};  

void LMAC_ROM_Init(void)
{   
    /* assign the FH ROM JT table */
    RAM_MAC_BASE_ADDR[ROM_RAM_LMAC_TABLE_INDEX] = (uint32)(LMAC_ROM_Flash_JT);
}

