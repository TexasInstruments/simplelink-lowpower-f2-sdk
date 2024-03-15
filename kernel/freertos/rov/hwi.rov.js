/*
 * Copyright (c) 2022-2023, Texas Instruments Incorporated
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

/* global xdc, helperGetPriorityGivenIndex, helperGetEnabledActivePending*/
let Program   = xdc.module('xdc.rov.Program');
let NVICModule = xdc.loadCapsule("nvic.rov.js");

/* eslint-disable-next-line no-unused-vars */
let moduleName = "Hwi";

/* eslint-disable-next-line no-unused-vars */
let viewMap = [
    {name: "Basic", fxn: "getBasicHwi", structName: "BasicHwi"},
    {name: "Detailed", fxn: "getDetailedHwi", structName: "DetailedHwi"},
    {name: "Module", fxn: "getHwiModule", structName: "HwiModule"}
];

function BasicHwi(){
    this.Address      = null;
    this.Name         = null;
    this.InterruptNum = null;
    this.Function     = null;
    this.Arg          = null;
    this.Priority     = null;
}

function DetailedHwi(){
    this.Address       = null;
    this.Name          = null;
    this.InterruptNum  = null;
    this.Function      = null;
    this.Arg           = null;
    this.Priority      = null;
    this.RawPriority   = null;
    this.Enabled       = null;
    this.Active        = null;
    this.Pending       = null;
}

function HwiModule(){
    this.InterruptPending          = null;
    this.PreemptedActiveExceptions = null;
    this.CurrentActive             = null;
    this.PrioPending               = null;
    this.NumPossibleInt            = null;
}

/* ======== getPriority ========
 * Sets the priority of an interrupt. If the argument makeDetailed
 * is true, then this function also sets the interrupts raw priority
 * which refers to all the bits of the priority field. This is the
 * priority shown in the NVIC table module */
function getPriority(hwi, NVIC, makeDetailed){
    /* IPR holds the priority of the interrupts (lower number means higher
     * priority). See page 233 & 234 of version 1b the Cortex M4 Devices
     * Generic User Guide or explanation for how to extract the priority. */

    let rawPriority = helperGetPriorityGivenIndex(hwi.InterruptNum, NVIC);
    hwi.Priority    = rawPriority >> 5;

    if (makeDetailed) {
        hwi.RawPriority = rawPriority;
    }
}

/* ======== getState ========
 * Sets the information concerning if a specific interrupt is
 * enabled, active and/or pending and gets this information
 * from a helper function. */
function getState(hwi, NVIC){
    let [enabled, active, pending] = helperGetEnabledActivePending(hwi.InterruptNum, NVIC);
    hwi.Enabled = enabled;
    hwi.Active  = active;
    hwi.Pending = pending;
}

/* ======== getHwi ========
 * Dispatched entry point for basic and detailed Hwi view */
function getHwi(makeDetailed){
    let view = new Array();

    let dispatchTable = Program.fetchVariable("HwiP_dispatchTable");
    let NVIC = NVICModule.parseNVIC();

    for (let i = 0; i < dispatchTable.length; i++) {
        if (dispatchTable[i] == 0) continue;
        let hwi = makeDetailed ? new DetailedHwi() : new BasicHwi();
        let hwiAddr = dispatchTable[i];
        let obj = Program.fetchFromAddr(hwiAddr, "HwiP_Obj", 1);

        hwi.Address      = hwiAddr;
        hwi.Name         = String(Program.lookupDataSymbol(Number(hwiAddr)));
        hwi.InterruptNum = obj.intNum;
        hwi.Function     = String(Program.lookupFuncName(Number(obj.fxn)));
        hwi.Arg          = "0x" + Number(obj.arg).toString(16);

        getPriority(hwi, NVIC, makeDetailed);
        if (makeDetailed) {
            getState(hwi, NVIC);
        }
        view.push(hwi);
    }

    return view;
}

/* ======== getBasicHwi ========
 * Entry point for basic view. Sets makeDetailed to false */
/* eslint-disable-next-line no-unused-vars */
function getBasicHwi(){
    return getHwi(false);
}

/* ======== getDetailedHwi ========
 * Entry point for detailed view. Sets makeDetailed to true */
/* eslint-disable-next-line no-unused-vars */
function getDetailedHwi(){
    return getHwi(true);
}

/* ======== getHwiModule ========
 * Main function for getting the Hwi module view */
/* eslint-disable-next-line no-unused-vars */
function getHwiModule(){

    let view = new Array();
    let NVIC = NVICModule.parseNVIC();

    // Fetch dispatchTable in order to find NUM_INTERRUPTS
    let dispatchTable = Program.fetchVariable("HwiP_dispatchTable");
    let maxInterrupts = dispatchTable.length;

    let hwiMod = new HwiModule();

    /* See page 230 of version 1b of Cortex M4 Devices Generic User Guide */
    hwiMod.InterruptPending          = Boolean(NVIC.ICSR & (1 << 22));
    hwiMod.PreemptedActiveExceptions = NVIC.ICSR & (1 << 11) ? false : true;
    hwiMod.NumPossibleInt            = maxInterrupts;
    let currentActive                = NVIC.ICSR & 0xff;
    hwiMod.CurrentActive = (currentActive != 0) ? "0x" + Number(currentActive).toString(16) : "None";
    let prioPending                  = (NVIC.ICSR & (0xff << 12)) >> 12;
    hwiMod.PrioPending = (prioPending != 0) ? "0x" + Number(prioPending).toString(16) : "None";

    view.push(hwiMod);
    return view;
}