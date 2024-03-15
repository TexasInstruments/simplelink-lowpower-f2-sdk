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
 *  ======== ble_profiles_config.syscfg.js ========
 */


"use strict";

// Get general long descriptions
const Docs = system.getScript("/ti/ble5stack/profiles/ble_profiles_config_docs.js");

// Get common Script
const Common = system.getScript("/ti/ble5stack/ble_common.js");


const config =
{
    name: "profiles_module",
    displayName: "Profiles Configurations",
    description: "Configure Profile Settings",
    config:[
        {
            name: "hideBasicBLEGroup",
            default: true,
            hidden: true
        },
        {
            name: "deviceInfo",
            displayName: "Device Info",
            legacyNames: ["DeviceInfo"],
            longDescription: Docs.deviceInfoLongDescription,
            default: true,
            hidden: true
        },
        {
            name: "profiles",
            displayName: "Profile Selection",
            longDescription: Docs.profileSelectionLongDescription,
            default: [],
            minSelections: 0,
            options: Common.profiles_list,
            onChange: onChangeProfile,
            hidden: true
        },
        {
            name: "maxNumPrepareWrites",
            displayName: "Maximum number of prepare writes",
            default: 5,
            longDescription: Docs.maxNumPrepareWritesLongDescription
        }
    ]
};

/*
 *  ======== onChangeProfile ========
 * Change General configurations according to changed profile
 *
 * @param inst  - Module instance containing the config that changed
 * @param ui    - The User Interface object
 */
function onChangeProfile(inst,ui)
{
 // UUID: 6152 - Glucose, UUID: 6175 - CGM, UUID: 6153 - HT
 // if one of this profiles was added, change PDU values to those number
 if(inst.profiles.includes("6152") || inst.profiles.includes("6175") || inst.profiles.includes("6153"))
 {
    inst.maxPDUNum = 100;
    inst.deviceName = "Profiles";
    // update max PDU size to 27
    inst.maxPDUSize = 27;
 }
 // if no profile was added
 if(inst.profiles.length == 0)
 {
    inst.deviceName = "Basic BLE Project";
    inst.maxPDUNum = 5;
    inst.maxPDUSize = 69;
 }
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
    if(inst.maxNumPrepareWrites < 0 || inst.maxNumPrepareWrites > 255)
    {
        validation.logError("The range of max number of prepare writes is 0 to 255", inst, "maxNumPrepareWrites");
    }
}

// Exports to the top level BLE module
exports = {
    config: config,
    validate: validate
};