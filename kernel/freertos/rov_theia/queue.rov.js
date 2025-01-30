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
class BasicQueue {
}
class DetailedQueue extends BasicQueue {
}
class Queue {
    constructor(ctx) {
        this.ctx = ctx;
        this.viewMap = [
            { name: 'Basic', fxn: this.getBasicQueue.bind(this), structName: BasicQueue },
            { name: 'Detailed', fxn: this.getDetailedQueue.bind(this), structName: DetailedQueue }
        ];
        this.Program = this.ctx.getProgram();
    }
    getModuleName() {
        return 'Queue';
    }
    get helper() {
        return this.ctx.getModule('helper.rov.js');
    }
    /* ======== getQueueType ========
     * Translates the queueType number to a string representation
     */
    getQueueType(ucQueueType) {
        switch (ucQueueType) {
            default:
            case 0:
                return "Queue";
            case 1:
                return "Mutex";
            case 2:
                return "Semaphore (Counting)";
            case 3:
                return "Semaphore (Binary)";
            case 4:
                return "Mutex (Recursive)";
        }
    }
    /* ======== getTaskAddressesAndNames ========
     * Fetches the task addresses and the
     * names of tasks from a list of task addresses
     */
    async getTaskAddressesAndNames(taskAddresses) {
        let namesAndAddresses = "";
        for (let i = 0; i < taskAddresses.length; i++) {
            let addr = taskAddresses[i];
            let name = await this.helper.helperGetNameOfTaskByAddress(addr);
            namesAndAddresses += name + ": " + String(addr) + ", ";
        }
        return namesAndAddresses;
    }
    /* ======== parseUnionInQueueStruct ========
     * This is a helper function used when parsing the Queue_t struct.
     */
    async parseUnionInQueueStruct(pointerToUnion, queueType) {
        let unionMap = {};
        /* Queue has queueType 0 anything else {1, 2, 3, 4} is a Mutex or Semaphore */
        if (queueType != 0) {
            /* This queue is used as a Mutex or Semaphore
             * Now, parse the union */
            let taskAddress = await this.Program.fetchFromAddr(pointerToUnion, "TaskHandle_t", 1);
            if (taskAddress == 0) {
                /* When a mutex is not held by any task the taskAddress will be zero */
                unionMap["xMutexHolder"] = null;
            }
            else {
                let name = await this.helper.helperGetNameOfTaskByAddress(taskAddress);
                let addr = this.helper.helperGetHexString(taskAddress);
                unionMap["xMutexHolder"] = name + ": " + addr;
            }
            unionMap["uxRecursiveCallCount"] = await this.Program.fetchFromAddr(pointerToUnion + 4, "UBaseType_t", 1);
        }
        else {
            /* This queue is used as a Queue
             * Now, parse the union */
            unionMap["pcTail"] = await this.Program.fetchFromAddr(pointerToUnion, "uint32_t", 1);
            unionMap["pcReadFrom"] = await this.Program.fetchFromAddr(pointerToUnion + 4, "uint32_t", 1);
        }
        return unionMap;
    }
    /* ======== parseQueueObjManually ========
     * The xdc tool cannot parse the Queue_t struct since it contains a union
     * and the tool seems unable to parse that. Therefore the command
     * Program.fetchFromAddr(xQueueRegistry[i].xHandle, "Queue_t");
     * does not work. A solution is to manually parse the Queue_t struct
     *
     * The manual parsing returns a map of key-value pairs instead of directly
     * creating a queue and adding it to the view. This abstraction allows us
     * to choose which aspects of the Queue_t object to display based on what
     * structure the Queue_t represents, without having different parsing
     * functions
     */
    async parseQueueObjManually(name, xHandle) {
        let currentPtr = xHandle;
        /* Mirror the Queue_t (QueueDefinition) struct */
        /* Assumes 32bit machine */
        let pcHead = await this.Program.fetchFromAddr(currentPtr, "uint32_t", 1);
        currentPtr += 4; //increment in bytes
        let pcWriteTo = await this.Program.fetchFromAddr(currentPtr, "uint32_t", 1);
        currentPtr += 4;
        let SemaphoreDataSize = (await this.Program.lookupType("SemaphoreData_t")).size;
        let QueuePointersSize = (await this.Program.lookupType("QueuePointers_t")).size;
        let BaseSize = (await this.Program.lookupType("UBaseType_t")).size;
        let ListSize = (await this.Program.lookupType("List_t")).size;
        /* Ignore union until we know queue type */
        let pointerAtUnion = currentPtr;
        currentPtr += Math.max(SemaphoreDataSize, QueuePointersSize);
        let xTasksWaitingToSend = await this.Program.fetchFromAddr(currentPtr, "List_t", 1);
        currentPtr += ListSize;
        let xTasksWaitingToReceive = await this.Program.fetchFromAddr(currentPtr, "List_t", 1);
        currentPtr += ListSize;
        let uxMessagesWaiting = await this.Program.fetchFromAddr(currentPtr, "UBaseType_t", 1);
        currentPtr += BaseSize;
        let uxLength = await this.Program.fetchFromAddr(currentPtr, "UBaseType_t", 1);
        currentPtr += BaseSize;
        let uxItemSize = await this.Program.fetchFromAddr(currentPtr, "UBaseType_t", 1);
        currentPtr += BaseSize;
        let CRxLock = await this.Program.fetchFromAddr(currentPtr, "int8_t", 1);
        currentPtr += 1;
        let CTxLock = await this.Program.fetchFromAddr(currentPtr, "int8_t", 1);
        currentPtr += 1;
        /* configSUPPORT_STATIC_ALLOCATION needs to be 1, which is standard in TI software */
        let ucStaticallyAllocated = await this.Program.fetchFromAddr(currentPtr, "uint8_t", 1);
        currentPtr++;
        /* configUSE_QUEUE_SETS     needs to be 0, which is standard in TI software */
        /* configUSE_TRACE_FACILITY needs to be 1, which is standard in TI software*/
        /* eslint-disable-next-line no-unused-vars */
        let uxQueueNumber = await this.Program.fetchFromAddr(currentPtr, "UBaseType_t", 1);
        currentPtr += BaseSize;
        /* Both TICLANG and GCC puts 1 byte of padding here */
        currentPtr += 1;
        let ucQueueType = await this.Program.fetchFromAddr(currentPtr, "uint8_t", 1);
        let unionMap = await this.parseUnionInQueueStruct(pointerAtUnion, ucQueueType);
        let map = {};
        map["Address"] = xHandle;
        map["Name"] = name;
        map["uxLength"] = uxLength;
        map["Type"] = this.getQueueType(ucQueueType);
        map["uxMessagesWaiting"] = uxMessagesWaiting;
        map["uxItemSize"] = uxItemSize;
        map["ucStaticallyAllocated"] = Boolean(ucStaticallyAllocated);
        map["CRxLock"] = CRxLock >= 0 ? CRxLock : "Not Locked";
        map["CTxLock"] = CTxLock >= 0 ? CTxLock : "Not Locked";
        map["TasksWaitingToSend"] = xTasksWaitingToSend.uxNumberOfItems;
        map["TasksWaitingToReceive"] = xTasksWaitingToReceive.uxNumberOfItems;
        let addressSend = await this.helper.helperGetListOfAddressesInListObj(xTasksWaitingToSend);
        let userViewSend = await this.getTaskAddressesAndNames(addressSend);
        let addressRec = await this.helper.helperGetListOfAddressesInListObj(xTasksWaitingToReceive);
        let userViewRec = await this.getTaskAddressesAndNames(addressRec);
        let blockedTasks = userViewSend + userViewRec;
        blockedTasks = blockedTasks.slice(0, -2); //remove last two chars ", "
        map["blockedTasks"] = blockedTasks.length > 0 ? blockedTasks : "-";
        map["blockedAddresses"] = addressSend.concat(addressRec);
        map["xMutexHolder"] = unionMap["xMutexHolder"];
        map["uxRecursiveCallCount"] = unionMap["uxRecursiveCallCount"];
        map["pcTail"] = this.helper.helperGetHexString(unionMap["pcTail"]);
        map["pcReadFrom"] = this.helper.helperGetHexString(unionMap["pcReadFrom"]);
        map["pcHead"] = this.helper.helperGetHexString(pcHead);
        map["pcWriteTo"] = this.helper.helperGetHexString(pcWriteTo);
        return map;
    }
    /* ======== fillQueueObjectFromMap ========
     * Takes the content from the map that is interesting for when the Queue_t
     * struct actually implements a Queue
     */
    fillQueueObjectFromMap(map, view, makeDetailed) {
        if (map["Type"] != "Queue") {
            return;
        }
        let queue = makeDetailed ? new DetailedQueue() : new BasicQueue();
        queue.Name = map["Name"];
        queue.Address = this.helper.helperGetHexString(map["Address"]);
        queue.Length = map["uxLength"];
        queue.MsgWaiting = map["uxMessagesWaiting"];
        queue.ItemSize = map["uxItemSize"];
        queue.TasksPendingSend = map["TasksWaitingToSend"];
        queue.TasksPendingReceive = map["TasksWaitingToReceive"];
        queue.BlockedTasks = map["blockedTasks"];
        if (makeDetailed) {
            queue.StorageStart = map["pcHead"];
            queue.StorageEnd = map["pcTail"];
            queue.StaticallyAlloc = map["ucStaticallyAllocated"];
            queue.NextFree = map["pcWriteTo"];
            queue.RemovedWhileLocked = map["CRxLock"];
            queue.AddedWhileLocked = map["CTxLock"];
        }
        view.push(queue);
    }
    /* ======== createMapForEachQueueObject ========
     * Reads the structure xQueueRegistry which holds names and pointers
     * to all queue objects added to the registry and return a map of info
     * about each Queue object
     */
    async createMapForEachQueueObject() {
        let xQueueRegistry;
        try {
            xQueueRegistry = await this.Program.fetchVariable("xQueueRegistry");
        }
        catch (e) {
            /* xQueueRegistry couldn't be found */
            /* Most likely configQUEUE_REGISTRY_SIZE is set to 0 */
            return null;
        }
        let maps = [];
        for (let i = 0; i < xQueueRegistry.length; i++) {
            /* The queue registry holds queue registry items*/
            let QRItem = xQueueRegistry[i];
            if (QRItem.pcQueueName != 0 && QRItem.xHandle != 0) {
                /* Set limit on 16 characters for name. If the name is longer,
                 * we get the first 16 characters. This choice of max length is
                 * arbitrary and may be changed */
                let name = await this.helper.helperReadStringFromAddr(QRItem.pcQueueName, 16);
                let map = await this.parseQueueObjManually(name, QRItem.xHandle);
                maps.push(map);
            }
        }
        return maps;
    }
    /* ======== getQueues ========
     * Dispatched entry point for the queue module
     */
    async getQueues(makeDetailed) {
        let view = new Array();
        let queueMaps = await this.createMapForEachQueueObject();
        /* queueMaps will be null here when configQUEUE_REGISTRY_SIZE
         * is <= 0 thus we add a message to guide the user */
        if (queueMaps == null) {
            let message = makeDetailed ? new DetailedQueue() : new BasicQueue();
            message.Address = "Set configQUEUE_REGISTRY_SIZE to the maximum number of queues + semaphores + mutexes that ROV should be able to read";
            view.push(message);
            return view;
        }
        for (let i = 0; i < queueMaps.length; i++) {
            this.fillQueueObjectFromMap(queueMaps[i], view, makeDetailed);
        }
        view.sort(this.helper.helperCompareAddress);
        return view;
    }
    /* ======== getBasicQueue ========
     * Entry point for basic queue view. Sets makeDetailed to false
     */
    async getBasicQueue() {
        return this.getQueues(false);
    }
    /* ======== getDetailedQueue ========
     * Entry point for detailed task view. Sets makeDetailed to true
     */
    async getDetailedQueue() {
        return this.getQueues(true);
    }
}
exports.classCtor = Queue;
