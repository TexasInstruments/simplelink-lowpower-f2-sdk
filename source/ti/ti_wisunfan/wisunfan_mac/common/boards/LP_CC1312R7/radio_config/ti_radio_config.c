/*
 *  ======== ti_radio_config.c ========
 *  Configured RadioConfig module definitions
 *  
 *  Radio Config module version : 1.8
 *  SmartRF Studio data version : 2.20.0
 */

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rf_mailbox.h)
#include DeviceFamily_constructPath(driverlib/rf_common_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_prop_cmd.h)
#include <ti/drivers/rf/RF.h>
#include DeviceFamily_constructPath(rf_patches/rf_patch_cpe_prop.h)
#include "ti_radio_config.h"


//*********************************************************************************
//  RF Setting:   50 kbps, 2-GFSK, 25 kHz deviation, IEEE 802.15.4g MR-FSK PHY mode
//
//  PHY:          2gfsk50kbps154g
//  Setting file: setting_tc106_154g.json
//*********************************************************************************

// PARAMETER SUMMARY
// RX Address0: 0xAA
// RX Address1: 0xBB
// RX Address Mode: No address check
// Frequency (MHz): 868.0000
// Deviation (kHz): 25.0
// Fixed Packet Length: 20
// Packet Length Config: Variable
// Max Packet Length: 2047
// Preamble Count: 7 Bytes
// Preamble Mode: Send 0 as the first preamble bit
// RX Filter BW (kHz): 98.0
// Symbol Rate (kBaud): 50.000
// Sync Word: 0x55904E
// Sync Word Length: 24 Bits
// TX Power (dBm): 0
// Whitening: Dynamically IEEE 802.15.4g compatible whitener and 16/32-bit CRC

// TI-RTOS RF Mode Object
RF_Mode RF_prop_2gfsk50kbps154g =
{
    .rfMode = RF_MODE_AUTO,
    .cpePatchFxn = &rf_patch_cpe_prop,
    .mcePatchFxn = 0,
    .rfePatchFxn = 0
};

// TX Power table
// The RF_TxPowerTable_DEFAULT_PA_ENTRY and RF_TxPowerTable_HIGH_PA_ENTRY macros are defined in RF.h.
// The following arguments are required:
// RF_TxPowerTable_DEFAULT_PA_ENTRY(bias, gain, boost coefficient)
// RF_TxPowerTable_HIGH_PA_ENTRY(bias, ibboost, boost, coefficient, ldoTrim)
// See the Technical Reference Manual for further details about the "txPower" Command field.
// The PA settings require the CCFG_FORCE_VDDR_HH = 0 unless stated otherwise.
RF_TxPowerTable_Entry txPowerTable_2gfsk50kbps154g[TX_POWER_TABLE_SIZE_2gfsk50kbps154g] =
{
    {-20, RF_TxPowerTable_DEFAULT_PA_ENTRY(0, 3, 0, 2) },
    {-15, RF_TxPowerTable_DEFAULT_PA_ENTRY(1, 3, 0, 3) },
    {-10, RF_TxPowerTable_DEFAULT_PA_ENTRY(2, 3, 0, 5) },
    {-5, RF_TxPowerTable_DEFAULT_PA_ENTRY(4, 3, 0, 5) },
    {0, RF_TxPowerTable_DEFAULT_PA_ENTRY(8, 3, 0, 8) },
    {1, RF_TxPowerTable_DEFAULT_PA_ENTRY(9, 3, 0, 9) },
    {2, RF_TxPowerTable_DEFAULT_PA_ENTRY(10, 3, 0, 9) },
    {3, RF_TxPowerTable_DEFAULT_PA_ENTRY(11, 3, 0, 10) },
    {4, RF_TxPowerTable_DEFAULT_PA_ENTRY(13, 3, 0, 11) },
    {5, RF_TxPowerTable_DEFAULT_PA_ENTRY(14, 3, 0, 14) },
    {6, RF_TxPowerTable_DEFAULT_PA_ENTRY(17, 3, 0, 16) },
    {7, RF_TxPowerTable_DEFAULT_PA_ENTRY(20, 3, 0, 19) },
    {8, RF_TxPowerTable_DEFAULT_PA_ENTRY(24, 3, 0, 22) },
    {9, RF_TxPowerTable_DEFAULT_PA_ENTRY(28, 3, 0, 31) },
    {10, RF_TxPowerTable_DEFAULT_PA_ENTRY(18, 2, 0, 31) },
    {11, RF_TxPowerTable_DEFAULT_PA_ENTRY(26, 2, 0, 51) },
    {12, RF_TxPowerTable_DEFAULT_PA_ENTRY(16, 0, 0, 82) },
    // The original PA value (12.5 dBm) has been rounded to an integer value.
    {13, RF_TxPowerTable_DEFAULT_PA_ENTRY(36, 0, 0, 89) },
    // This setting requires CCFG_FORCE_VDDR_HH = 1.
    {14, RF_TxPowerTable_DEFAULT_PA_ENTRY(63, 0, 1, 0) },
    RF_TxPowerTable_TERMINATION_ENTRY
};

// Overrides for CMD_PROP_RADIO_DIV_SETUP
uint32_t pOverrides_2gfsk50kbps154g[] =
{
    // override_prop_common.json
    // DC/DC regulator: In Tx, use DCDCCTL5[3:0]=0x7 (DITHER_EN=0 and IPEAK=7).
    (uint32_t)0x00F788D3,
    // override_prop_common_sub1g.json
    // Set RF_FSCA.ANADIV.DIV_SEL_BIAS = 1. Bits [0:16, 24, 30] are don't care..
    (uint32_t)0x4001405D,
    // Set RF_FSCA.ANADIV.DIV_SEL_BIAS = 1. Bits [0:16, 24, 30] are don't care..
    (uint32_t)0x08141131,
    // override_tc106.json
    // Tx: Configure PA ramp time, PACTL2.RC=0x3 (in ADI0, set PACTL2[4:3]=0x3)
    ADI_2HALFREG_OVERRIDE(0,16,0x8,0x8,17,0x1,0x1),
    // Rx: Set AGC reference level to 0x1A (default: 0x2E)
    HW_REG_OVERRIDE(0x609C,0x001A),
    // Rx: Set RSSI offset to adjust reported RSSI by -1 dB (default: -2), trimmed for external bias and differential configuration
    (uint32_t)0x000188A3,
    // Rx: Set anti-aliasing filter bandwidth to 0xD (in ADI0, set IFAMPCTL3[7:4]=0xD)
    ADI_HALFREG_OVERRIDE(0,61,0xF,0xD),
    // Tx: Set wait time before turning off ramp to 0x1A (default: 0x1F)
    HW_REG_OVERRIDE(0x6028,0x001A),
    (uint32_t) 0x00018B03, // HW bit reverse for Sub-1 GHz
    (uint32_t)0xFFFFFFFF
};



// CMD_PROP_RADIO_DIV_SETUP
// Proprietary Mode Radio Setup Command for All Frequency Bands
const rfc_CMD_PROP_RADIO_DIV_SETUP_t RF_cmdPropRadioDivSetup_2gfsk50kbps154g =
{
    .commandNo = 0x3807,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .modulation.modType = 0x1,
    .modulation.deviation = 0x64,
    .modulation.deviationStepSz = 0x0,
    .symbolRate.preScale = 0xF,
    .symbolRate.rateWord = 0x8000,
    .symbolRate.decimMode = 0x0,
    .rxBw = 0x52,
    .preamConf.nPreamBytes = 0x7,
    .preamConf.preamMode = 0x0,
    .formatConf.nSwBits = 0x18,
    .formatConf.bBitReversal = 0x0,
    .formatConf.bMsbFirst = 0x1,
    .formatConf.fecMode = 0x0,
    .formatConf.whitenMode = 0x7,
    .config.frontEndMode = 0x0,
    .config.biasMode = 0x1,
    .config.analogCfgMode = 0x0,
    .config.bNoFsPowerUp = 0x0,
    .config.bSynthNarrowBand = 0x0,
    .txPower = 0x10C8,
    .pRegOverride = pOverrides_2gfsk50kbps154g,
    .centerFreq = 0x0364,
    .intFreq = 0x8000,
    .loDivider = 0x05
};

// CMD_FS
// Frequency Synthesizer Programming Command
const rfc_CMD_FS_t RF_cmdFs_2gfsk50kbps154g =
{
    .commandNo = 0x0803,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .frequency = 0x0364,
    .fractFreq = 0x0000,
    .synthConf.bTxMode = 0x0,
    .synthConf.refFreq = 0x0,
    .__dummy0 = 0x00,
    .__dummy1 = 0x00,
    .__dummy2 = 0x00,
    .__dummy3 = 0x0000
};

// CMD_PROP_TX_ADV
// Proprietary Mode Advanced Transmit Command
const rfc_CMD_PROP_TX_ADV_t RF_cmdPropTxAdv_2gfsk50kbps154g =
{
    .commandNo = 0x3803,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x2,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x1,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .pktConf.bFsOff = 0x0,
    .pktConf.bUseCrc = 0x1,
    .pktConf.bCrcIncSw = 0x0,
    .pktConf.bCrcIncHdr = 0x0,
    .numHdrBits = 0x10,
    .pktLen = 0x0014,
    .startConf.bExtTxTrig = 0x0,
    .startConf.inputMode = 0x0,
    .startConf.source = 0x0,
    .preTrigger.triggerType = 0x4,
    .preTrigger.bEnaCmd = 0x0,
    .preTrigger.triggerNo = 0x0,
    .preTrigger.pastTrig = 0x1,
    .preTime = 0x00000000,
    .syncWord = 0x0055904E,
    .pPkt = 0
};

// CMD_PROP_RX_ADV
// Proprietary Mode Advanced Receive Command
const rfc_CMD_PROP_RX_ADV_t RF_cmdPropRxAdv_2gfsk50kbps154g =
{
    .commandNo = 0x3804,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .pktConf.bFsOff = 0x0,
    .pktConf.bRepeatOk = 0x0,
    .pktConf.bRepeatNok = 0x0,
    .pktConf.bUseCrc = 0x1,
    .pktConf.bCrcIncSw = 0x0,
    .pktConf.bCrcIncHdr = 0x0,
    .pktConf.endType = 0x0,
    .pktConf.filterOp = 0x1,
    .rxConf.bAutoFlushIgnored = 0x1,
    .rxConf.bAutoFlushCrcErr = 0x0,
    .rxConf.bIncludeHdr = 0x1,
    .rxConf.bIncludeCrc = 0x1,
    .rxConf.bAppendRssi = 0x1,
    .rxConf.bAppendTimestamp = 0x1,
    .rxConf.bAppendStatus = 0x1,
    .syncWord0 = 0x0055904E,
    .syncWord1 = 0x00000000,
    .maxPktLen = 0x07FF,
    .hdrConf.numHdrBits = 0x10,
    .hdrConf.lenPos = 0x0,
    .hdrConf.numLenBits = 0xB,
    .addrConf.addrType = 0x0,
    .addrConf.addrSize = 0x0,
    .addrConf.addrPos = 0x0,
    .addrConf.numAddr = 0x0,
    .lenOffset = 0xFC,
    .endTrigger.triggerType = 0x1,
    .endTrigger.bEnaCmd = 0x0,
    .endTrigger.triggerNo = 0x0,
    .endTrigger.pastTrig = 0x0,
    .endTime = 0x00000000,
    .pAddr = 0,
    .pQueue = 0,
    .pOutput = 0
};

// CMD_PROP_CS
// Carrier Sense Command
const rfc_CMD_PROP_CS_t RF_cmdPropCs_2gfsk50kbps154g =
{
    .commandNo = 0x3805,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x0,
    .condition.nSkip = 0x0,
    .csFsConf.bFsOffIdle = 0x0,
    .csFsConf.bFsOffBusy = 0x0,
    .__dummy0 = 0x00,
    .csConf.bEnaRssi = 0x0,
    .csConf.bEnaCorr = 0x0,
    .csConf.operation = 0x0,
    .csConf.busyOp = 0x0,
    .csConf.idleOp = 0x0,
    .csConf.timeoutRes = 0x0,
    .rssiThr = 0x00,
    .numRssiIdle = 0x00,
    .numRssiBusy = 0x00,
    .corrPeriod = 0x0000,
    .corrConfig.numCorrInv = 0x0,
    .corrConfig.numCorrBusy = 0x0,
    .csEndTrigger.triggerType = 0x0,
    .csEndTrigger.bEnaCmd = 0x0,
    .csEndTrigger.triggerNo = 0x0,
    .csEndTrigger.pastTrig = 0x0,
    .csEndTime = 0x00000000
};


//*********************************************************************************
//  RF Setting:   200 kbps, 2-GFSK, 50 kHz deviation, IEEE 802.15.4g FSK PHY mode, 915 MHz
//
//  PHY:          2gfsk200kbps154g
//  Setting file: setting_tc146_154g.json
//*********************************************************************************

// PARAMETER SUMMARY
// RX Address0: 0xAA
// RX Address1: 0xBB
// RX Address Mode: No address check
// Frequency (MHz): 915.0000
// Deviation (kHz): 50.0
// Fixed Packet Length: 20
// Packet Length Config: Variable
// Max Packet Length: 2047
// Preamble Count: 7 Bytes
// Preamble Mode: Send 0 as the first preamble bit
// RX Filter BW (kHz): 310.8
// Symbol Rate (kBaud): 200.000
// Sync Word: 0x55904E
// Sync Word Length: 24 Bits
// TX Power (dBm): 0
// Whitening: Dynamically IEEE 802.15.4g compatible whitener and 16/32-bit CRC

// TI-RTOS RF Mode Object
RF_Mode RF_prop_2gfsk200kbps154g =
{
    .rfMode = RF_MODE_AUTO,
    .cpePatchFxn = &rf_patch_cpe_prop,
    .mcePatchFxn = 0,
    .rfePatchFxn = 0
};

// TX Power table
// The RF_TxPowerTable_DEFAULT_PA_ENTRY and RF_TxPowerTable_HIGH_PA_ENTRY macros are defined in RF.h.
// The following arguments are required:
// RF_TxPowerTable_DEFAULT_PA_ENTRY(bias, gain, boost coefficient)
// RF_TxPowerTable_HIGH_PA_ENTRY(bias, ibboost, boost, coefficient, ldoTrim)
// See the Technical Reference Manual for further details about the "txPower" Command field.
// The PA settings require the CCFG_FORCE_VDDR_HH = 0 unless stated otherwise.
RF_TxPowerTable_Entry txPowerTable_2gfsk200kbps154g[TX_POWER_TABLE_SIZE_2gfsk200kbps154g] =
{
    {-20, RF_TxPowerTable_DEFAULT_PA_ENTRY(0, 3, 0, 2) },
    {-15, RF_TxPowerTable_DEFAULT_PA_ENTRY(1, 3, 0, 3) },
    {-10, RF_TxPowerTable_DEFAULT_PA_ENTRY(2, 3, 0, 5) },
    {-5, RF_TxPowerTable_DEFAULT_PA_ENTRY(4, 3, 0, 5) },
    {0, RF_TxPowerTable_DEFAULT_PA_ENTRY(8, 3, 0, 8) },
    {1, RF_TxPowerTable_DEFAULT_PA_ENTRY(9, 3, 0, 9) },
    {2, RF_TxPowerTable_DEFAULT_PA_ENTRY(10, 3, 0, 9) },
    {3, RF_TxPowerTable_DEFAULT_PA_ENTRY(11, 3, 0, 10) },
    {4, RF_TxPowerTable_DEFAULT_PA_ENTRY(13, 3, 0, 11) },
    {5, RF_TxPowerTable_DEFAULT_PA_ENTRY(14, 3, 0, 14) },
    {6, RF_TxPowerTable_DEFAULT_PA_ENTRY(17, 3, 0, 16) },
    {7, RF_TxPowerTable_DEFAULT_PA_ENTRY(20, 3, 0, 19) },
    {8, RF_TxPowerTable_DEFAULT_PA_ENTRY(24, 3, 0, 22) },
    {9, RF_TxPowerTable_DEFAULT_PA_ENTRY(28, 3, 0, 31) },
    {10, RF_TxPowerTable_DEFAULT_PA_ENTRY(18, 2, 0, 31) },
    {11, RF_TxPowerTable_DEFAULT_PA_ENTRY(26, 2, 0, 51) },
    {12, RF_TxPowerTable_DEFAULT_PA_ENTRY(16, 0, 0, 82) },
    // The original PA value (12.5 dBm) has been rounded to an integer value.
    {13, RF_TxPowerTable_DEFAULT_PA_ENTRY(36, 0, 0, 89) },
    // This setting requires CCFG_FORCE_VDDR_HH = 1.
    {14, RF_TxPowerTable_DEFAULT_PA_ENTRY(63, 0, 1, 0) },
    RF_TxPowerTable_TERMINATION_ENTRY
};

// Overrides for CMD_PROP_RADIO_DIV_SETUP
uint32_t pOverrides_2gfsk200kbps154g[] =
{
    // override_prop_common.json
    // DC/DC regulator: In Tx, use DCDCCTL5[3:0]=0x7 (DITHER_EN=0 and IPEAK=7).
    (uint32_t)0x00F788D3,
    // override_prop_common_sub1g.json
    // Set RF_FSCA.ANADIV.DIV_SEL_BIAS = 1. Bits [0:16, 24, 30] are don't care..
    (uint32_t)0x4001405D,
    // Set RF_FSCA.ANADIV.DIV_SEL_BIAS = 1. Bits [0:16, 24, 30] are don't care..
    (uint32_t)0x08141131,
    // override_tc146.json
    // Tx: Configure PA ramp time, PACTL2.RC=0x3 (in ADI0, set PACTL2[4:3]=0x3)
    ADI_2HALFREG_OVERRIDE(0,16,0x8,0x8,17,0x1,0x1),
    // Tx: Configure PA ramping, set wait time before turning off (0x1A ticks of 16/24 us = 17.3 us).
    HW_REG_OVERRIDE(0x6028,0x001A),
    // Rx: Set AGC reference level to 0x16 (default: 0x2E)
    HW_REG_OVERRIDE(0x609C,0x0016),
    // Rx: Set RSSI offset to adjust reported RSSI by -1 dB (default: -2), trimmed for external bias and differential configuration
    (uint32_t)0x000188A3,
    // Rx: Set anti-aliasing filter bandwidth to 0x8 (in ADI0, set IFAMPCTL3[7:4]=0x8)
    ADI_HALFREG_OVERRIDE(0,61,0xF,0x8),
    (uint32_t) 0x00018B03, // HW bit reverse for Sub-1 GHz
    (uint32_t)0xFFFFFFFF
};



// CMD_PROP_RADIO_DIV_SETUP
// Proprietary Mode Radio Setup Command for All Frequency Bands
const rfc_CMD_PROP_RADIO_DIV_SETUP_t RF_cmdPropRadioDivSetup_2gfsk200kbps154g =
{
    .commandNo = 0x3807,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .modulation.modType = 0x1,
    .modulation.deviation = 0xC8,
    .modulation.deviationStepSz = 0x0,
    .symbolRate.preScale = 0xF,
    .symbolRate.rateWord = 0x20000,
    .symbolRate.decimMode = 0x0,
    .rxBw = 0x59,
    .preamConf.nPreamBytes = 0x7,
    .preamConf.preamMode = 0x0,
    .formatConf.nSwBits = 0x18,
    .formatConf.bBitReversal = 0x0,
    .formatConf.bMsbFirst = 0x1,
    .formatConf.fecMode = 0x0,
    .formatConf.whitenMode = 0x7,
    .config.frontEndMode = 0x0,
    .config.biasMode = 0x1,
    .config.analogCfgMode = 0x0,
    .config.bNoFsPowerUp = 0x0,
    .config.bSynthNarrowBand = 0x0,
    .txPower = 0x10C8,
    .pRegOverride = pOverrides_2gfsk200kbps154g,
    .centerFreq = 0x0393,
    .intFreq = 0x0999,
    .loDivider = 0x05
};

// CMD_FS
// Frequency Synthesizer Programming Command
const rfc_CMD_FS_t RF_cmdFs_2gfsk200kbps154g =
{
    .commandNo = 0x0803,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .frequency = 0x0393,
    .fractFreq = 0x0000,
    .synthConf.bTxMode = 0x0,
    .synthConf.refFreq = 0x0,
    .__dummy0 = 0x00,
    .__dummy1 = 0x00,
    .__dummy2 = 0x00,
    .__dummy3 = 0x0000
};

// CMD_PROP_TX_ADV
// Proprietary Mode Advanced Transmit Command
const rfc_CMD_PROP_TX_ADV_t RF_cmdPropTxAdv_2gfsk200kbps154g =
{
    .commandNo = 0x3803,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x2,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x1,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .pktConf.bFsOff = 0x0,
    .pktConf.bUseCrc = 0x1,
    .pktConf.bCrcIncSw = 0x0,
    .pktConf.bCrcIncHdr = 0x0,
    .numHdrBits = 0x10,
    .pktLen = 0x0014,
    .startConf.bExtTxTrig = 0x0,
    .startConf.inputMode = 0x0,
    .startConf.source = 0x0,
    .preTrigger.triggerType = 0x4,
    .preTrigger.bEnaCmd = 0x0,
    .preTrigger.triggerNo = 0x0,
    .preTrigger.pastTrig = 0x1,
    .preTime = 0x00000000,
    .syncWord = 0x0055904E,
    .pPkt = 0
};

// CMD_PROP_RX_ADV
// Proprietary Mode Advanced Receive Command
const rfc_CMD_PROP_RX_ADV_t RF_cmdPropRxAdv_2gfsk200kbps154g =
{
    .commandNo = 0x3804,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .pktConf.bFsOff = 0x0,
    .pktConf.bRepeatOk = 0x0,
    .pktConf.bRepeatNok = 0x0,
    .pktConf.bUseCrc = 0x1,
    .pktConf.bCrcIncSw = 0x0,
    .pktConf.bCrcIncHdr = 0x0,
    .pktConf.endType = 0x0,
    .pktConf.filterOp = 0x1,
    .rxConf.bAutoFlushIgnored = 0x1,
    .rxConf.bAutoFlushCrcErr = 0x0,
    .rxConf.bIncludeHdr = 0x1,
    .rxConf.bIncludeCrc = 0x1,
    .rxConf.bAppendRssi = 0x1,
    .rxConf.bAppendTimestamp = 0x1,
    .rxConf.bAppendStatus = 0x1,
    .syncWord0 = 0x0055904E,
    .syncWord1 = 0x00000000,
    .maxPktLen = 0x07FF,
    .hdrConf.numHdrBits = 0x10,
    .hdrConf.lenPos = 0x0,
    .hdrConf.numLenBits = 0xB,
    .addrConf.addrType = 0x0,
    .addrConf.addrSize = 0x0,
    .addrConf.addrPos = 0x0,
    .addrConf.numAddr = 0x0,
    .lenOffset = 0xFC,
    .endTrigger.triggerType = 0x1,
    .endTrigger.bEnaCmd = 0x0,
    .endTrigger.triggerNo = 0x0,
    .endTrigger.pastTrig = 0x0,
    .endTime = 0x00000000,
    .pAddr = 0,
    .pQueue = 0,
    .pOutput = 0
};

// CMD_PROP_CS
// Carrier Sense Command
const rfc_CMD_PROP_CS_t RF_cmdPropCs_2gfsk200kbps154g =
{
    .commandNo = 0x3805,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x0,
    .condition.nSkip = 0x0,
    .csFsConf.bFsOffIdle = 0x0,
    .csFsConf.bFsOffBusy = 0x0,
    .__dummy0 = 0x00,
    .csConf.bEnaRssi = 0x0,
    .csConf.bEnaCorr = 0x0,
    .csConf.operation = 0x0,
    .csConf.busyOp = 0x0,
    .csConf.idleOp = 0x0,
    .csConf.timeoutRes = 0x0,
    .rssiThr = 0x00,
    .numRssiIdle = 0x00,
    .numRssiBusy = 0x00,
    .corrPeriod = 0x0000,
    .corrConfig.numCorrInv = 0x0,
    .corrConfig.numCorrBusy = 0x0,
    .csEndTrigger.triggerType = 0x0,
    .csEndTrigger.bEnaCmd = 0x0,
    .csEndTrigger.triggerNo = 0x0,
    .csEndTrigger.pastTrig = 0x0,
    .csEndTime = 0x00000000
};


//*********************************************************************************
//  RF Setting:   5 kbps, SimpleLink Long Range (20 kchip/s, 2-GFSK, conv. FEC r=1/2 K=7, DSSS SF=2, Tx dev.: 5 kHz, Rx BW: 34 kHz)
//
//  PHY:          slr5kbps2gfsk
//  Setting file: setting_tc480.json
//*********************************************************************************

// PARAMETER SUMMARY
// RX Address0: 0xAA
// RX Address1: 0xBB
// RX Address Mode: No address check
// Frequency (MHz): 868.0000
// Deviation (kHz): 5.0
// Fixed Packet Length: 20
// Packet Length Config: Variable
// Max Packet Length: 255
// Preamble Count: 2 Bytes
// Preamble Mode: Send 0 as the first preamble bit
// RX Filter BW (kHz): 34.1
// Symbol Rate (kBaud): 20.000
// Sync Word: 0x0
// Sync Word Length: 32 Bits
// TX Power (dBm): 0
// Whitening: Dynamically IEEE 802.15.4g compatible whitener and 16/32-bit CRC

// TI-RTOS RF Mode Object
RF_Mode RF_prop_slr5kbps2gfsk =
{
    .rfMode = RF_MODE_AUTO,
    .cpePatchFxn = &rf_patch_cpe_prop,
    .mcePatchFxn = 0,
    .rfePatchFxn = 0
};

// TX Power table
// The RF_TxPowerTable_DEFAULT_PA_ENTRY and RF_TxPowerTable_HIGH_PA_ENTRY macros are defined in RF.h.
// The following arguments are required:
// RF_TxPowerTable_DEFAULT_PA_ENTRY(bias, gain, boost coefficient)
// RF_TxPowerTable_HIGH_PA_ENTRY(bias, ibboost, boost, coefficient, ldoTrim)
// See the Technical Reference Manual for further details about the "txPower" Command field.
// The PA settings require the CCFG_FORCE_VDDR_HH = 0 unless stated otherwise.
RF_TxPowerTable_Entry txPowerTable_slr5kbps2gfsk[TX_POWER_TABLE_SIZE_slr5kbps2gfsk] =
{
    {-20, RF_TxPowerTable_DEFAULT_PA_ENTRY(0, 3, 0, 2) },
    {-15, RF_TxPowerTable_DEFAULT_PA_ENTRY(1, 3, 0, 3) },
    {-10, RF_TxPowerTable_DEFAULT_PA_ENTRY(2, 3, 0, 5) },
    {-5, RF_TxPowerTable_DEFAULT_PA_ENTRY(4, 3, 0, 5) },
    {0, RF_TxPowerTable_DEFAULT_PA_ENTRY(8, 3, 0, 8) },
    {1, RF_TxPowerTable_DEFAULT_PA_ENTRY(9, 3, 0, 9) },
    {2, RF_TxPowerTable_DEFAULT_PA_ENTRY(10, 3, 0, 9) },
    {3, RF_TxPowerTable_DEFAULT_PA_ENTRY(11, 3, 0, 10) },
    {4, RF_TxPowerTable_DEFAULT_PA_ENTRY(13, 3, 0, 11) },
    {5, RF_TxPowerTable_DEFAULT_PA_ENTRY(14, 3, 0, 14) },
    {6, RF_TxPowerTable_DEFAULT_PA_ENTRY(17, 3, 0, 16) },
    {7, RF_TxPowerTable_DEFAULT_PA_ENTRY(20, 3, 0, 19) },
    {8, RF_TxPowerTable_DEFAULT_PA_ENTRY(24, 3, 0, 22) },
    {9, RF_TxPowerTable_DEFAULT_PA_ENTRY(28, 3, 0, 31) },
    {10, RF_TxPowerTable_DEFAULT_PA_ENTRY(18, 2, 0, 31) },
    {11, RF_TxPowerTable_DEFAULT_PA_ENTRY(26, 2, 0, 51) },
    {12, RF_TxPowerTable_DEFAULT_PA_ENTRY(16, 0, 0, 82) },
    // The original PA value (12.5 dBm) has been rounded to an integer value.
    {13, RF_TxPowerTable_DEFAULT_PA_ENTRY(36, 0, 0, 89) },
    // This setting requires CCFG_FORCE_VDDR_HH = 1.
    {14, RF_TxPowerTable_DEFAULT_PA_ENTRY(63, 0, 1, 0) },
    RF_TxPowerTable_TERMINATION_ENTRY
};

// Overrides for CMD_PROP_RADIO_DIV_SETUP
uint32_t pOverrides_slr5kbps2gfsk[] =
{
    // override_prop_common.json
    // DC/DC regulator: In Tx, use DCDCCTL5[3:0]=0x7 (DITHER_EN=0 and IPEAK=7).
    (uint32_t)0x00F788D3,
    // override_prop_common_sub1g.json
    // Set RF_FSCA.ANADIV.DIV_SEL_BIAS = 1. Bits [0:16, 24, 30] are don't care..
    (uint32_t)0x4001405D,
    // Set RF_FSCA.ANADIV.DIV_SEL_BIAS = 1. Bits [0:16, 24, 30] are don't care..
    (uint32_t)0x08141131,
    // override_phy_simplelink_long_range_dsss2.json
    // PHY: Configure DSSS SF=2 for payload data
    HW_REG_OVERRIDE(0x5068,0x0100),
    // override_tc480_tc481_tc482.json
    // Tx: Configure PA ramp time, PACTL2.RC=0x3 (in ADI0, set PACTL2[4:3]=0x3)
    ADI_2HALFREG_OVERRIDE(0,16,0x8,0x8,17,0x1,0x1),
    // Rx: Set AGC reference level to 0x17 (default: 0x2E)
    HW_REG_OVERRIDE(0x609C,0x0017),
    // Rx: Set RSSI offset to adjust reported RSSI by -2 dB (default: -2), trimmed for external bias and differential configuration
    (uint32_t)0x000288A3,
    // Rx: Set anti-aliasing filter bandwidth to 0xD (in ADI0, set IFAMPCTL3[7:4]=0xD)
    ADI_HALFREG_OVERRIDE(0,61,0xF,0xD),
    // Tx: Set wait time before turning off ramp to 0x1A (default: 0x1F)
    HW_REG_OVERRIDE(0x6028,0x001A),
    (uint32_t) 0x00018B03, // HW bit reverse for Sub-1 GHz
    (uint32_t)0xFFFFFFFF
};



// CMD_PROP_RADIO_DIV_SETUP
// Proprietary Mode Radio Setup Command for All Frequency Bands
const rfc_CMD_PROP_RADIO_DIV_SETUP_t RF_cmdPropRadioDivSetup_slr5kbps2gfsk =
{
    .commandNo = 0x3807,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .modulation.modType = 0x1,
    .modulation.deviation = 0x14,
    .modulation.deviationStepSz = 0x0,
    .symbolRate.preScale = 0xF,
    .symbolRate.rateWord = 0x3333,
    .symbolRate.decimMode = 0x0,
    .rxBw = 0x4C,
    .preamConf.nPreamBytes = 0x2,
    .preamConf.preamMode = 0x0,
    .formatConf.nSwBits = 0x20,
    .formatConf.bBitReversal = 0x0,
    .formatConf.bMsbFirst = 0x0,
    .formatConf.fecMode = 0x8,
    .formatConf.whitenMode = 0x7,
    .config.frontEndMode = 0x0,
    .config.biasMode = 0x1,
    .config.analogCfgMode = 0x0,
    .config.bNoFsPowerUp = 0x0,
    .config.bSynthNarrowBand = 0x0,
    .txPower = 0x10C8,
    .pRegOverride = pOverrides_slr5kbps2gfsk,
    .centerFreq = 0x0364,
    .intFreq = 0x8000,
    .loDivider = 0x05
};

// CMD_FS
// Frequency Synthesizer Programming Command
const rfc_CMD_FS_t RF_cmdFs_slr5kbps2gfsk =
{
    .commandNo = 0x0803,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .frequency = 0x0364,
    .fractFreq = 0x0000,
    .synthConf.bTxMode = 0x0,
    .synthConf.refFreq = 0x0,
    .__dummy0 = 0x00,
    .__dummy1 = 0x00,
    .__dummy2 = 0x00,
    .__dummy3 = 0x0000
};

// CMD_PROP_TX_ADV
// Proprietary Mode Advanced Transmit Command
const rfc_CMD_PROP_TX_ADV_t RF_cmdPropTxAdv_slr5kbps2gfsk =
{
    .commandNo = 0x3803,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x0,
    .condition.nSkip = 0x0,
    .pktConf.bFsOff = 0x0,
    .pktConf.bUseCrc = 0x0,
    .pktConf.bCrcIncSw = 0x0,
    .pktConf.bCrcIncHdr = 0x0,
    .numHdrBits = 0x00,
    .pktLen = 0x0014,
    .startConf.bExtTxTrig = 0x0,
    .startConf.inputMode = 0x0,
    .startConf.source = 0x0,
    .preTrigger.triggerType = 0x0,
    .preTrigger.bEnaCmd = 0x0,
    .preTrigger.triggerNo = 0x0,
    .preTrigger.pastTrig = 0x0,
    .preTime = 0x00000000,
    .syncWord = 0x00000000,
    .pPkt = 0
};

// CMD_PROP_RX_ADV
// Proprietary Mode Advanced Receive Command
const rfc_CMD_PROP_RX_ADV_t RF_cmdPropRxAdv_slr5kbps2gfsk =
{
    .commandNo = 0x3804,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x0,
    .condition.nSkip = 0x0,
    .pktConf.bFsOff = 0x0,
    .pktConf.bRepeatOk = 0x0,
    .pktConf.bRepeatNok = 0x0,
    .pktConf.bUseCrc = 0x0,
    .pktConf.bCrcIncSw = 0x0,
    .pktConf.bCrcIncHdr = 0x0,
    .pktConf.endType = 0x0,
    .pktConf.filterOp = 0x0,
    .rxConf.bAutoFlushIgnored = 0x0,
    .rxConf.bAutoFlushCrcErr = 0x0,
    .rxConf.bIncludeHdr = 0x0,
    .rxConf.bIncludeCrc = 0x0,
    .rxConf.bAppendRssi = 0x0,
    .rxConf.bAppendTimestamp = 0x0,
    .rxConf.bAppendStatus = 0x0,
    .syncWord0 = 0x00000000,
    .syncWord1 = 0x00000000,
    .maxPktLen = 0x00FF,
    .hdrConf.numHdrBits = 0x0,
    .hdrConf.lenPos = 0x0,
    .hdrConf.numLenBits = 0x0,
    .addrConf.addrType = 0x0,
    .addrConf.addrSize = 0x0,
    .addrConf.addrPos = 0x0,
    .addrConf.numAddr = 0x0,
    .lenOffset = 0x00,
    .endTrigger.triggerType = 0x0,
    .endTrigger.bEnaCmd = 0x0,
    .endTrigger.triggerNo = 0x0,
    .endTrigger.pastTrig = 0x0,
    .endTime = 0x00000000,
    .pAddr = 0,
    .pQueue = 0,
    .pOutput = 0
};

// CMD_PROP_CS
// Carrier Sense Command
const rfc_CMD_PROP_CS_t RF_cmdPropCs_slr5kbps2gfsk =
{
    .commandNo = 0x3805,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x0,
    .condition.nSkip = 0x0,
    .csFsConf.bFsOffIdle = 0x0,
    .csFsConf.bFsOffBusy = 0x0,
    .__dummy0 = 0x00,
    .csConf.bEnaRssi = 0x0,
    .csConf.bEnaCorr = 0x0,
    .csConf.operation = 0x0,
    .csConf.busyOp = 0x0,
    .csConf.idleOp = 0x0,
    .csConf.timeoutRes = 0x0,
    .rssiThr = 0x00,
    .numRssiIdle = 0x00,
    .numRssiBusy = 0x00,
    .corrPeriod = 0x0000,
    .corrConfig.numCorrInv = 0x0,
    .corrConfig.numCorrBusy = 0x0,
    .csEndTrigger.triggerType = 0x0,
    .csEndTrigger.bEnaCmd = 0x0,
    .csEndTrigger.triggerNo = 0x0,
    .csEndTrigger.pastTrig = 0x0,
    .csEndTime = 0x00000000
};

