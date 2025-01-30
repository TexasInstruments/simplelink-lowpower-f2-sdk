/*
 * Copyright (c) 2019-2024, Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== UART2.syscfg.js ========
 */

"use strict";

let Common   = system.getScript("/ti/drivers/Common.js");
let family   = Common.device2Family(system.deviceData, "UART2");

/* Array of UART2 configurables that are common across device families */
let config = [

    /* baudRates is only needed by MSP432P but exists for all devs to enable
     * more portable syscfg scripts.
     */
    {
        name        : "baudRates",
        displayName : "Baud Rates",
        description : 'The set of baud rates that are used by this instance. '
                      + 'On some devices, these rates are used to pre-compute '
                      + 'a table of clock dividers necessary for each rate.',
        hidden      : true,
        default     : [115200],
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
    },
    {
        name        : "enableNonblocking",
        displayName : "Enable Nonblocking Mode",
        description : 'Enable nonblocking read/write-mode',
        longDescription: "If your application does not use UART2_Mode_NONBLOCKING you can disable this mode." +
        "This will reduce both flash-size and RAM-usage for your application.",
        default     : true,
        onChange: function (inst, ui) {
            /* There will be no TX ring buffer if nonblocking mode is disabled */
            ui["txRingBufferSize"].hidden = !(inst.enableNonblocking);
        }
    },
    {
        name        : "dataDirection",
        displayName : "Data Direction",
        default     : 'Send and Receive',
        options     : [
            { name : 'Send and Receive' },
            { name : 'Send Only' },
            { name : 'Receive Only' }
        ]
    },
    {
        name        : "flowControl",
        displayName : "Flow Control",
        default     : false,
        description : "Enable hardware flow control",
        longDescription : "Hardware flow control between two devices "
            + "is accomplished by connecting the UART Request-To-Send "
            + "(RTS) pin to the Clear-To-Send (CTS) input on the "
            + "receiving device, and connecting the RTS output of the "
            + "receiving device to the UART CTS pin"
    }
];

/* Array of static UART2 configurables that are common across device families */
let configStatic = [

    {
        name: "loggingEnabled",
        displayName: "Enable Logging",
        hidden : false,
        description: `This setting will enable logging for the UART2 module.`,
        longDescription: "With logging enabled a special version of the driver will be used." +
        "Log-statments are embedded in this driver version and it should only be used for debugging." +
        "This implies an overhead in both code size and runtime",
        default: false
    }
];

/*
 *  ======== _getPinResources ========
 */
/* istanbul ignore next */
function _getPinResources(inst)
{
    return;
}

/*
 *  ======== validateStatic ========
 *  Validate this module's static configuration
 *
 *  param inst       - The UART module's static instance to be validated
 *  param validation - object to hold detected validation issues
 *
 */
function validateStatic(inst, validation)
{
    /*
     * If logging is enabled in the UART2 driver, an error is thrown if LogSinkUART has
     * been selected as log sink by any driver.
     */
    if (inst.loggingEnabled) {
        for (let i = 0; i < system.modules["/ti/log/LogModule"].$instances.length; i++) {
            let tinst = system.modules["/ti/log/LogModule"].$instances[i];
            if (tinst.loggerSink == "/ti/log/LogSinkUART") {
                let message = 'LogSinkUART can not be selected when logging is enabled in UART2';
                validation.logError(message, tinst);
                break;
            }
        }
    }
}

/*
 *  ======== moduleInstances ========
 *  returns PIN instances
 */
function moduleInstances(inst) {
    let pinInstances = new Array();
    let shortName = inst.$name.replace("CONFIG_", "");

    if (inst.dataDirection != "Receive Only") {
        pinInstances.push(
            {
                name: "txPinInstance",
                displayName: "TX configuration when not in use",
                moduleName: "/ti/drivers/GPIO",
                collapsed: true,
                requiredArgs: {
                    parentInterfaceName: "uart",
                    parentSignalName: "txPin",
                    parentSignalDisplayName: "TX"
                },
                args: {
                    $name: "CONFIG_GPIO_" + shortName + "_TX",
                    initialOutputState: "High",
                    mode: "Output",
                    pull: "None"
                }
            }
        );
        if (inst.flowControl) {
            pinInstances.push({
                name: "ctsPinInstance",
                displayName: "CTS configuration when not in use",
                moduleName: "/ti/drivers/GPIO",
                collapsed: true,
                requiredArgs: {
                    parentInterfaceName: "uart",
                    parentSignalName: "ctsPin",
                    parentSignalDisplayName: "CTS"
                },
                args: {
                    $name: "CONFIG_GPIO_" + shortName + "_CTS",
                    mode: "Input",
                    pull: "Pull Down"
                }
            });
        }
    }

    if (inst.dataDirection != "Send Only") {
        pinInstances.push({
            name: "rxPinInstance",
            displayName: "RX configuration when not in use",
            moduleName: "/ti/drivers/GPIO",
            collapsed: true,
            requiredArgs: {
                parentInterfaceName: "uart",
                parentSignalName: "rxPin",
                parentSignalDisplayName: "RX"
            },
            args: {
                $name: "CONFIG_GPIO_" + shortName + "_RX",
                mode: "Input",
                pull: "Pull Down"
            }
        }
        );
        if (inst.flowControl) {
            pinInstances.push({
                name: "rtsPinInstance",
                displayName: "RTS configuration when not in use",
                moduleName: "/ti/drivers/GPIO",
                collapsed: true,
                requiredArgs: {
                    parentInterfaceName: "uart",
                    parentSignalName: "rtsPin",
                    parentSignalDisplayName: "RTS"
                },
                args: {
                    $name: "CONFIG_GPIO_" + shortName + "_RTS",
                    initialOutputState: "Low",
                    mode: "Output",
                    pull: "None"
                }
            });
        }
    }
    return (pinInstances);
}

/*
 *  ======== moduleInstancesStatic ========
 *  returns static instances
 */
function moduleInstancesStatic(inst) {
    let staticInstances = new Array();

    /* If logging is enabled, push a dependency on a log module */
    if (inst.loggingEnabled) {
        staticInstances.push(
            {
                name: "LogModule",
                displayName: "UART2 Log Configuration",
                moduleName: "/ti/log/LogModule",
                collapsed: true,
                requiredArgs: {
                    $name: "LogModule_UART2"
                },
                args: {
                    enable_DEBUG: false,
                    enable_INFO: false,
                    enable_VERBOSE: false,
                    // Only enable WARNING and ERROR enabled by default
                    enable_WARNING: true,
                    enable_ERROR: true
                }
            }
        );
    }
    return (staticInstances);
}

/*
 *  ======== base ========
 *  Define the base UART2 properties and methods
 */
let base = {
    displayName   : "UART2",
    description   : "Universal Asynchronous Receiver Transmitter (UART) Driver",

    longDescription: `
The [__UART2 driver__][1] provides device independent APIs for reading
and writing to the UART peripherals. The UART2 driver is an improved version
of the [__UART driver__][5].

* [Usage Synopsis][2]
* [Examples][3]
* [Configuration Options][4]

[1]: /drivers/doxygen/html/_u_a_r_t2_8h.html#details "C API reference"
[2]: /drivers/doxygen/html/_u_a_r_t2_8h.html#ti_drivers_UART_Synopsis "Basic C usage summary"
[3]: /drivers/doxygen/html/_u_a_r_t2_8h.html#ti_drivers_UART_Examples "C usage examples"
[4]: /drivers/syscfg/html/ConfigDoc.html#UART2_Configuration_Options "Configuration options reference"
[5]: /drivers/doxygen/html/_u_a_r_t_8h.html#details "C API reference"
`,
    /* instance properties and methods */
    defaultInstanceName: "CONFIG_UART2_",
    config        : Common.addNameConfig(config, "/ti/drivers/UART2", "CONFIG_UART2_"),
    modules       : Common.autoForceModules(["Board", "Power", "DMA"]),
    moduleInstances: moduleInstances,
    _getPinResources: _getPinResources,
    moduleStatic: {
        config: configStatic,
        moduleInstances: moduleInstancesStatic,
        validate: validateStatic
    }
};

/* get family-specific UART2 module */
let devUART2 = system.getScript("/ti/drivers/uart2/UART2" + family);

exports     = devUART2.extend(base);
