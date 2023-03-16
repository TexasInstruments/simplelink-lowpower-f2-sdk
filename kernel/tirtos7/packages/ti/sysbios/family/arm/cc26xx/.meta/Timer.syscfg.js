/*
 * Copyright (c) 2020-2023, Texas Instruments Incorporated - http://www.ti.com
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
 * ======== Timer.syscfg.js ========
 */

/* global exports, system */

/*
 *  ======== pinmuxRequirements ========
 *  Return peripheral pin requirements as a function of config
 */
function pinmuxRequirements(inst)
{
    let timer = {
        name          : "rtc",
        displayName   : "RTC",
        interfaceName : "RTC",
        resources     : []
    };

    return [timer];
}

/* use bigInt for zero padding hex numbers */
let bigInt = system.utils.bigInt;

exports = {
    staticOnly: true,
    displayName: "RTC Timer",
    moduleStatic: {
        name: "moduleGlobal",
        pinmuxRequirements: pinmuxRequirements,
        config: [
            {
                name: "funcHookCh1",
                displayName: "RTC Channel 1 Function Hook",
                description: `Function hook for RTC channel 1`,
                longDescription: `
The RTC on CC13XX/CC26XX devices has multiple channels but only a combined
interrupt vector. Since the timer module owns this interrupt vector, function
hooks are provided to permit the use of the other channels.

This function hook will be called when the RTC triggers on channel 1. It is
executed from interrupt context. It must thus obey the usual restrictions for
running from interrupt context.
                `,
                default: "NULL",
            },
            {
                name: "funcHookCh2",
                displayName: "RTC Channel 2 Function Hook",
                description: `Function hook for RTC channel 2`,
                longDescription: `
The RTC on CC13XX/CC26XX devices has multiple channels but only a combined
interrupt vector. Since the timer module owns this interrupt vector, function
hooks are provided to permit the use of the other channels.

This function hook will be called when the RTC triggers on channel 2. It is
executed from interrupt context. It must thus obey the usual restrictions for
running from interrupt context.
                `,
                default: "NULL",
            },
        ]
    },
    templates: {
        bios_config_h: "/ti/sysbios/family/arm/cc26xx/Timer_config.h.xdt",
        "/ti/utils/rov/syscfg_c.rov.xs.xdt": "/ti/sysbios/family/arm/cc26xx/Timer.rov.js"
    },
    getCFiles: () => { return ["ti/sysbios/family/arm/cc26xx/Timer.c"] }
};
