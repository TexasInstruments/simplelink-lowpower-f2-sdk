/******************************************************************************

 @file lmac_map_direct.h

 @brief LMAC API directly map the function implementation (declaration)

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

#ifndef LMAC_MAP_DIRECT_H
#define LMAC_MAP_DIRECT_H

#include "mac.h"
#include "mac_rx_onoff.h"
#include "mac_low_level.h"
#include "mac_symbol_timer.h"
#include "mac_radio.h"
#include "mac_radio_tx.h"

extern uint8 rxFhRsl;
#if defined(COMBO_MAC) || defined(FREQ_2_4G)
extern bool macSrcMatchIsEnabled;
#endif

#define MAP_macRegisterSfdDetect                        macRegisterSfdDetect
#define MAP_macTxFrame                                  macTxFrame
#define MAP_macRadioSetTxPower                          macRadioSetTxPower
#define MAP_macRxSoftEnable                             macRxSoftEnable
#define MAP_macRxSoftDisable                            macRxSoftDisable
#define MAP_macRadioSetChannel                          macRadioSetChannel
#define MAP_macRadioSetChannelRx                        macRadioSetChannelRx

#define MAP_macRxOffRequest                             macRxOffRequest
#define MAP_macRxDisable                                macRxDisable
#define MAP_macRxEnable                                 macRxEnable
#define MAP_macRxHardDisable                            macRxHardDisable
#define MAP_macRadioSetPanCoordinator                   macRadioSetPanCoordinator

#define MAP_macRadioRandomByte                          macRadioRandomByte
#define MAP_MAC_SrcMatchCheckAllPending                 MAC_SrcMatchCheckAllPending
#define MAP_MAC_SrcMatchAckAllPending                   MAC_SrcMatchAckAllPending

#define MAP_macLowLevelDiags                            macLowLevelDiags

#define MAP_macRadioSwInit                              macRadioSwInit
#define MAP_macRadioSetPanID                            macRadioSetPanID
#define MAP_macRadioSetShortAddr                        macRadioSetShortAddr
#define MAP_macRadioSetIEEEAddr                         macRadioSetIEEEAddr

#define MAP_macSymbolTimerBCWakeUp                      macSymbolTimerBCWakeUp
#define MAP_macSymbolTimerInit                          macSymbolTimerInit

#define MAP_MAC_SrcMatchEnable                          MAC_SrcMatchEnable
#define MAP_MAC_SrcMatchAddEntry                        MAC_SrcMatchAddEntry
#define MAP_MAC_SrcMatchDeleteEntry                     MAC_SrcMatchDeleteEntry
#define MAP_macCanSleep                                 macCanSleep
#define MAP_macGetRadioTxPowerReg                       macGetRadioTxPowerReg
#define MAP_macLowLevelReset                            macLowLevelReset
#define MAP_macLowLevelResume                           macLowLevelResume
#define MAP_macMcuBitReverse                            macMcuBitReverse
#define MAP_macRadioEnergyDetectStop                    macRadioEnergyDetectStop

#define MAP_macRadioSetRE                               macRadioSetRE
#define MAP_macRadioStartScan                           macRadioStartScan
#define MAP_macRadioStopScan                            macRadioStopScan
#define MAP_macRadioTxEnhAckCb                          macRadioTxEnhAckCb
#define MAP_macRatDisableChannelB                       macRatDisableChannelB
#define MAP_macRxSequenceNum                            macRxSequenceNum
#define MAP_macTxFrameRetransmit                        macTxFrameRetransmit
#define MAP_macSymbolBCTimerPowerUp                     macSymbolBCTimerPowerUp

#define MAP_macRadioTxBOBoundary                        macRadioTxBOBoundary
#define MAP_macTxDecrypt                                macTxDecrypt
#define MAP_txCsmaDelay                                 txCsmaDelay
#define MAP_macGetRadioState                            macGetRadioState

//#define MAP_macRadioTxBackoffBoundary                   macRadioTxBackoffBoundary
//#define MAP_macRadioPowerDown                           macRadioPowerDown
//#define MAP_macRadioPowerUp                             macRadioPowerUp
//#define MAP_macRadioPowerUpWait                         macRadioPowerUpWait
//#define MAP_macRadioPowerUpBeaconMode                   macRadioPowerUpBeaconMode
//#define MAP_macSymbolTimerChkPowerDown                  macSymbolTimerChkPowerDown
//#define MAP_macSymbolTimerCount                         macSymbolTimerCount
//#define MAP_macSymbolTimerRealign                       macSymbolTimerRealign
//#define MAP_macSymbolTimerSetCount                      macSymbolTimerSetCount
//#define MAP_macSymbolTimerSetRollover                   macSymbolTimerSetRollover
//#define MAP_macSymbolTimerSetTrigger                    macSymbolTimerSetTrigger
//#define MAP_macSymbolTimerSetTriggerRollover            macSymbolTimerSetTriggerRollover
//#define MAP_macRadioInit                                macRadioInit

/*
**  LMAC API directly map to implementation
**  This is used in CC131X/CC135X project with using any TIMAC 15.4 ROM image
*/

//#define MAP_FHAPI_reset                                             FHAPI_reset
//#define MAP_FHAPI_start                                             FHAPI_start
//#define MAP_FHAPI_startBS                                           FHAPI_startBS

#endif
