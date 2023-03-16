/* Copyright (c) 2022 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== FreeRTOS.syscfg.js ========
 */


"use strict";

/* get device specific Settings */
let Settings = system.getScript("/freertos/Settings.syscfg.js");

/*
 * ======== getLibs ========
 */
function getLibs(mod)
{
    let GenLibs = system.getScript("/ti/utils/build/GenLibs.syscfg.js");
    var toolchain = GenLibs.getToolchainDir();
    var isa = GenLibs.getDeviceIsa();

    var lib_base_name = "freertos/lib/" + toolchain + "/" + isa + "/";
    var link_info = {
        name: "freertos",
        deps: [],
        libs: []
    };

    if (toolchain == "iar") {
        if (system.modules["/ti/utils/TrustZone"]) {
            link_info.libs.push(lib_base_name + "freertos_tfm.a");
        }
        else {
            link_info.libs.push(lib_base_name + "freertos.a");
        }
    }

    return link_info;
}

/*
 *  ======== validate ========
 */
function validate(mod, validation)
{
    if (system.getRTOS() != "freertos") {
        validation.logError("Please configure sysconfig with --rtos freertos to use this module!", mod);
    }

    if (mod.idleStackSize % 4 != 0) {
        validation.logError("Stack size must be an integer number of words", mod, "idleStackSize");
    }
    if (mod.timerStackSize % 4 != 0) {
        validation.logError("Stack size must be an integer number of words", mod, "timerStackSize");
    }
    if (mod.posixThreadStackSize % 4 != 0) {
        validation.logError("Stack size must be an integer number of words", mod, "posixThreadStackSize");
    }

    if (mod.rovQueueEnabled && mod.queueRegistrySize == 0) {
        validation.logError("The Queue registry size may not be zero if extended kernel object decoding is enabled", mod, "queueRegistrySize");
    }
}

function getCFiles(kernel)
{
    let baseFiles = [
        /* FreeRTOS source files. Note that typically only an include path to
         * FreeRTOS/Source/include is provided. To try and minimise naming
         * conflicts, we go up to FreeRTOS, so we at least know these files are
         * in a 'Source' folder.
         */
        "../../Source/list.c",
        "../../Source/queue.c",
        "../../Source/tasks.c",
        "../../Source/timers.c",
        "../../Source/croutine.c",
        "../../Source/event_groups.c",
        "../../Source/stream_buffer.c"
    ];
    if (!kernel.useCustomHeap) {
        baseFiles.push("../../Source/portable/MemMang/heap_4.c");
    }

    return baseFiles;
}

function getPortableFiles()
{
    if (system.compiler == "iar") {
            return Settings.iarPortableFiles;
    }
    else {
            return Settings.gccPortableFiles;
    }
}

/*
 *  ======== base ========
 */
let base = {
    staticOnly: true,
    displayName: "FreeRTOS",
    moduleStatic: {
        name: "moduleGlobal",
        validate: validate,
        config: [
            {
                name: "stackOverflowMode",
                displayName: "Stack Overflow Checking",
                longDescription: `
Configure how FreeRTOS checks for a stack overflow. The default 16-byte sentinel mode effectively reduces all stack
sizes by 16 bytes, as this memory must remain untouched during normal operation. See the FreeRTOS documentation
on Stack Overflow Protection for more information.`,
                options: [
                    { name: 0, displayName: "Disabled" },
                    { name: 1, displayName: "Validate at task switch" },
                    { name: 2, displayName: "16-byte sentinel value" }
                ],
                default: 2
            },
            {
                name: "useTimers",
                displayName: "Enable Software Timers",
                description: `Enable or disable FreeRTOS Software Timers`,
                longDescription: `
When set to false, software timers are disabled and not available. See the FreeRTOS documentation on Software Timers
for more information.`,
                default: true
            },
            {
                name: "assertsEnabled",
                displayName: "Enable Asserts",
                description: `Enable FreeRTOS configASSERT macro`,
                longDescription: `
When set to false, assert calls will not have any effect. This option can improve runtime performance
as well as reduce the application's code size. This assert macro disables interrupts and spins forever.`,
                default: true
            },
            {
                name: "isrStackInitEnabled",
                displayName: "Enable ISR Stack Tracking",
                description: `Initialize ISR stack to a known value during startup to track with ROV`,
                longDescription: `
Initialize the ISR stack to a known value at startup. This enables ROV to monitor stack usage at runtime for the ISR
stack`,
                default: true
            },
            {
                name: "rovQueueEnabled",
                displayName: "Enable Kernel Object Tracking",
                longDescription: `Enable debugging and inspection of semaphores, mutexes, and gates using ROV.

The FreeRTOS kernel does not keep track of kernel objects based on the FreeRTOS Queue
at runtime. This means that debugging tools such as ROV cannot acquire a list of these kernel objects by default.
FreeRTOS has added a mechanism that optionally registers the address of each of these kernel objects in a separate
array by calling a utility function.

A set of wrapper functions are generated in ti_freertos_config.c with implementations depending on this setting. This
allows applications to toggle this setting and recompile to enable or disable this functionality as needed without
changing library or application code.

These FreeRTOS utilities or the generated wrappers must be called by the application for each kernel object the
application wishes to track and decode.
TI code calls the tracking wrapper functions that will register the kernel object or do nothing depending on this
setting.`,
                default: false,
                onChange: (inst, ui) => {
                    ui.queueRegistrySize.hidden = !(inst.rovQueueEnabled);
                }
            },
            {
                name: "queueRegistrySize",
                displayName: "Queue Registry Size",
                longDescription: `This structure must be long enough to hold all TI internal queues
and any application-registered queues. It should be sized according to a specific application's need based on the number
of kernel objects used by TI code and the application. If set to 0, the queue will be disabled.`,
                default: 32,
                hidden: true
            },
            {
                name: "useCustomHeap",
                displayName: "Enable Custom Heap",
                description: `Enable the applicaion to use a custom heap`,
                longDescription: `
When set to true, configAPPLICAITON_ALLOCATED_HEAP will be set in FreeRTOSConfig.h. The application is
responsible for defining vPortFree and pvPortMalloc with their custom heap configuration.`,
                default: false,
                onChange: onChooseCustomHeap

            },
            /* Memory size controls (collapsed by default) */
            {
                displayName: "Stack and Heap sizing",
                config: [
                    {
                        name: "idleStackSize",
                        description: `Idle task stack depth (bytes)`,
                        displayName: "Idle Task Stack Size",
                        displayFormat: { radix: "hex", bitSize: 32 },
                        default: 0x200
                    },
                    {
                        name: "timerStackSize",
                        description: `Timer task stack depth (bytes)`,
                        displayName: "Timer Task Stack Size",
                        displayFormat: { radix: "hex", bitSize: 32 },
                        default: 0x200
                    },
                    {
                        name: "posixThreadStackSize",
                        description: `Default Stack depth (bytes) for TI-POSIX threads`,
                        displayName: "Posix Thread Stack Size",
                        displayFormat: { radix: "hex", bitSize: 32 },
                        default: 0x400
                    },
                    {
                        name: "heapSize",
                        description: `Total size of system heap in bytes`,
                        displayName: "Heap Size",
                        displayFormat: { radix: "hex", bitSize: 32 },
                        default: Settings.defaultHeapSize
                    }
                ]
            },
            /* Hidden settings - internal use only, for specifying differences between device implementations
             * If you modify these settings in your application, the behaviour is undefined.
             */
            {
                name: "cpuFrequency",
                displayName: "CPU Frequency (Hz)",
                default: Settings.defaultCpuFrequency,
                hidden: true
            },
            {
                name: "maxInterruptPriority",
                displayName: "Max FreeRTOS SysCall Interrupt Priority",
                description: "Max FreeRTOS SysCall Interrupt Priority. Changing this value with IAR requires rebuilding the IAR port library.",
                default: Settings.defaultMaxInterruptPriority,
                hidden: true
            },
            {
                name: "nvicPriBits",
                displayName: "Number of NVIC Priority Bits available",
                default: Settings.defaultNvicPriBits,
                hidden: true
            },
            {
                name: "fpuEnabled",
                displayName: "FPU is enabled for ths device",
                description: "Enable FPU-related handling within the kernel. Changing this value with IAR requires rebuilding the IAR port library.",
                default: Settings.defaultFpuEnabled,
                hidden: true
            },
            {
                name: "idleSleepTicks",
                displayName: "Idle Sleep Ticks",
                description: "Free scheduler ticks before invoking power policy",
                default: Settings.defaultIdleSleepTicks,
                hidden: true
            },
            {
                name: "usePortTaskSelection",
                displayName: "Task Selection Implementation",
                default: Settings.defaultPortTaskSelection,
                options: [
                    { name: 0, displayName: "Generic implementation" },
                    { name: 1, displayName: "Port implementation" }
                ],
                hidden: true
            }
        ]
    },
    templates: {
        "/freertos/ti_freertos_config.h.xdt": true,
        "/freertos/ti_freertos_config.c.xdt": true,
        "/freertos/ti_freertos_portable_config.c.xdt": true,
        "/ti/utils/build/GenLibs.cmd.xdt": { modName: "/freertos/FreeRTOS", getLibs: getLibs },

        "/ti/utils/rov/syscfg_c.rov.xs.xdt": ["/kernel/freertos/rov/heap.rov.js",
                                              "/kernel/freertos/rov/helper.rov.js",
                                              "/kernel/freertos/rov/mutex.rov.js",
                                              "/kernel/freertos/rov/queue.rov.js",
                                              "/kernel/freertos/rov/semaphore.rov.js",
                                              "/kernel/freertos/rov/stack.rov.js",
                                              "/kernel/freertos/rov/task.rov.js",
                                              "/kernel/freertos/rov/timer.rov.js"]
    },
    getCFiles: getCFiles,
    getPortableFiles: getPortableFiles
};

function onChooseCustomHeap(inst, ui)
{
    if(inst.useCustomHeap)
    {
        ui.heapSize.hidden = true;
    }
    else
    {
        ui.heapSize.hidden = false;
    }
}
exports = base;
