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
    moduleName: "/ti/devices/radioconfig/settings/prop",
    args: {
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
            channelPage: "APIMAC_CHANNEL_PAGE_10"
        },
        freq863: {
            ID: "APIMAC_5KBPS_868MHZ_PHY_131",
            channelPage: "APIMAC_CHANNEL_PAGE_10"
        },
        freq915: {
            ID: "APIMAC_5KBPS_915MHZ_PHY_129",
            channelPage: "APIMAC_CHANNEL_PAGE_10"
        }
    }
};

// Object containing 2GFSK, 50kbps settings for the all devices/boards
const common2Gfsk50KbpsSettings = {
    moduleName: "/ti/devices/radioconfig/settings/prop",
    args: {
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
            channelPage: "APIMAC_CHANNEL_PAGE_10"
        },
        freq863: {
            ID: "APIMAC_50KBPS_868MHZ_PHY_3",
            channelPage: "APIMAC_CHANNEL_PAGE_9"
        },
        freq915: {
            ID: "APIMAC_50KBPS_915MHZ_PHY_1",
            channelPage: "APIMAC_CHANNEL_PAGE_9"
        }
    }
};

// Object containing 2GFSK, 100kbps settings for the all devices/boards
const common2Gfsk100KbpsSettings = {
    moduleName: "/ti/devices/radioconfig/settings/prop",
    args: {
        codeExportConfig: {
            symGenMethod: "Custom",
            useConst: true,
            rfMode: "RF_prop_2gfsk100kbps154g",
            txPower: "txPowerTable_2gfsk100kbps154g",
            txPowerSize: "TX_POWER_TABLE_SIZE_2gfsk100kbps154g",
            overrides: "pOverrides_2gfsk100kbps154g",
            cmdList_prop: ["cmdFs", "cmdPropTxAdv", "cmdPropRxAdv",
                "cmdPropCs"],
            cmdFs: "RF_cmdFs_2gfsk100kbps154g",
            cmdPropRxAdv: "RF_cmdPropRxAdv_2gfsk100kbps154g",
            cmdPropTxAdv: "RF_cmdPropTxAdv_2gfsk100kbps154g",
            cmdPropCs: "RF_cmdPropCs_2gfsk100kbps154g",
            useMulti: false
        }
    },
    phyDropDownOption: {
        name: "phy100kbps",
        displayName: "100 kbps, 2-GFSK"
    },
    phy154Settings: {
        freq866: {
            ID: "APIMAC_100KBPS_866MHZ_PHY_5",
            channelPage: "APIMAC_CHANNEL_PAGE_9"
        }
    }
};

// Object containing 2GFSK, 200kbps settings for the all devices/boards
const common2Gfsk200KbpsSettings = {
    moduleName: "/ti/devices/radioconfig/settings/prop",
    args: {
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
            channelPage: "APIMAC_CHANNEL_PAGE_10"
        },
        freq915: {
            ID: "APIMAC_200KBPS_915MHZ_PHY_132",
            channelPage: "APIMAC_CHANNEL_PAGE_10"
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

// Object containing IEEE settings for the all devices
const commonIEEESettings = {
    moduleName: "/ti/devices/radioconfig/settings/ieee_15_4",
    args: {
        phyType: "ieee154",
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
        displayName: "250 kbps, IEEE 802.15.4"
    },
    phy154Settings: {
        phyIEEE: {
            ID: "APIMAC_250KBPS_IEEE_PHY_0",
            channelPage: "APIMAC_CHANNEL_PAGE_NONE"
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
 * ======== getPropPhySettings ========
 * Get proprietary Sub-1 GHz RF defaults for the device and frequency being used
 *
 * @param inst - 15.4 instance
 * @param freqSub1 - Sub-1 GHz frequency
 * @returns Array - Array of phy settings
 */
function getPropPhySettings(inst, freqSub1)
{
    // Get all Sub-1 GHz phy defaults
    let propPhySettings = getBoardPhySettings(inst).defaultPropPhyList;

    // Filter out phy objects not applicable to current sub-1 frequency
    propPhySettings = _.filter(propPhySettings,
        (phy) => phy.phy154Settings[freqSub1]);

    return propPhySettings;
}

/*
 * ======== getIEEEPhySettings ========
 * Get IEEE RF defaults for the device being used
 *
 * @param inst - 15.4 instance
 * @returns Array - Array of phy settings
 */
function getIEEEPhySettings(inst)
{
    return getBoardPhySettings(inst).defaultIEEEPhyList;
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
    else
    {
        // Initialize with launchpad mapped from device
        phySettings = system.getScript("/ti/ti154stack/rf_config/"
            + Common.getLaunchPadFromDevice() + "_rf_defaults.js");
    }

    return(phySettings);
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

    // Possible phyTypeXXX properties in the phy settings
    const phyTypes = ["phyType868", "phyType433", "phyType"];

    // Set phy group based on frequency selected and supported
    if(inst.freqBand === "freqBandSub1"
        || (Common.isSub1GHzDevice() && !Common.is24GHzDevice(inst)))
    {
        // Get proprietary Sub-1 GHz RF defaults for the device being used
        const freqSub1 = getSafeFreqSub1(inst);
        allPhySettings = getPropPhySettings(inst, freqSub1);
        rfPhyGroup = commonRadioConfig.PHY_PROP;
    }
    else
    {
        // Get IEEE 2.4 GHz RF defaults for the device being used
        allPhySettings = getIEEEPhySettings(inst);
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
 *  ======== getDefaultPhyType ========
 *  Retrieves the default phyType
 *      * 50kbps for Sub-1 GHz
 *      * 250kbps for 2.4 Ghz
 *
 *  @param getSubGDefault - Boolean. True selects Sub-1 GHz default value,
 *      False returns 2.4 GHz default value
 *  @returns - name of default phyType (50kbps, 2-GFSK)
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
 * ======== getFreqSub1Options ========
 * Generates a list of sub-1 GHz frequency bands supported for drop down config
 *
 * @param inst - 15.4 instance (null during initialization)
 * @returns Array - array of options representing sub-1 GHz bands supported
 */
function getFreqSub1Options(inst)
{
    let freqSub1Options;
    if(Common.is433MHzDevice(inst))
    {
        freqSub1Options = [
            {
                name: "freq433",
                displayName: "433 MHz"
            }
        ];
    }
    else
    {
        freqSub1Options = [
            {
                name: "freq866",
                displayName: "866 MHz"
            },
            {
                name: "freq863",
                displayName: "868 MHz"
            },
            {
                name: "freq915",
                displayName: "915 MHz"
            }
        ];
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
    const sub1FreqOpts = getFreqSub1Options(inst);
    return _.first(sub1FreqOpts).name;
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
 *  ======== getDefaultPhyType ========
 *  Retrieves the default phyType
 *      * 50kbps for Sub-1 GHz
 *      * 250kbps for 2.4 Ghz
 *
 *  @param getSubGDefault - Boolean. True selects Sub-1 GHz default value,
 *      False returns 2.4 GHz default value
 *  @returns - name of default phyType (50kbps, 2-GFSK)
 */
function getDefaultPhyType(getSubGDefault)
{
    let defaultPhyType;
    if(getSubGDefault)
    {
        defaultPhyType = "phy50kbps";
    }
    else
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
    if(inst.freqBand === "freqBandSub1")
    {
        // Get proprietary Sub-1 GHz RF defaults for the device being used
        phyList = getPropPhySettings(inst, inst.freqSub1);
    }
    else
    {
        // Get IEEE 2.4 GHz RF defaults for the device being used
        phyList = getIEEEPhySettings(inst);
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
    const isSubGSelected = Common.isSub1GHzDevice();
    const defaultPhyType = getDefaultPhyType(isSubGSelected);

    const phyType = Common.getSafeDynamicConfig(inst, "phyType",
        defaultPhyType, validOptions);

    return(phyType);
}

exports = {
    commonSlLr5KbpsSettings: commonSlLr5KbpsSettings,
    common2Gfsk50KbpsSettings: common2Gfsk50KbpsSettings,
    common2Gfsk100KbpsSettings: common2Gfsk100KbpsSettings,
    common2Gfsk200KbpsSettings: common2Gfsk200KbpsSettings,
    commonIEEESettings: commonIEEESettings,
    mergeRFSettings: mergeRFSettings,
    getBoardPhySettings: getBoardPhySettings,
    getIEEEPhySettings: getIEEEPhySettings,
    getPropPhySettings: getPropPhySettings,
    getPhyTypeGroupFromRFConfig: getPhyTypeGroupFromRFConfig,
    getFreqBandOptions: getFreqBandOptions,
    getDefaultFreqBand: getDefaultFreqBand,
    getSafeFreqBand: getSafeFreqBand,
    getFreqSub1Options: getFreqSub1Options,
    getDefaultFreqSub1: getDefaultFreqSub1,
    getSafeFreqSub1: getSafeFreqSub1,
    getDefaultPhyType: getDefaultPhyType,
    getPhyTypeOptions: getPhyTypeOptions,
    getSafePhyType: getSafePhyType
};
