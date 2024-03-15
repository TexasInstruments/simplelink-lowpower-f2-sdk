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
 *  ======== ti154stack_toad_config.syscfg.js ========
 */

"use strict";

// Get TOAD setting descriptions
const docs = system.getScript("/ti/ti154stack/oad_config/toad_config/"
    + "ti154stack_toad_config_docs.js");

const toadReadOnlyStr = "Turbo OAD is not enabled for this project";
const CC13X2_CC26X2_MEMORY_MAX = 81920; // 80 KiB

// Configurables for the static TOAD settings group
const config = [
    {
        name: "enabled",
        displayName: "Enable Turbo OAD",
        default: false,
        description: docs.enabled.description,
        longDescription: docs.enabled.longDescription,
        onChange: onEnabledChanged
    },
    {
        name: "oldImgPath",
        displayName: "Old Image Path",
        default: "",
        readOnly: toadReadOnlyStr,
        fileFilter: ".bin",
        description: docs.oldImgPath.description,
        longDescription: docs.oldImgPath.longDescription
    },
    {
        name: "blockCacheSize",
        displayName: "OAD Block Cache Size (bytes)",
        default: 1024,
        readOnly: toadReadOnlyStr,
        description: docs.blockCacheSize.description,
        longDescription: docs.blockCacheSize.longDescription
    },
    {
        name: "writeBufferSize",
        displayName: "Write Buffer Size (bytes)",
        default: 4096,
        readOnly: toadReadOnlyStr,
        description: docs.writeBufferSize.description,
        longDescription: docs.writeBufferSize.longDescription
    }
];

/*
 * ======== onEnabledChanged ========
 * On change function for enabled config
 * Sets the module configurables to read only when not selected
 *
 * @param inst - TOAD module instance
 * @param ui   - user interface object
 */
function onEnabledChanged(inst, ui)
{
    if(inst.enabled)
    {
        ui.oldImgPath.readOnly = false;
        ui.blockCacheSize.readOnly = false;
        ui.writeBufferSize.readOnly = false;
    }
    else
    {
        // Revert to default values to prevent warnings
        inst.oldImgPath = "";
        inst.blockCacheSize = 1024;
        inst.writeBufferSize = 4096;

        ui.oldImgPath.readOnly = toadReadOnlyStr;
        ui.blockCacheSize.readOnly = toadReadOnlyStr;
        ui.writeBufferSize.readOnly = toadReadOnlyStr;
    }
}

/*
 * ======== validate ========
 * Validate this inst's configuration
 *
 * @param inst       - TOAD settings instance to be validated
 * @param validation - object to hold detected validation issues
 */
function validate(inst, validation)
{
    if(inst.enabled)
    {
        // oldImgPath validation
        if(inst.oldImgPath === "")
        {
            validation.logInfo("No path to image specified. Delta image "
                + "will not be created", inst, "oldImgPath");
        }

        // blockCacheSize validation
        if(!Number.isInteger(inst.blockCacheSize))
        {
            validation.logError("Must be a whole number", inst,
                "blockCacheSize");
        }

        if(inst.blockCacheSize <= 0
           || inst.blockCacheSize > CC13X2_CC26X2_MEMORY_MAX)
        {
            validation.logError(
                `Must be between 0 and ${CC13X2_CC26X2_MEMORY_MAX}`,
                inst, "blockCacheSize"
            );
        }

        // writeBufferSize validation
        if(!Number.isInteger(inst.writeBufferSize))
        {
            validation.logError("Must be a whole number", inst,
                "writeBufferSize");
        }

        if(inst.writeBufferSize <= 0
           || inst.writeBufferSize > CC13X2_CC26X2_MEMORY_MAX)
        {
            validation.logError(
                `Must be between 0 and ${CC13X2_CC26X2_MEMORY_MAX}`,
                inst, "writeBufferSize"
            );
        }
    }
}

/*
 *******************************************************************************
 Module Dependencies and Exports
 *******************************************************************************
 */

// Exports the Turbo OAD module
exports = {
    displayName: "Turbo OAD",
    description: docs.toadModule.description,
    longDescription: docs.toadModule.longDescription,
    config: config,
    templates: {
        "/ti/ti154stack/templates/ti_154stack_toad_config.h.xdt":
            "/ti/ti154stack/templates/ti_154stack_toad_config.h.xdt",
        "/ti/ti154stack/templates/ti_154stack_toad_config.json.xdt":
            "/ti/ti154stack/templates/ti_154stack_toad_config.json.xdt"
    },
    validate: validate
};
