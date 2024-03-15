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
 *  ======== zstack_table_size.syscfg.js ========
 */

"use strict";

const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Description text for configurables */
const groupTableDescription = `Maximum number of entries in the group table.`;

const groupTableLongDescription = groupTableDescription + `\n\n\
**Default:** 16

**Range:** 0 - 127`;

const routingTableDescription = `Maximum number of entries in the regular \
routing table plus additional entries for route repair.`;

const routingTableLongDescription = routingTableDescription + `\n\n\
**Default:** 40

**Range:** 0 - 65535`;

const bindingTableDescription = `Maximum number of entries in the binding \
table.`;

const bindingTableLongDescription = bindingTableDescription + `\n\n\
**Default:** 4

**Range:** 0 - 65535`;


/* Table Size submodule for the zstack module */
const tableSizeModule = {
    config: [
        {
            name: "groupTableSize",
            displayName: "Group Table Size",
            description: groupTableDescription,
            longDescription: groupTableLongDescription,
            default: 16
        },
        {
            name: "routingTableSize",
            displayName: "Routing Table Size",
            description: routingTableDescription,
            longDescription: routingTableLongDescription,
            default: 40
        },
        {
            name: "bindingTableSize",
            displayName: "Binding Table Size",
            description: bindingTableDescription,
            longDescription: bindingTableLongDescription,
            default: 4
        }
    ],
    validate: validate
};

/* Validation function for the Table Size submodule */
function validate(inst, validation)
{
    /* Validate Group Table Size */
    Common.validateRange(inst, validation, inst.groupTableSize,
        "groupTableSize", "Group Table Size", 1, 127);

    /* Validate Routing Table Size */
    Common.validateRange(inst, validation, inst.routingTableSize,
        "routingTableSize", "Routing Table Size", 1, 65535);

    /* Validate Binding Table Size */
    Common.validateRange(inst, validation, inst.bindingTableSize,
        "bindingTableSize", "Binding Table Size", 1, 65535);
}

exports = tableSizeModule;
