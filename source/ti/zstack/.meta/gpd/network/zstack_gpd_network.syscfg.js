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
 *  ======== zstack_gpd_network.syscfg.js ========
 */

"use strict";

const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Description text for configurables */
const gpdTypeLongDescription = `Specify the functional type of the device via \
an 8 bit ID.

This configurable identifies the functionality of a Green Power Device \
(e.g. GP On/Off Switch, GP Temperature Sensor). This is specified via the \
8 bit Device ID (different from the GPD ID). Refer to the Green Power feature \
specification for details on how Device ID corresponds to device type.

Currently, this value cannot be configured when using the GP example \
projects, as each example only supports one Green Power Device Type. \
To change Green Power Device Type, import the project associated with that \
device type, indicated by the project name.

For more information, refer to the [ZigBee Configuration](/zigbee/html/\
sysconfig/zigbee.html#zigbee-configuration) section of the ZigBee User's \
Guide.

**Default:** Depends on GPD project selected

**Range:** Any 8 bit number (hexidecimal format)`;

const gpdIDTypeLongDescription = `Specify whether the Green Power Device ID \
is a 32 bit configurable value or is the 64 bit IEEE address.

Choosing *Configurable GPD ID* will allow the Green Power Device ID to be \
configured in the *Green Power Device ID* field. Choosing \
*Use IEEE address as GPD ID* will automatically set the Green Power Device \
ID as the IEEE address of the Green Power Device.

For more information, refer to the [ZigBee Configuration](/zigbee/html/\
sysconfig/zigbee.html#zigbee-configuration) section of the ZigBee User's \
Guide.

**Default:** Configurable GPD ID`;

const gpdIDDescription = `Specifies the ${Common.GPD_ID_LEN * 8} bit Green \
Power Device ID of the device.`;

const gpdIDLongDescription = gpdIDDescription + `\n\n\
**Default:** 0x11223344

**Range:** Any ${Common.GPD_ID_LEN * 8} bit number (hexidecimal format)`;


/* GPD network submodule for zstack module */
const gpdNetworkModule = {
    config: [
        {
            name: "gpdType",
            displayName: "Green Power Device Type",
            description: "Specify the functional type of the device via "
                         + "an " + Common.GPD_TYPE_LEN * 8 + " bit ID.",
            longDescription: gpdTypeLongDescription,
            default: "0x00"
        },
        {
            name: "gpdTypeReadOnly",
            default: false,
            hidden: true,
            onChange: onGpdTypeReadOnlyChange
        },
        {
            name: "gpdIDType",
            displayName: "Green Power Device ID Type",
            description: "Specify whether the Green power Device ID is a 32 "
                         + "bit configurable value or the 64 bit IEEE address.",
            longDescription: gpdIDTypeLongDescription,
            default: "GPD_APP_TYPE_SRC_ID",
            options: [
                {
                    name: "GPD_APP_TYPE_SRC_ID",
                    displayName: "Configurable GPD ID"
                },
                {
                    name: "GPD_APP_TYPE_IEEE_ID",
                    displayName: "Use IEEE Address as GPD ID"
                }
            ],
            onChange: onGpdIDTypeChange
        },
        {
            name: "gpdID",
            displayName: "Green Power Device ID",
            description: gpdIDDescription,
            longDescription: gpdIDLongDescription,
            default: "0x11223344"
        }
    ],
    templates: {
        "/ti/zstack/templates/gpd/ti_zstack_config.h.xdt":
                            "/ti/zstack/templates/gpd/ti_zstack_config.h.xdt"
    },
    validate: validate
};

/* Makes the gpdType configurable read only when gpdTypeReadOnly is true */
function onGpdTypeReadOnlyChange(inst, ui)
{
    const readOnlyReason = "Each Green Power example only supports one Green "
        + "Power Device Type. Change Green Power Device Type by importing "
        + "the appropriate project.";

    if(inst.gpdTypeReadOnly === true)
    {
        ui.gpdType.readOnly = readOnlyReason;
    }
    else
    {
        ui.gpdType.readOnly = false;
    }
}

/* Function to handle changes in gpdIDType configurable */
function onGpdIDTypeChange(inst, ui)
{
    if(inst.gpdIDType === "GPD_APP_TYPE_SRC_ID")
    {
        ui.gpdID.hidden = false;
    }
    else
    {
        ui.gpdID.hidden = true;
    }
}

/* Validation function for the GPD network submodule */
function validate(inst, validation)
{
    /* Validate GPD Type */
    const gpdTypeReg = new RegExp(
        "^0x[0-9A-Fa-f]{" + Common.GPD_TYPE_LEN * 2 + "}$", "g"
    );
    if(gpdTypeReg.test(inst.gpdType) === false)
    {
        validation.logError(
            "GPD Type must be a valid hexidecimal number (0x...) of length "
            + Common.GPD_TYPE_LEN * 8 + " bits", inst, "gpdType"
        );
    }

    /* Info for two supported GPD Type values */
    if(inst.gpdType === "0x02")
    {
        validation.logInfo(
            "GPD Type of 0x02 indicates that the device is a GPD On/Off switch",
            inst, "gpdType"
        );
    }

    if(inst.gpdType === "0x30")
    {
        validation.logInfo(
            "GPD Type of 0x30 indicates that the device is a GPD Temperature "
            + "Sensor", inst, "gpdType"
        );
    }

    /* Validate GPD ID */
    const gpdIDReg = new RegExp(
        "^0x[0-9A-Fa-f]{" + Common.GPD_ID_LEN * 2 + "}$", "g"
    );
    if(gpdIDReg.test(inst.gpdID) === false)
    {
        validation.logError(
            "GPD ID must be a valid hexidecimal number (0x...) of length "
            + Common.GPD_ID_LEN * 8 + " bits", inst, "gpdID"
        );
    }
}

exports = gpdNetworkModule;
