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
 *  ======== LP_EM_CC2674P10_rf_defaults.js ========
 */

"use strict";

// Get common rf settings
const rfCommon = system.getScript("/ti/prop_rf/rf_config/"
    + "prop_rf_config_common.js");

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
 *              - cmdBle5RadioSetup: Name of the generated
 *                      rfc_CMD_PROP_RADIO_DIV_SETUP_t command
 */

// Object containing BLE settings for the LP_EM_CC2674P10
const devSpecificBleSettings = {
    args: {
        codeExportConfig: {
            cmdList_ble: ["cmdBle5RadioSetup"],
            cmdBle5RadioSetup: "RF_ble_cmdRadioSetup"
        }
    }
};

// IEEE settings for the LP_CC2653P10
const devSpecificIEEESettings = {
    args: {
        highPA: true,
        txPowerHi: "20",
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
 * These Arrays will pull the common phy settings defined in
 * prop_rf_config_common.js and merge them with the device specific phy
 * settings defined in this file.
 *
 * Note: The first element in the defaultPropPhyList with easyLinkOption defined
 *       will be the phy that EasyLink_Phy_Custom defaults to when added to a
 *       configuration
 */

// Array containing all the proprietary phy settings for the LP_EM_CC2674P10
const defaultPropPhyList = [];

// Array containing all the BLE phy settings for the LP_EM_CC2674P10
const defaultBlePhyList = [
    rfCommon.mergeRfSettings(devSpecificBleSettings, rfCommon.commonBleSettings)
];

// Array containing all the IEEE phy settings for the LP_EM_CC2674P10
const default154PhyList = [
    rfCommon.mergeRfSettings(devSpecificIEEESettings, rfCommon.commonIEEESettings)
];

exports = {
    defaultPropPhyList: defaultPropPhyList,
    defaultBlePhyList: defaultBlePhyList,
    default154PhyList: default154PhyList
};
