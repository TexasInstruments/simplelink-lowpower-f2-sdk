/*
 * Copyright (c) 2019-2024, Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== Board.syscfg.js ========
 */

/* Module used to invoke Board.c and Board.h templates */

"use strict";

/* Get ti/drivers common utility functions */
let Common = system.getScript("/ti/drivers/Common.js");

/* Get /ti/drivers family name from device object */
let family = Common.device2Family(system.deviceData, "Board");

let config = [];

function getLinkerDefs()
{
    let linkerDefs = [];

    /* NVS-related definitions */
    let nvsModule = system.modules["/ti/drivers/NVS"];
    if (nvsModule) {
        for (let i = 0; i < nvsModule.$instances.length; i++) {
            let inst = nvsModule.$instances[i];
            if ((inst.nvsType == "Internal") &&
                (inst.internalFlash.regionType == "Generated")) {
                    linkerDefs.push(
                    {
                        "name": "NVS_" + inst.$name + "_BASE",
                        "value": inst.internalFlash.regionBase
                    },
                    {
                        "name": "NVS_" + inst.$name + "_SIZE",
                        "value": inst.internalFlash.regionSize
                    }
                );
            }
        }
    }

    /* TODO: add UDMA-related config base addresses - currently disabled */
//    let deviceId = system.deviceData.deviceId;
//    if (deviceId.match(/CC2340/)) {
//        linkerDefs.push({ name: "UDMALPF3_CONFIG_BASE", value: 0x20000400 });
//    }

    return linkerDefs;
}

/*
 *  ======== getLibs ========
 */
function getLibs(mod)
{
    /* Get device ID to select appropriate libs */
    let devId = system.deviceData.deviceId;

    /* Get device information from DriverLib */
    var DriverLib = system.getScript("/ti/devices/DriverLib");
    let family = DriverLib.getAttrs(devId).libName;

    /* Get current RTOS configuration information */
    let rtos = system.getRTOS();

    /* Get toolchain specific information from GenLibs */
    let GenLibs = system.getScript("/ti/utils/build/GenLibs");

    /* The drivers libraries with logging enabled use a _log suffix vs the
     * unlogged library name.
     * If any LogModule is shared by a drivers module the _log suffix must
     * be selected.
     */
    var log_suffix = "";
    if (system.modules["/ti/log/LogModule"]) {
        // eslint-disable-next-line no-undef
        if (_.some(system.modules["/ti/log/LogModule"].$instances,
                   (inst) => {
                       if (inst.$ownedBy)
                       {
                           return inst.$ownedBy.$module.$name.startsWith("/ti/drivers");
                       }
                       else
                       {
                           return false;
                       }
                    }
                   )
            )
        {
            log_suffix = "_log";
        }
    }

    let libPath = GenLibs.libPath;
    let libs = [];

    if (family != "") {
        /* Add dependency on PSA Crypto library if key store module is present */
        if (system.modules["/ti/drivers/CryptoKeyKeyStore_PSA"]) {
            libs.push(libPath("third_party/psa_crypto", "psa_crypto_" + family + ".a"));
        }

        /* Check for TrustZone module */
        let tfmEnabled = family.match(/cc(13|26).[34]|cc27|cc35/) && system.modules["/ti/utils/TrustZone"];

        if(tfmEnabled){
            libs.push(libPath("ti/drivers","drivers_" + family + "_ns" + log_suffix +".a"));
        }
        else{
            libs.push(libPath("ti/drivers","drivers_" + family + log_suffix +".a"));
        }

        if (!family.match(/cc(13|26).[34]|cc23|cc27|cc35/)) {
            libs.push(libPath("ti/grlib", "grlib.a"));
        }

        /* Workaround to handle circular dependencies between the NoRTOS DPL lib
         * and the drivers lib. For example, the NoRTOS DPL depends on the Power
         * driver, and many drivers depend on HwiP.
         * The "/nortos/dpl/Settings" module defines through its getLibs()
         * function a dependency on the drivers lib for the NoRTOS DPL lib.
         * This results in the NoRTOS DPL lib to be linked first.
         * Below ensures that the NoRTOS DPL lib is also linked after the
         * drivers lib. Resulting in the NoRTOS DPL lib being linked both before
         * and after the drivers lib.
         * This is not needed for IAR, so below is skipped for IAR.
         */
        if (rtos == "nortos" && system.compiler != "iar") {
            let Settings = system.getScript("/nortos/dpl/Settings");
            libs = libs.concat(Settings.getLibs().libs);
        }

    }

    if (libs == null) {
        throw Error("device2LinkCmd: unknown device family ('" + family + "') for deviceId '" + devId + "'");
    }

    /* Create a GenLibs input argument */
    var linkOpts = {
        name: "/ti/drivers",
        vers: "1.0.0.0",
        deps: ["/ti/devices"],
        libs: libs
    };

    /* Add dependency on useFatFS configuration (if needed) */
    if (system.modules["/ti/drivers/SD"]) {
        let sdModule = system.modules["/ti/drivers/SD"];
        for (let i = 0; i < sdModule.$instances.length; i++) {
            let inst = sdModule.$instances[i];
            if (inst.useFatFS === true) {
                linkOpts.deps.push("/third_party/fatfs");
                break;
            }
        }
    }

    if (system.modules["/ti/drivers/ECDSA"] || system.modules["/ti/drivers/ECIES"]
        || system.modules["/ti/drivers/ECDH"]) {
        /* Add dependency on ECC library for CC13x1/CC26x1 and CC23x0 */
        if (family.match(/cc13.1/) || family.match(/cc26.1/) || family.match(/cc23.0/)) {
            linkOpts.deps.push("/third_party/ecc");
        }
    }

    if (system.modules["/ti/drivers/AESCCM"] ||
        system.modules["/ti/drivers/AESCMAC"] ||
        system.modules["/ti/drivers/SHA2"] ||
        system.modules["/ti/drivers/AESECB"] ||
        system.modules["/ti/drivers/AESCTR"] ||
        system.modules["/ti/drivers/AESCTRDRBG"] ||
        system.modules["/ti/drivers/AESGCM"] ||
        system.modules["/ti/drivers/AESCBC"] ||
        system.modules["/ti/drivers/ECDH"] ||
        system.modules["/ti/drivers/ECDSA"] ||
        system.modules["/ti/drivers/TRNG"] ||
        system.modules["/ti/drivers/RNG"] ||
        system.modules["/ti/drivers/CryptoKeyKeyStore_PSA"]) {
        /* Add dependency on HSMDDK library for CC27XX and CC35XX */
        if (family.match(/cc27/) || family.match(/cc35/)) {
            linkOpts.deps.push("/third_party/hsmddk");
        }
    }

    return (linkOpts);
}

/*
 *  ======== modules ========
 *  Reflect on HW components to "autoload" any support modules required
 */
function modules(mod)
{
    let reqs = [];

    reqs.push({
        name      : "GPIO",
        moduleName: "/ti/drivers/GPIO",
        hidden    : false
    });

    reqs.push({
        name      : "Driverlib",
        moduleName: "/ti/devices/DriverLib",
        hidden    : true
    });

    if (system.getRTOS() === "tirtos7") {
        reqs.push({
            name      : "DPL",
            moduleName: "/ti/dpl/Settings",
            hidden    : true
        });
    } else if (system.getRTOS() === "freertos") {
        reqs.push({
            name      : "DPL",
            moduleName: "/freertos/dpl/Settings",
            hidden    : true
        });
    } else if (system.getRTOS() === "nortos") {
        reqs.push({
            name      : "DPL",
            moduleName: "/nortos/dpl/Settings",
            hidden    : true
        });
    }

    if (system.deviceData.board && system.deviceData.board.components) {

        /* Accumulate all modules required by the board's components */
        let mods = {};
        let comps = system.deviceData.board.components;
        for (let cname in comps) {
            let comp = comps[cname];
            if (comp.settings && comp.settings.requiredModule) {
                mods[comp.settings.requiredModule] = comp;
            }
        }

        /* Create module requirements */
        for (let mname in mods) {
            reqs.push({
                name: mname.replace(/\//g, '_'), // private property name
                moduleName: mname,               // module to implicitly add
                hidden: true                     // don't show it as dependency
            });
        }
    }

    return (reqs);
}

/*
 *  ======== base ========
 */
let base = {
    displayName  : "Board",
    staticOnly   : true,
    maxInstances : 1,
    description  : "Board Support Module",
    alwaysShowLongDescription : true,
    longDescription: `
This module supports users utilizing Texas Instruments boards by generating
board-specific support functions. For more details, please visit the
[__Configuration Options__][1] and the [__Board Description__][2].

[1]: /drivers/syscfg/html/ConfigDoc.html#Board_Configuration_Options "Configuration options reference"
[2]: /drivers/syscfg/html/ConfigDoc.html#Board_Description "Board description reference"
`,

    documentation: `
This module provides basic validation required by any Hardware
Component that has been added to the board.

Hardware Components that require validation simply declare the name of
a module that must be added to a configuration.  The declaration is
made by adding a "requiredModule" field to the settings structure of the
Hardware Component.

The module named by a requiredModule field implements a validation
function that can reflect on the state of the configuration and plant
errors, warnings, and/or info messages throughout the system as
needed.

The example below is a fragment of the TMP116 component from the
BOOSTXL-BASSENSORS.syscfg.json:

        "TMP116": {
            "type": "TMP116",
            "displayName": "TMP116 Temperature Sensor",
            "description": "Accurate Digital Temperature Sensor ...",
            "settings": {
                "requiredModule": "/ti/boards/boosterpacks/bas/BAS",
                :
            },
            :
        }

In this case, whenever this TMP116 component is added to the board's
Hardware Components, the /ti/boards/boosterpacks/bas/BAS module will
be implicitly added to the application's configuration.  This, in
turn, will ensure that any validation functions defined by the BAS
module will execute.
`,

    templates    : {
        /* Contribute TI-DRIVERS libraries to linker command file */
        "/ti/utils/build/GenLibs.cmd.xdt"   :
            {modName: "/ti/drivers/Board", getLibs: getLibs},

        /* Contribute TI-DRIVERS definitions to linker command file */
        "/ti/utils/build/GenMap.cmd.xdt"   :
            {modName: "/ti/drivers/Board", getLinkerDefs: getLinkerDefs},

        /* Trigger generation of ti_drivers_config.[ch] */
        "/ti/drivers/templates/Board.c.xdt" : true,
        "/ti/drivers/templates/Board.h.xdt" : true
    },

    moduleStatic : {
        /* Ensure something supplies appropriate DriverLib library */
        modules  : modules,
        config   : config
    }
};

/* Extend the base exports to include family-specific content */
let deviceBoard = system.getScript("/ti/drivers/Board" + family);
exports = deviceBoard.extend(base);
