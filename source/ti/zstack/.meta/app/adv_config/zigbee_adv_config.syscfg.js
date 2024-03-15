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
 *  ======== zigbee_adv_config.syscfg.js ========
 */

"use strict";

/* Get common utility functions */
const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Get Clusters and Attributes from ZCL.js */
const clusters = system.getScript("/ti/zstack/stack/zcl/ZCL.js").ZCL.clusters;

/* Get devices from ZDL.js */
let devices = system.getScript("/ti/zstack/stack/zcl/ZDL.js").ZDL.devices;

/* Get Add long descriptions */
const docs = system.getScript("/ti/zstack/app/adv_config/zigbee_adv_config_docs.js");

Common.addGlobalClusters(devices, clusters);

/* Configurables for the Zigbee Application Advanced Settings module */
const advModule = {
    displayName: "Advanced Settings",
    config: [
        {
            name: "endpoint",
            displayName: "Endpoint",
            description: "Endpoint",
            longDescription: docs.endpointLongDescription,
            default: 0x8,
            displayFormat: "hex"
        },
        {
            name: "deviceVersion",
            displayName: "Device Version",
            description: "Device Version",
            longDescription: docs.deviceVersionLongDescription,
            default: 0x1,
            displayFormat: "hex"
        },
        {
            name: "flags",
            displayName: "Flags",
            description: "Flags",
            longDescription: docs.flagsLongDescription,
            default: 0x0,
            displayFormat: "hex"
        }
    ],
};

/* Exports to the top level of the Zigbee Application submodule */
exports = {
    advModule: advModule
};
