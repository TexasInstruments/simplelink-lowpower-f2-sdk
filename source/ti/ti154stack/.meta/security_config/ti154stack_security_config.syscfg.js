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
 *  ======== ti154stack_security_config.syscfg.js ========
 */

"use strict";

// Get common utility functions
const Common = system.getScript("/ti/ti154stack/ti154stack_common.js");

// Get security setting descriptions
const Docs = system.getScript("/ti/ti154stack/security_config/"
    + "ti154stack_security_config_docs.js");

// Get network settings script
const networkScript = system.getScript("/ti/ti154stack/network_config/"
    + "ti154stack_network_config");

// Configurables for the static 15.4 security settings group
const config = {
    displayName: "Security",
    description: "Configure settings for network security",
    config: [
        {
            name: "secureLevel",
            displayName: "Security Level",
            default: "macSecure",
            getDisabledOptions: getDisabledSecurityOptions(),
            options: [
                {
                    name: "macSecureDisabled",
                    displayName: "Disabled"
                },
                {
                    name: "macSecure",
                    displayName: "MAC Security"
                },
                {
                    name: "macSecureAndCommissioning",
                    displayName: "MAC Security & Commissioning"
                }
            ],
            description: Docs.secureLevel.description,
            longDescription: Docs.secureLevel.longDescription,
            onChange: onSecureLevelChange
        },
        {
            name: "tfmEnabled",
            displayName: "tfmEnabled",
            hidden: true,
            default: false,
            //displayFormat: "bool",
            description: Docs.tfmEnabled.description,
            longDescription: Docs.tfmEnabled.longDescription
        },
        {
            name: "keyTableDefaultKey",
            displayName: "Pre-Shared Network Key",
            default: "123456789ABCDEF00000000000000000",
            description: Docs.keyTableDefaultKey.description,
            longDescription: Docs.keyTableDefaultKey.longDescription
        },
        {
            name: "smCollectorAuthMethods",
            displayName: "Authentication Methods",
            hidden: true,
            default: ["SMMsgs_authMethod_justAllowed",
                "SMMsgs_authMethod_defaultCode", "SMMsgs_authMethod_passkey"],
            options: [
                {
                    name: "SMMsgs_authMethod_justAllowed",
                    displayName: "Just Allowed"
                },
                {
                    name: "SMMsgs_authMethod_defaultCode",
                    displayName: "Default Code"
                },
                {
                    name: "SMMsgs_authMethod_passkey",
                    displayName: "Passkey"
                }
            ],
            description: Docs.smCollectorAuthMethods.description,
            longDescription: Docs.smCollectorAuthMethods.longDescription,
            onChange: onSmAuthMethodsChange
        },
        {
            name: "smSensorAuthMethods",
            displayName: "Authentication Method",
            hidden: true,
            default: "SMMsgs_authMethod_defaultCode",
            options: [
                {
                    name: "SMMsgs_authMethod_justAllowed",
                    displayName: "Just Allowed"
                },
                {
                    name: "SMMsgs_authMethod_defaultCode",
                    displayName: "Default Code"
                },
                {
                    name: "SMMsgs_authMethod_passkey",
                    displayName: "Passkey"
                }
            ],
            description: Docs.smSensorAuthMethods.description,
            longDescription: Docs.smSensorAuthMethods.longDescription,
            onChange: onSmAuthMethodsChange
        },
        {
            name: "smDefaultAuthCode",
            displayName: "Default Authentication Code",
            hidden: true,
            default: 0x54321,
            displayFormat: "hex",
            description: Docs.smDefaultAuthCode.description,
            longDescription: Docs.smDefaultAuthCode.longDescription
        },
        {
            name: "smKeyrefreshTimeout",
            displayName: "Key Refresh Timeout (s)",
            hidden: true,
            default: 7200,
            description: Docs.smKeyrefreshTimeout.description,
            longDescription: Docs.smKeyrefreshTimeout.longDescription
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
    setSecurityConfigHiddenState(inst, ui, "keyTableDefaultKey");
    setSecurityConfigHiddenState(inst, ui, "smCollectorAuthMethods");
    setSecurityConfigHiddenState(inst, ui, "smSensorAuthMethods");
    setSecurityConfigHiddenState(inst, ui, "smKeyrefreshTimeout");
    setSecurityConfigHiddenState(inst, ui, "smDefaultAuthCode");

    networkScript.setDefaultMaxDevices(inst);
}

/*
 * ======== onSmAuthMethodsChange ========
 * On change function for secureLevel config
 * Updates visibility of default code config based on auth method selected
 *
 * @param inst - 15.4 instance
 * @param ui   - user interface object
 */
function onSmAuthMethodsChange(inst, ui)
{
    // Set visibility of dependent configs
    setSecurityConfigHiddenState(inst, ui, "smDefaultAuthCode");
}
/*
 *  ======== getDisabledSecurityOptions ========
 *  Generates a list of options that should be disabled in the security
 *  drop-down. Only secure commissioning is disabled when not an SM project
 *
 * @returns Array - array of strings that should be disabled
 */
function getDisabledSecurityOptions()
{
    return(inst) =>
    {
        const disabledOptions = [];
        // Add SM feature option to list of disabled options if not SM project
        if(!inst.project.includes("SM"))
        {
            disabledOptions.push({
                name: "macSecureAndCommissioning",
                reason: "Commissioning only available for SM projects"
            });
        }
        return(disabledOptions);
    };
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

    // Determine if SM feature enabled
    const smEnabled = (inst.project.includes("SM")
        && (inst.secureLevel === "macSecureAndCommissioning"));

    switch(cfgName)
    {
        case "keyTableDefaultKey":
        {
            isVisible = (inst.secureLevel !== "macSecureDisabled");
            break;
        }
        case "tfmEnabled":
        {
            isVisible = false; //don't want this to be visible
            break;
        }
        case "smSensorAuthMethods":
        {
            isVisible = (smEnabled && inst.project.includes("sensor"));
            break;
        }
        case "smDefaultAuthCode":
        {
            let defaultCodeOption = false;
            if(inst.project.includes("collector"))
            {
                defaultCodeOption = _.includes(inst.smCollectorAuthMethods,
                    "SMMsgs_authMethod_defaultCode");
            }
            else
            {
                defaultCodeOption = (inst.smSensorAuthMethods
                    === "SMMsgs_authMethod_defaultCode");
            }
            isVisible = (smEnabled && defaultCodeOption);
            break;
        }
        case "smCollectorAuthMethods":
        case "smKeyrefreshTimeout":
        {
            isVisible = (smEnabled && inst.project.includes("collector"));
            break;
        }
        case "secureLevel":
        default:
        {
            isVisible = true;
            break;
        }
    }

    // Hide all configs for coprocessor project
    isVisible = isVisible && (inst.project !== "coprocessor");

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
    Common.setConfigHiddenState(inst, ui, cfgName, config.config,
        getSecurityConfigHiddenState);
}

/*
 * ======== setAllSecurityConfigsHiddenState ========
 * Sets the visibility of all security configs
 *
 * @param inst    - module instance
 * @param ui      - user interface object
 */
function setAllSecurityConfigsHiddenState(inst, ui)
{
    Common.setAllConfigsHiddenState(inst, ui, config.config,
        getSecurityConfigHiddenState);
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
    const isSecurityDisabled = inst.secureLevel === "macSecureDisabled";
    const isSMEnabled = inst.secureLevel === "macSecureAndCommissioning";
    const isSMProject = inst.project.includes("SM");

    // If disabled option chosen after switching project type.
    // Should never occur in examples as project type is locked
    if(isSMEnabled && !isSMProject)
    {
        validation.logError("Commissioning only available for SM projects",
            inst, "secureLevel");
    }

    // Network Security Key - verify 32 hexadecimal digits entered
    const actualNumDigits = inst.keyTableDefaultKey.length;

    // Verify 32 digits entered if security is enabled
    if(!isSecurityDisabled)
    {
        if(actualNumDigits !== 32)
        {
            validation.logError("Must be 32 hex digits. Currently "
                + actualNumDigits + " digits", inst, "keyTableDefaultKey");
        }

        // Verify all digits are hexadecimal
        const hexOnly = _.every(inst.keyTableDefaultKey,
            (digit) => /^[a-fA-F0-9]$/.test(digit));
        if(!hexOnly)
        {
            validation.logError("Must be hex digits only", inst,
                "keyTableDefaultKey");
        }
    }

    // Validate SM ranges if configs not hidden
    if(!getSecurityConfigHiddenState(inst, "smDefaultAuthCode"))
    {
        Common.validateRangeHex(inst, validation, "smDefaultAuthCode", 0,
            0xFFFFF);
    }

    if(!getSecurityConfigHiddenState(inst, "smKeyrefreshTimeout"))
    {
        Common.validateRangeInt(inst, validation, "smKeyrefreshTimeout", 60,
            Common.cTypeMax.u_int32);
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

    if(inst.secureLevel === "macSecureAndCommissioning")
    {
        // Pull ECDH Module for SM Commissioning
        dependencyModule.push({
            name: "smECDH",
            displayName: "ECDH",
            description: "Settings for secure commissioning",
            moduleName: "/ti/drivers/ECDH",
            hidden: true
        });

        // Pull AESECB Module for SM Commissioning
        dependencyModule.push({
            name: "smAESECB",
            displayName: "AESECB",
            description: "Settings for secure commissioning",
            moduleName: "/ti/drivers/AESECB",
            hidden: true
        });

        // Pull TRNG Module for SM Commissioning
        dependencyModule.push({
            name: "smTRNG",
            displayName: "TRNG",
            description: "Settings for secure commissioning",
            moduleName: "/ti/drivers/TRNG",
            hidden: true
        });
    }

    if(inst.secureLevel !== "macSecureDisabled")
    {
        dependencyModule.push({
            name: "aesccmModule",
            displayName: "AESCCM",
            description: "Settings for MAC security",
            moduleName: "/ti/drivers/AESCCM",
            hidden: true
        });
    }

    return(dependencyModule);
}

// Exports to the top level 15.4 module
exports = {
    config: config,
    validate: validate,
    moduleInstances: moduleInstances,
    setSecureLevelReadOnlyState: setSecureLevelReadOnlyState,
    setSecurityConfigHiddenState: setSecurityConfigHiddenState,
    setAllSecurityConfigsHiddenState: setAllSecurityConfigsHiddenState,
    getSecurityConfigHiddenState: getSecurityConfigHiddenState
};
