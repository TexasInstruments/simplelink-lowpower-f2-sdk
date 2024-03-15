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
 *  ======== ti_wisunfan_docs.js ========
 */

"use strict";

const lockProject = {
    description: "Sets project-specific configs to read-only. Always hidden",
    longDescription: `
This is a hidden parameter that locks example parameters for SDK \
examples. It is set to true in each example's .syscfg file.
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
Wi-SUN FAN Stack developed for the CC13xx and CC26xx platforms.
\n\
__Default__: The project type selected in this project is automatically  \
configured based on the example. Please see the other examples provided in \
the SDK if another project is desired.
\n\
__Note__: For more information on the type of example projects available, \
refer to the \
[Example Application Guide](/ti_wisunfan/html/wisun-guide/example-applications.html) \
section of the TI Wi-SUN FAN Stack User's Guide.
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
devices hop on different frequencies. For more information, refer to the \
[MAC and Frequency Hopping](/ti_wisunfan/html/wisun-stack/MAC.html#mac-and-frequency-hopping/\
frequency-hopping-mode.html) section of the TI Wi-SUN FAN Stack User's Guide.
Fixed Channel | Mode of operation in which network \
devices stay on a fixed frequency.
`
};

const secureBoot = {
    description: "Configure whether to build as a secure boot image via serial bootloader.",
    longDescription: `
If enabled, build a secure boot binary image (.bin file) that can be loaded to the device via \
flashing or with a secure serial bootloader. MCUBoot must be be loaded on the device at the \
default location to boot into the secure image. \
Note that to build the secure boot image, you must also change the project configuration to \
SecureBoot instead of Release. \
If disabled, do not build the secure boot images. Note that the project configuration should be \
the default value of Release if this option is disabled.
`
};


const ti_wisunfanModule = {
    description: "TI Wi-SUN FAN Stack Configuration",
    longDescription: `
The [__TI Wi-SUN FAN Stack__](/ti_wisunfan/html/wisun-guide/index.html) \
implements the Wi-SUN standard, a wireless communication standard aiming to \
enable large-scale IOT networks in a mesh structure. The network typically \
consists of line powered routers and gateways, and battery powered end \
nodes. The Wi-SUN FAN Specification is given \
by the [Wi-SUN Alliance](https://wi-sun.org/).
\n\
* [Overview](/ti_wisunfan/html/wisun-stack/wisun-stack-overview.html)
* [Examples](/ti_wisunfan/html/wisun-guide/example-applications.html)
`
};

exports = {
    lockProject: lockProject,
    genLibs: genLibs,
    project: project,
    mode: mode,
    secureBoot: secureBoot,
    ti_wisunfanModule: ti_wisunfanModule
};
