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
 *  ======== zstack_network.syscfg.js ========
 */

"use strict";

const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Description text for configurables */
const panIDLongDescription = `The ${Common.PAN_ID_LEN * 8} bit PAN ID.

Configuration of the PAN ID has different effects depending on whether the \
device is forming a network or joining a network. During network formation, \
the device will form a network with the configured PAN ID. During joining, \
the device will attempt to join a network with the configured PAN ID.

Note that the broadcast PAN ID of 0xFFFF has special effects on network \
formation and joining. During network formation, the device will choose \
a random PAN ID for the created network. For joining, the device will attempt \
to join a network with any valid PAN ID.

For more information, refer to the [ZigBee Configuration](/zigbee/html/\
sysconfig/zigbee.html#zigbee-configuration) section of the ZigBee User's \
Guide.

**Default:** 0xFFFF

**Range:** Any ${Common.PAN_ID_LEN * 8} bit number (hexidecimal format)`;

const epidAsExtAddrDescription = `Specify whether to set the Extended PAN ID \
to the Extended Address, or specify a custom Extended PAN ID.`;

const epidAsExtAddrLongDescription = epidAsExtAddrDescription + `\n\n
**Default:** Set the Extended PAN ID to the Extended Address (checked)`;

const epidDescription = `The ${Common.EPID_LEN * 8} bit Extended PAN ID.`;

const epidLongDescription = epidDescription + `\n\n
**Default:** 0x0123456789ABCDEF

**Range:** Any ${Common.EPID_LEN * 8} bit number (hexidecimal format)`;

const defaultNwkKeyDescription = `Default network key. Set to 0 to generate \
a random key.`;

const defaultNwkKeyLongDescription = defaultNwkKeyDescription + `\n\n\
**Default:** 0x0

**Range:** Any ${Common.NWK_KEY_LEN * 8} bit number (hexidecimal format)`;

const distributedGlobalLinkKeyDescription = `ZigBee Alliance Pre-configured \
Distributed Link Key (for Distributed networks)`;

const distributedGlobalLinkKeyLongDescription = ""
+ distributedGlobalLinkKeyDescription
+ `\n\n **Default:** D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF

**Range:** Any ${Common.NWK_KEY_LEN * 8} bit number (hexidecimal format)`;

const tcLinkKeyDescription = `ZigBee Alliance Pre-configured TC Link Key - \
'ZigBeeAlliance09' (for Centralized networks)`;

const tcLinkKeyLongDescription = tcLinkKeyDescription
+ `\n\n **Default:** 5a6967426565416c6c69616e63653039

**Range:** Any ${Common.NWK_KEY_LEN * 8} bit number`;

const nwkMaxDeviceListDescription = `Number of nodes that the local device can \
support in its association table`;

const nwkMaxDeviceListLongDescription = nwkMaxDeviceListDescription
+ `\n\n **Default:** 20

**Range:** 0 - 65535`;

const zdsecmgrTcDeviceMaxDescription = `Defines the number of unique TC Link \
Keys (ZDSECMGR_TC_DEVICE_MAX) that the network supports. Each device that joins \
requires a unique key, so this limits the total number of devices in the network.`;

const zdsecmgrTcDeviceMaxLongDescription = zdsecmgrTcDeviceMaxDescription
+ `\n\n **Default:** 40

**Range:** 0 - 65535`;

const endDeviceTimeoutDescription = `Value of End Device Timeout`;

const endDeviceTimeoutLongDescription = endDeviceTimeoutDescription
+ `\n\n
If ZED, this is the value used when sending End Device Timeout Request.

If ZC/ZR, this is the default value if any End Device that does not
 negotiate a different timeout value

**Default:** 8

**Range:** 0 - 14`;

/* End device timeout values */
const timeoutValues = [
    {name: 0, displayName: "0 - 10 seconds"},
    {name: 1, displayName: "1 - 2 minutes"},
    {name: 2, displayName: "2 - 4 minutes"},
    {name: 3, displayName: "3 - 8 minutes"},
    {name: 4, displayName: "4 - 16 minutes"},
    {name: 5, displayName: "5 - 32 minutes"},
    {name: 6, displayName: "6 - 64 minutes"},
    {name: 7, displayName: "7 - 128 minutes"},
    {name: 8, displayName: "8 - 256 minutes"},
    {name: 9, displayName: "9 - 512 minutes"},
    {name: 10, displayName: "10 - 1024 minutes"},
    {name: 11, displayName: "11 - 2048 minutes"},
    {name: 12, displayName: "12 - 4096 minutes"},
    {name: 13, displayName: "13 - 8192 minutes"},
    {name: 14, displayName: "14 - 16384 minutes"}
];

/* Network submodule for the zstack module */
const networkModule = {
    config: [
        {
            name: "deviceType",
            displayName: "Device Type",
            description: "Hidden configurable for passing in device type",
            default: "",
            hidden: true,
            onChange: onDeviceTypeChange
        },
        {
            name: "panID",
            displayName: "PAN ID",
            description: "The " + Common.PAN_ID_LEN * 8 + " bit PAN ID.",
            longDescription: panIDLongDescription,
            default: "0xFFFF"
        },
        {
            name: "epidAsExtAddr",
            displayName: "Extended PAN ID to Extended Address",
            description: epidAsExtAddrDescription,
            longDescription: epidAsExtAddrLongDescription,
            default: true,
            onChange: onEpidAsExtAddrChange
        },
        {
            name: "epid",
            displayName: "Extended PAN ID",
            description: epidDescription,
            longDescription: epidLongDescription,
            default: "0123456789ABCDEF",
            hidden: true
        },
        {
            name: "defaultNwkKey",
            displayName: "Default Network Key",
            description: defaultNwkKeyDescription,
            longDescription: defaultNwkKeyLongDescription,
            default: "00000000000000000000000000000000"
        },
        {
            name: "distributedGlobalLinkKey",
            displayName: "Distributed Global Link Key",
            description: distributedGlobalLinkKeyDescription,
            longDescription: distributedGlobalLinkKeyLongDescription,
            default: "D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF",
            hidden: true
        },
        {
            name: "tcLinkKey",
            displayName: "TC Link Key",
            description: tcLinkKeyDescription,
            longDescription: tcLinkKeyLongDescription,
            default: "5a6967426565416c6c69616e63653039"
        },
        {
            name: "nwkMaxDeviceList",
            displayName: "Network Maximum Device List",
            description: nwkMaxDeviceListDescription,
            longDescription: nwkMaxDeviceListLongDescription,
            default: 20,
            hidden: true
        },
        {
            name: "zdsecmgrTcDeviceMax",
            displayName: "Maximum Unique TC Link Keys",
            description: zdsecmgrTcDeviceMaxDescription,
            longDescription: zdsecmgrTcDeviceMaxLongDescription,
            default: 40,
            hidden: true
        },
        {
            name: "endDeviceTimeout",
            displayName: "End Device Timeout",
            description: endDeviceTimeoutDescription,
            longDescription: endDeviceTimeoutLongDescription,
            default: 8,
            options: timeoutValues
        }
    ],
    validate: validate
};

/* Function to handle changes in deviceType configurable */
function onDeviceTypeChange(inst, ui)
{
    if(inst.deviceType === "zc" || inst.deviceType === "zr"
       || inst.deviceType === "znp")
    {
        ui.nwkMaxDeviceList.hidden = false;
        if(inst.deviceType === "zc" || inst.deviceType === "znp")
        {
            ui.zdsecmgrTcDeviceMax.hidden = false;
            inst.zdsecmgrTcDeviceMax = 40;
        }
        else /* zr */
        {
            ui.zdsecmgrTcDeviceMax.hidden = true;
            inst.zdsecmgrTcDeviceMax = 3;
        }
        if(inst.deviceType === "zc")
        {
            ui.distributedGlobalLinkKey.hidden = true;
        }
        else /* znp or zr */
        {
            ui.distributedGlobalLinkKey.hidden = false;
        }
    }
    else /* zed */
    {
        ui.distributedGlobalLinkKey.hidden = false;
        ui.nwkMaxDeviceList.hidden = true;
        ui.zdsecmgrTcDeviceMax.hidden = true;
        inst.zdsecmgrTcDeviceMax = 3;
    }
}

/* Function to handle changes in epidAsExtAddr configurable */
function onEpidAsExtAddrChange(inst, ui)
{
    ui.epid.hidden = inst.epidAsExtAddr;
}

/* Validation function for the network submodule */
function validate(inst, validation)
{
    /* Validate PAN ID */
    const panIDReg = new RegExp(
        "^0x[0-9A-Fa-f]{" + Common.PAN_ID_LEN * 2 + "}$", "g"
    );
    if(panIDReg.test(inst.panID) === false)
    {
        validation.logError(
            "PAN ID must be a valid hexidecimal number (0x...) of length "
            + Common.PAN_ID_LEN * 8 + " bits", inst, "panID"
        );
    }

    /* Info for broadcast PAN ID (0xFFFF) */
    const panIDReservedReg = new RegExp(
        "^0x[Ff]{" + Common.PAN_ID_LEN * 2 + "}$", "g"
    );
    if(panIDReservedReg.test(inst.panID) === true)
    {
        validation.logInfo(
            "Setting PAN ID to 0xFFFF will cause PAN ID to be "
            + "randomly generated.", inst, "panID"
        );
    }

    /* Validate EPID */
    const epidReg = new RegExp(
        "^[0-9A-Fa-f]{" + Common.EPID_LEN * 2 + "}$", "g"
    );
    if(epidReg.test(inst.epid) === false)
    {
        validation.logError(
            "Extended PAN ID must be a valid hexidecimal number (...) of "
            + "length " + Common.EPID_LEN * 8 + " bits", inst, "epid"
        );
    }

    /* Error for reserved EPIDs (0x0 and 0xFFFFFFFFFFFFFFFF) */
    const epidReservedReg1 = new RegExp(
        "^[0]{" + Common.EPID_LEN * 2 + "}$", "g"
    );
    const epidReservedReg2 = new RegExp(
        "^[Ff]{" + Common.EPID_LEN * 2 + "}$", "g"
    );
    if(epidReservedReg1.test(inst.epid) === true
       || epidReservedReg2.test(inst.epid) === true)
    {
        validation.logError(
            "Extended PAN ID values of 0000000000000000 and "
            + "FFFFFFFFFFFFFFFF are reserved", inst, "epid"
        );
    }

    /* Validate Default Network Key */
    const defaultNwkKeyReg = new RegExp(
        "^[0-9A-Fa-f]{" + Common.NWK_KEY_LEN * 2 + "}$", "g"
    );
    if(defaultNwkKeyReg.test(inst.defaultNwkKey) === false)
    {
        validation.logError(
            "Default Network Key must be a valid hexidecimal number (...) "
            + "of length " + Common.NWK_KEY_LEN * 8 + " bits",
            inst, "defaultNwkKey"
        );
    }

    /* Info for random Network Key (0x0) */
    const defaultNwkKeyRandomReg = new RegExp(
        "^[0]{" + Common.NWK_KEY_LEN * 2 + "}$", "g"
    );
    if(defaultNwkKeyRandomReg.test(inst.defaultNwkKey) === true)
    {
        validation.logInfo(
            "Setting Default Network Key to 0x0 will cause the network key "
            + "to be randomly generated.", inst, "defaultNwkKey"
        );
    }

    /* Validate Distributed Global Link Key */
    const distGlobalLinkKeyReg = new RegExp(
        "^[0-9A-Fa-f]{" + Common.NWK_KEY_LEN * 2 + "}$", "g"
    );
    if(distGlobalLinkKeyReg.test(inst.distributedGlobalLinkKey) === false)
    {
        validation.logError(
            "Distributed Global Link Key must be a valid hexidecimal number"
            + " (...) of length " + Common.NWK_KEY_LEN * 8 + " bits",
            inst, "distributedGlobalLinkKey"
        );
    }

    /* Validate TC Link Key */
    const tcLinkKeyReg = new RegExp(
        "^[0-9A-Fa-f]{" + Common.NWK_KEY_LEN * 2 + "}$", "g"
    );
    if(tcLinkKeyReg.test(inst.tcLinkKey) === false)
    {
        validation.logError(
            "TC Link Key must be a valid hexidecimal number "
            + "of length " + Common.NWK_KEY_LEN * 8 + " bits",
            inst, "tcLinkKey"
        );
    }

    /* Warn when TC Link Key is changed */
    if(inst.tcLinkKey !== "5a6967426565416c6c69616e63653039")
    {
        validation.logWarning(
            "If the TC Link Key is changed from the default value,"
            + " it might break interoperability between Zigbee Devices,"
            + " and might not be certifiable.", inst, "tcLinkKey"
        );
    }

    /* Warn when assoc table size is changed */
    if(inst.nwkMaxDeviceList > 50)
    {
        validation.logWarning(
            "Consider increasing NV size for larger device lists",
            inst, "nwkMaxDeviceList"
        );
    }

    /* Warn when TC Link Key list size is changed */
    if(inst.zdsecmgrTcDeviceMax > 50)
    {
        validation.logWarning(
            "Consider increasing NV size for larger device lists",
            inst, "zdsecmgrTcDeviceMax"
        );
    }

    /* Validate Network Max Device List */
    Common.validateRange(inst, validation, inst.nwkMaxDeviceList,
        "nwkMaxDeviceList", "Network Maximum Device List", 1, 65535);

    /* Validate ZDSECMGR TC Device Max */
    Common.validateRange(inst, validation, inst.zdsecmgrTcDeviceMax,
        "zdsecmgrTcDeviceMax", "Maximum Unique TC Link Keys", 1, 65535);
}

exports = networkModule;
