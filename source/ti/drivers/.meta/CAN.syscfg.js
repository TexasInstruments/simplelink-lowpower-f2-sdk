/*
 * Copyright (c) 2019-2023, Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== CAN.syscfg.js ========
 */
"use strict";

/* get Common /ti/drivers utility functions */
let Common = system.getScript("/ti/drivers/Common.js");
let logError = Common.logError;

/* get /ti/drivers family name from device object */
let family = Common.device2Family(system.deviceData, "CAN");

/* generic configuration parameters for CAN instances */
let config = [
    {
        name: "nomBitRate",
        displayName: "Nominal Bit Rate",
        description: "Specifies the nominal bit rate.",
        default: 125000,
        options: [
            { name: 125000 },
            { name: 250000 },
            { name: 500000 },
            { name: 1000000 }
        ],
        onChange: onNomRateChange
    },
    {
        name: "canFDEnable",
        displayName: "CAN FD Operation",
        description: "Enables CAN FD operation.",
        longDescription: "When CAN FD operation is disabled, all received frames " +
            "are interpreted as Classic CAN frames, which leads to the transmission " +
            "of an error frame if a CAN FD frame is received. Additionally, no CAN " +
            "FD frames can be transmitted.",
        default: false,
        onChange: onCANFDChange
    },
    {
        name: "brsEnable",
        displayName: "Bit Rate Switching (BRS)",
        description: "Enables bit rate switching for transmission.",
        longDescription: "When BRS is enabled, a higher bit rate can be used for the " +
            "data phase of CAN FD frames during transmission. See 'CAN FD Data Bit Rate'.",
        default: false,
        hidden: true,
        onChange: onBRSChange
    },
    {
        name: "dataBitRate",
        displayName: "CAN FD Data Bit Rate",
        description: "Specifies the CAN FD data bit rate when Bit Rate Switching (BRS) " +
            "is enabled.  Must be higher or equal to than the nominal bit rate.",
        default: 125000,
        hidden: true,
        options: [
            { name: 125000 },
            { name: 250000 },
            { name: 500000 },
            { name: 1000000 },
            { name: 2000000 },
            { name: 4000000 },
            { name: 5000000 }
        ]
    },
    {
        name: "rejectNonMatching",
        displayName: "Reject Non-Matching Messages",
        description: "Enables global filter to reject any received messages that " +
            " do not match any element of the filter list.",
        default: false
    },
    {
        name: "rejectRemoteFrames",
        displayName: "Reject Remote Frames",
        description: "Enables global filter to reject remote frames.",
        default: true,
        readOnly: true
    },

    /* Rx and Tx ring buffer sizes */
    {
        name        : "rxRingBufferSize",
        displayName : "Rx Element Ring Buffer Size",
        description : "Number of Rx buffer elements in the RX ring buffer",
        longDescription: "The Rx element ring buffer serves as storage for " +
            "all received messages. When messages are received into the CAN Message RAM " +
            "(MRAM), they are copied into the ring buffer and the Rx callback is " +
            "executed. When CAN_read() is called, the Rx element is freed from " +
            "the ring buffer. If the ring buffer is full when a message is received, " +
            "it will be discarded. The size can be changed based on the application " +
            "and MRAM configuration. Each Rx element occupies 78-bytes.",
        default     : 6
    },
    {
        name        : "txRingBufferSize",
        displayName : "Tx Element Ring Buffer Size",
        description : "Number of Tx buffer elements in the TX ring buffer",
        longDescription : "The TX element ring buffer serves as an extension " +
            "of the CAN Message RAM (MRAM) Tx buffers. If the MRAM Tx buffers are full when " +
            "CAN_write() is called, the Tx element is copied into the ring buffer. " +
            "After a MRAM Tx buffer is freed, the Tx element will be moved from the ring " +
            "buffer to the MRAM Tx FIFO/Queue buffer for transmission. The size can be changed " +
            "based on the application and MRAM configuration. Each Tx element occupies 100-bytes. " +
            "Set to 0 to use MRAM Tx buffers only.",
        default     : 0
    }
];

/*
 * ======== onCANFDChange ========
 */
function onCANFDChange(inst, ui)
{
    inst.brsEnable = false;
    ui.dataBitRate.hidden = true;

    if (inst.canFDEnable == true)
    {
        ui.brsEnable.hidden = false;
    }
    else
    {
        ui.brsEnable.hidden = true;
    }
}

/*
 * ======== onBRSChange ========
 */
function onBRSChange(inst, ui)
{
    if (inst.brsEnable == true)
    {
        ui.dataBitRate.hidden = false;

        if (inst.dataBitRate < inst.nomBitRate)
        {
            inst.dataBitRate = inst.nomBitRate;
        }
    }
    else
    {
        ui.dataBitRate.hidden = true;
        inst.dataBitRate = inst.nomBitRate;
    }
}

/*
 * ======== onNomRateChange ========
 */
function onNomRateChange(inst, ui)
{
    if (inst.dataBitRate < inst.nomBitRate)
    {
        inst.dataBitRate = inst.nomBitRate;
    }
}

/*
 *  ======== moduleInstances ========
 */
function moduleInstances(inst) {
    /* Devices with onboard CAN will override this module instance */
    let moduleInstance = [{
        name: "externalCAN",
        displayName: "External CAN",
        moduleName: "/ti/drivers/can/CAN" + family
    }];

    return (moduleInstance);
}

/*
 *  ======== validate ========
 *  Validate this instance's configuration
 *
 *  @param inst       - CAN instance to be validated
 *  @param validation - object to hold detected validation issues
 */
function validate(inst, validation)
{
    let message;

    if (inst.brsEnable && (inst.dataBitRate < inst.nomBitRate))
    {
        message = 'CAN FD Data Bit Rate must greater than or equal to the Nominal Bit Rate';
        logError(validation, inst, "dataBitRate", message);
    }

    if (inst.rxRingBufferSize == 0)
    {
        message = 'Rx Element Ring Buffer Size must be greater than 0';
        logError(validation, inst, "rxRingBufferSize", message);
    }
}

/*
 *  ======== base ========
 *  Define the base properties and methods
 */
let base = {
    displayName: "CAN",
    description: "Controller Area Network (CAN) Driver",
    longDescription: `
The [__CAN driver__][1] provides APIs to control a single external or onboard Controller Area Network (CAN).

* [Usage Synopsis][2]
* [Examples][3]
* [Configuration Options][4]

[1]: /drivers/doxygen/html/_c_a_n_8h.html#details "C API reference"
[2]: /drivers/doxygen/html/_c_a_n_8h.html#ti_drivers_CAN_Synopsis "Basic C usage summary"
[3]: /drivers/doxygen/html/_c_a_n_8h.html#ti_drivers_CAN_Examples "C usage examples"
[4]: /drivers/syscfg/html/ConfigDoc.html#CAN_Configuration_Options "Configuration options reference"
`,
    maxInstances: 1,
    defaultInstanceName: "CONFIG_CAN_",
    config: Common.addNameConfig(config, "/ti/drivers/CAN", "CONFIG_CAN_"),
    validate: validate,
    moduleInstances: moduleInstances,
    modules: Common.autoForceModules(["Board", "Power"]),
    templates: {
        boardc: "/ti/drivers/can/CAN.Board.c.xdt",
        boardh: "/ti/drivers/can/CAN.Board.h.xdt"
    }
};

/* extend the base exports to include family-specific content */
let deviceCAN = system.getScript("/ti/drivers/can/CAN" + family);
exports = deviceCAN.extend(base);
