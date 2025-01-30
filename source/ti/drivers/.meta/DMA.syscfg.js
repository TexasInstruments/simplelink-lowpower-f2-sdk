/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== DMA.syscfg.js ========
 */

"use strict";

/* get Common /ti/drivers utility functions */
let Common      = system.getScript("/ti/drivers/Common.js");

/* get /ti/drivers family name from device object */
let family      = Common.device2Family(system.deviceData, "DMA");

/*
 *  ======== base ========
 *  Define the base DMA properties and methods
 */
let base = {
    displayName : "DMA",
    description : "On-chip DMA resource allocation",
    defaultInstanceName : "CONFIG_DMA_",
    alwaysShowLongDescription : true,
    longDescription : `
The DMA module provides support to other drivers that
need to allocate exclusive access to a DMA resource to support
CPU-less peripheral data transfers. There is no user API for this module.
See [__Driver configurations reference__][1] for more information.

[1]: /drivers/syscfg/html/ConfigDoc.html#DMA_Configuration_Options "Configuration options reference"
`,
    maxInstances : 1,
    moduleStatic : {
        modules: Common.autoForceModules(["Board"])
    }
};

/* extend the base exports to include family-specific content */
let devDMA;
if (family == "WFF3") {
    devDMA = system.getScript("/ti/drivers/dma/DMA" + family);
} else {
    devDMA = system.getScript("/ti/drivers/dma/UDMA" + family);
}
exports = devDMA.extend(base);
