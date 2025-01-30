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
    },
    {
        name: "printfDelegate",
        displayName: "Printf Delegate Function",
        default: "LogSinkBuf_printfDepInjection",
        readOnly: true,
        /* getValue is evaluated every time printfDelgate is read in either a configurable or a template. */
        getValue: () => {
            /* Only return the singleton function if we have one instance named CONFIG_ti_log_LogSinkBuf_0 */
            if (system.modules["/ti/log/LogSinkBuf"].$instances.length == 1) {
                if (system.modules["/ti/log/LogSinkBuf"].$instances[0].$name === "CONFIG_ti_log_LogSinkBuf_0") {
                    return "LogSinkBuf_printfSingleton";
                }
            }
            /* Return the regular dependency injection implementation by default. */
            return "LogSinkBuf_printfDepInjection";
        }
    },
    {
        name: "bufDelegate",
        displayName: "Buf Delegate Function",
        default: "LogSinkBuf_bufDepInjection",
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

    return (modules);
}

function getLibs()
{
    /* Get device ID to select appropriate libs */
    let devId = system.deviceData.deviceId;

    /* Get device information from DriverLib */
    var DriverLib = system.getScript("/ti/devices/DriverLib");
    let family = DriverLib.getAttrs(devId).libName;

    /* Get toolchain specific information from GenLibs */
    let GenLibs = system.getScript("/ti/utils/build/GenLibs");
    let getToolchainDir = GenLibs.getToolchainDir;
    let getDeviceIsa = GenLibs.getDeviceIsa;

    let libs = [];
    libs.push("ti/log/lib/" + getToolchainDir() + "/" + getDeviceIsa() + "/log_" + family + ".a");

    /* Create a default GenLibs input argument */
    var linkOpts = {
        name: "/ti/log/LogSinkBuf",
        vers: "1.0.0.0",
        deps: [],
        libs: libs
    };

    if (system.getRTOS() == "nortos" && system.compiler == "gcc") {
        /* Workaround to handle circular dependencies between the NoRTOS DPL lib
         * and the log lib. This is only needed for GCC.
         */
        linkOpts = {
            name: "/ti/log/LogSinkBuf",
            vers: "1.0.0.0",
            deps: ["/nortos/dpl"],
            libs: libs
        };
    }

    return linkOpts;
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

When only using a single instance, an LTO-optimised implementation is used as
long as the instance is named CONFIG_ti_log_LogSinkBuf_0. This implementation
uses a singleton design pattern to allow the compiler to forgo loading one of
the function arguments to the delegate function when LTO is enabled but requires
linking against a known LogSinkBuf_Instance symbol name.

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
            "/ti/log/LogSinkBuf.rov.js.xdt",
        "/ti/utils/build/GenLibs.cmd.xdt":
            { modName: "/ti/log/LogSinkBuf", getLibs: getLibs }
    }
};

/* export the module */
exports = base;
