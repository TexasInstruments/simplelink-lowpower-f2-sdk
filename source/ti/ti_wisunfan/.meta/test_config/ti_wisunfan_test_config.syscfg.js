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
 *  ======== ti_wisunfan_test_config.syscfg.js ========
 */

"use strict";

// Get common utility functions
const Common = system.getScript("/ti/ti_wisunfan/ti_wisunfan_common.js");

// Get test mode setting descriptions
const Docs = system.getScript("/ti/ti_wisunfan/test_config/"
    + "ti_wisunfan_test_config_docs.js");

// Configurables for the static 15.4 test mode settings group
const config = {
    displayName: "Test Mode",
    description: "Configure settings for power and certification testing",
    config: [
        {
            name: "powerTestEnable",
            displayName: "Power Test Mode",
            default: false,
            hidden: false,
            description: Docs.powerTestEnable.description,
            longDescription: Docs.powerTestEnable.longDescription,
            onChange: onPowerTestEnableChange
        },
        {
            name: "certificationTestMode",
            displayName: "Certification Test Mode",
            default: false,
            description: Docs.certificationTestMode.description,
            longDescription: Docs.certificationTestMode.longDescription,
            onChange: onCommonTestModeChange
        },
        {
            name: "powerTestProfile",
            displayName: "Power Test Profile",
            default: "DATA_ACK",
            hidden: true,
            options: [
                {
                    name: "POLL_ACK",
                    displayName: "Polling Only"
                },
                {
                    name: "DATA_ACK",
                    displayName: "Data and ACK"
                },
                {
                    name: "POLL_DATA",
                    displayName: "Poll and Data"
                },
                {
                    name: "SLEEP",
                    displayName: "Sleep"
                }
            ],
            description: Docs.powerTestProfile.description,
            longDescription: Docs.powerTestProfile.longDescription
        },
        {
            name: "collectorTestRampDataSize",
            displayName: "Collector Test Ramp Data Size",
            default: 20,
            hidden: true,
            description: Docs.collectorTestRampDataSize.description,
            longDescription: Docs.collectorTestRampDataSize.longDescription
        },
        {
            name: "sensorTestRampDataSize",
            displayName: "Sensor Test Ramp Data Size",
            default: 0,
            hidden: true,
            description: Docs.sensorTestRampDataSize.description,
            longDescription: Docs.sensorTestRampDataSize.longDescription
        }
    ]
};

/*
 *******************************************************************************
 Test Group Config Functions
 *******************************************************************************
 */

/*
 * ======== onPowerTestEnableChange ========
 * On change function for powerTestEnable and certificationTestModes configs
 * Updates visibility of dependent configs based on whether power test enabled
 *
 * @param inst - 15.4 instance
 * @param ui   - user interface object
 */
function onPowerTestEnableChange(inst, ui)
{
    // Set visibility of common dependent configs
    onCommonTestModeChange(inst, ui);

    setTestConfigHiddenState(inst, ui, "powerTestProfile");

    if(inst.powerTestEnable === false)
    {
        // Restore the default power specific value if power test not enabled
        inst.powerTestProfile = "DATA_ACK";
    }
}

/*
 * ======== onCommonTestModeChange ========
 * On change function for powerTestEnable and certificationTestModes configs
 * Updates visibility of dependent configs based on whether test modes enabled
 *
 * @param inst - 15.4 instance
 * @param ui   - user interface object
 */
function onCommonTestModeChange(inst, ui)
{
    // Set visibility of dependent configs
    setTestConfigHiddenState(inst, ui, "collectorTestRampDataSize");
    setTestConfigHiddenState(inst, ui, "sensorTestRampDataSize");
}


/*
 * ======== getTestConfigHiddenState ========
 * Get the expected visibility of the selected config
 *
 * @param inst    - module instance containing the config that changed
 * @param cfgName - name of config
 * @returns bool  - true if hidden, false if visible
 */
function getTestConfigHiddenState(inst, cfgName)
{
    let isVisible = true;
    switch(cfgName)
    {
        case "powerTestProfile":
        {
            isVisible = inst.powerTestEnable;
            break;
        }
        case "collectorTestRampDataSize":
        {
            isVisible = inst.project.includes("borderrouter")
                && (inst.powerTestEnable || inst.certificationTestMode);
            break;
        }
        case "sensorTestRampDataSize":
        {
            isVisible = (inst.project.includes("routernode") || inst.project.includes("coapnode"))
                && (inst.powerTestEnable || inst.certificationTestMode);
            break;
        }
        case "powerTestEnable":
        case "certificationTestMode":
        default:
        {
            isVisible = true;
            break;
        }
    }

    // Return whether config is hidden
    return(!isVisible);
}

/*
 * ======== setTestConfigHiddenState ========
 * Sets the visibility of the selected config
 *
 * @param inst    - module instance containing the config that changed
 * @param ui      - user interface object
 * @param cfgName - name of config
 */
function setTestConfigHiddenState(inst, ui, cfgName)
{
    // Set visibility of config
    ui[cfgName].hidden = getTestConfigHiddenState(inst, cfgName);
    if(ui[cfgName].hidden)
    {
        // get a list of all nested and unnested configs
        const configToReset = Common.findConfig(config.config, cfgName);
        // restore the default value for the hidden parameter.
        Common.restoreDefaultValue(inst, configToReset, cfgName);
    }
}

/*
 *******************************************************************************
 Module Dependencies and Exports
 *******************************************************************************
*/

// Exports to the top level 15.4 module
exports = {
    config: config,
    setTestConfigHiddenState: setTestConfigHiddenState,
    getTestConfigHiddenState: getTestConfigHiddenState
};
