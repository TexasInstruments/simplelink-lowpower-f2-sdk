/*
 * Copyright (c) 2019 Texas Instruments Incorporated - http://www.ti.com
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
 *
 */

/*
 *  ======== CC1352P_4_LAUNCHXL_rf_defaults.js ========
 */

"use strict";

// Get common rf settings
const rfCommon = system.getScript("/ti/ti154stack/rf_config/"
    + "ti154stack_rf_config_common.js");

const customObj = system.getScript("/ti/ti154stack/rf_config/"
    + "phyCustom.js");

/*
 *  ======== Device Specific Proprietary PHY Settings ========
 *
 * These Objects will be used to extend the common proprietary phy settings
 * defined in ti154stack_rf_config_common.js.
 *
 * These Objects must contain the following elements:
 *      - args: Phy setting arguments to be passed to the radio config module
 *          - freqBand: The frequency band containing the carrierFrequency.
 *                      Valid options: 433, 868, 2440(reserved for future use)
 *          - phyType<band>: Name of the phy found in the radio config module.
 *                           Valid options: phyType868, phyType433,
 *                           phyType2440(reserved for future use)
 *          - carrierFrequency: Frequency setting within the freqBand
 *          - whitening: Type of whitening
 *          - preambleCount: Number of bytes in preamble
 *          - packetLengthConfig: Whether packets are variable or fixed length
 *          - syncWordLength: Number of bits in sync word
 *          - syncWord: Complete sync word
 *          - codeExportConfig: Exported code names to be passed to
 *                              radioConfig.codeExportConfig
 *              - rfMode (optional): Name of the generated RF_Mode object
 *              - txPower (optional): Name of the generated Tx power table
 *              - txPowerSize (optional): Name of generated Tx power table size
 *              - overrides (optional): Name of the generated overrides table
 *              - cmdPropRadioDivSetupPa: Name of the generated
 *                      rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t command
 *              - paExport: Determines how PA table is exported
 *              - cmdFs (optional): Name of the generated rfc_CMD_FS_t command
 *              - cmdPropCs (optional): Name of the generated rfc_CMD_PROP_CS_t
 *              - cmdPropRxAdv (optional): Name of the generated
 *                                         rfc_CMD_PROP_RX_ADV_t command
 *              - cmdPropTxAdv (optional): Name of the generated
 *                                         rfc_CMD_PROP_TX_ADV_t command
 */

// Object containing SimpleLink Long Range, 5kbps setting for CC1352P_4_LAUNCHXL
const devSpecificSlLr5KbpsSettings = {
    args: {
        freqBand: "433",
        phyType433: "slr5kbps2gfsk433mhz",
        txPower433: "0",
        codeExportConfig: {
            rfMode: "RF_prop_slr5kbps2gfsk433mhz",
            txPower: "txPowerTable_slr5kbps2gfsk433mhz",
            txPowerSize: "TX_POWER_TABLE_SIZE_slr5kbps2gfsk433mhz",
            overrides: "pOverrides_slr5kbps2gfsk433mhz",
            cmdList_prop: ["cmdPropRadioDivSetupPa"],
            cmdPropRadioDivSetupPa:
                "RF_cmdPropRadioDivSetup_slr5kbps2gfsk433mhz",
            paExport: "combined",
            cmdFs: "RF_cmdFs_slr5kbps2gfsk433mhz",
            cmdPropTxAdv: "RF_cmdPropTxAdv_slr5kbps2gfsk433mhz",
            cmdPropRxAdv: "RF_cmdPropRxAdv_slr5kbps2gfsk433mhz",
            cmdPropCs: "RF_cmdPropCs_slr5kbps2gfsk433mhz"
        }
    },
    unsupported: {
        freqSub1: ["freq920"]
    }
};

// Object containing 2GFSK, 50kbps settings for the CC1352P_4_LAUNCHXL
const devSpecific2Gfsk50KbpsSettings = {
    args: {
        freqBand: "433",
        phyType433: "2gfsk50kbps154g433mhz",
        txPower433: "0",
        codeExportConfig: {
            rfMode: "RF_prop_2gfsk50kbps154g433mhz",
            txPower: "txPowerTable_2gfsk50kbps154g433mhz",
            txPowerSize: "TX_POWER_TABLE_SIZE_2gfsk50kbps154g433mhz",
            overrides: "pOverrides_2gfsk50kbps154g433mhz",
            cmdList_prop: ["cmdPropRadioDivSetupPa"],
            cmdPropRadioDivSetupPa:
                "RF_cmdPropRadioDivSetup_2gfsk50kbps154g433mhz",
            paExport: "combined",
            cmdFs: "RF_cmdFs_2gfsk50kbps154g433mhz",
            cmdPropTxAdv: "RF_cmdPropTxAdv_2gfsk50kbps154g433mhz",
            cmdPropRxAdv: "RF_cmdPropRxAdv_2gfsk50kbps154g433mhz",
            cmdPropCs: "RF_cmdPropCs_2gfsk50kbps154g433mhz"
        }
    },
    unsupported: {
        freqSub1: ["freq920"]
    }
};

// Object containing IEEE settings for the CC1352P_4_LAUNCHXL
const devSpecificIEEESettings = {
    args: {
        phyType: "ieee154p10",
        highPA: true,
        txPowerHi: "10",
        codeExportConfig: {
            cmdList_ieee_15_4: ["cmdRadioSetupPa"],
            cmdRadioSetupPa: "RF_cmdRadioSetup_ieee154",
            paExport: "combined"
        }
    }
};

/*
 *  ======== Arrays Containing all PHY Settings ========
 *
 * These arrays will pull the common phy settings defined in
 * ti154stack_rf_config_common.js and merge them with the device specific phy
 * settings defined in this file.
 */

// Array containing all the phy settings for the CC1352P_4_LAUNCHXL
// Order of merge matters for P4 settings to overwrite common settings
const defaultPropPhyList = [
    rfCommon.mergeRFSettings(rfCommon.commonSlLr5KbpsSettings,
        devSpecificSlLr5KbpsSettings),
    rfCommon.mergeRFSettings(rfCommon.common2Gfsk50KbpsSettings,
        devSpecific2Gfsk50KbpsSettings)
];

// Array containing all the IEEE phy settings for the CC1352P_4_LAUNCHXL
// Order of merge matters for P4 settings to overwrite common settings
const defaultIEEEPhyList = [
    rfCommon.mergeRFSettings(rfCommon.commonIEEESettings,
        devSpecificIEEESettings)
];

// Contains the object over the configurable phy 
const customPhyList = [
    rfCommon.mergeRFSettings(customObj.devSpecificCustomPhySettings433Pdevices,
    customObj.commonCustomPhySettings)
];

exports = {
    defaultPropPhyList: defaultPropPhyList,
    defaultIEEEPhyList: defaultIEEEPhyList,
    customPhyList: customPhyList
};
