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
 *  ======== ti_wisunfan_rf_config_docs.js ========
 */

"use strict";

const rfDesign = {
    description: "Select which RF design to use as template",
    longDescription: `
The user must select an existing TI RF design to reference for radio \
configuration. This value must match the Based on RF Design parameter in the \
RF module.
\n\
__Default__: The RF design reference selected in this project is automatically \
configured based on the example. Please move to a custom board or see the \
other examples provided in the SDK if another reference board is desired.`
};

const region = {
    description: "Configures the selected PHY Region",
    longDescription: `
The TI Wi-SUN FAN Stack can be configured for operation in a variety \
of country regions. The PHY modes available will depend on the \
region selected.
\n\
__Default__: NA (North America)
\n\
Region | Abbreviation
------ | ---
Europe | EU
North America | NA
Brazil | BZ
Japan | JP
`
};

const regDomain = {
    description: "Configures the selected Regulatory Domain",
    readOnly: "Automatically set based on region selected",
    longDescription: `
The Regulatory Domain is automatically set based on the region \
selected.
`
};

const opClass = {
    description: "Configures the selected operating class",
    longDescription: `
The Operating Class (OC) field is an 8-bit unsigned integer \
which is used to achieve the 213 desired data rate and channel \
plan within the Regulatory Domain.
\n\
`
};

const channelSpacing = {
    description: "Configures the spacing in between each of the channels",
    readOnly: "Set automatically based on region and operating class selected",
    longDescription: `Configures the spacing in between each of the channels in kHz.`
};

const totalChannels = {
    description: "Configures the total number of channels",
    readOnly: "Set automatically based on region and operating class selected",
    longDescription: `Configures the total number of channels.`
};

const centerFrequency = {
    description: "Configures the frequency of channel 0",
    readOnly: "Set automatically based on region and operating class selected",
    longDescription: `
This parameter configures the frequency of channel 0, the first channel, in MHz.
\n\
The Wi-SUN FAN standard sets a channel's center frequency using the following \
formula: ChanCenterFreq = ChanCenterFreq0 + (NumChan × ChanSpacing).
\n\
* ChanCenterFreq0 is the first channel's center frequency in MHz
* ChanSpacing is the separation between adjacent channels in kHz
* NumChan is the channel number`
};

const customPhy = {
    description: "Allows configuration of advanced PHY Parameters",
    longDescription: `
Allow the configuration of the starting channel frequency, \
channel spacing and total channels if checked.
\n\
__Default__: False (Unchecked)
`
};

const phyModeID = {
    description: "Configures the PHY mode",
    longDescription: `
The physical layer (PHY) is the lowest layer of the TI Wi-SUN FAN Stack \
and configures the physical properties of the RF signal.\
The following modes are defined by the Wi-SUN Alliance and supported by \
TI WI-SUN FAN Stack.
\n\
Operating Mode ID | Mode ID | Data Rate (kbps) | Modulation Index | Deviation (kHz)
-- | - | --- | --- | ---
1a | 1 | 50  | 0.5 | 12.5
1b | 2 | 50  | 1   | 25
2a | 3 | 100 | 0.5 | 25
2b | 4 | 100 | 1   | 50
3  | 5 | 150 | 0.5 | 37.5
4a | 6 | 200 | 0.5 | 50
4b | 7 | 200 | 1   | 100
\n\
Note that the available PHY modes will depend on the region selected.
\n\
For more information, refer to the [Data Rate and PHY](/ti_wisunfan/html/\
wisun-stack/PHY.html) section of the TI Wi-SUN FAN User's Guide.
\n\
__Default__: Mode 2, 50kbps, Mod Index 1
`
};

const opModeID = {
    description: "Configures the Operating Mode ID",
    readOnly: "Automatically set based on Phy Mode ID selected",
    longDescription: `
The Operating Mode ID corresponds to the PHY descriptor to be used based on the region \
of operation. It is automatically configured based on the selected Phy Mode ID.

__Default__ : 1b
`
};

const opModeClass = {
    description: "Configures the Operating Mode Class",
    longDescription: `
Customers are expected to update this value based on the specification 
matching deployment requirements.  Generally, this value is set based on
the selected region and Phy Mode ID.
`
};

exports = {
    rfDesign: rfDesign,
    region: region,
    customPhy: customPhy,
    regDomain: regDomain,
    opModeID: opModeID,
    phyModeID: phyModeID,
    opClass: opClass,
    channelSpacing: channelSpacing,
    totalChannels: totalChannels,
    opModeClass: opModeClass,
    centerFrequency: centerFrequency
};
