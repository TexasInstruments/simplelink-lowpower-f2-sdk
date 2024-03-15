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
 *  ======== zstack_routing.syscfg.js ========
 */

"use strict";

const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Description text for configurables */
const routeExpiryTimeDescription = `Number of seconds before an entry in the \
routing table is marked as expired. Set to 255 to turn off route expiry.`;

const routeExpiryTimeLongDescription = routeExpiryTimeDescription + `\n\n\
**Default:** 30 seconds

**Range:** 0 - 255 seconds`;

const nwkMsgTimeoutDescription = `Default indirect message holding timeout \
in seconds.`;

const nwkMsgTimeoutLongDescription = nwkMsgTimeoutDescription + `\n\n\
**Default:** 7 seconds

**Range:** 0 - 127 seconds`;

const maxRouteReqEntriesDescription = `Maximum number of simultaneous route \
discoveries in the network.`;

const maxRouteReqEntriesLongDescription = maxRouteReqEntriesDescription
+ `\n\n**Default:** 8

**Range:** 0 - 65535`;


/* Routing submodule for the zstack module */
const routingModule = {
    config: [
        {
            name: "routeExpiryTime",
            displayName: "Route Expiry Time (s)",
            description: routeExpiryTimeDescription,
            longDescription: routeExpiryTimeLongDescription,
            default: 30
        },
        {
            name: "nwkIndirectMsgTimeout",
            displayName: "Network Indirect Message Timeout (s)",
            description: nwkMsgTimeoutDescription,
            longDescription: nwkMsgTimeoutLongDescription,
            default: 7
        },
        {
            name: "maxRouteReqEntries",
            displayName: "Max Route Request Entries",
            description: maxRouteReqEntriesDescription,
            longDescription: maxRouteReqEntriesLongDescription,
            default: 8
        }
    ],
    validate: validate
};

/* Validation function for the routing submodule */
function validate(inst, validation)
{
    /* Validate Route Expiry Time */
    Common.validateRange(inst, validation, inst.routeExpiryTime,
        "routeExpiryTime", "Route Expiry Time", 0, 255);

    /* Info for turning off Route Expiry Feature (255) */
    if(inst.routeExpiryTime === 255)
    {
        validation.logInfo(
            "Setting Route Expiry Time to 255 will disable the route expiry "
            + "feature.", inst, "routeExpiryTime"
        );
    }

    /* Validate Network Indirect Message Timeout */
    Common.validateRange(inst, validation, inst.nwkIndirectMsgTimeout,
        "nwkIndirectMsgTimeout", "Network Indirect Message Timeout", 1, 127);

    /* Validate Max Route Request Entries */
    Common.validateRange(inst, validation, inst.maxRouteReqEntries,
        "maxRouteReqEntries", "Max Route Request Entries", 1, 65535);
}

exports = routingModule;
