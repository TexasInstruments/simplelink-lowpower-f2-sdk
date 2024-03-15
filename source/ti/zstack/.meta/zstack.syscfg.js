/*
 * Copyright (c) 2018, Texas Instruments Incorporated
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
 *  ======== zstack.syscfg.js ========
 */

"use strict";

const genLibs = system.getScript("/ti/utils/build/GenLibs.syscfg.js");
// Get common 154 utility functions
const Common154 = system.getScript("/ti/ti154stack/ti154stack_common.js");

/* Description text for module and configurables */
const deviceTypeLongDescription = `The logical device type for the ZigBee \
node.

This can be a ZigBee Coordinator (ZC), ZigBee Router (ZR), ZigBee End Device \
(ZED), ZigBee Network Processor (ZNP), or a Green Power Device (GPD). For \
more information on ZigBee device types, refer to the \
[Logical Device Types](/zigbee/html/zigbee/z-stack-overview.html\
#logical-device-types) section of the ZigBee User's Guide.

Note that a GPD is a special type of ZigBee device distinct from the ZC, \
ZR, and ZED. For more information on GPDs, refer to the \
[Green Power Device](/zigbee/html/zigbee/gpd_application_overview.html) \
section of the ZigBee User's Guide.

Also note the ZNP is not a specific logical device type, but is a special \
co-processor interface that supports all device types. ZNPs select a role at \
runtime depending on ZNP NVS settings. For more information, refer to the \
[ZigBee Network Processor](/zigbee/html/zigbee/znp_interface.html) section of \
the ZigBee User's Guide.

Currently, Z-Stack SysConfig does not support the configuration of device \
type. Due to the differing file dependencies of each device type, all Z-Stack \
examples have a fixed device type. To change device type, import the project \
associated with that device type, indicated by the project name.

For more information, refer to the [ZigBee Configuration](/zigbee/html/\
sysconfig/zigbee.html#zigbee-configuration) section of the ZigBee User's \
Guide.`;

const modulelongDescription = `[__Z-Stack__][1] is the TI implementation of \
the ZigBee network stack, building the ZigBee 3.0 Specification on top of the \
IEEE 802.15.4 Specification with the added benefit of running TI-RTOS.

* [Usage Synopsis][2]
* [Examples][3]

*Important Note*

Some Z-Stack settings are stored in non-volatile storage, and Z-Stack \
prioritizes stored settings over SysConfig settings. To guarantee \
SysConfig settings are applied, perform a [factory reset][4] of the device to \
clear non-volatile storage.

[1]: /zigbee/zstack-apis.html
[2]: /zigbee/zigbee-users-guide.html
[3]: /zigbee/html/zigbee/application_overview.html
[4]: /zigbee/html/zigbee/application_overview.html#reset-to-fn-screen
`;


const genLibDescription = "Configures genLibs usage for local libraries. Always hidden";
const genLibLongDescription = `This is a hidden parameter that controls whether the \
stack module contributes libraries to the generated linker command file.
\n\__Default__: True (checked)`;

/* Static module definition for zstack module */
const moduleStatic = {
    config: [
        /* Device Type Configurable */
        {
            name: "deviceType",
            displayName: "Device Type",
            description: "The logical device type for the ZigBee node.",
            longDescription: deviceTypeLongDescription,
            default: "zc",
            options: [
                {name: "zc", displayName: "ZigBee Coordinator"},
                {name: "zr", displayName: "ZigBee Router"},
                {name: "zed", displayName: "ZigBee End Device"},
                {name: "gpd", displayName: "Green Power Device"},
                {name: "znp", displayName: "ZigBee Network Processor"}
            ]
        },
        {
            name: "genLibs",
            displayName: "Generate Z-Stack Libraries",
            default: true,
            hidden: true,
            description: genLibDescription,
            longDescription: genLibLongDescription
        },
        {
            name: "deviceTypeReadOnly",
            default: false,
            hidden: true,
            onChange: onDeviceTypeReadOnlyChange
        },
        {
            name: "appBuilder",
            default: false,
            hidden: true,
        }
    ],
    moduleInstances: moduleInstances,
    modules: modules
};

/*
 * ======== getLibs ========
 * Contribute libraries to linker command file
 *
 * @param inst  - zstack module instance
 * @returns     - Object containing the name of component, array of dependent
 *                components, and array of library names
 */
function getLibs(inst)
{
    // Create a GenLibs input argument
    let results = {
        name: "/ti/zstack",
        deps: [],
        libs: []
    };

    if(inst.$static.genLibs)
    {
        const toolchain = genLibs.getToolchainDir();
        const isa = genLibs.getDeviceIsa();
        const rtos = system.getRTOS();

        // Add mac lib
        const deviceId = system.deviceData.deviceId;
        var dev = "undefined";
        if(deviceId.includes("CC1352"))
        {
            dev = "cc13x2";
        }
        else if(deviceId.includes("CC2652"))
        {
            dev = "cc26x2";
        }
        else if(deviceId.includes("CC2651"))
        {
            dev = "cc26x1";
        }
        else if(deviceId.includes("CC2674"))
        {
            dev = "cc26x4";
        }
		else if(deviceId.includes("CC2653"))
        {
            dev = "cc26x4";
        }
		else if(deviceId.includes("CC1354"))
        {
            dev = "cc13x4";
        }
		else if(deviceId.includes("CC2654"))
        {
            dev = "cc26x4";
        }
        let maclib = "ti/ti154stack/lib/"
        maclib += toolchain + "/" + isa + "/";
        if (rtos === "tirtos7")
        {
            maclib += "maclib_nosecure_" + dev + "_2_4g_tirtos7.a";
        }
        else
        {
            maclib += "maclib_nosecure_" + dev + "_2_4g.a";
        }
        results.libs.push(maclib);

        // Add zstack lib
        let zlib = "ti/zstack/lib/";
        let devType = inst.$static.deviceType;
        switch(devType) {
            case "zc":
                devType = "nwk_zr";
                break;
            case "znp":
                devType = "nwk_all";
                break;
            case "gpd":
                devType = "gpd_sec";
                break;
            default:
                devType = "nwk_" + devType; // nwk_<zr/zed>_
                break;
        }
        zlib += toolchain + "/" + isa + "/";
        zlib += "libZStack_" + devType + ".a";
        results.libs.push(zlib);
    }

    return(results);
}

/* Submodule instance definitions */
function moduleInstances(inst)
{
    const submodules = [];

    if(inst.deviceType === "gpd")
    {
        submodules.push({
            name: "gpdRf",
            displayName: "Radio",
            description: "Configure default radio channel",
            moduleName: "/ti/zstack/gpd/rf/zstack_gpd_rf",
            collapsed: true
        });
        submodules.push({
            name: "gpdNetwork",
            displayName: "Network",
            description: "Configure Green Power network settings",
            moduleName: "/ti/zstack/gpd/network/zstack_gpd_network",
            collapsed: true
        });
        submodules.push({
            name: "gpdSecurity",
            displayName: "Security",
            description: "Configure Green Power security settings",
            moduleName: "/ti/zstack/gpd/security/zstack_gpd_security",
            collapsed: true
        });
        submodules.push({
            name: "gpdAdvanced",
            displayName: "Advanced",
            description: "Configure advanced Green Power settings",
            moduleName: "/ti/zstack/gpd/advanced/zstack_gpd_advanced",
            collapsed: true
        });
    }
    else
    {
        if( inst.appBuilder === true )
        {
          submodules.push({
              name: "app",
              displayName: "Zigbee Application Endpoint",
              description: "Configure Zigbee Cluster Library (ZCL) settings",
              moduleName: "/ti/zstack/app/zstack_app",
              collapsed: true
          });
          submodules.push({
              name: "otaClient",
              displayName: "Over-The-Air Upgrade Client Endpoint",
              description: "Configure OTA Client settings",
              moduleName: "/ti/zstack/app/zstack_ota_client",
              collapsed: true
          });
        }
        submodules.push({
            name: "touchlink",
            displayName: "Touchlink Commissioning Endpoint",
            description: "Enable/Disable Touchlink Commissioning",
            moduleName: "/ti/zstack/touchlink/zstack_touchlink",
            collapsed: true,
            args: {deviceType: inst.deviceType}
        });
        submodules.push({
            name: "pm",
            displayName: "Power Management",
            description: "Configure radio power settings",
            moduleName: "/ti/zstack/pm/zstack_pm",
            collapsed: true,
            args: {deviceType: inst.deviceType}
        });
        submodules.push({
            name: "rf",
            displayName: "Radio",
            description: "Configure radio channel and retransmission settings",
            moduleName: "/ti/zstack/rf/zstack_rf",
            collapsed: true
        });
        submodules.push({
            name: "network",
            displayName: "Network",
            description: "Configure network identification and security "
                         + "settings",
            moduleName: "/ti/zstack/network/zstack_network",
            collapsed: true,
            args: {deviceType: inst.deviceType}
        });
        submodules.push({
            name: "advanced",
            displayName: "Advanced",
            description: "Configure advanced Z-Stack settings",
            moduleName: "/ti/zstack/advanced/zstack_advanced",
            collapsed: true,
            args: {deviceType: inst.deviceType}
        });
    }

    return submodules;
}

/* Static submodule instance definitions */
function modules(inst)
{
    const submodules = [];

    submodules.push({
        name: "multiStack",
        displayName: "Multi-Stack Validation",
        moduleName: "/ti/common/multi_stack_validate",
        hidden: true
    });
    submodules.push({
        name: "rfDriver",
        displayName: "RF Driver",
        moduleName: "/ti/drivers/RF",
        hidden: true
    });

    submodules.push({
        name: "rfDesign",
        displayName: "RF Design",
        moduleName: "/ti/devices/radioconfig/rfdesign"
    });

    return submodules;
}

/* Makes the device type read only when deviceTypeReadOnly is true */
function onDeviceTypeReadOnlyChange(inst, ui)
{
    const readOnlyReason = "Device type cannot be configured via SysConfig "
        + "due to differing dependencies. Change device type by importing "
        + "the appropriate project.";

    if(inst.deviceTypeReadOnly === true)
    {
        ui.deviceType.readOnly = readOnlyReason;
    }
    else
    {
        ui.deviceType.readOnly = false;
    }
}

/* Zigbee module definition */
const zstackModule = {
    displayName: "Z-Stack",
    description: "Z-Stack configuration module",
    longDescription: modulelongDescription,
    moduleStatic: moduleStatic,
    templates: {
        "/ti/utils/build/GenLibs.cmd.xdt":
        {
            modName: "/ti/zstack/zstack",
            getLibs: getLibs
        }
    }
};

exports = zstackModule;
