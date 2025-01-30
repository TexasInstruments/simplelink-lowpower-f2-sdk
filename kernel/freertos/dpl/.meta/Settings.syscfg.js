/*
 * Copyright (c) 2022-2024, Texas Instruments Incorporated - http://www.ti.com
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
 * ======== Settings.syscfg.js ========
 * This module's main purpose is to provide a list of source files to include
 * in the ti_freertos_config.c file. The FreeRTOS module queries all modules
 * in the system for a getCFiles function.
 */

var dplFiles = [
    "dpl/AppHooks_freertos.c",
    "dpl/DebugP_freertos.c",
    "dpl/EventP_freertos.c",
    "dpl/MessageQueueP_freertos.c",
    "dpl/MutexP_freertos.c",
    "dpl/QueueP_freertos.c",
    "dpl/SemaphoreP_freertos.c",
    "dpl/StaticAllocs_freertos.c",
    "dpl/SwiP_freertos.c",
    "dpl/SystemP_freertos.c",
    "dpl/TaskP_freertos.c"
];

var cc13xxcc26xxDeviceFiles = [
    "dpl/ClockPCC26X2_freertos.c",
    "dpl/HwiPCC26X2_freertos.c",
    "dpl/PowerCC26X2_freertos.c",
    "dpl/TimerPCC26XX_freertos.c",
    "dpl/TimestampPCC26XX_freertos.c"
];

var cc23x0r5DeviceFiles = [
    "dpl/ClockPLPF3_freertos.c",
    "dpl/HwiPCC23X0_freertos.c",
    "dpl/PowerCC23X0_freertos.c",
    "dpl/TimestampPLPF3_freertos.c"
];

var cc23x0r2DeviceFiles = [
    "dpl/ClockPLPF3_freertos.c",
    "dpl/HwiPCC23X0_freertos.c",
    "dpl/PowerCC23X0_freertos.c",
    "dpl/TimestampPLPF3_freertos.c"
];

var cc27xxDeviceFiles = [
    "dpl/ClockPLPF3_freertos.c",
    "dpl/HwiPCC27XX_freertos.c",
    "dpl/PowerCC27XX_freertos.c",
    "dpl/TimestampPLPF3_freertos.c"
];

var cc35xxDeviceFiles = [
    "dpl/ClockPWFF3_freertos.c",
    "dpl/HwiPWFF3_freertos.c",
    "dpl/PowerWFF3_freertos.c",
    "dpl/TimestampPWFF3_freertos.c"
];

function getStartupFiles(family)
{
    var startupFile;
    if (system.modules["/ti/utils/TrustZone"]) {
        // TFM-enabled startup files have the suffix "_ns"
        startupFile = `startup/startup_${family}_${system.compiler}_ns.c`;
    }
    else {
        startupFile = `startup/startup_${family}_${system.compiler}.c`;
    }
    return [
        startupFile
    ];
}

function getCFiles(kernel)
{
    if (system.deviceData.deviceId.match(/CC(13|26).[12]/)) {
        return dplFiles.concat(cc13xxcc26xxDeviceFiles, getStartupFiles("cc13x2_cc26x2"));
    } else if (system.deviceData.deviceId.match(/CC(13|26).[34]/)) {
        return dplFiles.concat(cc13xxcc26xxDeviceFiles, getStartupFiles("cc13x4_cc26x4"));
    } else if (system.deviceData.deviceId.match(/CC23.0R22/)) {
        // cc2340r22 uses cc2340r5 driverlib
        return dplFiles.concat(cc23x0r5DeviceFiles, getStartupFiles("cc23x0r5"));
    } else if (system.deviceData.deviceId.match(/CC23.0R2/)) {
        return dplFiles.concat(cc23x0r2DeviceFiles, getStartupFiles("cc23x0r2"));
    } else if (system.deviceData.deviceId.match(/CC23.0R5/)) {
        return dplFiles.concat(cc23x0r5DeviceFiles, getStartupFiles("cc23x0r5"));
    } else if (system.deviceData.deviceId.match(/CC27../)) {
        return dplFiles.concat(cc27xxDeviceFiles, getStartupFiles("cc27xx"));
    } else if (system.deviceData.deviceId.match(/CC35../)) {
        return dplFiles.concat(cc35xxDeviceFiles, getStartupFiles("cc35xx"));
    } else {
        return dplFiles;
    }
}

exports = {
    staticOnly: true,
    displayName: "Settings",
    getCFiles: getCFiles,
    templates: {
        "/ti/utils/rov/syscfg_c.rov.xs.xdt": [
            "crov:/kernel/freertos/rov/clock.rov.js",
            "crov:/kernel/freertos/rov/exception.rov.js",
            "crov:/kernel/freertos/rov/hwi.rov.js",
            "crov:/kernel/freertos/rov/nvic.rov.js",
            "objView:/kernel/freertos/rov_theia/clock.rov.js",
            "objView:/kernel/freertos/rov_theia/exception.rov.js",
            "objView:/kernel/freertos/rov_theia/hwi.rov.js",
            "objView:/kernel/freertos/rov_theia/nvic.rov.js"

        ]
    }
};
