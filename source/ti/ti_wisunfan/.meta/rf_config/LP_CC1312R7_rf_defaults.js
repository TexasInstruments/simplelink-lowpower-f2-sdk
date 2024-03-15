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
 *  ======== CC1312R1_LAUNCHXL_rf_defaults.js ========
 */

"use strict";

// Get common rf settings
const rfCommon = system.getScript("/ti/ti_wisunfan/rf_config/"
    + "ti_wisunfan_rf_config_common.js");

/*
 *  ======== Device Specific PHY Settings ========
 *
 * These Objects will be used to extend the common phy settings defined
 * in ti_wisunfan_rf_config_common.js.
 *
 * These Objects must contain the following elements:
 *      - args: Phy setting arguments to be passed to the radio config module
 *          - freqBand: The frequency band containing the carrierFrequency.
 *                      Valid options: 433, 868, 2440(reserved for future use)
 *          - phyType<band>: Name of the phy found in the radio config module.
 *                           Valid options: phyType868, phyType433,
 *                           phyType2440(reserved for future use)
 *          - whitening: Types of whitening
 *          - codeExportConfig: Exported code names to be passed to
 *                              radioConfig.codeExportConfig
 *              - cmdPropRadioDivSetup: Name of the generated
 *                      rfc_CMD_PROP_RADIO_DIV_SETUP_t command
 */

// Object containing SimpleLink Long Range, 5kbps settings for CC1312R1_LAUNCHXL
const devSpecificSlLr5KbpsSettings = {
    args: {
        freqBand: "868", // options: 868 or 433
        phyType868: "slr5kbps2gfsk", // phyType suffix must match freqBand
        whitening: `Dynamically IEEE 802.15.4g compatible whitener and \
16/32-bit CRC`,
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetup"],
            cmdPropRadioDivSetup: "RF_cmdPropRadioDivSetup_slr5kbps2gfsk"
        }
    }
};

// Object containing 2GFSK, 50kbps settings for the CC1312R1_LAUNCHXL
const devSpecific2Gfsk50KbpsSettings_25dev = {
    args: {
        freqBand: "868", // options: 868 or 433
        phyType868: "2gfsk50kbps25dev915wsun1b", // phyType suffix must match freqBand
        whitening: `Dynamically IEEE 802.15.4g compatible whitener and \
16/32-bit CRC`,
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetup"],
            cmdPropRadioDivSetup: "RF_cmdPropRadioDivSetup_2gfsk50kbps25dev915wsun1b"
        }
    }
};

// Object containing 2GFSK, 50kbps settings for 866MHz WiSUN band
const devSpecific2Gfsk50KbpsWiSUNSettings_12_5dev = {
    args: {
        freqBand: "868", // options: 868 or 433
        phyType868: "2gfsk50kbps12dev868wsun1a", // phyType suffix must match freqBand
        whitening: `Dynamically IEEE 802.15.4g compatible whitener and \
16/32-bit CRC`,
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetup"],
            cmdPropRadioDivSetup: "RF_cmdPropRadioDivSetup_2gfsk50kbps12dev868wsun1a"
        }
    },
    phy154Settings: {
        freq866: {
            ID: "APIMAC_50KBPS_866MHZ_PHY_4",
            channelPage: "APIMAC_CHANNEL_PAGE_9"
        }
    }
};

// Object containing 2GFSK, 100kbps settings for 866MHz WiSUN band
const devSpecific2Gfsk100KbpsWiSUNSettings_25dev = {
    args: {
        freqBand: "868", // options: 868 or 433
        phyType868: "2gfsk100kbps25dev915wsun2a",
        whitening: `Dynamically IEEE 802.15.4g compatible whitener and \
16/32-bit CRC`,
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetup"],
            cmdPropRadioDivSetup: "RF_cmdPropRadioDivSetup_2gfsk100kbps25dev915wsun2a"
        }
    }
};

// Object containing 2GFSK, 100kbps settings for 866MHz WiSUN band
const devSpecific2Gfsk100KbpsWiSUNSettings_50dev = {
    args: {
        freqBand: "868", // options: 868 or 433
        phyType868: "2gfsk100kbps50dev915wsun2b",
        whitening: `Dynamically IEEE 802.15.4g compatible whitener and \
16/32-bit CRC`,
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetup"],
            cmdPropRadioDivSetup: "RF_cmdPropRadioDivSetup_2gfsk100kbps50dev915wsun2b"
        }
    }
};

// Object containing 2GFSK, 100kbps settings for 866MHz WiSUN band
const devSpecific2Gfsk150KbpsWiSUNSettings_75dev = {
    args: {
        freqBand: "868", // options: 868 or 433
        phyType868: "2gfsk150kbps75dev868wsun3",
        whitening: `Dynamically IEEE 802.15.4g compatible whitener and \
16/32-bit CRC`,
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetup"],
            cmdPropRadioDivSetup: "RF_cmdPropRadioDivSetup_2gfsk150kbps75dev868wsun3"
        }
    }
};

// Object containing 2GFSK, 200kbps settings for the CC1312R1_LAUNCHXL
const devSpecific2Gfsk200KbpsSettings_50dev = {
    args: {
        freqBand: "868",
        phyType868: "2gfsk200kbps50dev915wsun4a", // phyType sfx must match freqBand
        whitening: `Dynamically IEEE 802.15.4g compatible whitener and \
16/32-bit CRC`,
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetup"],
            cmdPropRadioDivSetup: "RF_cmdPropRadioDivSetup_2gfsk200kbps50dev915wsun4a"
        }
    }
};

// Object containing 2GFSK, 200kbps settings for 866MHz WiSUN band
const devSpecific2Gfsk200KbpsWiSUNSettings_100dev = {
    args: {
        freqBand: "868", // options: 868 or 433
        phyType868: "2gfsk200kbps100dev915wsun4b",
        whitening: `Dynamically IEEE 802.15.4g compatible whitener and \
16/32-bit CRC`,
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetup"],
            cmdPropRadioDivSetup: "RF_cmdPropRadioDivSetup_2gfsk200kbps100dev915wsun4b"
        }
    }
};

// Object containing 2GFSK, 300kbps settings for 866MHz WiSUN band
const devSpecific2Gfsk300KbpsWiSUNSettings_75dev = {
    args: {
        freqBand: "868", // options: 868 or 433
        phyType868: "2gfsk300kbps75dev915wsun5",
        whitening: `Dynamically IEEE 802.15.4g compatible whitener and \
16/32-bit CRC`,
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetup"],
            cmdPropRadioDivSetup: "RF_cmdPropRadioDivSetup_2gfsk300kbps75dev915wsun5"
        }
    }
};
/*
 *  ======== Arrays Containing all PHY Settings ========
 *
 * These arrays will pull the common phy settings defined in
 * ti_wisunfan_rf_config_common.js and merge them with the device specific phy
 * settings defined in this file.
 */

// Array containing all the phy settings for the CC1312R1_LAUNCHXL
const defaultPropPhyList = [
    rfCommon.mergeRFSettings(devSpecificSlLr5KbpsSettings,
        rfCommon.commonSlLr5KbpsSettings),
    rfCommon.mergeRFSettings(devSpecific2Gfsk50KbpsSettings_25dev,
        rfCommon.common2Gfsk50KbpsSettings),
    // Overwrite common phy154Settings for WiSUN band
    rfCommon.mergeRFSettings(devSpecific2Gfsk50KbpsWiSUNSettings_12_5dev,
        _.omit(rfCommon.common2Gfsk50KbpsSettings, ["phy154Settings"])),
    rfCommon.mergeRFSettings(devSpecific2Gfsk100KbpsWiSUNSettings_25dev,
        rfCommon.common2Gfsk100KbpsSettings),
    rfCommon.mergeRFSettings(devSpecific2Gfsk100KbpsWiSUNSettings_50dev,
        rfCommon.common2Gfsk100KbpsSettings),
    rfCommon.mergeRFSettings(devSpecific2Gfsk200KbpsSettings_50dev,
        rfCommon.common2Gfsk200KbpsSettings),
    rfCommon.mergeRFSettings(devSpecific2Gfsk200KbpsWiSUNSettings_100dev,
        rfCommon.common2Gfsk200KbpsSettings),
    rfCommon.mergeRFSettings(devSpecific2Gfsk300KbpsWiSUNSettings_75dev, rfCommon.common2Gfsk300KbpsSettings)
];

// IEEE phy setting not supported on the CC1312R1_LAUNCHXL
const defaultIEEEPhyList = [
];

const default5kbpsList = [
    rfCommon.mergeRFSettings(devSpecificSlLr5KbpsSettings,
        rfCommon.commonSlLr5KbpsSettings)
];

const default50kbpsList_12_5_dev = [
    rfCommon.mergeRFSettings(devSpecific2Gfsk50KbpsWiSUNSettings_12_5dev,
        _.omit(rfCommon.common2Gfsk50KbpsSettings, ["phy154Settings"])),
        ]
const default50kbpsList_25_dev = [
    rfCommon.mergeRFSettings(devSpecific2Gfsk50KbpsSettings_25dev,
        _.omit(rfCommon.common2Gfsk50KbpsSettings, ["phy154Settings"])),
        ]

const default100kbpsList_25dev = [
    rfCommon.mergeRFSettings(devSpecific2Gfsk100KbpsWiSUNSettings_25dev,
        rfCommon.common2Gfsk100KbpsSettings),
        ]

const default100kbpsList_50dev = [
    rfCommon.mergeRFSettings(devSpecific2Gfsk100KbpsWiSUNSettings_50dev,
        rfCommon.common2Gfsk100KbpsSettings),
        ]

const default150kbpsList_75dev = [
    rfCommon.mergeRFSettings(devSpecific2Gfsk150KbpsWiSUNSettings_75dev,
        rfCommon.common2Gfsk150KbpsSettings),
        ]

const default200kbpsList_50dev = [
    rfCommon.mergeRFSettings(devSpecific2Gfsk200KbpsSettings_50dev,
            rfCommon.common2Gfsk200KbpsSettings)
]
const default200kbpsList_100dev = [
    rfCommon.mergeRFSettings(devSpecific2Gfsk200KbpsWiSUNSettings_100dev,
            rfCommon.common2Gfsk200KbpsSettings)
]
const default300kbpsList_75dev = [
    rfCommon.mergeRFSettings(devSpecific2Gfsk300KbpsWiSUNSettings_75dev,
            rfCommon.common2Gfsk300KbpsSettings)
]



exports = {
    defaultPropPhyList: defaultPropPhyList,
    defaultIEEEPhyList: defaultIEEEPhyList,
    default5kbpsList: default5kbpsList,
    default50kbpsList_12_5_dev: default50kbpsList_12_5_dev,
    default50kbpsList_25_dev: default50kbpsList_25_dev,
    default100kbpsList_25dev: default100kbpsList_25dev,
    default100kbpsList_50dev: default100kbpsList_50dev,
    default150kbpsList_75dev: default150kbpsList_75dev,
    default200kbpsList_50dev: default200kbpsList_50dev,
    default200kbpsList_100dev: default200kbpsList_100dev,
    default300kbpsList_75dev: default300kbpsList_75dev
};
