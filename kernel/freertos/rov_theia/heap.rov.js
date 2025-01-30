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
class BasicHeap {
}
class DetailedHeap extends BasicHeap {
}
class FreeBlock {
}
class Heap {
    constructor(ctx) {
        this.ctx = ctx;
        this.viewMap = [
            { name: 'Basic', fxn: this.getBasicHeaps.bind(this), structName: BasicHeap },
            { name: 'Detailed', fxn: this.getDetailedHeaps.bind(this), structName: DetailedHeap },
            { name: 'Free List', fxn: this.getFreeList.bind(this), structName: FreeBlock }
        ];
        this.Program = this.ctx.getProgram();
    }
    getModuleName() {
        return 'Heap';
    }
    get helper() {
        return this.ctx.getModule('helper.rov.js');
    }
    /* ======== getHeap4 ======== */
    async getHeap4(view, makeDetailed) {
        // view[0] = {label: "Heap 4", elements: []};
        let heapInfo = makeDetailed ? new DetailedHeap() : new BasicHeap();
        let current, end, ucHeapType;
        /* Verify that heap 4 is the one being used */
        try {
            current = await this.Program.fetchVariable("xStart");
            end = await this.Program.fetchVariable("pxEnd");
            ucHeapType = await this.Program.lookupTypeByVariable("ucHeap");
        }
        catch (e) {
            /* Heap 1 does not have xStart and pxEnd
             * Heap 2 does not have pxEnd
             * Heap 3 does not have any of the the three
             * Heap 5 does not have ucHeap */
            heapInfo.HeapType = "Heap implementation not supported in ROV \
                            (only heap_4 is supported)";
            view.push(heapInfo);
            return view;
        }
        let heapAddress = await this.Program.lookupSymbolValue("ucHeap");
        heapInfo.HeapAddr = this.helper.helperGetHexString(heapAddress);
        heapInfo.HeapType = "heap_4";
        if (end == 0) {
            heapInfo.TotalSize = "Heap not initialized";
        }
        else {
            heapInfo.TotalSize = ucHeapType.elnum; // number of elements in ucHeap
            heapInfo.TotalFree = await this.Program.fetchVariable("xFreeBytesRemaining");
            let minEverFreeRem = await this.Program.fetchVariable("xMinimumEverFreeBytesRemaining");
            heapInfo.PeakUsage = heapInfo.TotalSize - minEverFreeRem;
            if (makeDetailed) {
                heapInfo.FirstFreeBlock = this.helper.helperGetHexString(current.pxNextFreeBlock);
                heapInfo.Allocations = await this.Program.fetchVariable("xNumberOfSuccessfulAllocations");
                heapInfo.Frees = await this.Program.fetchVariable("xNumberOfSuccessfulFrees");
            }
        }
        // view[0].elements[0] = heapInfo;
        view.push(heapInfo);
        return view;
    }
    /* ======== getHeaps ========
     * Dispatched entry point for the heap module
     */
    async getHeaps(makeDetailed) {
        let view = new Array();
        /* Since only one heap implementation will be used at a time
         * it might make more sense to have just one view that states
         * the currently used heap instead of the drop down menu
         * (even if more implementations are supported in the future) */
        await this.getHeap4(view, makeDetailed);
        return view;
    }
    /* ======== getBasicHeaps ========
     * Entry point for basic semaphore view. Sets makeDetailed to false
     */
    async getBasicHeaps() {
        return this.getHeaps(false);
    }
    /* ======== getDetailedHeaps ========
     * Entry point for detailed semaphore view. Sets makeDetailed to true
     */
    async getDetailedHeaps() {
        return this.getHeaps(true);
    }
    /* ======== getFreeList ========
     * Fetches the free list by traversing the linked list
     */
    async getFreeList() {
        let freeList = new Array();
        let current, end;
        try {
            current = await this.Program.fetchVariable("xStart");
            end = await this.Program.fetchVariable("pxEnd");
            /* Add this test to make sure that heap 4 is being used */
            /* eslint-disable-next-line no-unused-vars */
            let test = await this.Program.lookupTypeByVariable("ucHeap");
        }
        catch (e) {
            let block = new FreeBlock();
            block.Address = "Only heap 4 is supported by ROV";
            freeList.push(block);
            return freeList;
        }
        if (end == 0)
            return freeList;
        while (current.pxNextFreeBlock < end) {
            /* Traverse the list */
            let nextAddr = current.pxNextFreeBlock;
            current = await this.Program.fetchFromAddr(nextAddr, "BlockLink_t");
            let block = new FreeBlock();
            block.Address = '0x' + nextAddr.toString(16);
            block.Size = current.xBlockSize;
            freeList.push(block);
        }
        return freeList;
    }
}
exports.classCtor = Heap;
