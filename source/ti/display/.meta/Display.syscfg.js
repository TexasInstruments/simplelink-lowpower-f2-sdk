/*
 * Copyright (c) 2018-2024, Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== Display.syscfg.js ========
 */

"use strict";

let Common   = system.getScript("/ti/drivers/Common.js");
let UART     = system.getScript("/ti/drivers/UART2");
let logError = Common.logError;

let config = [
    {
        name        : "displayType",
        displayName : "Display Type",
        default     : "UART",
        onChange    : onChange,
        options : [
            {name: "UART"},
            {name: "LCD"},
            {name: "Host"}
        ]
    },
    {
        name: "displayImplementation",
        displayName: "Display Implementation",
        default: "DisplayUart2",
        options: [
            {name: "DisplayUart2"},
            {name: "DisplaySharp"},
            {name: "DisplayHost"}
        ],
        readOnly: true,
        description: "Displays Display delegates available for the " +
            system.deviceData.deviceId + " device and Display Type.",
        longDescription: "Displays Display delegates available for the " +
            system.deviceData.deviceId + " device and the " +
            "__Display Type__.\n\n" + `
Since there is only one delegate for each Display Type, it is a read-only
value that cannot be changed. Please refer to the
[__Display driver table__][0] for further documentation.

[0]: /drivers/doxygen/html/index.html#display
`
    },
    {
        name        : "lcdSize",
        displayName : "LCD Size",
        description : "Size of the LCD in pixels.",
        default     : 128,
        hidden      : true
    },
    {
        name        : "lcdFont",
        displayName : "LCD Font",
        description : "Font to be used on LCD.",
        default     : "Fixed",
        onChange    : onChange,
        hidden      : true,
        options     : [
            { name : "Fixed", description: "Custom designed 6x8 fixed-point font" },
            { name : "CMTT", description: "Computer Modern Typewriter font, compressed with a pixel RLE scheme. Multiple font sizes are available."},
            { name : "Custom" }
        ]
    },
    {
        name        : "lcdFontSize",
        displayName : "LCD Font Size",
        description : "Font size to be used on LCD. Valid options are even integers between 12 and 48, both included.",
        default     : 12,
        hidden      : true
    },
    {
        name        : "lcdCustomFontName",
        displayName : "Custom Font Name",
        description : "Name of the symbol containing a custom font. The Graphics_Font or Graphics_FontEx structure must be used.",
        default     : "",
        hidden      : true
    },
    {
        name        : "uartBufferSize",
        displayName : "UART Buffer Size",
        description : "UART display buffer size in bytes",
        default     : 1024
    },
    {
        name        : "enableANSI",
        displayName : "Enable ANSI",
        description: "Enables or disables ANSI cursor support.",
        default     : false
    },
    {
        name        : "maxPrintLength",
        displayName : "Maximum Print Length",
        default     : 256,
        hidden      : true
    },
    {
        name        : "mutexTimeout",
        displayName : "Mutex Timeout",
        description : "Timeout to acquire mutex for synchronizing access to UART.",
        default     : "Never Timeout",
        onChange    : onChangeMutexTimeout,
        options     : [
            { name : "Never Timeout" },
            { name : "Never Wait" },
            { name : "Custom", description: "Custom number of RTOS ticks" }
        ]
    },
    {
        name        : "mutexTimeoutValue",
        displayName : "Mutex Timeout Value",
        hidden      : true,
        default     : 1
    },
    {
        name        : "baudRate",
        displayName : "Baud Rate",
        description : "UART baud rate",
        default     : 115200,
        options     : [
            { name:   1200 },
            { name:   2400 },
            { name:   4800 },
            { name:   9600 },
            { name:  19200 },
            { name:  38400 },
            { name:  57600 },
            { name: 115200 },
            { name: 230400 },
            { name: 460800 },
            { name: 921600 }
        ]
    }
];

/*
 *  ======== pinmuxRequirements ========
 *  Returns peripheral pin requirements of the specified instance
 */
function pinmuxRequirements(inst)
{
    if (inst.displayType !== "LCD") {
        return [];
    }

    let lcdPins = [
        {
            name: "lcdEnablePin",
            displayName: "LCD Enable",
            interfaceName: "GPIO",
            signalTypes: ["DOUT"]
        },
        {
            name: "lcdPowerPin",
            displayName: "LCD Power",
            interfaceName: "GPIO",
            signalTypes: ["DOUT"]
        },
        {
            name: "lcdCSPin",
            legacyNames: ["lcdSSPin"],
            displayName: "LCD Chip Select",
            interfaceName: "GPIO",
            signalTypes: ["DOUT"]
        }
    ];

    /* If we have hardware, require the specific pins instead of generic DOUT pins */
    if (inst.$hardware) {
        lcdPins[0].signalTypes = ["LCD_ENABLE"];
        lcdPins[1].signalTypes = ["LCD_POWER"];
        lcdPins[2].signalTypes = ["SPI_CSN"];
    }

    return lcdPins;
}

/*
 * ======== moduleInstances ========
 */
function moduleInstances(inst)
{
    if (inst.displayType === "UART") {

        let displayName = "UART";

        let moduleName = "/ti/drivers/UART2";
        if (inst.$hardware && inst.$hardware.displayName) {
            displayName = inst.$hardware.displayName;
        }

        return ([
            {
                name       : "uart",
                displayName: displayName,
                moduleName : moduleName,
                hardware   : inst.$hardware
            }
        ]);
    }

    if (inst.displayType === "LCD") {

        let enableName = "LCD Enable";
        let enableHardware = null;
        let powerName = "LCD Power";
        let powerHardware = null;
        let selectName = "LCD Chip Select";
        let selectHardware = null;

        let shortName = inst.$name.replace("CONFIG_", "");

        /* Speculatively get hardware and displayName */
        if (inst.$hardware && inst.$hardware.subComponents) {
            let components = inst.$hardware.subComponents;

            enableHardware = components.ENABLE;
            powerHardware = components.POWER;
            selectHardware = components.SELECT;

            if (enableHardware && enableHardware.displayName) {
                enableName = enableHardware.displayName;
            }
            if (powerHardware && powerHardware.displayName) {
                powerName = powerHardware.displayName;
            }
            if (selectHardware && selectHardware.displayName) {
                selectName = selectHardware.displayName;
            }
        }

        return ([
            {
                name: "lcdEnable",
                displayName: enableName,
                moduleName: "/ti/drivers/GPIO",
                args: {
                    /* Sets default but user can reconfigure */
                    $name: "CONFIG_GPIO_" + shortName + "_ENABLE",
                    mode: "Output"
                },
                requiredArgs: {
                    /* Can't be changed by the user */
                    parentInterfaceName: "GPIO",
                    parentSignalName: "lcdEnablePin",
                    parentSignalDisplayName: enableName,
                    $hardware: enableHardware
                }
            },
            {
                name: "lcdPower",
                displayName: powerName,
                moduleName: "/ti/drivers/GPIO",
                args: {
                    /* Sets default but user can reconfigure */
                    $name: "CONFIG_GPIO_" + shortName + "_POWER",
                    mode: "Output"
                },
                requiredArgs: {
                    /* Can't be changed by the user */
                    parentInterfaceName: "GPIO",
                    parentSignalName: "lcdPowerPin",
                    parentSignalDisplayName: powerName,
                    $hardware: powerHardware
                }
            },
            {
                name: "lcdCS",
                legacyNames: ["lcdSS"],
                displayName: selectName,
                moduleName: "/ti/drivers/GPIO",
                args: {
                    /* Sets default but user can reconfigure */
                    $name: "CONFIG_GPIO_" + shortName + "_SELECT",
                    mode: "Output"
                },
                requiredArgs: {
                    /* Can't be changed by the user */
                    parentInterfaceName: "GPIO",
                    parentSignalName: "lcdCSPin",
                    parentSignalDisplayName: selectName,
                    $hardware: selectHardware
                }
            }
        ]);
    }

    return ([]);
}

/*
 * ======== sharedModuleInstances ========
 */
function sharedModuleInstances(inst)
{
    if (inst.displayType == "LCD") {

        let spiName = "LCD SPI";
        let spiHardware = null;

        /* Speculatively get hardware and displayName */
        if (inst.$hardware && inst.$hardware.subComponents) {
            let components = inst.$hardware.subComponents;
            spiHardware = components.SPI;
            if (spiHardware && spiHardware.displayName) {
                spiName = spiHardware.displayName;
            }
        }

        return ([
            {
                name       : "spi",
                displayName: spiName,
                moduleName : "/ti/drivers/SPI",
                hardware   : spiHardware
            }
        ]);
    }

    return ([]);
}

/*
 * ======== validate ========
 */
function validate(inst, validation)
{
    if (inst.mutexTimeoutValue <= 0) {
        logError(validation, inst, 'mutexTimeoutValue',
                 'Must be a positive integer.');
    }

    if (inst.uartBufferSize < 32) {
        logError(validation, inst, 'uartBufferSize',
                 'Must be greater than 32 bytes.');
    }

    if (inst.maxPrintLength <= 0) {
        logError(validation, inst, 'maxPrintLength',
                 'Must be a positive integer.');
    }

    if (inst.lcdSize <= 0) {
        logError(validation, inst, 'lcdSize', 'Must be a positive integer.');
    }

    if (inst.lcdFont == "CMTT")
    {
        if (inst.lcdFontSize < 12 || inst.lcdFontSize > 48 || inst.lcdFontSize % 2 != 0)
        {
            logError(validation, inst, 'lcdFontSize',
                     'Must be an even integer between 12 and 48, both included.');
        }
    }
}

/*
 * ======== onChangeMutexTimeout ========
 *
 */
function onChangeMutexTimeout(inst, ui)
{
    if (inst.mutexTimeout === "Custom") {
        ui.mutexTimeoutValue.hidden = false;
    }
    else {
        ui.mutexTimeoutValue.hidden = true;
        inst.mutexTimeoutValue = 1;
    }
}

/*
 *  ======== onChange ========
 *  Show/hide appropriate config options for each type of display
 */
function onChange(inst, ui)
{
    if (inst.displayType == "LCD") {
        ui.enableANSI.hidden = true;
        ui.maxPrintLength.hidden = true;
        ui.uartBufferSize.hidden = true;
        ui.baudRate.hidden = true;
        ui.lcdSize.hidden = false;
        ui.lcdFont.hidden = false;
        ui.mutexTimeout.hidden = true;
        ui.mutexTimeoutValue.hidden = true;
        inst.displayImplementation = "DisplaySharp";

        if(inst.lcdFont == "Fixed")
        {
            ui.lcdFontSize.hidden = true;
            ui.lcdCustomFontName.hidden = true;
        }else if(inst.lcdFont == "CMTT")
        {
            ui.lcdFontSize.hidden = false;
            ui.lcdCustomFontName.hidden = true;
        }
        else if (inst.lcdFont == "Custom")
        {
            ui.lcdFontSize.hidden = true;
            ui.lcdCustomFontName.hidden = false;
        }
    }
    else if (inst.displayType == "Host") {
        ui.enableANSI.hidden = true;
        ui.maxPrintLength.hidden = false;
        ui.uartBufferSize.hidden = true;
        ui.baudRate.hidden = true;
        ui.lcdSize.hidden = true;
        ui.lcdFont.hidden = true;
        ui.lcdFontSize.hidden = true;
        ui.lcdCustomFontName.hidden = true;
        ui.mutexTimeout.hidden = true;
        ui.mutexTimeoutValue.hidden = true;
        inst.displayImplementation = "DisplayHost";
    }
    else if (inst.displayType == "UART"){
        ui.enableANSI.hidden = false;
        ui.maxPrintLength.hidden = true;
        ui.uartBufferSize.hidden = false;
        ui.baudRate.hidden = false;
        ui.lcdSize.hidden = true;
        ui.lcdFont.hidden = true;
        ui.lcdFontSize.hidden = true;
        ui.lcdCustomFontName.hidden = true;
        ui.mutexTimeout.hidden = false;
        onChangeMutexTimeout(inst, ui);
        inst.displayImplementation = "DisplayUart2";
    }

    if (inst.$hardware) {
        ui.displayType.readOnly = true;
    }
    else {
        ui.displayType.readOnly = false;
    }
}

/*
 *  ========= filterHardware ========
 */
function filterHardware(component)
{
    let ret = false;

    if (component.type) {
        /* Check for known component types */
        if (Common.typeMatches(component.type, ["SHARP_LCD", "SPI_LCD"])) {
            return (true);
        }
    }

    ret = UART.filterHardware(component);

    return (ret);
}

/*
 * ======== onHardwareChanged ========
 */
function onHardwareChanged(inst, ui)
{
    if (inst.$hardware) {

        if (Common.typeMatches(inst.$hardware.type, ["SHARP_LCD", "SPI_LCD"])) {
            inst.displayType = "LCD";
            inst.lcdSize = inst.$hardware.settings.Display.size;
        } else if (Common.typeMatches(inst.$hardware.type, ["UART"])) {
            inst.displayType = "UART";
        }
    }
    else {
        /* Set defaults */
        inst.displayType = "UART";
        inst.mutexTimeout = "Never Timeout";
    }

    onChange(inst, ui);
}

/*
 *  ======== getLibs ========
 *  Argument to the /ti/utils/build/GenLibs.cmd.xdt template
 */
function getLibs(mod)
{
    let libGroup = {
        name: "/ti/display",
        vers: "1.0.0.0",
        deps: [],
        libs: []
    };

    /* Get device information from GenLibs */
    let GenLibs = system.getScript("/ti/utils/build/GenLibs");
    let libPath = GenLibs.libPath;

    /* get library name from DriverLib */
    var DriverLib = system.getScript("/ti/devices/DriverLib");
    let devId = system.deviceData.deviceId;
    let libFamilyName = DriverLib.getAttrs(devId).libName;

    /* add the display library to libGroup's libs */
    libGroup.libs.push(libPath("ti/display", "display_" + libFamilyName + ".a"));

    /* add dependency on /ti/drivers (if needed) */
    let needDrivers = false;
    for (let i = 0; i < mod.$instances.length; i++) {
        let inst =  mod.$instances[i];
        if (inst.displayType != "HOST") {
            needDrivers = true;
            break;
        }
    }
    libGroup.deps = needDrivers ? ["/ti/drivers"] : [];

    return (libGroup);
}

/*
 *  ======== base ========
 *  Define the base properties and methods
 */
let base = {
    displayName           : "Display",
    description           : "Display Driver",
    longDescription: `
The [__Display driver__][1] allows you to manage Display instances via simple
and portable APIs.

* [Usage Synopsis][2]
* [Examples][3]
* [Configuration Options][4]

[1]: /drivers/doxygen/html/_display_8h.html#details "C API reference"
[2]: /drivers/doxygen/html/_display_8h.html#ti_drivers_Display_Synopsis "Basic C usage summary"
[3]: /drivers/doxygen/html/_display_8h.html#ti_drivers_Display_Examples "C usage examples"
[4]: /drivers/syscfg/html/ConfigDoc.html#Display_Configuration_Options "Configuration options reference"
`,
    defaultInstanceName   : "CONFIG_Display_",
    config                : Common.addNameConfig(config, "/ti/display/Display","CONFIG_Display_"),
    validate              : validate,
    maxInstances          : 3,
    filterHardware        : filterHardware,
    onHardwareChanged     : onHardwareChanged,
    pinmuxRequirements    : pinmuxRequirements,
    modules               : Common.autoForceModules(["Board"]),
    moduleInstances       : moduleInstances,
    sharedModuleInstances : sharedModuleInstances,
    templates             : {
        /* contribute to TI-DRIVERS configuration file */
        boardc: "/ti/display/Display.Board.c.xdt",
        boardh: "/ti/display/Display.Board.h.xdt",

        /* contribute libraries to linker command file */
        "/ti/utils/build/GenLibs.cmd.xdt":
            {modName: "/ti/display/Display", getLibs: getLibs}
    }
};

exports = base;
