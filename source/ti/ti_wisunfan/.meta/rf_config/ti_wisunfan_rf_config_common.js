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
 *  ======== ti_wisunfan_rf_config_common.js ========
 */

"use strict";

// Get common utility functions
const Common = system.getScript("/ti/ti_wisunfan/ti_wisunfan_common.js");

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
            channelPage: "APIMAC_CHANNEL_PAGE_10",
            channelSpacing: "200",
            totalChannels: "7",
            ccaType: "1"
        },
        freq863: {
            ID: "APIMAC_50KBPS_868MHZ_PHY_3",
            channelPage: "APIMAC_CHANNEL_PAGE_9",
            channelSpacing: "200",
            totalChannels: "34",
            ccaType: "1"
        },
        freq915: {
            ID: "APIMAC_50KBPS_915MHZ_PHY_1",
            channelPage: "APIMAC_CHANNEL_PAGE_9",
            channelSpacing: "200",
            totalChannels: "129",
            ccaType: "1"
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


// Object containing 2GFSK, 150kbps settings for the all devices/boards
const common2Gfsk150KbpsSettings = {
    moduleName: "/ti/devices/radioconfig/settings/prop",
    args: {
        codeExportConfig: {
            symGenMethod: "Custom",
            useConst: true,
            rfMode: "RF_prop_2gfsk150kbps154g",
            txPower: "txPowerTable_2gfsk150kbps154g",
            txPowerSize: "TX_POWER_TABLE_SIZE_2gfsk150kbps154g",
            overrides: "pOverrides_2gfsk150kbps154g",
            cmdList_prop: ["cmdFs", "cmdPropTxAdv", "cmdPropRxAdv",
                "cmdPropCs"],
            cmdFs: "RF_cmdFs_2gfsk150kbps154g",
            cmdPropRxAdv: "RF_cmdPropRxAdv_2gfsk150kbps154g",
            cmdPropTxAdv: "RF_cmdPropTxAdv_2gfsk150kbps154g",
            cmdPropCs: "RF_cmdPropCs_2gfsk150kbps154g",
            useMulti: false
        }
    },
    phyDropDownOption: {
        name: "phy150kbps",
        displayName: "150 kbps, 2-GFSK"
    },
    phy154Settings: {
        freq866: {
            ID: "APIMAC_150KBPS_866MHZ_PHY",
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

// Object containing 2GFSK, 200kbps settings for the all devices/boards
const common2Gfsk300KbpsSettings = {
    moduleName: "/ti/devices/radioconfig/settings/prop",
    args: {
        codeExportConfig: {
            symGenMethod: "Custom",
            useConst: true,
            rfMode: "RF_prop_2gfsk300kbps154g",
            txPower: "txPowerTable_2gfsk300kbps154g",
            txPowerSize: "TX_POWER_TABLE_SIZE_2gfsk300kbps154g",
            overrides: "pOverrides_2gfsk300kbps154g",
            cmdList_prop: ["cmdFs", "cmdPropTxAdv", "cmdPropRxAdv",
                "cmdPropCs"],
            cmdFs: "RF_cmdFs_2gfsk300kbps154g",
            cmdPropRxAdv: "RF_cmdPropRxAdv_2gfsk300kbps154g",
            cmdPropTxAdv: "RF_cmdPropTxAdv_2gfsk300kbps154g",
            cmdPropCs: "RF_cmdPropCs_2gfsk300kbps154g",
            useMulti: false
        }
    },
    phyDropDownOption: {
        name: "phy300kbps",
        displayName: "300 kbps, 2-GFSK"
    },
    phy154Settings: {
        freq863: {
            ID: "APIMAC_300KBPS_868MHZ_PHY_133",
            channelPage: "APIMAC_CHANNEL_PAGE_10"
        },
        freq915: {
            ID: "APIMAC_300KBPS_915MHZ_PHY_XXX",
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

/* This mapping connects the phyMode type with it's associated display name */
const phyModeDisplayNames_updated = {
        "1":
        {
            name: "1",
            displayName: "Mode 1, 50 Kbps, Mod Index 0.5",
            channelPage: "APIMAC_CHANNEL_PAGE_9"
        },
        "2":
        {
            name: '2',
            displayName: 'Mode 2, 50 Kbps, Mod Index 1',
            channelPage: "APIMAC_CHANNEL_PAGE_9"
        },
        "3":
        {
            name: '3',
            displayName: 'Mode 3, 100 Kbps, Mod Index 0.5',
            channelPage: "APIMAC_CHANNEL_PAGE_10"
        },
        "4":
        {
            name: '4',
            displayName: 'Mode 4, 100 Kbps, Mod Index 1',
            channelPage: "APIMAC_CHANNEL_PAGE_10"
        },
        "5":
        {
            name: '5',
            displayName: 'Mode 5, 150 Kbps, Mod Index 0.5',
            channelPage: "APIMAC_CHANNEL_PAGE_10"
        },
        "6":
        {
            name: '6',
            displayName: 'Mode 6, 200 Kbps, Mod Index 0.5',
            channelPage: "APIMAC_CHANNEL_PAGE_10"
        },
        "7":
        {
            name: '7',
            displayName: 'Mode 7, 200 Kbps, Mod Index 1',
            channelPage: "APIMAC_CHANNEL_PAGE_10"
        },
        "8":
        {
            name: '8',
            displayName: 'Mode 8, 300 Kbps, Mod Index 0.5',
            channelPage: "APIMAC_CHANNEL_PAGE_10"
        }
};

const defaultRegion = 'NA';
const defaultRegDomain = '0x01';
const defaultPhyModeID = getDefaultPhyMode();
const defaultChanPlanID = '1';

/*
This phyMap is a dictionary of every possible PHY.
The region maps to a ChanPlanID, then selected phyModeID,
and the settings are automatically populated based on those 3 selections.

*/
// reference this using:
// phyMap_updated2[inst.region].phyModeID[inst.phyMode].ChanPlanID[inst.chanPlanID]
const phyMap_updated2 = {
    "EU": {
        regDomain: "0x03",
        phyModeID: {
            "1": {
                ChanPlanID: {
                    "32": {
                        TotalNumChan: 69,
                        ChanCenterFreq0: 863.1,
                        ChanSpacing: 100,
                        ChanPlanName: "863_870_100",
                        FreqBand: "863-876 MHz",
                        PhyModeID: [1],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68]
                    },
                    "34": {
                        TotalNumChan: 55,
                        ChanCenterFreq0: 870.1,
                        ChanSpacing: 100,
                        ChanPlanName: "870_876_100",
                        FreqBand: "863-876 MHz",
                        PhyModeID: [1],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54]
                    },
                    "36": {
                        TotalNumChan: 125,
                        ChanCenterFreq0: 863.1,
                        ChanSpacing: 100,
                        ChanPlanName: "863_876_100",
                        FreqBand: "863-876 MHz",
                        PhyModeID: [1],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124]
                    }
                }

            },
            "3": {
                ChanPlanID: {
                    "33": {
                        TotalNumChan: 35,
                        ChanCenterFreq0: 863.1,
                        ChanSpacing: 200,
                        ChanPlanName: "863_870_200",
                        FreqBand: "863-876 MHz",
                        PhyModeID: [3, 5],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34]
                    },
                    "35": {
                        TotalNumChan: 27,
                        ChanCenterFreq0: 870.2,
                        ChanSpacing: 200,
                        ChanPlanName: "870_876_200",
                        FreqBand: "863-876 MHz",
                        PhyModeID: [3, 5],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26]
                    },
                    "37": {
                        TotalNumChan: 62,
                        ChanCenterFreq0: 863.1,
                        ChanSpacing: 200,
                        ChanPlanName: "863_876_200",
                        FreqBand: "863-876 MHz",
                        PhyModeID: [3, 5],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61]
                    },
                }
            },
            "5": {
                ChanPlanID: {
                    "33": {
                        TotalNumChan: 35,
                        ChanCenterFreq0: 863.1,
                        ChanSpacing: 200,
                        ChanPlanName: "863_870_200",
                        FreqBand: "863-876 MHz",
                        PhyModeID: [3, 5],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34]
                    },
                    "35": {
                        TotalNumChan: 27,
                        ChanCenterFreq0: 870.2,
                        ChanSpacing: 200,
                        ChanPlanName: "870_876_200",
                        FreqBand: "863-876 MHz",
                        PhyModeID: [3, 5],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26]
                    },
                    "37": {
                        TotalNumChan: 62,
                        ChanCenterFreq0: 863.1,
                        ChanSpacing: 200,
                        ChanPlanName: "863_876_200",
                        FreqBand: "863-876 MHz",
                        PhyModeID: [3, 5],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61]
                    },
                }
            },
            "Custom": {
                ChanPlanID: {
                    "Custom": {
                        TotalNumChan: 69,
                        ChanCenterFreq0: 863.1,
                        ChanSpacing: 100,
                        ChanPlanName: "863_870_100",
                        FreqBand: "863-876 MHz",
                        PhyModeID: [1]
                    }
                }

            }
        }
    },
    "NA": {
        regDomain: "0x01",
        phyModeID: {
            "2": {
                ChanPlanID: {
                    "1": {
                        TotalNumChan: 129,
                        ChanPlanName: "902_928_200",
                        ChanCenterFreq0: 902.2,
                        ChanSpacing: 200,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [2, 4],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128]
                    }
                }
            },
            "3": {
                ChanPlanID: {
                    "1": {
                        TotalNumChan: 129,
                        ChanPlanName: "902_928_200",
                        ChanCenterFreq0: 902.2,
                        ChanSpacing: 200,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [2, 4],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128]
                    }
                }
            },
            "5": {
                ChanPlanID: {
                    "2": {
                        TotalNumChan: 64,
                        ChanPlanName: "902_928_400",
                        ChanCenterFreq0: 902.4,
                        ChanSpacing: 400,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [5, 6],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63]
                    }
                }
            },
            "6": {
                ChanPlanID: {
                    "2": {
                        TotalNumChan: 64,
                        ChanPlanName: "902_928_400",
                        ChanCenterFreq0: 902.4,
                        ChanSpacing: 400,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [5, 6],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63]
                    }
                }
            },
            "8": {
                ChanPlanID: {
                    "3": {
                        TotalNumChan: 42,
                        ChanPlanName: "902_928_600",
                        ChanCenterFreq0: 902.6,
                        ChanSpacing: 600,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [8],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41]
                    }
                }
            },
            "Custom": {
                ChanPlanID: {
                    "Custom": {
                        TotalNumChan: 129,
                        ChanPlanName: "902_928_200",
                        ChanCenterFreq0: 902.2,
                        ChanSpacing: 200,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [2, 4]
                    }
                }
            }
        }
    },
    "BZ": {
        regDomain: "0x07",
        phyModeID: {
            "2": {
                ChanPlanID: {
                    "1": {
                        TotalNumChan: 129,
                        ChanPlanName: "902_928_200",
                        ChanCenterFreq0: 902.2,
                        ChanSpacing: 200,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [2, 4],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128]
                    }
                }
            },
            "3": {
                ChanPlanID: {
                    "1": {
                        TotalNumChan: 129,
                        ChanPlanName: "902_928_200",
                        ChanCenterFreq0: 902.2,
                        ChanSpacing: 200,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [2, 4],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128]
                    }
                }
            },
            "5": {
                ChanPlanID: {
                    "2": {
                        TotalNumChan: 64,
                        ChanPlanName: "902_928_400",
                        ChanCenterFreq0: 902.4,
                        ChanSpacing: 400,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [5, 6],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63]
                    },
                }
            },
            "6": {
                ChanPlanID: {
                    "2": {
                        TotalNumChan: 64,
                        ChanPlanName: "902_928_400",
                        ChanCenterFreq0: 902.4,
                        ChanSpacing: 400,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [5, 6],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63]
                    },
                }
            },
            "8": {
                ChanPlanID: {
                    "3": {
                        TotalNumChan: 42,
                        ChanPlanName: "902_928_600",
                        ChanCenterFreq0: 902.6,
                        ChanSpacing: 600,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [8],
                        ChannelMask: [0, 1, 2, 3, 4, 5, 6, 7, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41]
                    }
                }
            },
            "Custom": {
                ChanPlanID: {
                    "Custom": {
                        TotalNumChan: 129,
                        ChanPlanName: "902_928_200",
                        ChanCenterFreq0: 902.2,
                        ChanSpacing: 200,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [2, 4]
                    }
                }
            }
        }
    },
    "JP": {
        regDomain: "0x02",
        phyModeID: {
            "2": {
                ChanPlanID: {
                    "21": {
                        TotalNumChan: 38,
                        ChanPlanName: "920_928_200",
                        ChanCenterFreq0: 920.6,
                        ChanSpacing: 200,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [2],
                        ChannelMask: [9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37]
                    }
                }
            },
            "4": {
                ChanPlanID: {
                    "22": {
                        TotalNumChan: 18,
                        ChanPlanName: "920_928_400",
                        ChanCenterFreq0: 920.9,
                        ChanSpacing: 400,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [4, 5],
                        ChannelMask: [4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17]
                    }
                }
            },
            "5": {
                ChanPlanID: {
                    "22": {
                        TotalNumChan: 18,
                        ChanPlanName: "920_928_400",
                        ChanCenterFreq0: 920.9,
                        ChanSpacing: 400,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [4, 5],
                        ChannelMask: [4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17]
                    }
                }
            },
            "7": {
                ChanPlanID: {
                    "23": {
                        TotalNumChan: 12,
                        ChanPlanName: "920_928_600",
                        ChanCenterFreq0: 920.8,
                        ChanSpacing: 600,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [7, 8],
                        ChannelMask: [3, 4, 5, 6, 7, 8, 9, 10, 11]
                    }
                }
            },
            "8": {
                ChanPlanID: {
                    "23": {
                        TotalNumChan: 12,
                        ChanPlanName: "920_928_600",
                        ChanCenterFreq0: 920.8,
                        ChanSpacing: 600,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [8],
                        ChannelMask: [3, 4, 5, 6, 7, 8, 9, 10, 11]
                    }
                }
            },
            "Custom": {
                ChanPlanID: {
                    "Custom": {
                        TotalNumChan: 38,
                        ChanPlanName: "920_928_200",
                        ChanCenterFreq0: 920.6,
                        ChanSpacing: 200,
                        FreqBand: "902-928 MHz",
                        PhyModeID: [2]
                    }
                }
            }
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
function getPropPhySettings(inst)
{
    // Get all Sub-1 GHz phy defaults
    let boardSpecificSettings = getBoardPhySettings(inst);
    //return boardSpecificSettings.defaultPropPhyList;
    let setting = null

    switch(inst.phyModeID)
    {
        case "1":
            setting = boardSpecificSettings.default50kbpsList_12_5_dev;
            break;
        case "2":
            setting = boardSpecificSettings.default50kbpsList_25_dev;
            break;
        case "3":
            setting = boardSpecificSettings.default100kbpsList_25dev;
            break;
        case "4":
            setting = boardSpecificSettings.default100kbpsList_50dev;
            break;
        case "5":
            setting = boardSpecificSettings.default150kbpsList_75dev;
            break;
        case "6":
            setting = boardSpecificSettings.default200kbpsList_50dev;
            break;
        case "7":
            setting = boardSpecificSettings.default200kbpsList_100dev;
            break;
        case "8":
            setting = boardSpecificSettings.default300kbpsList_75dev;
            break;

    }

    if(setting == null)
    {
        /* if you can't find an appropriate rf module, default to 50kbps */
        setting = boardSpecificSettings.default50kbpsList;
    }

    return setting;
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
            phySettings = system.getScript("/ti/ti_wisunfan/rf_config/"
                + "CC1352P_4_LAUNCHXL_rf_defaults.js");
        }
        else if(rfDesign === "LAUNCHXL-CC1352P1")
        {
            phySettings = system.getScript("/ti/ti_wisunfan/rf_config/"
                + "CC1352P1_LAUNCHXL_rf_defaults.js");
        }
        else if(rfDesign === "LAUNCHXL-CC1352P-2")
        {
            phySettings = system.getScript("/ti/ti_wisunfan/rf_config/"
                + "CC1352P_2_LAUNCHXL_rf_defaults.js");
        }
    }
    else if (inst !== null && system.deviceData.deviceId === "CC1352P7RGZ")
    {
        // Get the RF Design configurable
        const rfDesign = inst.rfDesign;
        if(rfDesign === "LP_CC1352P7-1")
        {
            phySettings = system.getScript("/ti/ti_wisunfan/rf_config/"
                + "LP_CC1352P7_1_rf_defaults.js");
        }
        else if(rfDesign === "LP_CC1352P7-4")
        {
            phySettings = system.getScript("/ti/ti_wisunfan/rf_config/"
                + "LP_CC1352P7_4_rf_defaults.js");
        }
    }
    else if(inst !== null && system.deviceData.deviceId === "CC1354P10RSK")
    {
        // Get the RF Design configurable
        const rfDesign = inst.rfDesign;
        if(rfDesign === "LP_EM_CC1354P10_1")
        {
            phySettings = system.getScript("/ti/ti_wisunfan/rf_config/"
                + "LP_EM_CC1354P10_1_rf_defaults.js");
        }
        else if(rfDesign === "LP_EM_CC1354P10_6")
        {
            phySettings = system.getScript("/ti/ti_wisunfan/rf_config/"
                + "LP_EM_CC1354P10_6_rf_defaults.js");
        }
    }
    else
    {
        // Initialize with launchpad mapped from device
        phySettings = system.getScript("/ti/ti_wisunfan/rf_config/"
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
function getPhyTypeGroupFromRFConfig(inst, localPhyType)
{
    let rfPhyType;
    let rfPhyGroup;
    let allPhySettings;

    // Possible phyTypeXXX properties in the phy settings
    const phyTypes = ["phyType868", "phyType433", "phyType"];


    allPhySettings = getPropPhySettings(inst);
    if(allPhySettings == null)
    {
        return;
    }
    rfPhyGroup = commonRadioConfig.PHY_PROP;


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
 * ======== getRegionOptions ========
 * Generates a list of regions supported for drop down config
 *
 * @param inst - 15.4 instance (null during initialization)
 * @returns Array - array of options representing regions supported
 */
function getRegionOptions(inst)
{
    const subGReg = ["EU", "NA", "BZ", "JP"]

    const regOpt = []
    if(Common.isSub1GHzDevice())
    {
        for(const setting in subGReg)
        {
            regOpt.push(
                {
                    name: subGReg[setting],
                    displayName: subGReg[setting]
                }
            );
        }
    }

    return(regOpt);
}


/*
 * ======== getSupportedPhyModes ========
 * Generate and return array of phyModes currently supported based on
 * selected region
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @returns array - array of ints
 */
function getSupportedPhyModes(inst)
{
    if(inst.region != "Custom")
    {
        let range = [];
        let displayRange = [];

        range = phyMap[inst.region][inst.regDomain][inst.opClass].phyRange;

        for(let item of range)
        {
            if(phyModeDisplayNames[item] != undefined)
            {
                displayRange.push(phyModeDisplayNames[item].displayName);
            }
        }
        return displayRange;
    }
}


/*
 * ======== getSupportedPhyTypes ========
 * Generate and return array of phyModes currently supported based on
 * selected region
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @sets - Name of phy type (phy5kbps, phy50kbps, phy100kbps, phy200kbps, phyIEEE)

 */
function setSupportedPhyType(inst)
{
    switch(inst.phyModeID)
    {
        case "1":
            inst.phyType = "phy50kbps";
            break;
        case "2":
            inst.phyType = "phy50kbps";
            break;
        case "3":
            inst.phyType = "phy100kbps";
            break;
        case "4":
            inst.phyType = "phy100kbps";
            break;
        case "5":
            inst.phyType = "phy150kbps";
            break;
        case "6":
            inst.phyType = "phy200kbps";
            break;
        case "7":
            inst.phyType = "phy200kbps";
            break;
    }

}

/*
 * ======== setPhyModeBasedOnRegion ========
 * Generate and set array of phyModes currently supported based on
 * selected region
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @returns array - array of ints
 */
function setPhyModeBasedOnRegion(inst)
{
    inst.phyMode = getPhyModeDisplayOpt(inst);
}

/*
 * ======== getPhyModeDisplayOpt ========
 * Get the display name for the selected phyMode.
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @returns array - array of ints
 */
function getPhyModeDisplayOpt(inst)
{
    let supportedModes = getSupportedPhyModes(inst);
    const phyMode = _.first(supportedModes);
    return phyMode;

}
/*
 * ======== setPhyTypeBasedOnRegion ========
 * Generate and return array of phyModes currently supported based on
 * selected region
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @returns array - array of ints
 */
function setPhyTypeBasedOnRegion(inst)
{
    let supportedModes = setSupportedPhyType(inst);

}


/*
 * ======== setRegDomainBasedOnRegion ========
 * Generate and return array of reg domains currently supported based on
 * selected region
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @returns array - array of ints
 */
function setRegDomainBasedOnRegion(inst)
{
    let range = "0x01";

    // sub1G
    if(Common.isSub1GHzDevice())
    {
        switch(inst.region)
        {
            case "Custom":
                range = "Custom"
                break
            case "CN":
                range = "0x04"
                break
            case "EU":
                range = "0x03"
                break
            case "IN":
                range = "0x05"
                break
            case "SG":
                range = "0x0D"
                break
            case "MX":
                range = "0x06"
                break
            case "NA":
                range = "0x01"
                break
            case "BZ":
                range = "0x07"
                break
            case "AZ":
            case "NZ":
                range = "0x08"
                break
            case "KR":
                range = "0x09"
                break
            case "PH":
                range = "0x0A"
                break
            case "MY":
                range = "0x0B"
                break
            case "HK":
                range = "0x0C"
                break
            case "TH":
                range = "0x0E"
                break
            case "VN":
                range = "0x0F"
                break
            case "JP":
                range = "0x02"
                break
            case "WW":
                range = "0x00"
                break

        }

    }

    return(range);
}


function getCenterFreqBasedOnID(region, phyModeID, ChanPlanID)
{
    if (phyMap_updated2[region] && phyMap_updated2[region].phyModeID[phyModeID] &&
         phyMap_updated2[region].phyModeID[phyModeID].ChanPlanID[ChanPlanID])
    {
        return phyMap_updated2[region].phyModeID[phyModeID].ChanPlanID[ChanPlanID].ChanCenterFreq0;
    }
    return -1;
}

function getChanSpacingBasedOnID(region, phyModeID, ChanPlanID)
{
    if (phyMap_updated2[region] && phyMap_updated2[region].phyModeID[phyModeID] &&
         phyMap_updated2[region].phyModeID[phyModeID].ChanPlanID[ChanPlanID])
    {
        return phyMap_updated2[region].phyModeID[phyModeID].ChanPlanID[ChanPlanID].ChanSpacing;
    }
    return -1;
}
function getTotalNumChannelsBasedOnID(region, phyModeID, ChanPlanID)
{
    if (phyMap_updated2[region] && phyMap_updated2[region].phyModeID[phyModeID] &&
         phyMap_updated2[region].phyModeID[phyModeID].ChanPlanID[ChanPlanID])
    {
        return phyMap_updated2[region].phyModeID[phyModeID].ChanPlanID[ChanPlanID].TotalNumChan;
    }
    return -1;
}
function getPhyModeInfoBasedOnID(phyModeID)
{
    if (phyModeDisplayNames_updated[phyModeID])
    {
        return phyModeDisplayNames_updated[phyModeID].displayName;
    }
    return "Invalid mode";
}

/*
 * ======== setCenterFrequency ========
 * Set centerFrequencyDisplayName based on selected region and op class
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @returns array - array of ints
 */

function setCenterFrequency(inst)
{
    let cFreq = getCenterFreqBasedOnID(inst.region, inst.phyModeID, inst.ChanPlanID);
    inst.centerFrequency = cFreq;
}
function setChanSpacing(inst)
{
    let chanSpacing = getChanSpacingBasedOnID(inst.region, inst.phyModeID, inst.ChanPlanID);
    inst.channelSpacing = chanSpacing;
}
function setTotalNumChannels(inst)
{
    let totalChannels = getTotalNumChannelsBasedOnID(inst.region, inst.phyModeID, inst.ChanPlanID);
    inst.totalChannels = totalChannels;
}

function getSupportedPhyIDMode(region)
{
    let phyModeIDList = [];

    switch(region)
    {
        case "EU":
            phyModeIDList = ["1", "3", "5"];
            break;
        case "NA":
            phyModeIDList = ["2", "3", "5", "6", "8"];
            break;
        case "BZ":
            phyModeIDList = ["2", "3", "5", "6", "8"];
            break;
        case "JP":
            phyModeIDList = ["2", "4", "5", "7", "8"];
            break;

        //phyModeIDList = ["1", "2", "3", "4", "5", "6", "7"];
    }
    //phyModeIDList.push("Custom");
    return phyModeIDList;
}

function getSupportedOpClassOptions()
{
    let regOpt = [];
    let startVal = 1;
    let endVal = 4;
    for(startVal = 1; startVal <= endVal; startVal += 1)
        {
            let setting = startVal;
            regOpt.push(
                {
                    name: setting.toString(),
                    displayName: setting.toString()
                }
            );
        }
    return regOpt;
}

function getAllRegions()
{
    return Object.keys(phyMap_updated2);
}

/*
 * ======== getPhyModeIDOptions ========
 * Generates a list of regions supported for drop down config
 *
 * @param inst - 15.4 instance (null during initialization)
 * @returns Array - array of options representing regions supported
 */
function getPhyModeIDOptions(inst)
{
    let regOpt = [];
    let phyModeIDList = [];

    phyModeIDList = getSupportedPhyIDMode(inst.region);

    for(const setting in phyModeIDList)
        {
            regOpt.push(
                {
                    name: phyModeIDList[setting],
                    displayName: getPhyModeInfoBasedOnID(phyModeIDList[setting])
                }
            );
        }

    return(regOpt);
}


/*
 * ======== getChanPlanIDOpts ========
 * Gets the list of channel plan ID options
 *
 * @param inst - 15.4 instance (null during initialization)
 * @returns Array - array of options representing regions supported
 */
function getChanPlanIDOpts(inst)
{
    let regOpt = [];
    let chanPlanList;
    if (phyMap_updated2[inst.region] && phyMap_updated2[inst.region].phyModeID &&
        phyMap_updated2[inst.region].phyModeID[inst.phyModeID])
    {
        chanPlanList = phyMap_updated2[inst.region].phyModeID[inst.phyModeID].ChanPlanID;
        for(const setting in chanPlanList)
        {
            regOpt.push(
                {
                    name: setting,
                    displayName: setting
                }
            );
        }
    }
    return regOpt;
}

function getOpModeIDDisplayName(phyModeID)
{
    switch(phyModeID)
    {
        case "1":
            return "1a";
            break;
        case "2":
            return "1b";
            break;
        case "3":
            return "2a";
            break;
        case "4":
            return "2b";
            break;
        case "5":
            return "3";
            break;
        case "6":
            return "4a";
            break;
        case "7":
            return "4b";
            break;
        case "8":
            return "5"
            break;
    }
}

function getOpModeClass(region, phyModeID)
{
    switch(region)
    {
        case "NA":
            switch(phyModeID)
            {
                case "2":
                    return "1";
                case "3":
                    return "1";
                case "5":
                    return "2";
                case "6":
                    return "2";
                case "8":
                    return "3";
            }
        case "EU":
            switch(phyModeID)
            {
                case "1":
                    return "1";
                case "3":
                    return "2";
                case "5":
                    return "2";
            }
        case "BZ":
            switch(phyModeID)
            {
                case "2":
                    return "1";
                case "3":
                    return "1";
                case "5":
                    return "2";
                case "6":
                    return "2";
                case "8":
                    return "3";
            }
        case "JP":
            switch(phyModeID)
            {
                case "2":
                    return "1";
                case "4":
                    return "2";
                case "5":
                    return "2";
                case "7":
                    return "3";
                case "8":
                    return "3";
            }
    }
}

function setOpModeID(inst)
{
    if (getOpModeIDDisplayName(inst.phyModeID))
    {
        inst.opModeID = getOpModeIDDisplayName(inst.phyModeID);
    }
}

function setOpModeClass(inst)
{
    if (getOpModeClass(inst.region, inst.phyModeID))
    {
        inst.opModeClass = getOpModeClass(inst.region, inst.phyModeID);
    }
}

function setChanPlanID(inst)
{
    if (getChanPlanIDOpts(inst) && getChanPlanIDOpts(inst).length > 0)
    {
        inst.ChanPlanID = _.first(getChanPlanIDOpts(inst)).displayName;
    }
}

function setPhyModeID(inst)
{
    const supportedPhyModeID = getPhyModeIDOptions(inst);
    if (supportedPhyModeID && supportedPhyModeID.length > 0)
    {
        inst.phyModeID =  _.first(supportedPhyModeID).name;
    }
}

/*
 * ======== getPhyModeOptions ========
 * Generates a list of regions supported for drop down config
 *
 * @param inst - 15.4 instance (null during initialization)
 * @returns Array - array of options representing regions supported
 */
function getPhyModeOptions(inst)
{
    const ieee = getSupportedPhyModes(inst)
    const regOpt = []
    const subG = getSupportedPhyModes(inst)

    if(Common.isSub1GHzDevice())
    {
        for(const setting in subG)
        {
            regOpt.push(
                {
                    name: subG[setting],
                    displayName: subG[setting]
                }
            );
        }
    }
    return(regOpt);
}


/*
 * ======== getRegDomainOptions ========
 * Generates a list of regions supported for drop down config
 *
 * @param inst - 15.4 instance (null during initialization)
 * @returns Array - array of options representing regions supported
 */
function setRegDomain(inst)
{
    const supportedDom = setRegDomainBasedOnRegion(inst);
    inst.regDomain = supportedDom
}

/*
 * ======== getDefaultPhyMode ========
 * Retrieves the default phy Mode based on device
 *
 * @returns - name of default phy mode
 */
function getDefaultPhyMode()
{
    let defaultPhyModeID = '2';
    return defaultPhyModeID;
}

function getChannelMask(region, phyModeID, ChanPlanID)
{
    if (phyMap_updated2[region] && phyMap_updated2[region].phyModeID[phyModeID] &&
        phyMap_updated2[region].phyModeID[phyModeID].ChanPlanID[ChanPlanID])
    {
        return phyMap_updated2[region].phyModeID[phyModeID].ChanPlanID[ChanPlanID].ChannelMask;
    }
    return [];
}
function getDefaults()
{
    return [defaultRegion, defaultRegDomain, defaultPhyModeID, defaultChanPlanID];
}
exports = {
    commonSlLr5KbpsSettings: commonSlLr5KbpsSettings,
    common2Gfsk50KbpsSettings: common2Gfsk50KbpsSettings,
    common2Gfsk100KbpsSettings: common2Gfsk100KbpsSettings,
    common2Gfsk150KbpsSettings: common2Gfsk150KbpsSettings,
    common2Gfsk200KbpsSettings: common2Gfsk200KbpsSettings,
    common2Gfsk300KbpsSettings: common2Gfsk300KbpsSettings,
    commonIEEESettings: commonIEEESettings,
    mergeRFSettings: mergeRFSettings,
    getBoardPhySettings: getBoardPhySettings,
    getIEEEPhySettings: getIEEEPhySettings,
    getPropPhySettings: getPropPhySettings,
    getPhyTypeGroupFromRFConfig: getPhyTypeGroupFromRFConfig,
    getRegionOptions: getRegionOptions,
    setRegDomain: setRegDomain,
    getPhyModeOptions: getPhyModeOptions,
    setRegDomainBasedOnRegion: setRegDomainBasedOnRegion,
    setPhyModeBasedOnRegion: setPhyModeBasedOnRegion,
    setCenterFrequency: setCenterFrequency,
    setPhyTypeBasedOnRegion: setPhyTypeBasedOnRegion,
    getPhyModeDisplayOpt: getPhyModeDisplayOpt,
    setPhyModeID: setPhyModeID,
    getPhyModeIDOptions: getPhyModeIDOptions,
    getChanPlanIDOpts: getChanPlanIDOpts,
    setChanPlanID: setChanPlanID,
    getCenterFreqBasedOnID: getCenterFreqBasedOnID,
    getTotalNumChannelsBasedOnID: getTotalNumChannelsBasedOnID,
    getChanSpacingBasedOnID: getChanSpacingBasedOnID,
    setChanSpacing: setChanSpacing,
    setTotalNumChannels: setTotalNumChannels,
    getPhyModeInfoBasedOnID: getPhyModeInfoBasedOnID,
    getChannelMask: getChannelMask,
    getDefaults: getDefaults,
    getOpModeIDDisplayName: getOpModeIDDisplayName,
    setOpModeClass: setOpModeClass,
    setOpModeID: setOpModeID,
    getSupportedOpClassOptions: getSupportedOpClassOptions
};
