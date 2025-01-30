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
let queueMapList = null;
class BasicTask {
}
class DetailedTask extends BasicTask {
}
class TaskScheduler {
}
class Task {
    constructor(ctx) {
        this.ctx = ctx;
        this.viewMap = [
            { name: 'Basic', fxn: this.getTaskBasic.bind(this), structName: BasicTask },
            { name: 'Detailed', fxn: this.getTaskDetailed.bind(this), structName: DetailedTask },
            { name: 'Task Scheduler', fxn: this.getTaskScheduler.bind(this), structName: TaskScheduler }
        ];
        this.Program = this.ctx.getProgram();
    }
    getModuleName() {
        return 'Task';
    }
    get helper() {
        return this.ctx.getModule('helper.rov.js');
    }
    get QueueModule() {
        return this.ctx.getModule('queue.rov.js');
    }
    /* ======== getTasks ========
     * Dispatched entry point for the task module.
     */
    async getTasks(makeDetailed) {
        let view = new Array();
        /* Set queueMapList to null to signal that the queue info should be
         * re-calculated */
        queueMapList = null;
        /* Fetch current task and pass it as an argument so that the fetch only
         * has to be done once. */
        let currentTask = await this.Program.fetchVariable("pxCurrentTCB");
        /* Ready Lists */
        let readyList = await this.Program.fetchVariable("pxReadyTasksLists");
        for (let i = 0; i < readyList.length; i++) {
            await this.addTasksFromListObj(view, readyList[i], "Ready", currentTask, makeDetailed);
        }
        /* Suspended List */
        let suspendedList = await this.Program.fetchVariable("xSuspendedTaskList");
        await this.addTasksFromListObj(view, suspendedList, "Blocked", currentTask, makeDetailed);
        /* Delay1 List */
        let delay1List = await this.Program.fetchVariable("xDelayedTaskList1");
        await this.addTasksFromListObj(view, delay1List, "Delayed", currentTask, makeDetailed);
        /* Delay2 List */
        let delay2List = await this.Program.fetchVariable("xDelayedTaskList2");
        await this.addTasksFromListObj(view, delay2List, "Delayed", currentTask, makeDetailed);
        /* Terminated List */
        let terminatedList = await this.Program.fetchVariable("xTasksWaitingTermination");
        await this.addTasksFromListObj(view, terminatedList, "Terminated", currentTask, makeDetailed);
        /* Pending ready list */
        let pendingReadyList = await this.Program.fetchVariable("xPendingReadyList");
        await this.addTasksFromListObj(view, pendingReadyList, "pendingReady", currentTask, makeDetailed);
        /* Sort by Address so the tasks don't bounce around in ROV */
        view.sort(this.helper.helperCompareAddress);
        return view;
    }
    /* ======== checkIfTaskIsBlockingOnQueue ======== */
    /* Iterates over all tasks blocked by a queue to check if
     * taskAddress is one of them
     */
    checkIfTaskIsBlockingOnQueue(taskAddress, queueMap) {
        let blocked = queueMap["blockedAddresses"];
        for (let i = 0; i < blocked.length; i++) {
            if (String(blocked[i]) == String(taskAddress)) {
                return true;
            }
        }
        return false;
    }
    /* ======== initQueueMapList ======== */
    async initQueueMapList() {
        queueMapList = await this.QueueModule.createMapForEachQueueObject();
    }
    /* ======== setBlockedOn ========
     * For all queue objects (which can represent Queues, Mutexes or Semaphores)
     * checks if the task represented by taskInfo is blocked by it.
     */
    setBlockedOn(taskInfo) {
        /* queueMapList will be null here when configQUEUE_REGISTRY_SIZE
         * is set to zero */
        if (queueMapList == null) {
            taskInfo.BlockedOn = "Unknown";
            return;
        }
        for (let i = 0; i < queueMapList.length; i++) {
            if (this.checkIfTaskIsBlockingOnQueue(taskInfo.Address, queueMapList[i])) {
                taskInfo.BlockedOn = queueMapList[i]["Type"] + ": " + queueMapList[i]["Name"];
                return;
            }
        }
        taskInfo.BlockedOn = "Unknown";
    }
    /* ======== getStaticAllocationInfo ======== */
    getStaticAllocationInfo(taskInfo, taskObj) {
        try {
            let staticallyAlloc = taskObj.ucStaticallyAllocated;
            if (staticallyAlloc == 2) {
                taskInfo.StaticAlloc = "Stack & TCB";
            }
            else if (staticallyAlloc == 1) {
                taskInfo.StaticAlloc = "Stack, not TCB";
            }
            else if (staticallyAlloc == 0) {
                taskInfo.StaticAlloc = "No";
            }
        }
        catch (e) {
            /* ucStaticallyAllocated was not present, likely because of that
             * tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE is not set */
            taskInfo.StaticAlloc = "No";
        }
    }
    /* ======== addTasksFromListObj ========
     * Loops through a list of task addresses
     * and adds each task in the list to the view
     */
    async addTasksFromListObj(view, listObj, state, currentTask, makeDetailed) {
        let taskAddresses = await this.helper.helperGetListOfAddressesInListObj(listObj);
        for (let i = 0; i < taskAddresses.length; i++) {
            let taskAddress = taskAddresses[i];
            let task = await this.Program.fetchFromAddr(taskAddress, "TCB_t");
            let taskInfo = makeDetailed ? new DetailedTask() : new BasicTask();
            taskInfo.Address = this.helper.helperGetHexString(taskAddress);
            taskInfo.TaskName = this.helper.helperGetNameOfTaskByTCBObj(task);

            /*
             * pthreads don't support names, however FreeRTOS requires all tasks
             * to have a valid name.  The pthread implementation uses a fixed,
             * single character name 'x' for all pthreads.  Whilst we don't have
             * a name, we can extract the pthread entry point, which is stored
             * inside pthread_Obj, which is itself found in the FreeRTOS task
             * 'tag'.  Here, we create a more descriptive name.
             */
            if (taskInfo.TaskName == "x") {
                /* at least report that it's a "ti-pthread... */
                taskInfo.TaskName = "[ti-pthread]";
                if (task.pxTaskTag) {
                    /* ... and if possible, add the function entry point */
                    let pthreadObj = await this.Program.fetchFromAddr(task.pxTaskTag, "pthread_Obj");
                    taskInfo.TaskName += ":" + await this.helper.lookupFuncName(Number(pthreadObj.fxn));
                }
            }

            taskInfo.BasePriority = task.uxBasePriority;
            if (Number(taskAddress) == Number(currentTask)) {
                taskInfo.State = "Running";
            }
            else {
                taskInfo.State = state;
            }
            if (state == "Blocked") {
                /* We only need to create the queueMapList once each time
                 * the task module is rendered (and some task is blocked)
                 * thus initializing it once here */
                if (queueMapList == null) {
                    await this.initQueueMapList();
                }
                this.setBlockedOn(taskInfo);
            }
            let limit = task.pxStack;
            taskInfo.StackSize = task.pxEndOfStack - limit;
            taskInfo.StackUsage = task.pxEndOfStack - task.pxTopOfStack;
            if (makeDetailed) {
                taskInfo.Priority = task.uxPriority;
                taskInfo.CurrentTaskSP = this.helper.helperGetHexString(task.pxTopOfStack);
                taskInfo.StackLimit = this.helper.helperGetHexString(limit);
                taskInfo.MutexesHeld = task.uxMutexesHeld;
                this.getStaticAllocationInfo(taskInfo, task);
                await this.findStackPeakFast(taskInfo);
            }
            view.push(taskInfo);
        }
    }
    /* ======== findStackPeakFast ========
     * Finds the stack peak by first fetching the whole stack
     * and then finding the first non 0xa5a5a5a5 word
     */
    async findStackPeakFast(taskInfo) {
        /* Read the whole stack */
        let stackData = await this.Program.fetchFromAddr(taskInfo.StackLimit, "uint32_t", Number(taskInfo.StackSize / 4));
        let memIndex = Number(taskInfo.StackLimit);
        /* Begin from the end of the stack (lowest address) and find the first
         * location which does not have the standard 0xa5. */
        for (let index = 0; stackData[index] == 0xa5a5a5a5; index++) {
            memIndex += 4;
        }
        /* The r4, r5, r6, r7, r8, r9, r10, r11, r14 registers could have
         * had 0xa5a5a5a5 in them and make the stack "shrink" when they
         * are written back onto the stack. This happens in the
         * xPortPendSVHandler function.
         * To error on the safe side, subtract 8 registers * 4 bytes
         * from the index.
         * TIRTOS-2091 covers this in more details. */
        memIndex -= 8 * 4;
        memIndex -= 4;
        let unUsedStackSize = memIndex - Number(taskInfo.StackLimit);
        taskInfo.StackPeak = taskInfo.StackSize - unUsedStackSize;
    }
    /* ======== getTaskBasic ========
     * Entry point for basic task view. Sets makeDetailed to false
     */
    async getTaskBasic() {
        return this.getTasks(false);
    }
    /* ======== getTaskDetailed ========
     * Entry point for detailed task view. Sets makeDetailed to true
     */
    async getTaskDetailed() {
        return this.getTasks(true);
    }
    /* ======== getTaskScheduler ========
     * Fetches the available information from the task scheduler
     */
    async getTaskScheduler() {
        let view = new Array();
        let taskInfo = new TaskScheduler();
        let readyList = await this.Program.fetchVariable("pxReadyTasksLists");
        taskInfo.TickCount = await this.Program.fetchVariable("xTickCount");
        taskInfo.NumPriorities = readyList.length;
        taskInfo.NumTasks = await this.Program.fetchVariable("uxCurrentNumberOfTasks");
        taskInfo.TopReadyPriority = await this.Program.fetchVariable("uxTopReadyPriority");
        taskInfo.NumOverflows = await this.Program.fetchVariable("xNumOfOverflows");
        taskInfo.SchedulerStarted = Boolean(await this.Program.fetchVariable("xSchedulerRunning"));
        taskInfo.State = await this.Program.fetchVariable("uxSchedulerSuspended") ? "Suspended" : "Running";
        view.push(taskInfo);
        return view;
    }
}
exports.classCtor = Task;
