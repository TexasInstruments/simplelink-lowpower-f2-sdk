/*
 * Copyright (c) 2017-2018 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== ti154stack.component.js ========
 */

const categoryName = "RF Stacks";
let topModules = [];
let templates = [];

/* Only export 15.4 module if device is supported */
if(system.deviceData.deviceId.match(/CC2652|CC13.2(R|P)/))
{
    topModules = [
        {
            displayName: categoryName,
            description: categoryName,
            modules: ["/ti/ti154stack/ti154stack"]
        }
    ];
    templates = [
        {
            name: "/ti/ti154stack/templates/ti_154stack_config_wisun.h.xdt",
            outputPath: "ti_154stack_config.h"
        },
        {
            name: "/ti/ti154stack/templates/ti_154stack_oad_config_wisun.h.xdt",
            outputPath: "ti_154stack_oad_config.h"
        },
        {
            name: "/ti/ti154stack/templates/ti_154stack_features_wisun.h.xdt",
            outputPath: "ti_154stack_features.h"
        },
        {
            name: "/ti/ti154stack/templates/ti_154stack_toad_config_wisun.h.xdt",
            outputPath: "ti_154stack_toad_config.h"
        },
        {
            name: "/ti/ti154stack/templates/"
            + "ti_154stack_toad_build_config.opts_wisun.xdt",
            outputPath: "ti_154stack_toad_build_config.opts"
        },
        {
            name: "/ti/ti154stack/templates/ti_154stack_toad_config_wisun.json.xdt",
            outputPath: "ti_154stack_toad_config.json"
        }
    ];
}

exports = {
    displayName: "TI 15.4 Stack",
    topModules: topModules,
    templates: templates
};
