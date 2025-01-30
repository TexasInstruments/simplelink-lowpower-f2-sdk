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
 *  ======== ti_wisunfan_wfantund_config_docs.js ========
 */

"use strict";

// ======== Top Level ========

const UseExternalDhcpServer = {
    description: "Relay DHCP traffic to the Host PC",
    longDescription: `
Enabling this setting will relay DHCP traffic destined to the Border Router to another address.
`
};

const DhcpServerAddress = {
    description: "IPv6 Address to relay DHCP traffic towards",
    longDescription: `
This address should be set to the IPv6 address of the wfantund interface that will connect to this Border Router.
`
};

const UseExternalRadiusServer = {
    description: "General RADIUS messages and send them to the Host PC",
    longDescription: `
Enabling this setting will allow RADIUS messages to be created and sends them up to the Host PC.
`
};

const RadiusServerAddress = {
    description: "IPv6 Address to send RADIUS messages to",
    longDescription: `
This address should be set to the IPv6 address of the RADIUS Server that will handle certificate authentication.
`
};

const RadiusServerSharedSecret = {
    description: "Shared secret to use for the RADIUS server.",
    longDescription: `
This is the secret shared between the border router and the authentication server. The text entered here will be encoded to UTF-8 and is limited to 32 total bytes.
`
};

exports = {
    UseExternalDhcpServer: UseExternalDhcpServer,
    DhcpServerAddress: DhcpServerAddress,
    UseExternalRadiusServer: UseExternalRadiusServer,
    RadiusServerAddress: RadiusServerAddress,
    RadiusServerSharedSecret: RadiusServerSharedSecret,
};
