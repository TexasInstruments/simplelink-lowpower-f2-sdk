/*
 * Copyright (c) 2018, Texas Instruments Incorporated
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
 */

/*
 *  ======== zstack_gpd_advanced.syscfg.js ========
 */

"use strict";

/* Description text for configurables */
const gpdfRetriesDescription = `The number of Green Power Data Frame retries \
per packet. This is recommended to be low for reduced power consumption.`;

const gpdfRetriesLongDescription = gpdfRetriesDescription + `\n\n\
**Default:** 3

**Range:** Any 8 bit number (hexidecimal format)`;

const autoCommissioningDescription = `Specify if the device will \
automatically start commissioning on power-up.`;

const autoCommissioningLongDescription = autoCommissioningDescription + `\n\n\
**Default:** Depends on GPD project selected`;

const rxAfterTxDescription = `Specify if the device will enable radio receive \
after transmitting a frame (whether the device is expecting a response.)`;

const rxAfterTxLongDescription = rxAfterTxDescription + `\n\n\
**Default:** Radio receive not enabled after transmit (unchecked)`;

const seqNumCapDescription = `Specify whether the device will use incremental \
sequence numbers (checked) or random sequence numbers (unchecked).`;

const seqNumCapLongDescription = seqNumCapDescription + `\n\n\
**Default:** Random sequence numbers (unchecked)`;


/* GPD advanced submodule for zstack module */
const gpdAdvancedModule = {
    config: [
        {
            name: "gpdfRetries",
            displayName: "Data Frame Retries",
            description: gpdfRetriesDescription,
            longDescription: gpdfRetriesLongDescription,
            default: 3
        },
        {
            name: "autoCommissioning",
            displayName: "Auto-commissioning",
            description: autoCommissioningDescription,
            longDescription: autoCommissioningLongDescription,
            default: false
        },
        {
            name: "rxAfterTx",
            displayName: "Radio Receive After Transmit",
            description: rxAfterTxDescription,
            longDescription: rxAfterTxLongDescription,
            default: false
        },
        {
            name: "seqNumCap",
            displayName: "Sequence Number Capability",
            description: seqNumCapDescription,
            longDescription: seqNumCapLongDescription,
            default: false
        }
    ],
    validate: validate
};

/* Validation function for the GPD advanced submodule */
function validate(inst, validation)
{
    /* Validate GPDF Retries */
    if(inst.gpdfRetries < 0 || inst.gpdfRetries > 255)
    {
        validation.logError(
            "Data Frame Retries must be between 0 and 255", inst,
            "gpdfRetries"
        );
    }
}

exports = gpdAdvancedModule;
