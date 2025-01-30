/*
 * Copyright (c) 2017-2018 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== wsms.syscfg.js ========
 */

// Get the wisun script 
const wisunScript = system.getScript("/ti/ti_wisunfan/"
    + "ti_wisunfan");

// Get radio settings script
const radioScript = system.getScript("/ti/ti_wisunfan/rf_config/"
    + "ti_wisunfan_rf_config");

// Get network settings script
const networkScript = system.getScript("/ti/ti_wisunfan/network_config/"
    + "ti_wisunfan_network_config");

// Get transmit power settings script
const powerScript = system.getScript("/ti/ti_wisunfan/power_config/"
    + "ti_wisunfan_power_config");

// Get test mode settings script
const testModeScript = system.getScript("/ti/ti_wisunfan/test_config/"
    + "ti_wisunfan_test_config");

// Get security settings script
const securityScript = system.getScript("/ti/ti_wisunfan/security_config/"
    + "ti_wisunfan_security_config");

// Get wfantund settings script
const wfantundScript = system.getScript("/ti/ti_wisunfan/wfantund_config/"
    + "ti_wisunfan_wfantund_config");

// Get advanced settings script
const advancedScript = system.getScript("/ti/ti_wisunfan/advanced_config/"
    + "ti_wisunfan_advanced_config");

// Get top level setting descriptions
const docs = system.getScript("/ti/ti_wisunfan/ti_wisunfan_docs.js");
const solarDocs = system.getScript("/ti/ti_wisunfan/wsms/wsms_docs.js");

// Get common utility functions
const Common = system.getScript("/ti/ti_wisunfan/ti_wisunfan_common.js");

// Static module implementation
const wsmsStatic = {
    // Configurables for the static 15.4 module
    config: [
        {
            name: "lockProject",
            displayName: "Lock Project Type",
            default: false,
            hidden: true,
            description: docs.lockProject.description,
            longDescription: docs.lockProject.longDescription,
            onChange: wisunScript.extraExports.onLockProjectChange
        },
        {
            name: "genLibs",
            displayName: "Generate Wi-SUN Libraries",
            default: "all",
            hidden: true,
            description: docs.genLibs.description,
            longDescription: docs.genLibs.longDescription
        },
        {
            name: "project",
            displayName: "Device Role",
            default: "borderrouter",
            options: [
                {
                    name: "borderrouter",
                    displayName: "Border Router"
                },
                {
                    name: "routernode",
                    displayName: "Router"
                },
                {
                    name: "coapnode",
                    displayName: "CoAP"
                },
                {
                    name: "solarborderrouter",
                    displayName: "Solar Base Station (Border Router)"
                },
                {
                    name: "solarcoapnode",
                    displayName: "Solar Node (Router Node)"
                }
            ],
            description: solarDocs.project.description,
            longDescription: solarDocs.project.longDescription,
            onChange: wisunScript.extraExports.onProjectChange
        },
        {
            name: "mode",
            displayName: "Channel Function",
            default: "frequencyHopping",
            getDisabledOptions: wisunScript.extraExports.getDisabledModeOptions(),
            options: [
                {
                    name: "frequencyHopping",
                    displayName: "Frequency Hopping"
                },
                {
                    name: "fixedChannel",
                    displayName: "Fixed Channel"
                },
                
            ],
            description: solarDocs.mode.description,
            longDescription: solarDocs.mode.longDescription,
            onChange: wisunScript.extraExports.onModeChange
        },
        {
            name: "secureBoot",
            displayName: "Secure Boot Image",
            default: false,
            description: docs.secureBoot.description,
            longDescription: docs.secureBoot.longDescription,
        },

        radioScript.config,
        networkScript.config,
        powerScript.config,
        securityScript.config,
        wfantundScript.config,
        advancedScript.config
        //testModeScript.config
    ],
    validate: wisunScript.extraExports.validate,
    moduleInstances: wisunScript.extraExports.moduleInstances,
    modules: wisunScript.extraExports.modules
};

const wsmsStackModule = {
    displayName: "Wireless Solar Management System",
    description: solarDocs.wsmsModule.description,
    longDescription: solarDocs.wsmsModule.longDescription,
    moduleStatic: wsmsStatic,
    templates: {
        "/ti/ti_wisunfan/wsms/templates/ti_wsms_config.h.xdt": true,
        "/ti/ti_wisunfan/wsms/templates/ti_wsms_features.h.xdt": true,
        "/ti/utils/build/GenLibs.cmd.xdt": {
            modName: "/ti/ti_wisunfan/wsms/wsms",
            getLibs: wisunScript.templates["/ti/utils/build/GenLibs.cmd.xdt"].getLibs
        },
        "/ti/utils/build/GenOpts.opt.xdt": {
            modName: "/ti/ti_wisunfan/wsms/wsms",
            getOpts: wisunScript.templates["/ti/utils/build/GenOpts.opt.xdt"].getOpts
        }
    }
};

exports = wsmsStackModule;
