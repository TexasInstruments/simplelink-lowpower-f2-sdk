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
"use strict";

Object.defineProperty(exports, "__esModule", { value: true });
class BasicClock {
}
class DetailedClock extends BasicClock {
}
class BasicModule {
}
class DetailedModule extends BasicModule {
}
class Clock {
    constructor(ctx) {
        this.ctx = ctx;
        this.viewMap = [
            { name: 'Basic', fxn: this.getBasicClock.bind(this), structName: BasicClock },
            { name: 'Detailed', fxn: this.getDetailedClock.bind(this), structName: DetailedClock },
            { name: 'Basic Module', fxn: this.getBasicModule.bind(this), structName: BasicModule },
            { name: 'Detailed Module', fxn: this.getDetailedModule.bind(this), structName: DetailedModule }
        ];
        this.Program = this.ctx.getProgram();
    }
    getModuleName() {
        return 'Clock';
    }
    get helper() {
        return this.ctx.getModule('helper.rov.js');
    }
    get TimerModule() {
        return this.ctx.getModule('timer.rov.js');
    }
    /* ======== handleListOfClockObj ========
     * Takes a list of addresses and adds these clocks
     * to the view.
     */
    async handleListOfClockObj(view, list, makeDetailed, device) {
        for (let i = 0; i < list.length; i++) {
            let clockAddress = list[i];
            let obj;
            if (device != "Unknown") {
                obj = await this.Program.fetchFromAddr(clockAddress, "ClockP_Obj");
            }
            else {
                obj = await this.Program.fetchFromAddr(clockAddress, "ClockP_FreeRTOSObj");
            }
            let clock = makeDetailed ? new DetailedClock() : new BasicClock();
            clock.Symbol = String(await this.helper.lookupDataSymbol(Number(clockAddress)));
            clock.Address = this.helper.helperGetHexString(clockAddress);
            clock.Function = String(await this.helper.lookupFuncName(Number(obj.fxn)));
            clock.Arg = this.helper.helperGetHexString(obj.arg);
            if (makeDetailed) {
                if (device != "Unknown") {
                    clock.Active = Boolean(obj.active);
                }
                clock.Timeout = obj.timeout;
                clock.Period = obj.period;
                clock.Periodic = Boolean(clock.Period > 0);
                /* Small naming difference between ClockP_FreeRTOSObj and ClockP_Obj */
                clock.CurrTimeout = ((device != "Unknown") ? obj.currTimeout : obj.curTimeout);
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
    async getClockListByReadingTimers() {
        let addressList = await this.TimerModule.getListOfAllActiveTimerAddresses();
        let list = [];
        for (let i = 0; i < addressList.length; i++) {
            let timerAddress = addressList[i];
            let timerObj = await this.Program.fetchFromAddr(timerAddress, "Timer_t", 1);
            let callbackNum = Number(timerObj.pxCallbackFunction);
            let callBackName = String(await this.helper.lookupFuncName(callbackNum));
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
    async getListOfClockAddresses(device) {
        let list = [];
        if (device == "CC26X2") {
            let module = await this.Program.fetchVariable("ClockP_module");
            let moduleAddress = await this.Program.lookupSymbolValue("ClockP_module");
            let clockAddress = module.clockQ.elem.next;
            while (clockAddress != moduleAddress) {
                /* Since clockQ is the first member of the ClockP_Obj struct
                 * the address of the QueueP_Elem object will be the same as the
                 * address of the ClockP_Obj. */
                list.push(clockAddress);
                /* Traverse the queue */
                let clockQ = await this.Program.fetchFromAddr(clockAddress, "QueueP_Elem", 1);
                clockAddress = clockQ.next;
            }
        }
        else if (device == "CC23XX") { /* This also covers CC27XX */
            let clockPList = await this.Program.fetchVariable("ClockP_list");
            let clockAddress = clockPList.head;
            while (clockAddress != 0) {
                /* Since elem is the first member of the ClockP_Obj struct
                 * the address of the List_Elem object will be the same as the
                 * address of the ClockP_Obj. */
                list.push(clockAddress);
                /* Traverse the list */
                let elem = await this.Program.fetchFromAddr(clockAddress, "List_Elem", 1);
                clockAddress = elem.next;
            }
        }
        else {
            /* If the current device is a CC32XX (or any other device which uses
             * ClockP_freertos.c) in dpl then then no global list or queue
             * keeps track of all Clock objects. Thus we iterate over the
             * FreeRTOS timers and see which of them are owned by a Clock obj */
            list = await this.getClockListByReadingTimers();
        }
        return list;
    }
    /* ======== getClock ========
     * Dispatched entry point for the clock module
     */
    async getClock(makeDetailed) {
        let view = new Array();
        try {
            /* ClockP_initialized is not present in the default dpl clock
             * freertos implementation. Therefore this check is try-catch */
            let initialized = await this.Program.fetchVariable("ClockP_initialized");
            /* If the variable was present and with a value of false then simply return */
            if (!initialized)
                return view;
        }
        catch (e) {
            /* do nothing */
         }
        let device = await this.helper.helperGetCurrentDevice();
        let listOfAddr = await this.getListOfClockAddresses(device);
        await this.handleListOfClockObj(view, listOfAddr, makeDetailed, device);
        return view;
    }
    /* ======== getBasicClock ========
     * Entry point for basic clock view. Sets makeDetailed to false
     */
    async getBasicClock() {
        return this.getClock(false);
    }
    /* ======== getDetailedClock ========
     * Entry point for detailed clock view. Sets makeDetailed to true
     */
    async getDetailedClock() {
        return this.getClock(true);
    }
    /* ======== getCC23XXModule ========
     * Fetches the available information about the clock module for CC23XX
     */
    async getCC23XXModule(clockModule) {
        let initialized = await this.Program.fetchVariable("ClockP_initialized");
        let ticks = await this.Program.fetchVariable("ClockP_ticks");
        let nextScheduledTick = await this.Program.fetchVariable("ClockP_nextScheduledTick");
        let inWorkFunc = await this.Program.fetchVariable("ClockP_inWorkFunc");
        let startDuringWorkFunc = await this.Program.fetchVariable("ClockP_startDuringWorkFunc");
        clockModule.Started = Boolean(initialized);
        clockModule.Ticks = ticks;
        clockModule.NextScheduledTick = nextScheduledTick;
        clockModule.InWorkFxn = Boolean(inWorkFunc);
        clockModule.StartDuringWorkFxn = Boolean(startDuringWorkFunc);
    }
    /* ======== getCC26X2Module ========
     * Fetches the available information about the clock module for CC26X2
     */
    async getCC26X2Module(clockModule, makeDetailed) {
        let mod = await this.Program.fetchVariable("ClockP_module");
        let initialized = await this.Program.fetchVariable("ClockP_initialized");
        clockModule.Ticks = mod.ticks;
        clockModule.Started = Boolean(initialized);
        clockModule.NextScheduledTick = mod.nextScheduledTick;
        clockModule.InWorkFxn = Boolean(mod.inWorkFunc);
        clockModule.StartDuringWorkFxn = Boolean(mod.startDuringWorkFunc);
        if (!makeDetailed)
            return;
        clockModule.Address = this.helper.helperGetHexString(await this.Program.lookupSymbolValue("ClockP_module"));
        clockModule.NumTickSkip = mod.numTickSkip;
        clockModule.MaxSkippable = mod.maxSkippable;
        clockModule.Ticking = Boolean(mod.ticking);
    }
    /* ======== getClockModule ========
     * Fetches the avaliable clock module information based on the device
     * that we are running on
     */
    async getClockModule(makeDetailed) {
        let view = new Array();
        let clockModule = makeDetailed ? new DetailedModule() : new BasicModule();
        let currentDevice = await this.helper.helperGetCurrentDevice();
        if (currentDevice == "CC26X2") {
            await this.getCC26X2Module(clockModule, makeDetailed);
        }
        else if (currentDevice == "CC23XX") {
            if (makeDetailed) {
                clockModule.Address = "Detailed module only applicable to CC13XX/CC26XX";
            }
            else {
                await this.getCC23XXModule(clockModule);
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
    async getBasicModule() {
        return this.getClockModule(false);
    }
    /* ======== getDetailedModule ========
     * Entry point for getting the detailed module. Sets makeDetailed to true
     */
    async getDetailedModule() {
        return this.getClockModule(true);
    }
}
exports.classCtor = Clock;
