/******************************************************************************

 @file lmac_rom_jt.h

 @brief LMAC API directly map the function to function jump table

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated

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

#ifndef LMAC_ROM_JT_H
#define LMAC_ROM_JT_H

#include "mac_rx_onoff.h"
#include "mac_low_level.h"
#include "mac_symbol_timer.h"
#include "mac_radio.h"
#include "mac_radio_tx.h"


#include "rom_jt_def_154.h"

/*
** LMAC API Proxy
** ROM-to ROM or ROM-to-Flash function
** if there is any patch function, replace the corresponding entries
** LMAC variables are put first in JT
*/

#define  macTxSlottedDelay                                  (*(uint8 * )                        ROM_LMAC_JT_OFFSET(0))
#define  macUnitBackoffPeriod                               (*(uint8 * )                        ROM_LMAC_JT_OFFSET(1))
#define  rxFhRsl                                            (*(uint8 * )                        ROM_LMAC_JT_OFFSET(2))
#define  bInSwi                                             (*(volatile bool * )                ROM_LMAC_JT_OFFSET(3))
#define  macPhyChannel                                      (*(uint8 * )                        ROM_LMAC_JT_OFFSET(4))
#define  macSrcMatchIsEnabled                               (*(bool * )                         ROM_LMAC_JT_OFFSET(5))
#define  macRadioYielded                                    (*(volatile uint8 * )               ROM_LMAC_JT_OFFSET(6))

#define LMAC_API_BASE_INDEX                   (7)
    /* function pointer */
#define MAP_macRegisterSfdDetect                        	((void     (*)(macSfdDetectCBack_t ))               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+0))
#define MAP_macTxFrame                                  	((void     (*)(uint8_t ))             		        ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+1))
#define MAP_macRadioSetTxPower                          	((uint8_t  (*)(int8_t ))             		        ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+2))
#define MAP_macRxSoftEnable                             	((void     (*)(uint8_t ))             			    ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+3))
#define MAP_macRxSoftDisable                            	((void     (*)(uint8_t ))             			    ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+4))
#define MAP_macRadioSetChannel                          	((bool     (*)(uint8_t ))             			    ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+5))
#define MAP_macRadioSetChannelRx                        	((bool     (*)(uint8_t ))             				ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+6))

#define MAP_macRxOffRequest                                 ((void     (*)(void ))                              ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+7))
#define MAP_macRxDisable                                    ((void     (*)(uint8_t ))                           ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+8))
#define MAP_macRxEnable                                     ((bool     (*)(uint8_t ))                           ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+9))
#define MAP_macRxHardDisable                                ((void     (*)(void ))                              ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+10))
#define MAP_macRadioSetPanCoordinator                       ((void     (*)(uint8_t ))                           ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+11))
#define MAP_macRadioRandomByte                              ((uint8    (*)(void ))                              ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+12))

#define MAP_MAC_SrcMatchCheckAllPending                     ((uint8    (*)(void ))                              ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+13))
#define MAP_MAC_SrcMatchAckAllPending                       ((void     (*)(uint8 ))                             ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+14))

#define MAP_macLowLevelDiags                                ((void    (*)(uint8 ))                              ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+15))
#define MAP_macRadioSwInit                                  ((void    (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+16))
#define MAP_macRadioSetPanID                                ((void    (*)(uint16 ))                             ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+17))
#define MAP_macRadioSetShortAddr                            ((void    (*)(uint16 ))                             ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+18))
#define MAP_macRadioSetIEEEAddr                             ((void    (*)(uint8 * ))                            ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+19))
#define MAP_macSymbolTimerBCWakeUp                          ((void    (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+20))
#define MAP_macSymbolTimerInit                              ((void    (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+21))

#define MAP_MAC_SrcMatchAddEntry                            ((uint8    (*)(sAddr_t *, uint16))                  ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+22))
#define MAP_MAC_SrcMatchDeleteEntry                         ((uint8    (*)(sAddr_t *, uint16))                  ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+23))
#define MAP_macCanSleep                                     ((uint8    (*)(void ))                              ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+24))
#define MAP_macGetRadioTxPowerReg                           ((uint32    (*)(int8 ))                             ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+25))
#define MAP_macLowLevelReset                                ((void    (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+26))
#define MAP_macLowLevelResume                               ((void    (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+27))
#define MAP_macMcuBitReverse                                ((void    (*)(uint8 *, uint16))                     ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+28))
#define MAP_macRadioEnergyDetectStop                        ((uint8    (*)(void ))                              ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+29))
#define MAP_macRadioSetRE                                   ((void    (*)(uint32 ))                             ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+30))
#define MAP_macRadioStartScan                               ((void    (*)(uint8 ))                              ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+31))
#define MAP_macRadioStopScan                                ((void    (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+32))
#define MAP_macRadioTxEnhAckCb                              ((void    (*)(macTx_t *pMsg ))                      ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+33))
#define MAP_macRatDisableChannelB                           ((void    (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+34))
#define MAP_macRxSequenceNum                                ((uint8    (*)(void ))                              ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+35))
#define MAP_macTxFrameRetransmit                            ((void    (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+36))
#define MAP_macSymbolBCTimerPowerUp                         ((void    (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+37))
#define MAP_macRadioTxBOBoundary                            ((uint32  (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+38))
#define MAP_macTxDecrypt                                    ((uint8   (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+39))
#define MAP_txCsmaDelay                                     ((void    (*)(uint8 ))                              ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+40))
#define MAP_macGetRadioState                                ((bool    (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+41))
#define MAP_MAC_SrcMatchEnable                              ((void    (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+42))
#define MAP_macLowLevelYield                                ((bool    (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+43))

//#define MAP_macRadioTxBackoffBoundary                       ((uint32    (*)(uint32 ))                           ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+15))

//#define MAP_macRadioPowerDown                               ((void    (*)(bool ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+6))
//#define MAP_macRadioPowerUp                                 ((void    (*)(bool ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+6))
//#define MAP_macRadioPowerUpWait                             ((void    (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+6))
//#define MAP_macRadioPowerUpBeaconMode                       ((void    (*)(bool ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+6))

//#define MAP_macSymbolTimerChkPowerDown                      ((void    (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+6))
//#define MAP_macSymbolTimerCount                             ((uint32  (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+6))
//#define MAP_macSymbolTimerRealign                           ((int32   (*)(macRx_t *pMsg ))                      ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+6))
//#define MAP_macSymbolTimerSetCount                          ((void    (*)(uint32 ))                             ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+6))
//#define MAP_macSymbolTimerSetRollover                       ((void    (*)(uint32 ))                             ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+6))
//#define MAP_macSymbolTimerSetTrigger                        ((void    (*)(uint32 ))                             ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+6))
//#define MAP_macSymbolTimerSetTriggerRollover                ((void    (*)(void ))                               ROM_LMAC_JT_OFFSET(LMAC_API_BASE_INDEX+6))

#endif
