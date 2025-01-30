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
class BasicSemaphore {
}
class DetailedSemaphore extends BasicSemaphore {
}
class Semaphore {
    constructor(ctx) {
        this.ctx = ctx;
        this.viewMap = [
            { name: 'Basic', fxn: this.getBasicSemaphore.bind(this), structName: BasicSemaphore },
            { name: 'Detailed', fxn: this.getDetailedSemaphore.bind(this), structName: DetailedSemaphore }
        ];
    }
    getModuleName() {
        return 'Semaphore';
    }
    get helper() {
        return this.ctx.getModule('helper.rov.js');
    }
    get QueueModule() {
        return this.ctx.getModule('queue.rov.js');
    }
    /* ======== fillSemaphoreObjectFromMap ========
     * Takes the content from the map that is interesting for when the Queue
     * struct implements a Semaphore
     */
    fillSemaphoreObjectFromMap(map, view, makeDetailed) {
        if (map["Type"] != "Semaphore (Counting)" && map["Type"] != "Semaphore (Binary)") {
            return;
        }
        let sem = makeDetailed ? new DetailedSemaphore() : new BasicSemaphore();
        sem.Name = map["Name"];
        sem.Address = this.helper.helperGetHexString(map["Address"]);
        sem.MaxCnt = map["uxLength"];
        sem.Type = map["Type"];
        sem.Available = map["uxMessagesWaiting"];
        sem.BlockedCount = map["TasksWaitingToReceive"];
        sem.BlockedTasks = map["blockedTasks"];
        if (makeDetailed) {
            sem.StaticallyAlloc = map["ucStaticallyAllocated"];
            sem.RemovedWhileLocked = map["CRxLock"];
            sem.AddedWhileLocked = map["CTxLock"];
        }
        view.push(sem);
    }
    /* ======== getSemaphore ========
     * Main function for getting the semaphore basic and detailed view
     */
    async getSemaphore(makeDetailed) {
        let view = new Array();
        let maps = await this.QueueModule.createMapForEachQueueObject();
        /* maps will be null here when
         * configQUEUE_REGISTRY_SIZE is <= 0 */
        if (maps == null) {
            let message = makeDetailed ? new DetailedSemaphore() : new BasicSemaphore();
            message.Address = "Set configQUEUE_REGISTRY_SIZE to the maximum number of queues + semaphores + mutexes that ROV should be able to read";
            view.push(message);
            return view;
        }
        for (let i = 0; i < maps.length; i++) {
            this.fillSemaphoreObjectFromMap(maps[i], view, makeDetailed);
        }
        view.sort(this.helper.helperCompareAddress);
        return view;
    }
    /* ======== getBasicSemaphore ========
     * Entry point for basic semaphore view. Sets makeDetailed to false
     */
    async getBasicSemaphore() {
        return this.getSemaphore(false);
    }
    /* ======== getDetailedSemaphore ========
     * Entry point for detailed semaphore view. Sets makeDetailed to true
     */
    async getDetailedSemaphore() {
        return this.getSemaphore(true);
    }
}
exports.classCtor = Semaphore;
