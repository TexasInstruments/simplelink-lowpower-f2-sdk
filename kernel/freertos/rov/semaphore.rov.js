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
let QueueModule = xdc.loadCapsule("queue.rov.js");

/* global helperCompareAddress */

/* eslint-disable-next-line no-unused-vars */
var moduleName = "Semaphore";

/* eslint-disable-next-line no-unused-vars */
var viewMap = [
    {name: "Basic", fxn: "getBasicSemaphore", structName: "BasicSemaphore"},
    {name: "Detailed", fxn: "getDetailedSemaphore", structName: "DetailedSemaphore"}
];

function BasicSemaphore(){
    this.Address               = null;
    this.Name                  = null;
    this.Type                  = null;
    this.MaxCnt                = null;
    this.Available             = null;
    this.BlockedCount          = null;
    this.BlockedTasks          = null;
}

function DetailedSemaphore(){
    this.Address               = null;
    this.Name                  = null;
    this.Type                  = null;
    this.MaxCnt                = null;
    this.Available             = null;
    this.BlockedCount          = null;
    this.BlockedTasks          = null;
    this.StaticallyAlloc       = null;
    this.RemovedWhileLocked    = null;
    this.AddedWhileLocked      = null;
}

/* ======== fillSemaphoreObjectFromMap ========
 * Takes the content from the map that is interesting for when the Queue
 * struct implements a Semaphore
 */
function fillSemaphoreObjectFromMap(map, view, makeDetailed){
    if (map["Type"] != "Semaphore (Counting)" && map["Type"] != "Semaphore (Binary)") {
        return;
    }

    let sem = makeDetailed ? new DetailedSemaphore() : new BasicSemaphore();
    sem.Name             = map["Name"];
    sem.Address          = map["Address"];
    sem.MaxCnt           = map["uxLength"];
    sem.Type             = map["Type"];
    sem.Available        = map["uxMessagesWaiting"];
    sem.BlockedCount     = map["TasksWaitingToReceive"];
    sem.BlockedTasks     = map["blockedTasks"];

    if (makeDetailed) {
        sem.StaticallyAlloc     = map["ucStaticallyAllocated"];
        sem.RemovedWhileLocked  = map["CRxLock"];
        sem.AddedWhileLocked    = map["CTxLock"];
    }

    view.push(sem);
}

/* ======== getSemaphore ========
 * Main function for getting the semaphore basic and detailed view
 */
function getSemaphore(makeDetailed){
    let view = new Array();

    let maps = QueueModule.createMapForEachQueueObject();

    /* maps will be null here when
     * configQUEUE_REGISTRY_SIZE is <= 0 */
    if (maps == null) {
        let message = makeDetailed ? new DetailedSemaphore() : new BasicSemaphore();
        message.Address = "Set configQUEUE_REGISTRY_SIZE to the maximum number of queues + semaphores + mutexes that ROV should be able to read";
        view.push(message);
        return view;
    }

    for (let i = 0; i < maps.length; i++) {
        fillSemaphoreObjectFromMap(maps[i], view, makeDetailed);
    }

    view.sort(helperCompareAddress);

    return view;
}

/* ======== getBasicSemaphore ========
 * Entry point for basic semaphore view. Sets makeDetailed to false
 */
/* eslint-disable-next-line no-unused-vars */
function getBasicSemaphore(){
    return getSemaphore(false);
}

/* ======== getDetailedSemaphore ========
 * Entry point for detailed semaphore view. Sets makeDetailed to true
 */
/* eslint-disable-next-line no-unused-vars */
function getDetailedSemaphore(){
    return getSemaphore(true);
}
