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

/* ------------------------------------------------------------------------------------------------
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */

#ifndef LIBRARY
#include "ti_wisunfan_features.h"
#endif

#include "mac.h"
#include "mac_user_config.h"
#include "mac_settings.h"
#include "rom_jt_154.h"

extern macUserCfg_t *pMacRadioConfig;
extern RF_Mode *pRfMode;
extern RF_TxPowerTable_Entry *pRfPowerTable;
extern macRfCfg_maxPower_t *pMaxPower;

#if defined(COMBO_MAC) || !defined(FREQ_2_4G)
// CMD_PROP_RADIO_DIV_SETUP
rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t RF_cmdPropRadioDivSetup;
// CMD_FS Rx
rfc_CMD_FS_t RF_cmdFsRx;
// CMD_FS Tx
rfc_CMD_FS_t RF_cmdFsTx;
// CMD_PROP_TX_ADV
rfc_CMD_PROP_TX_ADV_t RF_cmdPropTxAdv;
// CMD_PROP_RX_ADV
rfc_CMD_PROP_RX_ADV_t RF_cmdPropRxAdv;
// CMD_PROP_CS for CSMA or LBT
rfc_CMD_PROP_CS_t RF_cmdPropCs;
// CMD_PROP_CS
rfc_CMD_PROP_CS_t RF_cmdPropCsSlotted;

void rfSetConfigSubG(macRfCfg_prop_t *pRfCfg)
{
    size_t setupLength;
    ChipType_t chipType = ChipInfo_GetChipType();

    pRfMode = pRfCfg->pRfMode;
    pRfPowerTable = pRfCfg->pRfPowerTable;
    memset(&RF_cmdPropRadioDivSetup, 0, sizeof(rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t));
#if defined(DeviceFamily_CC13X4) || defined(DeviceFamily_CC26X4)
    if (chipType == CHIP_TYPE_CC1354P10)
#else
    if (chipType == CHIP_TYPE_CC1352P || chipType == CHIP_TYPE_CC1312P || chipType == CHIP_TYPE_CC1352P7 || chipType == CHIP_TYPE_CC1311P3)
#endif
    {
        setupLength = sizeof(rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t);
    }
    else
    {
        setupLength = sizeof(rfc_CMD_PROP_RADIO_DIV_SETUP_t);
    }
    memcpy(&RF_cmdPropRadioDivSetup, pRfCfg->pRfSetup, setupLength);
    RF_cmdPropRadioDivSetup.startTrigger.pastTrig = 1;
    RF_cmdPropRadioDivSetup.formatConf.bBitReversal = 0;
    RF_cmdPropRadioDivSetup.formatConf.bMsbFirst = 1;
    RF_cmdPropRadioDivSetup.formatConf.whitenMode = 7;
    RF_cmdPropRadioDivSetup.intFreq = 0x8000;

    memcpy(&RF_cmdFsRx, pRfCfg->pRfFs, sizeof(rfc_CMD_FS_t));
    RF_cmdFsRx.startTrigger.pastTrig = 1;

    memcpy(&RF_cmdFsTx, pRfCfg->pRfFs, sizeof(rfc_CMD_FS_t));
    RF_cmdFsTx.startTrigger.pastTrig = 1;


    memcpy(&RF_cmdPropTxAdv, pRfCfg->pRfTx, sizeof(rfc_CMD_PROP_TX_ADV_t));
    RF_cmdPropTxAdv.startTrigger.triggerType = TRIG_NOW;
    RF_cmdPropTxAdv.startTrigger.pastTrig = 1;
#ifdef MAC_PROTOCOL_TEST
    RF_cmdPropTxAdv.pktConf.bUseCrc = 0;
#else
    RF_cmdPropTxAdv.pktConf.bUseCrc = 1;
#endif
    RF_cmdPropTxAdv.numHdrBits = 0x10;
    RF_cmdPropTxAdv.pktLen = 0;
    RF_cmdPropTxAdv.preTrigger.triggerType = TRIG_REL_START;
    RF_cmdPropTxAdv.preTrigger.pastTrig = 1;

    memcpy(&RF_cmdPropRxAdv, pRfCfg->pRfRx, sizeof(rfc_CMD_PROP_RX_ADV_t));
    RF_cmdPropRxAdv.startTrigger.pastTrig = 1;
    RF_cmdPropRxAdv.pktConf.bFsOff = 0x0,
    RF_cmdPropRxAdv.pktConf.bRepeatOk = 0x0;
    RF_cmdPropRxAdv.pktConf.bRepeatNok = 0x0;
    RF_cmdPropRxAdv.pktConf.bUseCrc = 0x1;
    RF_cmdPropRxAdv.pktConf.bCrcIncSw = 0x0;
    RF_cmdPropRxAdv.pktConf.bCrcIncHdr = 0x0;
    RF_cmdPropRxAdv.pktConf.endType = 0x0;
    RF_cmdPropRxAdv.pktConf.filterOp = 0x1;
    RF_cmdPropRxAdv.rxConf.bAutoFlushIgnored = 0x0;
    RF_cmdPropRxAdv.rxConf.bAutoFlushCrcErr = 0x1;
    RF_cmdPropRxAdv.rxConf.bIncludeHdr = 0x1;
    RF_cmdPropRxAdv.rxConf.bIncludeCrc = 0x0;
    RF_cmdPropRxAdv.rxConf.bAppendRssi = 0x1;
    RF_cmdPropRxAdv.rxConf.bAppendTimestamp = 0x1;
    RF_cmdPropRxAdv.rxConf.bAppendStatus = 0x0;
    RF_cmdPropRxAdv.maxPktLen = macCfg.macMaxFrameSize;
    RF_cmdPropRxAdv.hdrConf.numHdrBits = 0x10;
    RF_cmdPropRxAdv.hdrConf.lenPos = 0x0;
    RF_cmdPropRxAdv.hdrConf.numLenBits = 0xB;
    RF_cmdPropRxAdv.lenOffset = 0xFC;
#ifdef RFCORE49
    RF_cmdPropRxAdv.endTrigger.triggerType = 0x4;
    RF_cmdPropRxAdv.endTrigger.bEnaCmd = 0x0;
    RF_cmdPropRxAdv.endTrigger.triggerNo = 0x0;
    RF_cmdPropRxAdv.endTrigger.pastTrig = 0x0;
    RF_cmdPropRxAdv.endTime = 1680000000;   //7 minutes
#endif

    memcpy(&RF_cmdPropCs, pRfCfg->pRfCs, sizeof(rfc_CMD_PROP_CS_t));
    RF_cmdPropCs.pNextOp = (rfc_radioOp_t *)&RF_cmdPropTxAdv;
    RF_cmdPropCs.startTrigger.triggerType = TRIG_REL_SUBMIT;
    RF_cmdPropCs.startTrigger.pastTrig = 1;
    RF_cmdPropCs.condition.rule = COND_STOP_ON_TRUE;
    RF_cmdPropCs.csConf.bEnaRssi = 0x1;
    RF_cmdPropCs.csConf.bEnaCorr = 0x0;
    RF_cmdPropCs.csConf.busyOp = 0x1;
    RF_cmdPropCs.csConf.idleOp = 0x1;
    RF_cmdPropCs.rssiThr = -83;
    RF_cmdPropCs.corrPeriod = 640;
    RF_cmdPropCs.corrConfig.numCorrInv = 0x03;
    RF_cmdPropCs.corrConfig.numCorrBusy = 0x00;
    RF_cmdPropCs.csEndTrigger.triggerType = TRIG_REL_START;
    RF_cmdPropCs.csEndTime = 8000; /* 2 ms timeout as a fallback */

    memcpy(&RF_cmdPropCsSlotted, pRfCfg->pRfCs, sizeof(rfc_CMD_PROP_CS_t));
    RF_cmdPropCsSlotted.pNextOp = (rfc_radioOp_t *)&RF_cmdPropCs;
    RF_cmdPropCsSlotted.startTrigger.triggerType = TRIG_ABSTIME;
    RF_cmdPropCsSlotted.startTrigger.pastTrig = 1;
    RF_cmdPropCsSlotted.condition.rule = COND_STOP_ON_TRUE;
    RF_cmdPropCsSlotted.csConf.bEnaRssi = 0x1;
    RF_cmdPropCsSlotted.csConf.bEnaCorr = 0x0; /* CC13xx sub-G can only support CCA mode 1 */
    RF_cmdPropCsSlotted.csConf.busyOp = 0x1;
    RF_cmdPropCsSlotted.csConf.idleOp = 0x1;
    RF_cmdPropCsSlotted.rssiThr = -83; /* -83 dBm */
    RF_cmdPropCsSlotted.corrPeriod = 640; /* Filler, used for correlation only */
    RF_cmdPropCsSlotted.corrConfig.numCorrInv = 0x03;
    RF_cmdPropCsSlotted.corrConfig.numCorrBusy = 0x00;
    RF_cmdPropCsSlotted.csEndTrigger.triggerType = TRIG_REL_START;
    RF_cmdPropCsSlotted.csEndTime = 8000; /* 2 ms timeout as a fallback */

    pMaxPower = (macRfCfg_maxPower_t *)pRfCfg->data;

    /* Setup MAC RX command */
    macSetupReceiveCmd();
}
#endif

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
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
};

// CMD_NOP Foreground command 0x0400 = FG
rfc_CMD_NOP_t RF_cmdNopFg =
{
    .commandNo = 0x0C01,
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
