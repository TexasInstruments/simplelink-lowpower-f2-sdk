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
 *  ======== prop_rf_config_common.js ========
 */

"use strict";

/*
 *  ======== Common Proprietary PHY Settings ========
 *
 * These Objects contain common proprietary phy settings for all boards/devices
 *
 * An entry in this array must contain the following elements:
 *      - args: Phy setting arguments to be passed to the radio config module
 *          - $name: The name to be displayed in the radio config module
 *          - packetLengthConfig: Whether packets are variable or fixed length
 *          - codeExportConfig: Exported code names to be passed to
 *                              radioConfig.codeExportConfig
 *              - symGenMethod: How the names of the generated symbols are
 *                              determined. Valid values are Custom, Legacy, or
 *                              Automatic
 *              - rfMode: Name of the generated RF_Mode object
 *              - txPower: Name of the generated Tx power table
 *              - txPowerSize: Name of the generated Tx power table size
 *              - overrides: Name of the generated overrides table
 *              - cmdList_prop: List of commands to generate
 *              - cmdFs: Name of the generated rfc_CMD_FS_t command
 *              - cmdPropTx: Name of the generated rfc_CMD_PROP_TX_t
 *              - cmdPropRx: Name of the generated rfc_CMD_PROP_RX_t
 *              - cmdPropRxAdv: Name of the generated rfc_CMD_PROP_RX_ADV_t
 *              - cmdPropTxAdv (optional): Name of the generated
 *                                         rfc_CMD_PROP_TX_ADV_t
 *              - useMulti: Boolean to generate the multi-protocol
 *                                     patch
 *      - easyLinkOption (optional): The options array for the Default PHY and
 *                                   additional supported phys drop-downs in the
 *                                   EasyLink RF Settings module. Exclude if the
 *                                   phy is not supported in Easylink (e.g. BLE)
 */

// Object containing SimpleLink Long Range, 5kbps settings for all devices/board
const commonSlLr5KbpsSettings = {
    args: {
        $name: "RF_SLR5KBPS_Setting",
        packetLengthConfig: "Variable",
        codeExportConfig: {
            symGenMethod: "Custom",
            rfMode: "RF_prop_sl_lr",
            txPower: "PROP_RF_txPowerTable_sl_lr",
            txPowerSize: "RF_PROP_TX_POWER_TABLE_SIZE_SL_LR",
            overrides: "pOverrides_sl_lr",
            cmdList_prop: ["cmdFs", "cmdPropTx", "cmdPropRxAdv"],
            cmdFs: "RF_cmdFs_sl_lr",
            cmdPropTx: "RF_cmdPropTx_sl_lr",
            cmdPropRxAdv: "RF_cmdPropRxAdv_sl_lr",
            useMulti: true
        }
    },
    easyLinkOption: {
        name: "EasyLink_Phy_5kbpsSlLr",
        displayName: "5 KBPS, SimpleLink Long Range"
    }
};

// Object containing 2GFSK, 50kbps settings for the all devices/boards
const common2Gfsk50KbpsSettings = {
    args: {
        $name: "RF_2GFSK50KBPS_Setting",
        packetLengthConfig: "Variable",
        codeExportConfig: {
            symGenMethod: "Custom",
            rfMode: "RF_prop_fsk_50kbps",
            txPower: "RF_PROP_txPowerTable_fsk_50kbps",
            txPowerSize: "RF_PROP_TX_POWER_TABLE_SIZE_fsk_50kbps",
            overrides: "pOverrides_fsk_50kbps",
            cmdList_prop: ["cmdFs", "cmdPropTx", "cmdPropRxAdv"],
            cmdFs: "RF_cmdFs_fsk_50kbps",
            cmdPropTx: "RF_cmdPropTx_fsk_50kbps",
            cmdPropRxAdv: "RF_cmdPropRxAdv_fsk_50kbps",
            useMulti: true
        }
    },
    easyLinkOption: {
        name: "EasyLink_Phy_50kbps2gfsk",
        displayName: "50 KBPS, 2-GFSK"
    }
};

const common2Gfsk200KbpsSettings = {
    args: {
        $name: "RF_2GFSK200KBPS_Setting",
        packetLengthConfig: "Variable",
        codeExportConfig: {
            symGenMethod: "Custom",
            rfMode: "RF_prop_fsk_200kbps",
            txPower: "RF_PROP_txPowerTable_fsk_200kbps",
            txPowerSize: "RF_PROP_TX_POWER_TABLE_SIZE_fsk_200kbps",
            overrides: "pOverrides_fsk_200kbps",
            cmdList_prop: ["cmdFs", "cmdPropTxAdv", "cmdPropRxAdv"],
            cmdFs: "RF_cmdFs_fsk_200kbps",
            cmdPropRxAdv: "RF_cmdPropRxAdv_fsk_200kbps",
            cmdPropTxAdv: "RF_cmdPropTxAdv_fsk_200kbps",
            useMulti: true
        }
    },
    easyLinkOption: {
        name: "EasyLink_Phy_200kbps2gfsk",
        displayName: "200 KBPS, 2-GFSK"
    }
};

const common2Gfsk250Kbps24GHzSettings = {
    args: {
        $name: "RF_2GFSK250KBPS_Setting",
        freqBand: "2400", // options: 2400, 868, or 433
        phyType2400: "2gfsk250kbps", // phyType suffix must match freqBand
        packetLengthConfig: "Variable",
        codeExportConfig: {
            symGenMethod: "Custom",
            rfMode: "RF_prop_2_4G_fsk_250kbps",
            txPower: "RF_PROP_txPowerTable_2_4G_fsk_250kbps",
            txPowerSize: "RF_PROP_TX_POWER_TABLE_SIZE_2_4G_fsk_250kbps",
            overrides: "pOverrides_2_4G_fsk_250kbps",
            cmdList_prop: ["cmdFs", "cmdPropTx", "cmdPropRxAdv"],
            cmdFs: "RF_cmdFs_2_4G_fsk_250kbps",
            cmdPropRxAdv: "RF_cmdPropRxAdv_2_4G_fsk_250kbps",
            cmdPropTx: "RF_cmdPropTx_2_4G_fsk_250kbps",
            useMulti: true
        }
    },
    easyLinkOption: {
        name: "EasyLink_Phy_2_4_250kbps2gfsk",
        displayName: "250 KBPS, 2-GFSK"
    }
};

const common2Gfsk100Kbps24GHzSettings = {
    args: {
        $name: "RF_2GFSK100KBPS_Setting",
        freqBand: "2400", // options: 2400, 868, or 433
        phyType2400: "2gfsk100kbps", // phyType suffix must match freqBand
        packetLengthConfig: "Variable",
        codeExportConfig: {
            symGenMethod: "Custom",
            rfMode: "RF_prop_2_4G_fsk_100kbps",
            txPower: "RF_PROP_txPowerTable_2_4G_fsk_100kbps",
            txPowerSize: "RF_PROP_TX_POWER_TABLE_SIZE_2_4G_fsk_100kbps",
            overrides: "pOverrides_2_4G_fsk_100kbps",
            cmdList_prop: ["cmdFs", "cmdPropTx", "cmdPropRxAdv"],
            cmdFs: "RF_cmdFs_2_4G_fsk_100kbps",
            cmdPropRxAdv: "RF_cmdPropRxAdv_2_4G_fsk_100kbps",
            cmdPropTx: "RF_cmdPropTx_2_4G_fsk_100kbps",
            useMulti: true
        }
    },
    easyLinkOption: {
        name: "EasyLink_Phy_2_4_100kbps2gfsk",
        displayName: "100 KBPS, 2-GFSK"
    }
};

/*
 *  ======== Common BLE PHY Settings ========
 *
 * Array containing all the ble phy settings for a given board/device.
 *
 * An entry in this array must contain the following elements:
 *      - args: Phy setting arguments to be passed to the radio config module
 *          - $name: The name to be displayed in the radio config module
 *          - phyType: Name of the phy found in the radio config module.
 *                     Valid options: bt5le1m, bt5le1madvnc, bt5le2m,
 *                     bt5lecodeds2, bt5lecodeds8
 *          - codeExportConfig: Exported code names to be passed to
 *                              radioConfig.codeExportConfig
 *              - symGenMethod: How the names of the generated symbols are
 *                              determined. Valid values are Custom, Legacy, or
 *                              Automatic
 *              - rfMode: Name of the generated RF_Mode object
 *              - txPower: Name of the generated Tx power table
 *              - txPowerSize: Name of the generated Tx power table size
 *              - overrides: Name of the generated overrides table
 *              - cmdList_ble: List of commands to generate
 *              - cmdBle5RadioSetup: Name of the generated
 *                                   rfc_CMD_BLE5_RADIO_SETUP_t command
 *              - cmdFs: Name of the generated rfc_CMD_FS_t command
 *              - cmdBle5AdvAux: Name of the generated rfc_CMD_BLE5_ADV_AUX_t
 *                               command
 *              - cmdBle5GenericRx: Name of the generated
 *                                  rfc_CMD_BLE5_GENERIC_RX_t command
 *              - useBleAdvCmd: Boolean to generate the rfc_CMD_BLE_ADV_NC_t
 *                              command
 *              - cmdBleAdvNc: Name of the generated rfc_CMD_BLE_ADV_NC_t
 *                             command
 *              - useMulti: Boolean to generate the multi-protocol
 *                          patch
 */

// Object containing BLE settings for the all devices
const commonBleSettings = {
    args: {
        $name: "RF_BLE_Setting",
        phyType: "bt5le1m",
        codeExportConfig: {
            symGenMethod: "Custom",
            rfMode: "RF_modeBle",
            txPower: "RF_BLE_txPowerTable",
            txPowerSize: "RF_BLE_TX_POWER_TABLE_SIZE",
            overrides: "pOverrides_ble",
            cmdList_ble: ["cmdFs", "cmdBle5AdvAux", "cmdBle5GenericRx",
                "cmdBleAdvNc"],
            cmdFs: "RF_ble_cmdFs",
            cmdBle5AdvAux: "RF_ble_cmdBle5AdvAux",
            cmdBle5GenericRx: "RF_ble_cmdBleGenericRx",
            cmdBleAdvNc: "RF_ble_cmdBleAdvNc",
            useMulti: true
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
    args: {
        phyType: "ieee154",
        txPower: "0",
        codeExportConfig: {
            symGenMethod: "Custom",
            useConst: false,
            rfMode: "RF_prop_ieee154",
            txPower: "txPowerTable_ieee154",
            txPowerSize: "TX_POWER_TABLE_SIZE_ieee154",
            overrides: "pOverrides_ieee154",
            cmdList_ieee_15_4: ["cmdFs", "cmdIeeeTx",
                "cmdIeeeRx"],
            cmdFs: "RF_cmdFs_ieee154",
            cmdIeeeTx: "RF_cmdIeeeTx_ieee154",
            cmdIeeeRx: "RF_cmdIeeeRx_ieee154",
            useMulti: false
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
 *  ======== mergeRfSettings ========
 * Helper function for merging two rf settings objects into a single object
 * with correct ordering. Ordering of the arguments matters. Source objects are
 * applied from left to right. Subsequent sources overwrite property assignments
 * of previous sources.
 *
 * @param object1 - first rf setting object to be merged. Must contain an args
 * and codeExportConfig property
 * @param object2 - second rf setting object to be merged. Must contain an args
 * and codeExportConfig property
 *
 * @returns Object - An object containing all the properties from object1 and
 * object2 with the correct ordering required for the Radio Configuration module
 */
function mergeRfSettings(obj1, obj2)
{
    // Possible cmdList_ and phyTypeXXX properties in the input objects
    const cmdLists = ["cmdList_prop", "cmdList_ieee_15_4", "cmdList_ble"];
    const phyTypes = ["phyType868", "phyType433", "phyType2400"];

    // Object to contain the newly merged properties
    const mergedObj = {args: {}};

    // If there is a freqBand property set it as the first property of args
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

exports = {
    commonSlLr5KbpsSettings: commonSlLr5KbpsSettings,
    common2Gfsk50KbpsSettings: common2Gfsk50KbpsSettings,
    common2Gfsk200KbpsSettings: common2Gfsk200KbpsSettings,
    common2Gfsk100Kbps24GHzSettings: common2Gfsk100Kbps24GHzSettings,
    common2Gfsk250Kbps24GHzSettings: common2Gfsk250Kbps24GHzSettings,
    commonBleSettings: commonBleSettings,
    commonIEEESettings: commonIEEESettings,
    mergeRfSettings: mergeRfSettings
};
