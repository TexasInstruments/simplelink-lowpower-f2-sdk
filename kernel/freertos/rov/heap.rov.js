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

/* global xdc, helperGetHexString */
let Program = xdc.module('xdc.rov.Program');

/* eslint-disable-next-line no-unused-vars */
var moduleName = "Heap";

/* eslint-disable-next-line no-unused-vars */
var viewMap = [
    {name: "Basic", fxn: "getBasicHeaps", structName: "BasicHeap", viewType: "Pages"},
    {name: "Detailed", fxn: "getDetailedHeaps", structName: "DetailedHeap", viewType: "Pages"},
    {name: "Free List", fxn: "getFreeList", structName: "FreeBlock"}
];

function BasicHeap(){
    this.HeapType       = null;
    this.HeapAddr       = null;
    this.TotalSize      = null;
    this.TotalFree      = null;
    this.PeakUsage      = null;
}

function DetailedHeap(){
    this.HeapType       = null;
    this.HeapAddr       = null;
    this.TotalSize      = null;
    this.TotalFree      = null;
    this.PeakUsage      = null;
    this.Allocations    = null;
    this.Frees          = null;
    this.FirstFreeBlock = null;
}

function FreeBlock() {
    this.Address = null;
    this.Size    = null;
}

/* ======== getHeap4 ======== */
function getHeap4(view, makeDetailed){

    view[0] = {label: "Heap 4", elements: []};

    let heapInfo = makeDetailed ? new DetailedHeap() : new BasicHeap();
    let current, end, ucHeapType;

    /* Verify that heap 4 is the one being used */
    try {
        current = Program.fetchVariable("xStart");
        end  = Program.fetchVariable("pxEnd");
        ucHeapType = Program.lookupTypeByVariable("ucHeap");
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

    let heapAddress   = Program.lookupSymbolValue("ucHeap");
    heapInfo.HeapAddr = helperGetHexString(heapAddress);
    heapInfo.HeapType = "heap_4";

    if (end == 0) {
        heapInfo.TotalSize = "Heap not initialized";
    }
    else {
        heapInfo.TotalSize = ucHeapType.elnum; // number of elements in ucHeap
        heapInfo.TotalFree = Program.fetchVariable("xFreeBytesRemaining");
        let minEverFreeRem = Program.fetchVariable("xMinimumEverFreeBytesRemaining");
        heapInfo.PeakUsage = heapInfo.TotalSize - minEverFreeRem;

        if (makeDetailed) {
            heapInfo.FirstFreeBlock = helperGetHexString(current.pxNextFreeBlock);
            heapInfo.Allocations = Program.fetchVariable("xNumberOfSuccessfulAllocations");
            heapInfo.Frees = Program.fetchVariable("xNumberOfSuccessfulFrees");
        }
    }

    view[0].elements[0] = heapInfo;

    return view;
}

/* ======== getHeaps ========
 * Dispatched entry point for the heap module
 */
function getHeaps(makeDetailed){
    let view = new Array();

    /* Since only one heap implementation will be used at a time
     * it might make more sense to have just one view that states
     * the currently used heap instead of the drop down menu
     * (even if more implementations are supported in the future) */

    getHeap4(view, makeDetailed);

    return view;
}

/* ======== getBasicHeaps ========
 * Entry point for basic semaphore view. Sets makeDetailed to false
 */
/* eslint-disable-next-line no-unused-vars */
function getBasicHeaps(){
    return getHeaps(false);
}

/* ======== getDetailedHeaps ========
 * Entry point for detailed semaphore view. Sets makeDetailed to true
 */
/* eslint-disable-next-line no-unused-vars */
function getDetailedHeaps(){
    return getHeaps(true);
}

/* ======== getFreeList ========
 * Fetches the free list by traversing the linked list
 */
/* eslint-disable-next-line no-unused-vars */
function getFreeList(){
    let freeList = new Array();
    let current, end;

    try {
        current  = Program.fetchVariable("xStart");
        end      = Program.fetchVariable("pxEnd");

        /* Add this test to make sure that heap 4 is being used */
        /* eslint-disable-next-line no-unused-vars */
        let test = Program.lookupTypeByVariable("ucHeap");
    }
    catch(e) {
        let block = new FreeBlock();
        block.Address = "Only heap 4 is supported by ROV";
        freeList.push(block);
        return freeList;
    }

    if (end == 0) return freeList;

    while (current.pxNextFreeBlock < end) {
        /* Traverse the list */
        let nextAddr = current.pxNextFreeBlock;
        current = Program.fetchFromAddr(nextAddr, "BlockLink_t");

        let block     = new FreeBlock();
        block.Address = nextAddr;
        block.Size    = current.xBlockSize;

        freeList.push(block);
    }

    return freeList;
}
