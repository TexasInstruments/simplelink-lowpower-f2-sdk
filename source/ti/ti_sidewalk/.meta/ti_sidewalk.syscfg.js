/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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
 *  ======== ti_sidewalk.syscfg.js ========
 */

"use strict";

const docs = system.getScript("/ti/ti_sidewalk/ti_sidewalk_docs.js");
const rfScript = system.getScript("/ti/ti_sidewalk/rf/ti_sidewalk_rf");
const provisioningScript = system.getScript("/ti/ti_sidewalk/provisioning/ti_sidewalk_provisioning");
const applicationScript = system.getScript("/ti/ti_sidewalk/application/ti_sidewalk_application");

/* Static module definition for ti_sidewalk module */
const moduleStatic = {
    config: [
        applicationScript.config,
        rfScript.config,
        provisioningScript.config
    ],
    moduleInstances: moduleInstances,
    modules: modules,
    validate: validate
};

/*
 * ======== getLibs ========
 * Contribute libraries to linker command file
 *
 * @param inst  - ti_sidewalk module instance
 * @returns     - Object containing the name of component, array of dependent
 *                components, and array of library names
 */
function getLibs(inst)
{
    const libs = [];
    if(inst.$static.enabledPhys.includes("fsk") && inst.$static.enabledPhys.includes("ble"))
    {
        libs.push("ti/ti_sidewalk/library/sid_demo/freertos/gcc/bin/sidewalk_fsk_ble.a");
    }
    else if (inst.$static.enabledPhys.includes("ble"))
    {
        libs.push("ti/ti_sidewalk/library/sid_demo/freertos/gcc/bin/sidewalk_ble.a");
    }
    else if (inst.$static.enabledPhys.includes("fsk"))
    {
        libs.push("ti/ti_sidewalk/library/sid_demo/freertos/gcc/bin/sidewalk_fsk.a");
    }

    // Create a GenLibs input argument
    let results = {
        name: "/ti/ti_sidewalk",
        deps: [],
        libs: libs
    };
    return (results);
}

/* Submodule instance definitions */
function moduleInstances(inst)
{
    const submodules = [];
    return submodules;
}

/* Static submodule instance definitions */
function modules(inst)
{
    const submodules = [];
    return submodules;
}

/*
 * ======== validate ========
 * Validate this inst's configuration
 *
 * @param inst       - EasyLink instance to be validated
 * @param validation - object to hold detected validation issues
 */
function validate(inst, validation)
{
    // Call validation methods of all groups
    rfScript.validate(inst, validation);
    provisioningScript.validate(inst, validation);
}

function getOpts(inst)
{
    /* get device ID to select appropriate defines */
    let deviceId = system.deviceData.deviceId;
    let defines = [];

    if(inst.$static.enabledPhys.includes("ble"))
    {
        defines.push('-DSID_SDK_CONFIG_ENABLE_LINK_TYPE_1=1')
    }
    else
    {
        defines.push('-DSID_SDK_CONFIG_ENABLE_LINK_TYPE_1=0')
    }

    if(inst.$static.enabledPhys.includes("fsk"))
    {
        defines.push('-DSID_SDK_CONFIG_ENABLE_LINK_TYPE_2=1')
    }
    else
    {
        defines.push('-DSID_SDK_CONFIG_ENABLE_LINK_TYPE_2=0')
    }

    if (inst.$static.linkType === "fsk")
    {
        defines.push('-DBUILD_SID_SDK_LINK_TYPE=2')
    }
    else if(inst.$static.linkType === "ble")
    {
        defines.push('-DBUILD_SID_SDK_LINK_TYPE=1')
    }
    else if(inst.$static.linkType === "ble_fsk")
    {
        defines.push('-DBUILD_SID_SDK_LINK_TYPE=4')
    }

    return defines;
}

const sidewalkModule = {
    displayName: "TI-Sidewalk",
    description: docs.module.description,
    longDescription: docs.module.longDescription,
    moduleStatic: moduleStatic,
    templates: {
        "/ti/ti_sidewalk/templates/ti_sidewalk_config.yaml.xdt":
                            "/ti/ti_sidewalk/templates/ti_sidewalk_config.yaml.xdt",
        "/ti/utils/build/GenLibs.cmd.xdt":
        {
            modName: "/ti/ti_sidewalk/ti_sidewalk",
            getLibs: getLibs,
        },
        "/ti/utils/build/GenOpts.opt.xdt":
        {
            modName: "/ti/ti_sidewalk/ti_sidewalk",
            getOpts: getOpts
        }
    },
    getLibs: getLibs,
    getOpts: getOpts
};

exports = sidewalkModule;

