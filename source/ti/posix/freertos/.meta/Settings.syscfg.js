/*
 * Copyright (c) 2022, Texas Instruments Incorporated - http://www.ti.com
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

function getCFiles()
{
    let posixFiles = [
        "ti/posix/freertos/clock.c",
        "ti/posix/freertos/memory.c",
        "ti/posix/freertos/mqueue.c",
        "ti/posix/freertos/pthread_barrier.c",
        "ti/posix/freertos/pthread.c",
        "ti/posix/freertos/pthread_cond.c",
        "ti/posix/freertos/pthread_mutex.c",
        "ti/posix/freertos/pthread_rwlock.c",
        "ti/posix/freertos/sched.c",
        "ti/posix/freertos/semaphore.c",
        "ti/posix/freertos/sleep.c",
        "ti/posix/freertos/timer.c"
    ];

    if (system.compiler == "iar") {
        posixFiles.push("ti/posix/freertos/Mtx.c");
    } else if (system.compiler == "ticlang") {
        posixFiles.push(
            "ti/posix/freertos/PTLS.c",
            "ti/posix/freertos/aeabi_portable.c"
        );
    }

    return posixFiles;
}

/* return string of C-syntax preprocessor commands */
function getCSrcPrelude() {
    let mod = system.modules["/ti/posix/freertos/Settings"].$static;

    if (mod.enableAdvMem) {
        return "#define TI_POSIX_FREERTOS_MEMORY_ENABLEADV";
    }
    else {
        return "";
    }
}

exports = {
    staticOnly: true,
    displayName: "POSIX Settings",
    moduleStatic: {
        config: [
            {
                name: "enableAdvMem",
                displayName: "Enable Advanced Memory Features",
                description: "Enable realloc() and memalign() services",
                longDescription: `
Enable advanced memory features, specifically realloc() and memalign(), with
added footprint cost.

Enabling this increases the runtime cost of each malloc-based allocation, as
well as lightly increases the code footprint.`,
                default: true
            }
        ]
    },
    getCFiles: getCFiles,
    getCSrcPrelude: getCSrcPrelude
};
