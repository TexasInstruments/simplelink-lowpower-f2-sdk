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
 *  ======== CC2652RB_LAUNCHXL_rf_defaults.js ========
 */

"use strict";

// Get common rf settings
const rfCommon = system.getScript("/ti/ti_wisunfan/rf_config/"
    + "ti_wisunfan_rf_config_common.js");

/*
 *  ======== Device Specific IEEE PHY Settings ========
 *
 * These Objects will be used to extend the common IEEE phy settings defined in
 * ti_wisunfan_rf_config_common.js.
 *
 * These Objects must contain the following elements:
 *      - args: Phy setting arguments to be passed to the radio config module
 *          - codeExportConfig: Exported code names to be passed to
 *                              radioConfig.codeExportConfig
 *              - cmdRadioSetup: Name of the generated rfc_CMD_RADIO_SETUP_t
 *                               command
 */

// Object containing IEEE settings for the CC2652RB_LAUNCHXL
const devSpecificIEEESettings = {
    args: {
        codeExportConfig: {
            cmdList_ieee_15_4: ["cmdRadioSetup"],
            cmdRadioSetup: "RF_cmdRadioSetup_ieee154"
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

// Sub-1 GHz phy setting not supported on the CC2652RB_LAUNCHXL
const defaultPropPhyList = [
];

// Array containing all the IEEE phy settings for the CC2652RB_LAUNCHXL
const defaultIEEEPhyList = [
    rfCommon.mergeRFSettings(devSpecificIEEESettings,
        rfCommon.commonIEEESettings)
];

exports = {
    defaultPropPhyList: defaultPropPhyList,
    defaultIEEEPhyList: defaultIEEEPhyList
};
