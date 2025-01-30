/******************************************************************************

 @file  mac_settings.c

 @brief This file contains CM0 radio command variables.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2015-2025, Texas Instruments Incorporated

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

//*********************************************************************************
// Default parameter summary
// Address: N/A
// Frequency: 920.00000 MHz
// Data Format: Serial mode disable
// Deviation: 25.000 kHz
// Packet Length Config: Variable
// Max Packet Length: 1040
// Packet Length: Variable
// RX Filter BW: 110 kHz
// Symbol Rate: 50.00000 kBaud
// Sync Word Length: 16 Bits
// Whitening: Whitening Enabled

/* ------------------------------------------------------------------------------------------------
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */

#ifndef LIBRARY
#include "ti_154stack_features.h"
#endif

#include <stdint.h>
#include "mac_settings.h"
#include "rf_mac_api.h"

// CMD_PROP_RADIO_DIV_SETUP
#if defined (DeviceFamily_CC13X2)
rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t RF_cmdPropRadioDivSetup =
#else
rfc_CMD_PROP_RADIO_DIV_SETUP_t RF_cmdPropRadioDivSetup =
#endif
{
    .commandNo = 0x3807,
    .status = 0x0000,
    .pNextOp = 0, // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x1,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .modulation.modType = 0x1, // GFSK
    .modulation.deviation = 0x64, // 100
    .symbolRate.preScale = 0xf, // 15
    .symbolRate.rateWord = 0x8000, // 32768
    .rxBw = 0x23, // 88 kHz
    .preamConf.nPreamBytes = 0x7, // 7 bytes
    .preamConf.preamMode = 0x0, // Send 0 as first bit in 2FSK
    .formatConf.nSwBits = 24, // 24-bit of syncword
    .formatConf.bBitReversal = 0x0,
    .formatConf.bMsbFirst = 0x1,
    .formatConf.fecMode = 0x0,
    .formatConf.whitenMode = 0x7,
    .config.frontEndMode = 0x0,
    .config.biasMode = 0x1,
#if defined (DeviceFamily_CC13X2)
    .config.analogCfgMode = 0x0,
#endif
    .config.bNoFsPowerUp = 0x0,
    .txPower = 0xa73f,
    .pRegOverride = NULL,
    .centerFreq = 915, // 915 MHz
    .intFreq = 0x8000, // Use default
    .loDivider = 0x05,
#if defined (DeviceFamily_CC13X2)
    .pRegOverrideTxStd = NULL,
    .pRegOverrideTx20 = NULL,
#endif
};

// CMD_FS
rfc_CMD_FS_t RF_cmdFs =
{
    .commandNo = 0x0803,
    .status = 0x0000,
    .pNextOp = 0, // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x1,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .frequency = 920, // 920 MHz
    .fractFreq = 0x0000,
    .synthConf.bTxMode = 0x0,
    .synthConf.refFreq = 0x0,
    .__dummy0 = 0x00,
    .__dummy1 = 0x00,
    .__dummy2 = 0x00,
    .__dummy3 = 0x0000,
};

// CMD_PROP_TX_ADV
rfc_CMD_PROP_TX_ADV_t RF_cmdPropTxAdv =
{
    .commandNo = 0x3803,
    .status = 0x0000,
    .pNextOp = 0, // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
    .startTime = 0x00000000,
    .startTrigger.triggerType = TRIG_NOW,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x1, // command can be delayed and appended to the end of the rf_cmdQ
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .pktConf.bFsOff = 0x0,
#ifdef MAC_PROTOCOL_TEST
    .pktConf.bUseCrc = 0x0,
#else
    .pktConf.bUseCrc = 0x1,
#endif
    .pktConf.bCrcIncSw = 0x0,
    .pktConf.bCrcIncHdr = 0x0,
    .numHdrBits = 16,
    .pktLen = 0,
    .startConf.bExtTxTrig = 0,
    .startConf.inputMode = 0,
    .startConf.source = 0,
    .preTrigger.triggerType = TRIG_REL_START, // CC13_RF_ROM_FW_CPE--BUG00016
    .preTrigger.bEnaCmd = 0,
    .preTrigger.triggerNo = 0,
    .preTrigger.pastTrig = 1,
    .preTime = 0,
    .syncWord = 0x0055904e,
    .pPkt = 0, // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
};

// CMD_PROP_CS for CSMA or LBT
rfc_CMD_PROP_CS_t RF_cmdPropCs =
{
    .commandNo = 0x3805,
    .status = 0x0000,
    .pNextOp = (rfc_radioOp_t *)&RF_cmdPropTxAdv,
    .startTime = 0x00000000,
    .startTrigger.triggerType = TRIG_REL_SUBMIT,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x1,
    .condition.rule = COND_STOP_ON_TRUE,
    .condition.nSkip = 0,
    .csFsConf.bFsOffIdle = 0x0,
    .csFsConf.bFsOffBusy = 0x0,
    .__dummy0 = 0x0,
    .csConf.bEnaRssi = 0x1,
    .csConf.bEnaCorr = 0x0, /* CC13xx sub-G can only support CCA mode 1 */
    .csConf.operation = 0x0,
    .csConf.busyOp = 0x1,
    .csConf.idleOp = 0x1,
    .csConf.timeoutRes = 0x0,
    .rssiThr = -83, /* -83 dBm */
    .numRssiIdle = 0x00,
    .numRssiBusy = 0x00,
    .corrPeriod = 640, /* Filler, used for correlation only */
    .corrConfig.numCorrInv = 0x03,
    .corrConfig.numCorrBusy = 0x00,
    .csEndTrigger.triggerType = TRIG_REL_START,
    .csEndTrigger.bEnaCmd = 0x0,
    .csEndTrigger.triggerNo = 0x0,
    .csEndTrigger.pastTrig = 0x0,
    .csEndTime = 8000, /* 2 ms timeout as a fallback */
};

// CMD_PROP_CS
rfc_CMD_PROP_CS_t RF_cmdPropCsSlotted =
{
    .commandNo = 0x3805,
    .status = 0x0000,
    .pNextOp = (rfc_radioOp_t *)&RF_cmdPropCs,
    .startTime = 0x00000000,
    .startTrigger.triggerType = TRIG_ABSTIME,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x1,
    .condition.rule = COND_STOP_ON_TRUE,
    .condition.nSkip = 0x0,
    .csFsConf.bFsOffIdle = 0x0,
    .csFsConf.bFsOffBusy = 0x0,
    .__dummy0 = 0x0,
    .csConf.bEnaRssi = 0x1,
    .csConf.bEnaCorr = 0x0, /* CC13xx sub-G can only support CCA mode 1 */
    .csConf.operation = 0x0,
    .csConf.busyOp = 0x1,
    .csConf.idleOp = 0x1,
    .csConf.timeoutRes = 0x0,
    .rssiThr = -83, /* -83 dBm */
    .numRssiIdle = 0x00,
    .numRssiBusy = 0x00,
    .corrPeriod = 640, /* Filler, used for correlation only */
    .corrConfig.numCorrInv = 0x03,
    .corrConfig.numCorrBusy = 0x00,
    .csEndTrigger.triggerType = TRIG_REL_START,
    .csEndTrigger.bEnaCmd = 0x0,
    .csEndTrigger.triggerNo = 0x0,
    .csEndTrigger.pastTrig = 0x0,
    .csEndTime = 8000, /* 2 ms timeout as a fallback */
};

// CMD_PROP_RX_ADV
rfc_CMD_PROP_RX_ADV_t RF_cmdPropRxAdv =
{
    .commandNo = 0x3804,
    .status = 0x0000,
    .pNextOp = 0, // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x1,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .pktConf.bFsOff = 0x0,
    .pktConf.bRepeatOk = 0x0, // TODO
    .pktConf.bRepeatNok = 0x0, // TODO
    .pktConf.bUseCrc = 0x1,
    .pktConf.bCrcIncSw = 0x0,
    .pktConf.bCrcIncHdr = 0x0,
    .pktConf.endType = 0x0, // TODO
    .pktConf.filterOp = 0x1, // TODO
    .rxConf.bAutoFlushIgnored = 0x0,
    .rxConf.bAutoFlushCrcErr = 0x1,
    .rxConf.bIncludeHdr = 0x1,
    .rxConf.bIncludeCrc = 0x0,
    .rxConf.bAppendRssi = 0x1,
    .rxConf.bAppendTimestamp = 0x1,
    .rxConf.bAppendStatus = 0x0,
    .syncWord0 = 0x0055904e,
    .syncWord1 = 0x00000000,
    .maxPktLen = 500,/* Will be re-initialized in RX init */
    .hdrConf.numHdrBits = 16,
    .hdrConf.lenPos = 0,
    .hdrConf.numLenBits = 11,
    .addrConf.addrType = 0,
    .addrConf.addrSize = 0, // TODO: no address is used for now
    .addrConf.addrPos = 0,
    .addrConf.numAddr = 0,
    .lenOffset = -4,
#ifdef RFCORE49
    .endTrigger.triggerType = 0x4,
    .endTrigger.bEnaCmd = 0x0,
    .endTrigger.triggerNo = 0x0,
    .endTrigger.pastTrig = 0x0,
    .endTime = 1680000000,   //7 minutes
#else
    .endTrigger.triggerType = 0x1,
    .endTrigger.bEnaCmd = 0x0,
    .endTrigger.triggerNo = 0x0,
    .endTrigger.pastTrig = 0x0,
    .endTime = 0x00000000,
#endif
    .pAddr = 0,
    .pQueue = 0, // INSERT APPLICABLE POINTER: (dataQueue_t*)&xxx
    .pOutput = 0, // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
};

// CMD_NOP
rfc_CMD_NOP_t RF_cmdNop =
{
    .commandNo = 0x0801,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x1,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
};

// CMD_SET_RAT_CMP
rfc_CMD_SET_RAT_CMP_t RF_cmdRat =
{
    .commandNo = 0x000A,
    .ratCh = 0,
    .__dummy0 = 0,
    .compareTime = 0,
};


// CMD_SET_RAT_CMP
rfc_CMD_SET_RAT_CMP_t RF_cmdRat1 =
{
    .commandNo = 0x000A,
    .ratCh = 1,
    .__dummy0 = 0,
    .compareTime = 0,
};
