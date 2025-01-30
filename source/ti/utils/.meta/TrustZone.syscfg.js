/*
 * Copyright (c) 2022-2024 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== TrustZone.syscfg.js ========
 *  TrustZone configuration support
 */

 "use strict";

 /* get ti/utils common utility functions */
let Common = system.getScript("/ti/utils/Common.js");

/*
 *  ======== getLibs ========
 *  GenLibs support function
 *
 *  Provides this module's library as input to GenLibs.
 */
function getLibs(mod)
{
    let GenLibs = system.getScript("/ti/utils/build/GenLibs.syscfg.js");
    let libs = [];
    let deviceId = system.deviceData.deviceId;

    /* Select which secure veneer object to use */
    if (mod.$static.secureImage == "1") {
        if (deviceId.match(/CC27/)) {
            libs.push("build/cc27xx/production_full/Release/export/tfm/veneers/s_veneers.o");
        } else { /* This is sufficient as other than CC27XX, only Thor supports TrustZone */
            libs.push("build/cc26x4/production_full/Release/export/tfm/veneers/s_veneers.o");
        }
    }

    /* Create a GenLibs input argument */
    var linkOpts = {
        name: "/ti/utils/TrustZone",
        deps: [],
        libs: libs
    };

    return (linkOpts);
}

/*
 *  ======== modules ========
 *  Express dependencies for other modules
 *
 *  Invoked on any configuration change to the given instance.
 */
function modules(inst)
{
    let modules = new Array();

    return (modules);
}

/*
 *  ======== longDescription ========
 *  Intro splash on GUI
 */
let longDescription = `
The TrustZone module links the SPE-enabled drivers library and the secure veneers
required for Secure/Non-Secure features.
`;

/*
 *  ======== base ========
 *  Module definition object
 */
let base = {
    displayName: "TrustZone",
    description: "TrustZone configuration",
    longDescription: longDescription,
    moduleStatic: {
        modules: modules,
        config: [
            {
                name: "secureImage",
                displayName: "Secure Image",
                description: "Select a secure image",
                longDescription: `Each secure image has a corresponding secure
                veneers object containing its secure symbols. Selecting the
                secure image links the symbols required for that image.
                `,
                default: "1",
                options: [
                    { name: "1", displayName: "Production Full" }
                ]
            }
        ],
    },
    templates: {
        /* contribute secure veneers object to linker command file */
        "/ti/utils/build/GenLibs.cmd.xdt"   :
            {modName: "/ti/utils/TrustZone", getLibs: getLibs}
    }
};

/* export the module */
exports = base;
