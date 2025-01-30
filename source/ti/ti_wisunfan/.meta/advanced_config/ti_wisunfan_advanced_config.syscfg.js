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

const networkProfileOptions = [
    {name: "Small", displayName: "Maximize Responsiveness",
        description: "Maximum network responsiveness (for small networks)"},
    {name: "Medium", displayName: "Balanced Mode",
        description: "Balance responsiveness and scalability (for medium size networks)"},
    {name: "Large", displayName: "Maximize Scalabiliy",
        description: "Maximize network scalability (for large networks)"},
]

// Configurables for the static 15.4 network settings group
const config = {
    displayName: "Advanced",
    description: "Configure advanced settings",
    config: [
        {
            name: "networkProfile",
            displayName: "Network Profile",
            options: networkProfileOptions,
            default: "Small",
            hidden: false,
            description: Docs.networkProfile.description,
            longDescription: Docs.networkProfile.longDescription,
            onChange: onNetworkSizeChange
        },
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
            default: 1800, // 1800s, 30 min
            description: Docs.rapidDisconnectDetectBr.description,
            longDescription: Docs.rapidDisconnectDetectBr.longDescription
        },
        {
            name: "rapidDisconnectDetectRn",
            displayName: "Router Node Disconnect Detection Time (s)",
            default: 7200, // 7200s, 2 hours
            description: Docs.rapidDisconnectDetectRn.description,
            longDescription: Docs.rapidDisconnectDetectRn.longDescription
        }
    ]
};

function onNetworkSizeChange(inst, ui)
{
    if (inst.networkProfile == "Small")
    {
        ui.rapidJoin.readOnly = false;
        inst.rapidDisconnectDetectBr = 30*60;  // 30 min
        inst.rapidDisconnectDetectRn = 2*3600; // 2 hours
    }
    else if (inst.networkProfile == "Medium")
    {
        ui.rapidJoin.readOnly = Docs.rapidJoin.readOnly;
        inst.rapidJoin = false;
        inst.mplLowLatency = false;
        inst.rapidDisconnectDetectBr = 60*60;  // 60 min
        inst.rapidDisconnectDetectRn = 4*3600; // 4 hours
    }
    else if (inst.networkProfile == "Large")
    {
        ui.rapidJoin.readOnly = Docs.rapidJoin.readOnly;
        inst.rapidJoin = false;
        inst.mplLowLatency = false;
        inst.rapidDisconnectDetectBr = 90*60;  // 90 min
        inst.rapidDisconnectDetectRn = 8*3600; // 8 hours
    }
}

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

    // Add info and errors for disconnection detection times
    if(inst.networkProfile == "Small")
    {
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
    }
    else if(inst.networkProfile == "Medium")
    {
        if(inst.rapidDisconnectDetectBr != 3600)
        {
            validation.logInfo("Disconnection detection time recommended \
                to be 3600 sec",
                inst, "rapidDisconnectDetectBr");
        }
        if(inst.rapidDisconnectDetectRn != 14400)
        {
            validation.logInfo("Disconnection detection time recommended \
                to be 14400 sec",
                inst, "rapidDisconnectDetectRn");
        }

    }
    else if(inst.networkProfile == "Large")
    {
        if(inst.rapidDisconnectDetectBr != 5400)
        {
            validation.logInfo("Disconnection detection time recommended \
                to be 5400 sec",
                inst, "rapidDisconnectDetectBr");
        }
        if(inst.rapidDisconnectDetectRn != 28800)
        {
            validation.logInfo("Disconnection detection time recommended \
                to be 28800 sec",
                inst, "rapidDisconnectDetectRn");
        }
    }
    return;
}

/*!
 * ======== setDefaultAdvancedSettings ========
 * Sets the default advanced settings for solar projects
 *
 * @param inst - module instance
 */
function setDefaultAdvancedSettings(inst)
{
    if (inst != null && inst.project.includes("solar"))
    {
        inst.mplLowLatency = true;
        inst.rapidDisconnectDetectBr = 300;
        inst.rapidDisconnectDetectRn = 300;
    }
    else
    {
        inst.mplLowLatency = false;
        inst.rapidDisconnectDetectBr = 1800;
        inst.rapidDisconnectDetectRn = 7200;
    }
}

/*!
 * ======== setDefaultAdvancedSettings ========
 * Sets the default advanced settings for solar projects
 *
 * @param inst - module instance
 */
function setAdvancedSettingsHiddenState(inst, ui)
{
    if (inst != null && inst.project.includes("solar"))
    {
        ui["rapidJoin"].hidden = true;
        ui["mplLowLatency"].hidden = true;
        ui["rapidDisconnectDetectBr"].hidden = true;
        ui["rapidDisconnectDetectRn"].hidden = true;
    }
    else
    {
        ui["rapidJoin"].hidden = false;
        ui["mplLowLatency"].hidden = false;
        ui["rapidDisconnectDetectBr"].hidden = false;
        ui["rapidDisconnectDetectRn"].hidden = false;
    }
    setDefaultAdvancedSettings(inst);
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
    setDefaultAdvancedSettings: setDefaultAdvancedSettings,
    setAdvancedSettingsHiddenState: setAdvancedSettingsHiddenState,
};
