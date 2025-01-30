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
 *  ======== ble_adv_config.syscfg.js ========
 */

"use strict";

// Get general long descriptions
const Docs = system.getScript("/ti/ble5stack/adv_config/ble_adv_config_docs.js");

// Get common Script
const Common = system.getScript("/ti/ble5stack/ble_common.js");

// OAD hidden value
const oadHiddenVal = Common.hiddenValue();

// Loki hidden value
const lokiHiddenVal = Common.hiddenValue();

// TBM values
const defaultTBMVal = Common.defaultValue();
const hiddenTBMVal = Common.hiddenValue();

const config = {
    displayName: "Advanced Settings",
    description: "Configure Extended Settings",
    config: [
        {
            name: "disableDisplayModule",
            displayName: "Disable The Display Module",
            longDescription: Docs.disableDisplayModuleLongDescription,
            default: false
        },
        {
            name: "powerMamagement",
            displayName: "Power Management",
            longDescription: Docs.powerMamagementLongDescription,
            default: true
        },
        {
            name: "halAssert",
            displayName: "Hal Assert",
            longDescription: Docs.halAssertLongDescription,
            default: false
        },
        {
            name: "tbmActiveItemsOnly",
            displayName: "Two Button Menu - Active Items Only",
            longDescription: Docs.tbmActiveItemsOnlyLongDescription,
            default: defaultTBMVal,
            hidden: hiddenTBMVal
        },
        {
            name: "stackLibrary",
            displayName: "Stack Library",
            longDescription: Docs.stackLibraryLongDescription,
            readOnly: true,
            hidden: true,
            default: true
        },
        {
            name: "dontTransmitNewRpa",
            displayName: "Don't Transmit New RPA",
            longDescription: Docs.dontTransmitNewRpaLongDescription,
            hidden: false,
            default: false
        },
        {
            name: "extendedStackSettings",
            displayName: "Extended Stack Settings",
            longDescription: Docs.extendedStackSettingsLongDescription,
            default: "EXTENDED_STACK_SETTINGS_DEFAULT",
            options: [
                {
                    displayName: "Default",
                    name: "EXTENDED_STACK_SETTINGS_DEFAULT"
                },
                {
                    displayName: "Guard Time",
                    name: Common.getExtendedStackSettingsGuardTimeName(),
                    legacyNames: Common.isFlashOnlyDevice() ? ["MASTER_GUARD_TIME_ENABLE"]: [""]
                }
            ]
        },
        {
            name: "noOsalSnv",
            displayName: "No Osal SNV",
            description: "Use only one page of SNV or do not use any NV pages "
                        + "if the GAP bond manager is not required",
            default: false
        },
        {
            name:"uartLog",
            displayName: "UART Log",
            default: false
        },
        {
            name: "ptm",
            displayName: "PTM",
            longDescription: Docs.ptmLongDescription,
            hidden: false,
            default: false,
            onChange: onPtmChange
        },
        {
            name: "adaptivity",
            displayName: "Adaptivity",
            longDescription: Docs.adaptivityLongDescription,
            hidden: false,
            default: false,
            onChange: onAdaptivityChange
        },
        {
            name: "useRcosc",
            displayName: "Use RCOSC",
            default: false,
            hidden: true,
        },
        {
            name: "peerConnParamUpdateRejectInd",
            displayName: "Peer Conn Param Update Reject Ind",
            longDescription: Docs.peerConnParamUpdateRejectIndLongDescription,
			hidden: false,
			default: false
        },
        {
            displayName: "NPI",
            config: [
                {
                    name: "flowControl",
                    displayName: "Flow Control",
                    longDescription: Docs.flowControlLongDescription,
                    default: "0",
                    hidden: true,
                    options: [
                        { name: "0" },
                        { name: "1" }
                    ]
                }
            ]
        },
        {
            displayName: "Adaptivity advance settings",
            config: [
                {
                    name: "fixedObservTime",
                    displayName: "Fixed Observation Time",
                    longDescription: Docs.fixedObservTimeLongDescription,
                    default: true,
                    hidden: true,
                    onChange: onDynamicObservTimeChange
                },
                {
                    name: "observationTime",
                    displayName: "Observation Time Value ( ms )",
                    longDescription: Docs.observationTimeLongDescription,
                    default: "1",
                    hidden: true,
                },
                {
                    name: "txUsageThresh",
                    displayName: "Tx Usage Threshold (%)",
                    longDescription: Docs.txUsageThreshLongDescription,
                    default: "0",
                    hidden: true,
                },
                {
                    name: "rxWindowDuration",
                    displayName: "Rx Window Duration",
                    longDescription: Docs.rxWindowDurationLongDescription,
                    default: "10",
                    hidden: true,
                },
                {
                    name: "rssiThreshold",
                    displayName: "RSSI Threshold",
                    longDescription: Docs.rssiThresholdLongDescription,
                    default: "-70",
                    hidden: true,
                },
                {
                    name: "numberOfNoisySamples",
                    displayName: "Number Of Noisy Samples",
                    longDescription: Docs.numberOfNoisySamplesLongDescription,
                    default: "1",
                    hidden: true,
                },
                {
                    name: "blockingChannelTime",
                    displayName: "Blocking Channel Time (sec)",
                    longDescription: Docs.blockingChannelTimeLongDescription,
                    default: "1",
                    hidden: true,
                },

            ]
        },
        {
            name: "oneLibSizeOpt",
            displayName: "One Lib Size Optimization",
            default: true,
            hidden: true,
        },
        {
            name: "useCoex",
            displayName: "Use Coex",
            default: false,
            hidden: true,
        },
        {
            displayName: "ICALL",
            config: [
                {
                    name: "icallEvents",
                    displayName: "ICALL Events",
                    readOnly: true,
                    hidden: true,
                    default: true
                },
                {
                    name: "icallJT",
                    displayName: "ICALL JT",
                    readOnly: true,
                    hidden: true,
                    default: true
                },
                {
                    name: "icallLite",
                    displayName: "ICALL Lite",
                    readOnly: true,
                    hidden: true,
                    default: true
                },
                {
                    name: "icallStackAddress",
                    displayName: "ICALL Stack Address",
                    readOnly: true,
                    hidden: true,
                    default: true
                },
                {
                    name: "useIcall",
                    displayName: "Use ICALL",
                    readOnly: true,
                    hidden: true,
                    default: true
                },
                {
                    name: "maxNumEntIcall",
                    displayName: "Max Num Of Entities That Use ICall",
                    longDescription: Docs.maxNumEntIcallLongDescription,
                    default: 6
                },
                {
                    name: "maxNumIcallEnabledTasks",
                    displayName: "Max Number of ICall Enabled Tasks",
                    longDescription: Docs.maxNumIcallEnabledTasksLongDescription,
                    default: 3
                }
            ]
        },
        {
            displayName: "OAD Only Defines",
            config: [
                {
                    name: "ledDebug",
                    displayName: "Led Debug",
                    default: false,
                    hidden: oadHiddenVal
                },
                {
                    name: "oadDebug",
                    displayName: "OAD Debug",
                    default: false,
                    hidden: oadHiddenVal
                },
                {
                    name: "oadFeature",
                    displayName: "OAD Feature",
                    default: false,
                    hidden: oadHiddenVal
                },
                {
                    name: "oadBleSecurity",
                    displayName: "OAD BLE Security",
                    default: false,
                    hidden: oadHiddenVal
                }
            ]
        }
    ]
};

function onPtmChange(inst, ui)
{
    ui.flowControl.hidden = !inst.ptm;

    // When PTM_MODE is selected change the number of ICALL
    // tasks to 4
    if(inst.ptm)
    {
        inst.maxNumIcallEnabledTasks = 4;
    }
    else
    {
        inst.maxNumIcallEnabledTasks = 3;
    }
}

function onAdaptivityChange(inst, ui)
{
    ui.rssiThreshold.hidden = !inst.adaptivity;
}

function onDynamicObservTimeChange(inst, ui)
{
    ui.observationTime.hidden = !inst.fixedObservTime;
}

/*
 *  ======== modules ========
 *  Determines what modules are added as static submodules
 *
 *  @param inst  - Module instance containing the config that changed
 *  @returns     - Array containing a static dependency modules
 */
function modules(inst)
{
    let dependencyModule = [];

    if(inst.ptm && system.getRTOS() === "tirtos7")
    {
        dependencyModule.push({
            name: "gateMutex",
            displayName: "GateMutex",
            moduleName: "/ti/sysbios/gates/GateMutex",
            hidden: true
        });
    }

    return(dependencyModule);
}

/*
 * ======== validate ========
 * Validate this inst's configuration
 *
 * @param inst       - Adv Config instance to be validated
 * @param validation - object to hold detected validation issues
 */
function validate(inst, validation)
{
    if(inst.maxNumEntIcall < 0)
    {
        validation.logError("The value must be a positive number", inst, "maxNumEntIcall");
    }

    if(inst.maxNumIcallEnabledTasks < 0)
    {
        validation.logError("The value must be a positive number", inst, "maxNumIcallEnabledTasks");
    }

    if(inst.ptm && inst.maxNumIcallEnabledTasks < 4)
    {
        validation.logError("The value must be >= 4", inst, "maxNumIcallEnabledTasks");
    }

    if(inst.rxWindowDuration < 10 || inst.rxWindowDuration > 100)
    {
        validation.logError("The value must be >=10 or <=100", inst, "rxWindowDuration");
    }

    if(inst.txUsageThresh < 0 || inst.txUsageThresh > 100)
    {
        validation.logError("The value must be >=0 or <=100", inst, "txUsageThresh");
    }
    if(inst.rssiThreshold < -90 || inst.rssiThreshold > -40)
    {
        validation.logError("The value must be >=-90 or <=-40", inst, "rssiThreshold");
    }
    if(inst.numberOfNoisySamples < 1 || inst.numberOfNoisySamples > 5)
    {
        validation.logError("The value must be >=1 or <=5", inst, "numberOfNoisySamples");
    }
    if(inst.blockingChannelTime < 0 || inst.blockingChannelTime > 255)
    {
        validation.logError("The value must be >=0 or <=255", inst, "blockingChannelTime");
    }
    if(inst.observationTime < 1 || inst.observationTime > 3000)
    {
        validation.logError("The value must be >=10 or <=100", inst, "observationTime");
    }
}

// Exports to the top level BLE module
exports = {
    config: config,
    modules: modules,
    validate: validate
};