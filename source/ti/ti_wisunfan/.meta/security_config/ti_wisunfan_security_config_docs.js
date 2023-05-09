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
__Default__: Enabled
\n\
Value | Description
--- | ---
Disabled | No encryption enabled
MAC Security | The TI Wi-SUN FAN Stack supports AES encryption for the MAC layer
\n\
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

exports = {
    secureLevel: secureLevel,
    keyTableDefaultKey: keyTableDefaultKey
};
