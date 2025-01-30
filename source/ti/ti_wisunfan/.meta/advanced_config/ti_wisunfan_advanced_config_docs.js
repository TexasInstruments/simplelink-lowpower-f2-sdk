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
 *  ======== ti_wisunfan_network_config_docs.js ========
 */

"use strict";

// ======== Top Level ========

const rapidJoin = {
    readOnly: "Rapid join is only supported for network responsiveness profile configuration.",
    description: "Enable rapid join network configuration",
    longDescription: `
Enables rapid join network configuration. These settings are settings calibrated for an expedited join process
for networks of around 50 or less devices. 

The performance benefits from rapid join come from a collection of speedups Wi-SUN async message trickle timers and timers for
certain RPL messages required for joining. As a result, rapid join comes at the cost of higher network bandwidth utilization and
power consuption. Rapid join may be less effective with larger networks due to network bandwidth saturation.
`
};

const mplLowLatency = {
    description: "Enable low latency MPL multicast",
    longDescription: `
Enables a number of latency optimizations to traditional MPL (Multicast Protocol for Low-Power and Lossy Networks).
MPL is the multicast forwarding protocol used for Wi-SUN. Please refer to the Wi-SUN specification and RF7331 for details.
Enabling low latency multicast via MPL will signficantly lower the latency of multicast frames. To accomplish this, the
following changes are applied:

1. Reduction of MPL trickle timers to their lowest value, reducing latency before transmission and forwarding.
2. Disabling of initial transmission replays, lowering network flooding.
3. Disabling of MPL trickle inconsistency checking, lowering network flooding.

The first change in trickle timer reduction is the most significant change, but also has a tradeoff in increased network flooding.
In cases where back-to-back multicast transmissions are attempted, network bandwidth may become saturated by multicast flooding and
result in worse latency and bandwidth usage than expected.

It is recommended to reduce the hop limit (recommended 2-4) of multicast frames when using low latency multicast mode
to reduce multicast storms caused by flooding.
`
};

const rapidDisconnectDetectBr = {
    description: "Configure time in seconds for router nodes to detect border router disconnection.",
    longDescription: `
Configure the time (in seconds) for a router node to detect the disconnection of a border router node. Though this is only applicable
to router nodes, both border routers and routers should share the same value for this configuration. Note that a lower value requires more
frequent exchange of Wi-SUN message required for keep-alive. The recommended value is between 300 sec (5 minutes) and 1800 sec (30 minutes).
This value cannot exceed 65535 sec.

__Default__ : 1800
`
};

const rapidDisconnectDetectRn = {
    description: "Configure time in seconds for border router to detect router node disconnection.",
    longDescription: `
Configure the time (in seconds) for a border router to detect the disconnection of a router node. Though this is only applicable
to the border router, both border routers and routers should share the same value for this configuration. Note that a lower value requires more
frequent exchange of Wi-SUN message required for keep-alive. TThe recommended value is between 300 sec (5 minutes) and 7200 sec (120 minutes).
This value cannot exceed 65535 sec.

__Default__ : 7200
`
};

const networkProfile = {
    description: "Configure the network traffic profile used in network joining, upkeep, and multicast latency.",
    longDescription: `
Configures network traffic profile. Given the objectives of the network and estimated network size,
configure specific internal network timing and behavior for async (PA/PC), RPL, EAPOL, and MPL messages.

Maximum Responsiveness profile provides the fastest join times, fastest disconnection detection and recovery of network nodes, and
lowest multicast latency at the cost of more network bandwidth used and a less scalable network.

Maximum scalability profile allows support for a larger network and minimization of bandwidth at the cost of slower join times and
network responsiveness.

Balanced profile provides a middle ground between the above options.

After changing the network profile, the rapid join, MPL low latency, and disconnection detection times in sysconfig will be set to the
recommended value for the selected network profile. It is possible, but not recommended to change these configurations except in
the small network configuration, and could result in reduced performance.
`
}

exports = {
    rapidJoin: rapidJoin,
    mplLowLatency: mplLowLatency,
    rapidDisconnectDetectBr: rapidDisconnectDetectBr,
    rapidDisconnectDetectRn: rapidDisconnectDetectRn,
    networkProfile: networkProfile
};
