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
 *  ======== ti154stack_rf_config_docs.js ========
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

const freqBand = {
    description: "Configures the frequency band used for radio operations",
    readOnly: "Automatically set based on the project selected",
    longDescription: `
The TI 15.4-Stack can be configured for operation in Sub-1 GHz and 2.4GHz \
frequency bands.
\n\
__Default__: The frequency band selected in this project is automatically \
configured based on the example. Please see the other examples provided in \
the SDK if another frequency band is desired.
`
};

const freqSub1 = {
    description: "Configures Sub-1 GHz frequency band used for radio operation",
    longDescription: `
Configures the Sub-1 GHz frequency band that will be used for radio \
configuration. The Sub-1 GHz frequency band should be selected based on region \
of operation.
\n\
For more information, refer to the [Data-Rate and PHY](/ti154stack/html/\
ti154stack/ti154stack-overview.html#data-rate-and-phy) section of the \
TI15.4-Stack User's Guide.
\n\
__Default__: 915 MHz
\n\
Value | Description
--- | ---
433 MHz | Sub-1 GHz 433 MHz ISM band 
868 MHz | Sub-1 GHz 868 MHz ISM band 
915 MHz | Sub-1 GHz 915 MHz ISM band 
`
};

const phyType = {
    description: "Configures the PHY frequency modulation and data-rate",
    readOnly: "Only phy type available for 2.4 GHz projects",
    longDescription: `
Configures the frequency modulation and data rate that will be used for radio \
configuration.
\n\
For more information, refer to the \
[Data-Rate and PHY](/ti154stack/html/ti154stack/\
ti154stack-overview.html#data-rate-and-phy) section of the TI15.4-Stack User's\
Guide.
\n\
__Default__ (Sub-1 GHz): 50kbps, 2-GFSK
\n\
__Default__ (2.4 GHz): 250kbps, IEEE 802.15.4
`
};

const phyID = {
    description: "Configures the PHY descriptor",
    readOnly: "Automatically set based on frequency and phy type selected",
    longDescription: `
The PHY ID corresponds to the PHY descriptor to be used based on the region \
of operation. It is automatically configured based on the selections made for \
frequency band, modulation, and data rate.
\n\
For more information, refer to the [Data-Rate and PHY](/ti154stack/html/\
ti154stack/ti154stack-overview.html#data-rate-and-phy) section of the \
TI15.4-Stack User's Guide.
\n\
__Default__ (Sub-1 GHz): APIMAC_50KBPS_915MHZ_PHY_1
\n\
__Default__ (2.4 GHz): APIMAC_250KBPS_IEEE_PHY_0
`
};

const channelPage = {
    description: "Configures the channel page on which to perform the scan",
    readOnly: "Automatically set based on frequency and phy type selected",
    longDescription: `
Configures the channel page on which to perform the scan. It is automatically \
configured based on the PHY ID.
\n\
__Default__ (Sub-1 GHz): APIMAC_CHANNEL_PAGE_9
\n\
__Default__ (2.4 GHz): APIMAC_CHANNEL_PAGE_NONE
`
};

exports = {
    rfDesign: rfDesign,
    freqBand: freqBand,
    freqSub1: freqSub1,
    phyType: phyType,
    phyID: phyID,
    channelPage: channelPage
};
