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
 *  ======== ti_wisunfan_network_config_docs.js ========
 */

"use strict";

// ======== Top Level ========

const panID = {
    description: "Configures personal area network ID",
    longDescription: `
Used to restrict the network to a certain PAN ID. If left as 0xFFFF on the \
border router, the border router starts with a randomly selected PAN ID between 0x0000 and 0xFFFE.
\n\
PAN ID is initially set to 0xFFFF on router nodes. On network join, it is updated to the PAN ID \
of the joined network.
\n\
__Default__: 0xFFFF
\n\
__Range__: 0x0000 to 0xFFFF
\n\
__Acceptable Values__: Accepts inputs in decimal, and in hexadecimal provided \
that the value is prefaced by '0x'.
`
};

const fhChannels = {
    description: "Configures channels to be scanned",
    longDescription: `
Represents the list of channels on which the device can hop. In frequency \
hopping, the actual sequence will be based on DH1CF function.
\n\
The first byte represents channels 0 to 7, and the last byte represents \
channels 128 to 135; for example, 0x01 0x10 represents the inclusion of \
channels 0 and 12.
\n\
__Default__: 0 to 128
\n\
For more information, refer to the \
[MAC and Frequency Hopping](/ti_wisunfan/html/wisun-stack/MAC.html#mac-and-frequency-hopping/\
frequency-hopping-mode.html) section of the TI Wi-SUN FAN Stack User's Guide.
`
};

const fhAsyncChannels = {
    description: "Configures channels to target async frames",
    longDescription: `
Configures the list of channels to target the async frames.
\n\
It is represented \
as a bit string with LSB representing Ch0; for example, 0x01 0x10 represents \
the inclusion of channels 0 and 12. It must cover all channels that could \
be used by a target device in its hopping sequence. Channels marked beyond \
number of channels supported by PHY Config are excluded by stack. To avoid \
interference on a channel, remove it from async mask and add it to the \
exclude channels for FH Channel Mask (\`CONFIG_FH_CHANNEL_MASK\`).
\n\
__Default__ (Border Router): 0 to 128
\n\
__Default__ (Router): 0 to 55
\n\
For more information, refer to the \
[MAC and Frequency Hopping](/ti_wisunfan/html/wisun-stack/MAC.html#mac-and-frequency-hopping/\
frequency-hopping-mode.html) section of the TI Wi-SUN FAN Stack User's Guide.
`
};

const maxDevices = {
    description: "Configures the maximum number of children for coordinator",
    longDescription: `
Configures the maximum number of children devices that can be supported by \
the coordinator. It is recommended that this value be set below 150.
\n\
__Default__: 50
\n\
__Range__: 0 to UINT16_MAX
\n\
***Additional Setup Required***:
\n\
\`MAX_DEVICE_TABLE_ENTRIES\` in the project specific .opts file must be \
updated if this parameter is modified from the default setting. For secure \
applications, \`MAX_DEVICE_TABLE_ENTRIES\` must be set equal to or greater \
than the maximum devices selected. For non-secure applications, \
\`MAX_DEVICE_TABLE_ENTRIES\` can be set to 0 to save space.
`
};

const fhNetname = {
    description: "Configures value for FH PIB attribute netname",
    longDescription: `
The network name is a unique network identifier that is configured \
by the application using frequency hopping and PAN information base (FH-PIB) \
attributes. Maintenance of the NetName is beyond the scope of TI Wi-SUN FAN Stack \
and is not used to filter frames; instead, the value is carried in a PAN \
Advertisement frame that can be parsed by a receiving application.
\n\
__Default__: Wi-SUN Network
\n\
__Acceptable Values__: 32-bit ASCII value
    `
};

const starNetwork = {
    description: "Force the network topology to a star network.",
    longDescription: `
Use the RPL minimum rank increase per hop to enforce a star network topology. \
All nodes will join the network directly connected to the base node at 1-hop.
`
};

// ======== Advanced MAC ========

const ccaThreshold = {
    description: "Configures network ccaThreshold",
    longDescription: `
Configures the CCA Threshold that you wish to use within the network.
\n\
__Default__: -60
\n\
__Range__: -60 to -120
`
};

const fhBroadcastInterval = {
    description: `Configures interval in ms between start of two broadcast \
slots`,
    longDescription: `
Configures the interval in milliseconds for broadcast message generation \
in frequency hopping. If set to 0, it shall disable broadcast messages and \
will not cause sleepy devices any additional power overhead.
\n\
It is recommended that this value be set above 200 ms.
\n\
__Default__: 10000
\n\
__Range__: 0 to UINT32_MAX
`
};


const unicastDwellTime = {
    description: `Configures interval in ms between start of two unicast \
slots`,
    longDescription: `
Configures the interval in milliseconds for unicast message generation \
in frequency hopping. If set to 0, it shall disable unicast messages and \
will not cause sleepy devices any additional power overhead.
\n\
It is recommended that this value be set above 200 ms.
\n\
__Default__: 255
\n\
__Range__: 0 to 255
`
};

const fhBroadcastDwellTime = {
    description: `Configures duration in ms of the node’s broadcast slot in \
frequency hopping`,
    longDescription: `
Configures the duration, in milliseconds, of a node's broadcast slot in \
frequency hopping. If set to 0, it shall disable broadcast hopping and \
broadcast message transmissions.
\n\
__Default__: 255
\n\
__Range__: 0 to 255
`
};

const customMinTxOff = {
    description: `Enable in order to set a custom minimum TX off-time`,
    longDescription: `
The minimum TX off-time, also known as the pause duration, specifies the \
minimum amount of time during which the transmitter shall remain off following \
a transmission.
\n\
If left unselected, the MAC will use the minimum TX off-time specified by the \
appropriate regulation:
* ETSI (EU region): 100 ms
* ARIB (JP region): 2 ms
\n\
__Default__: False (unchecked)
`
};

const minTxOff = {
    description: `Configures a custom minimum TX off-time`,
    longDescription: `
The minimum TX off-time, also known as the pause duration, specifies the \
minimum amount of time during which the transmitter shall remain off following \
a transmission.
\n\
Note that on the CoProcessor project this minimum TX off-time will be used for \
all PHYs.
\n\
__Default__ (ETSI (EU region)): 100 ms
\n\
__Default__ (ARIB (JP region)): 2 ms
\n\
__Range__: 0 to UINT32_MAX
`
};

const dutyCycleEnable = {
    description: `Enable duty cycling to set a network utilization threshold`,
    longDescription: `
The duty cycle feature keeps track of the network utilization in an arbitrary \
hour (3600 sec) and rejects any radio TX (including ACK frames) that would \
violate the network utilization setting.
\n\
If left unselected, the network will not use duty cycling. This feature is \
currently only available when using an EU or JP region PHY.

\n\
__Default__: False
`
};

const dutyCycle = {
    description: `Configures the network utilization threshold in percentage.`,
    longDescription: `
The duty cycle feature keeps track of the network utilization in an arbitrary \
hour (3600 sec), and rejects any radio TX (including ACK frames) that would \
violate the network utilization setting.
\n\
The configurable sets the allowable network utilization per hour as a \
percentage of the total 1 hour window. For example, a 10% duty cycle value \
would permit total network utilization of 3600*0.10 = 360 sec per hour window.
\n\
__Default__ (fixed channel network): 10%
\n\
__Range__: 0 to 100
`
};

exports = {
    panID: panID,
    channels: fhChannels,
    fhChannels: fhChannels,
    fhAsyncChannels: fhAsyncChannels,
    maxDevices: maxDevices,
    fhNetname: fhNetname,
    starNetwork: starNetwork,
    fhBroadcastInterval: fhBroadcastInterval,
    fhBroadcastDwellTime: fhBroadcastDwellTime,
    ccaThreshold: ccaThreshold,
    unicastDwellTime: unicastDwellTime,
    custMinTxOff: customMinTxOff,
    minTxOff: minTxOff,
    dcEnable: dutyCycleEnable,
    dutyCycle: dutyCycle
};
