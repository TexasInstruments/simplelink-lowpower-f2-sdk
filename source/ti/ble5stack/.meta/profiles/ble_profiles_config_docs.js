/*
 * Copyright (c) 2018-2022 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== ble_profiles_config_docs.js ========
 */

"use strict";

// Get common utility functions
const Common = system.getScript("/ti/ble5stack/ble_common.js");

// Long description for the maxBonds configuration parameter
const profileSelectionLongDescription = `Select profiles to include\n
__Warning__: Adding more than 2 profiles may result in memory error.\n
__Default__: None\n
Profile | Description
--- | ---
Simple Gatt | This profile is an example of a non BT SIG profile. It demonstrates various types of characteristics, permissions, and properties.
CGM: Continuous Glucose Monitoring | This profile enables a device to connect and interact with a CGM Sensor for use in consumer healthcare applications.
Glucose | This profile enables a device to connect and interact with a glucose sensor for use in consumer healthcare applications.
HT: Health Thermometer | This profile enables a device to connect and interact with a HT Sensor for use in consumer healthcare applications.\n`

const deviceInfoLongDescription = ` *Need to Add*.\n
__Default__: True\n`

const maxNumPrepareWritesLongDescription =
`The maximum number of prepare write (GATT_MAX_NUM_PREPARE_WRITES) on the GATT Server.\n
__Default__: 5\n
__Range__: 0 to 255\n`

 // Exports the long descriptions for each configurable
 exports = {
    profileSelectionLongDescription: profileSelectionLongDescription,
    deviceInfoLongDescription: deviceInfoLongDescription,
    maxNumPrepareWritesLongDescription: maxNumPrepareWritesLongDescription
};