/*
 * Copyright (c) 2022-2024, Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== LogSinkITM.syscfg.js ========
 */

"use strict";

/*
 *  ======== config_instance ========
 *  Define the config params of the module instance
 */
let config_instance = [
    {
        name: "printfDelegate",
        displayName: "Printf Delegate Function",
        default: "LogSinkITM_printfSingleton",
        readOnly: true
    },
    {
        name: "bufDelegate",
        displayName: "Buf Delegate Function",
        default: "LogSinkITM_bufSingleton",
        readOnly: true
    }
];

/*
 *  ======== modules ========
 *  Express dependencies for other modules
 */
function modules(inst)
{
    let modules = new Array();

    modules.push({
        name: "itm",
        displayName: "ITM",
        moduleName: "/ti/drivers/ITM"
    });

    return (modules);
}

/*
 *  ======== base ========
 *  Module definition object
 */
let base = {
    displayName: "LogSinkITM",
    description: "LogSinkITM module",
    longDescription: `
The [__LogSinkITM__][1] provides a log sink that transports log out through ITM,
which can be received by the tilogger host-side tool.

* [Log API][2]
* [Log Tools][3]

[1]: /drivers/doxygen/html/index.html#log "Log framework"
[2]: /tiutils/html/group__ti__log__LOG.html "Log API"
[3]: /../tools/log/tiutils/Readme.html "Log Tools"
`,
    defaultInstanceName: "CONFIG_ti_log_LogSinkITM_",
    config: config_instance,
    modules: modules,
    maxInstances        : 1,
    /* Board_init() priority to guarantee that the LogSink is initialized after GPIO */
    initPriority        : 5,

    templates: {
        "/ti/log/templates/ti_log_config.c.xdt":
            "/ti/log/templates/LogSinkITM.Config.c.xdt",
        "/ti/log/templates/ti_log_config.h.xdt":
            "/ti/log/templates/LogSinkITM.Config.h.xdt",
        boardc : "/ti/log/templates/LogSinkITM.Board.c.xdt",
        board_initc: "/ti/log/templates/LogSinkITM.Board_init.c.xdt"
    }
};

/* export the module */
exports = base;
