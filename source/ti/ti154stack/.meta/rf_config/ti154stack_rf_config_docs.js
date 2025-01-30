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
The user must select an existing TI RF design to reference for radio
configuration. This value must match the Based on RF Design parameter in the
RF module.
\n\
__Default__: The RF design reference selected in this project is automatically
configured based on the example. Please move to a custom board or see the
other examples provided in the SDK if another reference board is desired.`
};

const freqBand = {
    description: "Configures the frequency band used for radio operations",
    readOnly: "Automatically set based on the project selected",
    longDescription: `
The TI 15.4-Stack can be configured for operation in Sub-1 GHz and 2.4GHz
frequency bands.
\n\
__Default__: The frequency band selected in this project is automatically
configured based on the example. Please see the other examples provided in
the SDK if another frequency band is desired.
`
};

const freqSub1 = {
    description: "Configures Sub-1 GHz frequency band used for radio operation",
    longDescription: `
Configures the Sub-1 GHz ISM frequency band that will be used for radio
configuration. The Sub-1 GHz frequency band should be selected based on region
of operation.
\n\
For more information, refer to the [Data-Rate and PHY](/ti154stack/html/\
ti154stack/ti154stack-overview.html#data-rate-and-phy) section of the
TI15.4-Stack User's Guide.

Note that some PHYs are only supported on specific platforms. In particular,
the China band (433MHz) PHY is only supported on CC1352P-4 and CC1352P7_4
platforms.
\n\
__Default__: 902-927 MHz
\n\
Frequency Band | Region
---   | ---
433 - 434 MHz | China
863 - 869 MHz | European Union
902 - 927 MHz | North America
920 - 927 MHz | Japan
`
};

const phyType = {
    description: "Configures the PHY frequency modulation and data-rate",
    readOnly: "Only phy type available for 2.4 GHz projects",
    longDescription: `
Configures the frequency modulation and data rate that will be used for radio
configuration.
\n\
For more information, refer to the
[Data-Rate and PHY](/ti154stack/html/ti154stack/\
ti154stack-overview.html#data-rate-and-phy) section of the TI15.4-Stack User's
Guide.
\n\
__Default__ (Sub-1 GHz): 50kbps, 2-GFSK
\n\
__Default__ (2.4 GHz): 250kbps, OQPSK
`
};

const regulationType = {
    description: "Selects the PHY regulation",
    longDescription: `
Select the PHY regulation to be followed. Note that not all frequency
bands are supported on each standard.
\n\
__Supported Frequency Bands__:
\n\
Regulation|433-434 MHz|863-869 MHz|902-927 MHz|920-927 MHz|2.4 GHz|
:-------: | :-------: | :-------: | :-------: | :------: | :---: |
ARIB      |           |           |           | x        |       |
ETSI      |           | x         |           |          |       |
FCC       | x         |           | x         |          |       |
IEEE 802.15.4 |       |           |           |          | x     |
\n\
__Default__ (Sub-1 GHz): FCC
\n\
__Default__ (2.4 GHz): IEEE 802.15.4

`
};

const phyID = {
    description: "Configures the PHY descriptor",
    readOnly: "Automatically set based on frequency and phy type selected",
    longDescription: `
The PHY ID corresponds to the PHY descriptor to be used based on the region
of operation. It is automatically configured based on the selections made for
frequency band, modulation, and data rate.
\n\
For more information, refer to the [Data-Rate and PHY](/ti154stack/html/\
ti154stack/ti154stack-overview.html#data-rate-and-phy) section of the
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
Configures the channel page on which to perform the scan. It is automatically
configured based on the PHY ID.
\n\
__Default__ (Sub-1 GHz): APIMAC_CHANNEL_PAGE_9
\n\
__Default__ (2.4 GHz): APIMAC_CHANNEL_PAGE_NONE
`
};

const coex = {
    enable: {
        displayName: "RF Coexistence",
        description: "Select coex mode of operation",
        longDescription: `
Enable the use of external pin signaling to communicate with another RF capable
device, for the devices to coexist in the same 2.4GHz frequency band.
The coexistence (coex) feature is a wired signal interface between an IEEE 15.4
device and a Wi-Fi device:

\`\`\`
Coex Secondary          Coex Primary
   Device                  Device
+---------+             +---------+
|IEEE 15.4|   Signals   |  Wi-Fi  |
|         | <----/----> |         |
+---------+      n      +---------+
\`\`\`
The coex interface provides a set of signals based on the supported
Packet Traffic Arbitration (PTA) approach:

* REQUEST: Output signal, indicating a request to perform RF activity.
* PRIORITY: Output signal, time-shared between indicating (1) request priority
and (2) type of RF activity.
* GRANT: Input signal, indicating the response to the request to perform RF
activity.

The available combinations of these signals are defined as _coex modes_.
This option is used to match the interface of the coex device that the device
is coexisting with. The following signals are enabled for the different modes:

|Coex Mode       | REQUEST | PRIORITY | GRANT |
|----------------|:-------:|:--------:|:-----:|
| 3-Wire         | x       | x        | x     |
| 2-Wire         | x       |          | x     |

__Default__: Disabled
`
    },
    configGroup: {
        displayName: "RF Coexistence Configuration"
    },
    defaultPriority: {
        displayName: "Default Priority",
        description: `
Priority level used when time-shared PRIORITY signal indicates request
priority
`,
        longDescription: `
This option sets the priority level used by the PRIORITY signal when it
indicates the priority of the coex request. The duration of this priority
level is defined by the __Priority Indication Time__ option. See the
__Coex Mode__ description for more information on what this signal is used for.
`
    },
    assertRequestForRx: {
        displayName: "Assert REQUEST Signal For RX",
        description: "Specify if REQUEST signal is asserted for RX commands",
        longDescription: `
If this option is _disabled_, the IEEE device is configured to not assert the
REQUEST signal (and subsequently disregard any of the other coex signals)
when the scheduled RF activity is an RX command.

Note: If an RX command is chained with a TX command, the REQUEST signal
will be asserted in time for the TX activity and will _remain asserted_
until the end of the command chain, even if the following command is an RX.

\`\`\`
              _______________
REQ  ________|               |__
        ____    ____    ____
RF   __| rx |__| tx |__| rx |___
\`\`\`
`
    },
    useCaseConfigGroupIeee: {
        displayName: "IEEE Use Case Configuration",
        iniTx: {
            displayName: "TX Connection Establishment"
        },
        conTx: {
            displayName: "TX Connected"
        },
        iniRx: {
            displayName: "RX Connection Establishment"
        },
        conRx: {
            displayName: "RX Connected"
        }
    }
};

const customPhy = {
    description: "Enables usage and configuration of Sub1GHz custom PHY",
    readOnly: "Disabled by Custom PHY option. Setting is overridden by " +
        "custom PHY settings.",
    readOnlyMode: "Disabled by Custom PHY option. Only non-beacon mode is " +
        "supported for Custom PHY",
    longDescription: `
Enables the usage and configuration of Sub1GHz custom PHY. This will override
existing selections for regulation type, Sub1GHz frequency, and PHY type,
instead using the custom configurations below and the configurables provided
by the Custom PHY Radio Configuration tab.

This option is only available for collector and sensor projects. Coprocessor
is not supported. Additionally, only non-beacon mode is supported. Beacon and
FH mode are not supported.

Note that PHYs enabled by the custom PHY mode have not been individually
verified to function with the 15.4 MAC or application examples. Certain
combinations of configurations may not be fully functional.

The default settings provided by the custom PHY mode support the WB-DSSS PHYs.
To access new PHYs, change the PHY Type configurable in the Custom PHY Radio
Configuration tab. It is recommended to only change the PHY Type configurable
and leave all other settings to their default values.

After selecting the desired PHY Type, make compatible selections in the
custom frequency band, channel 0 center frequency, channel spacing, total
channel number, and symbol rate fields.

__Default__: false
`
};

const customFreqBand = {
    description: "Set Sub-1 GHz frequency band for custom PHY",
    longDescription: `
Configures the Sub-1 GHz ISM frequency band that will be used for radio
configuration. The Sub-1 GHz frequency band should be selected based on region
of operation. The band must also lie within the Frequency Band selection
in the Custom PHY Radio Configuration tab.

\n\
For more information, refer to the [Data-Rate and PHY](/ti154stack/html/\
ti154stack/ti154stack-overview.html#data-rate-and-phy) section of the
TI15.4-Stack User's Guide.
\n\
__Default__: 902-927 MHz
\n\
Frequency Band | Region
---   | ---
433 - 434 MHz | China
863 - 869 MHz | European Union
902 - 927 MHz | North America
920 - 927 MHz | Japan
`
};

const customCh0Freq = {
    description: "Set custom the channel 0 frequency in MHz",
    longDescription: `
Configure starting channel (channel 0) frequency in MHz. Must be within the
Sub1GHz frequency band range.

__Default__: 903.0 MHz
`
};

const customChSpacing = {
    description: "Set the channel spacing in kHz",
    longDescription: `
Configures the interchannel spacing in kHz. Must be twice as high as the RX
Filter BW set in the Custom PHY Radio Configuration tab (under PHY properties).

__Default__: 1600 kHz
`
};

const customChNum = {
    description: "Set the total number of channels used",
    longDescription: `
Set the total number of channels used. Combined with Ch0 frequency and channel
spacing configuration, final channel range should be within the Sub-1 GHz
frequency band selection.

__Default__: 16
`
};

exports = {
    rfDesign: rfDesign,
    freqBand: freqBand,
    regulationType: regulationType,
    freqSub1: freqSub1,
    phyType: phyType,
    phyID: phyID,
    channelPage: channelPage,
    coex: coex,
    customPhy: customPhy,
    customFreqBand: customFreqBand,
    customCh0Freq: customCh0Freq,
    customChSpacing: customChSpacing,
    customChNum: customChNum
};
