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
class SystemStack {
}
class Stack {
    constructor(ctx) {
        this.ctx = ctx;
        this.viewMap = [
            { name: 'System Stack', fxn: this.getSystemStack.bind(this), structName: SystemStack }
        ];
        this.Program = this.ctx.getProgram();
    }
    getModuleName() {
        return 'System Stack';
    }
    get helper() {
        return this.ctx.getModule('helper.rov.js');
    }
    /* ======== getSystemStack ========
     * Function for getting the system stack module
     */
    async getSystemStack() {
        let view = new Array();
        let stackInfo = new SystemStack();
        /* __STACK_END is not present in the GCC compiler...just TI's
         * if the symbol cannot be found the return value is -1 */
        let isGccCompiler = await (this.Program.lookupSymbolValue("__STACK_END")) == -1;
        stackInfo.Address = this.helper.helperGetHexString(await this.Program.lookupSymbolValue("__stack"));
        if (isGccCompiler) {
            stackInfo.StackSize = await this.Program.lookupSymbolValue("STACKSIZE");
        }
        else {
            stackInfo.StackSize = await this.Program.lookupSymbolValue("__STACK_SIZE");
        }

        let stackData = await this.Program.fetchArray(
        // {
        //     type: 'xdc.rov.support.ScalarStructs.S_UChar',
        //     isScalar: true
        // },
        '8u', stackInfo.Address, stackInfo.StackSize);
        let index = 0;
        /*
         * Find the first non-0xa5.
         */
        while (stackData[index] == 0xa5) {
            index += 4;
        }
        if (index < 4) {
            stackInfo.StackPeak = "Stack Overflow (or configENABLE_ISR_STACK_INIT is 0 in FreeRTOSConfig.h)";
        }
        else {
            stackInfo.StackPeak = stackInfo.StackSize - index;
        }
        view.push(stackInfo);
        return view;
    }
}
exports.classCtor = Stack;
