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
 *  ======== LP_CC1311P3_rf_defaults.js ========
 */

"use strict";

// Get common rf settings
const rfCommon = system.getScript("/ti/prop_rf/rf_config/"
    + "prop_rf_config_common.js");

/*
 *  ======== Device Specific Proprietary PHY Settings ========
 *
 * These Objects will be used to extend the common proprietary phy settings
 * defined in prop_rf_config_common.js.
 *
 * These Objects must contain the following elements:
 *      - args: Phy setting arguments to be passed to the radio config module
 *          - freqBand: The frequency band containing the carrierFrequency.
 *                      Valid options: 433, 868, 2440(reserved for future use)
 *          - phyType<band>: Name of the phy found in the radio config module.
 *                           Valid options: phyType868, phyType433,
 *                           phyType2440(reserved for future use)
 *          - carrierFrequency: Frequency setting within the freqBand
 *          - txPower: Transmit power setting, must match a value in radio
 *                     config's txPower drop-down
 *          - codeExportConfig: Exported code names to be passed to
 *                              radioConfig.codeExportConfig
 *              - cmdPropRadioDivSetup: Name of the generated
 *                      rfc_CMD_PROP_RADIO_DIV_SETUP_t command
 */

// Object containing 2GFSK, 50kbps settings for the CC1312R1_LAUNCHXL
const devSpecific2Gfsk50KbpsSettings = {
    args: {
        freqBand: "868", // options: 868 or 433
        phyType868: "2gfsk50kbps", // phyType suffix must match freqBand
        carrierFrequency: 868.00,
        highPA: true,
        txPowerHi: "20",
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetupPa"],
            cmdPropRadioDivSetupPa: "RF_cmdPropRadioDivSetup_fsk_50kbps",
            paExport: "combined"
        }
    }
};

// Object containing SimpleLink Long Range, 5kbps settings for CC1312R1_LAUNCHXL
const devSpecificSlLr5KbpsSettings = {
    args: {
        freqBand: "868", // options: 868 or 433
        phyType868: "slr5kbps2gfsk", // phyType suffix must match freqBand
        carrierFrequency: 868.00,
        highPA: true,
        txPowerHi: "20",
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetupPa"],
            cmdPropRadioDivSetupPa: "RF_cmdPropRadioDivSetup_sl_lr",
            paExport: "combined"
        }
    }
};

// Object containing 2GFSK, 200kbps settings for the CC1312R1_LAUNCHXL
const devSpecific2Gfsk200KbpsSettings = {
    args: {
        freqBand: "868",
        phyType868: "2gfsk200kbps154g", // phyType sfx must match freqBand
        carrierFrequency: 868.00,
        highPA: true,
        txPowerHi: "20",
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetupPa"],
            cmdPropRadioDivSetupPa: "RF_cmdPropRadioDivSetup_fsk_200kbps",
            paExport: "combined"
        }
    }
};

/*
 *  ======== Arrays Containing all PHY Settings ========
 *
 * These Arrays will pull the common phy settings defined in
 * prop_rf_config_common.js and merge them with the device specific phy
 * settings defined in this file.
 *
 * Note: The first element in the defaultPropPhyList with easyLinkOption defined
 *       will be the phy that EasyLink_Phy_Custom defaults to when added to a
 *       configuration
 */

// Array containing all the proprietary phy settings for the CC1312R1_LAUNCHXL
const defaultPropPhyList = [
    rfCommon.mergeRfSettings(devSpecific2Gfsk50KbpsSettings,
        rfCommon.common2Gfsk50KbpsSettings),
    rfCommon.mergeRfSettings(devSpecificSlLr5KbpsSettings,
        rfCommon.commonSlLr5KbpsSettings),
    rfCommon.mergeRfSettings(devSpecific2Gfsk200KbpsSettings,
        rfCommon.common2Gfsk200KbpsSettings)
];

// CC1312R1 does not support 2.4GHz band
const defaultBlePhyList = [];

// Reserved for future use
const default154PhyList = [];

exports = {
    defaultPropPhyList: defaultPropPhyList,
    defaultBlePhyList: defaultBlePhyList,
    default154PhyList: default154PhyList
};
