/*
 * Copyright (c) 2019 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== ti_wisunfan_security_config_docs.js ========
 */

"use strict";

const secureLevel = {
    description: "Configures MAC security level",
    longDescription: `
Controls the level of network security in the MAC layer.
\n\
__Default__: Enabled with Key Exchange
\n\
Value | Description
--- | ---
Disabled | No encryption enabled. Note that this option is not Wi-SUN \
standard compliant.
Only MAC security with Preshared Key | AES encryption of packets using a preshared key. \
No key exchange is performed. Note that this option is not Wi-SUN \
standard compliant.
Wi-SUN compliant security with Key Exchange | AES encryption of packets with key exchange. \
Wi-SUN standard compliant.
\n\
Note that using src projects allows for code savings for security code, especially when \
using Disabled or Only MAC security options. This is due to optimization of security and \
encryption functionality when building all Wi-SUN network-level code in src projects.
\n\
`
};

const euiJoin = {
    description: "Enables Controlled Device Joining based on EUI",
    longDescription: `
Enables controlled device joining capability for network devices. Must either be enabled \
or disabled for all network devices (BR, RN). Can only be used with Preshared Key security. \
\n\
__Default__: Disabled
\n\
This capability allows BRs to allow/disallow joining of devices based on their EUI \
(Extended Unique Identifier or MAC address). This is accomplished by co-opting the \
EAPOL protocol currently used for network authentication. When using Preshared Key security \
and enabling this controlled joining feature, the authentication stage usually used for \
mbedTLS key exchange is used to exchange custom join request and response messages.

The callback function \`bool customAuthCheckAllowedJoin(uint8_t* eui)\` is called on the BR \
when the custom join request message has been received via EAPOL protocol. The function \
is used to determine whether a joining device is allowed to join. The eui parameter \
is the EUI of the joining device. The function should return true if the BR allows this \
device to join the current network. The function is weakly defined in ws_bootstrap.c to \
allow all devices, but can be overriden by users to accept/reject devices with specific \
EUIs.
\n\
`
};

const keyTableDefaultKey = {
    description: "Configures pre-shared network key",
    longDescription: `
Configures the pre-shared keys used for encryption if \
preshared key security is used. \
4 network keys are cycled through the network over time. \
Must be 32 hexadecimal digits from least significant to most significant byte.
\n\
__Default__:
Key 1: BB0608572CE14D7BA2D155499CC8519B\n
Key 2: 1849835A01684FC8ACA583F37040F74C\n
Key 3: 59EA58A4B8834938ADCB6BE388C26263\n
Key 4: E426B491BC054AF39B59F053EC128E5F\n
\n\
__Acceptable Values__: 32 hexadecimal digits
`
};

exports = {
    secureLevel: secureLevel,
    euiJoin: euiJoin,
    keyTableDefaultKey: keyTableDefaultKey
};
