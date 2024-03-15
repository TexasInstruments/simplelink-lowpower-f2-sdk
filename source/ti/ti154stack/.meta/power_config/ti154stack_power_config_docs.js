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
 *  ======== ti154stack_power_config_docs.js ========
 */

"use strict";

const transmitPower = {
    description: "Configures transmit power in dBm",
    longDescription: `
Configures the transmit power in dBm. When the nodes in the network are close \
to each other, lowering this value helps reduce saturation.
\n\
For more information regarding transmit power levels supported by each board,\
 please refer to the \
[TX Power](/proprietary-rf/proprietary-rf-users-guide/proprietary-rf/\
tx-power.html) section of the RF User's Guide and the \
[CC1352P1 and CC1352P2 Support](/ti154stack/html/ti154stack/\
configuring-stack.html#cc1352p1-and-cc1352p2-support) section of the \
TI15.4-Stack User's Guide.
\n\
__Default__: 0
\n\
__Range__: Supported TX power levels vary by board and band
`
};

const rxOnIdle = {
    description: "Selects whether a device is sleepy or not",
    longDescription: `
Selects whether a device operates as a sleepy or nonsleepy node, and \
thus has RX on while idle: false for sleepy, and true for nonsleepy. \
Note that beacon mode will increase current due to beacon events if \
the device is sleepy.
\n\
__Default__: False (unchecked)
`
};

const forceVddr = {
    description: `Necessary for external load mode, or for maximum PA output \
power`,
    longDescription: `
This value must match the Force VDDR parameter in the Device Configuration \
module.
\n\
It must only also be configured in the TI 15.4 Stack module for \
CoProcessor projects in order to generate the appropriate radio setup command. \
For all other projects, the radio setup command will be configured using the \
Transmit Power parameter and the Force VDDR parameter in the Device \
Configuration module.
\n\
__Default__: False (unchecked)
`
};

const psipComplianceWarning = `To comply with the FCC RF module compliance \
the following has to be fulfilled (when using the +20 dBm PA): \n\
* 902.2 MHz (CH1) – 908.0 MHz (CH30) \n\
* Maximum RF power transmitted without duty cycling is 16 dBm and antenna gain less than 2.69 dBi \n\n\
* 908.2 MHz (CH31) – 927.8 MHz (CH129) \n\
* Maximum RF power transmitted without duty cycling is 19 dBm and antenna gain less than 2.69 dBi
`

exports = {
    transmitPower: transmitPower,
    rxOnIdle: rxOnIdle,
    forceVddr: forceVddr,
    psipComplianceWarning: psipComplianceWarning
};
