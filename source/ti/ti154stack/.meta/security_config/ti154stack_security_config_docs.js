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
 *  ======== ti154stack_security_config_docs.js ========
 */

"use strict";

const secureLevel = {
    description: "Configures network security level",
    readOnly: "Only SM is supported in this project",
    longDescription: `
Controls the level of network security.
\n\
__Default__: MAC Security
\n\
Value | Description
--- | ---
Disabled | No encryption enabled
MAC Security | The TI 15.4-Stack supports AES encryption as defined by the \
IEEE 802.15.4 Specification. For more information, refer to the \
[Security](/ti154stack/html/ti154stack/configuring-stack.html#security) \
section of the TI15.4-Stack User's Guide.
MAC Security & Commissioning | Secure device commissioning in addition to AES \
encryption. For more information, refer to the \
[Secure Commissioning](/ti154stack/html/ti154stack/secure-commissioning.html) \
section of the TI15.4-Stack User's Guide.
\n\
For more information about building an image with security disabled, see the \
[Configuring Stack](//ti154stack/html/ti154stack/\
configuring-stack.html#configuring-stack) section of the TI 15.4-Stack User's \
Guide.
`
};

const tfmEnabled = {
    description: "When set to true, genLibs will link to tfm libraries",
    longDescription: `
Controls the linking of tfm libraries.
\n\
__Default__: false
\n\
Value | Description
--- | ---
false | Don't use tfm libraries
true  | Use tfm libraries \
`
};

const keyTableDefaultKey = {
    description: "Configures pre-shared network key",
    longDescription: `
Configures the pre-shared key used for network communication and secure \
commissioning (if enabled). Must be 32 hexadecimal digits from least \
significant to most significant byte.
\n\
__Default__: 0x1234 0x5678 0x9abc 0xdef0 0x0000 0x0000 0x0000 0x0000
\n\
__Acceptable Values__: 32 hexadecimal digits
`
};

const smCollectorAuthMethods = {
    description: "Configures authentication methods supported by the collector",
    longDescription: `
Configures the authentication methods supported during the secure \
commissioning process.
\n\
For more information, refer to the
[Authentication](/ti154stack/html/ti154stack/\
secure-commissioning.html#authentication) section of the TI15.4-Stack User's \
Guide.
\n\
__Default__: Just Allowed, Default Code, and Passkey
\n\
Value | Description
--- | ---
Just Allow | No authentication. No man-in-the-middle (MITM) protection.
Default Code | A pre-installed authentication code that must be the same for \
all devices. Provides MITM protection as long as code is not compromised.
Passkey | Requires the user to input a 6 numerical digit passkey over UART \
during the commisisoning process. Highest level of MITM protection.
`
};

const smSensorAuthMethods = {
    description: "Configures authentication method supported by the sensor",
    longDescription: `
Configures the authentication method supported during the secure \
commissioning process.
\n\
For more information, refer to the
[Authentication](/ti154stack/html/ti154stack/\
secure-commissioning.html#authentication) section of the TI15.4-Stack User's \
Guide.
\n\
__Default__: Default Code
\n\
Value | Description
--- | ---
Just Allow | No authentication. No man-in-the-middle (MITM) protection.
Default Code | A pre-installed authentication code that must be the same for \
all devices. Provides MITM protection as long as code is not compromised.
Passkey | Requires the user to input a 6 numerical digit passkey over UART \
during the commisisoning process. Highest level of MITM protection.
`
};

const smDefaultAuthCode = {
    description: "Configures pre-defined code used for authentication",
    longDescription: `
Configures a pre-installed authentication code that must be the same on the \
collector and sensor devices. Provides MITM protection as long as code is not \
compromised.
\n\
__Default__: 0x54321
\n\
__Range__: 0x00000 to 0xFFFFF
\n\
__Acceptable Values__: Accepts inputs in decimal, and in hexadecimal provided \
that the value is prefaced by '0x'.
`
};

const smKeyrefreshTimeout = {
    description: "Configures time in secs between sensor key refreshments",
    longDescription: `
Collector-only parameter that configures the time in seconds between sensor \
key refreshments.
\n\
For more information refer to the
[Device Key Refreshment](/ti154stack/html/ti154stack/\
secure-commissioning.html#device-key-refreshment) section of the TI15.4-Stack \
User's Guide.
\n\
__Default__: 7200 (2 hours)
\n\
__Range__: 0 to UINT32_MAX
`
};

exports = {
    secureLevel: secureLevel,
    tfmEnabled: tfmEnabled,
    keyTableDefaultKey: keyTableDefaultKey,
    smSensorAuthMethods: smSensorAuthMethods,
    smCollectorAuthMethods: smCollectorAuthMethods,
    smDefaultAuthCode: smDefaultAuthCode,
    smKeyrefreshTimeout: smKeyrefreshTimeout
};
