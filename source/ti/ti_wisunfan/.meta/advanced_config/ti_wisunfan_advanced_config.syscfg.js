/*
 * Copyright (c) 2023 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== ti_wisunfan_advanced_config.syscfg.js ========
 */

"use strict";

// Get common utility functions
const Common = system.getScript("/ti/ti_wisunfan/ti_wisunfan_common.js");

// Get network setting descriptions
const Docs = system.getScript("/ti/ti_wisunfan/advanced_config/"
    + "ti_wisunfan_advanced_config_docs.js");

// Configurables for the static 15.4 network settings group
const config = {
    displayName: "Advanced",
    description: "Configure advanced Wi-SUN settings",
    config: [
        {
            name: "rapidJoin",
            displayName: "Rapid Join",
            default: true,
            description: Docs.rapidJoin.description,
            longDescription: Docs.rapidJoin.longDescription
        },
        {
            name: "mplLowLatency",
            displayName: "Low Latency Multicast",
            default: false,
            description: Docs.mplLowLatency.description,
            longDescription: Docs.mplLowLatency.longDescription
        },
        {
            name: "rapidDisconnectDetectBr",
            displayName: "Border Router Disconnect Detection Time (s)",
            default: 1800,
            description: Docs.rapidDisconnectDetectBr.description,
            longDescription: Docs.rapidDisconnectDetectBr.longDescription
        },
        {
            name: "rapidDisconnectDetectRn",
            displayName: "Router Node Disconnect Detection Time (s)",
            default: 7200,
            description: Docs.rapidDisconnectDetectRn.description,
            longDescription: Docs.rapidDisconnectDetectRn.longDescription
        }
    ]
};

/*
 * ======== validate ========
 * Validate this inst's configuration
 *
 * @param inst       - Network settings instance to be validated
 * @param validation - object to hold detected validation issues
 */
function validate(inst, validation)
{
    // Log info for Wi-SUN non-compliant settings
    if(inst.mplLowLatency == true)
    {
        validation.logInfo("Low latency multicast disables some \
            Wi-SUN MPL features and is not Wi-SUN standard compliant",
            inst, "mplLowLatency");
    }

    // Add info and errors for disconnection detection times
    if(inst.rapidDisconnectDetectBr > 65535)
    {
        validation.logError("Disconnection detection time cannot \
            exceed 65535 sec",
            inst, "rapidDisconnectDetectBr");
    }
    if(inst.rapidDisconnectDetectRn > 65535)
    {
        validation.logError("Disconnection detection time cannot \
            exceed 65535 sec",
            inst, "rapidDisconnectDetectRn");
    }
    if(inst.rapidDisconnectDetectBr < 300 ||
       inst.rapidDisconnectDetectBr > 1800)
    {
        validation.logInfo("Disconnection detection time recommended \
            to be between 300 and 1800 sec",
            inst, "rapidDisconnectDetectBr");
    }
    if(inst.rapidDisconnectDetectRn < 300 ||
       inst.rapidDisconnectDetectRn > 7200)
    {
        validation.logInfo("Disconnection detection time recommended \
            to be between 300 and 7200 sec",
            inst, "rapidDisconnectDetectRn");
    }
    return;
}

/*
 *******************************************************************************
 Module Dependencies and Exports
 *******************************************************************************
 */

// Exports to the top level 15.4 module
exports = {
    config: config,
    validate: validate,
};
