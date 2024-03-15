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
 *  ======== zstack_touchlink.syscfg.js ========
 */

"use strict";

const Common = system.getScript("/ti/zstack/zstack_common.js");

const touchlinkEnabledLongDescription =  `\n\n\
  Touchlink is only available on ZED and ZR type devices. For more information about \
  Touchlink Commissioning, please refer to the SDK documentation. `;

/* Power management submodule for zstack module */
const touchlinkModule = {
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
            name: "touchlinkEnabled",
            displayName: "Touchlink Enabled",
            description: "Enable Touchlink Functionality",
            longDescription: touchlinkEnabledLongDescription,
            default: false,
            readOnly: true,
            onChange: onTouchlinkEnabledChange
        },
        {
            name: "touchlinkMode",
            displayName: "Touchlink Commissioning Mode",
            description: "Choose whether to be a Touchlink Initiator or Target",
            default: "initiator",
            hidden: true,
            options: [
                {
                    name: "initiator",
                    displayName: "Touchlink Initiator"
                },
                {
                    name: "target",
                    displayName: "Touchlink Target"
                }
            ]
        }
    ],
    validate: validate,
    templates: {
        "/ti/zstack/templates/ti_zstack_config.h.xdt":
                            "/ti/zstack/templates/ti_zstack_config.h.xdt"
    }
};

/* Function to handle changes in onTouchlinkEnabledChange configurable */
function onTouchlinkEnabledChange(inst, ui)
{
    ui.touchlinkMode.hidden = !(ui.touchlinkMode.hidden);
}

/* Function to handle changes in deviceType configurable */
function onDeviceTypeChange(inst, ui)
{
    // touchlink is only supported on ZR and ZED devices
    if(inst.deviceType === "zr" || inst.deviceType === "zed")
    {
        ui.touchlinkEnabled.readOnly = false;
        ui.touchlinkEnabled.hidden = false;
    }
    else
    {
        ui.touchlinkEnabled.readOnly = true;
        ui.touchlinkEnabled.hidden = true;
    }
}

/* Validation function for the power management submodule */
function validate(inst, validation)
{

}

exports = touchlinkModule;
