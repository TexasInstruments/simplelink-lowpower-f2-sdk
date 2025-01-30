/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
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
 * ======== Settings.syscfg.js ========
 * This module's main purpose is to provide a list of libraries needed to use
 * the nortos DPL.
 */

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

    let tfmEnabled = family.match(/(cc.*4)/) && system.modules["/ti/utils/TrustZone"];
    if(tfmEnabled){
        libs.push("lib/" + getToolchainDir() + "/" + getDeviceIsa() + "/nortos_" + family + "_ns.a");
    }
    else{
        libs.push("lib/" + getToolchainDir() + "/" + getDeviceIsa() + "/nortos_" + family + ".a");
    }

    /* Create a GenLibs input argument */
    var linkOpts = {
        name: "/nortos/dpl",
        vers: "1.0.0.0",
        deps: ["/ti/drivers"],
        libs: libs
    };

    return linkOpts;
}

exports = {
    staticOnly: true,
    displayName: "Settings",
    getLibs: getLibs,
    templates: {
        /* Contribute NoRTOS libraries to linker command file */
        "/ti/utils/build/GenLibs.cmd.xdt": { modName: "/nortos/dpl/Settings", getLibs: getLibs }
    }
};
