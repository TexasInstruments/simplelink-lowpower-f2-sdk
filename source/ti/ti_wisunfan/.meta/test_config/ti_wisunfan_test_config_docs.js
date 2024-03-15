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
 *  ======== ti_wisunfan_test_config_docs.js ========
 */

"use strict";

const powerTestEnable = {
    description: "Enables power testing mode",
    longDescription: `
Enables power settings including data ramp sizes and power profile settings.
\n\
__Default__: False (unchecked)
`
};

const certificationTestMode = {
    description: `Enables certification mode for FCC or ETSI compliance \
testing`,
    longDescription: `
If set to true, the device joins the collector and transmits back-to-back \
frames to it. The collector does not generate any frames, but simply \
acknowledges the transmissions from the sensor. The frames can be captured \
using a spectrum analyzer to perform the channel occupancy test.
\n\
The mode can also be used for ETSI testing. The example application is only \
provided for a general guidance and for ease in performing regulation tests.
\n\
__Note__: The FH conformance certification example application is provided to \
allow users to perform a FCC or ETSI compliance tests related to channel
occupancy.
\n\
For more information, refer to the [Configuration Parameters](/ti_wisunfan/html/\
ti_wisunfan/example-applications.html#configuration-parameters) and the \
[FH Conformance Certification Application Example](/ti_wisunfan/html/ti_wisunfan/\
example-applications.html#fh-conformance-certification-application-example) \
sections of the TI15.4-Stack User's Guide.
\n\
__Default__: False (unchecked)
`
};

const powerTestProfile = {
    description: "Configures the power profile to be used during a power test",
    longDescription: `Configures the type of data and which devices transmit \
during a power test.
\n\
__Default__: Data and ACK
\n\
Value | Description
--- | ---
Polling Only | Polling only
Data and ACK | Application data and ACK is sent from the sensor to collector
Poll and Data | Poll and received data from the collector
Sleep | No poll or data.  Note that beacon mode will increase current due to \
beacon events if  the device is sleepy.
`
};

const collectorTestRampDataSize = {
    description: `Configures size of test data sent by the collector \
(dependent on the test profile)`,
    longDescription: `
Configures the size of the ramp data to be sent by the collector
\n\
__Default__: 20
\n\
__Range__: 0 to UINT16_MAX
`
};

const sensorTestRampDataSize = {
    description: `Configures size of test data sent by the sensor \
(dependent on the test profile)`,
    longDescription: `
If enabled, the periodic sensor message shall be sent as a fixed size packet \
of the specified size. If set to 0, the periodic sensor message shall be of \
the type sensor data specified in the smsgs.h file.
\n\
__Default__: 0
\n\
__Range__: 0 to UINT16_MAX
`
};

exports = {
    powerTestEnable: powerTestEnable,
    certificationTestMode: certificationTestMode,
    powerTestProfile: powerTestProfile,
    collectorTestRampDataSize: collectorTestRampDataSize,
    sensorTestRampDataSize: sensorTestRampDataSize
};
