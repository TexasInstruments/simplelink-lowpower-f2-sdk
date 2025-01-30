/*
 * Copyright (c) 2023-2024, Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== TCAN455X.syscfg.js ========
 */

"use strict";

/* get Common /ti/drivers utility functions */
let Common = system.getScript("/ti/drivers/Common.js");
let logError = Common.logError;

/* The default task priority is the highest task priority common to both
 * FreeRTOS and TIRTOS7 default configurations.
 */
const defaultTaskPriority = 5;

/* The default task stack size is based on worst case usage which is currently
 * dictated by FreeRTOS with GCC compiler. Size must be a word multiple.
 */
const defaultTaskStackSize = 664;

/* get device ID */
const deviceId = system.deviceData.deviceId;

let maxSPIBitRate;
if (deviceId.match(/CC(13|26).[12]/))
{
    maxSPIBitRate = 8000000;
}
else
{
    maxSPIBitRate = 12000000;
}

let config = [
    {
        name: "clkFreqMHz",
        displayName: "Input Clock Frequency (MHz)",
        description: "Specifies TCAN455X input clock frequency in MHz.",
        longDescription: "A minimum value of 20 MHz is needed to support " +
            "CAN FD with a rate of 2 Mbps. The recommended value is 40 MHz " +
            "to meet CAN FD rates up to 5 Mbps.",
        default: 40,
        readOnly: true
    },
    {
        name: "spiBitRate",
        displayName: "SPI Bit Rate",
        description: "Specifies the SPI controller bit rate in bits per second.",
        longDescription: "The TCAN455X SPI peripheral can support bit rates up " +
            "to 18 MHz; however, the max bit rate is limited to by the SPI " +
            "controller. Select the highest supported bit rate for maximum performance.",
        default: maxSPIBitRate
    },
    {
        name: "taskPri",
        displayName: "Task Priority",
        description: "Specifies the priority for the interrupt handler task.",
        longDescription: "Higher numbers denote higher priority. The max value " +
            "depends on the RTOS configuration. It is recommended to set this " +
            "to the highest priority to minimize interrupt processing latency.",
        default: defaultTaskPriority
    },
    {
        name: "taskStackSize",
        displayName: "Task Stack Size",
        description: "Specifies the stack size for the interrupt handler task.",
        default: defaultTaskStackSize
    }
];

/*
 *  ======== _getPinResources ========
 */
function _getPinResources(inst)
{
    let pin;

    let irqPin = inst.canIRQPin.$solution.devicePinName.replace("_", "");
    let csPin = inst.csPin.$solution.devicePinName.replace("_", "");

    pin = "\nTCAN455X INT: " + irqPin + "\nSPI CS: " + csPin;

    let mod = system.getScript("/ti/drivers/SPI.syscfg.js");
    let ss = mod._getPinResources(inst.sharedSpiInstance);
    pin += "\n" + ss;

    return (pin);
}

/*
 *  ======== pinmuxRequirements ========
 */
function pinmuxRequirements(inst)
{
    let requirements = [];

    requirements.push({
        name: "csPin",
        displayName: "SPI CS Pin",
        description: "SPI Chip Select pin",
        interfaceName: "GPIO",
        signalTypes: ["DOUT"]
    });

    requirements.push({
        name: "canIRQPin",
        displayName: "TCAN455X Interrupt Pin",
        description: "TCAN455X interrupt pin",
        interfaceName: "GPIO",
        signalTypes: ["DIN"]
    });

    return requirements;
}

/*
 *  ======== moduleInstances ========
 */
function moduleInstances(inst) {
    let modules = new Array();

    modules.push(
        {
            name: "csPinInstance",
            displayName: "SPI Chip Select GPIO Instance",
            moduleName: "/ti/drivers/GPIO",
            readOnly: true,
            requiredArgs: {
                parentInterfaceName: "GPIO",
                parentSignalName: "csPin",
                parentSignalDisplayName: "SPI CS Pin"
            },
            args: {
                $name: "CONFIG_GPIO_TCAN455X_SPI_CS",
                mode: "Output",
                outputType: "Standard",
                initialOutputState: "High"
            }
        }
    );

    modules.push(
        {
            name: "intPinInstance",
            displayName: "Interrupt GPIO Instance",
            moduleName: "/ti/drivers/GPIO",
            readOnly: true,
            requiredArgs: {
                parentInterfaceName: "GPIO",
                parentSignalName: "canIRQPin",
                parentSignalDisplayName: "TCAN455X Interrupt Pin"
            },
            args: {
                $name: "CONFIG_GPIO_TCAN455X_INT",
                mode: "Input",
                pull: "Pull Up"
            }
        }
    );

    return (modules);
}

/*
 *  ======== sharedModuleInstances ========
 */
function sharedModuleInstances(inst) {
    let sharedModules = new Array();

    /* Add SPI instance */
    sharedModules.push(
        {
            name: "sharedSpiInstance",
            displayName: "Shared SPI Flash Instance",
            moduleName: "/ti/drivers/SPI",
            hardware: inst.$hardware ? inst.$hardware.subComponents.SPI : null
        }
    );

    return (sharedModules);
}

/*
 *  ======== validate ========
 *  Validate this instance's configuration
 *
 *  @param inst       - instance to be validated
 *  @param validation - object to hold detected validation issues
 */
function validate(inst, validation)
{
    let message;
    let maxPriority;
    const taskModule = system.modules["/ti/sysbios/knl/Task"];

    if (taskModule)
    {
        const Task = taskModule.$static;
        maxPriority = Task.numPriorities - 1;

        if (inst.taskPri > maxPriority)
        {
            message = 'Task priority cannot be greater than ' + maxPriority;
            logError(validation, inst, "taskPri", message);
        }
    }

    if ((inst.taskStackSize & 0x3) != 0)
    {
        message = 'Task Stack Size must be a word multiple';
        logError(validation, inst, "taskStackSize", message);
    }

    if (inst.spiBitRate > maxSPIBitRate)
    {
        message = 'SPI Bit Rate cannot be greater than ' + (maxSPIBitRate / 1000000) + ' MHz';
        logError(validation, inst, "spiBitRate", message);
    }

    if (inst.taskPri < 0)
    {
        message = 'Task priority cannot be negative';
        logError(validation, inst, "taskPri", message);
    }
}

/*
 *  ======== exports ========
 */
exports = {
    config: config,
    defaultInstanceName: "CONFIG_CAN_TCAN455X_",
    sharedModuleInstances: sharedModuleInstances,
    moduleInstances: moduleInstances,
    pinmuxRequirements: pinmuxRequirements,
    _getPinResources: _getPinResources,
    validate: validate,
    templates: {
        boardc: "/ti/drivers/can/TCAN455X.Board.c.xdt"
    }
};
