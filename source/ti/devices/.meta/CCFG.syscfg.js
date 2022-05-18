/*
 * Copyright (c) 2019-2021 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== CCFG.syscfg.js ========
 */

const Common = system.getScript("/ti/drivers/Common.js");
const family = Common.device2Family(system.deviceData, "CCFG");

const templateModule = [{
    name: "ccfgTemplate",
    moduleName: "/ti/devices/CCFG/CCFG" + family + "Template"
}];

let templateModuleInstance = templateModule;

let config = [
    {
        name: "enableCodeGeneration",
        displayName: "Enable SysConfig CCFG Configuration",
        description: "Enables or disables generation of ti_devices_config.c.",
        longDescription: `This configurable may be used to enable or disable the
generation of the ti_devices_config.c file. Examples without ti_devices_config.c
generation may still be configured through the ccfg.c source file. To support early
initialization on some devices, this module will continue to generate functions to
__Board_init()__ inside the ti_drivers_config.c file. Additionally, any necessary
headers will be generated inside the ti_drivers_config.h file.
        `,
        hidden: true,
        default: true,
        onChange: (inst, ui) => {
            templateModuleInstance = inst.enableCodeGeneration ? templateModule : [];

            const configs = Object.keys(ui).filter((key) => (!key.includes("$")
                && key !== "ccfgTemplate"));

            // Hide module configs if code generation is disabled
            configs.forEach((cfgName) =>
            {
                if(cfgName === "enableCodeGeneration")
                {
                    const readOnly = "The CCFG area cannot be configured "
                        + "through SysConfig for this example. Refer to "
                        + "the ccfg.c source file in order to overwrite the "
                        + "default configuration.";

                    ui[cfgName].hidden = inst.enableCodeGeneration;
                    ui[cfgName].readOnly = inst.enableCodeGeneration ? false : readOnly;
                }
                else
                {
                    const cfg = inst.$module.$configByName[cfgName];
                    // Reset config visibility if code generation is enabled
                    ui[cfgName].hidden = inst.enableCodeGeneration ? cfg.hidden : true;
                    inst[cfgName] = cfg.default;
                }
            });
        }
    }
];

let base = {
    displayName: "Device Configuration",
    description: "Customer Configuration",
    defaultInstanceName: "CONFIG_CCFG_",
    alwaysShowLongDescription : true,
    maxInstances: 1,
    moduleStatic: {
        moduleInstances: () => { return templateModuleInstance; },
        config: config
    },
    initPriority: 0
};

/* get family-specific CCFG module */
let devCCFG = system.getScript("/ti/devices/CCFG/CCFG" + family);

exports     = devCCFG.extend(base);