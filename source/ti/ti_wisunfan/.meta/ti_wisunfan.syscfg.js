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
 *  ======== ti_wisunfan.syscfg.js ========
 */

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

// Get common utility functions
const Common = system.getScript("/ti/ti_wisunfan/ti_wisunfan_common.js");

// Static implementation of 15.4 module
const moduleStatic = {
    // Configurables for the static 15.4 module
    config: [
        {
            name: "lockProject",
            displayName: "Lock Project Type",
            default: false,
            hidden: true,
            description: docs.lockProject.description,
            longDescription: docs.lockProject.longDescription,
            onChange: onLockProjectChange
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
                    displayName: "Solar Base Station"
                },
                {
                    name: "solarcoapnode",
                    displayName: "Solar Node"
                }
            ],
            description: docs.project.description,
            longDescription: docs.project.longDescription,
            onChange: onProjectChange
        },
        {
            name: "mode",
            displayName: "Channel Function",
            default: "frequencyHopping",
            getDisabledOptions: getDisabledModeOptions(),
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
            description: docs.mode.description,
            longDescription: docs.mode.longDescription,
            onChange: onModeChange
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
    validate: validate,
    moduleInstances: moduleInstances,
    modules: modules
};

/*
 *******************************************************************************
 Top Level Group Config Functions
 *******************************************************************************
 */

/*
 * ======== onLockProjectChange ========
 * On change function for lockProject config
 * Sets project, frequency band, and security level (if SM) to read only when
 * selected
 *
 * @param inst - 15.4 instance
 * @param ui   - user interface object
 */
function onLockProjectChange(inst, ui)
{
    setProjectReadOnlyState(ui, inst.lockProject);
}

/*
 * ======== onProjectChange ========
 * On change function for project config
 * Sets visibility of project-specific configs
 *
 * @param inst - 15.4 instance
 * @param ui   - user interface object
 */
function onProjectChange(inst, ui)
{
    // Set visibility of network group dependents
    networkScript.setNetworkConfigHiddenState(inst, ui, "maxDevices");
    networkScript.setNetworkConfigHiddenState(inst, ui, "fhBroadcastDwellTime");
    networkScript.setNetworkConfigHiddenState(inst, ui, "fhBroadcastInterval");
    networkScript.setNetworkConfigHiddenState(inst, ui, "unicastDwellTime");
    networkScript.setNetworkConfigHiddenState(inst, ui, "fhNetname");
    networkScript.setNetworkConfigHiddenState(inst, ui, "fhBroadcastDwellTime");
    networkScript.setNetworkConfigHiddenState(inst, ui, "broadcastChannelMask");
    networkScript.setNetworkConfigReadonlyState(inst, ui, "panID");

    // Set visibility of power group dependents
    powerScript.setPowerConfigHiddenState(inst, ui, "rxOnIdle");

    // Set visibility of test group dependents
    /*testModeScript.setTestConfigHiddenState(inst, ui,
        "collectorTestRampDataSize");
    testModeScript.setTestConfigHiddenState(inst, ui,
        "sensorTestRampDataSize");*/

    // Solar example defaults/hidden setting changes
    networkScript.setDefaultNetname(inst);
    networkScript.setDefaultDwellTime(inst);
    wfantundScript.setDefaultWfantundSettings(inst);
    wfantundScript.setWfantundSettingsHiddenState(inst, ui);
    advancedScript.setDefaultAdvancedSettings(inst);
    advancedScript.setAdvancedSettingsHiddenState(inst, ui);
}

/*
 * ======== onModeChange ========
 * On change function for mode config
 * Sets visibility of mode-specific configs and updates beacon-mode order
 * configs
 *
 * @param inst - 15.4 instance
 * @param ui   - user interface object
 */
function onModeChange(inst, ui)
{
    // Update network group configs based on mode selected

    // Set visibility of network group dependents
    networkScript.setNetworkConfigHiddenState(inst, ui, "channels");
    networkScript.setNetworkConfigHiddenState(inst, ui, "unicastChannelMask");
    networkScript.setNetworkConfigHiddenState(inst, ui, "broadcastChannelMask");
    networkScript.setNetworkConfigHiddenState(inst, ui, "fhAsyncChannels");
    networkScript.setNetworkConfigHiddenState(inst, ui, "unicastChannelMask");
    networkScript.setNetworkConfigHiddenState(inst, ui, "broadcastChannelMask");
    networkScript.setNetworkConfigHiddenState(inst, ui, "fhNetname");
    networkScript.setNetworkConfigHiddenState(inst, ui, "fhBroadcastDwellTime");
    networkScript.setNetworkConfigHiddenState(inst, ui, "fhBroadcastInterval");

    // Polling interval not used in be
    networkScript.setDefaultChannelMasks(inst);

}

/*
 * ======== setProjectReadOnlyState ========
 * Sets the read only status of project config. If config is read only, a
 * read only reason is displayed
 *
 * @param ui       - user interface object
 * @param readOnly - true if config must be set to read only
 */
function setProjectReadOnlyState(ui, readOnly)
{
    // Set read only state of config
    ui.project.readOnly = (readOnly) ? docs.project.readOnly : false;
}

/*
 *  ======== getDisabledModeOptions ========
 *  Generates a list of options that should be disabled in the mode
 *  drop-down. Only FH is disabled when 2.4GHz band is selected
 *
 * @returns Array - array of strings that should be disabled
 */
function getDisabledModeOptions()
{
    return(inst) =>
    {
        const disabledOptions = [];
        // Add FH mode option to list of disabled options if 2.4GHz project
        if(inst.freqBand === "freqBand24")
        {
            disabledOptions.push({
                name: "frequencyHopping",
                reason: "Frequency hopping is not supported on 2.4 GHz band"
            });
        }
        return(disabledOptions);
    };
}

/*
 * ======== validateModeOptions ========
 * Validate this inst's mode configuration
 *
 * @param inst - 15.4 instance to be validated
 * @param vo   - object to hold detected validation issues
 */
function validateModeOptions(inst, vo)
{
    if((inst.freqBand === "freqBand24") && (inst.mode === "frequencyHopping" || inst.mode === "fixedChannel"))
    {
        vo.logError("Frequency hopping not available on 2.4GHz band", inst,
            "mode");
    }
}

/*
 * ======== validate ========
 * Validate this inst's configuration by calling each configurable's
 * validation function and any validation function for custom
 * configurables
 *
 * @param inst - 15.4 instance to be validated
 * @param vo   - object to hold detected validation issues
 */
function validate(inst, vo)
{
    // Add info statement for secure boot configuration
    if(inst.secureBoot === true)
    {
        vo.logInfo("Enabling secure boot image generation requires the \
            project configuration to be set to SecureBoot instead of Release",
            inst, "secureBoot");
    }

    // Validation for the custom configurables
    validateModeOptions(inst, vo);

    // Call validation methods of all groups
    radioScript.validate(inst, vo);
    networkScript.validate(inst, vo);
    powerScript.validate(inst, vo);
    securityScript.validate(inst, vo);
    if(inst.project.includes("borderrouter"))
    {
        wfantundScript.validate(inst, vo);
    }
    advancedScript.validate(inst, vo);
}

/*
 *******************************************************************************
 Module Dependencies and Exports
 *******************************************************************************
 */

/*
 * ======== getLibs ========
 * Contribute libraries to linker command file
 *
 * @param inst  - 15.4 module instance
 * @returns     - Object containing the name of component, array of dependent
 *                components, and array of library names
 */
function getLibs(inst)
{
    const libs = [];
    const board = Common.getLaunchPadFromDevice();

    if(inst.$static.genLibs !== "none")
    {
        // Get device ID and toolchain to select appropriate libs
        const GenLibs = system.getScript("/ti/utils/build/GenLibs.syscfg.js");
        const toolchain = GenLibs.getToolchainDir();
        const rtos = system.getRTOS();

        // Generate correct maclib library to link based on device, security
        // level, and frequency band selected
        let basePath = "ti/ti_wisunfan/lib/" + toolchain;

        let security;
        switch(inst.$static.secureLevel)
        {
            case "macSecureDisabled": security = "nosecure_"; break;
            default: security = "secure_"; break;
        }

        let mbedType;
        switch(inst.$static.project)
        {
            case "borderrouter": mbedType = "br"; break;
            case "routernode":   mbedType = "rn"; break;
            case "coapnode":     mbedType = "coap"; break;
            case "solarborderrouter": mbedType = "br"; break;
            case "solarcoapnode":     mbedType = "coap"; break;
        }

        let devType;
        if(board.includes("R7") || board.includes("P7"))
        {
            devType = Common.isSub1GHzDevice() ? "cc13x2x7" : "cc26x2x7";
            basePath += "/m4f/";
        }
        else if(board.includes("R3") || board.includes("P3"))
        {
            devType = Common.isSub1GHzDevice() ? "cc13x1" : "cc26x1";
            basePath += "/m4/";
        }
        else if(board.includes("R10") || board.includes("P10"))
        {
            devType = Common.isSub1GHzDevice() ? "cc13x4" : "cc26x4";
            basePath += "/m33f/";
        }
        else // cc13x2/cc26x2
        {
            devType = Common.isSub1GHzDevice() ? "cc13x2" : "cc26x2";
            basePath += "/m4f/";
        }
        const freq = (inst.$static.freqBand === "freqBand24") ? "_2_4g" : "";
        let rtosPath = (rtos === "tirtos7") ? "_tirtos7" : "";
        if (rtos === "freertos")
        {
            rtosPath = "_freertos";
        }

        const maclibName = "maclib_" + security + devType + freq + rtosPath;
        const maclib = basePath + maclibName + ".a";
        libs.push(maclib);

        if(inst.$static.genLibs !== "macOnly")
        {
            const mbedlibName = "wisun_" + mbedType + "_mbed_ns_tls_lib_" + devType + rtosPath;
            const mbedlib = basePath + mbedlibName + ".a";
            libs.push(mbedlib);
            if (mbedType != "coap")
            {
                const ncplibName = "wisun_ncp_lib_" + devType + rtosPath;
                const ncplib = basePath + ncplibName + ".a";
                libs.push(ncplib);
            }
        }
    }

    // Create a GenLibs input argument
    const linkOpts = {
        name: "/ti/ti_wisunfan",
        deps: [],
        libs: libs
    };

    return(linkOpts);
}

/*
 *  ======== getOpts ========
 */
function getOpts(inst) {
    let result =[]

    if (system.getRTOS() === "tirtos7") {
        result.push(
            "-DTIRTOS7_SUPPORT",
            "-DHEAPMGR_CONFIG=0x80",
            "-DHEAPMGR_SIZE=0x0"
        );
    }
    if (inst.$static.secureLevel === "macSecureDisabled") {
        //#define NETWORK_AUTH_TYPE NO_AUTH
        result.push("-DNO_AUTH_ENABLE");
    }
    else if(inst.$static.secureLevel === "macSecurePresharedKey") {
        if(inst.$static.euiJoin === false) {
            result.push("-DPRESHARED_KEY_AUTH_ENABLE");
        } else {
            result.push("-DCUSTOM_EUI_AUTH_ENABLE");
        }
    } else { // Default security
        result.push("-DDEFAULT_MBEDTLS_AUTH_ENABLE");
    }

    if (inst.$static.dutyCycleEnable) {
        result.push("-DMAC_DUTY_CYCLE_CHECKING");
        result.push("-DMAC_DUTY_CYCLE_THRESHOLD=(" + inst.$static.dutyCycle + ")");
    }

    if (inst.$static.customMinTxOff) {
        result.push("-DMAC_OVERRIDE_TX_DELAY");
        result.push("-DMAC_CONFIG_MIN_TX_OFF=(" + inst.$static.minTxOff + ")");
    }

    return result; 
}

/*
 *  ======== moduleInstances ========
 *  Determines what modules are added as non-static submodules
 *
 *  @param inst  - Module instance containing the config that changed
 *  @returns     - Array containing dependency modules
 */
function moduleInstances(inst)
{
    let dependencyModule = [];

    // Pull in dependency modules
    const radioScriptModuleInst = radioScript.moduleInstances(inst);
    const securityScriptModuleInst = securityScript.moduleInstances(inst);

    dependencyModule = dependencyModule.concat(radioScriptModuleInst);
    dependencyModule = dependencyModule.concat(securityScriptModuleInst);

    return(dependencyModule);
}

/*
 *  ======== modules ========
 *  Determines what modules are added as static submodules
 *
 *  @param inst  - Module instance containing the config that changed
 *  @returns     - Array containing static dependency modules
 */
function modules(inst)
{
    const dependencyModule = [];

    // Pull in static dependency modules
    dependencyModule.push({
        name: "multiStack",
        displayName: "Multi-Stack Validation",
        moduleName: "/ti/common/multi_stack_validate",
        hidden: true
    });

    dependencyModule.push({
        name: "rfDesign",
        displayName: "RF Design",
        moduleName: "/ti/devices/radioconfig/rfdesign",
        hidden: true
    });

    dependencyModule.push({
        name: "rfModule",
        displayName: "RF",
        moduleName: "/ti/drivers/RF",
        hidden: true

    });

    dependencyModule.push({
        name: "powerModule",
        displayName: "Power",
        moduleName: "/ti/drivers/Power",
        hidden: true

    });

    return(dependencyModule);
}

/*
 *  ======== ti154StackModule ========
 *  Define the TI 15.4 Stack module properties and methods
 */
const ti_wisunfanStackModule = {
    displayName: "TI Wi-SUN FAN Stack",
    description: docs.ti_wisunfanModule.description,
    longDescription: docs.ti_wisunfanModule.longDescription,
    moduleStatic: moduleStatic,
    templates: {
        "/ti/ti_wisunfan/templates/ti_wisunfan_config.h.xdt": true,
        "/ti/ti_wisunfan/templates/ti_wisunfan_features.h.xdt": true,
        "/ti/utils/build/GenLibs.cmd.xdt":
        {
            modName: "/ti/ti_wisunfan/ti_wisunfan",
            getLibs: getLibs
        },
        "/ti/utils/build/GenOpts.opt.xdt": {
            modName: "/ti/ti_wisunfan/ti_wisunfan",
            getOpts: getOpts
        }
    },
    extraExports: {
        onLockProjectChange: onLockProjectChange,
        onProjectChange: onProjectChange,
        getDisabledModeOptions: getDisabledModeOptions,
        onModeChange: onModeChange,
        validate: validate,
        moduleInstances: moduleInstances,
        modules: modules
    }
};

/*
 *  ======== exports ========
 *  Export the TI 15.4 Stack module definition
 */
exports = ti_wisunfanStackModule;
