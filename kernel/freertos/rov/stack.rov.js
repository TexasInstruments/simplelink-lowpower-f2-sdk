/*
 * Copyright (c) 2022-2023, Texas Instruments Incorporated
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

/* global helperGetHexString */

/* eslint-disable-next-line no-unused-vars */
var moduleName = "System Stack";

/* eslint-disable-next-line no-unused-vars */
var viewMap = [
    {name: "System Stack", fxn: "getSystemStack", structName: "SystemStack"}
];

function SystemStack(){
    this.Address   = null;
    this.StackSize = null;
    this.StackPeak = null;
}

/* ======== getSystemStack ========
 * Function for getting the system stack module
 */
/* eslint-disable-next-line no-unused-vars */
function getSystemStack(){
    let view = new Array();

    let stackInfo = new SystemStack();

    /* __STACK_END is not present in the GCC compiler...just TI's
     * if the symbol cannot be found the return value is -1 */
    let isGccCompiler = (Program.lookupSymbolValue("__STACK_END") == -1);

    stackInfo.Address = helperGetHexString(Program.lookupSymbolValue("__stack"));
    if (isGccCompiler) {
        stackInfo.StackSize = Program.lookupSymbolValue("STACKSIZE");
    }
    else {
        stackInfo.StackSize = Program.lookupSymbolValue("__STACK_SIZE");
    }

    let stackData = Program.fetchArray(
        {
            type: 'xdc.rov.support.ScalarStructs.S_UChar',
            isScalar: true
        }, stackInfo.Address, stackInfo.StackSize);

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
