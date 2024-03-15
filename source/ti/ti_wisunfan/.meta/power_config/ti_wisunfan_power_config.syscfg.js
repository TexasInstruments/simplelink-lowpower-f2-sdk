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
 *  ======== ti_wisunfan_power_config.syscfg.js ========
 */

"use strict";

// Get common utility functions
const Common = system.getScript("/ti/ti_wisunfan/ti_wisunfan_common.js");

// Get common rf settings
const rfCommon = system.getScript("/ti/ti_wisunfan/rf_config/"
    + "ti_wisunfan_rf_config_common.js");

// Get RF command handler
const CmdHandler = system.getScript("/ti/devices/radioconfig/cmd_handler.js");

// Get RF design functions
const RfDesign = system.getScript("/ti/devices/radioconfig/rfdesign");

// Get radio config parameter handler
const ParameterHandler = system.getScript("/ti/devices/radioconfig/"
    + "parameter_handler.js");

// Get power setting descriptions
const Docs = system.getScript("/ti/ti_wisunfan/power_config/"
    + "ti_wisunfan_power_config_docs.js");

// Configurables for the static 15.4 Power Settings group
const config = {
    displayName: "Power",
    description: "Configure power settings for radio operation",
    config: [
        {
            name: "transmitPower",
            displayName: "Transmit Power",
            options: getTxPowerConfigOptions,
            default: "0",
            description: Docs.transmitPower.description,
            longDescription: Docs.transmitPower.longDescription
        },
        {
            name: "rxOnIdle",
            displayName: "Non Sleepy Device",
            default: false,
            hidden: true,
            description: Docs.rxOnIdle.description,
            longDescription: Docs.rxOnIdle.longDescription
        },
        /* Note: transmitPowerSubG and transmitPower24G are legacy configs. In
         * order to seamlessly handle custom board changes at runtime, the new
         * trasmit power config, transmitPower, is dynamically updated with the
         * correct power level range based on the user's selected RF reference
         * board and frequency band.
         *
         * For backwards compatibility, any changes to legacy transmit power
         * configs (from legacy files) that are valid in the new config will
         * trigger an update to the corresponding new config.
         */
        {
            name: "transmitPowerSubG",
            displayName: "Legacy Transmit Power",
            default: 0,
            hidden: true,
            description: "Legacy configurable that should always be hidden",
            longDescription: "Legacy configurable that should always be hidden",
            onChange: onLegacyTransmitPowerChange
        },
        {
            name: "transmitPower24G",
            displayName: "Legacy Transmit Power",
            default: 0,
            hidden: true,
            description: "Legacy configurable that should always be hidden",
            longDescription: "Legacy configurable that should always be hidden",
            onChange: onLegacyTransmitPowerChange
        }
    ]
};

/*
 *******************************************************************************
 Power Group Config Functions
 *******************************************************************************
 */

/*
 * ======== onLegacyTransmitPowerChange ========
 * On change function to ensure backwards compatibility between former tx power
 * configs (transmitPowerSubG, transmitPower24G) and current, dynamically
 * updated tx power config (transmitPower)
 *
 * @param inst - 15.4 module instance
 * @param ui   - user interface object
 */
function onLegacyTransmitPowerChange(inst, ui)
{
    // Retrieve list of valid transmit power from new mask config
    const currTxPowerOptions = getTxPowerConfigOptions(inst);
    const txPowerLevelsSupported = _.map(currTxPowerOptions, "name");

    let legacyTransmitPower;
    
    legacyTransmitPower = String(inst.transmitPowerSubG);
    
    // Accept only transmit power level from legacy config that are valid
    // Update current transmit power config with value from legacy config
    if(_.includes(txPowerLevelsSupported, legacyTransmitPower))
    {
        inst.transmitPower = legacyTransmitPower;
    }
}

/*!
 * ======== getTxPowerConfigOptions ========
 * Get list of available Tx power values based on board and band set
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @returns - list of transmit power options available
 */
function getTxPowerConfigOptions(inst)
{
    // Retrieve phy and phy group from rf_defaults files
    const rfPhySettings = rfCommon.getPhyTypeGroupFromRFConfig(inst);
    if (!rfPhySettings)
    {
        return [];
    }
    const phyType = rfPhySettings.phyType;
    const phyGroup = rfPhySettings.phyGroup;

    const txPowerOptsObj = getTxPowerRFConfig(inst, phyType,
        phyGroup);
    const txPowerHiOpts = txPowerOptsObj.txPowerHi;
    const txPowerOpts = txPowerOptsObj.txPower;

    let txPowerValueList = _.unionWith(txPowerOpts, txPowerHiOpts, _.isEqual);

    // Round all tx power values
    _.forEach(txPowerValueList, (option) =>
    {
        // option.name = _.round(option.name);
        option.name = `${_.round(option.name)}`;
    });

    // Remove any duplicates
    txPowerValueList = _.uniqBy(txPowerValueList, "name");

    // Sort values in descending order
    txPowerValueList = _.orderBy(txPowerValueList, (opt) => Number(opt.name),
        "desc");

    return(txPowerValueList);
}

/*!
 *  ======== getTxPowerRFConfig ========
 *  Get the transmit power value options list from radio config module
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @param phyType - value set in phyType config in radio config module
 * @param phyGroup - String. Either "prop" or "ieee_15_4" depending on selected
 *                   frequency band
 * @returns - object with tx power config options for high and default PA
 */
function getTxPowerRFConfig(inst, phyType, phyGroup)
{
    // Retrieve launchpad type
    let board;
    if(inst === null)
    {
        board = Common.getLaunchPadFromDevice();
    }
    else
    {
        board = inst.rfDesign;
    }
    
    // Get the command handler for this phy instance
    const cmdHandler = CmdHandler.get(phyGroup, phyType);
    const freq = cmdHandler.getFrequency();

    // Get drop down options of RF tx power config
    // All boards support default PA
    // High PA not supported for 1312R1, 1352R1, 1352P2 (Sub-1), 26X2R1, 2652RB
    const txPowerOptions = RfDesign.getTxPowerOptions(freq, false);
    let txPowerHiOptions = {options: []};

    if(board.includes("CC1352P1") || board.includes("CC1312PSIP")
        || board.includes("CC1352P-4") || board.includes("CC1352P_4")
        || board.includes("CC1352P7-1") || board.includes("CC1352P7_1")
        || board.includes("CC1354P10-1") || board.includes("CC1354P10_1")
        || board.includes("CC1354P10-6") || board.includes("CC1354P10_6"))
    {
        // 1352P1 and 1352P4 have high PA in Sub-1 GHz
        txPowerHiOptions = RfDesign.getTxPowerOptions(freq, true);
    }
    else if((board.includes("CC1352P-2") || board.includes("CC1352P_2")))
    {
        // On 1352P2 the high PA is enabled for 2.4 GHz
        txPowerHiOptions = RfDesign.getTxPowerOptions(freq, true);
    }

    return{
        txPower: txPowerOptions,
        txPowerHi: txPowerHiOptions
    };
}

/*!
 *  ======== getRFTxPowerFrom154TxPower ========
 * Returns an object containing parameters that must be set in radio config
 * module to set radio config tx power to that of 15.4 transmit power
 *
 * @param isSub1BandSet - Boolean. Determines whether to return the default
 *                        power option for subG or IEEE bands
 * @param inst          - Module instance containing the config that changed
 * @returns                 - Object that holds params corresponding to tx power
 *                            set in radio config module
 *                              - cfgName: tx power config name
 *                                         Valid options: txPower, txPower433,
 *                                                        txPowerHi,
 *                                                        txPower433Hi
 *                              - highPA: true/false if supported; undefined
 *                                        otherwise
 *                              - txPower: unrounded tx power value
 */
function getRFTxPowerFrom154TxPower(inst, phyType, phyGroup)
{
    let retHighPA;
    let retTxPower;
    let retCfgName;

    // Get drop down options of RF tx power config
    const txPowerOptsObj = getTxPowerRFConfig(inst, phyType,
        phyGroup);
    const txPowerHiOpts = txPowerOptsObj.txPowerHi;
    const txPowerOpts = txPowerOptsObj.txPower;
    const transmitPower = getSafeTransmitPower(inst);

    // Find the first value that is less than current transmit power since
    // transmit power values in drop down are rounded up. Will be undefined
    // if config not found (e.g. high PA not supported for set band and board)
    const mappedRFTxPowerHi = _.find(txPowerHiOpts,
        (option) => (Number(option.name) <= Number(transmitPower)));

    const mappedRFTxPower = _.find(txPowerOpts,
        (option) => (Number(option.name) <= Number(transmitPower)));

    // Check if current tx power is valid with high PA since high PA will be
    // used to transmit if power level is available for both default and high PA
    if(!_.isUndefined(mappedRFTxPowerHi))
    {
        // txPowerHi config only available on high PA boards
        retHighPA = true;
        retTxPower = mappedRFTxPowerHi;

        // Get tx power config name based on board
        retCfgName = Common.is433MHzDevice(inst) ? "txPower433Hi" : "txPowerHi";
    }
    else
    {
        if(Common.isHighPADevice())
        {
            // Must set to false for high PA-capable devices to disable high PA
            retHighPA = false;
        }
        else
        {
            // Set to undefined for devices that do not support high PA
            retHighPA = undefined;
        }

        retTxPower = mappedRFTxPower;

        // Get tx power config name based on board
        retCfgName = Common.is433MHzDevice(inst) ? "txPower433" : "txPower";
    }

    return{
        cfgName: retCfgName,
        highPA: retHighPA,
        txPower: retTxPower.name
    };
}

/*!
 * ======== getSafeTransmitPower ========
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
function getSafeTransmitPower(inst)
{
    const validOptions = getTxPowerConfigOptions(inst);
    const transmitPower = Common.getSafeDynamicConfig(inst, "transmitPower",
        "0", validOptions);

    return(transmitPower);
}

/*
 * ======== validate ========
 * Validate this inst's configuration
 *
 * Verify that Force VDDR is on if at required transmit power level.
 *
 * @param inst       - RF Settings instance to be validated
 * @param validation - object to hold detected validation issues
 */
function validate(inst, validation)
{
    // Validate dynamic transmit power config
    const validOptions = getTxPowerConfigOptions(inst);
    Common.validateDynamicEnum(inst, validation, "transmitPower", validOptions);

    // Retrieve phy and phy group from rf_defaults files
    const rfPhySettings = rfCommon.getPhyTypeGroupFromRFConfig(inst);
    if (!rfPhySettings)
    {
        return;
    }
    const phyType = rfPhySettings.phyType;
    const phyGroup = rfPhySettings.phyGroup;

    // Get the command handler for this phy instance
    const cmdHandler = CmdHandler.get(phyGroup, phyType);
    const freq = cmdHandler.getFrequency();

    const rfTxPowerSettings = getRFTxPowerFrom154TxPower(inst,
        phyType, phyGroup);
    const rfTransmitPower = rfTxPowerSettings.txPower;
    const rfHighPA = rfTxPowerSettings.highPA;

    // Verify that ccfg forceVddr is set if required
    const ccfg = system.modules["/ti/devices/CCFG"].$static;

    if(ParameterHandler.validateTxPower(rfTransmitPower, freq, rfHighPA)
        && !ccfg.forceVddr)
    {
        validation.logWarning(`The selected RF TX Power requires `
                + `${system.getReference(ccfg, "forceVddr")} to be enabled in `
                + `the Device Configuration module`, inst, "transmitPower");
    }

    if(rfHighPA)
    {
        validation.logInfo("The selected RF TX Power enables high PA ", inst,
            "transmitPower");
    }

    let board;
    if(inst === null)
    {
        board = Common.getLaunchPadFromDevice();
    }
    else
    {
        board = inst.rfDesign;
    }
    if(board.includes("CC1312PSIP"))
    {
        if (rfTransmitPower >= 18)
        {
            validation.logWarning(Docs.psipComplianceWarning, inst, "transmitPower");
        }
        else
        {
            validation.logInfo(Docs.psipComplianceWarning, inst, "transmitPower");
        }
    }

}

/*
 * ======== getPowerConfigHiddenState ========
 * Get the expected visibility of the selected config
 *
 * @param inst    - module instance containing the config that changed
 * @param cfgName - name of config
 * @returns bool  - true if hidden, false if visible
 */
function getPowerConfigHiddenState(inst, cfgName)
{
    let isVisible = true;
    switch(cfgName)
    {
        case "transmitPowerSubG":
        case "transmitPower24G":
        {
            // Legacy transmit power configs that should always remain hidden
            isVisible = false;
            break;
        }
        case "rxOnIdle":
        {
            isVisible = false;
            break;
        }
        case "transmitPower":
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
 * ======== setPowerConfigHiddenState ========
 * Sets the visibility of the selected config
 *
 * @param inst    - module instance containing the config that changed
 * @param ui      - user interface object
 * @param cfgName - name of config
 */
function setPowerConfigHiddenState(inst, ui, cfgName)
{
    // Set visibility of config
    ui[cfgName].hidden = getPowerConfigHiddenState(inst, cfgName);
    if(ui[cfgName].hidden)
    {
        // get a list of all nested and unnested configs
        const configToReset = Common.findConfig(config.config, cfgName);
        // restore the default value for the hidden parameter.
        Common.restoreDefaultValue(inst, configToReset, cfgName);
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
    getRFTxPowerFrom154TxPower: getRFTxPowerFrom154TxPower,
    getTxPowerConfigOptions: getTxPowerConfigOptions,
    setPowerConfigHiddenState: setPowerConfigHiddenState,
    getPowerConfigHiddenState: getPowerConfigHiddenState
};
