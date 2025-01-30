/*
 * Copyright (c) 2022-2024 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== Settings.js ========
 */

let deviceSettingsTable = {
    "cc13x1_cc26x1": {
        defaultHeapSize: 0x2000,
        defaultCpuFrequency: 48000000,
        defaultIdleSleepTicks: 2,
        defaultPortTaskSelection: 1,
        defaultMaxInterruptPriority: 1,
        defaultNvicPriBits: 3,
        defaultFpuEnabled: false,
        gccPortableFiles: ["../portable/GCC/ARM_CM3/port.c"],
        iarPortableFiles: ["../portable/IAR/ARM_CM3/port.c"]
    },
    "cc13x2_cc26x2": {
        defaultHeapSize: 0x8000,
        defaultCpuFrequency: 48000000,
        defaultIdleSleepTicks: 2,
        defaultPortTaskSelection: 1,
        defaultMaxInterruptPriority: 1,
        defaultNvicPriBits: 3,
        defaultFpuEnabled: true,
        gccPortableFiles: ["../portable/GCC/ARM_CM4F/port.c"],
        iarPortableFiles: ["../portable/IAR/ARM_CM4F/port.c"]
    },
    "cc13x4_cc26x4": {
        defaultHeapSize: 0x8000,
        defaultCpuFrequency: 48000000,
        defaultIdleSleepTicks: 2,
        /* The CM33 port does not have optimised task selection */
        defaultPortTaskSelection: 0,
        defaultMaxInterruptPriority: 1,
        defaultNvicPriBits: 3,
        defaultFpuEnabled: true,
        gccPortableFiles: [
            "../portable/GCC/ARM_CM33_NTZ/non_secure/port.c",
            "../portable/GCC/ARM_CM33_NTZ/non_secure/portasm.c"
        ],
        iarPortableFiles: [
            "../portable/IAR/ARM_CM33_NTZ/non_secure/port.c"
        ]
    },
    "cc13x4_cc26x4_tfm": {
        defaultHeapSize: 0x8000,
        defaultCpuFrequency: 48000000,
        defaultIdleSleepTicks: 2,
        /* The CM33 port does not have optimised task selection */
        defaultPortTaskSelection: 0,
        defaultMaxInterruptPriority: 2,
        defaultNvicPriBits: 3,
        defaultFpuEnabled: true,
        gccPortableFiles: [
            "../portable/GCC/ARM_CM33_NTZ/non_secure/port.c",
            "../portable/GCC/ARM_CM33_NTZ/non_secure/portasm.c"
        ],
        iarPortableFiles: [
            "../portable/IAR/ARM_CM33_NTZ/non_secure/port.c"
        ]
    },
    "cc23x0": {
        defaultHeapSize: 0x2000,
        defaultCpuFrequency: 48000000,
        defaultIdleSleepTicks: 2,
        defaultPortTaskSelection: 0,
        defaultMaxInterruptPriority: 1,
        defaultNvicPriBits: 2,
        defaultFpuEnabled: false,
        gccPortableFiles: ["../portable/GCC/ARM_CM0/port.c"],
        iarPortableFiles: ["../portable/IAR/ARM_CM0/port.c"]
    },
    "cc35xx": {
        defaultHeapSize: 0x8000,
        defaultCpuFrequency: 160000000,
        defaultIdleSleepTicks: 2,
        /* The CM33 port does not have optimised task selection */
        defaultPortTaskSelection: 0,
        defaultMaxInterruptPriority: 1,
        defaultNvicPriBits: 3,
        defaultFpuEnabled: true,
        gccPortableFiles: [
            "../portable/GCC/ARM_CM33_NTZ/non_secure/port.c",
            "../portable/GCC/ARM_CM33_NTZ/non_secure/portasm.c"
        ],
        iarPortableFiles: [
            "../portable/IAR/ARM_CM33_NTZ/non_secure/port.c"
        ]
    },
    "cc35xx_tfm": {
        defaultHeapSize: 0x8000,
        defaultCpuFrequency: 160000000,
        defaultIdleSleepTicks: 2,
        /* The CM33 port does not have optimised task selection */
        defaultPortTaskSelection: 0,
        defaultMaxInterruptPriority: 2,
        defaultNvicPriBits: 3,
        defaultFpuEnabled: true,
        gccPortableFiles: [
            "../portable/GCC/ARM_CM33_NTZ/non_secure/port.c",
            "../portable/GCC/ARM_CM33_NTZ/non_secure/portasm.c"
        ],
        iarPortableFiles: [
            "../portable/IAR/ARM_CM33_NTZ/non_secure/port.c"
        ]
    },
    "cc27xx": {
        defaultHeapSize: 0x8000,
        defaultCpuFrequency: 96000000,
        defaultIdleSleepTicks: 2,
        /* The CM33 port does not have optimised task selection */
        defaultPortTaskSelection: 0,
        defaultMaxInterruptPriority: 1,
        defaultNvicPriBits: 4,
        defaultFpuEnabled: true,
        gccPortableFiles: [
            "../portable/GCC/ARM_CM33_NTZ/non_secure/port.c",
            "../portable/GCC/ARM_CM33_NTZ/non_secure/portasm.c"
        ],
        iarPortableFiles: [
            "../portable/IAR/ARM_CM33_NTZ/non_secure/port.c"
        ]
    },
    "cc27xx_tfm": {
        defaultHeapSize: 0x8000,
        defaultCpuFrequency: 96000000,
        defaultIdleSleepTicks: 2,
        /* The CM33 port does not have optimised task selection */
        defaultPortTaskSelection: 0,
        defaultMaxInterruptPriority: 2,
        defaultNvicPriBits: 4,
        defaultFpuEnabled: true,
        gccPortableFiles: [
            "../portable/GCC/ARM_CM33_NTZ/non_secure/port.c",
            "../portable/GCC/ARM_CM33_NTZ/non_secure/portasm.c"
        ],
        iarPortableFiles: [
            "../portable/IAR/ARM_CM33_NTZ/non_secure/port.c"
        ]
    }
};
let deviceId = system.deviceData.deviceId;

let deviceSettings;

if (deviceId.match(/CC23.0/)) {
    deviceSettings = deviceSettingsTable.cc23x0;
}
else if (deviceId.match(/CC13.1.*|CC26.1.*/)) {
    /* CC13X1/CC26X1 */
    deviceSettings = deviceSettingsTable.cc13x1_cc26x1;
}
else if (deviceId.match(/CC13.2.*|CC26.2.*/)) {
    /* CC13X2/CC26X2 */
    deviceSettings = deviceSettingsTable.cc13x2_cc26x2;
}
else if (deviceId.match(/CC13.4.*|CC26.4.*|CC2653.*/)) {
    /* CC13X4/CC26X4. Choose between a TFM-enabled or TFM-disabled
     * configuration
     */
    if (system.modules["/ti/utils/TrustZone"]) {
        deviceSettings = deviceSettingsTable.cc13x4_cc26x4_tfm;
    }
    else {
        deviceSettings = deviceSettingsTable.cc13x4_cc26x4;
    }
}
else if (deviceId.match(/CC35.*/)) {
    /* CC35XX. Choose between a TFM-enabled or TFM-disabled
     * configuration
     */
    if (system.modules["/ti/utils/TrustZone"]) {
        deviceSettings = deviceSettingsTable.cc35xx_tfm;
    }
    else {
        deviceSettings = deviceSettingsTable.cc35xx;
    }
}
else if (deviceId.match(/CC27.*/)) {
    /* CC27XX. Choose between a TFM-enabled or TFM-disabled
     * configuration
     */
    if (system.modules["/ti/utils/TrustZone"]) {
        deviceSettings = deviceSettingsTable.cc27xx_tfm;
    }
    else {
        deviceSettings = deviceSettingsTable.cc27xx;
    }
}

exports = {
    defaultHeapSize: deviceSettings.defaultHeapSize,
    defaultCpuFrequency: deviceSettings.defaultCpuFrequency,
    defaultIdleSleepTicks: deviceSettings.defaultIdleSleepTicks,
    defaultPortTaskSelection: deviceSettings.defaultPortTaskSelection,
    defaultMaxInterruptPriority: deviceSettings.defaultMaxInterruptPriority,
    defaultNvicPriBits: deviceSettings.defaultNvicPriBits,
    defaultFpuEnabled: deviceSettings.defaultFpuEnabled,
    gccPortableFiles: deviceSettings.gccPortableFiles,
    gccPortableFilesTzEnabled: deviceSettings.gccPortableFilesTzEnabled,
    iarPortableFiles: deviceSettings.iarPortableFiles,
    iarPortableFilesTzEnabled: deviceSettings.iarPortableFilesTzEnabled
};
