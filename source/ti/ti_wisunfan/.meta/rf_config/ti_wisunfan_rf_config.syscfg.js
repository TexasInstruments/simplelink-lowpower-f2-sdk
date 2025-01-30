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
 *  ======== ti_wisunfan_rf_config.syscfg.js ========
 */

"use strict";

// Get common utility functions
const Common = system.getScript("/ti/ti_wisunfan/ti_wisunfan_common.js");

// Get common rf settings
const rfCommon = system.getScript("/ti/ti_wisunfan/rf_config/"
    + "ti_wisunfan_rf_config_common.js");

// Get transmit power settings script
const powerScript = system.getScript("/ti/ti_wisunfan/power_config/"
    + "ti_wisunfan_power_config");

// Get network settings script
const networkScript = system.getScript("/ti/ti_wisunfan/network_config/"
    + "ti_wisunfan_network_config");

// Get RF setting descriptions
const Docs = system.getScript("/ti/ti_wisunfan/rf_config/"
    + "ti_wisunfan_rf_config_docs.js");

const defaults = rfCommon.getDefaults();
const defaultRegion = defaults[0];
const defaultRegDomain = defaults[1];
const defaultPhyModeID = defaults[2];
const defaultChanPlanID = defaults[3];

// Configurables for the RF Configuration module
const config = {
    name: "radioSettings",
    displayName: "Radio",
    description: "Configure PHY settings for radio operations",
    config: [
        {
            name: "rfDesign",
            displayName: "Based On RF Design",
            options: getRfDesignOptions(),
            default: getRfDesignOptions()[0].name,
            description: Docs.rfDesign.description,
            longDescription: Docs.rfDesign.longDescription,
            onChange: onRadioConfigChange
        },
        {
            name: "region",
            displayName: "Region",
            options: rfCommon.getRegionOptions,
            default: defaultRegion,
            description: Docs.region.description,
            longDescription: Docs.region.longDescription,
            onChange: onRegionChange
        },
        {
            name: "regDomain",
            displayName: "Regulatory Domain",
            default: defaultRegDomain,
            readOnly: Docs.regDomain.readOnly,
            description: Docs.regDomain.description,
            longDescription: Docs.regDomain.longDescription
            //onChange: onRegDomainChange
        },
        {
            name: "phyModeID",
            displayName: "Phy Mode ID",
            default: defaultPhyModeID,
            options: rfCommon.getPhyModeIDOptions,
            readOnly: Docs.phyModeID.readOnly,
            description: Docs.phyModeID.description,
            longDescription: Docs.phyModeID.longDescription,
            onChange: onPhyModeIDChange
        },
        {
            name: "opModeID",
            displayName: "Operating Mode ID",
            default: rfCommon.getOpModeIDDisplayName(defaultPhyModeID),
            readOnly: Docs.opModeID.readOnly,
            description: Docs.opModeID.description,
            longDescription: Docs.opModeID.longDescription
        },
        {
            name: "opModeClass",
            displayName: "Operating Mode Class",
            default: _.first(rfCommon.getSupportedOpClassOptions()).displayName,
            options: rfCommon.getSupportedOpClassOptions,
            description: Docs.opModeClass.description,
            longDescription: Docs.opModeClass.longDescription
        },
        {
            name: "ChanPlanID",
            displayName: "Channel Plan ID",
            default: defaultChanPlanID,
            options: rfCommon.getChanPlanIDOpts,
            onChange: onChanPlanIDChange,
            //readOnly: Docs.channelPlanID.readOnly,
            //description: Docs.channelPlanID.description,
            //longDescription: Docs.channelPlanID.longDescription
        },
        {
            name: "customPhy",
            displayName: "Select a Custom Channel Plan",
            default: false,
            hidden: false,
            description: Docs.customPhy.description,
            longDescription: Docs.customPhy.longDescription,
            onChange: onCustomPhyChange
        },
        {
            name: "centerFrequency",
            displayName: "Center Frequency Channel 0 (MHz)",
            default: rfCommon.getCenterFreqBasedOnID(defaultRegion, defaultPhyModeID, defaultChanPlanID),
            description: Docs.centerFrequency.description,
            longDescription: Docs.centerFrequency.longDescription,
            readOnly: Docs.centerFrequency.readOnly
        },
        {
            name: "channelSpacing",
            displayName: "Channel Spacing (kHz)",
            default: rfCommon.getChanSpacingBasedOnID(defaultRegion, defaultPhyModeID, defaultChanPlanID),
            readOnly: Docs.channelSpacing.readOnly,
            description: Docs.channelSpacing.description,
            longDescription: Docs.channelSpacing.longDescription
        },
        {
            name: "totalChannels",
            displayName: "Total Channels",
            default: rfCommon.getTotalNumChannelsBasedOnID(defaultRegion, defaultPhyModeID, defaultChanPlanID),
            readOnly: Docs.totalChannels.readOnly,
            description: Docs.totalChannels.description,
            longDescription: Docs.totalChannels.longDescription,
            onChange: onTotalNumChannelsChange
        }       
    ]
};


/*
 *******************************************************************************
 Radio Group-Specific Functions
 *******************************************************************************
 */

/*
 * ======== getRfDesignOptions ========
 * Generates an array of SRFStudio compatible rfDesign options based on device
 *
 * @param deviceId - device being used
 * @returns Array - Array of rfDesign options, if the device isn't supported,
 *                  returns null
 */
function getRfDesignOptions()
{
    const deviceId = system.deviceData.deviceId;
    let newRfDesignOptions = null;

    if(deviceId === "CC1352P1F3RGZ")
    {
        newRfDesignOptions = [
            {name: "LAUNCHXL-CC1352P1"},
            {name: "LAUNCHXL-CC1352P-2"},
            {name: "LAUNCHXL-CC1352P-4"}
        ];
    }
    else if(deviceId === "CC1352R1F3RGZ")
    {
        newRfDesignOptions = [{name: "LAUNCHXL-CC1352R1"}];
    }
    else if(deviceId === "CC1312R1F3RGZ")
    {
        newRfDesignOptions = [{name: "LAUNCHXL-CC1312R1"}];
    }
    else if(deviceId === "CC2642R1FRGZ")
    {
        newRfDesignOptions = [{name: "LAUNCHXL-CC26X2R1"}];
    }
    else if(deviceId === "CC2652R1FRGZ")
    {
        newRfDesignOptions = [{name: "LAUNCHXL-CC26X2R1"}];
    }
    else if(deviceId === "CC2652RB")
    {
        newRfDesignOptions = [{name: "LAUNCHXL-CC2652RB"}];
    }
    else if(deviceId === "CC1352P7RGZ")
    {
        newRfDesignOptions = [
            {name: "LP_CC1352P7-1"},
        ];
    }
    else if(deviceId === "CC1312R7RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC1312R7"}];
    }
    else if(deviceId === "CC1312PSIP")
    {
        newRfDesignOptions = [{name: "LP_EM_CC1312PSIP"}];
    }
    else if(deviceId === "CC1314R10RSK")
    {
        newRfDesignOptions = [{name: "LP_EM_CC1314R10"}];
    }
    else if(deviceId === "CC1314R10RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC1314R10_RGZ"}];
    }
    else if(deviceId === "CC1354R10RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC1354R10_RGZ"}];
    }
    else if(deviceId === "CC1354P10RSK")
    {
        newRfDesignOptions = [
            {name: "LP_EM_CC1354P10_1"},
            {name: "LP_EM_CC1354P10_6"}
        ];
    }
    else if(deviceId === "CC1354P10RGZ")
    {
        newRfDesignOptions = [
            {name: "LP_CC1354P10_1_RGZ"},
            {name: "LP_CC1354P10_6_RGZ"}
        ];
    }
    else
    {
        throw new Error("Unknown deviceId " + deviceId + ".");
    }
    return(newRfDesignOptions);
}

/*
 * ======== onRadioConfigChange ========
 * On change function for rfDesign config
 * Updates visibility and values of RF device-dependent configs
 *
 * @param inst - 15.4 instance
 * @param ui   - user interface object
 */
function onRadioConfigChange(inst, ui)
{
    // Update dependencies
    const isSubGSelected = true;
}


/*
 * ======== onRadioConfigChange ========
 * On change function for rfDesign config
 * Updates visibility and values of RF device-dependent configs
 *
 * @param inst - 15.4 instance
 * @param ui   - user interface object
 */
function onRegionChange(inst, ui)
{
    // Update dependencies
    ui["centerFrequency"].hidden = false;
    
    rfCommon.setRegDomain(inst);
    rfCommon.setPhyModeID(inst);
    onPhyModeIDChange(inst, ui);
    networkScript.setAdvancedMacConfigs(inst, ui, "region");
}

function onCustomPhyChange(inst, ui)
{
    setReadOnlyValues(inst, ui);    
    if(inst.customPhy == false)
    {
        networkScript.setDefaultChannelMasks(inst);
        rfCommon.setRegDomain(inst);
        rfCommon.getPhyModeIDOptions(inst);
        rfCommon.setPhyModeID(inst);
        rfCommon.setChanPlanID(inst);
        rfCommon.setCenterFrequency(inst);
        rfCommon.setTotalNumChannels(inst);
        rfCommon.setChanSpacing(inst);
        setReadOnlyValues(inst, ui);
        
    }
    else
    {
        networkScript.setDefaultChannelMasks(inst);
    }
}

/*
 * ======== onPhyModeIDChange ========
 * On change function for rfDesign config
 * Updates visibility and values of RF device-dependent configs
 *
 * @param inst - 15.4 instance
 * @param ui   - user interface object
 */

function onPhyModeIDChange(inst, ui)
{
    rfCommon.setOpModeID(inst);
    rfCommon.setOpModeClass(inst);
    rfCommon.setChanPlanID(inst);
    rfCommon.setCenterFrequency(inst);
    rfCommon.setTotalNumChannels(inst);
    rfCommon.setChanSpacing(inst);
    setReadOnlyValues(inst, ui); 
    networkScript.setDefaultChannelMasks(inst);

}

function setReadOnlyValues(inst, ui)
{
    if(inst.customPhy == true)
    {
        ui['centerFrequency'].readOnly = false;
        ui['channelSpacing'].readOnly = false;
        ui['totalChannels'].readOnly = false;
    }
    else
    {
        ui['centerFrequency'].readOnly = Docs.centerFrequency.readOnly;
        ui['channelSpacing'].readOnly = Docs.channelSpacing.readOnly;
        ui['totalChannels'].readOnly = Docs.totalChannels.readOnly;
    }
}

function onChanPlanIDChange(inst, ui)
{
    rfCommon.setCenterFrequency(inst);
    rfCommon.setTotalNumChannels(inst);
    rfCommon.setChanSpacing(inst);
    setReadOnlyValues(inst, ui);
    networkScript.setDefaultChannelMasks(inst);
}


function onTotalNumChannelsChange(inst, ui)
{
    if(inst.customPhy == true)
    {
        networkScript.setDefaultChannelMasks(inst);
    }
}

/*
 * ======== onFreqSub1orPhyTypeChange ========
 * On change function for freqBandSub1 and phy type configs
 * Updates values of phy ID and channel page based on frequency and phy type
 *
 * @param inst - 15.4 instance
 */
function onFreqSub1orPhyTypeChange(inst)
{
    setPhyIDChannelPage(inst);

    // Update values of frequency dependent configs
    // TODO: Update this
    networkScript.setDefaultChannelMasks(inst);
}

/*
 * ======== getPhyTypeOptions ========
 * Generates list of phys supported for drop down config
 *
 * @param inst - 15.4 instance (null during initialization)
 * @returns Array - array of options representing phys supported
 */
function getPhyTypeOptions(inst)
{
    let phyList;

    
    // Get proprietary Sub-1 GHz RF defaults for the device being used
    phyList = rfCommon.getPropPhySettings(inst);
    
    // Construct the drop down options array
    return(_.map(phyList, (phy) => phy.phyDropDownOption));
}

/*
 *  ======== getPhy154Settings ========
 *  Retrieves array of 15.4 phy ID and channel page settings corresponding
 *  to selected data rate from the <board_name>_rf_defaults.js
 *
 *  @param inst - 15.4 instance (null during initialization)
 *  @param freqSub1 - Sub-1 GHz frequency (Has no effect for 2.4 GHz)
 *  @param phyType - Name of phy type (phy5kbps, phy50kbps, phy200kbps, phyIEEE)
 *  @returns Array - an array containing dictionary with channel page and phy ID
 */
function getPhy154Settings(inst, phyType)
{
    let phy154Setting = null;

    // Get proprietary Sub-1 GHz RF defaults for the device being used
    const propPhySettings = rfCommon.getPropPhySettings(inst);

    // Find phy object associated with phy type
    const phyObj = _.find(propPhySettings,
        (settings) => (settings.phyDropDownOption.name === phyType));
   
    return(phy154Setting);
}

/*
 *  ======== setPhyType ========
 *  Update phy type based on frequency band
 *
 *  @param inst - Instance of this module
 */
function setPhyType(inst)
{
    const isSubGSelected = true;
    inst.phyType = getDefaultPhyType();
}

/*
 *  ======== setPhyIDChannelPage ========
 *  Update phy ID and channel page based on phy type and frequency band
 *
 *  @param inst - Instance of this module
 */
function setPhyIDChannelPage(inst)
{
    const phyType = getSafePhyType(inst);
    const newSettings = getPhy154Settings(inst, phyType);

    // Check needed to ensure combination of safe values is valid
    if(newSettings)
    {
        inst.channelPage = newSettings.channelPage;
        inst.phyID = newSettings.ID;
    }
}

/*
 *  ======== getDefaultPhyType ========
 *  Retrieves the default phyType
 *      * 50kbps for Sub-1 GHz
 *      * 250kbps for 2.4 Ghz
 *
 *  @param getSubGDefault - Boolean. True selects Sub-1 GHz default value,
 *      False returns 2.4 GHz default value
 *  @returns - name of default phyType (50kbps, 2-GFSK)
 */
function getDefaultPhyType()
{
    let defaultPhyType;
    
    defaultPhyType = "phy50kbps";
    return(defaultPhyType);
}

/*
 *  ======== getDefaultPhy154Settings ========
 *  Gets default channel page and phy ID array
 *
 *  @returns Array - an array with default channel page and phy ID
 */
function getDefaultPhy154Settings()
{
    const isSubGSelected = Common.isSub1GHzDevice();
    const defaultPhyType = getDefaultPhyType();
    const defaultFreqSub1 = rfCommon.getDefaultFreqSub1(null);

    return(getPhy154Settings(null, defaultFreqSub1,
        defaultPhyType));
}

/*
 * ======== getRFConfigHiddenState ========
 * Get the expected visibility of the selected config
 *
 * @param inst    - module instance containing the config that changed
 * @param cfgName - name of config
 * @returns bool  - true if hidden, false if visible
 */
function getRFConfigHiddenState(inst, cfgName)
{
    let isVisible = true;
    switch(cfgName)
    {
        case "channelSpacing":
        {
            isVisible = (inst.customPhy == true);
            break;
        }
        case "ccaType":
        {
            isVisible = (inst.customPhy == true);
            break;
        }
        case "totalChannels":
        {
            isVisible = (inst.customPhy == true);
            break;
        }
        case "rfDesign":
        case "phyType":
        case "channelPage":
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
 * ======== setRFConfigHiddenState ========
 * Sets the visibility of the selected config
 *
 * @param inst    - module instance containing the config that changed
 * @param ui      - user interface object
 * @param cfgName - name of config
 */
function setRFConfigHiddenState(inst, ui, cfgName)
{
    // Set visibility of config
    ui[cfgName].hidden = getRFConfigHiddenState(inst, cfgName);
    if(ui[cfgName].hidden)
    {
        // get a list of all nested and unnested configs
        const configToReset = Common.findConfig(config.config, cfgName);
        // restore the default value for the hidden parameter.
        Common.restoreDefaultValue(inst, configToReset, cfgName);
    }
}

/*!
 * ======== getSafePhyType ========
 * Safely retrieve the value of the config by returning the instance value it's
 * valid, otherwise returns the default value.
 *
 * Due to their nature, dynamic enum configurables may be incorrectly modified
 * through the .syscfg file. While all dynamic configs have validation functions
 * to detect such errors, the dependency of the radio config module requires
 * safe access to some configs to avoid SysCOnfig breaks.
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @returns - config value in instance (if valid), otherwise config default
 *            value
 */
function getSafePhyType(inst)
{
    const validOptions = getPhyTypeOptions(inst);
    const isSubGSelected = Common.isSub1GHzDevice();
    const defaultPhyType = getDefaultPhyType();

    const phyType = Common.getSafeDynamicConfig(inst, "phyType",
        defaultPhyType, validOptions);

    return(phyType);
}

/*
 * ======== validate ========
 * Validate this inst's configuration
 *
 * @param inst       - RF Settings instance to be validated
 * @param validation - object to hold detected validation issues
 */
function validate(inst, validation)
{
    // Validate dynamic configs

    /*
        TODO: Write validate functions for both freqSub1 and phyType.
    */

    //validOptions = rfCommon.getFreqSub1Options(inst);
    //Common.validateDynamicEnum(inst, validation, "freqSub1", validOptions);

    let validOptions = rfCommon.getSupportedOpClassOptions();
    Common.validateDynamicEnum(inst, validation, "opModeClass", validOptions);

    // Get the RF Design module to verify that RF Design configs match
    const rfDesign = system.modules["/ti/devices/radioconfig/rfdesign"].$static;
    if(inst.rfDesign !== rfDesign.rfDesign)
    {
        validation.logError(`Must match ${system.getReference(rfDesign,
            "rfDesign")} in the RF Design Module`, inst, "rfDesign");
    }
}

/*
 *******************************************************************************
 Module Dependencies and Exports
 *******************************************************************************
 */

/*
 *  ======== addRFSettingDependency ========
 *  Creates an RF setting dependency module
 *
 * @param inst  - Module instance containing the config that changed
 * @returns dictionary - containing a single RF setting dependency module
 */
function addRFSettingDependency(inst)
{
    const phyType = getSafePhyType(inst);
    // Get proprietary Sub-1 GHz RF defaults for the device being used
    const propPhySettings = rfCommon.getPropPhySettings(inst);

    /* Since phy is selected based on region and op code, there should only be 1 phy choice in the list at all times. */
    let selectedPhy = _.first(propPhySettings);
    if (!selectedPhy)
    {
        return [];
    }

    // Get settings from selected phy
    const radioConfigArgs = _.cloneDeep(selectedPhy.args);

    // Only generate either default PA or high PA table as required
    radioConfigArgs.codeExportConfig.paExport = "active";

    // Retrieve phy and phy group from rf_defaults files to get tx power
    // configuration that needs to be set in the radio config module
    const rfPhySettings = rfCommon.getPhyTypeGroupFromRFConfig(inst);
    const phy = rfPhySettings.phyType;
    const phyGroup = rfPhySettings.phyGroup;
    const txPower154Obj = powerScript.getRFTxPowerFrom154TxPower(inst,
        phy, phyGroup);

    // Set radio config tx power based on 15.4 tx power setting
    radioConfigArgs[txPower154Obj.cfgName] = txPower154Obj.txPower;

    // Set high PA in radio config if supported by board
    if(Common.isHighPADevice())
    {
        radioConfigArgs.highPA = txPower154Obj.highPA;
    }

    // Add Wisun specific overrides for sub-G projects
    const overridesMacro = "TI_154_STACK_OVERRIDES";
    radioConfigArgs.codeExportConfig.stackOverride = "ti/ti_wisunfan/wisunfan_mac/"
        + "common/boards/ti_154stack_overrides.h";
    radioConfigArgs.codeExportConfig.stackOverrideMacro = overridesMacro;


    return({
        name: "radioConfig",
        displayName: selectedPhy.phyDropDownOption.displayName,
        moduleName: selectedPhy.moduleName,
        description: "Radio configuration",
        readOnly: false,
        hidden: true,
        collapsed: true,
        group: "radioSettings",
        args: radioConfigArgs
    });
}

/*
 *  ======== moduleInstances ========
 *  Determines what modules are added as non-static sub-modules
 *
 *  @param inst  - Module instance containing the config that changed
 *  @returns     - Dependency module of selected phyType
 */
function moduleInstances(inst)
{
    // Add radio config module associated with phy selected
    return(addRFSettingDependency(inst));
}

// Exports to the top level 15.4 module
exports = {
    config: config,
    validate: validate,
    moduleInstances: moduleInstances,
    getPhyTypeOptions: getPhyTypeOptions,
    getPhy154Settings: getPhy154Settings,
    setRFConfigHiddenState: setRFConfigHiddenState,
    getRFConfigHiddenState: getRFConfigHiddenState
};
