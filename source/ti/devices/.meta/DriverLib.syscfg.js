/*
 * Copyright (c) 2019-2023, Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== DriverLib.syscfg.js ========
 *  Support use of DriverLib within SysConfig meta-code
 */

"use strict";

/*
 *  ======== config ========
 *  User Config options
 */
let config = [
    {
        name: "path", /* override computed path to driverlib archive */
        displayName: "Library Path",
        description: "Relative path to the driverlib library",
        default: "",
        hidden: true
    }
];

/*
 *  ======== getAttrs ========
 *  Map a SysConfig deviceId to a set of "device family" attrs
 *
 *  @param deviceId  - a pinmux deviceId (system.deviceData)
 *
 *  @returns object - a set of device attributes of the form
 *      {
 *          deviceDir:    "", // name of the directory in /ti/devices/
 *                            // containing driverlib
 *          deviceDefine: ""  // DeviceFamily #define
 *      }
 */
function getAttrs(deviceId, part)
{
    var result = {};

    if (deviceId.match(/CC13.1/)) {
        result.deviceDir  = "cc13x1_cc26x1";
        result.deviceDefine = "DeviceFamily_CC13X1";
        result.libName = "cc13x1";
    }
    else if (deviceId.match(/CC26.1/)) {
        result.deviceDir = "cc13x1_cc26x1";
        result.deviceDefine = "DeviceFamily_CC26X1";
        result.libName = "cc26x1";
    }
    else if (deviceId.match(/CC13.2.7/)) {
        result.deviceDir  = "cc13x2x7_cc26x2x7";
        result.deviceDefine = "DeviceFamily_CC13X2X7";
        result.libName = "cc13x2x7";
    }
    else if (deviceId.match(/CC26.2.7/)) {
        result.deviceDir = "cc13x2x7_cc26x2x7";
        result.deviceDefine = "DeviceFamily_CC26X2X7";
        result.libName = "cc26x2x7";
    }
    else if (deviceId.match(/CC13.2/)) {
        result.deviceDir  = "cc13x2_cc26x2";
        result.deviceDefine = "DeviceFamily_CC13X2";
        result.libName = "cc13x2";
    }
    else if (deviceId.match(/CC26.2/)) {
        result.deviceDir = "cc13x2_cc26x2";
        result.deviceDefine = "DeviceFamily_CC26X2";
        result.libName = "cc26x2";
    }
    else if (deviceId.match(/CC26.4/)) {
        result.deviceDir = "cc13x4_cc26x4";
        result.deviceDefine = "DeviceFamily_CC26X4";
        result.libName = "cc26x4";
    }
    else if (deviceId.match(/CC2653/)) {
        /* The CC2653 is very similar to CC26X4 from a software point of view,
         * so we use the same deviceDir and libName.
         */
        result.deviceDir = "cc13x4_cc26x4";
        result.deviceDefine = "DeviceFamily_CC26X3";
        result.libName = "cc26x4";
    }
    else if (deviceId.match(/CC13.4/)) {
        result.deviceDir = "cc13x4_cc26x4";
        result.deviceDefine = "DeviceFamily_CC13X4";
        result.libName = "cc13x4";
    }
    else if (deviceId.match(/CC23.0R2/)) {
        result.deviceDir = "cc23x0r2";
        result.deviceDefine = "DeviceFamily_CC23X0R2";
        result.libName = "cc23x0r2";
    }
    else if (deviceId.match(/CC23.0R5/)) {
        result.deviceDir = "cc23x0r5";
        result.deviceDefine = "DeviceFamily_CC23X0R5";
        result.libName = "cc23x0r5";
    }
    else if (deviceId.match(/CC27/)) {
        result.deviceDir = "cc27xx";
        result.deviceDefine = "DeviceFamily_CC27XX";
        result.libName = "cc27xx";
    }
    else if (deviceId.match(/CC32/)) {
        result.deviceDir = "cc32xx";
        result.deviceDefine = "DeviceFamily_CC3220";
        result.libName = "cc32xx";
    }
    else if (deviceId.match(/CC35/)) {
        result.deviceDir = "cc35xx";
        result.deviceDefine = "DeviceFamily_CC35XX";
        result.libName = "cc35xx";
    }
    else {
        result.deviceDir = "";
        result.deviceDefine = "";
        result.libName = "";
    }

    return (result);
}

function getLibs()
{
    /* Get toolchain specific information from GenLibs */
    let GenLibs = system.getScript("/ti/utils/build/GenLibs");
    let getToolchainDir = GenLibs.getToolchainDir;

    /* get device ID to select appropriate libs */
    let deviceId = system.deviceData.deviceId;

    let libs = [];

    if (deviceId.match(/CC13|CC26/)) {
        libs.push(
            "ti/devices/" + getAttrs(deviceId).deviceDir + "/driverlib/bin/" + getToolchainDir() + "/driverlib.lib"
        );
    } else if (deviceId.match(/CC32/)) {
        libs.push(
            "ti/devices/cc32xx/driverlib/" + getToolchainDir() + "/Release/driverlib.a"
        );
    } else {
        libs.push(
            "ti/devices/" + getAttrs(deviceId).deviceDir + "/driverlib/lib/" + getToolchainDir() + "/driverlib.a"
        );
    }

    /* create a GenLibs input argument */
    var linkOpts = {
        name: "/ti/devices",
        vers: "1.0.0.0",
        deps: [],
        libs: libs
    };

    return linkOpts;
}

function getOpts()
{
    /* get device ID to select appropriate defines */
    let deviceId = system.deviceData.deviceId;

    return ["-D" + getAttrs(deviceId).deviceDefine];
}

/*
 *  ======== exports ========
 */
exports = {
    staticOnly   : true,
    displayName  : "DriverLib",

    /* enable end-user to override computed library path */
    moduleStatic : {
        config: config
    },

    templates: {
        /* contribute TI-DRIVERS libraries to linker command file */
        "/ti/utils/build/GenLibs.cmd.xdt": { modName: "/ti/devices/DriverLib", getLibs: getLibs },
        "/ti/utils/build/GenOpts.opt.xdt": { modName: "/ti/devices/DriverLib", getOpts: getOpts }
    },

    /* DriverLib-specific exports */
    getAttrs: getAttrs,
    getOpts: getOpts,
    getLibs: getLibs
};
