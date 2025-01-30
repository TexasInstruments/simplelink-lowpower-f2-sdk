/*
 * Copyright (c) 2018, Texas Instruments Incorporated
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
 */

/*
 *  ======== zstack_rf.syscfg.js ========
 */

"use strict";

// Get RF command handler
const CmdHandler = system.getScript("/ti/devices/radioconfig/cmd_handler.js");

// Get radio config module functions
const RadioConfig = system.getScript(
    "/ti/devices/radioconfig/radioconfig_common.js"
);

const RadioConfigCommon = system.getScript("/ti/devices/radioconfig/radioconfig_common.js");

// Get RF design functions
const RfDesign = system.getScript("/ti/devices/radioconfig/rfdesign");

// Get common utility functions
const Common = system.getScript("/ti/zstack/zstack_common.js");

// Get RF setting descriptions
const Docs = system.getScript("/ti/ti154stack/rf_config/"
    + "ti154stack_rf_config_docs.js");

// Get common utility functions
const TI154Common = system.getScript("/ti/ti154stack/ti154stack_common.js");

const deviceId = system.deviceData.deviceId;

/* Structure for Coex config */
const coexConfig = {};

/* Array of level options */
const coexLevelOptions = [
    {
        name: 0,
        displayName: "Low"
    },
    {
        name: 1,
        displayName: "High"
    }
];

/* Map option to enum name */
const priorityEnumLookup = {
    0: "RF_PriorityCoexLow",
    1: "RF_PriorityCoexHigh"
};
const requestEnumLookup = {
    0: "RF_RequestCoexNoAssertRx",
    1: "RF_RequestCoexAssertRx"
};


/* Description text for configurables */
const primaryChannelsLongDescription = `The primary IEEE 802.15.4 frequency \
channels to use in network formation or joining.

During network formation, these channels are scanned to find the lowest noise \
channel to form a channel on. During joining, these channels are scanned for \
existing networks to join.

For more information, refer to the [ZigBee Configuration](/zigbee/html/\
sysconfig/zigbee.html#zigbee-configuration) section of the ZigBee User's \
Guide.

**Default:** Channel 11

**Range:** Any combination of channels 11-26`;

const secondaryChannelsLongDescription = `The secondary IEEE 802.15.4 \
frequency channels to use in network formation or joining.

This set of channels is only used after the primary channel set fails during \
network formation or joining.

During network formation, these channels are scanned to find the lowest noise \
channel to form a channel on. During joining, these channels are scanned for \
existing networks to join.

For more information, refer to the [ZigBee Configuration](/zigbee/html/\
sysconfig/zigbee.html#zigbee-configuration) section of the ZigBee User's \
Guide.

**Default:** No channel

**Range:** Any combination of channels 11-26`;

const txpowerDescription = `The default transmit power in dBm`;

const txpowerLongDescription = `The default transmit power in dBm

**Default Power:** 0`;

/* Frequency channel options for enumeration configurables */
const rfOptions = [
    {name: 11, displayName: "11 - 2405 MHz"},
    {name: 12, displayName: "12 - 2410 MHz"},
    {name: 13, displayName: "13 - 2415 MHz"},
    {name: 14, displayName: "14 - 2420 MHz"},
    {name: 15, displayName: "15 - 2425 MHz"},
    {name: 16, displayName: "16 - 2430 MHz"},
    {name: 17, displayName: "17 - 2435 MHz"},
    {name: 18, displayName: "18 - 2440 MHz"},
    {name: 19, displayName: "19 - 2445 MHz"},
    {name: 20, displayName: "20 - 2450 MHz"},
    {name: 21, displayName: "21 - 2455 MHz"},
    {name: 22, displayName: "22 - 2460 MHz"},
    {name: 23, displayName: "23 - 2465 MHz"},
    {name: 24, displayName: "24 - 2470 MHz"},
    {name: 25, displayName: "25 - 2475 MHz"},
    {name: 26, displayName: "26 - 2480 MHz"}
];

const phyDropDownOption = [{
    name: "phyIEEE",
    displayName: "250 kbps, IEEE 802.15.4"
}];

/* RF submodule for zstack module */
const rfModule = {
    config: [
        {
            name: "rfDesign",
            displayName: "Based On RF Design",
            options: getRfDesignOptions(),
            default: getRfDesignOptions()[0].name,
            description: Docs.rfDesign.description,
            longDescription: Docs.rfDesign.longDescription
        },
        {
            name: "primaryChannels",
            displayName: "Primary Channels",
            description: "The primary IEEE 802.15.4 frequency channels to use "
                         + "in network formation or joining.",
            longDescription: primaryChannelsLongDescription,
            default: [11],
            options: rfOptions,
            minSelections: 0
        },
        {
            name: "secondaryChannels",
            displayName: "Secondary Channels",
            description: "The secondary IEEE 802.15.4 frequency channels to "
                         + "use in network formation or joining. Used only "
                         + "after primary channel fails formation or joining.",
            longDescription: secondaryChannelsLongDescription,
            default: [11],
            options: rfOptions,
            minSelections: 0
        },
        {
            name: "freqBand",
            displayName: "Frequency Band",
            options: getFrequencyBandOptions(),
            default: getDefaultFreqBand(),
            description: Docs.freqBand.description,
            longDescription: Docs.freqBand.longDescription,
            readOnly: true
        },
        {
            name: "phyType",
            displayName: "Phy Type",
            options: phyDropDownOption,
            default: "phyIEEE",
            hidden: false,
            description: Docs.phyType.description,
            longDescription: Docs.phyType.longDescription
        },
        {
            name: "phyID",
            displayName: "Phy ID",
            default: "APIMAC_PHY_ID_NONE",
            readOnly: Docs.phyID.readOnly,
            description: Docs.phyID.description,
            longDescription: Docs.phyID.longDescription,
            hidden: true
        },
        {
            name: "txPower",
            displayName: "Transmit Power",
            description: txpowerDescription,
            longDescription: txpowerLongDescription,
            options: (inst) => getTxPowerConfigOptions(inst),
            default: "0"
        },
        {
            name: "coexMode",
            displayName: Docs.coex.enable.displayName,
            default: "coexModeDisabled",
            description: Docs.coex.enable.description,
            longDescription: Docs.coex.enable.longDescription,
            getDisabledOptions: getDisabledCoexModeOptions(),
            options: [
                {
                    name: "coexModeDisabled",
                    displayName: "Disabled"
                },
                {
                    name: "coexMode2Wire",
                    displayName: "2-Wire"
                },
                {
                    name: "coexMode3Wire",
                    displayName: "3-Wire"
                }
            ],
            onChange: onCoexEnableChange
        },
        {
            name: "coexUseCaseConfigGroup",
            displayName: Docs.coex.useCaseConfigGroupIeee.displayName,
            collapsed: true,
            config: [
                {
                    name: "ieeeIniTxGroup",
                    displayName:
                        Docs.coex.useCaseConfigGroupIeee.iniTx.displayName,
                    collapsed: false,
                    config: [
                        {
                            name: "ieeeIniTxDefaultPriority",
                            displayName: Docs.coex.defaultPriority.displayName,
                            description: Docs.coex.defaultPriority.description,
                            longDescription:
                                Docs.coex.defaultPriority.longDescription,
                            hidden: true,
                            default: 0,
                            options: coexLevelOptions,
                            onChange: updateCoexConfig
                        },
                        {
                            name: "ieeeIniTxAssertRequestForRx",
                            displayName:
                                Docs.coex.assertRequestForRx.displayName,
                            description:
                                Docs.coex.assertRequestForRx.description,
                            longDescription:
                                Docs.coex.assertRequestForRx.longDescription,
                            hidden: true,
                            default: true,
                            onChange: updateCoexConfig
                        }
                    ]
                },
                {
                    name: "ieeeConTxGroup",
                    displayName:
                        Docs.coex.useCaseConfigGroupIeee.conTx.displayName,
                    collapsed: false,
                    config: [
                        {
                            name: "ieeeConTxDefaultPriority",
                            displayName: Docs.coex.defaultPriority.displayName,
                            description: Docs.coex.defaultPriority.description,
                            longDescription:
                                Docs.coex.defaultPriority.longDescription,
                            hidden: true,
                            default: 0,
                            options: coexLevelOptions,
                            onChange: updateCoexConfig
                        },
                        {
                            name: "ieeeConTxAssertRequestForRx",
                            displayName:
                                Docs.coex.assertRequestForRx.displayName,
                            description:
                                Docs.coex.assertRequestForRx.description,
                            longDescription:
                                Docs.coex.assertRequestForRx.longDescription,
                            hidden: true,
                            default: true,
                            onChange: updateCoexConfig
                        }
                    ]
                },
                {
                    name: "ieeeIniRxGroup",
                    displayName:
                        Docs.coex.useCaseConfigGroupIeee.iniRx.displayName,
                    collapsed: false,
                    config: [
                        {
                            name: "ieeeIniRxDefaultPriority",
                            displayName: Docs.coex.defaultPriority.displayName,
                            description: Docs.coex.defaultPriority.description,
                            longDescription:
                                Docs.coex.defaultPriority.longDescription,
                            hidden: true,
                            default: 0,
                            options: coexLevelOptions,
                            onChange: updateCoexConfig
                        },
                        {
                            name: "ieeeIniRxAssertRequestForRx",
                            displayName:
                                Docs.coex.assertRequestForRx.displayName,
                            description:
                                Docs.coex.assertRequestForRx.description,
                            longDescription:
                                Docs.coex.assertRequestForRx.longDescription,
                            hidden: true,
                            default: true,
                            onChange: updateCoexConfig
                        }
                    ]
                },
                {
                    name: "ieeeConRxGroup",
                    displayName:
                        Docs.coex.useCaseConfigGroupIeee.conRx.displayName,
                    collapsed: false,
                    config: [
                        {
                            name: "ieeeConRxDefaultPriority",
                            displayName: Docs.coex.defaultPriority.displayName,
                            description: Docs.coex.defaultPriority.description,
                            longDescription:
                                Docs.coex.defaultPriority.longDescription,
                            hidden: true,
                            default: 0,
                            options: coexLevelOptions,
                            onChange: updateCoexConfig
                        },
                        {
                            name: "ieeeConRxAssertRequestForRx",
                            displayName:
                                Docs.coex.assertRequestForRx.displayName,
                            description:
                                Docs.coex.assertRequestForRx.description,
                            longDescription:
                                Docs.coex.assertRequestForRx.longDescription,
                            hidden: true,
                            default: true,
                            onChange: updateCoexConfig
                        }
                    ]
                }
            ]
        }
    ],
    validate: validate,
    moduleInstances: moduleInstances,
    getCoexConfigIeee: function() {
        return coexConfig;
    },
};

/* RadioConfig module definition and default configurations */
function moduleInstances(inst)
{
    const dependencyModule = [];

    let phySettings = Common.getBoardPhySettings(inst).defaultIEEEPhyList;

    // Get settings from selected phy
    const radioConfigArgs = _.cloneDeep(phySettings[0].args);


    if(deviceId.match(/CC1352P|CC265[12]P/))
    {
        if ( !isNaN(inst.txPower) )
        {
            // High PA Tx Power values start at 6dBm
            if ( parseInt(inst.txPower) > 5 )
            {
                radioConfigArgs.highPA = true;
                radioConfigArgs["txPowerHi"] = (inst.txPower);
            }
            else
            {
                radioConfigArgs.highPA = false;
                radioConfigArgs["txPower"] = (inst.txPower);
            }
        }
        else
        {
            radioConfigArgs.highPA = false;
        }
        radioConfigArgs.codeExportConfig.paExport = "combined";
    }
    else if(deviceId.match(/CC1354P|CC2653P|CC2654P/))
    {
        // Devices do not export HighPA right now
        radioConfigArgs.codeExportConfig.paExport = "active";
    }
    else
    {
        radioConfigArgs.codeExportConfig.paExport = "active";
    }

    const radioConfigModule = {
        name: "radioConfig",
        displayName: "Advanced Radio Settings",
        moduleName: "/ti/devices/radioconfig/settings/ieee_15_4",
        readOnly: true,
        collapsed: true,
        args: radioConfigArgs
    };

    dependencyModule.push(radioConfigModule);

    dependencyModule.push({
        name: "coexSettings",
        moduleName: "/ti/zstack/rf/zstack_coex_mod.js",
        group: "coexUseCaseConfigGroup"
    });

    return(dependencyModule);
}

/*
 *  ======== getRfDesignOptions ========
 *  Generates an array of SRFStudio compatible rfDesign options based on device
 *
 * @param deviceId - device being used
 *
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
            {name: "LAUNCHXL-CC1352P-2"},
            {name: "LAUNCHXL-CC1352P-4"}
        ];
    }
    else if(deviceId === "CC1352R1F3RGZ")
    {
        newRfDesignOptions = [{name: "LAUNCHXL-CC1352R1"}];
    }
    else if(deviceId === "CC2652R1FRGZ")
    {
        newRfDesignOptions = [{name: "LAUNCHXL-CC26X2R1"}];
    }
    else if(deviceId === "CC2652RB1FRGZ")
    {
        newRfDesignOptions = [{name: "LP_CC2652RB"}];
    }
    else if(deviceId === "CC2652R1FSIP")
    {
        newRfDesignOptions = [{name: "LP_CC2652RSIP"}];
    }
    else if(deviceId === "CC2652P1FSIP")
    {
        newRfDesignOptions = [{name: "LP_CC2652PSIP"}];
    }
    else if(deviceId === "CC2652R7RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC2652R7"}];
    }
    else if(deviceId === "CC1352P7RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC1352P7-4"}];
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
    else if(deviceId === "CC1354P10RSK")
    {
        newRfDesignOptions = [
            {name: "LP_EM_CC1354P10_1"},
            {name: "LP_EM_CC1354P10_6"}
        ];
    }
    else if(deviceId === "CC2653P10RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC2653P10"}];
    }
    else if(deviceId === "CC1314R10RSK")
    {
        newRfDesignOptions = [{name: "LP_EM_CC1314R10"}];
    }
    else if(deviceId === "CC2653P10RSK")
    {
        newRfDesignOptions = [{name: "LP_EM_CC2653P10"}];
    }
    else if(deviceId === "CC2674R10RSK")
    {
        newRfDesignOptions = [{name: "LP_EM_CC2674P10_RSK"}];
    }
    else if(deviceId === "CC2674R10RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC2674R10_RGZ"}];
    }
    else if(deviceId === "CC2674P10RSK")
    {
        newRfDesignOptions = [{name: "LP_EM_CC2674P10_RSK"}];
    }
    else if(deviceId === "CC2674P10RGZ")
    {
        newRfDesignOptions = [{name: "LP_EM_CC2674P10"}];
    }
    else if(deviceId == "CC1354P10RGZ"){
        newRfDesignOptions = [{name: "LP_CC1354P10_1_RGZ"}];
    }
    else if(deviceId == "CC1354R10RGZ"){
        newRfDesignOptions = [{name: "LP_CC1354R10_RGZ"}];
    }
    else
    {
        throw new Error("Unknown deviceId " + deviceId + ".");
    }
    
    return(newRfDesignOptions);
}


/*
 *  ======== getFrequencyBandOptions ========
 *  Gets the array of frequency bands supported by the board/devices
 *
 *  @returns Array - an array containing one or more dictionaries with the
 *                   following keys: displayName, name
 */
function getFrequencyBandOptions()
{
    const freqBandOptions = [];

    freqBandOptions.push(
        {
            name: "freqBand24",
            displayName: "2.4 GHz"
        }
    );

    return freqBandOptions;
}

/*
 *  ======== getDefaultFreqBand ========
 *  Retrieves the default frequency band:
 *      * 2.4 GHz for 26X2 boards
 *      * Sub-1 GHz for others
 *
 *  @returns - name of default frequency band
 */
function getDefaultFreqBand()
{
    const defaultFreqBand = "freqBand24";

    return defaultFreqBand;
}

/* Validation function for the RF submodule */
function validate(inst, validation)
{
    if(((inst.freqBand === "freqBandSub1") && (inst.phyType === "phyIEEE"))
        || ((inst.freqBand === "freqBand24") && (inst.phyType !== "phyIEEE")))
    {
        validation.logError("Phy type selected not supported by"
        + "this frequency band");
    }

    // Get the RF Design module
    const rfDesign = system.modules["/ti/devices/radioconfig/rfdesign"].$static;

    if(rfDesign.rfDesign === "LAUNCHXL-CC1352P1")
    {
        validation.logError(
            `This stack does not support this board configuration`,
            rfDesign, "rfDesign"
        );
    }

    if(inst.rfDesign !== rfDesign.rfDesign)
    {
        validation.logError(`Must match ${system.getReference(rfDesign,
            "rfDesign")} in the RF Design Module`, inst, "rfDesign");
    }

    const validOptions = getTxPowerConfigOptions(inst);
    TI154Common.validateDynamicEnum(inst, validation, "txPower", validOptions);

    // Get the RF module to verify that RF Coexistence configs match
    const rf = system.modules["/ti/drivers/RF"].$static;

    if(isCoexEnabled(inst) && inst.freqBand !== "freqBand24")
    {
        validation.logError(`RF coexistence only supported on 2.4 GHz`, inst,
            "coexMode");
    }
    else if((!isCoexEnabled(inst) && rf.coexEnable)
        || (isCoexEnabled(inst) && (!rf.coexEnable
        || rf.coexMode !== inst.coexMode)))
    {
        validation.logError(`Must match RF coexistence configuration in the \
            ${system.getReference(rf, "coexEnable")} module`, inst,
        "coexMode");
    }
}

/*!
 * ======== getTxPowerConfigOptions ========
 *
 * Get list of available Tx power values
 *
 * @returns a list of available transmit power options from the radio config
 */
function getTxPowerConfigOptions(inst)
{
    let txPowerValueList = [];
    let phyType = "ieee154";

    // Get the command handler for this phy instance
    if (inst.rfDesign == "LP_CC2652PSIP")
    {
        phyType = "ieee154p10";
    }
    const cmdHandler = CmdHandler.get(RadioConfig.PHY_IEEE_15_4, phyType);
    let freq = cmdHandler.getFrequency();

    // special case for P-4
    if (inst.rfDesign == "LAUNCHXL-CC1352P-4" ||
        inst.rfDesign == "LP_CC1352P7-4")
    {
      freq = 2499;
    }

    // TxPowerOptions with highPA disabled
    txPowerValueList = _.concat(
        txPowerValueList, RfDesign.getTxPowerOptions(freq, false)
    );

    if(deviceId.match(/CC135[24]P|CC265[124]P/))
    {
        txPowerValueList = _.concat(
            txPowerValueList, RfDesign.getTxPowerOptions(freq, true)
        );
    }

    // Round all tx power values
    _.forEach(txPowerValueList, (option) =>
    {
        option.name = _.round(option.name);
    });

    // Remove any duplicates
    txPowerValueList = _.uniqBy(txPowerValueList, "name");

    // Sort values in descending order
    txPowerValueList = _.orderBy(txPowerValueList, "name", "desc");

    // convert int array to string array
    _.forEach(txPowerValueList, (option) =>
    {
        option.name = option.name + "";
    });

    return(txPowerValueList);
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
        case "ieeeIniTxDefaultPriority":
        case "ieeeIniTxAssertRequestForRx":
        case "ieeeConTxDefaultPriority":
        case "ieeeConTxAssertRequestForRx":
        case "ieeeIniRxDefaultPriority":
        case "ieeeIniRxAssertRequestForRx":
        case "ieeeConRxDefaultPriority":
        case "ieeeConRxAssertRequestForRx":
        {
            isVisible = (inst.coexMode === "coexMode3Wire");
            break;
        }
        case "rfDesign":
        case "coexMode":
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
    TI154Common.setConfigHiddenState(inst, ui, cfgName, rfModule.config,
        getRFConfigHiddenState);

    // Separate case required for freqSub1 since default value depends
    // on board which can change at runtime via rfDesign
    if(cfgName === "freqSub1" && ui[cfgName].hidden)
    {
        inst.freqSub1 = getDefaultFreqSub1(inst);
    }
}

function onCoexEnableChange(inst, ui)
{
    const coexConfigs = Object.keys(ui).filter((key) => (key.includes("ieeeIni")
        || key.includes("ieeeCon")));

    coexConfigs.forEach((cfgName) =>
    {
        setRFConfigHiddenState(inst, ui, cfgName);
    });

    updateCoexConfig(inst);
}

function updateCoexConfig(inst)
{
    /* IEEE Use Case Config info */
    coexConfig.ieeeInitiatorTx = {
        defaultPriority: priorityEnumLookup[inst.ieeeIniTxDefaultPriority],
        assertRequestForRx:
            requestEnumLookup[Number(inst.ieeeIniTxAssertRequestForRx)]
    };
    coexConfig.ieeeConnectedTx = {
        defaultPriority: priorityEnumLookup[inst.ieeeConTxDefaultPriority],
        assertRequestForRx:
        requestEnumLookup[Number(inst.ieeeConTxAssertRequestForRx)]
    };
    coexConfig.ieeeInitiatorRx = {
        defaultPriority: priorityEnumLookup[inst.ieeeIniRxDefaultPriority],
        assertRequestForRx:
            requestEnumLookup[Number(inst.ieeeIniRxAssertRequestForRx)]
    };
    coexConfig.ieeeConnectedRx = {
        defaultPriority: priorityEnumLookup[inst.ieeeConRxDefaultPriority],
        assertRequestForRx:
        requestEnumLookup[Number(inst.ieeeConRxAssertRequestForRx)]
    };
}

/*
 *  ======== isCoexEnabled ========
 *  Check whether coexistence is enabled
 *
 * @returns Bool - true if coex enabled in Zstack module, false otherwise
 */
function isCoexEnabled(inst)
{
    return inst.coexMode !== "coexModeDisabled";
}

/*
 *  ======== getDisabledCoexModeOptions ========
 *  Generates a list of options that should be disabled in the coex
 *  drop-down
 *
 * @returns Array - array of strings that should be disabled
 */
function getDisabledCoexModeOptions()
{
    return(inst) =>
    {
        const disabledOptions = [];
        if(inst.freqBand !== "freqBand24")
        {
            const disableReason = "RF coexistence only supported on 2.4 GHz";
            disabledOptions.push({
                name: "coexMode2Wire",
                reason: disableReason
            });

            disabledOptions.push({
                name: "coexMode3Wire",
                reason: disableReason
            });
        }
        return(disabledOptions);
    };
}

exports = rfModule;
