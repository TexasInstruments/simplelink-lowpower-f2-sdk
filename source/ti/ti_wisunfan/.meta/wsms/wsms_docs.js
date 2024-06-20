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
 *  ======== wsms_docs.js ========
 */

"use strict";

const project = {
    description: "Configures project type for device configuration",
    readOnly: "Only this project is supported",
    longDescription: `
The software development kit (SDK) provides example applications for the TI \
Wireless Solar Management System developed for the CC13xx and CC26xx platforms.
\n\
__Default__: The project type selected in this project is automatically  \
configured based on the example. Please see the other examples provided in \
the SDK if another project is desired.
\n\
`
};

const mode = {
    description: "Configures the mode of network operation",
    longDescription: `
Configures the mode of network operation.
\n\
__Default__: Frequency Hopping
\n\
Mode | Description
--- | ---
Frequency Hopping | Mode of operation in which network \
devices hop on different frequencies.
Fixed Channel | Mode of operation in which network \
devices stay on a fixed frequency.
`
};

const wsmsModule = {
    description: "Wireless Solar Management System Configuration",
    longDescription: `
The Wireless Solar Management System implements a frequency hopping Sub1GHz \
network, aiming to enable large-scale solar panel control and similar IOT \
applications in a star or mesh network topology.
`
};

exports = {
    project: project,
    mode: mode,
    wsmsModule: wsmsModule
};
