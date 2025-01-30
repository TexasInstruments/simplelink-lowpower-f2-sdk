/*
 * Copyright (c) 2017-2018 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== ti_wisunfan.component.js ========
 */

const categoryName = "RF Stacks";
const supportedDevices = [
    "CC1312R1F3RGZ",
    "CC1312PSIP",
    "CC1352P1F3RGZ",
    "CC1352R1F3RGZ",
    "CC1312R7RGZ",
    "CC1352P7RGZ",
    "CC1314R10RSK",
    "CC1314R10RGZ",
    "CC1354R10RGZ",
    "CC1354P10RSK",
    "CC1354P10RGZ",
];

let topModules = [];
let templates = [];
let enableWisunModule = false;

/* Only export Wisun module if device is supported */
const deviceId = system.deviceData.deviceId;
for (const device of supportedDevices) {
    if (deviceId.match(device)) {
        enableWisunModule = true;
    }
}

if(enableWisunModule)
{
    topModules = [
        {
            displayName: categoryName,
            description: categoryName,
            modules: ["/ti/ti_wisunfan/ti_wisunfan"]
        }
    ];
    templates = [
        {
            name: "/ti/ti_wisunfan/templates/ti_wisunfan_config.h.xdt",
            outputPath: "ti_wisunfan_config.h"
        },
        {
            name: "/ti/ti_wisunfan/templates/ti_wisunfan_features.h.xdt",
            outputPath: "ti_wisunfan_features.h"
        },
    ];
}

exports = {
    displayName: "TI Wi-SUN FAN Stack",
    topModules: topModules,
    templates: templates
};
