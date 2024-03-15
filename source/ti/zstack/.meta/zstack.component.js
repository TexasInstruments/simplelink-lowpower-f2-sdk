/*
 * Copyright (c) 2018 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== zstack.component.js ========
 */

"use strict";

const deviceId = system.deviceData.deviceId;
const displayName = "Z-Stack";
let topModules = [];
let templates = [];

if(deviceId.match(/CC2651[RP]|CC2652[RP]|CC1352R1|CC2653P10|CC2654[RP]10|CC1354[RP]10|CC1352P[17]|CC2674[RP]/))
{
    let name = system.deviceData.deviceId;
    if(system.deviceData.board != null)
    {
        name = system.deviceData.board.source;
        /* Strip off everything up to and including the last '/' */
        name = name.replace(/.*\//, "");

        /* Strip off everything after and including the first '.' */
        name = name.replace(/\..*/, "");
    }
    if ((system.deviceData.board == null) || (system.deviceData.board != null && (!name.includes("1352P1")))) {
        topModules = [
            {
                displayName: "RF Stacks",
                description: "RF Stacks",
                modules: ["/ti/zstack/zstack"]
            }
        ];
        templates = [
            {
                name: "/ti/zstack/templates/ti_zstack_config.c.xdt",
                outputPath: "ti_zstack_config.c"
            },
            {
                name: "/ti/zstack/templates/ti_zstack_config.h.xdt",
                outputPath: "ti_zstack_config.h"
            },
            {
                name: "/ti/zstack/templates/gpd/ti_zstack_config.h.xdt",
                outputPath: "ti_zstack_config.h"
            },
	        {
	            name: "/ti/zstack/templates/zcl_config.c.xdt",
	            outputPath: "zcl_config.c"
	        },
	        {
	            name: "/ti/zstack/templates/zcl_config.h.xdt",
	            outputPath: "zcl_config.h"
	        },
        ];
    }
}

exports = {
    displayName: displayName,
    topModules: topModules,
    templates: templates
};
