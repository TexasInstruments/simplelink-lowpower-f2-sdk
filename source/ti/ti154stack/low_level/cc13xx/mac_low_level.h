/******************************************************************************

 @file  mac_low_level.h

 @brief Describe the purpose and contents of the file.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2006-2025, Texas Instruments Incorporated

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

#ifndef MAC_LOW_LEVEL_H
#define MAC_LOW_LEVEL_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_types.h"
#include "mac_high_level.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */

/*! Clock tick period in microseconds */
#define CLOCK_TICK_PERIOD     (MAP_ICall_getTickPeriod())

/*! Ticks per one millisecond  */
#define TICKPERIOD_MS_US      (1000/(CLOCK_TICK_PERIOD))

#define MAC_SYMBOL_TIMER_DEFAULT_ROLLOVER  (((uint32) MAC_A_BASE_SUPERFRAME_DURATION) << 14)

/* 15,728,640 symbols or 314.57 Seconds */
#define MAC_SYMBOL_TIMER_DEFAULT_NONBEACON_ROLLOVER  MAC_SYMBOL_TIMER_DEFAULT_ROLLOVER

/* Overhead delay in symbols incurred by radio driver and RTOS ~ 12ms
 * This delay should only be added for RX related timeout such as
 * ACK timeout. 75 symbols added to account for BT-IE.
 */
#define MAC_PROCESS_DELAY                   675
#define MAC_LRM_PROCESS_DELAY               3000
#define MAC_IEEE_PROCESS_DELAY              150

/* macTxFrame() parameter values for txType */
#define MAC_TX_TYPE_SLOTTED_CSMA            0x00
#define MAC_TX_TYPE_UNSLOTTED_CSMA          0x01
#define MAC_TX_TYPE_SLOTTED                 0x02
#define MAC_TX_TYPE_NO_CSMA                 0x03
#define MAC_TX_TYPE_FH_CSMA                 0x04
#define MAC_TX_TYPE_LBT                     0x05
#define MAC_TX_TYPE_GREEN_POWER             0x06

/* FH enhanced ACK, there is CCA in UTIE or BTIE */
#define MAC_TX_TYPE_FH_ENH_ACK              0xFF

/* macSleep() parameter values for sleepState */
#define MAC_SLEEP_STATE_OSC_OFF             0x01
#define MAC_SLEEP_STATE_RADIO_OFF           0x02

#ifdef FH_BENCHMARK_BUILD_TIMES
#define NUM_MAC_BENCHMARK_CNT 4
#endif

/* Slotted delay value for beacon mode transmission */
#ifndef HAL_MAC_TX_SLOTTED_DELAY
#define HAL_MAC_TX_SLOTTED_DELAY    48
#endif

#define MAC_MRFSK_20_KSPS_BEACON_DELAY_FACTOR     1 /* 20K symbol rate factor for beacon tx */
#define MAC_MRFSK_50_KSPS_BEACON_DELAY_FACTOR     1 /* 50K symbol rate factor for beacon tx */
#define MAC_MRFSK_150_KSPS_BEACON_DELAY_FACTOR    2 /* 150K symbol rate factor for beacon tx */
#define MAC_MRFSK_200_KSPS_BEACON_DELAY_FACTOR    5 /* 200K symbol rate factor for beacon tx */

#ifdef COMBO_MAC
#define SELECT_CALL(name, ...)                  \
  uint8 rfFreq = pMacPib->rfFreq;               \
                                                \
  if (rfFreq == MAC_RF_FREQ_SUBG)               \
  {                                             \
      (name ## SubG(__VA_ARGS__));              \
  }                                             \
  else                                          \
  {                                             \
      (name ## Ieee(__VA_ARGS__));              \
  }

#define SELECT_RCALL(name, ...)                 \
  uint8 rfFreq = pMacPib->rfFreq;               \
                                                \
  if (rfFreq == MAC_RF_FREQ_SUBG)               \
  {                                             \
      return (name ## SubG(__VA_ARGS__));       \
  }                                             \
  else                                          \
  {                                             \
      return (name ## Ieee(__VA_ARGS__));       \
  }
#else
#ifndef FREQ_2_4G
#define SELECT_CALL(name, ...)                  \
  (name ## SubG(__VA_ARGS__));

#define SELECT_RCALL(name, ...)                 \
  return (name ## SubG(__VA_ARGS__));
#else
#define SELECT_CALL(name, ...)                  \
  (name ## Ieee(__VA_ARGS__));

#define SELECT_RCALL(name, ...)                 \
  return (name ## Ieee(__VA_ARGS__));
#endif
#endif

#define RF_SWITCH_TO_2_4G   0
#define RF_SWITCH_TO_SUBG   1
/* ------------------------------------------------------------------------------------------------
 *                                           Global Externs
 * ------------------------------------------------------------------------------------------------
 */
extern uint8 macTxSlottedDelay;
extern uint8 macTxSlottedDelayFactor;

/* beacon interval margin */
extern uint16 macBeaconMargin[];

/* MAC_A_UNIT_BACKOFF_PERIOD in symbols */
extern uint8 macUnitBackoffPeriod;


/* ------------------------------------------------------------------------------------------------
 *                                           Prototypes
 * ------------------------------------------------------------------------------------------------
 */

/* mac_low_level.c */
MAC_INTERNAL_API void macLowLevelInit(void);
MAC_INTERNAL_API void macLowLevelBufferInit(void);
MAC_INTERNAL_API void macLowLevelReset(void);
MAC_INTERNAL_API void macLowLevelResume(void);
MAC_INTERNAL_API bool macLowLevelYield(void);
MAC_INTERNAL_API void macLowLevelDiags(uint8 pibAttribute);

/* mac_sleep.c */
MAC_INTERNAL_API void macSleepWakeUp(void);
MAC_INTERNAL_API uint8 macSleep(uint8 sleepState);
#if defined USE_ICALL || defined OSAL_PORT2TIRTOS
MAC_INTERNAL_API uint8 macCanSleep(void);
#endif /* defined USE_ICALL || defined OSAL_PORT2TIRTOS */

/* mac_radio.c */
MAC_INTERNAL_API uint8 macRadioRandomByte(void);
MAC_INTERNAL_API void macRadioSetPanCoordinator(uint8 panCoordinator);
MAC_INTERNAL_API void macRadioSetPanID(uint16 panID);
MAC_INTERNAL_API void macRadioSetShortAddr(uint16 shortAddr);
MAC_INTERNAL_API void macRadioSetIEEEAddr(uint8 * pIEEEAddr);
MAC_INTERNAL_API uint8 macRadioSetTxPower(int8 txPower);
MAC_INTERNAL_API uint32 macGetRadioTxPowerReg(int8 txPower);
MAC_INTERNAL_API bool macRadioSetChannel(uint8 channel);
MAC_INTERNAL_API bool macRadioSetChannelRx(uint8 channel);
MAC_INTERNAL_API void macRadioStartScan(uint8 scanType);
MAC_INTERNAL_API void macRadioStopScan(void);
MAC_INTERNAL_API uint8 macRadioEnergyDetectStop(void);

/* mac_radio_tx.c */
MAC_INTERNAL_API void macRadioTxEnhAckCb(macTx_t *pMsg);

/* mac_symbol_timer.c */
MAC_INTERNAL_API void macSymbolTimerSetRollover(uint32 rolloverSymbol);
MAC_INTERNAL_API void macSymbolTimerSetCount(uint32 symbol);
MAC_INTERNAL_API uint32 macSymbolTimerCount(void);
MAC_INTERNAL_API uint32 macSymbolTimerGetTrigger(void);
MAC_INTERNAL_API void macSymbolTimerSetTrigger(uint32 triggerSymbol);
MAC_INTERNAL_API void macSymbolTimerTriggerCallback(void);
MAC_INTERNAL_API void macSymbolTimerRolloverCallback(void);
MAC_INTERNAL_API int32 macSymbolTimerRealign(macRx_t *pMsg);

/* mac_tx.c */
MAC_INTERNAL_API void macTxFrame(uint8 txType);
MAC_INTERNAL_API void macTxFrameRetransmit(void);
MAC_INTERNAL_API void macTxCompleteCallback(uint8 status);

/* mac_rx.c */
MAC_INTERNAL_API bool macRxCheckPendingCallback(void);
MAC_INTERNAL_API bool macRxCheckMACPendingCallback(void);
MAC_INTERNAL_API void macRxCompleteCallback(macRx_t * pMsg);
MAC_INTERNAL_API uint8 macRxSequenceNum(void);

/* mac_rx_onoff.c */
MAC_INTERNAL_API bool macRxEnable(uint8 flags);
MAC_INTERNAL_API void macRxSoftEnable(uint8 flags);
MAC_INTERNAL_API void macRxDisable(uint8 flags);
MAC_INTERNAL_API void macRxSoftDisable(uint8 flags);
MAC_INTERNAL_API void macRxHardDisable(void);

/**************************************************************************************************
 */
#endif
