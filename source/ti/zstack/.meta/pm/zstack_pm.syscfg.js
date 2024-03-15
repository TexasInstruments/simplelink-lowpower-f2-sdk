/*
 * Copyright (c) 2018, Texas Instruments Incorporated
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
 */

/*
 *  ======== zstack_pm.syscfg.js ========
 */

"use strict";

const Common = system.getScript("/ti/zstack/zstack_common.js");

/* Description text for configurables */

const powerModeCapabilitiesLongDescription = `Specify the power mode features included \
in the build (compile-time features included).

Note that for ZigBee Coordinators and ZigBee Routers, the power mode of \
operation is fixed to *Always On*. This is required for coordinators and \
routers. ZigBee End Devices can be freely configured between *Sleepy* and \
*Always On*.

For more information, refer to the [ZigBee Configuration](/zigbee/html/\
sysconfig/zigbee.html#zigbee-configuration) section of the ZigBee User's \
Guide.

**Default:** Sleepy (Low Power Mode enabled)`;

const powerModeLongDescription = `Specify whether the radio should always be \
on or be allowed to sleep (selected run-time configuration).

Note that for ZigBee Coordinators and ZigBee Routers, the power mode of \
operation is fixed to *Always On*. This is required for coordinators and \
routers. ZigBee End Devices can be freely configured between *Sleepy* and \
*Always On*.

For more information, refer to the [ZigBee Configuration](/zigbee/html/\
sysconfig/zigbee.html#zigbee-configuration) section of the ZigBee User's \
Guide.

**Default:** Sleepy (Low Power Mode enabled)`;

const minPollPeriodDescription = `The minimal poll period (in milliseconds).`;

const minPollPeriodLongDescription = `The minimal poll period (in milliseconds).

**Default:** 100 ms

**Range:** 1 -  ${Common.POLL_PERIOD_MAX - 1}  ms`;
const pollPeriodDescription = `The period (in milliseconds) between poll \
messages.`;

const pollPeriodLongDescription = `The period (in milliseconds) between poll \
messages.

**Default:** 3000 ms

**Range:** Minimal Poll Period - ${Common.POLL_PERIOD_MAX} ms`;

/* Description text for configurables */
const queuedMessageDescription = `The period (in milliseconds) between \
sending queued messages, used after a data indication to poll for queued \
messages.`;

const queuedMessageLongDescription = queuedMessageDescription + `\n\n\
**Default:** 100 ms

**Range:** 0 - 65535 ms`;

const dataResponseDescription = `The period (in milliseconds) between \
sending response messages, used after a data indication to poll for \
response messages.`;

const dataResponseLongDescription = dataResponseDescription + `\n\n\
**Default:** 100 ms

**Range:** 0 - 65535 ms`;

const rejoinMessageDescription = `The period (in milliseconds) between \
sending rejoin request messages, used after a data indication to poll for \
rejoin request messages.`;

const rejoinMessageLongDescription = rejoinMessageDescription + `\n\n\
**Default:** 440 ms

**Range:** 0 - 65535 ms`;

/* Power management submodule for zstack module */
const pmModule = {
    config: [
        {
            name: "deviceType",
            displayName: "Device Type",
            description: "Hidden configurable for passing in device type",
            default: "",
            hidden: true,
            onChange: onDeviceTypeChange
        },
        {
            name: "powerModeCapabilities",
            displayName: "Power Mode Capabilities",
            description: "Choose End Device build power modes features",
            longDescription: powerModeCapabilitiesLongDescription,
            default: "sleepy",
            options: [
                {
                    name: "alwaysOn",
                    displayName: "Always On + Sleepy Supported"
                },
                {
                    name: "sleepy",
                    displayName: "Only Sleepy Supported"
                }
            ],
            onChange: onPowerModeCapabilitiesChange
        },
        {
            name: "powerModeOperation",
            displayName: "Power Mode of Operation",
            description: "Specify whether the radio should always be on "
                         + "or be allowed to sleep",
            longDescription: powerModeLongDescription,
            default: "sleepy",
            readOnly: true,
            options: [
                {
                    name: "alwaysOn",
                    displayName: "Always On (Low Power Mode disabled)"
                },
                {
                    name: "sleepy",
                    displayName: "Sleepy (Low Power Mode enabled)"
                }
            ],
            onChange: onPowerModeChange
        },
        {
            name: "minPollPeriod",
            displayName: "Minimal Poll Period (ms)",
            description: minPollPeriodDescription,
            longDescription: minPollPeriodLongDescription,
            default: 100,
            hidden: false
        },
        {
            name: "pollPeriod",
            displayName: "Poll Period (ms)",
            description: pollPeriodDescription,
            longDescription: pollPeriodLongDescription,
            default: 3000,
            hidden: false
        },
        {
            name: "queuedMessagePollPeriod",
            displayName: "Queued Message Poll Period (ms)",
            description: queuedMessageDescription,
            longDescription: queuedMessageLongDescription,
            default: 100,
            hidden: false
        },
        {
            name: "dataResponsePollPeriod",
            displayName: "Data Response Poll Period (ms)",
            description: dataResponseDescription,
            longDescription: dataResponseLongDescription,
            default: 100,
            hidden: false
        },
        {
            name: "rejoinMessagePollPeriod",
            displayName: "Rejoin Message Poll Period (ms)",
            description: rejoinMessageDescription,
            longDescription: rejoinMessageLongDescription,
            default: 440,
            hidden: false
        }

    ],
    validate: validate,
    templates: {
        "/ti/zstack/templates/ti_zstack_config.h.xdt":
                            "/ti/zstack/templates/ti_zstack_config.h.xdt"
    }
};

/* Function to handle changes in deviceType configurable */
function onDeviceTypeChange(inst, ui)
{
    if(inst.deviceType === "zc")
    {
        inst.powerModeCapabilities = "alwaysOn";
        ui.powerModeCapabilities.readOnly = true;
        ui.powerModeCapabilities.hidden = true;
        inst.powerModeOperation = "alwaysOn";
        ui.powerModeOperation.readOnly = true;
        ui.powerModeOperation.hidden = true;
        ui.minPollPeriod.hidden = true;
        ui.pollPeriod.hidden = true;
        ui.queuedMessagePollPeriod.hidden = true;
        ui.dataResponsePollPeriod.hidden = true;
        ui.rejoinMessagePollPeriod.hidden = true;
    }
    else if(inst.deviceType === "zr") // || inst.deviceType === "zc")
    {
        inst.powerModeCapabilities = "alwaysOn";
        ui.powerModeCapabilities.readOnly = true;
        ui.powerModeCapabilities.hidden = true;
        inst.powerModeOperation = "alwaysOn";
        ui.powerModeOperation.readOnly = true;
        ui.powerModeOperation.hidden = true;
        ui.minPollPeriod.hidden = true;
        ui.pollPeriod.hidden = true;
        ui.queuedMessagePollPeriod.hidden = true;
        ui.dataResponsePollPeriod.hidden = true;
        ui.rejoinMessagePollPeriod.hidden = true;
    }
    else if (inst.deviceType === "zed")
    {
        inst.powerModeCapabilities = "sleepy";
        ui.powerModeCapabilities.readOnly = false;
        ui.powerModeCapabilities.hidden = false;
        inst.powerModeOperation = "sleepy";
        ui.powerModeOperation.readOnly = false;
        ui.powerModeOperation.hidden = false;
        ui.minPollPeriod.hidden = false;
        ui.pollPeriod.hidden = false;
        ui.queuedMessagePollPeriod.hidden = false;
        ui.dataResponsePollPeriod.hidden = false;
        ui.rejoinMessagePollPeriod.hidden = false;
    }
    else if (inst.deviceType === "znp")
    {
        inst.powerModeCapabilities = "alwaysOn";
        ui.powerModeCapabilities.readOnly = true;
        ui.powerModeCapabilities.hidden = false;
        inst.powerModeOperation = "sleepy";
        ui.powerModeOperation.readOnly = false;
        ui.powerModeOperation.hidden = false;
        ui.minPollPeriod.hidden = false;
        ui.pollPeriod.hidden = false;
        ui.queuedMessagePollPeriod.hidden = false;
        ui.dataResponsePollPeriod.hidden = false;
        ui.rejoinMessagePollPeriod.hidden = false;
    }
}

/* Function to handle changes in powerModeOperation configurable */
function onPowerModeCapabilitiesChange(inst, ui)
{
    if(inst.powerModeCapabilities === "sleepy")
    {
        inst.powerModeOperation = "sleepy";
        ui.powerModeOperation.readOnly = true;
        ui.minPollPeriod.hidden = false;
        ui.pollPeriod.hidden = false;
        ui.queuedMessagePollPeriod.hidden = false;
        ui.dataResponsePollPeriod.hidden = false;
        ui.rejoinMessagePollPeriod.hidden = false;
    }
    else if(inst.powerModeCapabilities === "alwaysOn")
    {
        ui.powerModeOperation.readOnly = false;
        ui.minPollPeriod.hidden = false;
        ui.pollPeriod.hidden = false;
        ui.queuedMessagePollPeriod.hidden = false;
        ui.dataResponsePollPeriod.hidden = false;
        ui.rejoinMessagePollPeriod.hidden = false;
    }
}

/* Function to handle changes in powerModeOperation configurable */
function onPowerModeChange(inst, ui)
{
    if(inst.powerModeOperation === "alwaysOn")
    {
        ui.minPollPeriod.hidden = true;
        ui.pollPeriod.hidden = true;
        ui.queuedMessagePollPeriod.hidden = true;
        ui.dataResponsePollPeriod.hidden = true;
        ui.rejoinMessagePollPeriod.hidden = true;
    }
    else /* sleepy */
    {
        ui.minPollPeriod.hidden = false;
        ui.pollPeriod.hidden = false;
        ui.queuedMessagePollPeriod.hidden = false;
        ui.dataResponsePollPeriod.hidden = false;
        ui.rejoinMessagePollPeriod.hidden = false;
    }
}

/* Validation function for the power management submodule */
function validate(inst, validation)
{
    /* Verify min poll period min bound */
    if(inst.minPollPeriod < Common.POLL_PERIOD_MIN)
    {
        validation.logWarning(
            "Poll period less than " + Common.POLL_PERIOD_MIN
            + "ms may cause undefined stack behavior", inst, "minPollPeriod"
        );
    }

    /* Verify minimal poll period min bound */
    if(inst.minPollPeriod <= 0)
    {
        validation.logError(
            "Minimal Poll Period must be greater than 0", inst, "minPollPeriod"
        );
    }

    /* Verify minimal poll period max bound */
    if(inst.minPollPeriod > Common.POLL_PERIOD_MAX)
    {
        validation.logError(
            "Minimal Poll Period must be less than "
            + Common.POLL_PERIOD_MAX + " milliseconds (32 bits)",
            inst, "minPollPeriod"
        );
    }

    /* Verify poll period min bound */
    if(inst.pollPeriod < inst.minPollPeriod)
    {
        validation.logError(
            "Poll period must be greater than or equal to minimal poll period",
            inst, "pollPeriod"
        );
    }

    /* Verify poll period max bound */
    if(inst.pollPeriod > Common.POLL_PERIOD_MAX)
    {
        validation.logError(
            "Poll period must be less than or equal to "
            + Common.POLL_PERIOD_MAX + " milliseconds (32 bits)",
            inst, "pollPeriod"
        );
    }

    /* Validate Queued Poll Period */
    Common.validateRange(inst, validation, inst.queuedMessagePollPeriod,
        "queuedMessagePollPeriod", "Queued Message Poll Period", 0, 65535);

    /* Validate Response Poll Period */
    Common.validateRange(inst, validation, inst.dataResponsePollPeriod,
        "dataResponsePollPeriod", "Data Response Poll Period", 0, 65535);

    /* Validate Rejoin Poll Period */
    Common.validateRange(inst, validation, inst.rejoinMessagePollPeriod,
        "rejoinMessagePollPeriod", "Rejoin Message Poll Period", 0, 65535);
}

exports = pmModule;
