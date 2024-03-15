/*
 * Copyright (c) 2019-2022 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== ti154stack_docs.js ========
 */

"use strict";

const lockProject = {
    description: "Sets project-specific configs to read-only. Always hidden",
    longDescription: `
This is a hidden parameter that locks the project and frequency band for SDK \
examples. It also locks the security level in SM projects. It is set to true \
in each example's .syscfg file.
\n\
__Default__: False (unchecked)
`
};

const genLibs = {
    description: "Configures genLibs usage for local libraries. Always hidden",
    longDescription: `
This is a hidden parameter that controls whether the stack module contributes \
libraries to the generated linker command file.
\n\
__Default__: True (checked)
`
};

const project = {
    description: "Configures project type for device configuration",
    readOnly: "Only this project is supported",
    longDescription: `
The software development kit (SDK) provides example applications for the TI \
15.4-Stack developed for the CC13x2 and CC26x2 platforms.
\n\
__Default__: The project type selected in this project is automatically  \
configured based on the example. Please see the other examples provided in \
the SDK if another project is desired.
\n\
__Note__: For more information on the type of example projects available, \
refer to the \
[Example Applications](/ti154stack/html/ti154stack/example-applications.html) \
section of the TI15.4-Stack User's Guide.
`
};

const mode = {
    description: "Configures the mode of network operation",
    longDescription: `
Configures the mode of network operation.
\n\
For more information regarding the configuration of the mode, refer to the \
[Configuring Stack](/ti154stack/html/ti154stack/\
configuring-stack.html#mode-configuration) section of the TI15.4-Stack User's \
Guide.
\n\
__Default__: Non Beacon
\n\
Value | Description
--- | ---
Beacon | Beacon-enabled mode of network operation where coordinator device \
transmits periodic beacons to indicate its presence. For more information, \
refer to the [Beacon Mode](/ti154stack/html/ti154stack/beacon-mode.html) \
section of the TI15.4-Stack User's Guide.
Non Beacon | Non beacon-enabled mode of network operation where coordinator \
does not send out periodic beacons. For more information, refer to the \
[Non Beacon Mode](/ti154stack/html/ti154stack/non-beacon-mode.html) section of \
the TI15.4-Stack User's Guide.
Frequency Hopping | Frequency hopping mode of network operation where network \
devices hop on different frequencies. For more information, refer to the \
[Frequency Hopping Mode](/ti154stack/html/ti154stack/\
frequency-hopping-mode.html) section of the TI15.4-Stack User's Guide.
`
};

const ti154stackModule = {
    description: "TI 15.4 Stack Configuration",
    longDescription: `
The [__TI 15.4 Stack__](/ti154stack/html/ti154stack/api-reference.html) \
implements the standard IEEE 802.15.4e and 802.15.4g specification, and is a \
complete software platform for developing applications that require extremely \
low-power, long-range, reliable, robust and secure wireless star-topology \
based networking solutions.
\n\
* [Usage Synopsis](/ti154stack/html/ti154stack-guide/index-cc13xx_26xx.html)
* [Examples](/ti154stack/html/ti154stack/example-applications.html)
`
};

exports = {
    lockProject: lockProject,
    genLibs: genLibs,
    project: project,
    mode: mode,
    ti154stackModule: ti154stackModule
};
