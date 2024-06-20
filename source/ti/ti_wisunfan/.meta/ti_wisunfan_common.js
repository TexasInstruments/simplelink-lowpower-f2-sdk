/*
 * Copyright (c) 2017-2019 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== ti_wisunfan_common.js ========
 */

"use strict";

// Max values for C types
const cTypeMax = {
    u_int8: 255,
    u_int16: 65535,
    u_int32: 4294967295,
    int8_t: 127
};

// Settings for ti/devices/CCFG module
const ti_wisunfanCCFGSettings = {
    CC1312R1_LAUNCHXL_CCFG_SETTINGS: {
        forceVddr: false
    },
    LP_EM_CC1312PSIP_CCFG_SETTINGS: {
        forceVddr: false
    },
    CC1352R1_LAUNCHXL_CCFG_SETTINGS: {
        forceVddr: false
    },
    CC1352P1_LAUNCHXL_CCFG_SETTINGS: {
        forceVddr: false
    },
    CC1352P_2_LAUNCHXL_CCFG_SETTINGS: {
        forceVddr: false
    },
    CC1352P_4_LAUNCHXL_CCFG_SETTINGS: {
        forceVddr: false
    },
    LP_CC1312R7_CCFG_SETTINGS: {
        forceVddr: false
    },
    LP_EM_CC1312PSIP_CCFG_SETTINGS: {
        forceVddr: false
    },
    LP_CC1352P7_1_CCFG_SETTINGS: {
        forceVddr: false
    },
    LP_CC1352P7_4_CCFG_SETTINGS: {
        forceVddr: false
    },
    LP_EM_CC1314R10_CCFG_SETTINGS: {
        forceVddr: false
    },
    LP_EM_CC1354P10_1_CCFG_SETTINGS: {
        forceVddr: false
    },
    LP_EM_CC1354P10_6_CCFG_SETTINGS: {
        forceVddr: false
    },
    LP_CC1354R10_RGZ_CCFG_SETTINGS: {
        forceVddr: false
    },
};

// Dictionary mapping a device name to default LaunchPad
const deviceToBoard = {
    CC1352P7: "LP_CC1352P7_1",
    CC1312R7: "LP_CC1312R7",
    CC1352R: "CC1352R1_LAUNCHXL",
    CC1352P: "CC1352P1_LAUNCHXL",
    CC1312R: "CC1312R1_LAUNCHXL",
    CC1312PSIP: "LP_EM_CC1312PSIP",
    CC1314R10: "LP_EM_CC1314R10",
    CC1354P10: "LP_EM_CC1354P10_1",
    CC1354R10: "LP_CC1354R10_RGZ",
    CC2652R1: "CC26X2R1_LAUNCHXL",
    CC2652RB: "CC2652RB_LAUNCHXL"
};

const boardName = getDeviceOrLaunchPadName(true);
const ccfgSettings = ti_wisunfanCCFGSettings[boardName + "_CCFG_SETTINGS"];

/*!
 *  ======== getDeviceOrLaunchPadName ========
 *  Get the name of the board (or device)
 *
 *  @param convertToBoard - Boolean. When true, return the associated LaunchPad
 *                          name if a device is being used without a LaunchPad
 *
 *  @returns String - Name of the board with prefix /ti/boards and
 *                    suffix .syscfg.json stripped off.  If no board
 *                    was specified, the device name is returned.
 */
function getDeviceOrLaunchPadName(convertToBoard)
{
    let name = system.deviceData.deviceId;

    if(system.deviceData.board != null)
    {
        name = system.deviceData.board.source;

        /* Strip off everything up to and including the last '/' */
        name = name.replace(/.*\//, "");

        /* Strip off everything after and including the first '.' */
        name = name.replace(/\..*/, "");
    }

    // Check if this is a standalone device without a LaunchPad
    if(convertToBoard && !name.includes("LAUNCHXL"))
    {
        name = getLaunchPadFromDevice();
    }

    return(name);
}

/*!
 * ======== isSub1GHzDevice ========
 * Returns whether device supports Sub-1 GHz frequencies
 *
 * @returns - Boolean
 */
function isSub1GHzDevice()
{
    const board = getLaunchPadFromDevice();
    return(board.includes("CC13"));
}

/*!
 * ======== is433MHzDevice ========
 * Returns whether device supports 433 MHz frequency band
 *
 * @param inst - 15.4 instance (null during initialization - uses device)
 * @returns - Boolean
 */
function is433MHzDevice(inst)
{
    let board = getLaunchPadFromDevice();
    if(inst !== null)
    {
        board = inst.rfDesign;
    }

    return(board.includes("P-4") || board.includes("P_4")
        || board.includes("P7-4") || board.includes("P7_4"));
}

/*!
 *  ======== isHighPADevice ========
 *  Returns whether device supports high PA
 *
 *  @returns - Boolean
 */
function isHighPADevice()
{
    const board = getLaunchPadFromDevice(boardName);
    return(board.includes("CC1352P") || board.includes("CC1312P"));
}

/*!
 * ======== getLaunchPadFromDevice ========
 * Get the launchpad mapped to the device. With the exception of P-boards, all
 * devices have a 1 to 1 mapping with a launchpad. Note that P-devices default
 * to a P1 launchpad
 *
 *  @returns String - board that corresponds to device
 */
function getLaunchPadFromDevice()
{
    let name = system.deviceData.deviceId;

    // Find the LaunchPad name in deviceToBoard dictionary
    let key = null;
    for(key in deviceToBoard)
    {
        if(name.includes(key))
        {
            name = deviceToBoard[key];
            break;
        }
    }

    return(name);
}

/*!
 *  ======== toHexString ========
 *  Convert decimal number to hexadecimal string
 *
 *  @param number - decimal number to be converted
 *  @param padding - length of padding to be added
 *  @returns String - hexadecimal string representation of decimal value
 */
function toHexString(number, padding)
{
    let str = _.toUpper(number.toString(16));
    if(padding)
    {
        str = _.padStart(str, padding, "0");
    }
    return("0x" + str);
}

/*!
 *  ======== isPositiveInteger ========
 *  Returns whether number passed is a positive whole number
 *
 *  @param value - number to be verified
 *  @returns Boolean - true if positive whole number, false otherwise
 */
function isPositiveInteger(value)
{
    return(Number.isInteger(value) && value >= 0);
}

/*!
 *  ======== validateRange ========
 *  Validates value of config is within range passed
 *
 *  @param inst       - module instance containing the config to be validated
 *  @param validation - object to hold detected validation issues
 *  @param cfgName    - name of config to be validated
 *  @param min        - minimum value allowed
 *  @param max        - maximum value allowed
 *  @param transform  - function to transform value to hex/decimal
 *  @param integer    - config expected to be an integer
 *  @returns Boolean  - true if value does not fall within range
 */
function validateRange(inst, validation, cfgName, min, max, transForm,
    integer = true)
{
    const value = inst[cfgName];

    if((integer === true) && (!isPositiveInteger(value)))
    {
        validation.logError("Must be a whole number", inst, cfgName);
    }

    if(value < min || value > max)
    { // not in range
        validation.logError(`Must be between ${transForm(min)} and `
            + `${transForm(max)}`, inst, cfgName);
    }

    return(!((value < min) || (value > max)));
}

/*!
 *  ======== validateRangeHex ========
 *  Validates the value of config is within range passed and prints error
 *  message, if any, with hexadecimal values
 *
 *  @param inst       - module instance containing the config to be validated
 *  @param validation - object to hold detected validation issues
 *  @param cfgName    - name of config to be validated
 *  @param min        - minimum value allowed
 *  @param max        - maximum value allowed
 *  @returns Boolean  - false if value does not fall within range
 */
function validateRangeHex(inst, validation, cfgName, min, max)
{
    return(validateRange(inst, validation, cfgName, min, max, toHexString));
}

/*!
 *  ======== validateRangeInt ========
 *  Validates the value of config is within range passed and prints error
 *  message, if any, with integer values
 *
 *  @param inst       - module instance containing the config to be validated
 *  @param validation - object to hold detected validation issues
 *  @param cfgName    - name of config to be validated
 *  @param min        - minimum value allowed
 *  @param max        - maximum value allowed
 *  @returns Boolean  - false if value does not fall within range
 */
function validateRangeInt(inst, validation, cfgName, min, max)
{
    return(validateRange(inst, validation, cfgName, min, max, Number));
}

/*!
 *  ======== validateRangeDecimal ========
 *  Validates the value of config is within range passed and prints error
 *  message, if any, with decimal values
 *
 *  @param inst       - module instance containing the config to be validated
 *  @param validation - object to hold detected validation issues
 *  @param cfgName    - name of config to be validated
 *  @param min        - minimum value allowed
 *  @param max        - maximum value allowed
 *  @returns Boolean  - false if value does not fall within range
 */
function validateRangeDecimal(inst, validation, cfgName, min, max)
{
    return(validateRange(inst, validation, cfgName, min, max, Number, false));
}

/*!
 * ======== validateDynamicEnum ========
 * Verifies that the selected option in the dynamic drop down config is valid
 * If non-null validation and instance are passed will generate error
 *
 * @param inst       - module instance containing the config to be validated
 * @param validation - object to hold detected validation issues
 * @param validOpts - current options displayed in the config
 * @param cfgName - name of config to be validated
 */
function validateDynamicEnum(inst, validation, cfgName, validOpts)
{
    const selectedOpt = inst[cfgName];
    const found = _.find(validOpts, (o) => o.name === selectedOpt);

    if(inst !== null && validation !== null && !found)
    {
        validation.logError("Previous option is not supported by the current "
            + "configuration. Please re-select option", inst, cfgName);
    }

    return(found);
}

/*!
 * ======== validateDynamicMultiEnum ========
 * Verifies that the selected option in the multi-selection dynamic drop down
 * config are valid. If non-null validation and instance are passed will
 * generate error
 *
 * @param inst       - module instance containing the config to be validated
 * @param validation - object to hold detected validation issues
 * @param cfgName - name of config to be validated
 * @param selectedOpt - value of config to be validated
 * @param validOpts - current options displayed in the config
 */
function validateDynamicMultiEnum(inst, validation, cfgName, selectedOpt,
    validOpts)
{
    const arraySupported = _.map(validOpts, "name");
    const validArrayOptsSelected = _.intersection(arraySupported,
        selectedOpt);

    const valid = (_.isEqual(_.sortBy(selectedOpt),
        _.sortBy(validArrayOptsSelected)));

    if(inst !== null && validation !== null && !valid)
    {
        validation.logError("Selected option is invalid", inst, cfgName);
    }

    return{
        validOptsSelected: validArrayOptsSelected,
        valid: valid
    };
}

/*!
 * ======== getSafeDynamicConfig ========
 * Safely retrieve the value of the config by returning the instance value it's
 * valid, otherwise returns the default value.
 *
 * Due to their nature, dynamic enum configurables may be incorrectly modified
 * through the .syscfg file. While all dynamic configs have validation functions
 * to detect such errors, the dependency of the radio config module requires
 * safe access to certain dynamic configs to avoid SysConfig breaks.
 *
 * @param inst - 15.4 module instance (null during initialization)
 * @returns - config value in instance (if valid), otherwise config default
 */
function getSafeDynamicConfig(inst, cfgName, defaultValue, validOptions)
{
    // Access instance value
    let safeConfig = defaultValue;
    if(!_.isNil(inst))
    {
        // Access instance value
        safeConfig = inst[cfgName];

        // Verify config value without raising GUI error (handled in validate())
        const valid = validateDynamicEnum(inst, null, cfgName, validOptions);
        if(!valid)
        {
            safeConfig = defaultValue;
        }
    }

    return(safeConfig);
}

/*!
 *  ======== convertToCByteArray ========
 *  Turns an array specifying bits (starting from bit 0) to a byte array with
 *  the specified bits set. LSB is the 0th byte of the array.
 *  Example:
 *  bitsArray = [1,8]
 *  corresponding c byte array
 *  cByteArray = [2,1,0,0,0,0,0,0,0,0,0,.. up to num bytes]
 *
 *  @param bitsArray - array of bits to be set, e.g. [1,8]
 *  @param numBytes  - total number of bytes
 *  @returns Array   - byte array with specified bits set
 */
function convertToCByteArray(bitsArray, numBytes)
{
    const arr = _.fill(Array(numBytes), 0); // array filled with number of 0's
    _.each(bitsArray, (bit) =>
    {
        const arrIndex = Math.floor(bit / 8);
        const arrBit = bit - (arrIndex * 8);
        arr[arrIndex] |= (1 << arrBit);
    });
    return(arr);
}

/*!
 *  ======== channelMaskCHexStrArr ========
 *  Returns array of bytes where each byte is presented as a hexadecimal string
 *  Example: ["0x02","0x00",...]
 *
 *  @param channelMask - channels selected in config
 *  @returns String    - array of hexadecimal strings representing channel mask
 */
function channelMaskCHexStrArr(channelMask)
{
    const channelMaskCByteArr = convertToCByteArray(channelMask, 17);

    return(_.map(channelMaskCByteArr, (byte) => toHexString(byte, 2)));
}

/*
 * ======== findConfig ========
 * Finds and returns the configurable with the matching provided name
 *
 * @param config  - A module's configurable array
 * @param configName - The name of the configurable to search for
 *
 * @returns - undefined if the configurable is not found, otherwise the entire
 *            configurable object
 */
function findConfig(config, configName)
{
    let enumDef;

    let element = null;
    for(element of config)
    {
        // If the element contains a group, need to search it's configurables
        if("config" in element)
        {
            // Recursively search the sub-groups config array
            enumDef = findConfig(element.config, configName);

            // Stop searching if the configurable was found in the sub-group
            if(enumDef !== undefined)
            {
                break;
            }
        }
        else if(element.name === configName)
        {
            // Stop searching if the current element is the correct configurable
            enumDef = element;
            break;
        }
    }

    return(enumDef);
}


/*
 * ======== restoreDefaultValue ========
 * Restore the configurable's value to the default.
 *
 * @param inst  - The module instance
 * @param _cfg - the config that will be restored to default
 * @param cfgName - The name of the configurable to change back to default
 *
 * @returns - void
 */
function restoreDefaultValue(inst, _cfg, cfgName)
{
    /* if you find a match, set the default and return */
    if(_cfg.name === cfgName)
    {
        /* Make sure a default exists */
        if(_cfg.default !== null && _cfg.default !== undefined)
        {
            /* Set the instance based on the default. */
            inst[cfgName] = _cfg.default;
        }
    }
}

/*
 * ======== findAllConfigs ========
 * Finds and returns a list of all configurables within array
 *
 * @param configArray - A module's configurable arrays
 * @returns - list of names of all configurable objects within array
 */
function findAllConfigs(configArray)
{
    let element = null;
    let allConfigs = [];

    for(element of configArray)
    {
        // If the element contains a group, need to search it's configurables
        if("config" in element)
        {
            // Recursively search the sub-groups config array
            allConfigs = allConfigs.concat(findAllConfigs(element.config));
        }
        else if(element.name !== undefined)
        {
            // Add to list if the current element is a configurable
            allConfigs.push(element.name);
        }
    }

    return(allConfigs);
}

exports = {
    isSub1GHzDevice: isSub1GHzDevice,
    is433MHzDevice: is433MHzDevice,
    ccfgSettings: ccfgSettings,
    isHighPADevice: isHighPADevice,
    getLaunchPadFromDevice: getLaunchPadFromDevice,
    cTypeMax: cTypeMax,
    toHexString: toHexString,
    validateRangeHex: validateRangeHex,
    validateRangeInt: validateRangeInt,
    validateRangeDecimal: validateRangeDecimal,
    validateDynamicEnum: validateDynamicEnum,
    validateDynamicMultiEnum: validateDynamicMultiEnum,
    channelMaskCHexStrArr: channelMaskCHexStrArr,
    findConfig: findConfig,
    findAllConfigs: findAllConfigs,
    restoreDefaultValue: restoreDefaultValue,
    getSafeDynamicConfig: getSafeDynamicConfig
};
