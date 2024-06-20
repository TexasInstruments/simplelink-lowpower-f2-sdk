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
 *  ======== ti_wisunfan_security_config.syscfg.js ========
 */

"use strict";

// Get common utility functions
const Common = system.getScript("/ti/ti_wisunfan/ti_wisunfan_common.js");

// Get security setting descriptions
const Docs = system.getScript("/ti/ti_wisunfan/security_config/"
    + "ti_wisunfan_security_config_docs.js");

// Configurables for the static 15.4 security settings group
const config = {
    displayName: "Security",
    description: "Configure settings for network security",
    config: [
        {
            name: "secureLevel",
            displayName: "Security Level",
            default: "macSecure",
            options: [
                {
                    name: "macSecureDisabled",
                    displayName: "Disabled"
                },
                {
                    name: "macSecurePresharedKey",
                    displayName: "MAC Security with Preshared Key"
                },
                {
                    name: "macSecure",
                    displayName: "Wi-SUN Compliant Security With Key Exchange"
                }
            ],
            description: Docs.secureLevel.description,
            longDescription: Docs.secureLevel.longDescription,
            onChange: onSecureLevelChange
        },
        {
            name: "euiJoin",
            displayName: "Controlled Device Joining based on EUI",
            default: false,
            description: Docs.euiJoin.description,
            longDescription: Docs.euiJoin.longDescription,
            hidden: true
        },
        {
            name: "keyTableDefaultKey1",
            displayName: "Pre-Shared Network Key 1",
            default: "BB0608572CE14D7BA2D155499CC8519B",
            description: Docs.keyTableDefaultKey.description,
            longDescription: Docs.keyTableDefaultKey.longDescription,
            hidden: true
        },
        {
            name: "keyTableDefaultKey2",
            displayName: "Pre-Shared Network Key 2",
            default: "1849835A01684FC8ACA583F37040F74C",
            description: Docs.keyTableDefaultKey.description,
            longDescription: Docs.keyTableDefaultKey.longDescription,
            hidden: true
        },
        {
            name: "keyTableDefaultKey3",
            displayName: "Pre-Shared Network Key 3",
            default: "59EA58A4B8834938ADCB6BE388C26263",
            description: Docs.keyTableDefaultKey.description,
            longDescription: Docs.keyTableDefaultKey.longDescription,
            hidden: true
        },
        {
            name: "keyTableDefaultKey4",
            displayName: "Pre-Shared Network Key 4",
            default: "E426B491BC054AF39B59F053EC128E5F",
            description: Docs.keyTableDefaultKey.description,
            longDescription: Docs.keyTableDefaultKey.longDescription,
            hidden: true
        }
    ]
};

/*
 *******************************************************************************
 Security Group Config Functions
 *******************************************************************************
 */

/*
 * ======== onSecureLevelChange ========
 * On change function for secureLevel config
 * Updates visibility of dependent configs based on security level selected
 *
 * @param inst - 15.4 instance
 * @param ui   - user interface object
 */
function onSecureLevelChange(inst, ui)
{
    // Set visibility of dependent configs
    setSecurityConfigHiddenState(inst, ui, "euiJoin");
    setSecurityConfigHiddenState(inst, ui, "keyTableDefaultKey1");
    setSecurityConfigHiddenState(inst, ui, "keyTableDefaultKey2");
    setSecurityConfigHiddenState(inst, ui, "keyTableDefaultKey3");
    setSecurityConfigHiddenState(inst, ui, "keyTableDefaultKey4");
}

/*
 * ======== setSecureLevelReadOnlyState ========
 * Sets the read only status of secure level config. If config is read only, a
 * read only reason is displayed
 *
 * @param ui       - user interface object
 * @param readOnly - true if config must be set to read only
 */
function setSecureLevelReadOnlyState(ui, readOnly)
{
    // Set read only state of config
    ui.secureLevel.readOnly = (readOnly) ? Docs.secureLevel.readOnly : false;
}

/*
 * ======== getSecurityConfigHiddenState ========
 * Get the expected visibility of the selected config
 *
 * @param inst    - module instance containing the config that changed
 * @param cfgName - name of config
 * @returns bool  - true if hidden, false if visible
 */
function getSecurityConfigHiddenState(inst, cfgName)
{
    let isVisible = true;

    switch(cfgName)
    {
        case "euiJoin":
            isVisible = (inst.secureLevel == "macSecurePresharedKey");
            break;
        case "keyTableDefaultKey1":
        case "keyTableDefaultKey2":
        case "keyTableDefaultKey3":
        case "keyTableDefaultKey4":
        {
            isVisible = (inst.secureLevel == "macSecurePresharedKey" &&
                !inst.project.includes("solar"));
            break;
        }
        case "secureLevel":
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
 * ======== setSecurityConfigHiddenState ========
 * Sets the visibility of the selected config
 *
 * @param inst    - module instance containing the config that changed
 * @param ui      - user interface object
 * @param cfgName - name of config
 */
function setSecurityConfigHiddenState(inst, ui, cfgName)
{
    // Set visibility of config
    ui[cfgName].hidden = getSecurityConfigHiddenState(inst, cfgName);
    if(ui[cfgName].hidden)
    {
        // get a list of all nested and unnested configs
        const configToReset = Common.findConfig(config.config, cfgName);
        // restore the default value for the hidden parameter.
        Common.restoreDefaultValue(inst, configToReset, cfgName);
    }
}

/*
 * ======== validate ========
 * Validate this inst's configuration
 *
 * @param inst       - settings instance to be validated
 * @param validation - object to hold detected validation issues
 */
function validate(inst, validation)
{
    // Network Security Key - verify 32 hexadecimal digits entered
    var keyModules = []
    keyModules[0] = ["keyTableDefaultKey1", inst.keyTableDefaultKey1];
    keyModules[1] = ["keyTableDefaultKey2", inst.keyTableDefaultKey2];
    keyModules[2] = ["keyTableDefaultKey3", inst.keyTableDefaultKey3];
    keyModules[3] = ["keyTableDefaultKey4", inst.keyTableDefaultKey4];

    // Log info for Wi-SUN non-compliant settings
    if (!inst.project.includes("solar"))
    {
        if(inst.secureLevel != "macSecure")
        {
            validation.logInfo("Network security must be enabled with key exchange \
            to be Wi-SUN standard compliant", inst, "secureLevel");
        }
    }
    // Verify 32 digits entered if security is enabled
    if(inst.secureLevel === "macSecurePresharedKey")
    {
        keyModules.forEach((module) =>
        {
            const name = module[0];
            const value = module[1];

            if (value.length !== 32)
            {
                validation.logError("Must be 32 hex digits. Currently "
                    + value.length + " digits", inst, name);
            }

            // Verify all digits are hexadecimal
            const hexOnly = _.every(value, (digit) => /^[a-fA-F0-9]$/.test(digit));
            if(!hexOnly)
            {
                validation.logError("Must be hex digits only", inst, name);
            }
        });
    }

}

/*
 *******************************************************************************
 Module Dependencies and Exports
 *******************************************************************************
 */

/*
 *  ======== moduleInstances ========
 *  Determines what modules are added as non-static sub-modules
 *
 *  @param inst  - Module instance containing the config that changed
 *  @returns     - Array containing dependency modules
 */
function moduleInstances(inst)
{
    const dependencyModule = [];

    dependencyModule.push({
        name: "aesccmModule",
        displayName: "AESCCM",
        description: "Settings for MAC security",
        moduleName: "/ti/drivers/AESCCM",
        hidden: true
    });

    dependencyModule.push({
        name: "aesecbModule",
        displayName: "AESECB",
        description: "Settings for MAC security",
        moduleName: "/ti/drivers/AESECB",
        hidden: true
    });

    dependencyModule.push({
        name: "aescbcModule",
        displayName: "AESCBC",
        description: "Settings for MAC security",
        moduleName: "/ti/drivers/AESCBC",
        hidden: true
    });

    return(dependencyModule);
}

// Exports to the top level 15.4 module
exports = {
    config: config,
    validate: validate,
    moduleInstances: moduleInstances,
    setSecureLevelReadOnlyState: setSecureLevelReadOnlyState,
    setSecurityConfigHiddenState: setSecurityConfigHiddenState,
    getSecurityConfigHiddenState: getSecurityConfigHiddenState
};
