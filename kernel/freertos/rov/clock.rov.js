/*
 * Copyright (c) 2022-2024, Texas Instruments Incorporated
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

/* global xdc, helperGetHexString, helperGetCurrentDevice */
let Program = xdc.module('xdc.rov.Program');
let TimerModule = xdc.loadCapsule("timer.rov.js");

/* eslint-disable-next-line no-unused-vars */
let moduleName = "Clock";

/* eslint-disable-next-line no-unused-vars */
let viewMap = [
    {name: "Basic", fxn: "getBasicClock", structName: "BasicClock"},
    {name: "Detailed", fxn: "getDetailedClock", structName: "DetailedClock"},
    {name: "Basic Module", fxn: "getBasicModule", structName: "BasicModule"},
    {name: "Detailed Module", fxn: "getDetailedModule", structName: "DetailedModule"}
];

function BasicClock(){
    this.Address     = null;
    this.Symbol      = null;
    this.Function    = null;
    this.Arg         = null;
}

function DetailedClock(){
    this.Address     = null;
    this.Symbol      = null;
    this.Function    = null;
    this.Arg         = null;
    this.Active      = null;
    this.Timeout     = null;
    this.Period      = null;
    this.CurrTimeout = null;
    this.Periodic    = null;
}

function BasicModule(){
    this.Ticks               = null;
    this.Started             = null;
    this.NextScheduledTick   = null;
    this.InWorkFxn           = null;
    this.StartDuringWorkFxn  = null;
}

function DetailedModule(){
    this.Address             = null;
    this.Ticks               = null;
    this.Started             = null;
    this.NumTickSkip         = null;
    this.NextScheduledTick   = null;
    this.MaxSkippable        = null;
    this.InWorkFxn           = null;
    this.StartDuringWorkFxn  = null;
    this.Ticking             = null;
}

/* ======== handleListOfClockObj ========
 * Takes a list of addresses and adds these clocks
 * to the view.
 */
function handleListOfClockObj(view, list, makeDetailed, device){

    for (let i = 0; i < list.length; i++) {
        let clockAddress = list[i];

        let obj;
        if (device != "Unknown") {
            obj = Program.fetchFromAddr(clockAddress, "ClockP_Obj");
        }
        else {
            obj = Program.fetchFromAddr(clockAddress, "ClockP_FreeRTOSObj");
        }

        let clock = makeDetailed ? new DetailedClock() : new BasicClock();

        clock.Symbol   = String(Program.lookupDataSymbol(Number(clockAddress)));
        clock.Address  = clockAddress;
        clock.Function = String(Program.lookupFuncName(Number(obj.fxn)));
        clock.Arg      = helperGetHexString(obj.arg);

        if (makeDetailed) {
            if (device != "Unknown") {
                clock.Active = Boolean(obj.active);
            }

            clock.Timeout  = obj.timeout;
            clock.Period   = obj.period;
            clock.Periodic = Boolean(clock.Period > 0);
            /* Small naming difference between ClockP_FreeRTOSObj and ClockP_Obj */
            clock.CurrTimeout = (device != "Unknown") ? obj.currTimeout : obj.curTimeout;
        }

           view.push(clock);
    }
}

/* ======== getClockListByReadingTimers ========
 * Looks at all OS timers and sees if their callback function is
 * ClockP_callbackFxn, in that case that timer was created via
 * the clock module and should be visible is this view.
 * This is only checked if the device is not a CC23XX or CC26XX
 */
function getClockListByReadingTimers(){

    let addressList = TimerModule.getListOfAllActiveTimerAddresses();
    let list = [];

    for (let i = 0; i < addressList.length; i++) {
        let timerAddress = addressList[i];
        let timerObj     = Program.fetchFromAddr(timerAddress, "Timer_t", 1);
        let callbackNum  = Number(timerObj.pxCallbackFunction);
        let callBackName = String(Program.lookupFuncName(callbackNum));

        if (callBackName == "ClockP_callbackFxn") {
            // This timer has been created from within a Clock obj
            // The address of the clock is kept as the timer's ID

            let clockAddress = timerObj.pvTimerID;
            list.push(clockAddress);
        }
    }

    return list;
}

/* ======== getListOfClockAddresses ========
 * Gets a list of addresses of the clocks
 */
function getListOfClockAddresses(device){

    let list = [];

    if (device == "CC26X2") {
        let module        = Program.fetchVariable("ClockP_module");
        let moduleAddress = Program.lookupSymbolValue("ClockP_module");

        let clockAddress  = module.clockQ.elem.next;
        while (clockAddress != moduleAddress) {
            /* Since clockQ is the first member of the ClockP_Obj struct
             * the address of the QueueP_Elem object will be the same as the
             * address of the ClockP_Obj. */

            list.push(clockAddress);

            /* Traverse the queue */
            let clockQ  = Program.fetchFromAddr(clockAddress, "QueueP_Elem", 1);
            clockAddress = clockQ.next;
        }
    }
    else if (device == "CC23XX") { /* This also covers CC27XX */
        let clockPList = Program.fetchVariable("ClockP_list");

        let clockAddress = clockPList.head;

        while (clockAddress != 0) {
            /* Since elem is the first member of the ClockP_Obj struct
             * the address of the List_Elem object will be the same as the
             * address of the ClockP_Obj. */

            list.push(clockAddress);

            /* Traverse the list */
            let elem = Program.fetchFromAddr(clockAddress, "List_Elem", 1);
            clockAddress = elem.next;
        }
    }
    else {
        /* If the current device is a device which uses ClockP_freertos.c in
         * dpl then then no global list or queue keeps track of all Clock
         * objects. Thus we iterate over the FreeRTOS timers and see which of
         * them are owned by a Clock obj */
        list = getClockListByReadingTimers();
    }

    return list;
}

/* ======== getClock ========
 * Dispatched entry point for the clock module
 */
function getClock(makeDetailed){
    let view = new Array();

    try{
        /* ClockP_initialized is not present in the default dpl clock
         * freertos implementation. Therefore this check is try-catch */
        let initialized = Program.fetchVariable("ClockP_initialized");

        /* If the variable was present and with a value of false then simply return */
        if (!initialized) return view;
    }
    catch(e) {
        /* Continue regardless of error */
    }

    let device     = helperGetCurrentDevice();
    let listOfAddr = getListOfClockAddresses(device);
    handleListOfClockObj(view, listOfAddr, makeDetailed, device);

    return view;
}

/* ======== getBasicClock ========
 * Entry point for basic clock view. Sets makeDetailed to false
 */
/* eslint-disable-next-line no-unused-vars */
function getBasicClock(){
    return getClock(false);
}

/* ======== getDetailedClock ========
 * Entry point for detailed clock view. Sets makeDetailed to true
 */
/* eslint-disable-next-line no-unused-vars */
function getDetailedClock(){
    return getClock(true);
}

/* ======== getCC23XXModule ========
 * Fetches the available information about the clock module for CC23XX
 */
function getCC23XXModule(clockModule){
    let initialized         = Program.fetchVariable("ClockP_initialized");
    let ticks               = Program.fetchVariable("ClockP_ticks");
    let nextScheduledTick   = Program.fetchVariable("ClockP_nextScheduledTick");
    let inWorkFunc          = Program.fetchVariable("ClockP_inWorkFunc");
    let startDuringWorkFunc = Program.fetchVariable("ClockP_startDuringWorkFunc");

    clockModule.Started            = Boolean(initialized);
    clockModule.Ticks              = ticks;
    clockModule.NextScheduledTick  = nextScheduledTick;
    clockModule.InWorkFxn          = Boolean(inWorkFunc);
    clockModule.StartDuringWorkFxn = Boolean(startDuringWorkFunc);
}

/* ======== getCC26X2Module ========
 * Fetches the available information about the clock module for CC26X2
 */
function getCC26X2Module(clockModule, makeDetailed){
    let mod         = Program.fetchVariable("ClockP_module");
    let initialized = Program.fetchVariable("ClockP_initialized");

    clockModule.Ticks              = mod.ticks;
    clockModule.Started            = Boolean(initialized);
    clockModule.NextScheduledTick  = mod.nextScheduledTick;
    clockModule.InWorkFxn          = Boolean(mod.inWorkFunc);
    clockModule.StartDuringWorkFxn = Boolean(mod.startDuringWorkFunc);

    if (!makeDetailed) return;

    clockModule.Address = helperGetHexString(Program.lookupSymbolValue("ClockP_module"));

    clockModule.NumTickSkip  = mod.numTickSkip;
    clockModule.MaxSkippable = mod.maxSkippable;
    clockModule.Ticking      = Boolean(mod.ticking);
}

/* ======== getClockModule ========
 * Fetches the avaliable clock module information based on the device
 * that we are running on
 */
function getClockModule(makeDetailed){
    let view = new Array();
    let clockModule = makeDetailed ? new DetailedModule() : new BasicModule();
    let currentDevice = helperGetCurrentDevice();

    if (currentDevice == "CC26X2") {
        getCC26X2Module(clockModule, makeDetailed);
    }
    else if (currentDevice == "CC23XX") {
        if (makeDetailed) {
            clockModule.Address = "Detailed module only applicable to CC13XX/CC26XX";
        }
        else {
            getCC23XXModule(clockModule);
        }
    }
    else {
        /* The generic Clock dpl implementation does not expose
         * module information */
        clockModule.Ticks = "Clock module only applicable to CC13XX/CC26XX and CC23XX/CC27XX devices";
    }

    view.push(clockModule);

    return view;
}

/* ======== getBasicModule ========
 * Entry point for getting the basic module. Sets makeDetailed to false
 */
/* eslint-disable-next-line no-unused-vars */
function getBasicModule(){
    return getClockModule(false);
}

/* ======== getDetailedModule ========
 * Entry point for getting the detailed module. Sets makeDetailed to true
 */
/* eslint-disable-next-line no-unused-vars */
function getDetailedModule(){
    return getClockModule(true);
}
