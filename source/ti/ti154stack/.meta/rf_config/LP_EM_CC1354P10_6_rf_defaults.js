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
 *  ======== LP_EM_CC1354P10_6_rf_defaults.js ========
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
 *              - cmdPropRadioDivSetupPa: Name of the generated
 *                      rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t command
 *              - paExport: Determines how PA table is exported
 */

// Object containing SimpleLink Long Range, 5kbps settings for LP_EM_CC1354P10_6
const devSpecificSlLr5KbpsSettings = {
    args: {
        freqBand: "868", // options: 868 or 433
        phyType868: "slr5kbps2gfsk", // phyType suffix must match freqBand
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetupPa"],
            cmdPropRadioDivSetupPa: "RF_cmdPropRadioDivSetup_slr5kbps2gfsk",
            paExport: "combined"
        }
    },
    unsupported: {
        freqSub1: ["freq920"]
    }
};

// Object containing 2GFSK, 50kbps settings for the LP_EM_CC1354P10_6
const devSpecific2Gfsk50KbpsSettings = {
    args: {
        freqBand: "868", // options: 868 or 433
        phyType868: "2gfsk50kbps154g", // phyType suffix must match freqBand
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetupPa"],
            cmdPropRadioDivSetupPa: "RF_cmdPropRadioDivSetup_2gfsk50kbps154g",
            paExport: "combined"
        }
    },
    unsupported: {
        freqSub1: ["freq920"]
    }
};

// Object containing 2GFSK, 200kbps settings for the LP_EM_CC1354P10_6
const devSpecific2Gfsk200KbpsSettings = {
    args: {
        freqBand: "868",
        phyType868: "2gfsk200kbps154g", // phyType sfx must match freqBand
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetupPa"],
            cmdPropRadioDivSetupPa: "RF_cmdPropRadioDivSetup_2gfsk200kbps154g",
            paExport: "combined"
        }
    },
    unsupported: {
        freqSub1: ["freq920"]
    }
};

// Object containing Wi-SUN #4a 200kbps settings for the LP_EM_CC1354P10_6
const devSpecific2Gfsk200KbpsWisun4aSettings = {
    radioConfigModName: "radioConfig200Kbps2GfskWsun4a",
    args: {
        freqBand: "868",
        phyType868: "2gfsk200kbps50dev915wsun4a",
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetupPa"],
            cmdPropRadioDivSetupPa:
                "RF_cmdPropRadioDivSetup_2gfsk200kbpsWisun4a",
            paExport: "combined",
            rfMode: "RF_prop_2gfsk200kbpsWisun4a",
            txPower: "txPowerTable_2gfsk200kbpsWisun4a",
            txPowerSize: "TX_POWER_TABLE_SIZE_2gfsk200kbpsWisun4a",
            overrides: "pOverrides_2gfsk200kbpsWisun4a",
            cmdFs: "RF_cmdFs_2gfsk200kbpsWisun4a",
            cmdPropRxAdv: "RF_cmdPropRxAdv_2gfsk200kbpsWisun4a",
            cmdPropTxAdv: "RF_cmdPropTxAdv_2gfsk200kbpsWisun4a",
            cmdPropCs: "RF_cmdPropCs_2gfsk200kbpsWisun4a"
        }
    },
    unsupported: {
        freqSub1: ["freq433", "freq863", "freq915"]
    }
};

// Object containing IEEE settings for the LP_EM_CC1354P10_6
const devSpecificIEEESettings = {
    args: {
        txPowerHi:"10",
        codeExportConfig: {
            cmdList_ieee_15_4: ["cmdRadioSetupPa"],
            cmdRadioSetupPa: "RF_cmdRadioSetup_ieee154",
            paExport: "combined"
        }
    },
};

/*
 *  ======== Arrays Containing all PHY Settings ========
 *
 * These arrays will pull the common phy settings defined in
 * ti154stack_rf_config_common.js and merge them with the device specific phy
 * settings defined in this file.
 */

// Array containing all the phy settings for the LP_EM_CC1354P10_6
const defaultPropPhyList = [
    rfCommon.mergeRFSettings(devSpecificSlLr5KbpsSettings,
        rfCommon.commonSlLr5KbpsSettings),
    rfCommon.mergeRFSettings(devSpecific2Gfsk50KbpsSettings,
        rfCommon.common2Gfsk50KbpsSettings),
    rfCommon.mergeRFSettings(devSpecific2Gfsk200KbpsSettings,
        rfCommon.common2Gfsk200KbpsSettings),
    // Order of merge matters to overwrite common settings
    rfCommon.mergeRFSettings(rfCommon.common2Gfsk200KbpsSettings,
        devSpecific2Gfsk200KbpsWisun4aSettings)
];

const defaultIEEEPhyList = [
    rfCommon.mergeRFSettings(devSpecificIEEESettings,
        rfCommon.commonIEEESettings)
];

// Contains the object over the configurable phy 
const customPhyList = [
    rfCommon.mergeRFSettings(customObj.devSpecificCustomPhySettingsPdevices,
    customObj.commonCustomPhySettings)
];

exports = {
    defaultPropPhyList: defaultPropPhyList,
    defaultIEEEPhyList: defaultIEEEPhyList,
    customPhyList: customPhyList
};
