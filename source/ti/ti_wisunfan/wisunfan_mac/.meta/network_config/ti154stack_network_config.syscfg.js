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
 *  ======== ti154stack_network_config.syscfg.js ========
 */

"use strict";

// Get common utility functions
const Common = system.getScript("/ti/ti154stack/ti154stack_common.js");

// Get network setting descriptions
const Docs = system.getScript("/ti/ti154stack/network_config/"
    + "ti154stack_network_config_docs.js");

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
            longDescription: Docs.channels.longDescription
        },
        {
            name: "fhChannels",
            displayName: "FH Channel Mask",
            default: selectAllOptions(getChannelOptions(null, false)),
            options: (inst) => getChannelOptions(inst, false),
            hidden: true,
            description: Docs.fhChannels.description,
            longDescription: Docs.fhChannels.longDescription
        },
        {
            name: "fhAsyncChannels",
            displayName: "FH Async Channel Mask",
            default: selectAllOptions(getChannelOptions(null, false)),
            options: (inst) => getChannelOptions(inst, false),
            hidden: true,
            description: Docs.fhAsyncChannels.description,
            longDescription: Docs.fhAsyncChannels.longDescription
        },
        {
            name: "fhNetname",
            displayName: "FH Network Name",
            default: "FHTest",
            hidden: true,
            description: Docs.fhNetname.description,
            longDescription: Docs.fhNetname.longDescription
        },
        {
            name: "maxDevices",
            displayName: "Max Devices",
            default: 50,
            description: Docs.maxDevices.description,
            longDescription: Docs.maxDevices.longDescription
        },
        {
            displayName: "Application",
            description: "Configure settings for application-level operations",
            config: [
                {
                    name: "reportingInterval",
                    displayName: "Reporting Interval (ms)",
                    default: 3000,
                    description: Docs.reportingInterval.description,
                    longDescription: Docs.reportingInterval.longDescription
                },
                {
                    name: "pollingInterval",
                    displayName: "Polling Interval (ms)",
                    default: 2000,
                    description: Docs.pollingInterval.description,
                    longDescription: Docs.pollingInterval.longDescription
                },
                {
                    name: "trackingDelayTime",
                    displayName: "Tracking Time Delay (ms)",
                    default: 5000,
                    description: Docs.trackingDelayTime.description,
                    longDescription: Docs.trackingDelayTime.longDescription
                },
                {
                    name: "scanBackoffInterval",
                    displayName: "Scan Back-off Interval (ms)",
                    default: 5000,
                    hidden: true,
                    description: Docs.scanBackoffInterval.description,
                    longDescription: Docs.scanBackoffInterval.longDescription
                },
                {
                    name: "orphanBackoffInterval",
                    displayName: "Orphan Back-off Interval (ms)",
                    default: 300000,
                    hidden: true,
                    description: Docs.orphanBackoffInterval.description,
                    longDescription: Docs.orphanBackoffInterval.longDescription
                },
                {
                    name: "scanDuration",
                    displayName: "Scan Duration (s)",
                    default: 5,
                    description: Docs.scanDuration.description,
                    longDescription: Docs.scanDuration.longDescription
                }
            ]
        },
        {
            displayName: "MAC",
            description: "Configure settings for MAC-level operations",
            config: [
                {
                    name: "macBeaconOrder",
                    displayName: "MAC Beacon Order",
                    default: 15,
                    description: Docs.macBeaconOrder.description,
                    longDescription: Docs.macBeaconOrder.longDescription,
                    readOnly: Docs.macBeaconOrder.readOnly
                },
                {
                    name: "macSuperframeOrder",
                    displayName: "MAC Super Frame Order",
                    default: 15,
                    description: Docs.macSuperframeOrder.description,
                    longDescription: Docs.macSuperframeOrder.longDescription,
                    readOnly: Docs.macSuperframeOrder.readOnly
                },
                {
                    name: "minBe",
                    displayName: "Min Back-off Exponent",
                    default: 3,
                    description: Docs.minBe.description,
                    longDescription: Docs.minBe.longDescription
                },
                {
                    name: "maxBe",
                    displayName: "Max Back-off Exponent",
                    default: 5,
                    description: Docs.maxBe.description,
                    longDescription: Docs.maxBe.longDescription
                },
                {
                    name: "fhBroadcastInterval",
                    displayName: "Broadcast Interval (ms)",
                    default: 10000,
                    hidden: true,
                    description: Docs.fhBroadcastInterval.description,
                    longDescription: Docs.fhBroadcastInterval.longDescription
                },
                {
                    name: "fhBroadcastDwellTime",
                    displayName: "Broadcast Dwell Time (ms)",
                    default: 100,
                    hidden: true,
                    description: Docs.fhBroadcastDwellTime.description,
                    longDescription: Docs.fhBroadcastDwellTime.longDescription
                }
            ]
        },
        /* Note: channelMask, fhChannelMask and fhAsyncChannelMask are legacy
         * configs. In order to seamlessly handle custom board changes at
         * runtime, the new channel configs (channels, fhChannels,
         * fhAsyncChannels) are dynamically updated with the correct channel
         * options based on the user's selected RF reference board, frequency
         * band, and data-rate
         *
         * For backwards compatibility, any changes to legacy channel mask
         * configs (from legacy files) that are valid in the new configs will
         * trigger updates to the corresponding new config.
         */
        {
            name: "channelMask",
            displayName: "Legacy Channel Mask",
            default: selectAllOptions(getChannelOptions(null, true)),
            options: getChannelOptions(null, true),
            hidden: true,
            description: "Legacy configurable that should always be hidden",
            longDescription: "Legacy configurable that should always be hidden",
            onChange: (inst) => onLegacyChannelMaskChange(inst, "channelMask",
                "channels")
        },
        {
            name: "fhChannelMask",
            displayName: "Legacy FH Channel Mask",
            default: selectAllOptions(getChannelOptions(null, true)),
            options: getChannelOptions(null, true),
            hidden: true,
            description: "Legacy configurable that should always be hidden",
            longDescription: "Legacy configurable that should always be hidden",
            onChange: (inst) => onLegacyChannelMaskChange(inst, "fhChannelMask",
                "fhChannels")
        },
        {
            name: "fhAsyncChannelMask",
            displayName: "Legacy FH Async Channel Mask",
            default: selectAllOptions(getChannelOptions(null, true)),
            options: getChannelOptions(null, true),
            hidden: true,
            description: "Legacy configurable that should always be hidden",
            longDescription: "Legacy configurable that should always be hidden",
            onChange: (inst) => onLegacyChannelMaskChange(inst,
                "fhAsyncChannelMask", "fhAsyncChannels")
        }
    ]
};

/*
 *******************************************************************************
 Network Group Config Functions
 *******************************************************************************
 */

/*
 * ======== onLegacyChannelMaskChange ========
 * On change function to ensure backwards compatibility between former channel
 * mask configs (channelMask, fhChannelMask, fhAsyncChannelMask) and current,
 * dynamically updated channel masks (channels, fhChannels, fhAsyncChannels).
 *
 * New channel mask configs will only be updated if legacy channel setting is
 * valid.
 *
 * @param inst - 15.4 module instance
 * @param legacyCfgName - name of legacy config
 * @param inst - name of new config to which legacy config is mapped

 */
function onLegacyChannelMaskChange(inst, legacyCfgName, newCfgName)
{
    // Retrieve list of valid channels from new mask config
    const currChannelOptions = getChannelOptions(inst, true);

    // Accept only channel levels from legacy config that are valid
    const currLegacyChannels = inst[legacyCfgName];
    const validLegacyChannels = Common.validateDynamicMultiEnum(inst, null,
        null, currLegacyChannels, currChannelOptions).validOptsSelected;

    // Update current channel config with values from legacy config
    if(!_.isEmpty(validLegacyChannels))
    {
        inst[newCfgName] = _.map(validLegacyChannels, String);
    }
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
    const defaultChannels = channels.slice(0, 4);

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
 * ======== setDefaultChannelMasks ========
 * Sets the default channel mask config based on default frequency band
 * (sub-1 GHz if supported)
 *
 * @param inst - 15.4 module instance (null during initialization)
 */
function setDefaultChannelMasks(inst)
{
    // Set default channel mask for regular channel mask config
    inst.channels = getDefaultChannelMask(inst);

    // Select all channels supported for FH channel mask configs
    const allSupportedChannels = getChannelOptions(inst, false);
    inst.fhChannels = selectAllOptions(allSupportedChannels);
    inst.fhAsyncChannels = selectAllOptions(allSupportedChannels);
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

    if((inst == null && !Common.isSub1GHzDevice())
        || (inst != null && inst.freqBand === "freqBand24"))
    {
        range = _.range(11, 27); // Channels 11 - 26
    }
    else if(inst != null && inst.freqSub1 === "freq433")
    {
        range = _.range(0, 7); // Channels 0 - 6
    }
    else if(inst != null && inst.freqSub1 === "freq863")
    {
        if(inst.phyType === "phy5kbps" || inst.phyType === "phy50kbps")
        {
            range = _.range(0, 34); // Channels 0 - 33
        }
        else
        {
            range = _.range(0, 17); // Channels 0 - 16
        }
    }
    else if(inst != null && inst.freqSub1 === "freq915")
    {
        if(inst.phyType === "phy200kbps")
        {
            range = _.range(0, 64); // Channels 0 - 63
        }
        else
        {
            range = _.range(0, 129); // Channels 0 - 128
        }
    }
    else if(inst != null && inst.freqSub1 === "freq866"
        && inst.phyType === "phy100kbps")
    {
        range = _.range(0, 10); // Channels 0 - 9
    }
    else
    {
        // Default case at module initialization (when inst is null)
        // Default PHY is 866 MHZ 50kbps
        range = _.range(0, 19); // Channels 0 - 18
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
 * ======== setBeaconSuperFrameOrders ========
 * Update beacon and super frame order configs value and visibility based on
 * selected mode
 *
 * @param inst    - module instance containing the config that changed
 * @param ui      - user interface object
 */
function setBeaconSuperFrameOrders(inst, ui)
{
    if(inst.mode === "beacon")
    {
        ui.macBeaconOrder.readOnly = false;
        ui.macSuperframeOrder.readOnly = false;
        inst.macSuperframeOrder = 8;
        inst.macBeaconOrder = 8;
    }
    else
    {
        ui.macBeaconOrder.readOnly = Docs.macBeaconOrder.readOnly;
        ui.macSuperframeOrder.readOnly = Docs.macSuperframeOrder.readOnly;
        inst.macSuperframeOrder = 15;
        inst.macBeaconOrder = 15;
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
    const freqHoppingSelected = (inst.mode === "frequencyHopping");
    const beaconModeSelected = (inst.mode === "beacon");
    const isCollectorProject = (inst.project.includes("collector"));
    const isSensorProject = (inst.project.includes("sensor"));
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
        case "fhChannels":
        case "fhAsyncChannels":
        {
            isVisible = freqHoppingSelected;
            break;
        }
        case "fhBroadcastInterval":
        case "fhBroadcastDwellTime":
        {
            isVisible = isCollectorProject && freqHoppingSelected;
            break;
        }
        case "orphanBackoffInterval":
        case "scanBackoffInterval":
        {
            isVisible = isSensorProject && !freqHoppingSelected;
            break;
        }
        case "trackingDelayTime":
        case "maxDevices":
        {
            isVisible = isCollectorProject;
            break;
        }
        case "pollingInterval":
        {
            isVisible = !beaconModeSelected;
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
 * ======== validateBackOffExponents ========
 * Validate minBe and maxBe configs
 *
 * @param inst       - Network settings instance to be validated
 * @param validation - object to hold detected validation issues
 */
function validateBackOffExponents(inst, validation)
{
    if(inst.maxBe < inst.minBe)
    {
        validation.logError("Max must be more than min", inst,
            ["minBe", "maxBe"]);
    }

    Common.validateRangeInt(inst, validation, "minBe", 0,
        Common.cTypeMax.u_int8);
    Common.validateRangeInt(inst, validation, "maxBe", 0,
        Common.cTypeMax.u_int8);
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

    if(!getNetworkConfigHiddenState(inst, "fhChannels"))
    {
        // Verify that at least one channel is selected
        validateOneChannelSelected(inst, validation, "fhChannels");
        Common.validateDynamicMultiEnum(inst, validation, "fhChannels",
            inst.fhChannels, validOptions);
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

    // Validate reporting interval range -- always visible
    Common.validateRangeInt(inst, validation, "reportingInterval", 0,
        Common.cTypeMax.u_int32);

    // Add info msgs if reporting interval value below recommended value
    if(inst.reportingInterval < 500)
    {
        validation.logInfo("It is not recommended that this value be set below "
            + "500", inst, "reportingInterval");
    }

    // Validate polling interval range -- always visible
    Common.validateRangeInt(inst, validation, "pollingInterval", 0,
        Common.cTypeMax.u_int32);

    // Add info msgs if polling interval value below recommended value
    if(inst.phyID.includes("LRM"))
    {
        if(inst.pollingInterval < 500)
        {
            validation.logInfo("It is not recommended that this value be set "
            + "below 500 ms for a data rate of 5 kbps", inst,
            "pollingInterval");
        }
    }
    else if(inst.pollingInterval < 100)
    {
        validation.logInfo("It is not recommended that this value be set "
            + "below 100 ms for this data rate", inst, "pollingInterval");
    }

    // Validate scan backoff interval ranges
    if(!getNetworkConfigHiddenState(inst, "trackingDelayTime"))
    {
        Common.validateRangeInt(inst, validation, "trackingDelayTime", 0,
            Common.cTypeMax.u_int32);
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

    // Validate scan duration range -- always visible
    Common.validateRangeInt(inst, validation, "scanDuration", 0,
        Common.cTypeMax.u_int8);

    // Validate superframe and beacon order configs -- always visible
    validateOrder(inst, validation, "macBeaconOrder");
    validateOrder(inst, validation, "macSuperframeOrder");

    // Validate backoff exponent configs -- always visible
    validateBackOffExponents(inst, validation);

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
    selectAllOptions: selectAllOptions,
    setNetworkConfigHiddenState: setNetworkConfigHiddenState,
    getNetworkConfigHiddenState: getNetworkConfigHiddenState,
    setDefaultChannelMasks: setDefaultChannelMasks,
    setBeaconSuperFrameOrders: setBeaconSuperFrameOrders,
    getChannelOptions: getChannelOptions
};
