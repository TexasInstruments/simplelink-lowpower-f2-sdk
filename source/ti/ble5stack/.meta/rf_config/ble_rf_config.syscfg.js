/*
 * Copyright (c) 2018 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== ble_rf_config.syscfg.js ========
 */

"use strict";

// Get rf_config long descriptions
const Docs = system.getScript("/ti/ble5stack/rf_config/ble_rf_config_docs.js");

// Get common Script
const Common = system.getScript("/ti/ble5stack/ble_common.js");

// RadioConfig module scripts
const CommonRadioConfig = system.getScript("/ti/devices/radioconfig/radioconfig_common.js");
const RfDesign = CommonRadioConfig.getScript("rfdesign");

// Get the rfDesign options according to the device
const rfDesignOptions = getRfDesignOptions(system.deviceData.deviceId);

const config = {
    name: "bleRadioConfig",
    displayName: "Radio",
    description: "Configure BLE Radio Settings",
    config: [
        {
            name: "rfDesign",
            displayName: "Based On RF Design",
            description: "Select which RF Design to use as a template",
            options: rfDesignOptions,
            default: rfDesignOptions ? rfDesignOptions[0].name : ""
        },
        {
            // RF Front End Settings
            // Note: The use of these values completely depends on how the PCB is laid out.
            //       Please see Device Package and Evaluation Module (EM) Board below.
            name: "frontEndMode",
            displayName: "Front End Mode",
            default: "RF_FE_DIFFERENTIAL",
            deprecated: true,
            longDescription: Docs.frontEndModeLongDescription,
            options: [
                {
                    displayName: "Differential",
                    name: "RF_FE_DIFFERENTIAL"
                },
                {
                    displayName: "Single Ended RFP",
                    name: "RF_FE_SINGLE_ENDED_RFP"
                },
                {
                    displayName: "Single Ended RFN",
                    name: "RF_FE_SINGLE_ENDED_RFN"
                },
                {
                    displayName: "Antenna Diversity RFP First",
                    name: "RF_FE_ANT_DIVERSITY_RFP_FIRST"
                },
                {
                    displayName: "Antenna Diversity RFN First",
                    name: "RF_FE_ANT_DIVERSITY_RFN_FIRST"
                },
                {
                    displayName: "Single Ended RFP External Pins",
                    name: "RF_FE_SINGLE_ENDED_RFP_EXT_PINS"
                },
                {
                    displayName: "Single Ended RFN External Pins",
                    name: "RF_FE_SINGLE_ENDED_RFN_EXT_PINS"
                }
            ]
        },
        {
            name: "biasConfiguration",
            displayName: "Bias Configuration",
            default: "RF_FE_INT_BIAS",
            deprecated: true,
            longDescription: Docs.biasConfigurationLongDescription,
            options: [
                {
                    displayName: "Internal BIAS",
                    name: "RF_FE_INT_BIAS"
                },
                {
                    displayName: "External BIAS",
                    name: "RF_FE_EXT_BIAS"
                }
            ]
        },
        {
            name: "defaultTxPower",
            displayName: "Default Tx Power Value",
            deprecated: true,
            default: "0",
        },
        {
            name: "defaultTxPowerValue",
            displayName: "Default Tx Power Value",
            default: "0",
            description: "This is the Tx Power value the BLE stack will use",
            options: (inst) => { return getPaTableValues(inst.rfDesign); }
        },
        {
            name: "thorPg",
            displayName: "Thor PG version",
            default: 2,
            options: [
                {
                    displayName: "1",
                    name: 1
                },
                {
                    displayName: "2",
                    name: 2
                }
            ],
            description: "Thor pg version",
            hidden: true
        }
    ]
}

/*
 * ======== getPaTableValues ========
 * Returns the tx power values options for the current device
 *
 * @param rfDesign - the selected device
 *
 * @returns - a list with the valid pa levels from the tableOptions
 */
function getPaTableValues(rfDesign)
{
    const frequency = 2400;
    let currentOptions = [];
    // Get the device PA table levels
    const txPowerTableType = Common.getRadioScript(rfDesign,
                             system.deviceData.deviceId).radioConfigParams.paExport;
    let isHighPA = txPowerTableType == "combined"? true : false;

    if(isHighPA)
    {
        // Get the options list from the rfDesign module
        currentOptions = RfDesign.getTxPowerOptions(frequency, isHighPA);
    }

    currentOptions = currentOptions.concat(RfDesign.getTxPowerOptions(frequency, false));


    return currentOptions;
}
/*
 * ======== getRfDesignOptions ========
 * Generates an array of SRFStudio compatible rfDesign options based on device
 *
 * @param deviceId - device being used
 *
 * @returns Array - Array of rfDesign options, if the device isn't supported,
 *                  returns null
 */
function getRfDesignOptions(deviceId)
{
    let newRfDesignOptions = null;
    if(deviceId === "CC1352P1F3RGZ")
    {
        newRfDesignOptions = [
            {name: "LAUNCHXL-CC1352P1"},
            {name: "LAUNCHXL-CC1352P-2"},
            {name: "LAUNCHXL-CC1352P-4"}
        ];
    }
    else if(deviceId === "CC1352P7RGZ")
    {
        newRfDesignOptions = [
            {name: "LP_CC1352P7-1"},
            {name: "LP_CC1352P7-4"}
        ];
    }
    else if(deviceId === "CC1352R1F3RGZ")
    {
        newRfDesignOptions = [{name: "LAUNCHXL-CC1352R1"}];
    }
    else if(deviceId === "CC2642R1FRGZ")
    {
        newRfDesignOptions = [{name: "LAUNCHXL-CC26X2R1"}];
    }
    else if(deviceId === "CC2642R1FRGZQ1")
    {
        newRfDesignOptions = [{name: "LAUNCHXL-CC26X2R1"}];
    }
    else if(deviceId === "CC2642R1FRTCQ1")
    {
        newRfDesignOptions = [{name: "LAUNCHXL-CC26X2R1"}];
    }
    else if(deviceId === "CC2652R1FRGZ")
    {
        newRfDesignOptions = [{name: "LAUNCHXL-CC26X2R1"}];
    }
    else if(deviceId === "CC2652RB1FRGZ")
    {
        newRfDesignOptions = [{name: "LP_CC2652RB"}];
    }
    else if(deviceId === "CC2652P1FSIP")
    {
        newRfDesignOptions = [{name: "LP_CC2652PSIP"}];
    }
    else if(deviceId === "CC2652R1FSIP")
    {
        newRfDesignOptions = [{name: "LP_CC2652RSIP"}];
    }
    else if(deviceId === "CC2652R7RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC2652R7"}];
    }
    else if(deviceId === "CC2651P3RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC2651P3"}];
    }
    else if(deviceId === "CC2651R3RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC2651R3"}];
    }
    else if(deviceId === "CC2651R3SIPA")
    {
        newRfDesignOptions = [{name: "LP_CC2651R3SIPA"}];
    }
	  else if(deviceId === "CC1354P10RSK" || deviceId === "CC1354P10RGZ")
    {
        newRfDesignOptions = [{name: "LP_EM_CC1354P10_1"},
                              {name: "LP_EM_CC1354P10_6"}];
    }
	  else if(deviceId === "CC1354R10RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC1354R10_RGZ"}];
    }
    else if(deviceId === "CC2674P10RGZ" || deviceId === "CC2674P10")
    {
        newRfDesignOptions = [{name: "LP_EM_CC2674P10"}];
    }
    else if(deviceId === "CC2674R10RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC2674R10_RGZ"}];
    }
    else if(deviceId === "CC2340R2RGE")
    {
        newRfDesignOptions = [{name: "LP_EM_CC2340R2"}]
    }
    if(deviceId === "CC2340R5RKP")
    {
        newRfDesignOptions = [{name: "LP_EM_CC2340R5"}];
    }
    else if(deviceId === "CC2340R5RGE")
    {
        newRfDesignOptions = [{name: "LP_EM_CC2340R5"}]
    }
    else if(deviceId === "CC2340R5RHB")
    {
        newRfDesignOptions = [{name: "LP_EM_CC2340R5_Q1"}]
    }
    return(newRfDesignOptions);
}

/*
 * ======== validate ========
 * Validate this inst's configuration
 *
 * @param inst       - BLE instance to be validated
 * @param validation - object to hold detected validation issues
 */
function validate(inst, validation)
{
    // Validate the value is part of the options
    let validOptions = inst.$module.$configByName.defaultTxPowerValue.options(inst);
    let isValid = validOptions.find((option) => {
        if(option.name == inst.defaultTxPowerValue)
            return true;
        });

    if(!isValid)
    {
        validation.logError("Selected option is invalid, please select a valid option", inst, "defaultTxPowerValue");
    }

    // Get the RF Design module
    const rfDesign = system.modules["/ti/devices/radioconfig/rfdesign"].$static;

    // inst.rfDesign configurable value should always be equal to
    // the value of the rfDesign configurable in rfdesign module
    if(inst.rfDesign !== rfDesign.rfDesign)
    {
        validation.logError(`Must match ${system.getReference(rfDesign,
            "rfDesign")} in the RF Design Module`, inst, "rfDesign");
    }
}

/*
 *  ======== moduleInstances ========
 *  Determines what modules are added as non-static submodules
 *
 *  @param inst  - Module instance containing the config that changed
 *  @returns     - Array containing dependency modules
 */
function moduleInstances(inst)
{
    const dependencyModule = [];
    let args;
    let modulePath;

    // Get the board default rf settings
    const radioSettings = Common.getRadioScript(inst.rfDesign,system.deviceData.deviceId).radioConfigParams;

    if(!hideRadioConfig())
    {
        modulePath = "/ti/devices/radioconfig/settings/ble";
        args = {
            $name: "RF_BLE_Setting",
            phyType: "bt5le2m",
            codeExportConfig: radioSettings,
            paramVisibility: false,
            permission: "ReadOnly"
        }

        if(inst.rfDesign == "LAUNCHXL-CC1352P-2" || inst.rfDesign == "LAUNCHXL-CC1352P-4" || inst.rfDesign == "LP_CC2652PSIP" || inst.rfDesign == "LP_CC1352P7-4" || inst.rfDesign == "LP_CC2651P3" || inst.rfDesign == "LP_EM_CC1354P10_6" || inst.rfDesign == "LP_EM_CC2674P10")
        {
            args.highPA = true;
            if(inst.rfDesign == "LAUNCHXL-CC1352P-4" || inst.rfDesign == "LP_CC1352P7-4" || inst.rfDesign == "LP_CC2652PSIP" || inst.rfDesign == "LP_EM_CC1354P10_6" )
            {
                args.phyType = "bt5le2mp10";
                args.txPowerHi = "10";
            }
            if(inst.rfDesign == "LP_EM_CC2674P10")
            {
                args.phyType = "bt5le2mp10";
                args.txPowerHi = "20";
            }
        }

        if( (inst.rfDesign == "LP_EM_CC1354P10_6") && (inst.thorPg == 1) )
        {
            args.phyType = "bt5le2mp10_pg10";
        }
        else if ( (inst.rfDesign == "LP_EM_CC1354P10_1") && (inst.thorPg == 1) )
        {
            args.phyType = "bt5le2m_pg10";
        }

    }
    else
    {
        modulePath = "/ti/devices/radioconfig/phy_groups/ble";
        args = {
            $name: "RF_BLE_Setting",
            phyType: "ble",
            codeExportConfig: radioSettings,
            paramVisibility: false,
            permission: "ReadOnly"
        }
    }

    dependencyModule.push({
        name: "radioConfig",
        group: "bleRadioConfig",
        displayName: "BLE Radio Configuration",
        moduleName: modulePath,
        collapsed: true,
        args: args
    });

    return(dependencyModule);
}

/*
 *  ======== hideRadioConfig ========
 *  Check which device is used.
 *  @return Bool - True if radio config should be hidden
 *                 False if radio config should not be hidden
 */
function hideRadioConfig()
{
    if( Common.device2DeviceFamily(system.deviceData.deviceId) == "DeviceFamily_CC23X0R5" ||
        Common.device2DeviceFamily(system.deviceData.deviceId) == "DeviceFamily_CC23X0R2" )
    {
       return true;
    }

    return false;
}

/*
 *  ======== exports ========
 *  Export the BLE RF Settings Configuration
 */
exports = {
    config: config,
    validate: validate,
    moduleInstances: moduleInstances
};