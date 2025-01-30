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
class NvicInterrupt {
}
class CC23XXNVIC {
}
class Nvic {
    constructor(ctx) {
        this.ctx = ctx;
        this.viewMap = [
            { name: 'Table', fxn: this.getNVIC.bind(this), structName: NvicInterrupt }
        ];
        this.Program = this.ctx.getProgram();
    }
    getModuleName() {
        return 'NVIC';
    }
    get helper() {
        return this.ctx.getModule('helper.rov.js');
    }
    /* ======== getDescription ========
     * Returns the type of interrupt based on the interrupt number */
    getDescription(interruptNum) {
        /* See page 19 of version 1b of Cortex M4 Devices Generic User Guide */
        if (interruptNum >= 16) {
            return "IRQ";
        }
        switch (interruptNum) {
            case 0:
                return "ThreadMode";
            case 2:
                return "NMI";
            case 3:
                return "HardFault";
            case 4:
                return "MemManage";
            case 5:
                return "BusFault";
            case 6:
                return "UsageFault";
            case 11:
                return "SVCall";
            case 14:
                return "PendSV";
            case 15:
                return "SysTick";
            default:
                return "Reserved";
        }
    }
    /* ======== parseNVICManually ========
     * Fetches the NVIC fields that are needed. This is done when a NVIC struct
     * is not present in the dpl implementation. */
    async parseNVICManually(address) {
        let NVIC = new CC23XXNVIC();
        NVIC.STCSR = await this.Program.fetchFromAddr(address + 16, "uint32_t", 1);
        NVIC.ISER = await this.Program.fetchFromAddr(address + 256, "uint32_t", 8);
        NVIC.ISPR = await this.Program.fetchFromAddr(address + 512, "uint32_t", 8);
        NVIC.IABR = await this.Program.fetchFromAddr(address + 768, "uint32_t", 8);
        NVIC.IPR = await this.Program.fetchFromAddr(address + 1024, "uint8_t", 240);
        NVIC.ICSR = await this.Program.fetchFromAddr(address + 3332, "uint32_t", 1);
        NVIC.VTOR = await this.Program.fetchFromAddr(address + 3336, "uint32_t", 1);
        NVIC.AIRCR = await this.Program.fetchFromAddr(address + 3340, "uint32_t", 1);
        NVIC.SHPR = await this.Program.fetchFromAddr(address + 3352, "uint8_t", 12);
        NVIC.SHCSR = await this.Program.fetchFromAddr(address + 3364, "uint32_t", 1);
        return NVIC;
    }
    /* ======== parseNVIC ========
     * Parses the NVIC based on its memory address, CC23XX does not have
     * a NVIC struct (that mirrors the memory leayout). Thus we parse it
     * manually in that case */
    async parseNVIC() {
        let NVIC;
        let addressOfNVIC = 0xE000E000;
        try {
            NVIC = await this.Program.fetchFromAddr(addressOfNVIC, "HwiP_NVIC", 1);
        }
        catch (e) {
            // CC23XX or struct name has been changed. Either way parse NVIC manually
            NVIC = await this.parseNVICManually(addressOfNVIC);
        }
        return NVIC;
    }
    /* ======== getVTORTable ========
     * Fetches the dispatch function addresses that corresponds to each interrupt */
    async getVTORTable(NVIC, numInterrupts) {
        let vtor = [];
        /* Bits 7-29 contains the address (other bits reserved) */
        let vtorMask = 0x3fffff80;
        let vtorAddress = NVIC.VTOR & vtorMask;
        /* Fetch the interrupt handler function addresses from VTOR.
         * This will be a list of pointers to handler functions. */
        vtor = await this.Program.fetchFromAddr(vtorAddress, "int32_t", numInterrupts);
        return vtor;
    }
    /* ======== getHwipDispatchMap ========
     * Creates a map from interruptNumber to name and address of
     * dispatch function for fast lookup. This is needed to not
     * have to iterate through the whole dispatchMap
     * once for every interrupt*/
    async getHwipDispatchMap(dispatchTable) {
        let map = {};
        for (let i = 0; i < dispatchTable.length; i++) {
            let hwiObjAddr = dispatchTable[i];
            if (hwiObjAddr == 0)
                continue;
            let hwiObj = await this.Program.fetchFromAddr(hwiObjAddr, "HwiP_Obj", 1);
            let dispatchFuncName = String(await this.helper.lookupFuncName(Number(hwiObj.fxn)));
            let dispatchFuncAddr = this.helper.helperGetHexString(Number(hwiObj.fxn));
            map[hwiObj.intNum] = [dispatchFuncAddr, dispatchFuncName];
        }
        return map;
    }
    /* ======== getNVIC ========
     * Main function for getting the NVIC view */
    async getNVIC() {
        let view = new Array();
        let NVIC = await this.parseNVIC();
        /* See page 618 of version E.e ARMv7-M Architecture Reference Manual
         * for how to find an upper limit to the number of interrupts.
         * "let numPossibleInterrupts = 32 * ((NVIC.ICTR & 7) + 1)"
         * However using the length of the dispatch table gets us
         * the exact answer */
        let dispatchTable = await this.Program.fetchVariable("HwiP_dispatchTable");
        let maxInterrupts = dispatchTable.length;
        let vtorAddresses = await this.getVTORTable(NVIC, maxInterrupts);
        let dispatchMap = await this.getHwipDispatchMap(dispatchTable);
        for (let i = 0; i < maxInterrupts; i++) {
            let NvicInt = new NvicInterrupt();
            NvicInt.InterruptNum = i;
            NvicInt.Description = this.getDescription(i);
            let vtorHandlerName = String(await this.helper.lookupFuncName(vtorAddresses[i]));
            NvicInt.VTORHandler = this.helper.helperGetHexString(vtorAddresses[i]) + " (" + vtorHandlerName + ")";
            if (vtorHandlerName == "HwiP_dispatch") {
                let [dispatchAddr, dispatchName] = dispatchMap[i];
                NvicInt.HwiPDispatch = dispatchAddr + " (" + dispatchName + ")";
            }
            let [enabled, active, pending] = this.helper.helperGetEnabledActivePending(i, NVIC);
            NvicInt.Enabled = (enabled == "-") ? "-" : (enabled ? 1 : 0);
            NvicInt.Active = (active == "-") ? "-" : (active ? 1 : 0);
            NvicInt.Pending = (pending == "-") ? "-" : (pending ? 1 : 0);
            NvicInt.Priority = this.helper.helperGetPriorityGivenIndex(i, NVIC);
            view.push(NvicInt);
        }
        return view;
    }
}
exports.classCtor = Nvic;
