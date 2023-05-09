/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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

/*
 * =========================== ti_sidewalk_application.syscfg.js ===========================
 *
 * Submodule definition for the SysConfig TI Sidewalk Application submodule
 */

"use strict";

/*!
 * ======== applicationModule ========
 *
 * Syscfg submodule for TI-Sidewalk Application
 */
const applicationModule = {
    displayName: "Application",
    description: "Configure Application Layer Settings",
    collapsed: false,
    config: [
        {
            name: "linkType",
            displayName: "Link Type",
            options: getPhySelectionOptions(),
            default: getPhySelectionOptions()[0].name,
            description: "Select which link is used at the application level",
            longDescription: "Select which link is used at the application level",
        },
    ]
};

/*
 * ======== getPhySelectionOptions ========
 * Generates an array of SRFStudio compatible rfDesign options based on device
 *
 * @param deviceId - device being used
 * @returns Array - Array of rfDesign options, if the device isn't supported,
 *                  returns null
 */
function getPhySelectionOptions()
{
    const deviceId = system.deviceData.deviceId;
    let newPhySelectionOptions = null;
    if(deviceId === "CC1352P7RGZ")
    {
        newPhySelectionOptions = [
            {name: "ble", displayName: "BLE"},
            {name: "fsk", displayName: "FSK 900MHz"},
            {name: "ble_fsk", displayName: "BLE Registration + FSK 900MHz Data"}
        ];
    }
    else if(deviceId === "CC2652R7RGZ")
    {
        newPhySelectionOptions = [
            {name: "ble", displayName: "BLE"}
        ];
    }
    return(newPhySelectionOptions);

}

exports = applicationModule;

exports = {
    config: applicationModule,
};
