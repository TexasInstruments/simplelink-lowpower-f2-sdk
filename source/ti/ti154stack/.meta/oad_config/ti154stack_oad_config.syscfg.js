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
 *  ======== ti154stack_oad_config.syscfg.js ========
 */

"use strict";

// Get common utility functions
const Common = system.getScript("/ti/ti154stack/ti154stack_common.js");

// Get OAD setting descriptions
const Docs = system.getScript("/ti/ti154stack/oad_config/"
    + "ti154stack_oad_config_docs.js");

// Get TOAD module descriptions
const toadModuleDocs = system.getScript("/ti/ti154stack/oad_config/"
    + "toad_config/ti154stack_toad_config_docs.js");

// Configurables for the static 15.4 OAD settings group
const config = {
    name: "oadSettings",
    displayName: "Over-the-Air Download (OAD)",
    description: "Configure settings for Over-the-Air Downloads",
    config: [
        {
            name: "oadBlockSize",
            displayName: "Block Size (bytes)",
            default: getDefaultOadBlockSize(Common.isSub1GHzDevice()),
            hidden: true,
            description: Docs.oadBlockSize.description,
            longDescription: Docs.oadBlockSize.longDescription
        },
        {
            name: "oadBlockReqRate",
            displayName: "Block Request Rate (ms)",
            default: 200,
            hidden: true,
            description: Docs.oadBlockReqRate.description,
            longDescription: Docs.oadBlockReqRate.longDescription
        },
        {
            name: "oadBlockReqPollDelay",
            displayName: "Block Request Poll Delay (ms)",
            default: 50,
            hidden: true,
            description: Docs.oadBlockReqPollDelay.description,
            longDescription: Docs.oadBlockReqPollDelay.longDescription
        }
    ]
};

/*
 * ======== getDefaultOadBlockSize ========
 * Returns default OAD block size based on frequency band selected
 *
 * @returns int - default block size
 */
function getDefaultOadBlockSize(subGSelected)
{
    return subGSelected ? 128 : 64;
}

/*
 * ======== getDefaultOadBlockReqRate ========
 * Returns default OAD block request rate based on the PHY type and mode
 * selected
 *
 * @returns int - default block size
 */
function getDefaultOadBlockReqRate(mode, phyType)
{
    return mode === "frequencyHopping" && phyType === "phy5kbps" ? 400 : 200;
}

/*
 * ======== getDefaultOadBlockReqPollDelay ========
 * Returns default OAD block request poll delay based on the PHY type and mode
 * selected
 *
 * @returns int - default block size
 */
function getDefaultOadBlockReqPollDelay(mode, phyType)
{
    return mode === "frequencyHopping" && phyType === "phy5kbps" ? 100 : 50;
}

/*
 * ======== setDefaultOADBlockSize ========
 * Sets default OAD block size based on frequency band selected
 *
 * @param inst   - module instance containing the config to be changed
 * @param freqBandSelected  - frequency band currently selected
 */
function setDefaultOADBlockSize(inst, freqBandSelected)
{
    const subGSelected = (freqBandSelected === "freqBandSub1");
    inst.oadBlockSize = getDefaultOadBlockSize(subGSelected);
}

/*
 * ======== setDefaultOadBlockReqRate ========
 * Sets default OAD block request rate based on the PHY type and mode selected
 *
 * @param inst   - module instance containing the config to be changed
 */
function setDefaultOadBlockReqRate(inst)
{
    inst.oadBlockReqRate = getDefaultOadBlockReqRate(inst.mode, inst.phyType);
}

/*
 * ======== setDefaultOadBlockReqPollDelay ========
 * Sets default OAD block request poll delay based on the PHY type and mode
 * selected
 *
 * @param inst   - module instance containing the config to be changed
 */
function setDefaultOadBlockReqPollDelay(inst)
{
    inst.oadBlockReqPollDelay = getDefaultOadBlockReqPollDelay(inst.mode,
        inst.phyType);
}

/*
 * ======== getOADConfigHiddenState ========
 * Get the expected visibility of the selected config
 *
 * @param inst    - module instance containing the config that changed
 * @param cfgName - name of config
 * @returns bool  - true if hidden, false if visible
 */
function getOADConfigHiddenState(inst, cfgName)
{
    let isVisible = true;
    switch(cfgName)
    {
        case "oadBlockSize":
        case "oadBlockReqRate":
        case "oadBlockReqPollDelay":
        {
            isVisible = inst.project.includes("OAD");
            break;
        }
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
 * ======== setOADConfigHiddenState ========
 * Sets the visibility of the selected config
 *
 * @param inst    - module instance containing the config that changed
 * @param ui      - user interface object
 * @param cfgName - name of config
 */
function setOADConfigHiddenState(inst, ui, cfgName)
{
    Common.setConfigHiddenState(inst, ui, cfgName, config.config,
        getOADConfigHiddenState);
}

/*
 * ======== setAllOADConfigsHiddenState ========
 * Sets the visibility of all OAD configs
 *
 * @param inst    - module instance
 * @param ui      - user interface object
 */
function setAllOADConfigsHiddenState(inst, ui)
{
    Common.setAllConfigsHiddenState(inst, ui, config.config,
        getOADConfigHiddenState);
}

/*
 * ======== validate ========
 * Validate this inst's configuration
 *
 * @param inst       - OAD settings instance to be validated
 * @param validation - object to hold detected validation issues
 */
function validate(inst, validation)
{
    // Validate ranges if config is not hidden
    if(!getOADConfigHiddenState(inst, "oadBlockSize"))
    {
        if(inst.freqBand === "freqBand24")
        {
            Common.validateRangeInt(inst, validation, "oadBlockSize", 16, 64);
        }
        else
        {
            Common.validateRangeInt(inst, validation, "oadBlockSize", 16, 496);
        }
    }

    if(!getOADConfigHiddenState(inst, "oadBlockReqPollDelay"))
    {
        Common.validateRangeInt(inst, validation, "oadBlockReqPollDelay", 0,
            Common.cTypeMax.u_int32);
    }
    if(!getOADConfigHiddenState(inst, "oadBlockReqRate"))
    {
        Common.validateRangeInt(inst, validation, "oadBlockReqRate", 0,
            Common.cTypeMax.u_int32);
    }

    // Checking for null since the TOAD module is only added for OAD projects
    if(inst.toadSettings != null && inst.toadSettings.enabled)
    {
        // 15.4 Stack specific T-OAD validation
        if(inst.toadSettings.blockCacheSize < inst.oadBlockSize)
        {
            validation.logError(
                "Block cache size must be greater than the 15.4 OAD block size",
                inst.toadSettings, "blockCacheSize"
            );
        }
    }
}

/*
 *******************************************************************************
 Module Dependencies and Exports
 *******************************************************************************
 */

/*
 *  ======== moduleInstances ========
 *  Determines what modules are added as non-static sub-modules
 *
 *  @param inst  - Module instance containing the config that changed
 *  @returns     - Array containing dependency modules
 */
function moduleInstances(inst)
{
    const dependencyModule = [];

    // Pull OAD module for ti_154stack_oad_config.h generation
    if(inst.project === "sensorOAD")
    {
        dependencyModule.push({
            name: "oadSettings",
            moduleName: "/ti/ti154stack/oad_config/ti154stack_oad_mod.js",
            group: "oadSettings"
        });

        // Hide the Turbo OAD module for on-chip OAD projects until supported
        const extFlashModule = "/ti/drivers/nvs/NVSSPI25XDevice";
        const isToadHidden = !(extFlashModule in system.modules);

        // Prevent adding module so it will not show in 'other dependencies'
        if(!isToadHidden)
        {
            dependencyModule.push({
                name: "toadSettings",
                displayName: "Turbo OAD",
                description: toadModuleDocs.toadModule.description,
                moduleName: "/ti/ti154stack/oad_config/toad_config/"
                            + "ti154stack_toad_config",
                group: "oadSettings",
                hidden: isToadHidden
            });
        }
    }

    return(dependencyModule);
}

// Exports to the top level 15.4 module
exports = {
    config: config,
    validate: validate,
    moduleInstances: moduleInstances,
    setOADConfigHiddenState: setOADConfigHiddenState,
    getOADConfigHiddenState: getOADConfigHiddenState,
    setAllOADConfigsHiddenState: setAllOADConfigsHiddenState,
    setDefaultOADBlockSize: setDefaultOADBlockSize,
    setDefaultOadBlockReqRate: setDefaultOadBlockReqRate,
    setDefaultOadBlockReqPollDelay: setDefaultOadBlockReqPollDelay
};
