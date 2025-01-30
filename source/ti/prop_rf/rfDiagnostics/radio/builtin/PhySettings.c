/*
 * Copyright (c) 2015-2022, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Includes Files */
#include "PhySettings.h"
#include <stdint.h>

// TI Drivers
#include <ti/drivers/rf/RF.h>
#include <ti_drivers_config.h>

// Radio Config
#include <ti_radio_config.h>

rfc_CMD_PROP_TX_t RF_cmdPropTxBle;
rfc_CMD_PROP_RX_t RF_cmdPropRxBle;

/*
 *  Read this section on what steps are required to add a new PHY from SysConfig.
 *
 *  Consider the new PHY below (available in ti_radio_config.c)
 *
 *  ***********************************************************************************
 *  RF Setting:   Setting Description
 *
 *  PHY:          Phy Name
 *  Setting file: setting_tcxxx.json
 *  ***********************************************************************************
 *
 *  // PA table usage
 *  #define TX_POWER_TABLE_SIZE_xxxx_x TXPOWERTABLE_xxx_PAxx_SIZE
 *
 *  #define txPowerTable_xxxx_x txPowerTable_xxx_paxx
 *
 *  // TI-RTOS RF Mode object
 *  extern RF_Mode RF_prop_xxxx_x;
 *
 *  // RF Core API commands
 *  extern rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t RF_cmdPropRadioDivSetup_xxxx_x;
 *  extern rfc_CMD_FS_t RF_cmdFs_xxxx_x;
 *  extern rfc_CMD_PROP_TX_ADV_t RF_cmdPropTxAdv_xxxx_x;
 *  extern rfc_CMD_PROP_RX_ADV_t RF_cmdPropRxAdv_xxxx_x;
 *
 *
 *  // RF Core API overrides
 *  extern uint32_t pOverrides_xxxx_x[];
 *
 *  The following must be added to PhySettings_supportedPhys:
 *
 *  {
 *      .PhySettings_phyIndex       = ?,
 *      .PhySettings_phyName        = ?,
 *      .PhySettings_apiType        = ?,
 *      .PhySettings_testType       = ?,
 *      .RF_pMode                   = ?,
 *      .RF_pCmdPropRadioDivSetupPa = ?,
 *      .RF_pCmdPropRadioDivSetup   = ?,
 *      .RF_pCmdBle5RadioSetupPa    = ?,
 *      .RF_pCmdBle5RadioSetup      = ?,
 *      .RF_pCmdRadioSetupPa        = ?,
 *      .RF_pCmdRadioSetup          = ?,
 *      .PhySettings_setupType      = ?,
 *      .RF_pCmdFs                  = ?,
 *      .RF_pCmdPropTx              = ?,
 *      .RF_pCmdPropTxAdv           = ?,
 *      .RF_pCmdPropRx              = ?,
 *      .RF_pCmdPropRxAdv           = ?,
 *      .RF_pCmdBle5AdvAux          = ?,
 *      .RF_pCmdBle5GenericRx       = ?,
 *      .RF_pCmdIeeeTx              = ?,
 *      .RF_pCmdIeeeRx              = ?,
 *      .PhySettings_cmdType        = ?,
 *      .RF_pTxPowerTable           = ?,
 *      .RF_txPowerTableSize        = ?,
 *  },
 *
 *  How to fill in the different fields:
 *  {
 *      // Phy index starting at 0
 *      // 0 <= X <= 9
 *      // If more than 10 PHYs is required, PhySettingsExample_phyIndex in phySettings.h must be modified
 *
 *      .PhySettings_phyIndex                = PhySettings_PHY_X,
 *
 *      // PHY Name
 *      // Here you can put anything you want (max 70 char) but a good rule of thumb is to include
 *      // PHY Index, Phy Name as given in ti_radio_config.c, and Test Type (see example below)
 *
 *      .PhySettings_phyName                 = {"PHY 0: custom433, PhySettings_TEST_STUDIO_COMPL"}
 *
 *      // API type, where X = {PROP, BLE, IEEE}
 *
 *      .PhySettings_apiType                 = PhySettings_API_X,
 *
 *      // Test type, where X = {RF_PERF, STUDIO_COMPL}
 *      // RF_PERF (RF Performance Test):                   All packets uses the same format independent of the
 *      //                                                  packet format used by SmartRF Studio. In this mode,
 *      //                                                  the BLE PHY is used together with PROP API commands.
 *      //                                                  The PROP API is not supported for the IEEE PHY (2.4 GHz),
 *      //                                                  so in this case, the IEEE API is used for
 *      //                                                  performance testing also
 *      // STUDIO_COMPL (SmartRF Studio Compliant Test):    Packets can be sent to SmartRF STudio
 *      //                                                  and received from SmartRF Studio
 *
 *      .PhySettings_testType                = PhySettings_TEST_X,
 *
 *      // TI-RTOS RF Mode object as given by ti_radio_config.c (generated by SysConfig)
 *
 *      .RF_pMode                   = &RF_prop_xxxx_x,
 *
 *      // Setup Command as given by ti_radio_config.c (generated by SysConfig)
 *      // Set all others to NULL
 *
 *      .RF_pCmdPropRadioDivSetupPa = &RF_cmdPropRadioDivSetup_xxxx_x,  // rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t (PROP_PA)
 *      .RF_pCmdPropRadioDivSetup   = NULL,                             // rfc_CMD_PROP_RADIO_DIV_SETUP_t    (PROP)
 *      .RF_pCmdBle5RadioSetupPa    = NULL,                             // rfc_CMD_BLE5_RADIO_SETUP_PA_t     (BLE_PA)
 *      .RF_pCmdBle5RadioSetup      = NULL,                             // rfc_CMD_BLE5_RADIO_SETUP_t        (BLE)
 *      .RF_pCmdRadioSetupPa        = NULL,                             // rfc_CMD_RADIO_SETUP_PA_t          (IEEE_PA)
 *      .RF_pCmdRadioSetup          = NULL,                             // rfc_CMD_RADIO_SETUP_t             (IEEE)
 *
 *      // Setup Command type, where X = {PROP, BLE, IEEE, PROP_PA, BLE_PA, IEEE_PA}                         (See above)
 *
 *      .PhySettings_setupType               = PhySettings_SETUP_X,
 *
 *      // FS Command as given by ti_radio_config.c (generated by SysConfig)
 *
 *      .RF_pCmdFs                  = &RF_cmdFs_xxxx_x,
 *
 *      // TX/RX commands as given by ti_radio_config.c (generated by SysConfig)
 *      // The only exception is when .PhySettings_apiType = PhySettings_API_BLE and .PhySettings_testType = PhySettings_TEST_RF_PERF,
 *      // then use RF_cmdPropTxBle, and RF_cmdPropRxBle
 *      // Set all others to NULL
 *
 *      .RF_pCmdPropTx              = NULL,                             // rfc_CMD_PROP_TX_t         (STANDARD)
 *      .RF_pCmdPropTxAdv           = &RF_cmdPropTxAdv_xxxx_x,          // rfc_CMD_PROP_TX_ADV_t     (ADVANCED)
 *      .RF_pCmdPropRx              = NULL,                             // rfc_CMD_PROP_TX_t         (STANDARD)
 *      .RF_pCmdPropRxAdv           = &RF_cmdPropRxAdv_xxxx_x,          // rfc_CMD_PROP_RX_ADV_t     (ADVANCED)
 *      .RF_pCmdBle5AdvAux          = NULL,                             // rfc_CMD_BLE5_ADV_AUX_t    (Don't care)
 *      .RF_pCmdBle5GenericRx       = NULL,                             // rfc_CMD_BLE5_GENERIC_RX_t (Don't care)
 *      .RF_pCmdIeeeTx              = NULL,                             // rfc_CMD_IEEE_TX_t         (Don't care)
        .RF_pCmdIeeeRx              = NULL,                             // rfc_CMD_IEEE_RX_t         (Don't care)
 *
 *      // Setup the command type, where X = {STANDARD, ADVANCED}                                    (See above)
 *
 *      .PhySettings_cmdType                 = PhySettings_CMD_X,
 *
 *      //  TX Power as given by ti_radio_config.c (generated by SysConfig)
 *
 *      .RF_pTxPowerTable           = txPowerTable_xxx_paxx,
 *      .RF_txPowerTableSize        = TXPOWERTABLE_xxx_PAxx_SIZE,
 *  },
 */

PhySettings_RfSetting PhySettings_supportedPhys[] =
{
#if !((defined CONFIG_CC26X2R1_LAUNCHXL) || (defined CONFIG_CC2652R1FRGZ) || \
      (defined CONFIG_LP_CC2652RSIP)     || (defined CONFIG_CC2652R1FSIP) || \
      (defined CONFIG_LP_CC2652PSIP)     || (defined CONFIG_CC2652P1FSIP) || \
      (defined CONFIG_LP_CC2652R7)       || (defined CONFIG_CC2652R7RGZ)  || \
      (defined CONFIG_LP_CC2651P3)       || (defined CONFIG_CC2651P3RGZ)  || \
      (defined CONFIG_LP_CC2651R3)       || (defined CONFIG_CC2651R3RGZ)  || \
      (defined CONFIG_LP_CC2652R7)       || (defined CONFIG_CC2652R7RGZ)  || \
      (defined CONFIG_LP_CC2651R3SIPA)   || (defined CONFIG_CC2651R3SIPA) || \
      (defined CONFIG_LP_CC2653P10)      || (defined CONFIG_CC2653P10RGZ) || \
      (defined CONFIG_LP_CC2674R10)      || (defined CONFIG_CC2674R10RGZ) || \
      (defined CONFIG_CC2674R10RSK)                                       || \
      (defined CONFIG_LP_EM_CC2674P10)   || (defined CONFIG_CC2674P10RGZ) || \
      (defined CONFIG_CC2674P10RSK))
 //*********************************************************************************
 //  RF Setting:   50 kbps, 25kHz Deviation, 2-GFSK, 78 kHz RX Bandwidth
 //
 //  PHY:          2gfsk50kbps
 //  Setting file: setting_tc112.json
 //*********************************************************************************
    {
        .PhySettings_phyIndex       = PhySettings_PHY_0,
        .PhySettings_phyName        = {"PHY 0: Prop50kbps, PhySettings_TEST_STUDIO_COMPL"},
        .PhySettings_apiType        = PhySettings_API_PROP,
        .PhySettings_testType       = PhySettings_TEST_STUDIO_COMPL,
        .RF_pMode                   = &RF_prop_fsk_50kbps,
#if ((defined LAUNCHXL_CC1352P1)        || (defined LP_CC1352P7_1)            || \
     (defined LP_CC1352P7_4)            || (defined LAUNCHXL_CC1352P_4)       || \
     (defined CONFIG_LP_CC1311P3)       || (defined CONFIG_CC1311P3RGZ)       || \
     (defined LAUNCHXL_CC1352P_2)       || (defined CONFIG_LP_EM_CC1354P10_6) || \
     (defined CONFIG_LP_EM_CC1354P10_1) || (defined CONFIG_CC1354P10RSK)      || \
     (defined CONFIG_LP_EM_CC1312PSIP)  || (defined CONFIG_CC1312PSIP))
        .RF_pCmdPropRadioDivSetupPa = &RF_cmdPropRadioDivSetup_fsk_50kbps,
        .RF_pCmdPropRadioDivSetup   = NULL,
        .RF_pCmdBle5RadioSetupPa    = NULL,
        .RF_pCmdBle5RadioSetup      = NULL,
        .RF_pCmdRadioSetupPa        = NULL,
        .RF_pCmdRadioSetup          = NULL,
        .PhySettings_setupType      = PhySettings_SETUP_PROP_PA,
#else
        .RF_pCmdPropRadioDivSetupPa = NULL,
        .RF_pCmdPropRadioDivSetup   = &RF_cmdPropRadioDivSetup_fsk_50kbps,
        .RF_pCmdBle5RadioSetupPa    = NULL,
        .RF_pCmdBle5RadioSetup      = NULL,
        .RF_pCmdRadioSetupPa        = NULL,
        .RF_pCmdRadioSetup          = NULL,
        .PhySettings_setupType      = PhySettings_SETUP_PROP,
#endif
        .RF_pCmdFs                  = &RF_cmdFs_fsk_50kbps,
        .RF_pCmdPropTx              = &RF_cmdPropTx_fsk_50kbps,
        .RF_pCmdPropTxAdv           = NULL,
        .RF_pCmdPropRx              = &RF_cmdPropRx_fsk_50kbps,
        .RF_pCmdPropRxAdv           = NULL,
        .RF_pCmdBle5AdvAux          = NULL,
        .RF_pCmdBle5GenericRx       = NULL,
        .RF_pCmdIeeeTx              = NULL,
        .RF_pCmdIeeeRx              = NULL,
        .PhySettings_cmdType        = PhySettings_CMD_STANDARD,
        .RF_pTxPowerTable           = RF_PROP_txPowerTable_fsk_50kbps,
        .RF_txPowerTableSize        = RF_PROP_TX_POWER_TABLE_SIZE_fsk_50kbps
    },
#endif
#if !((defined CONFIG_CC1312R1_LAUNCHXL)  || (defined CONFIG_CC1312R1F3RGZ)      || \
       (defined CONFIG_LP_CC1312R7)       || (defined CONFIG_CC1312R7RGZ)        || \
       (defined CONFIG_LP_CC1311P3)       || (defined CONFIG_CC1311P3RGZ)        || \
       (defined CONFIG_LP_EM_CC1314R10)   || (defined CONFIG_CC1314R10RSK)       || \
       (defined CONFIG_LP_EM_CC1312PSIP)  || (defined CONFIG_CC1312PSIP))
#if !((defined CONFIG_LP_CC2653P10)      || (defined CONFIG_CC2653P10RGZ)       || \
      (defined CONFIG_LP_EM_CC1354P10_6) || (defined CONFIG_LP_EM_CC1354P10_1)  || \
      (defined CONFIG_LP_CC2674R10)      || (defined CONFIG_CC2674R10RGZ)       || \
      (defined CONFIG_CC2674R10RSK)                                             || \
      (defined CONFIG_LP_EM_CC2674P10)   || (defined CONFIG_CC2674P10RGZ)       || \
      (defined CONFIG_CC2674P10RSK)      || (defined CONFIG_CC1354P10RSK))
    //*********************************************************************************
    //  RF Setting:   250 kbps, 125 kHz Deviation, 2-GFSK, 471 kHz RX Bandwidth
    //
    //  PHY:          2gfsk250kbps
    //  Setting file: setting_tc901.json
    //*********************************************************************************
    {
#if ((defined CONFIG_CC26X2R1_LAUNCHXL) || (defined CONFIG_CC2652R1FRGZ) || \
      (defined CONFIG_LP_CC2652RSIP)    || (defined CONFIG_CC2652R1FSIP) || \
      (defined CONFIG_LP_CC2652PSIP)    || (defined CONFIG_CC2652P1FSIP) || \
      (defined CONFIG_LP_CC2652R7)      || (defined CONFIG_CC2652R7RGZ)  || \
      (defined CONFIG_LP_CC2651P3)      || (defined CONFIG_CC2651P3RGZ)  || \
      (defined CONFIG_LP_CC2651R3)      || (defined CONFIG_CC2651R3RGZ)  || \
      (defined CONFIG_LP_CC2652R7)      || (defined CONFIG_CC2652R7RGZ)  || \
      (defined CONFIG_LP_CC2651R3SIPA)  || (defined CONFIG_CC2651R3SIPA))
        .PhySettings_phyIndex       = PhySettings_PHY_0,
        .PhySettings_phyName        = {"PHY 0: Prop250kbps, PhySettings_TEST_STUDIO_COMPL"},
#else
        .PhySettings_phyIndex       = PhySettings_PHY_1,
        .PhySettings_phyName        = {"PHY 1: Prop250kbps, PhySettings_TEST_STUDIO_COMPL"},
#endif
        .PhySettings_apiType        = PhySettings_API_PROP,
        .PhySettings_testType       = PhySettings_TEST_STUDIO_COMPL,
        .RF_pMode                   = &RF_prop_2_4G_fsk_250kbps,
#if ((defined LAUNCHXL_CC1352P1)        || (defined LP_CC1352P7_1)            || \
     (defined LP_CC1352P7_4)            || (defined LAUNCHXL_CC1352P_4)       || \
     (defined LAUNCHXL_CC1352P_2)       || (defined CONFIG_LP_CC2651P3)       || \
     (defined CONFIG_CC2651P3RGZ)       || (defined CONFIG_LP_CC2652PSIP)     || \
     (defined CONFIG_CC2652P1FSIP))
        .RF_pCmdPropRadioDivSetupPa = &RF_cmdPropRadioDivSetup_2_4G_fsk_250kbps,
        .RF_pCmdPropRadioDivSetup   = NULL,
        .RF_pCmdBle5RadioSetupPa    = NULL,
        .RF_pCmdBle5RadioSetup      = NULL,
        .RF_pCmdRadioSetupPa        = NULL,
        .RF_pCmdRadioSetup          = NULL,
        .PhySettings_setupType      = PhySettings_SETUP_PROP_PA,
#else
        .RF_pCmdPropRadioDivSetupPa = NULL,
        .RF_pCmdPropRadioDivSetup   = &RF_cmdPropRadioDivSetup_2_4G_fsk_250kbps,
        .RF_pCmdBle5RadioSetupPa    = NULL,
        .RF_pCmdBle5RadioSetup      = NULL,
        .RF_pCmdRadioSetupPa        = NULL,
        .RF_pCmdRadioSetup          = NULL,
        .PhySettings_setupType      = PhySettings_SETUP_PROP,
#endif
        .RF_pCmdFs                  = &RF_cmdFs_2_4G_fsk_250kbps,
        .RF_pCmdPropTx              = &RF_cmdPropTx_2_4G_fsk_250kbps,
        .RF_pCmdPropTxAdv           = NULL,
        .RF_pCmdPropRx              = &RF_cmdPropRx_2_4G_fsk_250kbps,
        .RF_pCmdPropRxAdv           = NULL,
        .RF_pCmdBle5AdvAux          = NULL,
        .RF_pCmdBle5GenericRx       = NULL,
        .RF_pCmdIeeeTx              = NULL,
        .RF_pCmdIeeeRx              = NULL,
        .PhySettings_cmdType        = PhySettings_CMD_STANDARD,
        .RF_pTxPowerTable           = RF_PROP_txPowerTable_2_4G_fsk_250kbps,
        .RF_txPowerTableSize        = RF_PROP_TX_POWER_TABLE_SIZE_2_4G_fsk_250kbps
    },
#endif
    //*********************************************************************************
    //  RF Setting:   IEEE 802.15.4-2006, 250 kbps, OQPSK, DSSS = 1:8
    //
    //  PHY:          ieee154
    //  Setting file: setting_ieee_802_15_4.json
    //*********************************************************************************
    {
#if ((defined CONFIG_LP_CC2653P10) || (defined CONFIG_CC2653P10RGZ)             || \
      (defined CONFIG_LP_CC2674R10)      || (defined CONFIG_CC2674R10RGZ)       || \
      (defined CONFIG_CC2674R10RSK)                                             || \
      (defined CONFIG_LP_EM_CC2674P10)   || (defined CONFIG_CC2674P10RGZ)       || \
      (defined CONFIG_CC2674P10RSK))
        .PhySettings_phyIndex       = PhySettings_PHY_0,
        .PhySettings_phyName        = {"PHY 0: IEEE250kbps, PhySettings_TEST_STUDIO_COMPL"},
#elif ((defined CONFIG_CC26X2R1_LAUNCHXL) || (defined CONFIG_CC2652R1FRGZ)      || \
       (defined CONFIG_LP_CC2652RSIP)     || (defined CONFIG_CC2652R1FSIP)      || \
       (defined CONFIG_LP_CC2652PSIP)     || (defined CONFIG_CC2652P1FSIP)      || \
       (defined CONFIG_LP_CC2652R7)       || (defined CONFIG_CC2652R7RGZ)       || \
       (defined CONFIG_LP_CC2651P3)       || (defined CONFIG_CC2651P3RGZ)       || \
       (defined CONFIG_LP_CC2651R3)       || (defined CONFIG_CC2651R3RGZ)       || \
       (defined CONFIG_LP_CC2652R7)       || (defined CONFIG_CC2652R7RGZ)       || \
       (defined CONFIG_LP_CC2651R3SIPA)   || (defined CONFIG_CC2651R3SIPA)      || \
       (defined CONFIG_LP_EM_CC1354P10_6) || (defined CONFIG_LP_EM_CC1354P10_1) || \
       (defined CONFIG_CC1354P10RSK))
         .PhySettings_phyIndex       = PhySettings_PHY_1,
         .PhySettings_phyName        = {"PHY 1: IEEE250kbps, PhySettings_TEST_STUDIO_COMPL"},
#else
         .PhySettings_phyIndex       = PhySettings_PHY_2,
         .PhySettings_phyName        = {"PHY 2: IEEE250kbps, PhySettings_TEST_STUDIO_COMPL"},
#endif
        .PhySettings_apiType        = PhySettings_API_IEEE,
        .PhySettings_testType       = PhySettings_TEST_STUDIO_COMPL,
        .RF_pMode                   = &RF_prop_ieee154,
#if ((defined LAUNCHXL_CC1352P1)        || (defined LP_CC1352P7_1)            || \
     (defined LP_CC1352P7_4)            || (defined LAUNCHXL_CC1352P_4)       || \
     (defined LAUNCHXL_CC1352P_2)       || (defined CONFIG_LP_CC2651P3)       || \
     (defined CONFIG_CC2651P3RGZ)       || (defined CONFIG_LP_CC2652PSIP)     || \
     (defined CONFIG_CC2652P1FSIP)      || (defined CONFIG_LP_EM_CC1354P10_6) || \
     (defined CONFIG_LP_EM_CC1354P10_1) || (defined CONFIG_CC1354P10RSK)      || \
     (defined CONFIG_LP_CC2653P10)      || (defined CONFIG_CC2653P10RGZ)      || \
     (defined CONFIG_LP_EM_CC2674P10)   || (defined CONFIG_CC2674P10RGZ)      || \
     (defined CONFIG_CC2674P10RSK))
        .RF_pCmdPropRadioDivSetupPa = NULL,
        .RF_pCmdPropRadioDivSetup   = NULL,
        .RF_pCmdBle5RadioSetupPa    = NULL,
        .RF_pCmdBle5RadioSetup      = NULL,
        .RF_pCmdRadioSetupPa        = &RF_cmdRadioSetup_ieee154,
        .RF_pCmdRadioSetup          = NULL,
        .PhySettings_setupType      = PhySettings_SETUP_IEEE_PA,
#else
        .RF_pCmdPropRadioDivSetupPa = NULL,
        .RF_pCmdPropRadioDivSetup   = NULL,
        .RF_pCmdBle5RadioSetupPa    = NULL,
        .RF_pCmdBle5RadioSetup      = NULL,
        .RF_pCmdRadioSetupPa        = NULL,
        .RF_pCmdRadioSetup          = &RF_cmdRadioSetup_ieee154,
        .PhySettings_setupType      = PhySettings_SETUP_IEEE,
#endif
        .RF_pCmdFs                  = &RF_cmdFs_ieee154,
        .RF_pCmdPropTx              = NULL,
        .RF_pCmdPropTxAdv           = NULL,
        .RF_pCmdPropRx              = NULL,
        .RF_pCmdPropRxAdv           = NULL,
        .RF_pCmdBle5AdvAux          = NULL,
        .RF_pCmdBle5GenericRx       = NULL,
        .RF_pCmdIeeeTx              = &RF_cmdIeeeTx_ieee154,
        .RF_pCmdIeeeRx              = &RF_cmdIeeeRx_ieee154,
        .PhySettings_cmdType        = PhySettings_CMD_STANDARD,
        .RF_pTxPowerTable           = txPowerTable_ieee154,
        .RF_txPowerTableSize        = TX_POWER_TABLE_SIZE_ieee154
    }
#endif
};

const uint8_t PhySettings_numSupportedPhys = sizeof(PhySettings_supportedPhys)/sizeof(PhySettings_RfSetting);

//-------------------------------------------------------------------------------------
// Proprietary Mode Transmit Command used when testing BLE PHYs
// These commands are not available when selecting API commands for BLE PHYs and
// are therefore added manually
//-------------------------------------------------------------------------------------
rfc_CMD_PROP_TX_t RF_cmdPropTxBle =
{
    .commandNo = 0x3801,
    .status = 0x0000,
    .pNextOp = 0, // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .pktConf.bFsOff = 0x0,
    .pktConf.bUseCrc = 0x1,
    .pktConf.bVarLen = 0x1,
    .pktLen = 0x14,
    .syncWord = 0x930B51DE,
    .pPkt = 0 // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
};

rfc_CMD_PROP_RX_t RF_cmdPropRxBle =
{
    .commandNo = 0x3802,
    .status = 0x0000,
    .pNextOp = 0, // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
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
    .pktConf.bVarLen = 0x1,
    .pktConf.bChkAddress = 0x0,
    .pktConf.endType = 0x0,
    .pktConf.filterOp = 0x0,
    .rxConf.bAutoFlushIgnored = 0x0,
    .rxConf.bAutoFlushCrcErr = 0x0,
    .rxConf.bIncludeHdr = 0x1,
    .rxConf.bIncludeCrc = 0x0,
    .rxConf.bAppendRssi = 0x0,
    .rxConf.bAppendTimestamp = 0x0,
    .rxConf.bAppendStatus = 0x1,
    .syncWord = 0x930B51DE,
    .maxPktLen = 0xFF,
    .address0 = 0xAA,
    .address1 = 0xBB,
    .endTrigger.triggerType = 0x1,
    .endTrigger.bEnaCmd = 0x0,
    .endTrigger.triggerNo = 0x0,
    .endTrigger.pastTrig = 0x0,
    .endTime = 0x00000000,
    .pQueue = 0, // INSERT APPLICABLE POINTER: (dataQueue_t*)&xxx
    .pOutput = 0 // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
};
