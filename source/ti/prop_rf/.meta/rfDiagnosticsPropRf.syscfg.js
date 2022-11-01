/*
 * Copyright (c) 2022, Texas Instruments Incorporated
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
 
/* This file provides Prop RF configuration for rfDiagnostics, to be loaded in a .sycfg file
 */
 
/* ======== Radio Configuration ======== */
const commonRf = system.getScript("/ti/prop_rf/prop_rf_common.js");
const boardName = commonRf.getDeviceOrLaunchPadName(true);
const phyFilePath = "/ti/prop_rf/rf_config/" + boardName + "_rf_defaults.js";

// Get the supported proprietary phys for this board as defined by EasyLink
const supportedPropPhys = system.getScript(phyFilePath).defaultPropPhyList;
const supportedIeeePhys = system.getScript(phyFilePath).default154PhyList;

// Add an instance of the "Custom" RF Stack
const customStack = scripting.addModule("/ti/devices/radioconfig/custom");
const phyInstances = [];

/*
 * Iterate over all the proprietary phys supported by this board adding them to
 * the configuration
 */
for(let phy of supportedPropPhys)
{
    if(phy.args.$name === "RF_2GFSK50KBPS_Setting" ||  phy.args.$name === "RF_2GFSK250KBPS_Setting")
    {
        phyInstances.push(commonRf.addPropPhy(customStack, phy, false));
    }
}

if(boardName != "CC1312R1_LAUNCHXL" || boardName != "LP_CC1311P3" || boardName != "LP_CC1312R7" || boardName != "LP_CC1314R10")
{
    for(let phy of supportedIeeePhys)
    {
        phyInstances.push(commonRf.addIeeePhy(customStack, phy));
    }
}

/* ======== RF Design ======== */
var rfDesign = scripting.addModule("ti/devices/radioconfig/rfdesign");
const rfDesignSettings = system.getScript("/ti/common/lprf_rf_design_settings.js").rfDesignSettings;
for(var setting in rfDesignSettings)
{
    rfDesign[setting] = rfDesignSettings[setting];
}

/* ======== Device ======== */
var device = scripting.addModule("ti/devices/CCFG");
const ccfgSettings = system.getScript("/ti/common/lprf_ccfg_settings.js").ccfgSettings;
for(var setting in ccfgSettings)
{
    device[setting] = ccfgSettings[setting];
}

if(boardName === "CC1352R1_LAUNCHXL" || boardName === "CC1352P_2_LAUNCHXL")
{
	device.forceVddr = false;
    customStack.radioConfig2gfsk50kbps.txPower = "12.5";
}