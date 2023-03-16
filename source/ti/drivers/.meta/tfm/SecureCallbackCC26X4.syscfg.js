/*
 * Copyright (c) 2022 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== SecureCallbackCC26X4.syscfg.js ========
 */

"use strict";

/* get Common /ti/drivers utility functions */
let Common   = system.getScript("/ti/drivers/Common.js");


let intPriority = Common.newIntPri()[0];
intPriority.name = "interruptPriority";
intPriority.displayName = "Interrupt Priority";
intPriority.description = "Secure Callback interrupt priority";

/* Array of configurables for this device family */
let config = [
    {
        name        : "intLine",
        displayName : "Interrupt Line",
        description : "Which interrupt line secure callbacks should reserve",
        longDescription:`
The callback mechanism requires an interrupt to switch between secure and
non-secure contexts. Assign an interrupt line that is not in use elsewhere in
the system.
Changing this value from its default requires rebuilding the TFM image.
`,
        default     : 59,
        options: [
            { name: 43, displayName: "Software event 0 (IRQ 43)" },
            { name: 59, displayName: "Software event 1 (IRQ 59)" },
            { name: 60, displayName: "Software event 2 (IRQ 60)" },
            { name: 61, displayName: "Software event 3 (IRQ 61)" },
            { name: 62, displayName: "Software event 4 (IRQ 62)" }
        ]
    },
    intPriority
];

/*
 *  ======== devSpecific ========
 *  Device-specific extensions to be added to base configuration
 */
let devSpecific = {
    moduleStatic: {
        config
    },
    templates: {
        boardc: "/ti/drivers/tfm/SecureCallback.Board.c.xdt"
    }
};

/*
 *  ======== extend ========
 *  Extends a base exports object to include any device specifics
 *
 *  This function is invoked by the generic module to
 *  allow us to augment/override as needed for CC26X4
 */
function extend(base)
{
    /* display which driver implementation can be used */
    devSpecific = Common.addImplementationConfig(devSpecific, "SecureCallback", null, [{name: "SecureCallbackCC26X4"}], null);
    return (Object.assign({}, base, devSpecific));
}

/*
 *  ======== exports ========
 *  Export device-specific extensions to base exports
 */
exports = {
    extend: extend
};
