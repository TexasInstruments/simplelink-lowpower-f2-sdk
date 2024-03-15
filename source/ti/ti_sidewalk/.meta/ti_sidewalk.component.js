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
 *  =========================== ti_sidewalk.component.js ===========================
 *  Component definition for the SysConfig TI Sidewalk module
 */

"use strict";

const deviceId = system.deviceData.deviceId;
const displayName = "TI-Sidewalk";

let topModules = [];
let templates = [];
let enableSidewalkModule = false;

const supportedDevices = [
    "CC1352P7RGZ",
    "CC2652R7RGZ",
];

for (const device of supportedDevices) {
    if (deviceId.match(device)) {
        enableSidewalkModule = true;
    }
}
/* Only export TI-Sidewalk module if device is supported */
if(enableSidewalkModule)
{
    topModules = [
        {
            displayName: "RF Stacks",
            description: "RF Stacks",
            modules: ["/ti/ti_sidewalk/ti_sidewalk"]
        }
    ];
    templates = [
        {
            name: "/ti/ti_sidewalk/templates/ti_sidewalk_config.yaml.xdt",
            outputPath: "ti_sidewalk_config.yaml"
        }
    ];
}


exports = {
    displayName: displayName,
    topModules: topModules,
    templates: templates
};
