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
 *  ======== ti154stack_power_config.syscfg.js ========
 */

"use strict";

// Get common utility functions
const Common = system.getScript("/ti/ti154stack/ti154stack_common.js");

// Get common rf settings
const rfCommon = system.getScript("/ti/ti154stack/rf_config/"
    + "ti154stack_rf_config_common.js");

// Get RF command handler
const CmdHandler = system.getScript("/ti/devices/radioconfig/cmd_handler.js");

// Get RF design functions
const RfDesign = system.getScript("/ti/devices/radioconfig/rfdesign");

// Get radio config parameter handler
const ParameterHandler = system.getScript("/ti/devices/radioconfig/"
    + "parameter_handler.js");

// Get power setting descriptions
const Docs = system.getScript("/ti/ti154stack/power_config/"
    + "ti154stack_power_config_docs.js");

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
        {
            name: "forceVddr",
            displayName: "Force VDDR",
            default: false,
            hidden: true,
            description: Docs.forceVddr.description,
            longDescription: Docs.forceVddr.longDescription
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
    if(inst.freqBand === "freqBandSub1")
    {
        legacyTransmitPower = String(inst.transmitPowerSubG);
    }
    else
    {
        legacyTransmitPower = String(inst.transmitPower24G);
    }

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
    const rfPhyType = rfPhySettings.phyType;
    const rfPhyGroup = rfPhySettings.phyGroup;

    // Get drop down options of RF tx power config
    const freqBand = rfCommon.getSafeFreqBand(inst);
    const txPowerOptsObj = getTxPowerRFConfig(inst, freqBand, rfPhyType,
        rfPhyGroup);
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
function getTxPowerRFConfig(inst, freqBand, phyType, phyGroup)
{
    // Retrieve launchpad type
    const board = Common.getDeviceOrLaunchPadName(true, null, inst);

    // Get the command handler for this phy instance
    const cmdHandler = CmdHandler.get(phyGroup, phyType);
    const freq = cmdHandler.getFrequency();

    // Get drop down options of RF tx power config
    // All boards support default PA
    // High PA not supported for 1312R1, 1352R1, 1352P2 (Sub-1), 26X2R1, 2652RB
    const txPowerOptions = RfDesign.getTxPowerOptions(freq, false);
    let txPowerHiOptions = {options: []};

    if(((board.includes("CC1352P1") || board.includes("CC1312PSIP")
        || board.includes("CC1352P7-1") || board.includes("CC1352P7_1")
        || board.includes("CC1354P10-1") || board.includes("CC1354P10_1")
        || board.includes("CC1311P3")) && freqBand === "freqBandSub1")
        || ((board.includes("CC1352P-2") || board.includes("CC1352P_2")
        || board.includes("CC1352P-4") || board.includes("CC1352P_4")
        || board.includes("CC1352P7-4") || board.includes("CC1352P7_4")
        || board.includes("CC1354P10-6") || board.includes("CC1354P10_6")
        || board.includes("CC2652PSIP") || board.includes("CC2651P3")
        || board.includes("CC2653P10")|| board.includes("CC2674P10")) && freqBand === "freqBand24"))
    {
        // On 1352P1 the high PA is enabled for Sub-1 GHz
        // On 2652PSIP, 1352P2, and P4 the high PA is enabled for 2.4 GHz
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
function getRFTxPowerFrom154TxPower(inst, freqBand, phyType, phyGroup)
{
    let retHighPA;
    let retTxPower;
    let retCfgName;

    // Get drop down options of RF tx power config
    const txPowerOptsObj = getTxPowerRFConfig(inst, freqBand, phyType,
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

        // Get tx power config name (independent of board as highPA on P4 is on
        // 2.4 GHz band)
        retCfgName = "txPowerHi";
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

        // Get tx power config name based on board and band
        if(Common.is433MHzDevice(inst) && freqBand === "freqBandSub1")
        {
            retCfgName = "txPower433";
        }
        else
        {
            retCfgName = "txPower";
        }
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
    const ccfg = system.modules["/ti/devices/CCFG"].$static;

    if(inst.project === "coprocessor")
    {
        if(Common.isSub1GHzDevice() && !Common.is433MHzDevice(inst))
        {
            // Verify that forceVddr configs match for CoP projects
            if(inst.forceVddr !== ccfg.forceVddr)
            {
                validation.logError(`Must match Force VDDR setting in the \
                    ${system.getReference(ccfg, "forceVddr")} module`, inst,
                "forceVddr");
            }

            const currBoard = Common.getDeviceOrLaunchPadName(true, null, inst);
            const is1352P1Board = (currBoard != null
                && currBoard.includes("P1"));

            if(inst.forceVddr && is1352P1Board)
            {
                validation.logInfo(`Enabling Force VDDR on this device will `
                    + `disable high PA`, inst, "forceVddr");
            }
        }
    }
    else
    {
        // Validate dynamic transmit power config
        const validOptions = getTxPowerConfigOptions(inst);
        Common.validateDynamicEnum(inst, validation, "transmitPower",
            validOptions);

        // Retrieve phy and phy group from rf_defaults files
        const rfPhySettings = rfCommon.getPhyTypeGroupFromRFConfig(inst);
        const rfPhyType = rfPhySettings.phyType;
        const rfPhyGroup = rfPhySettings.phyGroup;

        // Get the command handler for this phy instance
        const cmdHandler = CmdHandler.get(rfPhyGroup, rfPhyType);
        const freq = cmdHandler.getFrequency();

        // Get transmit power from RF config module
        const freqBand = rfCommon.getSafeFreqBand(inst);
        const rfTxPowerSettings = getRFTxPowerFrom154TxPower(inst, freqBand,
            rfPhyType, rfPhyGroup);
        const rfTransmitPower = rfTxPowerSettings.txPower;
        const rfHighPA = rfTxPowerSettings.highPA;

        if(inst.customPhy == true && inst.radioConfig.txPower !=
            inst.transmitPower)
        {
            validation.logWarning(`The TX power level was changed through the `
                + `Custom PHY tab instead of the Power tab. Please make sure `
                + `the Power tab transmit power level matches the Custom PHY `
                + `selected power level`, inst, "transmitPower");
        }

        // Verify that ccfg forceVddr is set if required
        if(ParameterHandler.validateTxPower(rfTransmitPower, freq, rfHighPA)
            && !ccfg.forceVddr)
        {
            validation.logWarning(`The selected TX Power requires Force VDDR`
                + `in ${system.getReference(ccfg, "forceVddr")} to be`
                + `enabled`, inst, "transmitPower");
        }

        if(rfHighPA)
        {
            validation.logInfo("The selected TX Power enables high PA ",
                inst, "transmitPower");
        }

        let board;
        if (inst === null)
        {
            board = Common.getLaunchPadFromDevice()
        }
        else
        {
            board = inst.rfDesign;
        }

        if(board.includes("CC1312PSIP"))
        {
            if (rfTransmitPower >= 16)
            {
                validation.logWarning(Docs.psipComplianceWarning, inst, "transmitPower");
            }
            else
            {
                validation.logInfo(Docs.psipComplianceWarning, inst, "transmitPower");
            }
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
    const isCoPProject = (inst.project === "coprocessor");

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
            isVisible = (inst.project.includes("sensor") && !isCoPProject);
            break;
        }
        case "forceVddr":
        {
            isVisible = (isCoPProject && Common.isSub1GHzDevice()
                && !Common.is433MHzDevice(inst));
            break;
        }
        case "transmitPower":
        {
            isVisible = !isCoPProject;
            break;
        }
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
    Common.setConfigHiddenState(inst, ui, cfgName, config.config,
        getPowerConfigHiddenState);
}

/*
 * ======== setAllPowerConfigsHiddenState ========
 * Sets the visibility of all power configs
 *
 * @param inst    - module instance
 * @param ui      - user interface object
 */
function setAllPowerConfigsHiddenState(inst, ui)
{
    Common.setAllConfigsHiddenState(inst, ui, config.config,
        getPowerConfigHiddenState);
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
    getTxPowerRFConfig: getTxPowerRFConfig,
    getRFTxPowerFrom154TxPower: getRFTxPowerFrom154TxPower,
    getTxPowerConfigOptions: getTxPowerConfigOptions,
    setPowerConfigHiddenState: setPowerConfigHiddenState,
    getPowerConfigHiddenState: getPowerConfigHiddenState,
    setAllPowerConfigsHiddenState: setAllPowerConfigsHiddenState
};
