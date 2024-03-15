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
 *  ======== zstack_packet_sending.syscfg.js ========
 */

"use strict";

const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Description text for configurables */
const apsAckWaitDescription = `Number of milliseconds a device will wait for \
an APS ACK before resending the APS frame.`;

const apsAckWaitLongDescription = apsAckWaitDescription + `\n\n\
**Default:** 6000 ms

**Range:** 0 - 65535 ms`;

const macFrameRetriesDescription = `Maximum number of MAC frame transmission \
retries.`;

const macFrameRetriesLongDescription = macFrameRetriesDescription + `\n\n\
**Default:** 3

**Range:** 0 - 7`;

const apsRetriesDescription = `Maximum number of APS Retries after a missed APS ACK`;

const apsRetriesLongDescription = apsRetriesDescription + `\n\n\
**Default:** 3

**Range:** 0 - 127`;

const nwkDataRetriesDescription = `Maximum number of network data retries.`;

const nwkDataRetriesLongDescription = nwkDataRetriesDescription + `\n\n\
**Default:** 2

**Range:** 0 - 7`;

/* Packet Sending submodule for the zstack module */
const packetSendingModule = {
    config: [
        {
            name: "macFrameRetries",
            displayName: "MAC Frame Retries",
            description: macFrameRetriesDescription,
            longDescription: macFrameRetriesLongDescription,
            default: 3
        },
        {
            name: "nwkDataRetries",
            displayName: "NWK Data Retries",
            description: nwkDataRetriesDescription,
            longDescription: nwkDataRetriesLongDescription,
            default: 2
        },
        {
            name: "apscMaxFrameRetries",
            displayName: "APS Retries",
            description: apsRetriesDescription,
            longDescription: apsRetriesLongDescription,
            default: 3
        },
        {
            name: "apsAckWaitDurationPolled",
            displayName: "APS ACK Wait Duration (ms)",
            description: apsAckWaitDescription,
            longDescription: apsAckWaitLongDescription,
            default: 6000
        },
    ],
    validate: validate
};

/* Validation function for the Packet Sending submodule */
function validate(inst, validation)
{
    /* Validate APS Ack Wait Duration */
    Common.validateRange(inst, validation, inst.apsAckWaitDurationPolled,
        "apsAckWaitDurationPolled", "APS ACK Wait Duration", 0, 65535);

    /* APS Ack Wait Duration must be a multiple of 2 ms */
    if(inst.apsAckWaitDurationPolled % 2 !== 0)
    {
        validation.logError(
            "APS ACK Wait Duration must be a multiple of 2 milliseconds",
            inst, "apsAckWaitDurationPolled"
        );
    }

    /* Validate MAC Frame Attempts */
    Common.validateRange(inst, validation, inst.macFrameRetries,
        "macFrameRetries", "MAC Frame Retries", 0, 7);

    /* Validate MAC Frame Attempts */
    Common.validateRange(inst, validation, inst.apscMaxFrameRetries,
        "macFrameRetries", "MAC Frame Retries", 0, 127);

    /* Validate NWK Data Retries */
    Common.validateRange(inst, validation, inst.nwkDataRetries,
        "nwkDataRetries", "NWK Data Retries", 0, 7);
}

exports = packetSendingModule;
