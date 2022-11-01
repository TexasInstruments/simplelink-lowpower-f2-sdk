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
 *  ======== LP_CC2652PSIP_rf_defaults.js ========
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
 *                      Valid options: 433, 868, 2400
 *          - phyType<band>: Name of the phy found in the radio config module.
 *                           Valid options: phyType868, phyType433,
 *                           phyType2400
 *          - carrierFrequency: Frequency setting within the freqBand
 *          - txPower: Transmit power setting, must match a value in radio
 *                     config's txPower drop-down
 *          - codeExportConfig: Exported code names to be passed to
 *                              radioConfig.codeExportConfig
 *              - cmdPropRadioDivSetup: Name of the generated
 *                      rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t command
 */

// Object containing 2GFSK, 250kbps settings for the LP_CC2652PSIP
// Left empty because there are no overrides from the common settings
const devSpecific2Gfsk250Kbps24GHzSettings = {
    args: {
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetupPa"],
            cmdPropRadioDivSetupPa: "RF_cmdPropRadioDivSetup_2_4G_fsk_250kbps",
            paExport: "combined"
        }
    }
};

// Object containing 2GFSK, 100kbps settings for the LP_CC2652PSIP
// Left empty because there are no overrides from the common settings
const devSpecific2Gfsk100Kbps24GHzSettings = {
    args: {
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetupPa"],
            cmdPropRadioDivSetupPa: "RF_cmdPropRadioDivSetup_2_4G_fsk_100kbps",
            paExport: "combined"
        }
    }
};

/*
 *  ======== Device Specific BLE PHY Settings ========
 *
 * These Objects will be used to extend the common BLE phy settings defined in
 * prop_rf_config_common.js.
 *
 * These Objects must contain the following elements:
 *      - args: Phy setting arguments to be passed to the radio config module
 *          - codeExportConfig: Exported code names to be passed to
 *                              radioConfig.codeExportConfig
 *          - cmdBle5RadioSetupPa: Name of the generated
 *                                 rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t command
 */
// Object containing BLE settings for the LP_CC2652PSIP
const devSpecificBleSettings = {
    args: {
        highPA: true,
        txPowerHi: "10",
        phyType: "bt5le1mp10",
        codeExportConfig: {
            cmdList_ble: ["cmdBle5RadioSetupPa"],
            cmdBle5RadioSetupPa: "RF_ble_cmdRadioSetup",
            paExport: "combined"
        }
    }
};

// IEEE settings for the LP_CC2652PSIP
const devSpecificIEEESettings = {
    args: {
        txPower: "5",
        phyType: "ieee154p10",
        codeExportConfig: {
            cmdList_ieee_15_4: ["cmdRadioSetupPa"],
            cmdRadioSetupPa: "RF_cmdRadioSetup_ieee154",
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

// Array containing all the proprietary phy settings for the LP_CC2652PSIP
const defaultPropPhyList = [
    rfCommon.mergeRfSettings(devSpecific2Gfsk100Kbps24GHzSettings,
        rfCommon.common2Gfsk100Kbps24GHzSettings),
    rfCommon.mergeRfSettings(devSpecific2Gfsk250Kbps24GHzSettings,
        rfCommon.common2Gfsk250Kbps24GHzSettings)
];

// Array containing all the BLE phy settings for the LP_CC2652PSIP
const defaultBlePhyList = [
    rfCommon.mergeRfSettings(rfCommon.commonBleSettings, devSpecificBleSettings)
];

// Array containing all the IEEE phy settings for the LP_CC2652PSIP
const default154PhyList = [
    rfCommon.mergeRfSettings(rfCommon.commonIEEESettings, devSpecificIEEESettings)
];

exports = {
    defaultPropPhyList: defaultPropPhyList,
    defaultBlePhyList: defaultBlePhyList,
    default154PhyList: default154PhyList
};
