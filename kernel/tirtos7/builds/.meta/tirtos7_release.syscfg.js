/*
 * Copyright (c) 2021-2024, Texas Instruments Incorporated
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

/* This file can be loaded in .syscfg files programmatically in order to load
 * the tirtos 7 kernel configuration that used to be provided as default on
 * tirtos 6.
 */

var device = system.deviceData.deviceId;

if (system.getRTOS() === "tirtos7") {
    if (device.match(/CC13.1/) || device.match(/CC26.1/)) {
        system.getScript("cc13x1_cc26x1/release/tirtos7_release.syscfg.js");
    }
    else if (device.match(/CC13.2/) || device.match(/CC26.2/)) {
        system.getScript("cc13x2_cc26x2/release/tirtos7_release.syscfg.js");
    }
    else if (device.match(/CC13.2.7/) || device.match(/CC26.2.7/)) {
        system.getScript("cc13x2x7_cc26x2x7/release/tirtos7_release.syscfg");
    }
    else if (device.match(/CC13.4/) || device.match(/CC26.4/) || device.match(/CC26.3/)) {
        system.getScript("cc13x4_cc26x4/release/tirtos7_release.syscfg.js");
    }
}