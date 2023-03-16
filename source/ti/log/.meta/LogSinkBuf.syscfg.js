/*
 * Copyright (c) 2022-2023 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== LogSinkBuf.syscfg.js ========
 */

"use strict";


/*
 *  ======== config_instance ========
 *  Define the config params of the module instance
 */
let config_instance = [
    {
        name: "bufType",
        displayName: "Buffer Type",
        default: "LogSinkBuf_Type_CIRCULAR",
        options: [
            {
                name: "LogSinkBuf_Type_CIRCULAR",
                displayName: "Circular Buffer"
            },
            {
                name: "LogSinkBuf_Type_LINEAR",
                displayName: "Linear Buffer"
            }
        ]
    },
    {
        name: "numEntries",
        displayName: "Number of Entries",
        default: 100
    }
];

/*
 *  ======== modules ========
 *  Express dependencies for other modules
 */
function modules(inst)
{
    let modules = new Array();

    return (modules);
}

/*
 *  ======== base ========
 *  Module definition object
 */
let base = {
    displayName: "LogSinkBuf",
    description: "LogSinkBuf module",
    longDescription: `
The [__LogSinkBuf__][1] provides a log sink that stores log output in RAM on target,
which can be read by ROV or other host-side tools capable of reading target-memory.

* [Log API][2]
* [Log Tools][3]

[1]: /drivers/doxygen/html/index.html#log "Log framework"
[2]: /tiutils/html/group__ti__log__LOG.html "Log API"
[3]: /../tools/log/tiutils/Readme.html "Log Tools"
`,
    config: config_instance,
    defaultInstanceName: "CONFIG_ti_log_LogSinkBuf_",
    modules: modules,

    templates: {
        "/ti/log/templates/ti_log_config.c.xdt":
            "/ti/log/templates/LogSinkBuf.Config.c.xdt",
        "/ti/log/templates/ti_log_config.h.xdt":
            "/ti/log/templates/LogSinkBuf.Config.h.xdt",
        "/ti/utils/rov/syscfg_c.rov.xs.xdt":
            "/ti/log/LogSinkBuf.rov.js",
        "/ti/log/templates/rov.js.xdt":
            "/ti/log/LogSinkBuf.rov.js.xdt"
    }
};

/* export the module */
exports = base;
