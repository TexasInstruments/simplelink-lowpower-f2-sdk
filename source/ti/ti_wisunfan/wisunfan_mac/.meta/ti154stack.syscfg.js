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
                }
            ],
            description: docs.project.description,
            longDescription: docs.project.longDescription,
            onChange: onProjectChange
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
    // Set visibility of network group dependents
    networkScript.setNetworkConfigHiddenState(inst, ui, "maxDevices");
    networkScript.setNetworkConfigHiddenState(inst, ui, "trackingDelayTime");
    networkScript.setNetworkConfigHiddenState(inst, ui, "fhBroadcastDwellTime");
    networkScript.setNetworkConfigHiddenState(inst, ui, "fhBroadcastInterval");
    networkScript.setNetworkConfigHiddenState(inst, ui, "scanBackoffInterval");
    networkScript.setNetworkConfigHiddenState(inst, ui,
        "orphanBackoffInterval");

    // Set visibility of power group dependents
    powerScript.setPowerConfigHiddenState(inst, ui, "rxOnIdle");

    // Set visibility of security group dependents
    securityScript.setSecurityConfigHiddenState(inst, ui, "smDefaultAuthCode");
    securityScript.setSecurityConfigHiddenState(inst, ui,
        "smSensorAuthMethods");
    securityScript.setSecurityConfigHiddenState(inst, ui,
        "smCollectorAuthMethods");
    securityScript.setSecurityConfigHiddenState(inst, ui,
        "smKeyrefreshTimeout");

    // Set visibility of OAD group dependents
    oadScript.setOADConfigHiddenState(inst, ui, "oadBlockSize");
    oadScript.setOADConfigHiddenState(inst, ui, "oadBlockReqRate");
    oadScript.setOADConfigHiddenState(inst, ui, "oadBlockReqPollDelay");

    // Set visibility of test group dependents
    testModeScript.setTestConfigHiddenState(inst, ui,
        "collectorTestRampDataSize");
    testModeScript.setTestConfigHiddenState(inst, ui,
        "sensorTestRampDataSize");
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
    networkScript.setBeaconSuperFrameOrders(inst, ui);

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
    // Validation for the custom configurables
    validateModeOptions(inst, vo);

    // Call validation methods of all groups
    radioScript.validate(inst, vo);
    networkScript.validate(inst, vo);
    powerScript.validate(inst, vo);
    oadScript.validate(inst, vo);
    securityScript.validate(inst, vo);
}

/*
 *******************************************************************************
 Module Dependencies and Exports
 *******************************************************************************
 */

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
        moduleName: "/ti/easylink/multi_stack_validate",
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
        "/ti/ti154stack/templates/ti_154stack_config_wisun.h.xdt": true,
        "/ti/ti154stack/templates/ti_154stack_features_wisun.h.xdt": true
    }
};

/*
 *  ======== exports ========
 *  Export the TI 15.4 Stack module definition
 */
exports = ti154StackModule;
