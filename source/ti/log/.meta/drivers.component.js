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
 *
 */

/*
 *  ======== drivers.component.js ========
 */

"use strict";

let LogModule = system.getScript("/ti/log/LogModule.syscfg.js");
let deviceId = system.deviceData.deviceId;

let topModules;
let templates;

/* Include LogSinkBuf for all devices*/
let logSinks = ["/ti/log/LogSinkBuf"];

/* Include LogSinks conditionally on the board */
if(!(deviceId.match(/CC23.0/) || deviceId.match(/CC35.0/))) {

    logSinks.push("/ti/log/LogSinkITM");
}
/* Include LogSinks conditionally on the board */
if (deviceId.match(/CC23.0/) || deviceId.match(/CC27/))
{
    logSinks.push("/ti/log/LogSinkTraceLPF3");
}

/* Include LogSinkUART for all devices*/
logSinks.push("/ti/log/LogSinkUART");

topModules = [
    LogModule.sinksToTopModule(logSinks)
];

templates = [
    {
        name: "/ti/log/templates/rov.js.xdt",
        outputPath: "ti_utils_runtime_rov.js.xs"
    }
];

exports = {
    displayName: LogModule.topModuleDisplayName,
    topModules: topModules,
    templates: templates
};
