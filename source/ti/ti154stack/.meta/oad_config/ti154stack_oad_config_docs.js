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
 *  ======== ti154stack_oad_config_docs.js ========
 */

"use strict";

const oadBlockSize = {
    description: "Configures the number of bytes sent in an OAD Block",
    longDescription: `
Configures the number of bytes sent in an OAD Block, and thus affects the \
time taken to complete the OAD. Care must be taking when setting this to a \
high value in environments where noise is a concern, as the chances of a \
block being corrupted due to an interfere increases with the size of the \
block.
\n\
__Default__ (Sub-1 GHz): 128
\n\
__Default__ (2.4 GHz): 64
\n\
__Range__(Sub-1 GHz): 16 to 496. Note that increasing the block size past 128 \
bytes on a Linux VM may cause instability.
\n\
__Range__(2.4 GHz): 16 to 64
`
};

const oadBlockReqRate = {
    description: "Configures interval in ms between block requests from client",
    longDescription: `
Configures the amount of time in milliseconds on the OAD client between 1 \
block request and the next block request.
\n\
Reducing this value will reduce the time taken to send an OAD. The time that \
OAD takes to complete is application dependent, severaly power constrained \
devices may need large periods between block requests to allow the power
source to recover. Other applications may require the OAD to complete quickly, \
such as in systems where a service engineer is required to performing the OAD.
\n\
__Default__:
The following table holds the formula or optimized value used to set the \
default \`OAD_BLOCK_REQ_RATE\` value upon a \`BEACON_INTERVAL\` change.
\n\
Beacon Order| Block Request Rate
--- | ---
15 | 200
Other (explicit polling) | 200
Other (implicit polling) | BEACON_INTERVAL - 100 \`(1)\`
\n\
\`(1) BEACON_INTERVAL = 2 ^ CONFIG_BEACON_ORDER * 960 * SYMBOL_DURATION_IN_MS\`
\n\
For frequency hopping mode when using a 5kbps PHY, the default block request \
rate is 400.
\n\
For beacon mode, the speed of OAD transfer is affected by the \
\`CONFIG_BEACON_ORDER\` and \`PHY_ID\` set. Additionally, the constant value \
(100) that is subtracted from the \`OAD_BLOCK_REQ_RATE\` can be modified in \
user calculations to improve performance.
\n\
__Range__: 0 to UINT32_MAX
`
};

const oadBlockReqPollDelay = {
    description: `Configures delay in ms between sending block request and \
block response poll request`,
    longDescription: `
Configures the delay, in milliseconds, on the OAD client between sending the \
block request and sending the poll request for the OAD block response.
\n\
A smaller \`OAD_BLOCK_REQ_POLL_DELAY\` decreases the amount of time the OAD \
block is queued in the collector, and hence the chance that the collector will \
suffer a buffer overflow. The collector, however, will take a finite amount of \
time to retrieve the OAD block and queue the OAD block response, so care \
must be taken to make the \`OAD_BLOCK_REQ_POLL_DELAY\` long enough to allow \
the collector to respond.
\n\
It is advisable to make this as small as possible in large networks where the \
collector needs to queue messages for many Reduced Functional Devices (RFDs).
\n\
The following table holds the formula or optimized value used to set the \
default \`OAD_BLOCK_REQ_POLL_DELAY\` value upon a \`BEACON_INTERVAL change\`.
\n\
Beacon Order| Block Request Poll Delay
--- | ---
15 | 50
Other (explicit polling) | 50
Other (implicit polling) | BEACON_INTERVAL - 400 \`(1)\`
\n\
\`(1) BEACON_INTERVAL = 2^(CONFIG_BEACON_ORDER) * 960 * SYMBOL_DURATION_IN_MS\`
\n\
For frequency hopping mode when using a 5kbps PHY, the default block request \
poll delay is 100.
\n\
For beacon mode, the speed of OAD transfer is affected by the \
\`CONFIG_BEACON_ORDER\` and \`PHY_ID\` set. Additionally, the constant value \
(400) that is subtracted from the \`OAD_BLOCK_REQ_POLL_DELAY\` can be modified \
in user calculations to improve performance.
\n\
__Range__: 0 to UINT32_MAX
`
};

exports = {
    oadBlockSize: oadBlockSize,
    oadBlockReqRate: oadBlockReqRate,
    oadBlockReqPollDelay: oadBlockReqPollDelay
};
