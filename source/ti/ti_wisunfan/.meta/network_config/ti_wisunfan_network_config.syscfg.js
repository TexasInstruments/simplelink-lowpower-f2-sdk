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
 *  ======== ti_wisunfan_network_config.syscfg.js ========
 */

"use strict";

// Get common utility functions
const Common = system.getScript("/ti/ti_wisunfan/ti_wisunfan_common.js");

// Get common rf settings
const rfCommon = system.getScript("/ti/ti_wisunfan/rf_config/"
    + "ti_wisunfan_rf_config_common.js");
// Get network setting descriptions
const Docs = system.getScript("/ti/ti_wisunfan/network_config/"
    + "ti_wisunfan_network_config_docs.js");

// Configurables for the static 15.4 network settings group
const config = {
    displayName: "Network",
    description: "Configure network settings",
    config: [
        {
            name: "panID",
            displayName: "Pan ID",
            default: 0xffff,
            displayFormat: "hex",
            description: Docs.panID.description,
            longDescription: Docs.panID.longDescription
        },
        {
            name: "channels",
            displayName: "Channel Mask",
            default: getDefaultChannelMask(null),
            options: (inst) => getChannelOptions(inst, false),
            description: Docs.channels.description,
            longDescription: Docs.channels.longDescription,
            hidden: true
        },
        {
            name: "unicastChannelMask",
            displayName: "Unicast Channel Mask",
            default: getDefaultChannelMask(null),
            options: (inst) => getChannelOptions(inst, false),
            hidden: false,
            description: Docs.fhChannels.description,
            longDescription: Docs.fhChannels.longDescription
        },
        {
            name: "broadcastChannelMask",
            displayName: "Broadcast Channel Mask",
            default: getDefaultChannelMask(null),
            options: (inst) => getChannelOptions(inst, false),
            hidden: false,
            description: Docs.fhChannels.description,
            longDescription: Docs.fhChannels.longDescription
        },
        {
            name: "fhAsyncChannels",
            displayName: "Async Channel Mask",
            default: getDefaultChannelMask(null),
            options: (inst) => getChannelOptions(inst, false),
            hidden: false,
            description: Docs.fhAsyncChannels.description,
            longDescription: Docs.fhAsyncChannels.longDescription
        },
        {
            name: "fhNetname",
            displayName: "Network Name",
            default: getDefaultNetname(null),
            hidden: false,
            description: Docs.fhNetname.description,
            longDescription: Docs.fhNetname.longDescription
        },
        {
            name: "maxDevices",
            displayName: "Max Devices",
            default: 50,
            description: Docs.maxDevices.description,
            longDescription: Docs.maxDevices.longDescription,
            hidden: true
        },
        {
            name: "starNetwork",
            displayName: "Force star network topology",
            default: false,
            hidden: false,
            description: Docs.starNetwork.description,
            longDescription: Docs.starNetwork.longDescription
        },
        {
            displayName: "MAC",
            description: "Configure settings for MAC-level operations",
            config: [
                {
                    name: "ccaThreshold",
                    displayName: "MAC CCA Threshold (dBm)",
                    default: -83,
                    description: Docs.ccaThreshold.description,
                    longDescription: Docs.ccaThreshold.longDescription
                },
                {
                    name: "unicastDwellTime",
                    displayName: "Unicast Dwell Time (ms)",
                    default: 255,
                    hidden: false,
                    description: Docs.unicastDwellTime.description,
                    longDescription: Docs.unicastDwellTime.longDescription
                },
                {
                    name: "fhBroadcastInterval",
                    displayName: "Broadcast Interval (ms)",
                    default: 1020,
                    hidden: false,
                    description: Docs.fhBroadcastInterval.description,
                    longDescription: Docs.fhBroadcastInterval.longDescription
                },
                {
                    name: "fhBroadcastDwellTime",
                    displayName: "Broadcast Dwell Time (ms)",
                    default: 255,
                    hidden: false,
                    description: Docs.fhBroadcastDwellTime.description,
                    longDescription: Docs.fhBroadcastDwellTime.longDescription
                },
                {
            		displayName: "Advanced",
            		config: [
		                {
		                    name: "customMinTxOff",
		                    displayName: "Custom Minimum TX Off-Time",
		                    default: false,
		                    hidden: true,
		                    description: Docs.custMinTxOff.description,
		                    longDescription: Docs.custMinTxOff.longDescription,
		                    onChange: (inst, ui) => setAdvancedMacConfigs(inst,
		                        ui, "customMinTxOff")
		                },
		                {
		                    name: "minTxOff",
		                    displayName: "Minimum TX Off-Time (ms)",
		                    default: getDefaultMinTxOff(null),
		                    hidden: true,
		                    description: Docs.minTxOff.description,
		                    longDescription: Docs.minTxOff.longDescription
		                },
		                {
		                    name: "dutyCycleEnable",
		                    displayName: "Enable Duty Cycle",
		                    default: false,
		                    hidden: true,
		                    description: Docs.dcEnable.description,
		                    longDescription: Docs.dcEnable.longDescription,
		                    onChange: (inst, ui) => setAdvancedMacConfigs(inst,
		                        ui, "dutyCycleEnable")
		                },
		                {
		                    name: "dutyCycle",
		                    displayName: "Duty Cycle Threshold (%)",
		                    default: getDefaultDutyCycle(null),
		                    hidden: true,
		                    description: Docs.dutyCycle.description,
		                    longDescription: Docs.dutyCycle.longDescription
		                }
            		]
        		}
            ]
        }
    ]
};

/*
 *******************************************************************************
 Network Group Config Functions
 *******************************************************************************
 */


 /*!
 * ======== getDefaultMinTxOff ========
 * Returns the default minTxOff config based on PHY requirement:
 *   - ARIB: 2 ms
 *   - ETSI: 100 ms
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @returns - default min TX off
 */
function getDefaultMinTxOff(inst)
{
    let minTxOff = 0;
    if(inst !== null)
    {
        if(inst.region === "JP")
        {
            minTxOff = 2;
        }
        else if(inst.region === "EU")
        {
            minTxOff = 100;
        }
    }
    else
    {
        minTxOff = 0;
    }

    return(minTxOff);
}

/*!
 * ======== getDefaultDutyCycle ========
 * Returns the default duty cycle threshold based on the mode
 *  - FH (multiple channels): 20%
 *  - Else: 10%
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @returns - default duty cycle
 */
function getDefaultDutyCycle(inst)
{
    let dutyCycle = 0;
    if(inst !== null && inst.mode === "frequencyHopping"
        && inst.fhAsyncChannels.length > 1)
    {
        dutyCycle = 20;
    }
    else
    {
        dutyCycle = 10;
    }

    return(dutyCycle);
}

/*!
 * ======== getDefaultNetname ========
 * Returns the default network name depending on the current project.
 *
 * @param inst - module instance (null during initialization)
 * @returns - Network name
 */
function getDefaultNetname(inst)
{
    let netname = "Wi-SUN Network";
    if (inst != null && inst.project.includes("solar"))
    {
        netname = "Solar Network"
    }
    return netname;
}

/*!
 * ======== getDefaultChannelMask ========
 * Returns the default channel mask config based on default frequency band.
 * Default channel mask is first four supported channels
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @returns - default channel mask config
 */
function getDefaultChannelMask(inst)
{
    const channels = getSupportedChannels(inst);

    if(inst != null)
    {
        if(inst.mode.includes("fixedChannel"))
        {
            const defaultChannels = channels.slice(0, 1);

            // Return list of channels as strings
            return(defaultChannels.map(String));
        }
    }

    const defaultChannels = channels;

    // Return list of channels as strings
    return(defaultChannels.map(String));
}
/*!
 * ======== getDefaultChannelMask ========
 * Returns the default channel mask config based on default frequency band.
 * Default channel mask is first four supported channels
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @returns - default channel mask config
 */
function getDefaultChannelMaskCustom(inst)
{
    const totalNumChannels = inst.totalChannels;
    const channels = getSupportedChannels(inst);
    const defaultChannels = channels.slice(0, totalNumChannels);

    // Return list of channels as strings
    return(defaultChannels.map(String));
}

/*!
 *  ======== selectAllOptions ========
 *  Returns array with all values from provided drop down options array
 *
 * @param options  - drop down options array
 * @returns        - array with all values from options array
 */
function selectAllOptions(options)
{
    return(_.map(options, "name"));
}

/*!
 * ======== setDefaultNetname ========
 * Sets the default network name
 *
 * @param inst - module instance
 */
function setDefaultNetname(inst)
{
    inst.fhNetname = getDefaultNetname(inst);
}

/*!
 * ======== setDefaultDwellTime ========
 * Sets the default network dwell times for solar projects
 *
 * @param inst - module instance
 */
function setDefaultDwellTime(inst)
{
    if (inst != null && inst.project.includes("solar"))
    {
        inst.fhBroadcastInterval = 300;
        inst.fhBroadcastDwellTime = 250;
    }
}

/*!
 * ======== setDefaultChannelMasks ========
 * Sets the default channel mask config based on default frequency band
 * (sub-1 GHz if supported)
 *
 * @param inst - 15.4 module instance (null during initialization)
 */
function setDefaultChannelMasks(inst)
{
    if(inst.customPhy == false)
    {
        // Set default channel mask for regular channel mask config
        inst.unicastChannelMask = getDefaultChannelMask(inst);
        inst.broadcastChannelMask = getDefaultChannelMask(inst);
        inst.fhAsyncChannels = getDefaultChannelMask(inst);
    }
    else
    {
        inst.unicastChannelMask = getDefaultChannelMaskCustom(inst);
        inst.broadcastChannelMask = getDefaultChannelMaskCustom(inst);
        inst.fhAsyncChannels = getDefaultChannelMaskCustom(inst);
    }
}

/*
 * ======== getSupportedChannels ========
 * Generate and return array of channels currently supported based on
 * frequency band and phy type currently selected
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @returns array - array of ints
 */
function getSupportedChannels(inst)
{
    let range = [];
    if(inst != null)
    {
        range = rfCommon.getChannelMask(inst.region, inst.phyModeID, inst.ChanPlanID);
    }
    else
    {
        const defaultList = rfCommon.getDefaults();
        // Default case at module initialization (when inst is null)
        // Default PHY is 866 MHZ 50kbps
        range = rfCommon.getChannelMask(defaultList[0], defaultList[2], defaultList[3]);
    }

    return(range);
}

/*
 * ======== getChannelOptions ========
 * Generate and return options array for channel configs drop down menu
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @param inst - Boolean. True if function called to populate legacy configs
 * @returns array - array of name and display name objects for each channel
 */
function getChannelOptions(inst, isLegacyConfig)
{
    const options = [];

    // Get largest subset of channels allowed by device
    const allowedRange = getSupportedChannels(inst);

    // Create an array of drop down options for channel configs
    _.each(allowedRange, (channel) =>
    {
        // Dynamically populated configs only accept arrays of strings, not ints
        // (as used in legacy configs)
        let optionName;
        if(!isLegacyConfig)
        {
            optionName = String(channel);
        }
        else
        {
            optionName = channel;
        }

        options.push({
            name: optionName,
            displayName: `Channel ${channel}`
        });
    });

    return(options);
}


/*
 * ======== setAdvancedMacConfigs ========
 * Update minTxOff and dutyCycle configs value and visibility
 *
 * @param inst    - module instance containing the config that changed
 * @param ui      - user interface object
 * @param changedConfig - config that changed and called this function
 */
function setAdvancedMacConfigs(inst, ui, changedConfig)
{
    if(changedConfig === "dutyCycleEnable"
        || changedConfig === "region")
    {
        inst.dutyCycle = getDefaultDutyCycle(inst);
    }

    if(changedConfig === "customMinTxOff"
        || changedConfig === "region")
    {
        inst.minTxOff = getDefaultMinTxOff(inst);
    }

    // Get list of all configs in Advanced MAC group
    const macConfigArray = _.filter(config.config,
        (o) => o.displayName === "MAC");
    const advMacConfigArray = _.filter(macConfigArray[0].config,
        (o) => o.displayName === "Advanced");
    const advMacConfigs = Common.findAllConfigs(advMacConfigArray);

    for(const cfg of advMacConfigs)
    {
        /* Calling setXConfigHiddenState sets a config to its default value
         * if it is hidden. An onChange handler can never change itself
         */
        if(cfg !== changedConfig)
        {
            setNetworkConfigHiddenState(inst, ui, cfg);
        }
    }
}


/*
 * ======== getNetworkConfigHiddenState ========
 * Get the expected visibility of the selected config
 *
 * @param inst    - module instance containing the config that changed
 * @param cfgName - name of config
 * @returns bool  - true if hidden, false if visible
 */
function getNetworkConfigHiddenState(inst, cfgName)
{
    const freqHoppingSelected = true;
    const beaconModeSelected = (inst.mode === "beacon");
    const isCollectorProject = (inst.project.includes("borderrouter"));
    const isSensorProject = (inst.project.includes("routernode") || inst.project.includes("coapnode"));
    let isVisible = true;
    switch(cfgName)
    {
        case "channelMask":
        case "fhChannelMask":
        case "fhAsyncChannelMask":
        {
            // Legacy channel mask configs that should always remain hidden
            isVisible = false;
            break;
        }
        case "channels":
        {
            isVisible = !freqHoppingSelected;
            break;
        }
        case "fhNetname":
        case "unicastChannelMask":
        case "fhAsyncChannels":
        {
            isVisible = freqHoppingSelected;
            break;
        }
        case "fhBroadcastInterval":
        case "broadcastChannelMask":
        case "fhBroadcastDwellTime":
        {
            isVisible = isCollectorProject && freqHoppingSelected;
            break;
        }
        case "orphanBackoffInterval":
        case "scanBackoffInterval":
        {
            isVisible = false;
            break;
        }
        case "trackingDelayTime":
        case "maxDevices":
        {
            isVisible = false;
            break;
        }
        case "pollingInterval":
        {
            isVisible = false;
            break;
        }
        case "customMinTxOff":
        {
            isVisible = inst.region === "JP"
                || inst.region === "EU";
            break;
        }
        case "dutyCycleEnable":
        {
            isVisible = inst.region === "JP"
                || inst.region === "EU";
            break;
        }
        case "dutyCycle":
        {
            isVisible = inst.dutyCycleEnable;
            break;
        }
        case "minTxOff":
        {
            isVisible = inst.customMinTxOff;
            break;
        }

        case "reportingInterval":
        case "panID":
        case "scanDuration":
        case "macBeaconOrder":
        case "macSuperframeOrder":
        case "minBe":
        case "maxBe":
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
 * ======== setNetworkConfigHiddenState ========
 * Sets the visibility of the selected config
 *
 * @param inst    - module instance containing the config that changed
 * @param ui      - user interface object
 * @param cfgName - name of config
 */
function setNetworkConfigHiddenState(inst, ui, cfgName)
{
    // Set visibility of config
    ui[cfgName].hidden = getNetworkConfigHiddenState(inst, cfgName);

    if(ui[cfgName].hidden)
    {
        // get a list of all nested and unnested configs
        const configToReset = Common.findConfig(config.config, cfgName);
        // restore the default value for the hidden parameter.
        Common.restoreDefaultValue(inst, configToReset, cfgName);
    }
}

function getNetworkConfigReadonlyState(inst, cfgName)
{
    if (cfgName === "panID")
    {
        if (inst.project.includes("borderrouter"))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    return false;
}

function setNetworkConfigReadonlyState(inst, ui, cfgName)
{
    ui[cfgName].readOnly = getNetworkConfigReadonlyState(inst, cfgName);
}

/*
 * ======== validateOneChannelSelected ========
 * Validate that at least one channel is selected
 *
 * @param inst       - Network settings instance to be validated
 * @param validation - object to hold detected validation issues
 * @param cfgName    - name of channel mask config to be validated
 */
function validateOneChannelSelected(inst, validation, cfgName)
{
    // Verify that at least one channel is selected (if config not hidden)
    if((!getNetworkConfigHiddenState(inst, cfgName))
        && (inst[cfgName].length === 0))
    {
        validation.logError("Must select at least one channel", inst, cfgName);
    }
}

/*
 * ======== validateOrder ========
 * Validate the beacon or superframe order for beacon mode
 *
 * @param inst       - Network settings instance to be validated
 * @param validation - object to hold detected validation issues
 * @param cfgName    - name of beacon/superframe order config to be validated
 */
function validateOrder(inst, validation, cfgName)
{
    if(inst.mode === "beacon")
    {
        Common.validateRangeInt(inst, validation, cfgName, 1, 14);
        if(inst[cfgName] !== 8)
        {
            validation.logInfo("Optimal value is 8", inst, cfgName);
        }
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
    // Validate PAN ID range -- always visible
    Common.validateRangeHex(inst, validation, "panID", 0, 0xffff);

    // Validate dynamic channel configs
    const validOptions = getChannelOptions(inst, false);

    if(!getNetworkConfigHiddenState(inst, "channels"))
    {
        // Verify that at least one channel is selected
        validateOneChannelSelected(inst, validation, "channels");
        Common.validateDynamicMultiEnum(inst, validation, "channels",
            inst.channels, validOptions);
    }

    if(!getNetworkConfigHiddenState(inst, "unicastChannelMask"))
    {
        // Verify that at least one channel is selected
        validateOneChannelSelected(inst, validation, "unicastChannelMask");
        Common.validateDynamicMultiEnum(inst, validation, "unicastChannelMask",
            inst.unicastChannelMask, validOptions);
    }

    if(!getNetworkConfigHiddenState(inst, "broadcastChannelMask"))
    {
        // Verify that at least one channel is selected
        validateOneChannelSelected(inst, validation, "broadcastChannelMask");
        Common.validateDynamicMultiEnum(inst, validation, "broadcastChannelMask",
            inst.broadcastChannelMask, validOptions);
    }

    if(!getNetworkConfigHiddenState(inst, "fhAsyncChannels"))
    {
        // Verify that at least one channel is selected
        validateOneChannelSelected(inst, validation, "fhAsyncChannels");
        Common.validateDynamicMultiEnum(inst, validation, "fhAsyncChannels",
            inst.fhAsyncChannels, validOptions);
    }

    // Validate FH net name if not hidden
    if(!getNetworkConfigHiddenState(inst, "fhNetname"))
    {
        if(inst.fhNetname.length >= 32)
        {
            validation.logError("Must be less than 32 characters", inst,
                "fhNetname");
        }
    }

    // Validate max devices config if not hidden
    if(!getNetworkConfigHiddenState(inst, "maxDevices"))
    {
        // Validate max devices range
        Common.validateRangeInt(inst, validation, "maxDevices", 0,
            Common.cTypeMax.int16_t);

        // Add info msgs if max devices value updated from default
        const maxDevicesDefault = Common.findConfig(config.config,
            "maxDevices").default;

        if(inst.maxDevices !== maxDevicesDefault)
        {
            validation.logInfo("The selected value requires "
            + "MAX_DEVICE_TABLE_ENTRIES to be updated in collector.opts",
            inst, "maxDevices");
        }

        if(inst.maxDevices > 150)
        {
            validation.logInfo("It is not recommended that this value be set "
                + "above 150", inst, "maxDevices");
        }
    }

    // Validate scan backoff interval ranges
    if(!getNetworkConfigHiddenState(inst, "scanBackoffInterval"))
    {
        Common.validateRangeInt(inst, validation, "scanBackoffInterval", 0,
            Common.cTypeMax.u_int32);
    }

    // Validate orphan backoff interval ranges
    if(!getNetworkConfigHiddenState(inst, "orphanBackoffInterval"))
    {
        Common.validateRangeInt(inst, validation, "orphanBackoffInterval", 0,
            Common.cTypeMax.u_int32);
    }

    // Validate FH unicast interval time range if not hidden
    if(!getNetworkConfigHiddenState(inst, "unicastDwellTime"))
    {
        Common.validateRangeInt(inst, validation, "unicastDwellTime", 0,
            Common.cTypeMax.u_int32);
    }

    // Validate FH broadcast interval time range if not hidden
    if(!getNetworkConfigHiddenState(inst, "fhBroadcastInterval"))
    {
        Common.validateRangeInt(inst, validation, "fhBroadcastInterval", 0,
            Common.cTypeMax.u_int32);
    }

    // Validate FH broadcast dwell time range if not hidden
    if(!getNetworkConfigHiddenState(inst, "fhBroadcastDwellTime"))
    {
        Common.validateRangeInt(inst, validation, "fhBroadcastDwellTime", 0,
            Common.cTypeMax.u_int8);
    }

    // Validate FH broadcast dwell time range if not hidden
    if(!getNetworkConfigHiddenState(inst, "fhBroadcastDwellTime"))
    {
        Common.validateRangeInt(inst, validation, "fhBroadcastDwellTime", 0,
            Common.cTypeMax.u_int8);
    }

    // Validate FH broadcast dwell time range if not hidden
    if(!getNetworkConfigHiddenState(inst, "fhBroadcastDwellTime"))
    {
        Common.validateRangeInt(inst, validation, "fhBroadcastDwellTime", 0,
            Common.cTypeMax.u_int8);
    }

    if (getNetworkConfigReadonlyState(inst, "panID"))
    {
        validation.logInfo("Router device PAN ID is updated based on " +
            "joined network.", inst, "panID");
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
    validate: validate,
    setNetworkConfigReadonlyState: setNetworkConfigReadonlyState,
    setNetworkConfigHiddenState: setNetworkConfigHiddenState,
    getNetworkConfigHiddenState: getNetworkConfigHiddenState,
    setDefaultChannelMasks: setDefaultChannelMasks,
    setDefaultNetname: setDefaultNetname,
    setDefaultDwellTime: setDefaultDwellTime,
    getChannelOptions: getChannelOptions,
    setAdvancedMacConfigs: setAdvancedMacConfigs
};
