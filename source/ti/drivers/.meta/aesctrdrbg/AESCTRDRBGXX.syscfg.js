/*
 * Copyright (c) 2019-2024, Texas Instruments Incorporated
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
 *  ======== AESCTRDRBGXX.js ========
 */

"use strict";

/* get Common /ti/drivers utility functions */
let Common = system.getScript("/ti/drivers/Common.js");

/* get device ID */
let deviceId = system.deviceData.deviceId;

/* Interrupt Priority for internal AESCTR instance */
let intPriority = Common.newIntPri()[0];
intPriority.name = "interruptPriority";
intPriority.displayName = "Interrupt Priority";
intPriority.description = "Crypto peripheral interrupt priority";

/*
 *  ======== getLibs ========
 *  Argument to the /ti/utils/build/GenLibs.cmd.xdt template
 */
function getLibs(mod)
{
    /* Get device information from GenLibs */
    let GenLibs = system.getScript("/ti/utils/build/GenLibs");

    let libGroup = {
        name: "/third_party/hsmddk",
        deps: [],
        libs: [],
        allowDuplicates: true
    };

    if (!system.modules["/ti/utils/TrustZone"]) {
        libGroup.libs.push(GenLibs.libPath("third_party/hsmddk", "hsmddk_cc27xx_its.a"));
    }

    return (libGroup);
}

/*
 *  ======== devSpecific ========
 *  Device-specific extensions to be added to base AESCTRDRBG configuration
 */
let devSpecific = {
    config: [
        intPriority
    ],

    /* override device-specific templates */
    templates: {
        boardh: "/ti/drivers/aesctrdrbg/AESCTRDRBG.Board.h.xdt",
        boardc: "/ti/drivers/aesctrdrbg/AESCTRDRBGXX.Board.c.xdt"
    },

    modules: (inst) => {
        if (deviceId.match(/CC23|CC27/))
        {
            /* LAES driver requires DMA module */
            return Common.autoForceModules(["Board", "Power", "DMA"])();
        }
        else
        {
            return Common.autoForceModules(["Board", "Power"])();
        }
    }
};

/*
 *  ======== extend ========
 *  Extends a base exports object to include any device specifics
 *
 *  This function is invoked by the generic AESCTRDRBG module to
 *  allow us to augment/override as needed for the generic implementation.
 */
function extend(base)
{
    /* display which driver implementation can be used */
    base = Common.addImplementationConfig(base, "AESCTRDRBG", null,
        [{name: "AESCTRDRBGXX"}], null);

    /* merge and overwrite base module attributes */
    let result = Object.assign({}, base, devSpecific);

    if (deviceId.match(/CC27../)) {
        devSpecific["templates"]["/ti/utils/build/GenLibs.cmd.xdt"] = {modName: "/ti/drivers/AESCTRDRBG", getLibs: getLibs};
    }

    /* concatenate device-specific configs */
    result.config = base.config.concat(devSpecific.config);

    return (result);
}

/*
 *  ======== exports ========
 *  Export device-specific extensions to base exports
 */
exports = {
    /* required function, called by base AESCTRDRBG module */
    extend: extend
};
