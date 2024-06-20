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
 *  ======== ti154stack_rf_config_common.js ========
 */

"use strict";

// Get common utility functions
const Common = system.getScript("/ti/ti154stack/ti154stack_common.js");

// Get common radio config module functions
const commonRadioConfig = system.getScript("/ti/devices/radioconfig/"
    + "radioconfig_common.js");

/*
 *  ======== Common PHY Settings ========
 *
 * These Objects contain common phy settings for all boards/devices
 *
 * An entry in this array must contain the following elements:
 *      - radioConfigModName: name given to PHY's radio config module by
 *                            15.4 module
 *      - moduleName: Path to IEEE vs proprietary Sub-1 GHz settings
 *      - args: Phy setting arguments to be passed to the radio config module
 *          - codeExportConfig: Exported code names to be passed to
 *                              radioConfig.codeExportConfig
 *              - symGenMethod: How the names of the generated symbols are
 *                              determined. Valid values are Custom, Legacy, or
 *                              Automatic
 *              - useConst: Add const prefix to generated commands
 *              - rfMode: Name of the generated RF_Mode object
 *              - txPower: Name of the generated Tx power table
 *              - txPowerSize: Name of the generated Tx power table size
 *              - overrides: Name of the generated overrides table
 *              - cmdList_prop: List of commands to generate
 *              - cmdFs: Name of the generated rfc_CMD_FS_t command
 *              - cmdPropCs: Name of the generated rfc_CMD_PROP_CS_t command
 *              - cmdPropRxAdv: Name of the generated rfc_CMD_PROP_RX_ADV_t
 *              - cmdPropTxAdv: Name of the generated rfc_CMD_PROP_TX_ADV_t
 *              - useMulti: Boolean to generate the multi-protocol
 *                          patch
 *      - phyDropDownOption: The options array for the supported phys
 *                           drop-downs in the 15.4 RF Settings module.
 *      - phy154Settings: The array of phy ID and channel page settings for each
 *                        supported frequency
 */

// Object containing SimpleLink Long Range, 5kbps settings for all devices/board
const commonSlLr5KbpsSettings = {
    radioConfigModName: "radioConfig5KbpsSimpleLinkLongRange",
    moduleName: "/ti/devices/radioconfig/settings/prop",
    args: {
        txPower: "0",
        whitening: `Dynamically IEEE 802.15.4g compatible whitener and \
16/32-bit CRC`,
        preambleCount: "2 Bytes",
        syncWordLength: "32 Bits",
        syncWord: 0x5555904E,
        codeExportConfig: {
            symGenMethod: "Custom",
            useConst: true,
            rfMode: "RF_prop_slr5kbps2gfsk",
            txPower: "txPowerTable_slr5kbps2gfsk",
            txPowerSize: "TX_POWER_TABLE_SIZE_slr5kbps2gfsk",
            overrides: "pOverrides_slr5kbps2gfsk",
            cmdList_prop: ["cmdFs", "cmdPropTxAdv", "cmdPropRxAdv",
                "cmdPropCs"],
            cmdFs: "RF_cmdFs_slr5kbps2gfsk",
            cmdPropTxAdv: "RF_cmdPropTxAdv_slr5kbps2gfsk",
            cmdPropRxAdv: "RF_cmdPropRxAdv_slr5kbps2gfsk",
            cmdPropCs: "RF_cmdPropCs_slr5kbps2gfsk",
            useMulti: false
        }
    },
    phyDropDownOption: {
        name: "phy5kbps",
        displayName: "5 kbps, SimpleLink Long Range"
    },
    phy154Settings: {
        freq433: {
            ID: "APIMAC_5KBPS_433MHZ_PHY_130",
            channelPage: "APIMAC_CHANNEL_PAGE_10",
            chanStart: 0,
            chan0Freq: 433300,
            chanSpacing: 200
        },
        freq863: {
            ID: "APIMAC_5KBPS_868MHZ_PHY_131",
            channelPage: "APIMAC_CHANNEL_PAGE_10",
            chanStart: 0,
            chan0Freq: 863125,
            chanSpacing: 200
        },
        freq915: {
            ID: "APIMAC_5KBPS_915MHZ_PHY_129",
            channelPage: "APIMAC_CHANNEL_PAGE_10",
            chanStart: 0,
            chan0Freq: 902200,
            chanSpacing: 200
        }
    }
};

// Object containing 2GFSK, 50kbps settings for all devices/boards
const common2Gfsk50KbpsSettings = {
    radioConfigModName: "radioConfig50Kbps2Gfsk",
    moduleName: "/ti/devices/radioconfig/settings/prop",
    args: {
        txPower: "0",
        whitening: `Dynamically IEEE 802.15.4g compatible whitener and \
16/32-bit CRC`,
        preambleCount: "7 Bytes",
        syncWordLength: "24 Bits",
        syncWord: 0x55904E,
        codeExportConfig: {
            symGenMethod: "Custom",
            useConst: true,
            rfMode: "RF_prop_2gfsk50kbps154g",
            txPower: "txPowerTable_2gfsk50kbps154g",
            txPowerSize: "TX_POWER_TABLE_SIZE_2gfsk50kbps154g",
            overrides: "pOverrides_2gfsk50kbps154g",
            cmdList_prop: ["cmdFs", "cmdPropTxAdv", "cmdPropRxAdv",
                "cmdPropCs"],
            cmdFs: "RF_cmdFs_2gfsk50kbps154g",
            cmdPropTxAdv: "RF_cmdPropTxAdv_2gfsk50kbps154g",
            cmdPropRxAdv: "RF_cmdPropRxAdv_2gfsk50kbps154g",
            cmdPropCs: "RF_cmdPropCs_2gfsk50kbps154g",
            useMulti: false
        }
    },
    phyDropDownOption: {
        name: "phy50kbps",
        displayName: "50 kbps, 2-GFSK"
    },
    phy154Settings: {
        freq433: {
            ID: "APIMAC_50KBPS_433MHZ_PHY_128",
            channelPage: "APIMAC_CHANNEL_PAGE_10",
            chanStart: 0,
            chan0Freq: 433300,
            chanSpacing: 200
        },
        freq863: {
            ID: "APIMAC_50KBPS_868MHZ_PHY_3",
            channelPage: "APIMAC_CHANNEL_PAGE_9",
            chanStart: 0,
            chan0Freq: 863125,
            chanSpacing: 200
        },
        freq915: {
            ID: "APIMAC_50KBPS_915MHZ_PHY_1",
            channelPage: "APIMAC_CHANNEL_PAGE_9",
            chanStart: 0,
            chan0Freq: 902200,
            chanSpacing: 200
        }
    }
};

// Object containing 2GFSK, 200kbps settings for all devices/boards
const common2Gfsk200KbpsSettings = {
    radioConfigModName: "radioConfig200Kbps2Gfsk",
    moduleName: "/ti/devices/radioconfig/settings/prop",
    args: {
        txPower: "0",
        whitening: `Dynamically IEEE 802.15.4g compatible whitener and \
16/32-bit CRC`,
        preambleCount: "7 Bytes",
        syncWordLength: "24 Bits",
        syncWord: 0x55904E,
        codeExportConfig: {
            symGenMethod: "Custom",
            useConst: true,
            rfMode: "RF_prop_2gfsk200kbps154g",
            txPower: "txPowerTable_2gfsk200kbps154g",
            txPowerSize: "TX_POWER_TABLE_SIZE_2gfsk200kbps154g",
            overrides: "pOverrides_2gfsk200kbps154g",
            cmdList_prop: ["cmdFs", "cmdPropTxAdv", "cmdPropRxAdv",
                "cmdPropCs"],
            cmdFs: "RF_cmdFs_2gfsk200kbps154g",
            cmdPropRxAdv: "RF_cmdPropRxAdv_2gfsk200kbps154g",
            cmdPropTxAdv: "RF_cmdPropTxAdv_2gfsk200kbps154g",
            cmdPropCs: "RF_cmdPropCs_2gfsk200kbps154g",
            useMulti: false
        }
    },
    phyDropDownOption: {
        name: "phy200kbps",
        displayName: "200 kbps, 2-GFSK"
    },
    phy154Settings: {
        freq863: {
            ID: "APIMAC_200KBPS_868MHZ_PHY_133",
            channelPage: "APIMAC_CHANNEL_PAGE_10",
            chanStart: 0,
            chan0Freq: 863225,
            chanSpacing: 400
        },
        freq915: {
            ID: "APIMAC_200KBPS_915MHZ_PHY_132",
            channelPage: "APIMAC_CHANNEL_PAGE_10",
            chanStart: 0,
            chan0Freq: 902400,
            chanSpacing: 400
        },
        freq920: {
            ID: "APIMAC_200KBPS_920MHZ_PHY_136",
            channelPage: "APIMAC_CHANNEL_PAGE_10",
            chanStart: 24,
            chan0Freq: 920700,
            chanSpacing: 200
        }
    }
};

/*
 *  ======== Common IEEE PHY Settings ========
 *
 * Array containing all the IEEE phy settings for a given board/device.

 * An entry in this array must contain the following elements:
 *      - args: Phy setting arguments to be passed to the radio config module
 *          - phyType: Name of the phy found in the radio config module.
 *                     Valid options: ieee154
 *          - codeExportConfig: Exported code names to be passed to
 *                              radioConfig.codeExportConfig
 *              - symGenMethod: How the names of the generated symbols are
 *                              determined. Valid values are Custom, Legacy, or
 *                              Automatic
 *              - useConst: Add const prefix to generated commands
 *              - rfMode: Name of the generated RF_Mode object
 *              - txPower: Name of the generated Tx power table
 *              - txPowerSize: Name of the generated Tx power table size
 *              - overrides: Name of the generated overrides table
 *              - cmdList_ieee_15_4: List of commands to generate
 *              - cmdFs: Name of the generated rfc_CMD_FS_t command
 *              - cmdIeeeTx: Name of the generated rfc_CMD_IEEE_TX_t command
 *              - cmdIeeeRx: Name of the generated rfc_CMD_IEEE_RX_t command
 *              - cmdIeeeCsma: Name of generated rfc_CMD_IEEE_CSMA_t command
 *              - cmdIeeeRxAck: Name of the generated rfc_CMD_IEEE_RX_ACK_t
 *                              command
 *              - useMulti: Boolean to generate the multi-protocol
 *                          patch
 */

// Object containing IEEE settings for all devices
const commonIEEESettings = {
    radioConfigModName: "radioConfig250KbpsIeee802154",
    moduleName: "/ti/devices/radioconfig/settings/ieee_15_4",
    args: {
        phyType: "ieee154",
        txPower: "0",
        codeExportConfig: {
            symGenMethod: "Custom",
            useConst: true,
            rfMode: "RF_prop_ieee154",
            txPower: "txPowerTable_ieee154",
            txPowerSize: "TX_POWER_TABLE_SIZE_ieee154",
            overrides: "pOverrides_ieee154",
            cmdList_ieee_15_4: ["cmdFs", "cmdIeeeTx",
                "cmdIeeeRx", "cmdIeeeCsma", "cmdIeeeRxAck"],
            cmdFs: "RF_cmdFs_ieee154",
            cmdIeeeTx: "RF_cmdIeeeTx_ieee154",
            cmdIeeeRx: "RF_cmdIeeeRx_ieee154",
            cmdIeeeCsma: "RF_cmdIeeeCsma_ieee154",
            cmdIeeeRxAck: "RF_cmdIeeeRxAck_ieee154",
            useMulti: false
        }
    },
    phyDropDownOption: {
        name: "phyIEEE",
        displayName: "250 kbps, OQPSK"
    },
    phy154Settings: {
        phyIEEE: {
            ID: "APIMAC_250KBPS_IEEE_PHY_0",
            channelPage: "APIMAC_CHANNEL_PAGE_NONE",
            chanStart: 0,
            chan0Freq: 2405000,
            chanSpacing: 5000
        }
    }
};

/*!
 *  ======== arrayMerge ========
 *  Helper function for merging two Arrays when using _.mergeWith
 *
 *  @param objValue - new property to be merged
 *  @param srcValue - property of the original object
 *  @returns Array - If objValue is an array, concat of objValue and srcValue
 *  @returns undefined - If objValue is not an array
 */
function arrayMerge(objValue, srcValue)
{
    let concatArray; // undefined array

    if(_.isArray(objValue))
    {
        // concatArray = _.union(objValue, srcValue);
        concatArray = objValue.concat(srcValue);
    }

    return(concatArray);
}

/*!
 *  ======== mergeRFSettings ========
 * Helper function for merging two rf settings objects into a single object
 * with correct ordering. Ordering of the arguments matters. Source objects are
 * applied from left to right. Subsequent sources overwrite property assignments
 * of previous sources.
 *
 * Ordering required:
 *  1. freqBand (subG only)
 *  2. phyTypeX
 *  3. symGenMethod (first property of codeExportConfig)
 *  4. cmdList_X (second property of codeExportConfig)
 *
 * @param object1 - first rf setting object to be merged. Must contain an args
 * and codeExportConfig property
 * @param object2 - second rf setting object to be merged. Must contain an args
 * and codeExportConfig property
 *
 * @returns Object - An object containing all the properties from object1 and
 * object2 with the correct ordering required for the Radio Configuration module
 */
function mergeRFSettings(obj1, obj2)
{
    // Possible cmdList_ and phyTypeXXX properties in the input objects
    const cmdLists = ["cmdList_prop", "cmdList_ieee_15_4"];
    const phyTypes = ["phyType868", "phyType433", "phyType"];

    // Object to contain the newly merged properties
    const mergedObj = {args: {}};

    // If there is a freqBand property (i.e. subg) set it as the first property
    // of args
    if(_.has(obj1.args, "freqBand") || _.has(obj2.args, "freqBand"))
    {
        // Set to dummy value to specify ordering, overwritten by _.mergeWith
        mergedObj.args.freqBand = "dummy";
    }

    // If there is a phyType property set it as the second property of args
    let phyType = null;
    for(phyType of phyTypes)
    {
        if(_.has(obj1.args, phyType) || _.has(obj2.args, phyType))
        {
            // Set to dummy value to specify order, overwritten by _.mergeWith
            mergedObj.args[phyType] = "dummy";
            break;
        }
    }

    // If a symGenMethod property set as first property of codeExportConfig
    mergedObj.args.codeExportConfig = {};
    if(_.has(obj1.args.codeExportConfig, "symGenMethod")
        || _.has(obj2.args.codeExportConfig, "symGenMethod"))
    {
        // Set to dummy value to specify ordering, overwritten by _.mergeWith
        mergedObj.args.codeExportConfig.symGenMethod = "dummy";
    }

    // If a cmdList_ property set as second property of codeExportConfig
    let list = null;
    for(list of cmdLists)
    {
        if(_.has(obj1.args.codeExportConfig, list)
            || _.has(obj2.args.codeExportConfig, list))
        {
            // Set to dummy value to specify order, overwritten by _.mergeWith
            mergedObj.args.codeExportConfig[list] = [];
            break;
        }
    }

    // Merge args.codeExportConfig property of new object, obj1, and obj2 using
    _.mergeWith(mergedObj, obj1, obj2, arrayMerge);

    return(mergedObj);
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
    const isCoPProject = (inst.project === "coprocessor");

    switch(cfgName)
    {
        case "phyID":
        case "channelPage":
        {
            // Deprecated
            isVisible = false;
            break;
        }
        case "freqSub1":
        {
            isVisible = (inst.freqBand === "freqBandSub1" && !isCoPProject);
            break;
        }
        case "freqBand":
        case "phyType":
        case "phyRegulation":
        {
            isVisible = !isCoPProject;
            break;
        }
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
        case "customFreqBand":
        case "customCh0Freq":
        case "customChSpacing":
        case "customChNum":
        {
            isVisible = false;
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
 * ======== getBoardPhySettings ========
 * Determines which rf_defaults script to use based on device or inst.rfDesign
 *
 * @param inst - Instance of this module
 *
 * @returns Obj - rf_defaults script from which to get phy settings in
 *                radioconfig format. If device is not supported, returns null
 */
function getBoardPhySettings(inst)
{
    let phySettings;

    if(inst !== null && system.deviceData.deviceId === "CC1352P1F3RGZ")
    {
        // Get the RF Design configurable
        const rfDesign = inst.rfDesign;
        if(rfDesign === "LAUNCHXL-CC1352P-4")
        {
            phySettings = system.getScript("/ti/ti154stack/rf_config/"
                + "CC1352P_4_LAUNCHXL_rf_defaults.js");
        }
        else if(rfDesign === "LAUNCHXL-CC1352P1")
        {
            phySettings = system.getScript("/ti/ti154stack/rf_config/"
                + "CC1352P1_LAUNCHXL_rf_defaults.js");
        }
        else if(rfDesign === "LAUNCHXL-CC1352P-2")
        {
            phySettings = system.getScript("/ti/ti154stack/rf_config/"
                + "CC1352P_2_LAUNCHXL_rf_defaults.js");
        }
    }
    else if(inst !== null && system.deviceData.deviceId === "CC1352P7RGZ")
    {
        // Get the RF Design configurable
        const rfDesign = inst.rfDesign;
        if(rfDesign === "LP_CC1352P7-1")
        {
            phySettings = system.getScript("/ti/ti154stack/rf_config/"
                + "LP_CC1352P7_1_rf_defaults.js");
        }
        else if(rfDesign === "LP_CC1352P7-4")
        {
            phySettings = system.getScript("/ti/ti154stack/rf_config/"
                + "LP_CC1352P7_4_rf_defaults.js");
        }
    }
    else if(inst !== null && system.deviceData.deviceId === "CC1354P10RSK")
    {
        // Get the RF Design configurable
        const rfDesign = inst.rfDesign;
        if(rfDesign === "LP_EM_CC1354P10_1")
        {
            phySettings = system.getScript("/ti/ti154stack/rf_config/"
                + "LP_EM_CC1354P10_1_rf_defaults.js");
        }
        else if(rfDesign === "LP_EM_CC1354P10_6")
        {
            phySettings = system.getScript("/ti/ti154stack/rf_config/"
                + "LP_EM_CC1354P10_6_rf_defaults.js");
        }
    }
    else
    {
        // Initialize with launchpad mapped from device
        phySettings = system.getScript("/ti/ti154stack/rf_config/"
            + Common.getLaunchPadFromDevice() + "_rf_defaults.js");
    }

    const validPhySettings = _.cloneDeep(phySettings);

    if(!_.isNil(inst))
    {
        // Filter out unapplicable phy settings based on current configuration
        const phyLists = ["defaultPropPhyList", "defaultIEEEPhyList", "customPhyList"];
        const phyTypes = ["phyType868", "phyType433", "phyType"];

        for(const phyList of phyLists)
        {
            for(const phy of phySettings[phyList])
            {
                if(_.has(phy, "unsupported"))
                {
                    for(const cfg in phy.unsupported)
                    {
                        /* A phy is not supported when any of the configs listed
                         * in the 'unsupported' element of the phy object are
                         * both:
                         *   - Visible (i.e. applicable)
                         *   - Set to any of the unsupported values listed
                         *
                         * It is expected that the only parameters that will
                         * determine whether a config is visible are the project
                         * config (which is always visible) and those in the
                         * radio module. By default, all getXConfigHiddenState
                         * will return false (i.e. not hidden) if the config is
                         * not listed, thus it is not necessary to explicitly
                         * check the status of the project config
                         */
                        if((phy.unsupported[cfg]).includes(inst[cfg])
                            && !getRFConfigHiddenState(inst, cfg))
                        {
                            let radioConfigPhy = null;
                            let phyType = null;

                            // Find phy's specific phyType key and setting
                            for(phyType of phyTypes)
                            {
                                if(_.has(phy.args, phyType))
                                {
                                    radioConfigPhy = phy.args[phyType];
                                    break;
                                }
                            }

                            // Remove unsupported phy from phy list
                            _.remove(validPhySettings[phyList],
                                (o) => o.args[phyType] === radioConfigPhy);

                            break;
                        }
                    }
                }
            }
        }
    }

    return(validPhySettings);
}

/*
 *  ======== getPhySysConfigInfo ========
 *  Get phy ID and channel page based on phy type and frequency band
 *
 *  @param inst - Instance of this module
 *  @returns object with ID and channelPage values
 */
function getPhySysConfigInfo(inst)
{
    const phyType = getSafePhyType(inst);
    const freqBand = getSafeFreqBand(inst);
    const freqSub1 = getSafeFreqSub1(inst);
    const rfPhySettings = getBoardPhySettings(inst);
    let phy154Setting;

    // If custom Phy is enabled use the Phy settings from phyCustom.js
    if(inst != null && inst.customPhy && freqBand === "freqBandSub1")
    {
        phy154Setting = {};
        phy154Setting.ID = "APIMAC_CUSTOM_PHY_ID",
        phy154Setting.channelPage = "APIMAC_CHANNEL_PAGE_10",
        phy154Setting.freqBand = inst.customFreqBand;
        phy154Setting.chanStart = 0;
        phy154Setting.chan0Freq = inst.customCh0Freq * 1000;
        phy154Setting.chanSpacing = inst.customChSpacing;
        phy154Setting.numChannels = inst.customChNum;
    }
    else if(freqBand === "freqBandSub1")
    {
        // Get proprietary Sub-1 GHz RF defaults for the device being used
        const propPhySettings = rfPhySettings.defaultPropPhyList;

        // Find phy object associated with phy type
        const phyObj = _.find(propPhySettings,
            (settings) => (settings.phyDropDownOption.name === phyType));

        // Get phy ID and channel page of given sub-1 frequency and rate
        if(phyObj)
        {
            phy154Setting = phyObj.phy154Settings[freqSub1];
        }
    }

    else if(freqBand === "freqBand24")
    {
        // Get IEEE 2.4 GHz RF defaults for the device being used
        const ieeePhySettings = rfPhySettings.defaultIEEEPhyList;

        // Only one phy type for 2.4GHz
        phy154Setting = ieeePhySettings[0].phy154Settings.phyIEEE;
    }

    return(phy154Setting);
}

function getPhySysConfigInfoForCustomPhy(inst)
{

    const rfPhySettings = getBoardPhySettings(inst);
    let phy154Setting = null;
   
    // Get proprietary Sub-1 GHz RF defaults for the device being used
    const customPhySettings = rfPhySettings.customPhyList;

    phy154Setting = customPhySettings[0].phy154Settings["freq"];

    return(phy154Setting);
}

/*
 * ======== getPhyTypeGroupFromRFConfig ========
 * Returns phy group and phy type based on frequency set and radio config module
 * defaults files.
 *
 * Required in order to set tx power parameters in radio config module when it
 * is added in moduleInstances(). The radio config instance cannot be accessed
 * during this process to retrieve these values
 *
 * @param inst       - 15.4 instance
 */
function getPhyTypeGroupFromRFConfig(inst)
{
    let rfPhyType;
    let rfPhyGroup;
    let allPhySettings;

    const localPhyType = getSafePhyType(inst);
    const freqBand = getSafeFreqBand(inst);

    // Possible phyTypeXXX properties in the phy settings
    const phyTypes = ["phyType868", "phyType433", "phyType"];

    // Set phy group based on frequency selected and supported
    if((freqBand === "freqBandSub1" && !localPhyType.includes("IEEE"))
        || (Common.isSub1GHzDevice() && !Common.is24GHzDevice(inst)))
    {
        // Get proprietary Sub-1 GHz RF defaults for the device being used
        allPhySettings = getBoardPhySettings(inst).defaultPropPhyList;
        rfPhyGroup = commonRadioConfig.PHY_PROP;
    }
    else
    {
        // Get IEEE 2.4 GHz RF defaults for the device being used
        allPhySettings = getBoardPhySettings(inst).defaultIEEEPhyList;
        rfPhyGroup = commonRadioConfig.PHY_IEEE_15_4;
    }

    // Extract data rate from 15.4 phy type config
    // Convert string to lowercase to convert "IEEE" to "ieee" for matching
    const dataRate = _.toLower(_.replace(localPhyType, "phy", ""));

    // Search through phy settings array which contains the objects with
    // settings for phys/data rates supported from rf_defaults files
    let phySetting = null;
    for(phySetting of allPhySettings)
    {
        let phyType = null;
        for(phyType of phyTypes)
        {
            // Check for phy property that contains the data rate currently
            // selected
            if(_.has(phySetting.args, phyType)
                && phySetting.args[phyType].includes(dataRate))
            {
                rfPhyType = phySetting.args[phyType];
                break;
            }
        }
    }

    return{
        phyType: rfPhyType,
        phyGroup: rfPhyGroup
    };
}

/*
 * ======== getDefaultPhyRegulation ========
 * Retrieves the default phyRegulation type from the drop down options
 *     * fcc for CC13xx boards
 *     * ieee154 for CC26xx boards
 *
 * @param inst - 15.4 instance (null during initialization)
 * @returns - name of default phyRegulation type
 */
function getDefaultPhyRegulation(inst)
{
    let reg;
    if(inst !== null)
    {
        reg = (inst.freqBand === "freqBandSub1") ? "fcc" : "ieee154";
    }
    else if(Common.is24GHzDevice(inst) && !Common.isSub1GHzDevice())
    {
        reg = "ieee154";
    }
    else
    {
        reg = "fcc";
    }

    return(reg);
}

/*
 * ======== getFreqSub1Options ========
 * Generates a list of sub-1 GHz frequency bands supported for drop down config
 *
 * @param inst - 15.4 instance (null during initialization)
 * @returns Array - array of options representing sub-1 GHz bands supported
 */
function getFreqSub1Options(inst)
{
    let freqSub1Options;

    const phyRegulation = _.isNil(inst) ? getDefaultPhyRegulation(inst)
        : inst.phyRegulation;

    if(phyRegulation === "fcc")
    {
        if(Common.is433MHzDevice(inst))
        {
            freqSub1Options = [
                {
                    name: "freq433",
                    displayName: "433 - 434 MHz"
                }
            ];
        }
        else
        {
            freqSub1Options = [
                {
                    name: "freq915",
                    displayName: "902 - 927 MHz"
                }
            ];
        }
    }
    else if(phyRegulation === "etsi")
    {
        freqSub1Options = [
            {
                name: "freq863",
                displayName: "863 - 869 MHz"
            }
        ];
    }
    else if(phyRegulation === "arib")
    {
        freqSub1Options = [
            {
                name: "freq920",
                displayName: "920 - 927 MHz"
            }
        ];
    }
    else
    {
        freqSub1Options = [];
    }

    return(freqSub1Options);
}

/*
 * ======== getDefaultFreqSub1 ========
 * Retrieves the default sub-1 GHz frequency value from the drop down options
 *     * 433MHz for CC1352P4
 *     * 915MHz for others
 *
 * @param inst - 15.4 instance (null during initialization)
 * @returns - name of default frequency
 */
function getDefaultFreqSub1(inst)
{
    let defaultFreq;
    if(Common.is433MHzDevice(inst))
    {
        defaultFreq = "freq433";
    }
    else
    {
        defaultFreq = "freq915";
    }
    return defaultFreq;
}

/*
 * ======== getSafeFreqSub1 ========
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
function getSafeFreqSub1(inst)
{
    const validOptions = getFreqSub1Options(inst);
    const defaultFreqBand = getDefaultFreqSub1(inst);

    const freqBand = Common.getSafeDynamicConfig(inst, "freqSub1",
        defaultFreqBand, validOptions);

    return(freqBand);
}

/*
 * ======== getFreqBandOptions ========
 * Generates a list of frequency bands supported for drop down config
 *
 * @param inst - 15.4 instance (null during initialization)
 * @returns Array - array of options representing frequency bands supported
 */
function getFreqBandOptions(inst)
{
    const freqBandOptions = [];

    if(Common.isSub1GHzDevice())
    {
        freqBandOptions.push(
            {
                name: "freqBandSub1",
                displayName: "Sub-1 GHz"
            }
        );
    }

    if(Common.is24GHzDevice(inst))
    {
        freqBandOptions.push(
            {
                name: "freqBand24",
                displayName: "2.4 GHz"
            }
        );
    }

    return(freqBandOptions);
}

/*
 *  ======== getDefaultFreqBand ========
 *  Retrieves the default frequency band
 *
 *  @param getSubGDefault - Boolean. True selects Sub-1 GHz default value,
 *      False returns 2.4 GHz default value
 *  @returns - name of default frequency band
 */
function getDefaultFreqBand(getSubGDefault)
{
    let defaultFreqBand;
    if(getSubGDefault)
    {
        defaultFreqBand = "freqBandSub1";
    }
    else
    {
        defaultFreqBand = "freqBand24";
    }

    return(defaultFreqBand);
}

/*!
 * ======== getSafeFreqBand ========
 * Safely retrieve the value of the config by returning the instance value it's
 * valid, otherwise returns the default value.
 *
 * Due to their nature, dynamic enum configurables may be incorrectly modified
 * through the .syscfg file. While all dynamic configs have validation functions
 * to detect such errors, the dependency of the radio config module requires
 * safe access to some configs to avoid SysCOnfig breaks.
 *
 * Access to function needed from main power_config and rf_config files
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @returns - config value in instance (if valid), otherwise config default
 *            value
 */
function getSafeFreqBand(inst)
{
    const validOptions = getFreqBandOptions(inst);
    const isSubGSelected = Common.isSub1GHzDevice();
    const defaultFreqBand = getDefaultFreqBand(isSubGSelected);

    const freqBand = Common.getSafeDynamicConfig(inst, "freqBand",
        defaultFreqBand, validOptions);

    return(freqBand);
}

/*
 *  ======== getDefaultPhyType ========
 *  Retrieves the default phyType
 *      * 50kbps for Sub-1 GHz
 *      * 250kbps for 2.4 Ghz
 *
 *  @param inst - 15.4 instance
 *  @returns - name of default phyType (50kbps, 2-GFSK)
 */
function getDefaultPhyType(inst)
{
    let defaultPhyType = "phy50kbps";

    if(inst !== null && inst.freqSub1 === "freq920")
    {
        defaultPhyType = "phy200kbps";
    }
    else if((inst !== null && inst.freqBand === "freqBand24")
        || !Common.isSub1GHzDevice())
    {
        defaultPhyType = "phyIEEE";
    }

    return(defaultPhyType);
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

    // Extract phy settings from RF defaults files
    if((!_.isNil(inst) && inst.freqBand === "freqBandSub1") ||
       (_.isNil(inst) && Common.isSub1GHzDevice()))
    {
        // Get proprietary Sub-1 GHz RF defaults for the device being used
        phyList = getBoardPhySettings(inst).defaultPropPhyList;
    }
    else
    {
        // Get IEEE 2.4 GHz RF defaults for the device being used
        phyList = getBoardPhySettings(inst).defaultIEEEPhyList;
    }

    // Construct the drop down options array
    return(_.map(phyList, (phy) => phy.phyDropDownOption));
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
    const defaultPhyType = getDefaultPhyType(inst);

    const phyType = Common.getSafeDynamicConfig(inst, "phyType",
        defaultPhyType, validOptions);

    return(phyType);
}

// Get frequency options for custom PHY
function getCustomFreqOptions(inst)
{
    let freqSub1Options;

    if(Common.is433MHzDevice(inst))
    {
        freqSub1Options = [{name: "freq433", displayName: "433 - 434 MHz"}];
    }
    else
    {
        freqSub1Options = [{name: "freq915", displayName: "902 - 927 MHz"}];
    }
    freqSub1Options.push({name: "freq863", displayName: "863 - 869 MHz"});
    freqSub1Options.push({name: "freq920", displayName: "920 - 927 MHz"});

    return(freqSub1Options);
}

exports = {
    commonSlLr5KbpsSettings: commonSlLr5KbpsSettings,
    common2Gfsk50KbpsSettings: common2Gfsk50KbpsSettings,
    common2Gfsk200KbpsSettings: common2Gfsk200KbpsSettings,
    commonIEEESettings: commonIEEESettings,
    mergeRFSettings: mergeRFSettings,
    getRFConfigHiddenState: getRFConfigHiddenState,
    getBoardPhySettings: getBoardPhySettings,
    getPhyTypeGroupFromRFConfig: getPhyTypeGroupFromRFConfig,
    getFreqBandOptions: getFreqBandOptions,
    getDefaultFreqBand: getDefaultFreqBand,
    getSafeFreqBand: getSafeFreqBand,
    getDefaultPhyType: getDefaultPhyType,
    getPhyTypeOptions: getPhyTypeOptions,
    getSafePhyType: getSafePhyType,
    getFreqSub1Options: getFreqSub1Options,
    getDefaultFreqSub1: getDefaultFreqSub1,
    getSafeFreqSub1: getSafeFreqSub1,
    getPhySysConfigInfo: getPhySysConfigInfo,
    getDefaultPhyRegulation: getDefaultPhyRegulation,
    getPhySysConfigInfoForCustomPhy: getPhySysConfigInfoForCustomPhy,
    getCustomFreqOptions: getCustomFreqOptions,
};
