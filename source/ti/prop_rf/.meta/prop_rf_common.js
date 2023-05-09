/*
 * Copyright (c) 2019 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== prop_rf_common.js ========
 */

"use strict";

// Max and min C type values
const maxUint32t = 0xFFFFFFFF;
const maxUint32tRatUsTime = Math.floor(maxUint32t / 4);
const maxUint32tRatMsTime = Math.floor(maxUint32t / 4000);
const maxInt8t = 127;
const minInt8t = -128;

// Dictionary mapping a device name to default LaunchPad; used to discover the
// appropriate RF settings when a device is being used without a LaunchPad
const deviceToBoard = {
    CC1352R: "CC1352R1_LAUNCHXL",
    CC1352P1: "CC1352P1_LAUNCHXL",
    CC1312R1F3RGZ: "CC1312R1_LAUNCHXL",
    CC2642: "CC26X2R1_LAUNCHXL",
    CC2652P1FSIP: "LP_CC2652PSIP",
    CC2652R1FSIP: "LP_CC2652RSIP",
    CC2652R1: "CC26X2R1_LAUNCHXL",
    CC2652RB: "CC2652RB_LAUNCHXL",
    CC2652P1FRGZ: "CC1352P_4_LAUNCHXL",
    CC2652R7: "LP_CC2652R7",
    CC1312R7RGZ: "LP_CC1312R7",
    CC1352P7: "LP_CC1352P7_1",
    CC2651P3: "LP_CC2651P3",
    CC2651R3SIPA: "LP_CC2651R3SIPA",
    CC2651R3: "LP_CC2651R3",
    CC1311P3: "LP_CC1311P3",
    CC2674R10RGZ: "LP_CC2674R10_FPGA",
    CC1354R10: "LP_CC1354R10_RGZ",
    CC1354P10RSK: "LP_EM_CC1354P10_1",
    CC1314R10RSK: "LP_EM_CC1314R10",
    CC2653P10RGZ: "LP_CC2653P10",
	CC1312PSIP: "LP_EM_CC1312PSIP"
};

// Settings for ti/devices/CCFG module
const propRfCCFGSettings = {
    CC1312R1_LAUNCHXL_CCFG_SETTINGS: {
        forceVddr: false,
        enableBootloaderBackdoor: false
    },
    CC1352R1_LAUNCHXL_CCFG_SETTINGS: {
        forceVddr: true,
        enableBootloaderBackdoor: false
    },
    CC1352P1_LAUNCHXL_CCFG_SETTINGS: {
        forceVddr: false,
        enableBootloaderBackdoor: false
    },
    CC1352P_2_LAUNCHXL_CCFG_SETTINGS: {
        forceVddr: true,
        enableBootloaderBackdoor: false
    },
    CC1352P_4_LAUNCHXL_CCFG_SETTINGS: {
        forceVddr: false,
        enableBootloaderBackdoor: false
    },
    CC26X2R1_LAUNCHXL_CCFG_SETTINGS: {
        forceVddr: false,
        enableBootloaderBackdoor: false
    },
    CC2652RB_LAUNCHXL_CCFG_SETTINGS: {
        forceVddr: false,
        enableBootloaderBackdoor: false
    },
    LP_CC2652PSIP_CCFG_SETTINGS: {
        forceVddr: false,
        enableBootloaderBackdoor: false
    },
    LP_CC2652RSIP_CCFG_SETTINGS: {
        forceVddr: false,
        enableBootloaderBackdoor: false
    },
    LP_CC1312R7_CCFG_SETTINGS: {
        forceVddr: false,
        enableBootloaderBackdoor: false
    },
    LP_CC2652R7_CCFG_SETTINGS: {
        forceVddr: false,
        enableBootloaderBackdoor: false
    },
    LP_CC1352P7_1_CCFG_SETTINGS: {
        forceVddr: false,
        enableBootloaderBackdoor: false
    },
    LP_CC1352P7_4_CCFG_SETTINGS: {
        forceVddr: false,
        enableBootloaderBackdoor: false
    },
    LP_CC2651P3_CCFG_SETTINGS: {
        enableBootloaderBackdoor: false
    },
     LP_CC2651R3_CCFG_SETTINGS: {
        enableBootloaderBackdoor: false
    },
    LP_CC1311P3_CCFG_SETTINGS: {
        enableBootloaderBackdoor: false
    },
    LP_CC2674R10_FPGA_CCFG_SETTINGS: {        
        forceVddr: false,
        enableBootloaderBackdoor: false
    },
    LP_CC1354R10_RGZ_CCFG_SETTINGS: {
        forceVddr: true,
        enableBootloaderBackdoor: false
    },
    LP_CC2651R3SIPA_CCFG_SETTINGS: {
        enableBootloaderBackdoor: false    
    },
    LP_EM_CC1314R10_CCFG_SETTINGS: {
        forceVddr: false,
        enableBootloaderBackdoor: false
    },
    LP_EM_CC1354P10_1_CCFG_SETTINGS: {
        forceVddr: false,
        enableBootloaderBackdoor: false
    },
    LP_EM_CC1354P10_6_CCFG_SETTINGS: {
        forceVddr: false,
        enableBootloaderBackdoor: false
    },
    LP_CC2653P10_CCFG_SETTINGS: {
        forceVddr: false,
        enableBootloaderBackdoor: false
    },
	LP_EM_CC1312PSIP_CCFG_SETTINGS: {
        forceVddr: false,
        enableBootloaderBackdoor: false
	}
};

const boardName = getDeviceOrLaunchPadName(true);
const ccfgSettings = propRfCCFGSettings[boardName + "_CCFG_SETTINGS"];

const sub1To24Text = "Cannot migrate from a Sub-1GHZ device to a 2.4GHz "
    + "device";
const twoFourToSub1Text = "Cannot migrate from a Sub-1GHZ device to a 2.4GHz "
    + "device";
const supportedMigrations = {
    CC1352R1_LAUNCHXL: {
        CC1352R1F3RGZ: {},
        CC1312R1F3RGZ: {},
        CC1312R1_LAUNCHXL: {},
        CC1352R1_LAUNCHXL: {}
    },
    CC1352P1_LAUNCHXL: {
        CC1352P1F3RGZ: {},
        CC1352P1_LAUNCHXL: {}
    },
    LP_CC1352P7_1: {
        CC1352P7RGZ: {},
        LP_CC1352P7_1: {}
    },
    CC1352P_2_LAUNCHXL: {
        CC1352P1F3RGZ: {},
        CC1352P_2_LAUNCHXL: {},
        CC2652P1FRGZ: {}
    },
    CC1352P_4_LAUNCHXL: {
        CC1352P1F3RGZ: {},
        CC1352P_4_LAUNCHXL: {},
        CC2652P1FRGZ: {}
    },
    LP_CC1352P7_4: {
        CC1352P7RGZ: {},
        LP_CC1352P7_4: {}
    },
    LP_CC1354R10_RGZ: {
        CC1354R10RGZ: {},
        LP_CC1354R10_RGZ: {}
    },
    LP_EM_CC1354P10_1: {
        CC1354P10RSK: {},
        LP_EM_CC1354P10_1: {}
    },
    LP_EM_CC1354P10_6: {
        CC1354P10RSK: {},
        LP_EM_CC1354P10_6: {}
    },
    LP_EM_CC1314R10: {
        CC1314R10RSK: {},
        LP_EM_CC1314R10: {},
        LP_CC2652RSIP: {disable: sub1To24Text},
        CC26X2R1_LAUNCHXL: {disable: sub1To24Text},
        CC2652R1FSIP: {disable: sub1To24Text},
        CC2652R1FRGZ: {disable: sub1To24Text},
        CC2652R7RGZ: {disable: sub1To24Text},
        LP_CC2652R7: {disable: sub1To24Text},
        LP_CC2651P3: {disable: sub1To24Text},
        LP_CC2651R3: {disable: sub1To24Text},
        CC2651P3RGZ: {disable: sub1To24Text},
        CC2651R3RGZ: {disable: sub1To24Text},
        CC2652P1FSIP: {disable: sub1To24Text},
        LP_CC2652PSIP: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_CC2653P10: {disable: sub1To24Text}
    },
    CC1312R1_LAUNCHXL: {
        CC1312R1F3RGZ: {},
        CC1312R1_LAUNCHXL: {},
        LP_CC2652RSIP: {disable: sub1To24Text},
        CC26X2R1_LAUNCHXL: {disable: sub1To24Text},
        CC2652R1FSIP: {disable: sub1To24Text},
        CC2652R1FRGZ: {disable: sub1To24Text},
        CC2652R7RGZ: {disable: sub1To24Text},
        LP_CC2652R7: {disable: sub1To24Text},
        LP_CC2651P3: {disable: sub1To24Text},
        LP_CC2651R3: {disable: sub1To24Text},
        CC2651P3RGZ: {disable: sub1To24Text},
        CC2651R3RGZ: {disable: sub1To24Text},
        CC2652P1FSIP: {disable: sub1To24Text},
        LP_CC2652PSIP: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_EM_CC2653P10: {disable: sub1To24Text},
        CC2651R3SIPA: {disable: sub1To24Text},
        LP_CC2651R3SIPA: {disable: sub1To24Text},
        LP_CC2652PSIP: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_CC2653P10: {disable: sub1To24Text}
    },
    LP_CC1312R7: {
        CC1312R7RGZ: {},
        LP_CC1312R7: {},
        LP_CC2652RSIP: {disable: sub1To24Text},
        CC26X2R1_LAUNCHXL: {disable: sub1To24Text},
        CC2652R1FSIP: {disable: sub1To24Text},
        CC2652R1FRGZ: {disable: sub1To24Text},
        CC2652R7RGZ: {disable: sub1To24Text},
        LP_CC2652R7: {disable: sub1To24Text},
        LP_CC2651P3: {disable: sub1To24Text},
        LP_CC2651R3: {disable: sub1To24Text},
        CC2651P3RGZ: {disable: sub1To24Text},
        CC2651R3RGZ: {disable: sub1To24Text},
        CC2652P1FSIP: {disable: sub1To24Text},
        LP_CC2652PSIP: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_EM_CC2653P10: {disable: sub1To24Text},
        CC2651R3SIPA: {disable: sub1To24Text},
        LP_CC2651R3SIPA: {disable: sub1To24Text},
        LP_CC2652PSIP: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_CC2653P10: {disable: sub1To24Text}
    },
    LP_EM_CC1312PSIP: {
        CC1312PSIP: {},
        LP_EM_CC1312PSIP: {},
        LP_CC2652RSIP: {disable: sub1To24Text},
        CC26X2R1_LAUNCHXL: {disable: sub1To24Text},
        CC2652R1FSIP: {disable: sub1To24Text},
        CC2652R1FRGZ: {disable: sub1To24Text},
        CC2652R7RGZ: {disable: sub1To24Text},
        LP_CC2652R7: {disable: sub1To24Text},
        LP_CC2651P3: {disable: sub1To24Text},
        LP_CC2651R3: {disable: sub1To24Text},
        CC2651P3RGZ: {disable: sub1To24Text},
        CC2651R3RGZ: {disable: sub1To24Text},
        CC2652P1FSIP: {disable: sub1To24Text},
        LP_CC2652PSIP: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_EM_CC2653P10: {disable: sub1To24Text},
        CC2651R3SIPA: {disable: sub1To24Text},
        LP_CC2651R3SIPA: {disable: sub1To24Text},
        LP_CC2652PSIP: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_CC2653P10: {disable: sub1To24Text}
    },
    LP_CC1311P3: {
        CC1311P3RGZ: {},
        LP_CC1311P3: {},
        LP_CC2652RSIP: {disable: sub1To24Text},
        CC26X2R1_LAUNCHXL: {disable: sub1To24Text},
        CC2652R1FSIP: {disable: sub1To24Text},
        CC2652R1FRGZ: {disable: sub1To24Text},
        CC2652R7RGZ: {disable: sub1To24Text},
        LP_CC2652R7: {disable: sub1To24Text},
        LP_CC2651P3: {disable: sub1To24Text},
        LP_CC2651R3: {disable: sub1To24Text},
        CC2651P3RGZ: {disable: sub1To24Text},
        CC2651R3RGZ: {disable: sub1To24Text},
        CC2652P1FSIP: {disable: sub1To24Text},
        LP_CC2652PSIP: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_EM_CC2653P10: {disable: sub1To24Text},
        CC2651R3SIPA: {disable: sub1To24Text},
        LP_CC2651R3SIPA: {disable: sub1To24Text},
        LP_CC2652PSIP: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_CC2653P10: {disable: sub1To24Text}
    },
    CC26X2R1_LAUNCHXL: {
        CC2652R1FRGZ: {},
        CC2652R1FSIP: {},
        CC26X2R1_LAUNCHXL: {},
        LP_CC2652RSIP: {},
        CC1312R1_LAUNCHXL: {disable: twoFourToSub1Text},
        CC1312R1F3RGZ: {disable: twoFourToSub1Text},
        LP_CC1312R7: {disable: twoFourToSub1Text},
        CC1312R7RGZ: {disable: twoFourToSub1Text},
        CC1311P3RGZ: {disable: twoFourToSub1Text},
        LP_CC1311P3: {disable: twoFourToSub1Text},
        LP_EM_CC1314R10: {disable: twoFourToSub1Text},
        CC1314R10RSK: {disable: twoFourToSub1Text}
    },
    LP_CC2652RSIP: {
        CC2652R1FRGZ: {},
        CC2652R1FSIP: {},
        CC26X2R1_LAUNCHXL: {},
        LP_CC2652RSIP: {},
        CC1312R1_LAUNCHXL: {disable: twoFourToSub1Text},
        CC1312R1F3RGZ: {disable: twoFourToSub1Text},
        LP_CC1312R7: {disable: twoFourToSub1Text},
        CC1312R7RGZ: {disable: twoFourToSub1Text},
        CC1311P3RGZ: {disable: twoFourToSub1Text},
        LP_CC1311P3: {disable: twoFourToSub1Text},
        LP_EM_CC1314R10: {disable: twoFourToSub1Text},
        CC1314R10RSK: {disable: twoFourToSub1Text}
    },
    LP_CC2652PSIP: {
        CC2652P1FSIP: {},
        LP_CC2652PSIP: {},
        CC1312R1_LAUNCHXL: {disable: twoFourToSub1Text},
        CC1312R1F3RGZ: {disable: twoFourToSub1Text},
        LP_CC1312R7: {disable: twoFourToSub1Text},
        CC1312R7RGZ: {disable: twoFourToSub1Text},
        CC1311P3RGZ: {disable: twoFourToSub1Text},
        LP_CC1311P3: {disable: twoFourToSub1Text},
        LP_EM_CC1314R10: {disable: twoFourToSub1Text},
        CC1314R10RSK: {disable: twoFourToSub1Text}
    },
    LP_CC2652R7: {
        CC2652R7RGZ: {},
        LP_CC2652R7: {},
        CC1312R1_LAUNCHXL: {disable: twoFourToSub1Text},
        CC1312R1F3RGZ: {disable: twoFourToSub1Text},
        LP_CC1312R7: {disable: twoFourToSub1Text},
        CC1312R7RGZ: {disable: twoFourToSub1Text},
        CC1311P3RGZ: {disable: twoFourToSub1Text},
        LP_CC1311P3: {disable: twoFourToSub1Text},
        LP_EM_CC1314R10: {disable: twoFourToSub1Text},
        CC1314R10RSK: {disable: twoFourToSub1Text}
    },
    LP_CC2651P3: {
        CC2651P3RGZ: {},
        LP_CC2651P3: {},
        CC1312R1_LAUNCHXL: {disable: twoFourToSub1Text},
        CC1312R1F3RGZ: {disable: twoFourToSub1Text},
        LP_CC1312R7: {disable: twoFourToSub1Text},
        CC1312R7RGZ: {disable: twoFourToSub1Text},
        CC1311P3RGZ: {disable: twoFourToSub1Text},
        LP_CC1311P3: {disable: twoFourToSub1Text},
        LP_EM_CC1314R10: {disable: twoFourToSub1Text},
        CC1314R10RSK: {disable: twoFourToSub1Text}
    },
    LP_CC2651R3: {
        CC2651R3RGZ: {},
        LP_CC2651R3: {},
        CC1312R1_LAUNCHXL: {disable: twoFourToSub1Text},
        CC1312R1F3RGZ: {disable: twoFourToSub1Text},
        LP_CC1312R7: {disable: twoFourToSub1Text},
        CC1312R7RGZ: {disable: twoFourToSub1Text},
        CC1311P3RGZ: {disable: twoFourToSub1Text},
        LP_CC1311P3: {disable: twoFourToSub1Text},
        LP_EM_CC1314R10: {disable: twoFourToSub1Text},
        CC1314R10RSK: {disable: twoFourToSub1Text}
    },
    LP_CC2651R3SIPA: {
        CC2651R3SIPA: {},
        LP_CC2651R3SIPA: {},
        CC1312R1_LAUNCHXL: {disable: twoFourToSub1Text},
        CC1312R1F3RGZ: {disable: twoFourToSub1Text},
        LP_CC1312R7: {disable: twoFourToSub1Text},
        CC1312R7RGZ: {disable: twoFourToSub1Text},
        CC1311P3RGZ: {disable: twoFourToSub1Text},
        LP_CC1311P3: {disable: twoFourToSub1Text}
    },
    LP_CC2674R10_FPGA: {
        CC2674R10RGZ: {},
        LP_CC2674R10_FPGA: {}
    },
    LP_CC2653P10: {
        CC2653P10RGZ: {},
        LP_CC2653P10: {},
        CC1312R1_LAUNCHXL: {disable: twoFourToSub1Text},
        CC1312R1F3RGZ: {disable: twoFourToSub1Text},
        LP_CC1312R7: {disable: twoFourToSub1Text},
        CC1312R7RGZ: {disable: twoFourToSub1Text},
        CC1311P3RGZ: {disable: twoFourToSub1Text},
        LP_CC1311P3: {disable: twoFourToSub1Text},
        LP_EM_CC1314R10: {disable: twoFourToSub1Text},
        CC1314R10RSK: {disable: twoFourToSub1Text}
    },

    // Devices
    CC1352R1F3RGZ: {
        CC1352R1F3RGZ: {},
        CC1312R1F3RGZ: {},
        CC1312R1_LAUNCHXL: {},
        CC1352R1_LAUNCHXL: {},
        CC1312R7RGZ: {},
        LP_CC1312R7: {}
    },
    CC1352P1F3RGZ: {
        CC1352P1F3RGZ: {},
        CC1352P1_LAUNCHXL: {},
        CC1352P_2_LAUNCHXL: {},
        CC1352P_4_LAUNCHXL: {}
    },
    CC1352P7RGZ: {
        CC1352P7RGZ: {},
        LP_CC1352P7_1: {},
        LP_CC1352P7_4: {}
    },
    CC1354R10RGZ: {
        CC1354R10RGZ: {},
        LP_CC1354R10_RGZ: {}
    },
    CC1354P10RSK: {
        CC1354P10RSK: {},
        LP_EM_CC1354P10_1: {},
        LP_EM_CC1354P10_6: {}
    },
    LP_EM_CC1354P10_6: {
        CC1354P10RSK: {},
        LP_EM_CC1354P10_6: {}
    },
    CC1314R10RSK: {
        CC1314R10RSK: {},
        LP_EM_CC1314R10: {},
        LP_CC2652RSIP: {disable: sub1To24Text},
        CC26X2R1_LAUNCHXL: {disable: sub1To24Text},
        CC2652R1FSIP: {disable: sub1To24Text},
        CC2652R1FRGZ: {disable: sub1To24Text},
        CC2652R7RGZ: {disable: sub1To24Text},
        LP_CC2652R7: {disable: sub1To24Text},
        LP_CC2651P3: {disable: sub1To24Text},
        LP_CC2651R3: {disable: sub1To24Text},
        CC2651P3RGZ: {disable: sub1To24Text},
        CC2651R3RGZ: {disable: sub1To24Text},
        CC2652P1FSIP: {disable: sub1To24Text},
        LP_CC2652PSIP: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_CC2653P10: {disable: sub1To24Text}
    },
    CC1312R1F3RGZ: {
        CC1312R1F3RGZ: {},
        CC1312R1_LAUNCHXL: {},
        LP_CC2652RSIP: {disable: sub1To24Text},
        CC26X2R1_LAUNCHXL: {disable: sub1To24Text},
        CC2652R1FSIP: {disable: sub1To24Text},
        CC2652R1FRGZ: {disable: sub1To24Text},
        CC2652R7RGZ: {disable: sub1To24Text},
        LP_CC2652R7: {disable: sub1To24Text},
        LP_CC2651P3: {disable: sub1To24Text},
        LP_CC2651R3: {disable: sub1To24Text},
        CC2651P3RGZ: {disable: sub1To24Text},
        CC2651R3RGZ: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_EM_CC2653P10: {disable: sub1To24Text},
        CC2651R3SIPA: {disable: sub1To24Text},
        LP_CC2651R3SIPA: {disable: sub1To24Text},
        CC2651R3RGZ: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_CC2653P10: {disable: sub1To24Text}
    },
    CC1312R7RGZ: {
        CC1312R7RGZ: {},
        LP_CC1312R7: {},
        LP_CC2652RSIP: {disable: sub1To24Text},
        CC26X2R1_LAUNCHXL: {disable: sub1To24Text},
        CC2652R1FSIP: {disable: sub1To24Text},
        CC2652R1FRGZ: {disable: sub1To24Text},
        CC2652R7RGZ: {disable: sub1To24Text},
        LP_CC2652R7: {disable: sub1To24Text},
        LP_CC2651P3: {disable: sub1To24Text},
        LP_CC2651R3: {disable: sub1To24Text},
        CC2651P3RGZ: {disable: sub1To24Text},
        CC2651R3RGZ: {disable: sub1To24Text},
        CC2652P1FSIP: {disable: sub1To24Text},
        LP_CC2652PSIP: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_EM_CC2653P10: {disable: sub1To24Text},
        CC2651R3SIPA: {disable: sub1To24Text},
        LP_CC2651R3SIPA: {disable: sub1To24Text},
        LP_CC2652PSIP: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_CC2653P10: {disable: sub1To24Text}
    },
    CC1312PSIP: {
        CC1312PSIP: {},
        LP_EM_CC1312PSIP: {},
        LP_CC2652RSIP: {disable: sub1To24Text},
        CC26X2R1_LAUNCHXL: {disable: sub1To24Text},
        CC2652R1FSIP: {disable: sub1To24Text},
        CC2652R1FRGZ: {disable: sub1To24Text},
        CC2652R7RGZ: {disable: sub1To24Text},
        LP_CC2652R7: {disable: sub1To24Text},
        LP_CC2651P3: {disable: sub1To24Text},
        LP_CC2651R3: {disable: sub1To24Text},
        CC2651P3RGZ: {disable: sub1To24Text},
        CC2651R3RGZ: {disable: sub1To24Text},
        CC2652P1FSIP: {disable: sub1To24Text},
        LP_CC2652PSIP: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_EM_CC2653P10: {disable: sub1To24Text},
        CC2651R3SIPA: {disable: sub1To24Text},
        LP_CC2651R3SIPA: {disable: sub1To24Text},
        LP_CC2652PSIP: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_CC2653P10: {disable: sub1To24Text}
    },
    CC1311P3RGZ: {
        CC1311P3RGZ: {},
        LP_CC1311P3: {},
        LP_CC2652RSIP: {disable: sub1To24Text},
        CC26X2R1_LAUNCHXL: {disable: sub1To24Text},
        CC2652R1FSIP: {disable: sub1To24Text},
        CC2652R1FRGZ: {disable: sub1To24Text},
        CC2652R7RGZ: {disable: sub1To24Text},
        LP_CC2652R7: {disable: sub1To24Text},
        LP_CC2651P3: {disable: sub1To24Text},
        LP_CC2651R3: {disable: sub1To24Text},
        CC2651P3RGZ: {disable: sub1To24Text},
        CC2651R3RGZ: {disable: sub1To24Text},
        CC2652P1FSIP: {disable: sub1To24Text},
        LP_CC2652PSIP: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_EM_CC2653P10: {disable: sub1To24Text},
        CC2651R3SIPA: {disable: sub1To24Text},
        LP_CC2651R3SIPA: {disable: sub1To24Text},
        LP_CC2652PSIP: {disable: sub1To24Text},
        CC2653P10RGZ: {disable: sub1To24Text},
        LP_CC2653P10: {disable: sub1To24Text}
    },
    CC2652R1FRGZ: {
        CC2652R1FRGZ: {},
        CC2652R1FSIP: {},
        CC26X2R1_LAUNCHXL: {},
        LP_CC2652RSIP: {},
        CC1312R1_LAUNCHXL: {disable: twoFourToSub1Text},
        CC1312R1F3RGZ: {disable: twoFourToSub1Text},
        LP_CC1312R7: {disable: twoFourToSub1Text},
        CC1312R7RGZ: {disable: twoFourToSub1Text},
        CC1311P3RGZ: {disable: twoFourToSub1Text},
        LP_CC1311P3: {disable: twoFourToSub1Text},
        LP_EM_CC1314R10: {disable: twoFourToSub1Text},
        CC1314R10RSK: {disable: twoFourToSub1Text}
    },
    CC2652R1FSIP: {
        CC2652R1FRGZ: {},
        CC2652R1FSIP: {},
        CC26X2R1_LAUNCHXL: {},
        LP_CC2652RSIP: {},
        CC1312R1_LAUNCHXL: {disable: twoFourToSub1Text},
        CC1312R1F3RGZ: {disable: twoFourToSub1Text},
        CC1311P3RGZ: {disable: twoFourToSub1Text},
        LP_CC1311P3: {disable: twoFourToSub1Text},
        LP_EM_CC1314R10: {disable: twoFourToSub1Text},
        CC1314R10RSK: {disable: twoFourToSub1Text}
    },
    CC2652P1FSIP: {
        CC2652P1FSIP: {},
        LP_CC2652PSIP: {},
        CC1312R1_LAUNCHXL: {disable: twoFourToSub1Text},
        CC1312R1F3RGZ: {disable: twoFourToSub1Text},
        LP_CC1312R7: {disable: twoFourToSub1Text},
        CC1312R7RGZ: {disable: twoFourToSub1Text},
        CC1311P3RGZ: {disable: twoFourToSub1Text},
        LP_CC1311P3: {disable: twoFourToSub1Text},
        LP_EM_CC1314R10: {disable: twoFourToSub1Text},
        CC1314R10RSK: {disable: twoFourToSub1Text}
    },
    CC2652P1FRGZ: {
        CC1352P_2_LAUNCHXL: {},
        CC1352P_4_LAUNCHXL: {},
        CC2652P1FRGZ: {}
    },
    CC2652R7RGZ: {
        CC2652R7RGZ: {},
        LP_CC2652R7: {},
        CC1312R1_LAUNCHXL: {disable: twoFourToSub1Text},
        CC1312R1F3RGZ: {disable: twoFourToSub1Text},
        LP_CC1312R7: {disable: twoFourToSub1Text},
        CC1312R7RGZ: {disable: twoFourToSub1Text},
        CC1311P3RGZ: {disable: twoFourToSub1Text},
        LP_CC1311P3: {disable: twoFourToSub1Text},
        LP_EM_CC1314R10: {disable: twoFourToSub1Text},
        CC1314R10RSK: {disable: twoFourToSub1Text}
    },
    CC2651P3RGZ: {
        CC2651P3RGZ: {},
        LP_CC2651P3: {},
        CC1312R1_LAUNCHXL: {disable: twoFourToSub1Text},
        CC1312R1F3RGZ: {disable: twoFourToSub1Text},
        LP_CC1312R7: {disable: twoFourToSub1Text},
        CC1312R7RGZ: {disable: twoFourToSub1Text},
        CC1311P3RGZ: {disable: twoFourToSub1Text},
        LP_CC1311P3: {disable: twoFourToSub1Text},
        LP_EM_CC1314R10: {disable: twoFourToSub1Text},
        CC1314R10RSK: {disable: twoFourToSub1Text}
    },
    CC2651R3RGZ: {
        CC2651R3RGZ: {},
        LP_CC2651R3: {},
        CC1312R1_LAUNCHXL: {disable: twoFourToSub1Text},
        CC1312R1F3RGZ: {disable: twoFourToSub1Text},
        LP_CC1312R7: {disable: twoFourToSub1Text},
        CC1312R7RGZ: {disable: twoFourToSub1Text},
        CC1311P3RGZ: {disable: twoFourToSub1Text},
        LP_CC1311P3: {disable: twoFourToSub1Text},
        LP_EM_CC1314R10: {disable: twoFourToSub1Text},
        CC1314R10RSK: {disable: twoFourToSub1Text}
    },
    CC2651R3SIPA: {
        CC2651R3SIPA: {},
        LP_CC2651R3SIPA: {},
        CC1312R1_LAUNCHXL: {disable: twoFourToSub1Text},
        CC1312R1F3RGZ: {disable: twoFourToSub1Text},
        LP_CC1312R7: {disable: twoFourToSub1Text},
        CC1312R7RGZ: {disable: twoFourToSub1Text},
        CC1311P3RGZ: {disable: twoFourToSub1Text},
        LP_CC1311P3: {disable: twoFourToSub1Text}
    },
    CC2674R10RGZ: {
        LP_CC2674R10_FPGA: {},
        CC2674R10RGZ: {}
    },
    CC2653P10RGZ: {
        CC2653P10RGZ: {},
        LP_CC2653P10: {},
        CC1312R1_LAUNCHXL: {disable: twoFourToSub1Text},
        CC1312R1F3RGZ: {disable: twoFourToSub1Text},
        LP_CC1312R7: {disable: twoFourToSub1Text},
        CC1312R7RGZ: {disable: twoFourToSub1Text},
        CC1311P3RGZ: {disable: twoFourToSub1Text},
        LP_CC1311P3: {disable: twoFourToSub1Text},
        LP_EM_CC1314R10: {disable: twoFourToSub1Text},
        CC1314R10RSK: {disable: twoFourToSub1Text}
    }
};

/*!
 *  ======== convertDeviceToBoard ========
 *  Converts a provided device to a board
 *
 *  @param device - String. Possible device that should be mapped to a board
 *
 *  @returns String - Name of the board with prefix /ti/boards and
 *                    suffix .syscfg.json stripped off.
 */
function convertDeviceToBoard(device)
{
    let name = device;

    // Check if this is a standalone device without a LaunchPad
    if(!name.includes("LAUNCHXL") && !name.includes("LP_"))
    {
        // Find the LaunchPad name in deviceToBoard dictionary
        let key = null;
        for(key in deviceToBoard)
        {
            if(name.includes(key))
            {
                name = deviceToBoard[key];
                break;
            }
        }
    }

    return(name);
}

/*
 * ======== isMigrationValid ========
 * Determines whether a migration from one board/device to another board/device
 * is supported by Prop RF.
 *
 * @returns One of the following Objects:
 *    - {} <--- Empty object if migration is valid
 *    - {warn: "Warning markdown text"} <--- Object with warn property
 *                                           if migration is valid but
 *                                           might require user action
 *    - {disable: "Disable markdown text"} <--- Object with disable property
 *                                              if migration is not valid
 */
function isMigrationValid(currentTarget, migrationTarget)
{
    let migSupported = {
        warn: "This migration requires manual steps and  has not been fully "
        + "tested. See additional documentation for more details"
    };

    if(supportedMigrations[currentTarget]
        && supportedMigrations[currentTarget][migrationTarget])
    {
        migSupported = supportedMigrations[currentTarget][migrationTarget];
    }

    return(migSupported);
}

/*
 * ======== migrate ========
 * Perform stack specific changes to the SysConfig env POST migration
 *
 * @param currTarget - Board/device being migrated FROM
 * @param migrationTarget - Board/device being migrated TO
 * @param env - SysConfig environment providing access to all configurables
 * @param projectName - Optional name of the project being migrated
 *
 * @returns boolean - true when migration is supported and succesful, false when
 *                    migration is not supported and/or unsuccesful
 */
function migrate(currTarget, migrationTarget, env, projectName = null)
{
    const migrationInfo = isMigrationValid(currTarget, migrationTarget);
    let migrationValid = true;
    if(migrationInfo.disable)
    {
        migrationValid = false;
    }

    if(migrationValid)
    {
        /* ======== RF Design Settings ======== */
        const rfDesign = env.system.modules[
            "/ti/devices/radioconfig/rfdesign"].$static;
        const rfDesignSettings = env.system.getScript(
            "/ti/common/lprf_rf_design_settings.js"
        ).rfDesignSettings;

        if(rfDesignSettings.rfDesign !== undefined)
        {
            let setting = null;
            for(setting in rfDesignSettings)
            {
                if(Object.prototype.hasOwnProperty.call(rfDesignSettings,
                    setting))
                {
                    rfDesign[setting] = rfDesignSettings[setting];
                }
            }

            if(env.system.modules["/ti/easylink/easylink"])
            {
                const EasyLink = env.system.modules[
                    "/ti/easylink/easylink"].$static;
                EasyLink.rfDesign = rfDesignSettings.rfDesign;
            }
        }

        /* ======== CCFG Settings ======== */
        const device = env.system.modules["/ti/devices/CCFG"].$static;
        const ccfgSettingObj = env.system.getScript(
            "/ti/common/lprf_ccfg_settings.js"
        ).ccfgSettings;

        let setting = null;
        for(setting in ccfgSettingObj)
        {
            if(Object.prototype.hasOwnProperty.call(ccfgSettingObj, setting))
            {
                device[setting] = ccfgSettingObj[setting];
            }
        }

        const phyInstances = [];
        if(env.system.modules["/ti/easylink/easylink"])
        {
            const easylink = env.system.modules[
                "/ti/easylink/easylink"].$static;
            _.forIn(easylink, (value, key) =>
            {
                if(key.includes("radioConfigEasylink"))
                {
                    phyInstances.push(easylink[key]);
                }
            });
        }
        else
        {
            const customStack = env.system.modules[
                "/ti/devices/radioconfig/custom"].$static;
            _.forIn(customStack, (value, key) =>
            {
                if(key.includes("radioConfig"))
                {
                    phyInstances.push(customStack[key]);
                }
            });
        }

        /*
        * The rfPacketErrorRate projects support both Sub-1GHz and 2.4GHz PHYs.
        * By default, the Sub-1GHz PHYs transmit at 14dBm power requiring
        * CCFG_FORCE_VDDR = 1. The 2.4GHz PHYs do not support boost mode. For
        * both to operate at runtime for this project, the TX power of Sub-1GHz
        * phys must be < 14dBm and CCFG_FORCE_VDDR = 0.
        */
        const boardRepresentation = convertDeviceToBoard(migrationTarget);
        if((projectName === "rfPacketErrorRate"
            || projectName === "rfEasyLinkNp")
            && ((boardRepresentation === "CC1352R1_LAUNCHXL")
            || (boardRepresentation === "CC1352P_2_LAUNCHXL")
            || (boardRepresentation === "CC1352P_4_LAUNCHXL")))
        {
            for(const phy of phyInstances)
            {
                if((phy.txPower) && (phy.txPower === "14"))
                {
                    device.forceVddr = false;
                    phy.txPower = "12.5";
                }
                else if((phy.txPower433) && (phy.txPower433 === "14.5"))
                {
                    device.forceVddr = false;
                    phy.txPower433 = "13";
                }
            }
        }

        /*
        * The rfDualMode projects support both Sub-1GHz and 2.4GHz PHYs. By
        * default, the Sub-1GHz PHYs transmit at 14dBm power requiring
        * CCFG_FORCE_VDDR = 1. The 2.4GHz PHYs do not support boost mode. For
        * both to operate at runtime for this project, the TX power of Sub-1GHz
        * phys must be < 14dBm and CCFG_FORCE_VDDR = 0.
        */
        if((projectName.includes("rfDualMode"))
            && ((boardRepresentation === "CC1352R1_LAUNCHXL")
            || (boardRepresentation === "CC1352P_2_LAUNCHXL")))
        {
            for(const phy of phyInstances)
            {
                if((phy.txPower) && (phy.txPower === "14"))
                {
                    device.forceVddr = false;
                    phy.txPower = "12.5";
                }
            }
        }

        if(boardRepresentation === "CC1312R1_LAUNCHXL"
           || boardRepresentation === "LP_CC1312R7")
        {
            for(const phy of phyInstances)
            {
                if((phy.txPower) && (phy.txPower === "14"))
                {
                    device.forceVddr = false;
                    phy.txPower = "12.5";
                }
            }
        }
    }

    return(migrationValid);
}

/*
 * ======== getMigrationMarkdown ========
 * Returns text in markdown format that customers can use to aid in migrating a
 * project between device/boards. It's recommended to use at most 3 bullets,
 * each line should have a maximum of 120 characters, and a header will be
 * automatically added via the consumer of this API.
 *
 * @param currTarget - Board/device being migrated FROM
 *
 * @returns string - Markdown formatted string
 */
function getMigrationMarkdown(currTarget)
{
    const migrationText = `\
* PHY settings are not common across all devices, verify they match your \
desired application and modify accordingly\n
* Migration from R to P devices requires different rf commands and high pa \
setup\n\
* More Detailed Guidelines: [Link to Docs](/proprietary-rf/proprietary-rf-\
users-guide/sysconfig/proprietary-rf-migration.html#sysconfig-migration-tool)`;

    return(migrationText);
}

/*
 * ======== isValidAddress ========
 * Checks that the address is hex format
 *
 * @param addr    - The address to be validated in string format
 * @returns bool     - True or false depending on if the string format is valid
 */
function isValidAddress(addr)
{
    let isValid = false;

    // Check if the address is in hex format
    if(addr.substring(0, 2) === "0x")
    {
        // Parse the address
        const parsedNum = Number(addr);

        // Check if the parsed number is valid
        if(!isNaN(parsedNum))
        {
            isValid = true;
        }
    }

    return(isValid);
}


/*!
 *  ======== underscoreToCamelCase ========
 *  Convert a string separated by underscores to a camelCase representation
 *
 *  @param underscoreCase  - a string separated by underscores with no spaces
 *
 *  @returns String - the corresponding camelCase representation
 */
function underscoreToCamelCase(underscoreCase)
{
    let camelCase = null;

    // Check the string is not empty and contains no spaces before parsing
    if(underscoreCase.length > 0 && !underscoreCase.includes(" "))
    {
        const splitString = underscoreCase.split("_"); // Split on underscore
        let i = 0;
        for(i = 0; i < splitString.length; i++)
        {
            // Convert first letter of each entry to upper case
            splitString[i] = splitString[i].charAt(0).toUpperCase()
                + splitString[i].slice(1).toLowerCase();
        }

        camelCase = splitString.join("");
    }

    return(camelCase);
}

/*!
 *  ======== device2DeviceFamily ========
 *  Map a pinmux deviceID to a TI-driver device family string
 *
 *  @param deviceId  - a pinmux deviceId (system.deviceData)
 *
 *  @returns String - the corresponding "DeviceFamily_xxxx" string
 *                    used by driverlib header files.
 */
function device2DeviceFamily(deviceId)
{
    let driverString = null;

    /* Determine libraries required by device name. */
    if(deviceId.match(/CC13.2.7/))
    {
        driverString = "DeviceFamily_CC13X2X7";
    }
    else if(deviceId.match(/CC13.2/))
    {
        driverString = "DeviceFamily_CC13X2";
    }
    else if(deviceId.match(/CC13.0/))
    {
        driverString = "DeviceFamily_CC13X0";
    }
    else if(deviceId.match(/CC26.4/))
    {
        driverString = "DeviceFamily_CC26X4";
    }
    else if(deviceId.match(/CC26.3/))
    {
        driverString = "DeviceFamily_CC26X3";
    }
    else if(deviceId.match(/CC13.4/))
    {
        driverString = "DeviceFamily_CC13X4";
    }
    else if(deviceId.match(/CC26.0R2/))
    {
        driverString = "DeviceFamily_CC26X0R2";
    }
    else if(deviceId.match(/CC26.2.7/))
    {
        driverString = "DeviceFamily_CC26X2X7";
    }
    else if(deviceId.match(/CC26.2/))
    {
        driverString = "DeviceFamily_CC26X2";
    }
    else if(deviceId.match(/CC26.0/))
    {
        driverString = "DeviceFamily_CC26X0";
    }
    else if(deviceId.match(/CC3220/))
    {
        driverString = "DeviceFamily_CC3220";
    }
    else if(deviceId.match(/MSP432E.*/))
    {
        driverString = "DeviceFamily_MSP432E401Y";
    }
    else if(deviceId.match(/MSP432P4.1.I/)
            || deviceId.match(/MSP432P4111/))
    {
        driverString = "DeviceFamily_MSP432P4x1xI";
    }
    else if(deviceId.match(/MSP432P4.1.T/))
    {
        driverString = "DeviceFamily_MSP432P4x1xT";
    }
    else if(deviceId.match(/MSP432P401/))
    {
        driverString = "DeviceFamily_MSP432P401x";
    }
    else
    {
        driverString = "";
    }

    return(driverString);
}

/*!
 *  ======== isCName ========
 *  Determine if specified id is either empty or a valid C identifier
 *
 *  @param id  - String that may/may not be a valid C identifier
 *
 *  @returns true if id is a valid C identifier OR is the empty
 *           string; otherwise false.
 */
function isCName(id)
{
    if((id !== null && id.match(/^[a-zA-Z_][0-9a-zA-Z_]*$/) !== null)
            || id === "") /* '' is a special value that means "default" */
    {
        return true;
    }
    return false;
}

/*!
 *  ======== getDeviceOrLaunchPadName ========
 *  Get the name of the board (or device)
 *
 *  @param convertToBoard - Boolean. When true, return the associated LaunchPad
 *                          name if a device is being used without a LaunchPad
 *
 *  @returns String - Name of the board with prefix /ti/boards and
 *                    suffix .syscfg.json stripped off.  If no board
 *                    was specified, the device name is returned.
 */
function getDeviceOrLaunchPadName(convertToBoard)
{
    let name = system.deviceData.deviceId;

    if(system.deviceData.board != null)
    {
        name = system.deviceData.board.source;

        /* Strip off everything up to and including the last '/' */
        name = name.replace(/.*\//, "");

        /* Strip off everything after and including the first '.' */
        name = name.replace(/\..*/, "");
        
    }

    // Check if this is a standalone device without a LaunchPad
    if(convertToBoard && !name.includes("LAUNCHXL") && !name.includes("LP_"))
    {
        name = convertDeviceToBoard(name);
    }

    return(name);
}

/*
 * ======== stringToArray ========
 * Convert a comma separated string to an array
 *
 * @param string       - The string to be converted
 * @returns Array      - An array containing the values separated from string
 *                       without whitespace characters
 */
function stringToArray(string)
{
    let splitString = [];

    // Ignore spaces in user input
    const stringNoSpaces = string.replace(/\s/g, "");

    // Check if the string is empty before parsing
    if(stringNoSpaces.length > 0)
    {
        splitString = stringNoSpaces.split(","); // Split based on comma
    }

    return(splitString);
}

/*
 * ======== addPropPhy ========
 * Adds an instance of a proprietary phy in the "Custom" stack module. Used in
 * RF Driver .syscfg files
 *
 * @param stackInst - Object. Instance of the "Custom" stack
 * @param phy       - Object. The phy containing args and codeExportConfig
 * @param isCustom  - Boolean. Denotes whether the phy added should be custom
 *
 * @returns - Object. The instance of the newly added phy
 */
function addPropPhy(stackInst, phy, isCustom)
{
    // Get the frequency band and phy type of the phy
    const freqBand = phy.args.freqBand;
    const phyType = phy.args["phyType" + freqBand];

    const blackList = ["codeExportConfig", "freqBand", "phyType" + freqBand];

    const phyName = isCustom ? "custom" + freqBand : phyType;

    // Add the instance of the new PHY
    const selectedPhyOptions = stackInst["prop" + freqBand[0]].slice(0);
    selectedPhyOptions.push(phyName);
    stackInst["prop" + freqBand[0]] = selectedPhyOptions;

    // Get the instance of the newly added PHY
    const phyInst = stackInst["radioConfig" + phyName];

    // Set RF settings of the phy instance to match RF Driver project defaults
    let key = null;
    for(key in phy.args)
    {
        if(!blackList.includes(key) || (isCustom && key.includes("phyType")))
        {
            phyInst[key] = phy.args[key];
        }
    }

    // Set code export settings of phy inst to match RF Driver project defaults
    for(key in phy.args.codeExportConfig)
    {
        if(Object.prototype.hasOwnProperty.call(phy.args.codeExportConfig, key))
        {
            phyInst.codeExportConfig[key] = phy.args.codeExportConfig[key];
        }
    }

    // If Custom, remove rx adv command and replace with a standard rx command
    if(isCustom)
    {
        phyInst.codeExportConfig.symGenMethod = "Legacy";
    }

    // If not an IEEE 15.4 command, switch from using cmdPropRxAdv to cmdPropRx
    if(!phyName.includes("154g"))
    {
        // Create a copy of the cmdList and get the index of cmdPropRxAdv
        const exportedCommands = phyInst.codeExportConfig.cmdList_prop.slice(0);
        const index = exportedCommands.indexOf("cmdPropRxAdv");

        let suffix = "";
        if(!isCustom)
        {
            suffix = phyInst.codeExportConfig.rfMode.substring(7);
        }

        if(index > -1)
        {
            exportedCommands.splice(index, 1, "cmdPropRx");
        }
        else if(!exportedCommands.includes("cmdPropRx"))
        {
            exportedCommands.push("cmdPropRx");
        }

        phyInst.codeExportConfig.cmdPropRx = "RF_cmdPropRx" + suffix;
        phyInst.codeExportConfig.cmdList_prop = exportedCommands;
    }

    return(phyInst);
}

/*
 * ======== addBlePhy ========
 * Adds an instance of a ble phy in the "Custom" stack module. Used in
 * RF Driver .syscfg files
 *
 * @param stackInst - Object. Instance of the "Custom" stack
 * @param phy       - Object. The phy containing args and codeExportConfig
 *
 * @returns - Object. The instance of the newly added phy
 */
function addBlePhy(stackInst, phy)
{
    // Add the instance of the new PHY
    const selectedPhyOptions = stackInst.ble.slice(0);
    selectedPhyOptions.push(phy.args.phyType);
    stackInst.ble = selectedPhyOptions;

    const blackList = ["codeExportConfig", "phyType"];

    // Get the instance of the newly added PHY
    const phyInst = stackInst["radioConfig" + phy.args.phyType];

    // Set RF settings of the phy instance to match RF Driver project defaults
    let key = null;
    for(key in phy.args)
    {
        if(!blackList.includes(key))
        {
            phyInst[key] = phy.args[key];
        }
    }

    // Set code export settings of phy inst to match RF Driver project defaults
    for(key in phy.args.codeExportConfig)
    {
        if(Object.prototype.hasOwnProperty.call(phy.args.codeExportConfig, key))
        {
            phyInst.codeExportConfig[key] = phy.args.codeExportConfig[key];
        }
    }

    return(phyInst);
}

 /* ======== addIeeePhy ========
 * Adds an instance of a ble phy in the "Custom" stack module. Used in
 * RF Driver .syscfg files
 *
 * @param stackInst - Object. Instance of the "Custom" stack
 * @param phy       - Object. The phy containing args and codeExportConfig
 *
 * @returns - Object. The instance of the newly added phy
 */
function addIeeePhy(stackInst, phy)
{
    // Add the instance of the new PHY
    const selectedPhyOptions = stackInst.ieee.slice(0);
    selectedPhyOptions.push(phy.args.phyType);
    stackInst.ieee = selectedPhyOptions;

    const blackList = ["codeExportConfig", "phyType"];

    // Get the instance of the newly added PHY
    const phyInst = stackInst["radioConfig" + phy.args.phyType];

    // Set RF settings of the phy instance to match RF Driver project defaults
    let key = null;
    for(key in phy.args)
    {
        if(!blackList.includes(key))
        {
            phyInst[key] = phy.args[key];
        }
    }

    // Set code export settings of phy inst to match RF Driver project defaults
    for(key in phy.args.codeExportConfig)
    {
        if(Object.prototype.hasOwnProperty.call(phy.args.codeExportConfig, key))
        {
            phyInst.codeExportConfig[key] = phy.args.codeExportConfig[key];
        }
    }

    return(phyInst);
}

/*
 * ======== findConfig ========
 * Finds and returns the configurable with the matching provided name
 *
 * @param config     - A module's configurable array
 * @param configName - The name of the configurable to search for
 *
 * @returns - undefined if the configurable is not found, otherwise the entire
 *            configurable object
 */
function findConfig(config, configName)
{
    let enumDef;

    let element = null;
    for(element of config)
    {
        // If the element contains a group, need to search it's configurables
        if("config" in element)
        {
            // Recursively search the sub-groups config array
            enumDef = findConfig(element.config, configName);

            // Stop searching if the configurable was found in the sub-group
            if(enumDef !== undefined)
            {
                break;
            }
        }
        else if(element.name === configName)
        {
            // Stop searching if the current element is the correct configurable
            enumDef = element;
            break;
        }
    }

    return(enumDef);
}

/*
 * ======== getDropDownOptions ========
 * Finds and returns an array of the "name" property of each option available in
 * the drop-down
 *
 * @param inst       - The module instance to search for the configurable in
 * @param configName - The script name of the configurable to search for
 *
 * @returns - empty array if the configurable is not found, otherwise an array
 * of the "name" property of each option available in the drop-down
 */
function getDropDownOptions(inst, configName)
{
    const dropDownOptions = [];

    const enumDef = findConfig(inst.$module.config, configName);

    // Verify the enum was found and it is a drop-down before extracting names
    if(enumDef !== undefined && ("options" in enumDef))
    {
        // Create an array with only the "name" elements
        let option = null;
        for(option of enumDef.options)
        {
            dropDownOptions.push(option.name);
        }
    }

    return(dropDownOptions);
}

exports = {
    isValidAddress: isValidAddress,
    device2DeviceFamily: device2DeviceFamily,
    isCName: isCName,
    getDeviceOrLaunchPadName: getDeviceOrLaunchPadName,
    underscoreToCamelCase: underscoreToCamelCase,
    stringToArray: stringToArray,
    addPropPhy: addPropPhy,
    addBlePhy: addBlePhy,
    addIeeePhy: addIeeePhy,
    getDropDownOptions: getDropDownOptions,
    maxUint32t: maxUint32t,
    maxUint32tRatUsTime: maxUint32tRatUsTime,
    maxUint32tRatMsTime: maxUint32tRatMsTime,
    maxInt8t: maxInt8t,
    minInt8t: minInt8t,
    ccfgSettings: ccfgSettings,
    isMigrationValid: isMigrationValid,
    migrate: migrate,
    getMigrationMarkdown: getMigrationMarkdown
};
