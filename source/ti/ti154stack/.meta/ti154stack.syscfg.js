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
 *  ======== ti154stack.syscfg.js ========
 */

// Get radio settings script
const radioScript = system.getScript("/ti/ti154stack/rf_config/"
    + "ti154stack_rf_config");

// Get network settings script
const networkScript = system.getScript("/ti/ti154stack/network_config/"
    + "ti154stack_network_config");

// Get transmit power settings script
const powerScript = system.getScript("/ti/ti154stack/power_config/"
    + "ti154stack_power_config");

// Get test mode settings script
const testModeScript = system.getScript("/ti/ti154stack/test_config/"
    + "ti154stack_test_config");

// Get OAD settings script
const oadScript = system.getScript("/ti/ti154stack/oad_config/"
    + "ti154stack_oad_config");

// Get security settings script
const securityScript = system.getScript("/ti/ti154stack/security_config/"
    + "ti154stack_security_config");

// Get top level setting descriptions
const docs = system.getScript("/ti/ti154stack/ti154stack_docs.js");

// Get common utility functions
const Common = system.getScript("/ti/ti154stack/ti154stack_common.js");

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
            displayName: "Generate 15.4 Libraries",
            default: true,
            hidden: true,
            description: docs.genLibs.description,
            longDescription: docs.genLibs.longDescription
        },
        {
            name: "loggingEnabled",
            displayName: "Enable Logging",
            hidden : false,
            description: `This setting will enable logging for the TI 15.4 Stack`,
            default: false
        },
        {
            name: "project",
            displayName: "Device Role",
            default: "collector",
            options: [
                {
                    name: "collector",
                    displayName: "Collector"
                },
                {
                    name: "collectorSM",
                    displayName: "Collector SM"
                },
                {
                    name: "sensor",
                    displayName: "Sensor"
                },
                {
                    name: "sensorSM",
                    displayName: "Sensor SM"
                },
                {
                    name: "sensorOAD",
                    displayName: "Sensor OAD"
                },
                {
                    name: "coprocessor",
                    displayName: "CoProcessor"
                }
            ],
            description: docs.project.description,
            longDescription: docs.project.longDescription,
            onChange: onProjectChange
        },
        {
            name: "rtos",
            displayName: "RTOS",
            default: "tirtos",
            options: [
                {
                    name: "tirtos",
                    displayName: "TIRTOS"
                },
                {
                    name: "freertos",
                    displayName: "FreeRTOS"
                }
            ],
            hidden: true
        },
        {
            name: "mode",
            displayName: "Mode",
            default: "nonBeacon",
            getDisabledOptions: getDisabledModeOptions(),
            options: [
                {
                    name: "beacon",
                    displayName: "Beacon"
                },
                {
                    name: "nonBeacon",
                    displayName: "Non Beacon"
                },
                {
                    name: "frequencyHopping",
                    displayName: "Frequency Hopping"
                }
            ],
            description: docs.mode.description,
            longDescription: docs.mode.longDescription,
            onChange: onModeChange
        },
        radioScript.config,
        networkScript.config,
        powerScript.config,
        securityScript.config,
        oadScript.config,
        testModeScript.config
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
    radioScript.setFreqBandReadOnlyState(ui, inst.lockProject);

    // Only lock the security dropdown if it's an SM project
    if(inst.project.includes("SM"))
    {
        securityScript.setSecureLevelReadOnlyState(ui,
            inst.lockProject);
    }
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
    // Set visibility of all configs
    networkScript.setAllNetworkConfigsHiddenState(inst, ui);
    powerScript.setAllPowerConfigsHiddenState(inst, ui);
    securityScript.setAllSecurityConfigsHiddenState(inst, ui);
    oadScript.setAllOADConfigsHiddenState(inst, ui);
    testModeScript.setAllTestConfigsHiddenState(inst, ui);
    radioScript.setAllRFConfigsHiddenState(inst, ui);

    setTopLevelConfigHiddenState(inst, ui, "mode");
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
    // Update group configs based on mode selected
    networkScript.setBeaconSuperFrameOrders(inst, ui);
    oadScript.setDefaultOadBlockReqRate(inst);
    oadScript.setDefaultOadBlockReqPollDelay(inst);

    // Set visibility of network group dependents
    networkScript.setNetworkConfigHiddenState(inst, ui, "channels");
    networkScript.setNetworkConfigHiddenState(inst, ui, "fhChannels");
    networkScript.setNetworkConfigHiddenState(inst, ui, "fhAsyncChannels");
    networkScript.setNetworkConfigHiddenState(inst, ui, "fhNetname");
    networkScript.setNetworkConfigHiddenState(inst, ui, "fhBroadcastDwellTime");
    networkScript.setNetworkConfigHiddenState(inst, ui, "fhBroadcastInterval");

    // Polling interval not used in beacon mode
    networkScript.setNetworkConfigHiddenState(inst, ui, "pollingInterval");
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
 * ======== getTopLevelConfigHiddenState ========
 * Get the expected visibility of the selected config
 *
 * @param inst    - module instance containing the config that changed
 * @param cfgName - name of config
 * @returns bool  - true if hidden, false if visible
 */
function getTopLevelConfigHiddenState(inst, cfgName)
{
    let isVisible = true;

    switch(cfgName)
    {
        case "lockProject":
        case "genLibs":
        {
            isVisible = false;
            break;
        }
        case "mode":
        {
            isVisible = inst.project !== "coprocessor";
            break;
        }
        case "project":
        default:
        {
            isVisible = true;
            break;
        }
    }

    // Return whether config is hidden
    return(!isVisible);
}

/*
 * ======== setTopLevelConfigHiddenState ========
 * Sets the visibility of the selected config
 *
 * @param inst    - module instance containing the config that changed
 * @param ui      - user interface object
 * @param cfgName - name of config
 */
function setTopLevelConfigHiddenState(inst, ui, cfgName)
{
    Common.setConfigHiddenState(inst, ui, cfgName, moduleStatic.config,
        getTopLevelConfigHiddenState);
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
    if((inst.freqBand === "freqBand24") && (inst.mode === "frequencyHopping"))
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
    // No validation needed for CoP as all configs are hidden and unused
    if(inst.project !== "coprocessor")
    {
        // Validation for the custom configurables
        validateModeOptions(inst, vo);

        // Call validation methods of all groups
        oadScript.validate(inst, vo);
        securityScript.validate(inst, vo);
    }

    networkScript.validate(inst, vo);
    powerScript.validate(inst, vo);
    radioScript.validate(inst, vo);
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
    let basePath = "";

    if(inst.$static.genLibs)
    {
        // Get device ID and toolchain to select appropriate libs
        const GenLibs = system.getScript("/ti/utils/build/GenLibs.syscfg.js");
        const toolchain = GenLibs.getToolchainDir();
        const rtos = system.getRTOS();

        // Generate correct maclib library to link based on device, security
        // level, and frequency band selected
        if (toolchain == "gcc")
        {
            basePath = "ti/ti154stack/lib/" +"ticlang";
        }
        else
        {
            basePath = "ti/ti154stack/lib/" + toolchain;
        }
        let security;
        switch(inst.$static.secureLevel)
        {
            case "macSecureAndCommissioning": security = "sm_"; break;
            case "macSecureDisabled": security = "nosecure_"; break;
            default: security = "secure_"; break;
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

        var maclibName = "maclib_" + security + devType + freq + rtosPath;

        if(inst.$static.tfmEnabled) //tfm projects use different libraries
        {
            maclibName = "tfm_" + maclibName
        }

        const maclib = inst.$static.loggingEnabled ? basePath + maclibName + "_log.a" : basePath + maclibName + ".a";
        libs.push(maclib);
        if(system.modules["/ti/dmm/dmm"] === undefined)
        {
            let macosallib = ""

            if(inst.$static.tfmEnabled) //tfm projects use different libraries
            {
                macosallib = basePath + "tfm_maclib_osal_" + rtos;
            }
            else
            {
                macosallib = basePath + "maclib_osal_" + rtos;
            }

            if(board.includes("R7") || board.includes("P7"))
            {
                macosallib += "_cc13x2x7_26x2x7.a";
            }
            else if(board.includes("R3") || board.includes("P3"))
            {
                macosallib += "_cc13x1_26x1.a";
            }
            else if(board.includes("R10") || board.includes("P10"))
            {
                macosallib += "_cc13x4_26x4.a";
            }
            else // cc13x2/cc26x2
            {
                macosallib += "_cc13x2_26x2.a";
            }

            if(inst.$static.loggingEnabled)
            {
                macosallib = macosallib.replace(".a", "_log.a")
            }
            libs.push(macosallib);
        }
    }

    // Create a GenLibs input argument
    const linkOpts = {
        name: "/ti/ti154stack",
        deps: [],
        libs: libs
    };

    return(linkOpts);
}

/*
 *  ======== getOpts ========
 */
function getOpts(mod) {
    const tmp_mod = system.modules["/ti/ti154stack/ti154stack"] ;
    const toad_mod = system.modules["/ti/ti154stack/oad_config/toad_config/ti154stack_toad_config"]
    let result = []

    if (system.getRTOS() === "tirtos7") {
        result.push(
            /*
            * The following -D requirements are unconditional.  Note that they
            * appear to only be used to configure the OSAL heap, and likely
            * could/should be reviewed and ideally removed.
            *
            * For example, -DTIRTOS7_SUPPORT only appears to be used in
            * ti/ti154stack/common/heapmgr/rtos_heaposal.h, and only to support (no
            * longer supported!) BIOS 6/XDC.
            */
            "-DTIRTOS7_SUPPORT",
            "-DHEAPMGR_CONFIG=0x80",
            "-DHEAPMGR_SIZE=0x0"
        );
    }
    if (tmp_mod.$static.dutyCycleEnable) {
        result.push(
            "-DMAC_DUTY_CYCLE_CHECKING",
            "-DMAC_DUTY_CYCLE_THRESHOLD=" + tmp_mod.$static.dutyCycle
        )
    }
    if (tmp_mod.$static.customMinTxOff) {
        result.push(
            "-DMAC_OVERRIDE_TX_DELAY",
            "-DMAC_CONFIG_MIN_TX_OFF=" + tmp_mod.$static.minTxOff
        )
    }
    if (!(toad_mod===undefined) && toad_mod.$instances[0].enabled) {
        result.push("-DFEATURE_TOAD")
    }

    // This is always defined in .opt, but I don't think it's actually used by us
    result.push("-DTI154STACK")

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
    const oadScriptModuleInst = oadScript.moduleInstances(inst);

    dependencyModule = dependencyModule.concat(radioScriptModuleInst);
    dependencyModule = dependencyModule.concat(securityScriptModuleInst);
    dependencyModule = dependencyModule.concat(oadScriptModuleInst);

    // Pull module for ti_154stack_config.h generation
    if(inst.project !== "coprocessor")
    {
        dependencyModule.push({
            name: "ti154stackModule",
            moduleName: "/ti/ti154stack/ti154stack_config_mod.js"
        });
    }

    /* If logging is enabled, push a dependency on a log module */
    if (inst.loggingEnabled) {
        dependencyModule.push(
            {
                name: "LogModule_154_App",
                displayName: "TI 15.4 Stack Application Level Log Configuration",
                moduleName: "/ti/log/LogModule",
                collapsed: true,
                args: {
                    $name: "LogModule_154_App",
                    // All logs enabled by default for the App Level Logs
                    enable_DEBUG: true,
                    enable_INFO: true,
                    enable_VERBOSE: true,
                    enable_WARNING: true,
                    enable_ERROR: true
                }
            }
        );
        dependencyModule.push(
            {
                name: "LogModule_154_Low_Level_MAC",
                displayName: "TI 15.4 Stack Low Level MAC Log Configuration",
                moduleName: "/ti/log/LogModule",
                collapsed: true,
                args: {
                    $name: "LogModule_154_Low_Level_MAC",
                    enable_DEBUG: false,
                    enable_INFO: false,
                    enable_VERBOSE: false,
                    // Only enable WARNING and ERROR enabled by default
                    enable_WARNING: true,
                    enable_ERROR: true
                }
            }
        );
        dependencyModule.push(
            {
                name: "LogModule_154_Low_Level_TX",
                displayName: "TI 15.4 Stack Low Level TX Log Configuration",
                moduleName: "/ti/log/LogModule",
                collapsed: true,
                args: {
                    $name: "LogModule_154_Low_Level_TX",
                    enable_DEBUG: false,
                    enable_INFO: false,
                    enable_VERBOSE: false,
                    // Only enable WARNING and ERROR enabled by default
                    enable_WARNING: true,
                    enable_ERROR: true
                }
            }
        );
        dependencyModule.push(
            {
                name: "LogModule_154_Low_Level_RX",
                displayName: "TI 15.4 Stack Low Level RX Log Configuration",
                moduleName: "/ti/log/LogModule",
                collapsed: true,
                args: {
                    $name: "LogModule_154_Low_Level_RX",
                    enable_DEBUG: false,
                    enable_INFO: false,
                    enable_VERBOSE: false,
                    // Only enable WARNING and ERROR enabled by default
                    enable_WARNING: true,
                    enable_ERROR: true
                }
            }
        );
    }

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
const ti154StackModule = {
    displayName: "TI 15.4 Stack",
    description: docs.ti154stackModule.description,
    longDescription: docs.ti154stackModule.longDescription,
    moduleStatic: moduleStatic,
    templates: {
        "/ti/ti154stack/templates/ti_154stack_features.h.xdt": true,
        "/ti/utils/build/GenLibs.cmd.xdt":
        {
            modName: "/ti/ti154stack/ti154stack",
            getLibs: getLibs
        },
        "/ti/utils/build/GenOpts.opt.xdt": {
            modName: "/ti/ti154stack/ti154stack",
            getOpts: getOpts
        }
    }
};

/*
 *  ======== exports ========
 *  Export the TI 15.4 Stack module definition
 */
exports = ti154StackModule;