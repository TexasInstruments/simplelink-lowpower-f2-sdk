/* Copyright (c) 2023 Texas Instruments Incorporated - http://www.ti.com
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

let displayName = "FreeRTOS";
let description = "FreeRTOS Global Settings";

let topModules = [
    {
        displayName: displayName,
        description: description,
        "modules": [
            "/freertos/FreeRTOS",
            "/freertos/idleHooks"
        ]
    }
];

let templates = [
    {
        "name": "/freertos/ti_freertos_config.h.xdt",
        "outputPath": "FreeRTOSConfig.h",
        "alwaysRun": false
    },
    {
        "name": "/freertos/ti_freertos_config.c.xdt",
        "outputPath": "ti_freertos_config.c",
        "alwaysRun": false
    },
    {
        "name": "/freertos/ti_freertos_portable_config.c.xdt",
        "outputPath": "ti_freertos_portable_config.c",
        "alwaysRun": false
    }
];

let finalTopModules = [];

/*
 * If nortos or tirtos is configured, omit the FreeRTOS part of the list. Since
 * it is needed for compatibility and tooling, continue to display the modules
 * in the UI if RTOS is "none" - but if the user references the FreeRTOS module,
 * it will throw a validation error demanding the user switch to --rtos freertos.
 */
if (system.getRTOS() == "freertos" || system.getRTOS() == "none")
{
    finalTopModules = topModules;
}

exports = {
    displayName: "FreeRTOS",
    topModules: finalTopModules,
    templates: templates
};
