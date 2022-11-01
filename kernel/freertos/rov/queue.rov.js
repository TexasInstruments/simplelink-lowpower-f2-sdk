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

/* global helperGetNameOfTaskByAddress, helperGetHexString, helperGetListOfAddressesInListObj,
   helperReadStringFromAddr, helperCompareAddress
 */

/* eslint-disable-next-line no-unused-vars */
var moduleName = "Queue";

/* eslint-disable-next-line no-unused-vars */
var viewMap = [
    {name: "Basic", fxn: "getBasicQueue", structName: "BasicQueue"},
    {name: "Detailed", fxn: "getDetailedQueue", structName: "DetailedQueue"}
];

function BasicQueue(){
    this.Address               = null;
    this.Name                  = null;
    this.ItemSize              = null;
    this.Length                = null;
    this.MsgWaiting            = null;
    this.TasksPendingSend      = null;
    this.TasksPendingReceive   = null;
    this.BlockedTasks          = null;
}

function DetailedQueue(){
    this.Address               = null;
    this.Name                  = null;
    this.ItemSize              = null;
    this.Length                = null;
    this.MsgWaiting            = null;
    this.TasksPendingSend      = null;
    this.TasksPendingReceive   = null;
    this.BlockedTasks          = null;
    this.StaticallyAlloc       = null;
    this.StorageStart          = null;
    this.StorageEnd            = null;
    this.NextFree              = null;
    this.RemovedWhileLocked    = null;
    this.AddedWhileLocked      = null;
}

/* ======== getQueueType ========
 * Translates the queueType number to a string representation
 */
function getQueueType(ucQueueType){
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
function getTaskAddressesAndNames(taskAddresses){
    let namesAndAddresses = "";

    for (let i = 0; i < taskAddresses.length; i++) {
        let addr = taskAddresses[i];
        let name = helperGetNameOfTaskByAddress(addr);
        namesAndAddresses += name + ": " + String(addr)  + ", ";
    }

    return namesAndAddresses;
}

/* ======== parseUnionInQueueStruct ========
 * This is a helper function used when parsing the Queue_t struct.
 */
function parseUnionInQueueStruct(pointerToUnion, queueType){
    let unionMap = {};

    /* Queue has queueType 0 anything else {1, 2, 3, 4} is a Mutex or Semaphore */
    if (queueType != 0) {
        /* This queue is used as a Mutex or Semaphore
         * Now, parse the union */
        let taskAddress = Program.fetchFromAddr(pointerToUnion, "TaskHandle_t", 1);
        if (taskAddress == 0) {
            /* When a mutex is not held by any task the taskAddress will be zero */
            unionMap["xMutexHolder"] = null;
        }
        else {
            let name = helperGetNameOfTaskByAddress(taskAddress);
            let addr = helperGetHexString(taskAddress);
            unionMap["xMutexHolder"] = name + ": " + addr;
        }
        unionMap["uxRecursiveCallCount"] = Program.fetchFromAddr(pointerToUnion + 4, "UBaseType_t", 1);
    }
    else {
        /* This queue is used as a Queue
         * Now, parse the union */
        unionMap["pcTail"]     = Program.fetchFromAddr(pointerToUnion, "uint32_t", 1);
        unionMap["pcReadFrom"] = Program.fetchFromAddr(pointerToUnion + 4, "uint32_t", 1);
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
function parseQueueObjManually(name, xHandle){

    let currentPtr = xHandle;

    /* Mirror the Queue_t (QueueDefinition) struct */

    /* Assumes 32bit machine */
    let pcHead                 = Program.fetchFromAddr(currentPtr, "uint32_t", 1);
    currentPtr                 += 4; //increment in bytes

    let pcWriteTo              = Program.fetchFromAddr(currentPtr, "uint32_t", 1);
    currentPtr                 += 4;

    let SemaphoreDataSize      = Program.lookupType("SemaphoreData_t").size;
    let QueuePointersSize      = Program.lookupType("QueuePointers_t").size;
    let BaseSize               = Program.lookupType("UBaseType_t").size;
    let ListSize               = Program.lookupType("List_t").size;

    /* Ignore union until we know queue type */
    let pointerAtUnion         = currentPtr;
    currentPtr                 += Math.max(SemaphoreDataSize, QueuePointersSize);

    let xTasksWaitingToSend    = Program.fetchFromAddr(currentPtr, "List_t", 1);
    currentPtr                 += ListSize;

    let xTasksWaitingToReceive = Program.fetchFromAddr(currentPtr, "List_t", 1);
    currentPtr                 += ListSize;

    let uxMessagesWaiting      = Program.fetchFromAddr(currentPtr, "UBaseType_t", 1);
    currentPtr                 += BaseSize;

    let uxLength               = Program.fetchFromAddr(currentPtr, "UBaseType_t", 1);
    currentPtr                 += BaseSize;

    let uxItemSize             = Program.fetchFromAddr(currentPtr, "UBaseType_t", 1);
    currentPtr                 += BaseSize;

    let CRxLock                = Program.fetchFromAddr(currentPtr, "int8_t", 1);
    currentPtr                 += 1;

    let CTxLock                = Program.fetchFromAddr(currentPtr, "int8_t", 1);
    currentPtr                 += 1;

    /* configSUPPORT_STATIC_ALLOCATION needs to be 1, which is standard in TI software */

    let ucStaticallyAllocated  = Program.fetchFromAddr(currentPtr, "uint8_t", 1);
    currentPtr++;

    /* configUSE_QUEUE_SETS     needs to be 0, which is standard in TI software */
    /* configUSE_TRACE_FACILITY needs to be 1, which is standard in TI software*/
    /* eslint-disable-next-line no-unused-vars */
    let uxQueueNumber          = Program.fetchFromAddr(currentPtr, "UBaseType_t", 1);
    currentPtr                 += BaseSize;

    /* Both TICLANG and GCC puts 1 byte of padding here */
    currentPtr += 1;

    let ucQueueType  = Program.fetchFromAddr(currentPtr, "uint8_t", 1);

    let unionMap     = parseUnionInQueueStruct(pointerAtUnion, ucQueueType);

    let map = {};

    map["Address"]                  = xHandle;
    map["Name"]                     = name;
    map["uxLength"]                 = uxLength;
    map["Type"]                     = getQueueType(ucQueueType);
    map["uxMessagesWaiting"]        = uxMessagesWaiting;
    map["uxItemSize"]               = uxItemSize;
    map["ucStaticallyAllocated"]    = Boolean(ucStaticallyAllocated);
    map["CRxLock"]                  = CRxLock >= 0 ? CRxLock : "Not Locked";
    map["CTxLock"]                  = CTxLock >= 0 ? CTxLock : "Not Locked";
    map["TasksWaitingToSend"]       = xTasksWaitingToSend.uxNumberOfItems;
    map["TasksWaitingToReceive"]    = xTasksWaitingToReceive.uxNumberOfItems;
    let addressSend                 = helperGetListOfAddressesInListObj(xTasksWaitingToSend);
    let userViewSend                = getTaskAddressesAndNames(addressSend);
    let addressRec                  = helperGetListOfAddressesInListObj(xTasksWaitingToReceive);
    let userViewRec                 = getTaskAddressesAndNames(addressRec);
    let blockedTasks                = userViewSend + userViewRec;
    blockedTasks                    = blockedTasks.slice(0, -2); //remove last two chars ", "
    map["blockedTasks"]             = blockedTasks.length > 0 ? blockedTasks : "-";
    map["blockedAddresses"]         = addressSend.concat(addressRec);
    map["xMutexHolder"]             = unionMap["xMutexHolder"];
    map["uxRecursiveCallCount"]     = unionMap["uxRecursiveCallCount"];
    map["pcTail"]                   = helperGetHexString(unionMap["pcTail"]);
    map["pcReadFrom"]               = helperGetHexString(unionMap["pcReadFrom"]);
    map["pcHead"]                   = helperGetHexString(pcHead);
    map["pcWriteTo"]                = helperGetHexString(pcWriteTo);

    return map;
}

/* ======== fillQueueObjectFromMap ========
 * Takes the content from the map that is interesting for when the Queue_t
 * struct actually implements a Queue
 */
function fillQueueObjectFromMap(map, view, makeDetailed){
    if (map["Type"] != "Queue") {
        return;
    }

    let queue = makeDetailed ? new DetailedQueue() : new BasicQueue();
    queue.Name                  = map["Name"];
    queue.Address               = map["Address"];
    queue.Length                = map["uxLength"];
    queue.MsgWaiting            = map["uxMessagesWaiting"];
    queue.ItemSize              = map["uxItemSize"];
    queue.TasksPendingSend      = map["TasksWaitingToSend"];
    queue.TasksPendingReceive   = map["TasksWaitingToReceive"];
    queue.BlockedTasks          = map["blockedTasks"];
    if (makeDetailed) {
        queue.StorageStart       = map["pcHead"];
        queue.StorageEnd         = map["pcTail"];
        queue.StaticallyAlloc    = map["ucStaticallyAllocated"];
        queue.NextFree           = map["pcWriteTo"];
        queue.RemovedWhileLocked = map["CRxLock"];
        queue.AddedWhileLocked   = map["CTxLock"];
    }

    view.push(queue);
}

/* ======== createMapForEachQueueObject ========
 * Reads the structure xQueueRegistry which holds names and pointers
 * to all queue objects added to the registry and return a map of info
 * about each Queue object
 */
function createMapForEachQueueObject(){
    let xQueueRegistry;
    try {
        xQueueRegistry = Program.fetchVariable("xQueueRegistry");
    }
    catch(e) {
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
            let name = helperReadStringFromAddr(QRItem.pcQueueName, 16);
            let map  = parseQueueObjManually(name, QRItem.xHandle);
            maps.push(map);
        }
    }

    return maps;
}

/* ======== getQueues ========
 * Dispatched entry point for the queue module
 */
function getQueues(makeDetailed){
    let view = new Array();

    let queueMaps = createMapForEachQueueObject();

    /* queueMaps will be null here when configQUEUE_REGISTRY_SIZE
     * is <= 0 thus we add a message to guide the user */
    if (queueMaps == null) {
        let message = makeDetailed ? new DetailedQueue() : new BasicQueue();
        message.Address = "Set configQUEUE_REGISTRY_SIZE to the maximum number of queues + semaphores + mutexes that ROV should be able to read";
        view.push(message);
        return view;
    }

    for (let i = 0; i < queueMaps.length; i++) {
        fillQueueObjectFromMap(queueMaps[i], view, makeDetailed);
    }

    view.sort(helperCompareAddress);

    return view;
}

/* ======== getBasicQueue ========
 * Entry point for basic queue view. Sets makeDetailed to false
 */
/* eslint-disable-next-line no-unused-vars */
function getBasicQueue(){
    return getQueues(false);
}

/* ======== getDetailedQueue ========
 * Entry point for detailed task view. Sets makeDetailed to true
 */
/* eslint-disable-next-line no-unused-vars */
function getDetailedQueue(){
    return getQueues(true);
}
