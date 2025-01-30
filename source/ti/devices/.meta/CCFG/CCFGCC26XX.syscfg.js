/*
 * Copyright (c) 2019-2024, Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== CCFGCC26XX.syscfg.js ========
 */

const Common = system.getScript("/ti/drivers/Common.js");

const device = system.deviceData.deviceId;
const board = system.deviceData.board;
const isBAW = device.match(/CC2652RB/) !== null;
const isSIP = device.match(/SIP/) !== null;
const isM33 = device.match(/CC(?:13|26).[34]/) !== null;
const isCC1312PSIP = device.match(/CC1312PSIP/) !== null;

const NUM_PINS = 48;
var isBAWBoard = false;

if (board != null) {
    isBAWBoard = board.name.match(/CC2652RB/) !== null;
}

const moduleDesc = `
The [__CCFG area__][1] is a dedicated flash memory region and must contain a
Customer Configuration section (CCFG) that is used by boot ROM and TI provided
drivers to configure the device. This configuration is done by simply letting
Syscfg generate the file ti_devices_config.c and include it in the project.

* [Detailed Field Description][2]
* [API access][3]

[1]: /driverlib/cc13x2_cc26x2/driverlib/index.html#ccfg "Basic usage summary"
[2]: /driverlib/cc13x2_cc26x2/register_descriptions/CPU_MMAP/CCFG.html "CCFG field description"
[3]: /driverlib/cc13x2_cc26x2/driverlib/group__ccfgread__api.html "CCFG access API"`;

let devSpecific = {
    templates : {
        board_initc : "/ti/devices/CCFG.Board_init.c.xdt",
        boardh : "/ti/devices/CCFG.Board.h.xdt"
    },
    longDescription: moduleDesc,
    moduleStatic: {

        modules: modules,
        validate: validate,
        config : [
            // Power settings
            {
                name: "forceVddr",
                displayName: "Force VDDR",
                description: "Necessary for external load mode, or for maximum PA output power",
                default: false,
                readOnly: false,
                hidden: false
            },
            {
                name: "enableDCDC",
                displayName: "Enable DCDC",
                description: "Enable or disable use of DCDC.",
                readOnly: false,
                hidden: false,
                default: true
            },
            // Clock settings
            {
                name: "srcClkLF",
                displayName: "LF Clock Source",
                description: "Low frequency clock source",
                readOnly: false,
                hidden: false,
                options: [
                    {name: "Derived from HF XOSC"},
                    {name: "External LF clock"},
                    {name: "LF XOSC"},
                    {name: "LF RCOSC"}
                ],
                // The default LF clock source is set to LF RCOSC, only if LP_CC2652RB
                // board is selected. In all other cases, including the selection of
                // CC2652RB device(without the board), the default remains as LF XOSC.
                // This reflects the default absence of LF crystal on the LP_CC2652RB
                // board.
                default: isBAWBoard ? "LF RCOSC" : "LF XOSC",
                onChange: onChangesrcClkLF
            },
            {
                name: "extClkDio",
                displayName: "External LF Clock DIO",
                description: "DIO number if using external LF clock",
                readOnly: false,
                hidden: true,
                displayFormat: {radix: "dec", fixedPoint: 0},
                default: 1
            },
            {
                name: "rtcIncrement",
                displayName: "RTC Increment",
                description: "RTC increment for the external LF clock frequency",
                readOnly: false,
                hidden: true,
                displayFormat: "hex",
                default: 0x800000
            },
            {
                name: "xoscCapArray",
                displayName: "XOSC Cap Array Modification",
                description: "Enables modification (delta) to XOSC cap-array",
                default: false,
                readOnly: false,
                hidden: false,
                onChange: (inst, ui) => {
                    if (inst.xoscCapArray === true) {
                        ui.xoscCapArrayDelta.hidden = false;
                    }
                    else {
                        ui.xoscCapArrayDelta.hidden = true;
                    }
                }
            },
            {
                name: "xoscCapArrayDelta",
                displayName: "XOSC Cap Array Delta",
                description: "Modify the high frequency oscillator cap-array, changing the frequency offset",
                longDescription: "The cap-array delta value is an offset from the default value of the load"
                    + " capacitor on the high frequency oscillator.\n\n"
                    + " More information can be found in the application report [CC13xx/CC26xx Hardware"
                    + " Configuration and PCB Design Considerations]"
                    + "(http://www.ti.com/general/docs/lit/getliterature.tsp?baseLiteratureNumber=swra640)",
                displayFormat: {radix: "hex", bitSize: 2},
                default: 0xFF,
                readOnly: false,
                hidden: true
            },
            // Special HF clock source setting
            {
                name: "srcClkHF",
                displayName: "HF Clock Source",
                description: "High Frequency Clock Source",
                readOnly: isBAW,
                hidden: false,
                options: isBAW
                    ? [
                        {name: "Internal High Precision Oscillator"}
                    ] : [
                        {name: "48 MHz XOSC_HF"},
                        {name: "External 48Mhz TCXO"}
                    ],
                default: isBAW ? "Internal High Precision Oscillator" : "48 MHz XOSC_HF",
                onChange: (inst, ui) => {
                    if (inst.srcClkHF === "External 48Mhz TCXO") {
                        ui.tcxoType.hidden = false;
                        ui.tcxoMaxStartup.hidden = false;
                    }
                    else {
                        ui.tcxoType.hidden = true;
                        ui.tcxoMaxStartup.hidden = true;
                    }
                }
            },
            {
                name: "tcxoType",
                displayName: "TCXO Type",
                description: "Sets the TCXO Type",
                longDescription: "Sets the Temperature Compensated Crystal Oscillator type\n\n"
                    + "* _CMOS type:_ Internal common-mode bias will not be enabled\n"
                    + "* _Clipped-sine type:_ Internal common-mode bias will be enabled when TCXO is used.",
                readOnly: false,
                hidden: true,
                options: [
                    {name: "CMOS Type"},
                    {name: "Clipped-sine Type"}
                ],
                default: "CMOS Type"
            },
            {
                name: "tcxoMaxStartup",
                displayName: "TCXO Max Startup Time",
                description: "Maximum Temperature Compensated Crystal Oscillator startup time in units of 100us",
                displayFormat: {radix: "hex", bitSize: 2},
                readOnly: false,
                hidden: true,
                default: 0x7F
            },
            // RF temperature compensation
            {
                name: "enableXoscHfComp",
                displayName: "RF Temperature Compensation",
                description: "Compensate XOSC_HF frequency for temperature during radio transmissions.",
                longDescription: "Compensate XOSC_HF frequency for temperature during radio transmissions. This improves the accuracy of the XOSC_HF over temperature. This should only be enabled if the selected XOSC_HF source is not accurate enough for the selected stack. It is primarily needed when using HPOSC or when running a stack that requires the XOSC_HF to be compensated for temperature",
                readOnly: isBAW,
                hidden: false,
                default: (isBAW || isCC1312PSIP) ? true : false,
                onChange: (inst, ui) => {
                    ui.useFcfgXoscHfInsertion.hidden = (inst.enableXoscHfComp === false) || isBAW || !isSIP;
                    ui.xoscSinglePointCalibration.hidden = (inst.useFcfgXoscHfInsertion === true) || (inst.enableXoscHfComp === false) || isBAW;
                }
            },
            {
                name: "useFcfgXoscHfInsertion",
                displayName: "Use FCFG XOSC_HF Calibration",
                description: "Use XOSC_HF single point temperature calibration measurement stored in FCFG.",
                longDescription: "Some devices, such as the SIP modules, have an XOSC_HF single point temperature calibration measurement programmed into FCFG. It is recommended to use this setting if it is available. It may be deselected to provide application-specified calibration measurements for debug purposes.",
                readOnly: false,
                hidden: isCC1312PSIP ? false : true,
                default: isSIP,
                onChange: (inst, ui) => {
                    ui.xoscSinglePointCalibration.hidden = (inst.useFcfgXoscHfInsertion === true);
                }
            },
            {
                name: "xoscSinglePointCalibration",
                displayName: "XOSC Single Point Calibration",
                description: "XOSC_HF single point temperature calibration measurement used to characterize the XOSC_HF.",
                readOnly: false,
                hidden: true,
                default: ""
            },
            // RTC temperature compensation
            {
                name        : "enableXoscLfComp",
                displayName : "RTC XOSC LF Compensation",
                description : "Enable XOSC_LF compensation for the real-time clock",
                default     : false,
                onChange    : onChangeenableXoscLfComp
            },
            {
                name        : "RTCCustomXOSCLFCoeff",
                displayName : "Custom RTC XOSC Coefficients",
                description : "Use Custome RTC Coefficients",
                longDescription:`
        If the ppm offset of the XOSC_LF can be described by a second order polynomial function of temperature,
        a*T^2 + b*T + c, where the coefficients a, b, and c are known, they can be supplied below.
        `,
                default     : false,
                hidden      : true,
                onChange    : onChangeRTCCustomXOSCLFCoeff
            },
            {
                name        : "rtcXOSCLFCoefficientA",
                displayName : "XOSC LF Coefficient A",
                description : "XOSC LF Coefficient A",
                default     : -0.035,
                hidden      : true
            },
            {
                name        : "rtcXOSCLFCoefficientB",
                displayName : "XOSC LF Coefficient B",
                description : "XOSC LF Coefficient B",
                default     : 1.75,
                hidden      : true
            },
            {
                name        : "rtcXOSCLFCoefficientC",
                displayName : "XOSC LF Coefficient C",
                description : "XOSC LF Coefficient C",
                default     : -21.875,
                hidden      : true
            },
            // Bootloader
            {
                name: "enableBootloader",
                displayName: "Enable Bootloader",
                description: "Enable ROM Bootloader, for flashing from external host",
                default: false,
                onChange: (inst, ui) => {
                    if (inst.enableBootloader === true) {
                        ui.enableBootloaderBackdoor.hidden = false;
                        if (inst.enableBootloaderBackdoor === true) {
                            ui.dioBootloaderBackdoor.hidden = false;
                            ui.levelBootloaderBackdoor.hidden = false;
                        }
                    }
                    else {
                        ui.enableBootloaderBackdoor.hidden = true;
                        ui.dioBootloaderBackdoor.hidden = true;
                        ui.levelBootloaderBackdoor.hidden = true;
                    }
                }
            },
            {
                name: "enableBootloaderBackdoor",
                displayName: "Enable Bootloader Backdoor",
                description: "Enables bootloader backdoor",
                longDescription: "When enabling the Bootloader Backdoor, the "
                    + "Bootloader can be activated externally by pulling a pin, "
                    + "even when a valid flash image is present. "
                    + "Note, enabling the backdoor allows an external host to "
                    + "read back a flash image from the device",
                readOnly: false,
                hidden: true,
                default: false,
                onChange: (inst, ui) => {
                    if (inst.enableBootloaderBackdoor === true) {
                        ui.dioBootloaderBackdoor.hidden = false;
                        ui.levelBootloaderBackdoor.hidden = false;
                    }
                    else {
                        ui.dioBootloaderBackdoor.hidden = true;
                        ui.levelBootloaderBackdoor.hidden = true;
                    }
                }
            },
            {
                name: "dioBootloaderBackdoor",
                displayName: "Bootloader Backdoor DIO",
                description: "DIO (pin) to trigger Bootloader backdoor, "
                    + "this pin is only needed at boot",
                readOnly: false,
                hidden: true,
                displayFormat: {radix: "dec", fixedPoint: 0},
                default: 32
            },
            {
                name: "levelBootloaderBackdoor",
                displayName: "Trigger Level of Bootloader Backdoor",
                description: "Active high or low on selected DIO to open "
                    + "boot loader backdoor",
                readOnly: false,
                hidden: true,
                options: [
                    {name: "Active high"},
                    {name: "Active low"}
                ],
                default: "Active high"
            },
            // Alternative IEEE 802.15.4 MAC address
            {
                name: "configureIEEE",
                displayName: "Configure IEEE MAC Address",
                description: "Configure alternative IEEE 802.15.4 MAC address",
                readOnly: false,
                hidden: false,
                default: false,
                onChange: (inst, ui) => {
                    ui.addressIEEE.hidden = (inst.configureIEEE === false);
                }
            },
            {
                name: "addressIEEE",
                displayName: "IEEE MAC Address",
                description: "Alternative IEEE 802.15.4 MAC address",
                textType: "mac_address_64",
                readOnly: false,
                hidden: true,
                default: "ff:ff:ff:ff:ff:ff:ff:ff"
            },
            // Alternative BLE address
            {
                name: "configureBLE",
                displayName: "Configure BLE Address",
                description: "Configure alternative Bluetooth Low Energy MAC address",
                readOnly: false,
                hidden: false,
                default: false,
                onChange: (inst, ui) => {
                    ui.addressBLE.hidden = (inst.configureBLE === false);
                }
            },
            {
                name: "addressBLE",
                displayName: "BLE Address",
                description: "Alternative BLE address",
                textType: "mac_address_64",
                readOnly: false,
                hidden: true,
                default: "ff:ff:ff:ff:ff:ff:ff:ff"
            },
            // Select between cache or GPRAM
            {
                name: "disableCache",
                displayName: "Disable Flash Cache",
                description: "Disable Flash Cache, making GPRAM available "
                    + "for alternative use",
                readOnly: false,
                hidden: false,
                default: false
            },
            //Alternative address of vector table of the Flash Image
            {
                name: "setFlashVectorTable",
                displayName: "Set Address Of Flash Vector Table",
                description: "Set the address value of the flash vector table to enable the boot "
                    + "FW in ROM to transfer control to a flash image.",
                longDescription: "On reset, the software in the ROM of the device will execute "
                    + "basic startup routines, and then jump to the location pointed to by this field. "
                    + "Most standard Flash images will have the vector table located at address 0x00000000. \n\n"
                    + "NOTE: Any illegal vector table start address value will force the boot FW in ROM "
                    + "to transfer control to the serial boot loader in ROM. ",
                readOnly: false,
                hidden: false,
                default: false,
                onChange: (inst, ui) => {
                    ui.addressFlashVectorTable.hidden = (inst.setFlashVectorTable === false);
                }
            },
            {
                name: "addressFlashVectorTable",
                displayName: "Address of Flash Vector Table",
                description: "Set the address of the vector table of the flash image ",
                readOnly: false,
                hidden: true,
                displayFormat: {radix: "hex", bitSize: 32},
                default: 0x00000000
            },
            /////////////////////////////////////
            // This section contains settings for for M33 cores with TrustZone
            {
                name: "disableRamParity",
                displayName: "Disable RAM Parity",
                description: "Enables or disables parity checking on SRAM.",
                hidden: !isM33,
                default: false
            },
            {
                displayName: "Security boundary configuration",
                description: "Flash and SRAM",
                config: [
                    {
                        name: "markAllFlashSecure",
                        displayName: "Mark All Flash Secure-only",
                        description: "Configure all of flash for secure-only access.",
                        longDescription: `Configuring all of flash to secure-only requires a special configuration value, tick
                this box to enable it.`,
                        hidden: true,
                        default: isM33
                    }
                ]
            },
            {
                displayName: "Bus Security",
                description: "Access Locks",

                config: [
                    {
                        name: "lockNonSecureVectorTableBaseAddress",
                        displayName: "Lock Non-Secure Vector Table Address",
                        description: "Disables write access to non-secure vector table base address.",
                        hidden: true,
                        default: false
                    },
                    {
                        name: "lockSecureInterruptConfig",
                        displayName: "Lock Secure Interrupt Configuration",
                        description: "Disables write access to the secure interrupt configuration.",
                        hidden: true,
                        default: false
                    },
                    {
                        name: "lockSAURegions",
                        displayName: "Lock SAU Regions",
                        description: "Disables write access to the Security Attribution Unit (SAU) region configuration.",
                        hidden: true,
                        default: false
                    },
                    {
                        name: "lockNonSecureMPU",
                        displayName: "Lock Non-Secure MPU",
                        description: "Disables write access to the non-secure memory protection unit.",
                        hidden: true,
                        default: false
                    },
                    {
                        name: "lockSecureMPU",
                        displayName: "Lock Secure MPU",
                        description: "Disables write access to the secure memory protection unit.",
                        hidden: true,
                        default: false
                    },
                    {
                        name: "disableSecureNonInvasiveDebug",
                        displayName: "Disable Invasive Non-Secure Debug",
                        description: `Disables debug invasive debugging of non-secure bus controllers with actions that
        involve stopping execution, modifying registers, or reading from and writing to memory using the core.`,
                        hidden: true,
                        default: false
                    },
                    {
                        name: "disableSecureInvasiveDebug",
                        displayName: "Disable Invasive Secure Debug",
                        description: `Disables debug invasive debugging of secure bus controllers with actions that
        involve stopping execution, modifying registers, or reading from and writing to memory using the core.`,
                        hidden: true,
                        default: false
                    }
                ]
            },
            // End of M33 with Trustzone section
            /////////////////////////////////////

            // Debug access
            {
                displayName: "Debug Access",
                description: "Debug access options",
                config: [
                    {
                        name: "FailureAnalysis",
                        displayName: "TI Failure Analysis",
                        description: "TI Failure Analysis",
                        longDescription: "If enabled, it is possible for TI to unlock access to the "
                            + "DAP and all TAPs on the device with an unlock key to perform failure analysis",
                        readOnly: false,
                        hidden: false,
                        default: false
                    },
                    {
                        name: "FailureAnalysisCustomerKey",
                        displayName: "TI Failure Analysis Customer Key",
                        description: "Enable customer key to be XOR'ed with unlock key during TI Failure Analysis",
                        readOnly: false,
                        hidden: !isM33,
                        default: false,
                        onChange: (inst, ui) => {
                            ui.FailureAnalysisCustomerKeyValue.hidden = (inst.FailureAnalysisCustomerKey === false);
                        }
                    },
                    {
                        name: "FailureAnalysisCustomerKeyValue",
                        displayName: "128-bit Customer Key",
                        description: "128-bit customer key XOR'ed with unlock key to enter TI Failure Analysis",
                        readOnly: false,
                        hidden: true,
                        default: "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
                    },
                    {
                        name: "CpuDap",
                        displayName: "CPU DAP",
                        description: "CPU Debug Access Port, used for memory access during debugging",
                        readOnly: false,
                        hidden: false,
                        default: true
                    },
                    {
                        name: "PwrprofTap",
                        displayName: "PWRPROF TAP",
                        description: "PWRPROF Test Access Port, used for power profiling, "
                            + "measuring of current consumption and more",
                        readOnly: false,
                        hidden: false,
                        default: true
                    },
                    {
                        name: "TestTap",
                        displayName: "TEST TAP",
                        description: "TEST TAP, For TI internal use",
                        readOnly: false,
                        hidden: false,
                        default: false
                    },
                    {
                        name: "Pbist2Tap",
                        displayName: "PBIST2 TAP",
                        description: "PBIST2 TAP, For TI internal use",
                        readOnly: false,
                        hidden: false,
                        default: false
                    },
                    {
                        name: "Pbist1Tap",
                        displayName: "PBIST1 TAP",
                        description: "PBIST1 TAP, For TI internal use",
                        readOnly: false,
                        hidden: false,
                        default: false
                    },
                    {
                        name: "AonTap",
                        displayName: "AON TAP",
                        description: "Always On Test Access Port, used for flash and chip erase",
                        readOnly: false,
                        hidden: false,
                        default: false
                    }
                ]
            }
        ]
    }
};

/*
 *  ======== onChangesrcClkLF ========
 *  onChange callback function for the srcClkLF config
 */
function onChangesrcClkLF(inst, ui)
{
    ui.extClkDio.hidden = !(inst.srcClkLF === "External LF clock");
    ui.rtcIncrement.hidden = !(inst.srcClkLF === "External LF clock");
    /* CLK_LF derived from XOSC_LF? */
    let subState = !(inst.srcClkLF === "LF XOSC");
    /* If no, hide XOSC_LF compensation option */
    ui.enableXoscLfComp.hidden = subState;
    /* CLK_LF derived from XOSC_LF, and XOSC_LF compensation enabled? */
    subState = subState || (inst.enableXoscLfComp == false);
    /* If no, hide option to choose custom compensation coefficients */
    ui.RTCCustomXOSCLFCoeff.hidden = subState;
    subState = subState || (inst.RTCCustomXOSCLFCoeff == false);
    /* Hide custom coefficient values, if custom coefficients are not enabled */
    ui.rtcXOSCLFCoefficientA.hidden = subState;
    ui.rtcXOSCLFCoefficientB.hidden = subState;
    ui.rtcXOSCLFCoefficientC.hidden = subState;
}

/*
 *  ======== onChangeenableXoscLfComp ========
 *  onChange callback function for the enableXoscLfComp config
 */
function onChangeenableXoscLfComp(inst, ui)
{
    let subState = (inst.enableXoscLfComp == false);
    ui.RTCCustomXOSCLFCoeff.hidden = subState;
    subState = subState || (inst.RTCCustomXOSCLFCoeff == false);
    ui.rtcXOSCLFCoefficientA.hidden = subState;
    ui.rtcXOSCLFCoefficientB.hidden = subState;
    ui.rtcXOSCLFCoefficientC.hidden = subState;

}

/*
 *  ======== onChangeRTCCustomXOSCLFCoeff ========
 *  onChange callback function for the RTCCustomXOSCLFCoeff config
 */
function onChangeRTCCustomXOSCLFCoeff(inst, ui)
{
    let subState = (inst.RTCCustomXOSCLFCoeff == false);
    ui.rtcXOSCLFCoefficientA.hidden = subState;
    ui.rtcXOSCLFCoefficientB.hidden = subState;
    ui.rtcXOSCLFCoefficientC.hidden = subState;
}

/*!
 *  ======== validate ========
 *  Validate this module's configuration
 *
 *  @param inst       - RF Settings instance to be validated
 *  @param validation - Issue reporting object
 */
function validate(inst, validation) {
    const MIN_DIO = 1;
    const MAX_DIO = NUM_PINS;

    // Check that the single point insertion value exists
    if (inst.enableXoscHfComp === true &&
        inst.useFcfgXoscHfInsertion === false &&
        !isBAW &&
        inst.xoscSinglePointCalibration === "") {
        Common.logError(validation, inst, "xoscSinglePointCalibration",
            "No XOSC_HF calibration temperature measurement provided!");
    }

    // DIO check Ext Clk DIO
    if (inst.extClkDio < MIN_DIO || inst.extClkDio > MAX_DIO) {
        Common.logError(validation, inst, "extClkDio",
            "Valid range for integer: " + MIN_DIO + " to " + MAX_DIO);
    }

    // DIO check backdoor
    if (inst.dioBootloaderBackdoor < MIN_DIO
        || inst.dioBootloaderBackdoor > MAX_DIO) {
        Common.logError(validation, inst, "dioBootloaderBackdoor",
            "Valid range for integer: " + MIN_DIO + " to " + MAX_DIO);
    }

    // RTC increment check
    if (inst.rtcIncrement < 0 || inst.rtcIncrement > 0xffffffff) {
        Common.logError(validation, inst, "rtcIncrement",
            "Valid range: 0 to 0xffffffff");
    }

    // XOSC Cap array delta
    if (inst.xoscCapArrayDelta < 0 || inst.xoscCapArrayDelta > 0xFF) {
        Common.logError(validation, inst, "xoscCapArrayDelta",
            "Valid range: 0 to 0xFF");
    }

    // TCXO Max Startup
    if (inst.tcxoMaxStartup < 0 || inst.tcxoMaxStartup > 0xFF) {
        Common.logError(validation, inst, "tcxoMaxStartup",
            "Valid range: 0 to 0xFF");
    }

    // Flash Vector table address check
    if (inst.addressFlashVectorTable < 0 || inst.addressFlashVectorTable > 0xFFFFFFFF) {
        Common.logError(validation, inst, "addressFlashVectorTable",
            "Valid range: 0 to 0xFFFFFFFF");
    }

    // Ensure a valid LF clock source for BAW launchpad
    if (inst.srcClkLF == "LF XOSC" && isBAWBoard) {
        Common.logWarning(validation, inst, "srcClkLF",
            "The LP_CC2652RB launchpad does not contain an LF crystal by default.\
            Make sure to mount an OSC before using this option");
    }

    // Customer key must be 128-bit hex value
    if (!(inst.FailureAnalysisCustomerKeyValue.match(/^[a-fA-F0-9]{32}$/))) {
        Common.logError(validation, inst, "FailureAnalysisCustomerKeyValue",
            "Must be 128-bit hex-formatted value");
    }
}

/*
 *  ======== modules ========
 */
function modules(inst) {
    let tmpModules = [];

    // If XOSC_LF RTC compensation is enabled, include the temperature driver.
    // If SCLK_LF is derived from HPOSC, tell the power driver it needs to
    // include the Temperature driver and setup the RTC compensation
    // Alternatively, if RF temperature compensation is enabled, always
    // include temperature regardless of SCLK_LF source.
    // This is done to avoid unconditional inclusion of temperature in the RF
    // module, see https://jira.itg.ti.com/browse/RFDRIVER-474
    if (((inst.srcClkHF === "Internal High Precision Oscillator") &&
         (inst.srcClkLF === "Derived from HF XOSC")) ||
         (inst.enableXoscHfComp === true) ||
         (inst.enableXoscLfComp === true)) {

        tmpModules.push({
                            name: "Temperature",
                            moduleName: "/ti/drivers/Temperature"
                         });
    }
    return tmpModules;
}

/*
 *  ======== extend ========
 */
function extend(base)
{
    /* merge and overwrite base module attributes */
    let result = Object.assign({}, base, devSpecific);

    /* moduleInstances should be retained from base */
    result.moduleStatic.moduleInstances = base.moduleStatic.moduleInstances;

    /* concatenate device-specific configs */
    result.moduleStatic.config = base.moduleStatic.config.concat(devSpecific.moduleStatic.config);

    return (result);
}

/*
 *  ======== exports ========
 *  Export device-specific extensions to base exports
 */
exports = {
    /* required function, called by base CCFG module */
    extend: extend
};
