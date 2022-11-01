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

/* global xdc */
let Program = xdc.module('xdc.rov.Program');

/* global helperGetListOfAddressesInListObj, helperGetHexString, helperReadStringFromAddr,
   helperCompareAddress, helperGetHexString
 */

/* eslint-disable-next-line no-unused-vars */
var moduleName = "Timer (OS)";

/* eslint-disable-next-line no-unused-vars */
var viewMap = [
    {name: "Basic", fxn: "getBasicTimer", structName: "BasicTimer"},
    {name: "Detailed", fxn: "getDetailedTimer", structName: "DetailedTimer"}
];

/* Bitmasks defined in timers.c */
let activeMask      = 0x1; // (tmrSTATUS_IS_ACTIVE)
let staticAllocMask = 0x2; // (tmrSTATUS_IS_STATICALLY_ALLOCATED)
let reloadMask      = 0x4; // (tmrSTATUS_IS_AUTORELOAD)

function BasicTimer(){
    this.Address          = null;
    this.Name             = null;
    this.TimerID          = null;
    this.CallbackFunction = null;
    this.PeriodInTicks    = null;
}

function DetailedTimer(){
    this.Address          = null;
    this.Name             = null;
    this.TimerID          = null;
    this.CallbackFunction = null;
    this.PeriodInTicks    = null;
    this.Active           = null;
    this.Reloads          = null;
    this.StaticallyAloc   = null;
}

/* ======== getListOfAllActiveTimerAddresses ========
 * Note that this function also used in the clock module
 */
function getListOfAllActiveTimerAddresses(){
    let activeList1 = Program.fetchVariable("xActiveTimerList1");
    let activeList2 = Program.fetchVariable("xActiveTimerList2");

    let list1 = helperGetListOfAddressesInListObj(activeList1);
    let list2 = helperGetListOfAddressesInListObj(activeList2);

    return list1.concat(list2);
}

/* ======== traverseTimerAddressList ========
 * Iterates through the list of timer addresses and adds
 * the timers to the view.
 */
function traverseTimerAddressList(view, list, makeDetailed){

    for (let i = 0; i < list.length; i++) {
        let timerAddress = list[i];
        let timerObj = Program.fetchFromAddr(timerAddress, "Timer_t", 1);

        let timer     = makeDetailed ? new DetailedTimer() : new BasicTimer();
        timer.Address = helperGetHexString(timerAddress);
        timer.Name    = helperReadStringFromAddr(timerObj.pcTimerName, 12);
        timer.TimerID = timerObj.pvTimerID;
        timer.PeriodInTicks = timerObj.xTimerPeriodInTicks;

        let callbackNum        = Number(timerObj.pxCallbackFunction);
        let callbackAddress    = helperGetHexString(callbackNum);
        let callBackName       = String(Program.lookupFuncName(callbackNum));
        timer.CallbackFunction = callbackAddress + " (" + callBackName + ")";

        if (makeDetailed) {
            let statusBits       = timerObj.ucStatus;
            timer.Active         = Boolean(statusBits & activeMask);
            timer.StaticallyAloc = Boolean(statusBits & staticAllocMask);
            timer.Reloads        = Boolean(statusBits & reloadMask);
        }

        view.push(timer);
    }
}

/* ======== getTimer ========
 * Dispatched entry point for getting the timer module
 */
function getTimer(makeDetailed){
    let view = new Array();

    let addresses = getListOfAllActiveTimerAddresses();
    traverseTimerAddressList(view, addresses, makeDetailed);

    view.sort(helperCompareAddress);

    return view;
}

/* ======== getBasicTimer ========
 * Entry point for basic timer view. Sets makeDetailed to false
 */
/* eslint-disable-next-line no-unused-vars */
function getBasicTimer(){
    return getTimer(false);
}

/* ======== getDetailedTimer ========
 * Entry point for detailed timer view. Sets makeDetailed to true
 */
/* eslint-disable-next-line no-unused-vars */
function getDetailedTimer(){
    return getTimer(true);
}
