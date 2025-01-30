/*
 * Copyright (c) 2023-2024 Texas Instruments Incorporated - http://www.ti.com
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
 */

/*
 *  ======== LogSinkUART.syscfg.js ========
 */

"use strict";

let Common   = system.getScript("/ti/drivers/Common.js");
let logError = Common.logError;
let deviceId = system.deviceData.deviceId;

/*
 *  ======== config_instance ========
 *  Define the config params of the module instance
 */
let config_instance = [
    {
        name        : "baudRate",
        displayName : "Baud Rate",
        default     : 3000000
    },
    {
        name        : "parity",
        displayName : "Parity",
        default     : "UART2_Parity_NONE",
        options: [
            {
                name: "UART2_Parity_NONE",
                displayName: "No parity"
            },
            {
                name: "UART2_Parity_EVEN",
                displayName: "Parity bit is even"
            },
            {
                name: "UART2_Parity_ODD",
                displayName: "Parity bit is odd"
            },
            {
                name: "UART2_Parity_ZERO",
                displayName: "Parity bit is always zero"
            },
            {
                name: "UART2_Parity_ONE",
                displayName: "Parity bit is always one"
            }
        ]
    },
    {
        name        : "BufferSize",
        displayName : "Buffer Size",
        description : "Intermediate ring buffer size in bytes",
        longDescription: `
The ring buffer is filled by log statements. It is automatically flushed when
the device is idle, or it can be manually flushed by calling LogSinkUART_flush().
The size of the ring buffer affects how many log statements can be stored
at once before flushing it.

If a log statement would overflow the ring buffer, an overflow message
will be stored instead, indicating that at least that message would have
overfowed the ring buffer. When the ring buffer is full no more log statements or
overflow messages will be recorded.

When flushing the ring buffer not all data is flushed at once. The UART TX Ring
Buffer size determines the maximum amount of data that can be flushed at each
tick of the idle loop / LogSinkUART_flush() call.
`,
        default     : 1024
    },
    {
        name: "printfDelegate",
        displayName: "Printf Delegate Function",
        default: "LogSinkUART_printfDepInjection",
        readOnly: true,
        /* getValue is evaluated every time printfDelgate is read in either a configurable or a template. */
        getValue: () => {
            /* Only return the singleton function if we have one instance */
            if (system.modules["/ti/log/LogSinkUART"].$instances.length == 1) {
                return "LogSinkUART_printfSingleton";
            }
            /* Return the regular dependency injection implementation by default. */
            return "LogSinkUART_printfDepInjection";
        }
    },
    {
        name: "bufDelegate",
        displayName: "Buf Delegate Function",
        default: "LogSinkUART_bufDepInjection",
        readOnly: true
    }
];

/*
 * ======== moduleInstances ========
 */
function moduleInstances(inst)
{
    let sharedInstance = new Array();

    /* Check device ID for concatenateFIFOs option*/
    if (deviceId.match(/CC23|CC27/))
    {
        sharedInstance.push(
            {
                name       : "uart",
                displayName: "UART",
                moduleName : "/ti/drivers/UART2",
                collapsed: true,
                requiredArgs: {
                    dataDirection: 'Send Only',
                    enableNonblocking: true,
                    concatenateFIFOs: true
                }
            }
        );
    }
    else
    {
        sharedInstance.push(
            {
                name       : "uart",
                displayName: "UART",
                moduleName : "/ti/drivers/UART2",
                collapsed: true,
                requiredArgs: {
                    dataDirection: 'Send Only',
                    enableNonblocking: true
                }
            }
        );
    }

    return (sharedInstance);
}

/*
 *  ======== sharedModuleInstances ========
 */
function sharedModuleInstances(inst) {
    let sharedInstance = new Array();

    if (system.getRTOS() == "freertos")
    {
        sharedInstance.push(
            {
                name       : "idleHooks",
                displayName: "IDLE HOOK",
                moduleName : "/freertos/idleHooks",
                collapsed: true,
                requiredArgs: {
                    $name: "LogSinkUART_flush",
                    idleHookFunctionName: "LogSinkUART_flush"
                }
            }
        );
    }
    else if (system.getRTOS() == "tirtos7")
    {
        sharedInstance.push(
            {
                name       : "idleHooks",
                displayName: "IDLE HOOK",
                moduleName : "/ti/sysbios/knl/Idle",
                collapsed: true,
                requiredArgs: {
                    $name: "LogSinkUART_flush",
                    idleFxn: "LogSinkUART_flush"
                }
            }
        );
    }

    return (sharedInstance);
}

/*
 * ======== validate ========
 */
function validate(inst, validation)
{
    if (inst.baudRate > 3e6 || inst.baudRate <= 0) {
        logError(validation, inst, "baudRate", "Baud rate must be greater than"
        + " 0 and less than " + 3e6);
    }

    if (inst.BufferSize < 8) {
        logError(validation, inst, 'BufferSize',
                 'Must be greater than 8 bytes.');
    }
}

/*
 *  ======== base ========
 *  Module definition object
 */
let base = {
    displayName: "LogSinkUART",
    description: "LogSinkUART module",
    longDescription: `
The [__LogSinkUART__][1] provides a log sink that transports log out through UART2,
which can be received by the tilogger host-side tool.

* [Log API][2]
* [Log Tools][3]

[1]: /drivers/doxygen/html/index.html#log "Log framework"
[2]: /tiutils/html/group__ti__log__LOG.html "Log API"
[3]: /../tools/log/tiutils/Readme.html "Log Tools"
`,
    config              : config_instance,
    defaultInstanceName : "CONFIG_ti_log_LogSinkUART_",
    moduleInstances     : moduleInstances,
    sharedModuleInstances: sharedModuleInstances,
    validate: validate,
    /* Board_init() priority to guarantee that the LogSink is initialized after GPIO */
    initPriority        : 5,
    templates: {
        "/ti/log/templates/ti_log_config.c.xdt":
            "/ti/log/templates/LogSinkUART.Config.c.xdt",
        "/ti/log/templates/ti_log_config.h.xdt":
            "/ti/log/templates/LogSinkUART.Config.h.xdt",
        boardc : "/ti/log/templates/LogSinkUART.Board.c.xdt",
        board_initc: "/ti/log/templates/LogSinkUART.Board_init.c.xdt"
    }

};

/* export the module */
exports = base;
