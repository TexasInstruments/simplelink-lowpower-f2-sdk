/*
 * Copyright (c) 2023 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== ti_wisunfan_wfantund_config.syscfg.js ========
 */

"use strict";

// Get common utility functions
const Common = system.getScript("/ti/ti_wisunfan/ti_wisunfan_common.js");

// Get network setting descriptions
const Docs = system.getScript("/ti/ti_wisunfan/wfantund_config/"
    + "ti_wisunfan_wfantund_config_docs.js");

// Get network setting descriptions
const IPv6_regex = system.getScript("/ti/ti_wisunfan/wfantund_config/"
    + "ti_wisunfan_ipv6_string_regex.js");

/*
 * ======== onUseExternalDhcpServerChange ========
 * On change function for UseExternalDhcpServer config
 * Sets DHCP Server Address to hidden when external DHCP Server is disabled
 *
 * @param inst - 15.4 instance
 * @param ui   - user interface object
 */
function onUseExternalDhcpServerChange(inst, ui)
{
    ui["DhcpServerAddress"].hidden = !inst.UseExternalDhcpServer;
}

/*
 * ======== onUseExternalRadiusServerChange ========
 * On change function for UseExternalRadiusServer config
 * Sets RADIUS Server Address to hidden when external RADIUS Server is disabled
 *
 * @param inst - 15.4 instance
 * @param ui   - user interface object
 */
function onUseExternalRadiusServerChange(inst, ui)
{
    ui["RadiusServerAddress"].hidden = !inst.UseExternalRadiusServer;
    ui["RadiusServerSharedSecret"].hidden = !inst.UseExternalRadiusServer;
}

// Configurables for the static 15.4 network settings group
const config = {
    displayName: "Linux",
    description: "Configure Linux host settings",
    config: [
        {
            name: "UseExternalDhcpServer",
            displayName: "Use External DHCPv6 Server",
            hidden: false,
            default: false,
            description: Docs.UseExternalDhcpServer.description,
            longDescription: Docs.UseExternalDhcpServer.longDescription,
            onChange: onUseExternalDhcpServerChange
        },
        {
            name: "DhcpServerAddress",
            hidden: true,
            displayName: "Address for External DHCPv6 Server",
            default: "2020:ABCD::",
            placeholder: "2001:db8::",
            description: Docs.DhcpServerAddress.description,
            longDescription: Docs.DhcpServerAddress.longDescription,
        },
        {
            name: "UseExternalRadiusServer",
            displayName: "Use External RADIUS Server",
            hidden: false,
            default: false,
            description: Docs.UseExternalRadiusServer.description,
            longDescription: Docs.UseExternalRadiusServer.longDescription,
            onChange: onUseExternalRadiusServerChange
        },
        {
            name: "RadiusServerAddress",
            hidden: true,
            displayName: "Address for External RADIUS Server",
            default: "2020:ABCD::",
            placeholder: "2001:db8::",
            description: Docs.RadiusServerAddress.description,
            longDescription: Docs.RadiusServerAddress.longDescription,
        },
        {
            name: "RadiusServerSharedSecret", //TODO: Limit the length of this field!
            hidden: true,
            displayName: "Shared Secret for External RADIUS Server",
            default: "topsecret",
            placeholder: "test",
            description: Docs.RadiusServerSharedSecret.description,
            longDescription: Docs.RadiusServerSharedSecret.longDescription,
        },
    ]
};

/*
 * ======== validate ========
 * Validate this inst's configuration
 *
 * @param inst       - Network settings instance to be validated
 * @param validation - object to hold detected validation issues
 */
function validate(inst, validation)
{
    // Log info for Wi-SUN non-compliant settings
    if(inst.UseExternalDhcpServer == true)
    {
        validation.logInfo("This setting disables the DHCP server on \
        the Border Router and instead relays \
        all DHCP traffic to the address given below.",
        inst, "UseExternalDhcpServer");
    }

    if(!IPv6_regex.valid_ipv6_regex.exec(inst.DhcpServerAddress))
    {
        validation.logError("Invalid IPv6 Address!",
            inst, "DhcpServerAddress");
    }
    if(!IPv6_regex.valid_ipv6_regex.exec(inst.RadiusServerAddress))
    {
        validation.logError("Invalid IPv6 Address!",
            inst, "RadiusServerAddress");
    }

    if(inst.UseExternalRadiusServer == true)
    {
        let utf8Encode = new TextEncoder();
        let sharedSecretAsBytes = utf8Encode.encode(inst.RadiusServerSharedSecret)
        if (sharedSecretAsBytes.length > 32)
        {
            validation.logError("Shared Secret encodes to more than 32 bytes!",
                inst, "RadiusServerSharedSecret");
        }
    }
 

    return;
}

/*!
 * ======== setDefaultWfantundSettings ========
 * Sets the default wfantund settings 
 *
 * @param inst - module instance
 */
function setDefaultWfantundSettings(inst)
{
    inst.UseExternalDhcpServer = false;
    inst.UseExternalRadiusServer = false;
}

/*!
 * ======== setDefaultWfantundSettings ========
 * Sets the default wfantund settings hidden state
 *
 * @param inst - module instance
 */
function setWfantundSettingsHiddenState(inst, ui)
{
    if(!inst.project.includes("borderrouter"))
    {
        ui["UseExternalDhcpServer"].hidden = true;
        ui["UseExternalRadiusServer"].hidden = true;
    }
    setDefaultWfantundSettings(inst);
}
/*
 *******************************************************************************
 Module Dependencies and Exports
 *******************************************************************************
 */

// Exports to the top level 15.4 module
exports = {
    config: config,
    validate: validate,
    setDefaultWfantundSettings: setDefaultWfantundSettings,
    setWfantundSettingsHiddenState: setWfantundSettingsHiddenState,
};
