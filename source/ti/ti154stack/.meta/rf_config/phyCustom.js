/*
 *  ======== Common Custom PHY Settings ========
 *
 * These Objects contain common phy settings for all boards/devices
 *
 * An entry in this array must contain the following elements:
 *      - radioConfigModName: name given to PHY's radio config module by
 *                            15.4 module
 *      - moduleName: Path to IEEE vs proprietary Sub-1 GHz settings
 *      - args: Phy setting arguments to be passed to the radio config module
 *              Only required changes here are for preambleCount, syncWordLength and syncWord.
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
 *      - phy154Settings: More specific settings for the custom phy. The Phy ID will
 *                        always be the same. However, the other settings need to be set
 *                        based on which PHY the customer wish to use and what the frequency
 *                        band should be.
 
 *      Would need some changes when using 2.4GHz devices
 */

const commonCustomPhySettings = {
    radioConfigModName: "radioConfigCustom",
    moduleName: "/ti/devices/radioconfig/settings/prop",
    args: {
        txPower: "0",
        codeExportConfig: {
            symGenMethod: "Custom",
            useConst: true,
            rfMode: "RF_prop_custom",
            txPower: "txPowerTable_custom",
            txPowerSize: "TX_POWER_TABLE_SIZE_custom",
            overrides: "pOverrides_custom",
            cmdList_prop: ["cmdFs", "cmdPropTxAdv", "cmdPropRxAdv",
                "cmdPropCs"],
            cmdFs: "RF_cmdFs_custom",
            cmdPropTxAdv: "RF_cmdPropRxAdv_custom",
            cmdPropRxAdv: "RF_cmdPropTxAdv_custom",
            cmdPropCs: "RF_cmdPropCs_custom",
            useMulti: false
        }
    },
    phyDropDownOption: {
        name: "customPhy",
        displayName: "Custom PHY"
    },
    // phy154Settings are configured by sysconfig configurables
};

/*
 *  ======== Device Specific Custom PHY Settings ========
 *
 * These Objects must contain the following elements (only changes for phyType868 and cmdPropRadioDivSetup are required):
 *      - args: Phy setting arguments to be passed to the radio config module
 *          - The argument set for the property phyType868 needs to be a viable phy name
 *          - codeExportConfig: Exported code names to be passed to
 *                              radioConfig.codeExportConfig
 *              - cmdRadioSetup: Name of the generated rfc_CMD_RADIO_SETUP_t
 *                               command - There are fixed command names for all supported phys
 *              - cmdPropRadioDivSetupPa: Name of the generated
 *                      rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t command (P-devices)
 *              - paExport: Determines how PA table is exported (Only applicable for P-devices)
 *              - Only one of the objects below are used at a time. Object is chosen depending if the device
 *                is a P-device or not
 */

const devSpecificCustomPhySettings = {
    args: {
        freqBand: "868", // options: 868 or 433
        phyType868: "wbdsss480ksps120kbps", // phyType suffix must match freqBand
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetup"],
            cmdPropRadioDivSetup: "RF_cmdPropRadioDivSetup_custom"
        }
    }
};

// These settings are only used for P-devices
const devSpecificCustomPhySettingsPdevices = {
    args: {
        freqBand: "868", // options: 868 or 433
        phyType868: "wbdsss480ksps120kbps", // phyType suffix must match freqBand
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetupPa"],
            cmdPropRadioDivSetupPa: "RF_cmdPropRadioDivSetup_custom",
            paExport: "combined"
        }
    }
};

const devSpecificCustomPhySettings433 = {
    args: {
        freqBand: "433", // options: 868 or 433
        phyType433: "2gfsk50kbps154g433mhz", // phyType suffix must match freqBand
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetup"],
            cmdPropRadioDivSetup: "RF_cmdPropRadioDivSetup_custom"
        }
    }
};

// These settings are only used for P-devices
const devSpecificCustomPhySettings433Pdevices = {
    args: {
        freqBand: "433", // options: 868 or 433
        phyType433: "2gfsk50kbps154g433mhz", // phyType suffix must match freqBand
        codeExportConfig: {
            cmdList_prop: ["cmdPropRadioDivSetupPa"],
            cmdPropRadioDivSetupPa: "RF_cmdPropRadioDivSetup_custom",
            paExport: "combined"
        }
    }
};

exports = {
    commonCustomPhySettings: commonCustomPhySettings,
    devSpecificCustomPhySettings: devSpecificCustomPhySettings,
    devSpecificCustomPhySettingsPdevices: devSpecificCustomPhySettingsPdevices,
    devSpecificCustomPhySettings433: devSpecificCustomPhySettings433,
    devSpecificCustomPhySettings433Pdevices: devSpecificCustomPhySettings433Pdevices
};

