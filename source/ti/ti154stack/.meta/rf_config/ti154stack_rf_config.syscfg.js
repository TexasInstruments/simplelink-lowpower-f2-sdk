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
 *  ======== ti154stack_rf_config.syscfg.js ========
 */

"use strict";

// Get common utility functions
const Common = system.getScript("/ti/ti154stack/ti154stack_common.js");

// Get common rf settings
const rfCommon = system.getScript("/ti/ti154stack/rf_config/"
    + "ti154stack_rf_config_common.js");

// Get transmit power settings script
const powerScript = system.getScript("/ti/ti154stack/power_config/"
    + "ti154stack_power_config");

// Get network settings script
const networkScript = system.getScript("/ti/ti154stack/network_config/"
    + "ti154stack_network_config");

// Get network settings script
const oadScript = system.getScript("/ti/ti154stack/oad_config/"
    + "ti154stack_oad_config");

// Get RF setting descriptions
const Docs = system.getScript("/ti/ti154stack/rf_config/"
    + "ti154stack_rf_config_docs.js");

// Get RF command handler
const CmdHandler = system.getScript("/ti/devices/radioconfig/cmd_handler.js");

// Get common radio config module functions
const commonRadioConfig = system.getScript("/ti/devices/radioconfig/"
    + "radioconfig_common.js");

// Get radio config parameter handler
const ParameterHandler = system.getScript("/ti/devices/radioconfig/"
    + "parameter_handler.js");

// Get custom phy settings
const customObj = system.getScript("/ti/ti154stack/rf_config/"
    + "phyCustom.js");

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
            name: "freqBand",
            displayName: "Frequency Band",
            options: rfCommon.getFreqBandOptions,
            default: rfCommon.getDefaultFreqBand(Common.isSub1GHzDevice()),
            description: Docs.freqBand.description,
            longDescription: Docs.freqBand.longDescription,
            onChange: onFreqBandChange
        },
        {
            name: "phyRegulation",
            displayName: "Regulation Type",
            options: getPhyRegulationOptions,
            default: rfCommon.getDefaultPhyRegulation(null),
            description: Docs.regulationType.description,
            longDescription: Docs.regulationType.longDescription,
            onChange: onPhyRegulationChange
        },
        {
            name: "freqSub1",
            displayName: "Sub-1 GHz Frequency",
            options: rfCommon.getFreqSub1Options,
            default: rfCommon.getDefaultFreqSub1(null),
            hidden: !Common.isSub1GHzDevice(),
            description: Docs.freqSub1.description,
            longDescription: Docs.freqSub1.longDescription,
            onChange: onFreqSub1orPhyTypeChange
        },
        {
            name: "phyType",
            displayName: "Phy Type",
            options: rfCommon.getPhyTypeOptions,
            default: rfCommon.getDefaultPhyType(null),
            description: Docs.phyType.description,
            longDescription: Docs.phyType.longDescription,
            onChange: onFreqSub1orPhyTypeChange
        },
        /* Note: phyID and channelPage are deprecated configs. These configs
         * were always set to read only and thus were never truly configurable.
         */
        {
            name: "phyID",
            displayName: "Phy ID",
            default: "Deprecated",
            hidden: true,
            readOnly: Docs.phyID.readOnly,
            description: Docs.phyID.description,
            longDescription: Docs.phyID.longDescription
        },
        {
            name: "channelPage",
            displayName: "Channel Page",
            default: "Deprecated",
            hidden: true,
            readOnly: Docs.channelPage.readOnly,
            description: Docs.channelPage.description,
            longDescription: Docs.channelPage.longDescription
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
        },
        {
            name: "customPhy",
            displayName: "Custom Phy",
            default: false,
            hidden: false,
            description: Docs.customPhy.description,
            longDescription: Docs.customPhy.longDescription,
            onChange: onCustomPhyChange,
        },
        {
            name: "customFreqBand",
            displayName: "Custom Sub-1 GHz Frequency Band",
            options: rfCommon.getCustomFreqOptions,
            default: rfCommon.getDefaultFreqSub1(null),
            hidden: true,
            description: Docs.customFreqBand.description,
            longDescription: Docs.customFreqBand.longDescription,
            onChange:onCustomSettingsChange,
        },
        {
            name: "customCh0Freq",
            displayName: "Custom Channel 0 Center Frequency (MHz)",
            default: 903.000,
            hidden: true,
            description: Docs.customCh0Freq.description,
            longDescription: Docs.customCh0Freq.longDescription,
            onChange:onCustomSettingsChange,
        },
        {
            name: "customChSpacing",
            displayName: "Custom Channel Spacing (kHz)",
            default: 1600,
            hidden: true,
            description: Docs.customChSpacing.description,
            longDescription: Docs.customChSpacing.longDescription,
            onChange:onCustomSettingsChange,
        },
        {
            name: "customChNum",
            displayName: "Custom Total Channel Number",
            default: 16,
            hidden: true,
            description: Docs.customChNum.description,
            longDescription: Docs.customChNum.longDescription,
            onChange:onCustomSettingsChange,
        }
    ]
};

/*
 *******************************************************************************
 Radio Group-Specific Functions
 *******************************************************************************
 */

/*
 * ======== onCustomPhyChange ========
 * Checks if the custom Phy checkbox is checked and performs
 * UI changes accordingly
 */
 function onCustomPhyChange(inst, ui)
 {
    if(inst.customPhy === true)
    {
        inst.mode = "nonBeacon";
        inst.customFreqBand = rfCommon.getDefaultFreqSub1(inst);
        ui["phyRegulation"].readOnly = Docs.customPhy.readOnly;
        ui["freqSub1"].readOnly = Docs.customPhy.readOnly;
        ui["phyType"].readOnly = Docs.customPhy.readOnly;
        ui["mode"].readOnly = Docs.customPhy.readOnlyMode;

        ui["customFreqBand"].hidden = false;
        ui["customCh0Freq"].hidden = false;
        ui["customChSpacing"].hidden = false;
        ui["customChNum"].hidden = false;
        networkScript.setDefaultChannelMasks(inst, null);
    }
    else 
    {
        ui["phyRegulation"].readOnly = false;
        ui["freqSub1"].readOnly = false;
        ui["phyType"].readOnly = false;
        ui["mode"].readOnly = false;

        ui["customFreqBand"].hidden = true;
        ui["customCh0Freq"].hidden = true;
        ui["customChSpacing"].hidden = true;
        ui["customChNum"].hidden = true;
    }
    networkScript.setAdvancedMacConfigs(inst, ui, "customPhy");
}

// Common onChange function for all custom configurables
function onCustomSettingsChange(inst)
{
    // Update values of frequency dependent configs
    networkScript.setDefaultChannelMasks(inst);
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
    else if(deviceId === "CC2651R3SIPA")
    {
        newRfDesignOptions = [{name: "LP_CC2651R3SIPA"}];
    }
    else if(deviceId === "CC1352P7RGZ")
    {
        newRfDesignOptions = [
            {name: "LP_CC1352P7-1"},
            {name: "LP_CC1352P7-4"}
        ];
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
    else if(deviceId === "CC2652R7RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC2652R7"}];
    }
    else if(deviceId === "CC2674R10RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC2674R10_RGZ"}];
    }
    else if(deviceId === "CC2653P10RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC2653P10"}];
    }
    else if(deviceId === "CC1311R3RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC1311R3"}];
    }
    else if(deviceId === "CC1311P3RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC1311P3"}];
    }
    else if(deviceId === "CC2651R3RGZ")
    {
        newRfDesignOptions = [{name: "LP_CC2651R3"}];
    }
    else if(deviceId === "CC2651P3RGZ")
    {
        newRfDesignOptions = [
            {name: "LP_CC2651P3"},
            {name: "LP_CC2651P3, 10DBM"}
        ];
    }
    else if(deviceId === "CC2674R10RSK")
    {
        newRfDesignOptions = [{name: "LP_EM_CC2674R10_RSK"}];
    }
    else if(deviceId === "CC2674P10RGZ")
    {
        newRfDesignOptions = [{name: "LP_EM_CC2674P10"}];
    }
    else if(deviceId === "CC2674P10RSK")
    {
        newRfDesignOptions = [{name: "LP_EM_CC2674P10_RSK"}];
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
    if(inst.project !== "coprocessor")
    {
        // Update dependencies
        const isSubGSelected = (inst.freqBand === "freqBandSub1");

        inst.freqBand = rfCommon.getDefaultFreqBand(isSubGSelected);
        inst.freqSub1 = rfCommon.getDefaultFreqSub1(inst);
        onFreqBandChange(inst, ui);
    }
}

/*
 * ======== onFreqBandChange ========
 * On change function for freqBand config
 * Updates visibility and values of frequency band-dependent configs
 *
 * @param inst - 15.4 instance
 * @param ui   - user interface object
 */
function onFreqBandChange(inst, ui)
{
    // Set visibility of dependent configs
    setRFConfigHiddenState(inst, ui, "freqSub1");

    inst.phyRegulation = rfCommon.getDefaultPhyRegulation(inst);
    inst.phyType = rfCommon.getDefaultPhyType(inst);

    onFreqSub1orPhyTypeChange(inst);

    // Update values of frequency dependent configs
    oadScript.setDefaultOADBlockSize(inst, inst.freqBand);
}

/*
 * ======== onPhyRegulationChange ========
 * On change function for phy regulation config
 * Updates visibility and value of minTxOff and dutyCycle configs
 *
 * @param inst - 15.4 instance
 * @param ui   - user interface object
 */
function onPhyRegulationChange(inst, ui)
{
    // Update visibility and value of dependent configs
    networkScript.setAdvancedMacConfigs(inst, ui, "phyRegulation");
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
    // For backwards compatability update PHY regulation
    // This handles the case in which an old .syscfg file (from before the PHY
    // regulation config was added) attempts to set the sub-1 frequency to
    // 868 MHz. This is not necessary for the 433 MHz band as the phy regulation
    // is set to FCC by default for 433MHz-enabled boards and FCC supports the
    // 433 MHz band
    if(inst.freqSub1 === "freq863" && inst.phyRegulation === "fcc")
    {
        inst.phyRegulation = "etsi";
    }

    // Update values of frequency dependent configs
    networkScript.setDefaultChannelMasks(inst);
    oadScript.setDefaultOadBlockReqRate(inst);
    oadScript.setDefaultOadBlockReqPollDelay(inst);
}

/*
 * ======== getPhyRegulationOptions ========
 * Generates a list of phyRegulations supported for drop down config
 *
 * @param inst - 15.4 instance
 * @returns Array - array of options representing phy regulation types supported
 */
function getPhyRegulationOptions(inst)
{
    const stdOptions = [];
    if(inst.freqBand === "freqBandSub1")
    {
        if(!Common.is433MHzDevice(inst))
        {
            stdOptions.push({
                name: "arib",
                displayName: "ARIB"
            });

            stdOptions.push({
                name: "etsi",
                displayName: "ETSI"
            });
        }

        stdOptions.push({
            name: "fcc",
            displayName: "FCC"
        });
    }
    else
    {
        stdOptions.push({
            name: "ieee154",
            displayName: "IEEE 802.15.4"
        });
    }

    return(stdOptions);
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
    Common.setConfigHiddenState(inst, ui, cfgName, config.config,
        rfCommon.getRFConfigHiddenState);

    // Separate case required for freqSub1 since default value depends
    // on board which can change at runtime via rfDesign
    if(cfgName === "freqSub1" && ui[cfgName].hidden)
    {
        inst.freqSub1 = rfCommon.getDefaultFreqSub1(inst);
    }

    // For coprocessor project the custom Phy option is removed
    if(inst.project == "coprocessor")
    {
        ui["customPhy"].hidden = true;
    }
}

/*
 * ======== setAllRFConfigsHiddenState ========
 * Sets the visibility of all RF configs
 *
 * @param inst    - module instance
 * @param ui      - user interface object
 */
function setAllRFConfigsHiddenState(inst, ui)
{
    Common.setAllConfigsHiddenState(inst, ui, config.config,
        rfCommon.getRFConfigHiddenState, setRFConfigHiddenState);
}

/*
 * ======== setFreqBandReadOnlyState ========
 * Sets the read only status of freqBand config. If config is read only, a
 * read only reason is displayed
 *
 * @param ui       - user interface object
 * @param readOnly - true if freqBand config must be set to read only
 */
function setFreqBandReadOnlyState(ui, readOnly)
{
    // Set read only state of config
    ui.freqBand.readOnly = (readOnly) ? Docs.freqBand.readOnly : false;
}

/*
 *  ======== isCoexEnabled ========
 *  Check whether coexistence is enabled
 *
 * @returns Bool - true if coex enabled in 15.4 module, false otherwise
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

/*
 * ======== validate ========
 * Validate this inst's configuration
 *
 * @param inst       - RF Settings instance to be validated
 * @param validation - object to hold detected validation issues
 */
function validate(inst, validation)
{
    if(inst.project !== "coprocessor")
    {
        // Validate dynamic configs
        let validOptions = rfCommon.getFreqBandOptions(inst);
        Common.validateDynamicEnum(inst, validation, "freqBand", validOptions);

        if(!rfCommon.getRFConfigHiddenState(inst, "freqSub1"))
        {
            validOptions = rfCommon.getFreqSub1Options(inst);
            Common.validateDynamicEnum(inst, validation, "freqSub1",
                validOptions);
        }

        validOptions = rfCommon.getPhyTypeOptions(inst);
        Common.validateDynamicEnum(inst, validation, "phyType", validOptions);

        validOptions = getPhyRegulationOptions(inst);
        Common.validateDynamicEnum(inst, validation, "phyRegulation",
            validOptions);
    }

    // Get the RF Design module to verify that RF Design configs match
    const rfDesign = system.modules["/ti/devices/radioconfig/rfdesign"].$static;
    const isLPSTKBoard = rfDesign.rfDesign.includes("LPSTK");
    if((!isLPSTKBoard && inst.rfDesign !== rfDesign.rfDesign)
       || (isLPSTKBoard && !inst.rfDesign.includes("CC1352R1")))
    {
        validation.logError(`Must match Based On RF Design setting in the \
            ${system.getReference(rfDesign, "rfDesign")} module`, inst,
        "rfDesign");
    }

    // Get the RF module to verify that RF Coexistence configs match
    const rf = system.modules["/ti/drivers/RF"].$static;

    if(inst.customPhy === true && inst.project !== "coprocessor")
    {
        validation.logWarning(`Custom PHYs have not been individually \
            verified to function with the 15.4 stack or examples. Certain \
            combinations of configurations may not be fully functional.`,
            inst, "customPhy");
        const board = Common.getDeviceOrLaunchPadName(true, null, inst);
        if (board.includes("CC1311P") || board.includes("CC26"))
        {
            validation.logError(`Custom PHYs are current unsupported on \
                CC1311P and CC26XX platforms.`, inst, "customPhy");
        }
    }

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
function addRFSettingDependency(inst, selectedPhy)
{
    // Get settings from selected phy
    const radioConfigArgs = _.cloneDeep(selectedPhy.args);

    const phyName = selectedPhy.phyDropDownOption.displayName;
    const is24GPhy = phyName.includes("250 kbps");

    let radioConfigModName = selectedPhy.radioConfigModName;
    radioConfigArgs.$name = "ti154stack_" + radioConfigModName;

    // Set appropriate TX power settings within radio config module
    if(inst.project !== "coprocessor")
    {
        // Overwrite radio config module name for legacy purposes
        // It is okay to use same module name as only PHY added at a time for
        // non-CoP examples
        radioConfigModName = "radioConfig";

        const freqBand = rfCommon.getSafeFreqBand(inst);

        // Retrieve phy and phy group from rf_defaults files to get tx power
        // configuration that needs to be set in the radio config module
        const rfPhySettings = rfCommon.getPhyTypeGroupFromRFConfig(inst);
        const rfPhyType = rfPhySettings.phyType;
        const rfPhyGroup = rfPhySettings.phyGroup;
        const txPower154Obj = powerScript.getRFTxPowerFrom154TxPower(inst,
            freqBand, rfPhyType, rfPhyGroup);

        // Set radio config tx power based on 15.4 tx power setting
        radioConfigArgs[txPower154Obj.cfgName] = txPower154Obj.txPower;

        // Set high PA in radio config if supported by board
        if(Common.isHighPADevice())
        {
            radioConfigArgs.highPA = txPower154Obj.highPA;
        }
    }
    else
    {
        /*
         * Set TX power and high PA settings in radio config:
         * 1) If force VDDR is set, then set radio config's TX power config
         *    to the appropriate level in order for radio setup command to
         *    update. Not applicable to 2.4G or 433MHz PHYs
         * 2) Otherwise enable high PA if supported on current PHY
         * 3) If high PA not supported on board for given PHY, use the default
         *    TX power (0)
         */
        const currBoard = Common.getDeviceOrLaunchPadName(true, null, inst);

        if(inst.forceVddr && _.has(selectedPhy.args, "phyType868"))
        {
            const rfPhyType = selectedPhy.args.phyType868;

            // Get the command handler for this phy instance
            const cmdHandler = CmdHandler.get(commonRadioConfig.PHY_PROP,
                rfPhyType);
            const freq = cmdHandler.getFrequency();

            // Get drop down options of default PA, radio config TX power
            // config
            // Force VDDR CCFG setting only applicable to Sub-G
            const txPowerOpts = powerScript.getTxPowerRFConfig(inst,
                "freqBandSub1", rfPhyType,
                commonRadioConfig.PHY_PROP).txPower;

            // Find TX power level that requires Force VDDR setting
            _.each(txPowerOpts, (txPowerOpt) =>
            {
                if(ParameterHandler.validateTxPower(txPowerOpt.name, freq,
                    false))
                {
                    radioConfigArgs.txPower = txPowerOpt.name;
                }
            });
        }

        if(Common.isHighPADevice())
        {
            // High PA supported on sub-G on P1, P7_1 and CC1311P3 boards and
            // on 2.4G on other P-boards
            const subGHighPA = Common.isSubGHighPADevice(inst, currBoard);

            radioConfigArgs.highPA = ((!subGHighPA && is24GPhy)
                || (subGHighPA && !is24GPhy && !inst.forceVddr));
        }
    }

    // Add 15.4 specific overrides for sub-G projects
    if((inst.project === "coprocessor" && !is24GPhy)
        || (inst.project !== "coprocessor" && inst.freqBand === "freqBandSub1"))
    {
        const overridesMacro = "TI_154_STACK_OVERRIDES";
        radioConfigArgs.codeExportConfig.stackOverride = "ti/ti154stack/"
            + "common/boards/ti_154stack_overrides.h";
        radioConfigArgs.codeExportConfig.stackOverrideMacro = overridesMacro;
    }
    
    return({
        name: radioConfigModName,
        displayName: phyName,
        moduleName: selectedPhy.moduleName,
        description: "Radio Configuration",
        readOnly: false,
        hidden: (!inst.customPhy),
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
    const dependencyModule = [];

    // Determine PHYs to which to add radio config
    const boardPhySettings = rfCommon.getBoardPhySettings(inst);

    // Get proprietary Sub-1 GHz RF defaults for the device being used
    const propPhySettings = boardPhySettings.defaultPropPhyList;

    // Get IEEE 2.4 GHz RF defaults for the device being used
    const ieeePhySettings = boardPhySettings.defaultIEEEPhyList;
    const phyList = propPhySettings.concat(ieeePhySettings);

    // Get the rf settings from the custom phy configurables
    const customPhyListSettings = boardPhySettings.customPhyList;

    // If custom phy option is enabled use the custom phy list settings
    const board = Common.getDeviceOrLaunchPadName(true, null, inst);
    if(inst.customPhy === true && inst.project !== "coprocessor" &&
        !board.includes("CC1311P") && !board.includes("CC26"))
    {
        const selectedPhy = _.find(customPhyListSettings,
            (setting) => (setting.phyDropDownOption.name === "customPhy"));
        dependencyModule.push(addRFSettingDependency(inst, selectedPhy));
    }
    else if(inst.project !== "coprocessor")
    {
        // Find PHY selected
        const phyType = rfCommon.getSafePhyType(inst);

        const selectedPhy = _.find(phyList,
            (setting) => (setting.phyDropDownOption.name === phyType));
        dependencyModule.push(addRFSettingDependency(inst, selectedPhy));
    }
    else
    {
        // CoProcessor projects need all available PHYs
        _.each(phyList, (selectedPhy) =>
        {
            dependencyModule.push(addRFSettingDependency(inst, selectedPhy));
        });
    }

    // Pull module for ti_154stack_config.c generation in all 2.4GHz examples
    if(inst.freqBand === "freqBand24")
    {
        dependencyModule.push({
            name: "coexSettings",
            moduleName: "/ti/ti154stack/rf_config/ti154stack_coex_mod.js",
            group: "coexUseCaseConfigGroup"
        });
    }

    return(dependencyModule);
}


// Check if the device is a P-device, so that correct PA rf command is exported
function IsPdevice() {
    var PaRF_command = [];
    
    if (Common.isHighPADevice()) {
        PaRF_command = customObj.devSpecificCustomPhySettingsPdevices.args.codeExportConfig.cmdPropRadioDivSetupPa;
    }
    else {
        PaRF_command = customObj.devSpecificCustomPhySettings.args.codeExportConfig.cmdPropRadioDivSetup;
    }

    return(PaRF_command);
}

// Exports to the top level 15.4 module
exports = {
    config: config,
    validate: validate,
    moduleInstances: moduleInstances,
    getPhyRegulationOptions: getPhyRegulationOptions,
    setFreqBandReadOnlyState: setFreqBandReadOnlyState,
    setRFConfigHiddenState: setRFConfigHiddenState,
    setAllRFConfigsHiddenState: setAllRFConfigsHiddenState,
    getCoexConfigIeee: function()
    {
        return coexConfig;
    },
    isCoexEnabled: isCoexEnabled,
    IsPdevice, IsPdevice
};
