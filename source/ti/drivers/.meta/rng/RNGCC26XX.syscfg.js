/*
 * Copyright (c) 2021 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== RNGCC26XX.syscfg.js ========
 */

"use strict";

/* get Common /ti/drivers utility functions */
let Common = system.getScript("/ti/drivers/Common.js");

let intPriority = Common.newIntPri()[0];
intPriority.name = "interruptPriority";
intPriority.displayName = "TRNG Interrupt Priority";
intPriority.description = "TRNG peripheral interrupt priority, only used in "
                        + "callback mode.";

let logError = Common.logError;
let logInfo  = Common.logInfo;

/*
 *  ======== devSpecific ========
 *  Device-specific extensions to be added to base RNG configuration
 */
let devSpecific = {
    moduleStatic: {
        name: "rngSettings",
        displayName: "Settings for RNG",
        config: [
            {
                name        : "rngPoolSize",
                displayName : "RNG Entropy Pool Size",
                description : 'Size of entropy pool kept in memory by the RNG '
                            + 'driver for faster responses to entropy '
                            + 'requests.',
                default     : 32
            },
            intPriority,
            {
                name        : "entropyGenerationCycles",
                displayName : "TRNG Entropy Generation Cycles",
                description : "Clock cycles used for entropy generation by TRNG. Value lower than default is not recommended.",
                longDescription: `
Default value (240000) configures TRNG to generate 64 bits of entropy in 5ms with all FROs active.
Selection of smaller values than default will result in lower quality entropy and is not recommended.
`,
                default     : 240000
            }
        ]
    },

    config: [],

    templates : {
        boardc: "/ti/drivers/rng/RNGCC26XX.Board.c.xdt",
        boardh: "/ti/drivers/rng/RNG.Board.h.xdt"
    }
};

function validate_trng_settings(inst, validation) {
    let RNG = system.modules["/ti/drivers/RNG"];

    if (RNG.$static.entropyGenerationCycles < 256) {
        logError(validation, RNG.$static, "entropyGenerationCycles",
                 "value must be greater than 256");
    }

    if (RNG.$static.entropyGenerationCycles > 16777216) {
        logError(validation, RNG.$static, "entropyGenerationCycles",
                 "value must be less than 16777216 (2^24)");
    }
}

function validate_pool_settings(inst, validation) {
    let RNG = system.modules["/ti/drivers/RNG"];

    if (RNG.$static.rngPoolSize < 0) {
        logError(validation, RNG.$static, "rngPoolSize",
                 "value must be positive");
    }
    else {
        if (RNG.$static.rngPoolSize % 16 != 0) {
            logError(validation, RNG.$static, "rngPoolSize",
                     "value must be a multiple of 16");
        }

        if (RNG.$static.rngPoolSize < 32) {
            logInfo(validation, RNG.$static, "rngPoolSize",
                    "consider using a larger value for better performance");
        }
    }
}

function validate(inst, validation, $super) {
    validate_trng_settings(inst, validation);
    validate_pool_settings(inst, validation);

    if ($super.validate) {
        $super.validate(inst, validation);
    }
}

/*
 *  ======== extend ========
 */
function extend(base)
{
    /* display which driver implementation can be used */
    base = Common.addImplementationConfig(base, "RNG", null,
        [{name: "RNGCC26XX"}], null);

    /* override base validate */
    devSpecific.validate = function (inst, validation) {
        return validate(inst, validation, base);
    };

    /* merge and overwrite base module attributes */
    let result = Object.assign({}, base, devSpecific);

    /* concatenate device-specific configs */
    result.config = base.config.concat(devSpecific.config);

    return (result);
}

/*
 *  ======== exports ========
 *  Export device-specific extensions to base exports
 */
exports = {
    /* required function, called by base RNG module */
    extend: extend
};
