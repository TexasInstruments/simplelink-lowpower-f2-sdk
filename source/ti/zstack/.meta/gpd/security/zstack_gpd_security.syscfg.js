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
 *  ======== zstack_gpd_security.syscfg.js ========
 */

"use strict";

const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Description text for configurables */
const gpdfSecLevelLongDescription = `Specify the security level which Green \
Power Device Frames are secured at.

Three options are supported:
* No Security: No integrity or encryption verification.
* Frame Counter and Message Integrity Code: A 4 byte frame counter and 4 byte \
Message Integrity Code (MIC) are used to verify frame order and integrity.
* Encryption, Frame Counter, and Message Integrity Code: Same as above \
with added encryption of the data frame.

More information about the options can be found in the ZigBee Green Power \
Specification.

For more information, refer to the [ZigBee Configuration](/zigbee/html/\
sysconfig/zigbee.html#zigbee-configuration) section of the ZigBee User's \
Guide.

**Default:** Depends on GPD project selected`;

const gpdfSecKeyTypeLongDescription = `Specify the type of key which Green \
Power Device Frames are secured with.

 The supported options for Green Power Devices (GPDs) are as follows:

| Option                            | Comment                                 \
                                                                          |
|-------------------------------    |-----------------------------------------\
------------------------------------------------------------------------- |
| No key                            | No encryption with a security key is \
performed.                                                                   |
| Zigbee NWK key                    | The Zigbee Network key (as stored in \
the NIB Key parameter) is used for securing the communication with the GPD.  |
| GPD group key                     | Use a group key, shared between GPDs \
and GP infrastructure devices.                                               |
| NWK-key derived GPD group key     | Use a group key, shared between GPDs \
and GP infrastructure devices and derived from the Zigbee NWK Key.           |
| Out-of-the-box GPD key            | GPD is pre-configured with a security \
key.                                                                        |
| Derived individual GPD key        | An individual key is derived from the \
GPD independent group key used by a particular network.                     |

More information about the options can be found in the ZigBee Green Power \
Specification.

For more information, refer to the [ZigBee Configuration](/zigbee/html/\
sysconfig/zigbee.html#zigbee-configuration) section of the ZigBee User's \
Guide.

**Default:** Out-of-the-box GPD Key`;

const gpdfSecKeyDescription = `The ${Common.GPDF_SEC_KEY_LEN * 8} bit \
security key used to encrypt Green Power Data Frames. Only used when *Data \
Frame Security Key Type* is set to *Out-of-the-box GPD Key*.`;

const gpdfSecKeyLongDescription = gpdfSecKeyDescription + `\n\n\
**Default:** CFCECDCCCBCAC9C8C7C6C5C4C3C2C1C0

**Range:** Any ${Common.GPDF_SEC_KEY_LEN * 8} bit number (hexidecimal format)`;

/* GPD security submodule for zstack module */
const gpdSecurityModule = {
    config: [
        {
            name: "gpdfSecurityLevel",
            displayName: "Data Frame Security Level",
            description: "Specify the security level which Green Power Device "
                         + "Frames are secured at.",
            longDescription: gpdfSecLevelLongDescription,
            default: "GP_SECURITY_LVL_NO_SEC",
            options: [
                {
                    name: "GP_SECURITY_LVL_NO_SEC",
                    displayName: "No Security"
                },
                {
                    name: "GP_SECURITY_LVL_4FC_4MIC",
                    displayName: "Frame Counter and Message Integrity Code"
                },
                {
                    name: "GP_SECURITY_LVL_4FC_4MIC_ENCRYPT",
                    displayName: "Encryption, Frame Counter, and "
                                 + "Message Integrity Code"
                }
            ]
        },
        {
            name: "gpdfSecurityKeyType",
            displayName: "Data Frame Security Key Type",
            description: "Specify the type of key which Green Power Device "
                         + "Frames are secured with.",
            longDescription: gpdfSecKeyTypeLongDescription,
            default: "KEY_TYPE_OUT_BOX_GPD_KEY",
            options: [
                {
                    name: "KEY_TYPE_NO_KEY",
                    displayName: "No Key"
                },
                {
                    name: "KEY_TYPE_ZIGBEE_NWK_KEY",
                    displayName: "Zigbee NWK Key"
                },
                {
                    name: "KEY_TYPE_GPD_GROUP_KEY",
                    displayName: "GPD Group Key"
                },
                {
                    name: "KEY_TYPE_NWK_KEY_DERIVED_GPD_GROUP_KEY",
                    displayName: "NWK-key Derived GPD Group Key"
                },
                {
                    name: "KEY_TYPE_OUT_BOX_GPD_KEY",
                    displayName: "Out-of-the-box GPD Key"
                },
                {
                    name: "KEY_TYPE_DERIVED_INDIVIDUAL_GPD_KEY",
                    displayName: "Derived Individual GPD Key"
                }
            ],
            onChange: gpdfSecurityKeyTypeChange
        },
        {
            name: "gpdfSecurityKey",
            displayName: "Data Frame Security Key",
            description: gpdfSecKeyDescription,
            longDescription: gpdfSecKeyLongDescription,
            default: "CFCECDCCCBCAC9C8C7C6C5C4C3C2C1C0"
        }
    ],
    validate: validate
};

/* Function to handle changes in gpdfSecurityKeyType configurable */
function gpdfSecurityKeyTypeChange(inst, ui)
{
    if(inst.gpdfSecurityKeyType !== "KEY_TYPE_OUT_BOX_GPD_KEY")
    {
        ui.gpdfSecurityKey.hidden = true;
    }
    else
    {
        ui.gpdfSecurityKey.hidden = false;
    }
}

/* Validation function for the GPD security submodule */
function validate(inst, validation)
{
    /* Validate GPDF Security Key */
    const gpdfSecKeyReg = new RegExp(
        "^[0-9A-Fa-f]{" + Common.GPDF_SEC_KEY_LEN * 2 + "}$", "g"
    );
    if(gpdfSecKeyReg.test(inst.gpdfSecurityKey) === false)
    {
        validation.logError(
            "GPDF Security Key must be a valid hexidecimal number of "
            + "length " + Common.GPDF_SEC_KEY_LEN * 8 + " bits", inst,
            "gpdfSecurityKey"
        );
    }
}

exports = gpdSecurityModule;
