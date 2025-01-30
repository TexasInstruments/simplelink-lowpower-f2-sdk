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

/* global xdc, helperGetHexString */
let Program   = xdc.module('xdc.rov.Program');
let CallStack = xdc.useModule('xdc.rov.CallStack');

/* eslint-disable-next-line no-unused-vars */
var moduleName = "Exception";

/* eslint-disable-next-line no-unused-vars */
var viewMap = [
    {name: "Exception Info", fxn: "viewExceptionInfo", structName: "ExceptionInfo"},
    {name: "Exception Call Stack", fxn: "viewCallStack", structName: "CallStackFrame"}
];

function ExcContext(){
    /* Core Registers */
    this.R0 = 0;
    this.R1 = 0;
    this.R2 = 0;
    this.R3 = 0;
    this.R4 = 0;
    this.R5 = 0;
    this.R6 = 0;
    this.R7 = 0;
    this.R8 = 0;
    this.R9 = 0;
    this.R10 = 0;
    this.R11 = 0;
    this.R12 = 0;
    this.SP = 0;
    this.LR = 0;
    this.PC = 0;
    this.PSR = 0;

    /* NVIC registers */

    this.ICSR = 0;
    this.SHCSR = 0;
    this.MMFSR = 0;
    this.BFSR = 0;
    this.UFSR = 0;
    this.HFSR = 0;
    this.DFSR = 0;
    this.MMAR = 0;
    this.BFAR = 0;
    this.AFSR = 0;
    this.SFSR = 0;
    this.SFAR = 0;
}

function ExceptionInfo() {
    this.exceptionInfo = null;
}

function CallStackFrame(){
    this.frame = null;
}

/*
 * ======== toPaddedHexString ========
 * converts a number to a hex string with "0x" prefix.
 * inserts up to (len - 1) leading zeros.
 * up to a max of 7 zeros.
 */
function toPaddedHexString(number, len)
{
    return ("0x" + ("0000000" + number.toString(16)).substr(-len));
}

/* ======== getExcContext ========
 * Gets the exception context, this is the register values that were present
 * before the exception occured. The PC will point to the next instruction
 * after the exception handling is done. Getting this context is done by the
 * OS for TIRTOS7 but is done manually here for FreeRTOS. We also fetch certain
 * fields from the NVIC that are used to decode the exception.
 */
function getExcContext() {
    /* See page 40 of version 1b of the
     * Cortex M4 Devices Generic User Guide. */

    CallStack.fetchRegisters(["xPSR", "SP", "PSP", "LR", "R4", "R5", "R6", "R7", "R8", "R9", "R10", "R11"]);

    // Check if an exception has triggered
    let currentxPSR = Number(CallStack.getRegister("xPSR"));
    let ISRNum = currentxPSR & 0x1ff;
    if (ISRNum < 2 || (ISRNum >= 8 && ISRNum <= 10)) {
        /* No exception to handle. See page 19 of version 1b
         * of the Cortex M4 Devices Generic User Guide.*/

        return null;
    }

    let newLR = CallStack.getRegister("LR");
    let stackPointer;
    if (newLR == 0xfffffffd || newLR == 0xffffffed) {
        // Used the program stack pointer before the exception
        stackPointer = CallStack.getRegister("PSP");
    }
    else {
        // Used the main stack pointer before the exception
        stackPointer = CallStack.getRegister("SP");
    }

    let oldStackPointer;
    if (newLR == 0xffffffe1 || newLR == 0xffffffe9 || newLR == 0xffffffed) {
        // Stores floating-point-state on the stack
        oldStackPointer = stackPointer + 26 * 4;
    }
    else {
        // Does not store floating-point-state on the stack
        oldStackPointer = stackPointer + 8 * 4;
    }

    let stackVals = Program.fetchFromAddr(stackPointer, "uint32_t", 8);
    let R0   = stackVals[0];
    let R1   = stackVals[1];
    let R2   = stackVals[2];
    let R3   = stackVals[3];
    let R4   = CallStack.getRegister("R4");
    let R5   = CallStack.getRegister("R5");
    let R6   = CallStack.getRegister("R6");
    let R7   = CallStack.getRegister("R7");
    let R8   = CallStack.getRegister("R8");
    let R9   = CallStack.getRegister("R9");
    let R10  = CallStack.getRegister("R10");
    let R11  = CallStack.getRegister("R11");
    let R12  = stackVals[4];
    let LR   = stackVals[5];
    let PC   = stackVals[6];
    let xPSR = stackVals[7];

    var excContext = new ExcContext();

    excContext.ICSR = Program.fetchFromAddr(0xe000ed04, "uint32_t", 1);

    var nvicRegs = Program.fetchFromAddr(0xe000ed24, "uint32_t", 7);

    excContext.SHCSR = nvicRegs[0];
    excContext.MMFSR = nvicRegs[1] & 0xff;
    excContext.BFSR  = (nvicRegs[1] & 0x0000ff00) >> 8;
    excContext.UFSR  = (nvicRegs[1] & 0xffff0000) >> 16;

    excContext.HFSR = nvicRegs[2];
    excContext.DFSR = nvicRegs[3];
    excContext.MMAR = nvicRegs[4];
    excContext.BFAR = nvicRegs[5];
    excContext.AFSR = nvicRegs[6];

    var sfRegs = Program.fetchFromAddr(0xe000ede4, "uint32_t", 6);
    excContext.SFSR = sfRegs[0];
    excContext.SFAR = sfRegs[1];

    excContext.R0 = R0;
    excContext.R1 = R1;
    excContext.R2 = R2;
    excContext.R3 = R3;
    excContext.R4 = R4;
    excContext.R5 = R5;
    excContext.R6 = R6;
    excContext.R7 = R7;
    excContext.R8 = R8;
    excContext.R9 = R9;
    excContext.R10 = R10;
    excContext.R11 = R11;
    excContext.R12 = R12;
    excContext.SP = oldStackPointer;
    excContext.LR = LR;
    excContext.PC = PC;
    excContext.PSR = xPSR;

    return excContext;
}

/*
 * =============================================
 * The way that the exceptions are decoded below
 * is exacly the same as for TIRTOS7.
 * ============================================
 */

/*
 *  ======== viewDecodeNMI ========
 */
function viewDecodeNMI(excContext){
    return("NMI Exception");
}

/*
 *  ======== viewDecodeHardFault ========
 */
function viewDecodeHardFault(excContext){
    var fault = "Hard Fault: ";

    if (excContext.HFSR & 0x40000000) {
        fault += "FORCED: ";
        fault += viewDecodeUsageFault(excContext);
        fault += viewDecodeBusFault(excContext);
        fault += viewDecodeMemFault(excContext);
        return (fault);
    }
    else if (excContext.HFSR & 0x80000000) {
        fault += "DEBUGEVT: ";
        fault += viewDecodeDebugMon(excContext);
        return (fault);
    }
    else if (excContext.HFSR & 0x00000002) {
        fault += "VECTBL";
    }
    else {
        fault += "Unknown";
    }

    return (fault);
}

/*
 *  ======== viewDecodeMemFault ========
 */
function viewDecodeMemFault(excContext){
    var fault = "";

    if (excContext.MMFSR != 0) {

        fault = "MEMFAULT: ";
        if (excContext.MMFSR & 0x10) {
            fault += "MSTKERR";
        }
        else if (excContext.MMFSR & 0x08) {
            fault += "MUNSTKERR";
        }
        else if (excContext.MMFSR & 0x02) {
            fault += "DACCVIOL ";
            fault += "Data Access Error. Address = 0x" + Number(excContext.MMAR).toString(16);
        }
        else if (excContext.MMFSR & 0x01) {
            fault += "IACCVIOL ";
            fault += "Instruction Fetch Error. Address = 0x" + Number(excContext.MMAR).toString(16);
        }
        else {
            fault += "Unknown";
        }
    }
    return (fault);
}

/*
 *  ======== viewDecodeBusFault ========
 */
function viewDecodeBusFault(excContext){
    var fault = "";

    if (excContext.BFSR != 0) {

        fault = "BUSFAULT: ";

        if (excContext.BFSR & 0x10) {
            fault += "STKERR";
        }
        else if (excContext.BFSR & 0x08) {
            fault += "UNSTKERR";
        }
        else if (excContext.BFSR & 0x04) {
            fault += "IMPRECISERR";
        }
        else if (excContext.BFSR & 0x02) {
            fault += "PRECISERR.";
            fault += "Data Access Error. Address = 0x" + Number(excContext.BFAR).toString(16);
        }
        else if (excContext.BFSR & 0x01) {
            fault += "IBUSERR";
        }
        else {
            fault += "Unknown";
        }
    }
    return (fault);
}

/*
 *  ======== viewDecodeUsageFault ========
 */
function viewDecodeUsageFault(excContext){
    var fault = "";

    if (excContext.UFSR != 0) {
        fault = "USAGE: ";
        if (excContext.UFSR & 0x0001) {
            fault += "UNDEFINSTR";
        }
        else if (excContext.UFSR & 0x0002) {
            fault += "INVSTATE";
        }
        else if (excContext.UFSR & 0x0004) {
            fault += "INVPC";
        }
        else if (excContext.UFSR & 0x0008) {
            fault += "NOCP";
        }
        else if (excContext.UFSR & 0x0010) {
            fault += "STKOF";
        }
        else if (excContext.UFSR & 0x0100) {
            fault += "UNALIGNED";
        }
        else if (excContext.UFSR & 0x0200) {
            fault += "DIVBYZERO";
        }
        else {
            fault += "Unknown";
        }
    }
    return (fault);
}

/*
 *  ======== viewDecodeSecureFault ========
 */
function viewDecodeSecureFault(excContext){
    var fault = "";

    var SFSR = excContext.SFSR;

    if (SFSR != 0) {
        fault = "SECFAULT:";
        if (SFSR & 0x80) {
            fault += " LSERR";
        }
        if (SFSR & 0x20) {
            fault += " LSPERR";
        }
        if (SFSR & 0x10) {
            fault += " INVTRAN";
        }
        if (SFSR & 0x08) {
            fault += " AUVIOL";
        }
        if (SFSR & 0x04) {
            fault += " INVERR";
        }
        if (SFSR & 0x02) {
            fault += " INVIS";
        }
        if (SFSR & 0x01) {
            fault += " INVEP (Jump to invalid SG address: " +
                      toPaddedHexString(excContext.PC,8) + ")";
        }
    }

    if (SFSR & 0x40) {
        fault += " Access error at address = " +
                  toPaddedHexString(excContext.SFAR, 8);
    }

    return (fault);
}

/*
 *  ======== viewDecodeSvCall ========
 */
function viewDecodeSvCall(excContext){
    return("SV Call Exception, pc = " + Number(excContext.pc).toString(16));
}

/*
 *  ======== viewDecodeDebugMon ========
 */
function viewDecodeDebugMon(excContext){
    var fault = "";

    if (excContext.DFSR != 0) {

        fault = "DEBUG: ";

        if (excContext.DFSR & 0x00000010) {
            fault += "EXTERNAL";
        }
        else if (excContext.DFSR & 0x00000008) {
            fault += "VCATCH";
        }
        else if (excContext.DFSR & 0x00000004) {
            fault += "DWTTRAP";
        }
        else if (excContext.DFSR & 0x00000002) {
            fault += "BKPT";
        }
        else if (excContext.DFSR & 0x00000001) {
            fault += "HALTED";
        }
        else {
            fault += "Unknown";
        }
    }
    return (fault);
}

/*
 *  ======== viewDecodeReserved ========
 */
function viewDecodeReserved(excContext, excNum){
    return ("Reserved vector: " + String(excNum));
}

/*
 *  ======== viewDecodeNoIsr ========
 */
function viewDecodeNoIsr(excContext, excNum){
    return ("Undefined Hwi: " + String(excNum));
}

/*
 *  ======== viewDecodeException ========
 */
function viewDecodeException(excContext){
    var excNum = String(excContext.ICSR & 0xff);

    switch (excNum) {
        case "2":
            return (viewDecodeNMI(excContext));         /* NMI */
        case "3":
            return (viewDecodeHardFault(excContext));   /* Hard Fault */
        case "4":
            return (viewDecodeMemFault(excContext));    /* Mem Fault */
        case "5":
            return (viewDecodeBusFault(excContext));    /* Bus Fault */
        case "6":
            return (viewDecodeUsageFault(excContext));  /* Usage Fault */
        case "7":
            return (viewDecodeSecureFault(excContext)); /* Secure Fault */
        case "11":
            return (viewDecodeSvCall(excContext));      /* SVCall */
        case "12":
            return (viewDecodeDebugMon(excContext));    /* Debug Mon */
        case "8":
        case "9":
        case "10":
        case "13":
            return (viewDecodeReserved(excContext, excNum));/* reserved */
        default:
            return (viewDecodeNoIsr(excContext, excNum));   /* no ISR */
    }

}

/*
 *  ======== viewExceptionInfo ========
 */
/* eslint-disable-next-line no-unused-vars */
function viewExceptionInfo(){
    var excContext = getExcContext();
    var instView;

    if (excContext == null) {
        instView = new ExceptionInfo();
        instView.exceptionInfo = "No Interpretable Exception Pending";
        return (instView);
    }

    var view = new Array();

    /* Add decoded exception */
    var decodeView = new ExceptionInfo();
    decodeView.exceptionInfo = "Decoded Exception:";
    view.push(decodeView);

    decodeView = new ExceptionInfo();
    decodeView.exceptionInfo = viewDecodeException(excContext);
    view.push(decodeView);

    var registersView = new ExceptionInfo();
    registersView.exceptionInfo = "---";
    view.push(registersView);

    /* Add register dump */
    registersView = new ExceptionInfo();
    registersView.exceptionInfo = "Registers:";
    view.push(registersView);

    for (var z in excContext) {
        instView = new ExceptionInfo();
        instView.exceptionInfo = z + " = " + helperGetHexString(excContext[z], 8);
        view.push(instView);
    }

    return (view);
}

/*
 *  ======== viewCallStack ========
 */
/* eslint-disable-next-line no-unused-vars */
function viewCallStack(){
    try {
        var CallStack = xdc.useModule('xdc.rov.CallStack');
        CallStack.clearRegisters();
    }
    catch (e) {
        return (null);
    }

    var excContext = getExcContext();
    var instView;

    if (excContext == null) {
        instView = new CallStackFrame();
        instView.frame = "No Interpretable Exception Pending";
        return (instView);
    }

    CallStack.setRegister("R0", Number(excContext.R0));
    CallStack.setRegister("R1", Number(excContext.R1));
    CallStack.setRegister("R2", Number(excContext.R2));
    CallStack.setRegister("R3", Number(excContext.R3));
    CallStack.setRegister("R4", Number(excContext.R4));
    CallStack.setRegister("R5", Number(excContext.R5));
    CallStack.setRegister("R6", Number(excContext.R6));
    CallStack.setRegister("R7", Number(excContext.R7));
    CallStack.setRegister("R8", Number(excContext.R8));
    CallStack.setRegister("R9", Number(excContext.R9));
    CallStack.setRegister("R10", Number(excContext.R10));
    CallStack.setRegister("R11", Number(excContext.R11));
    CallStack.setRegister("R12", Number(excContext.R12));
    CallStack.setRegister("SP", Number(excContext.SP));
    CallStack.setRegister("R13", Number(excContext.SP));
    CallStack.setRegister("R14", Number(excContext.LR));
    CallStack.setRegister("PC", Number(excContext.PC));
    CallStack.setRegister("xPSR", Number(excContext.PSR));

    /* fetch call stack string */
    var frames = CallStack.toText();

    /* break up into separate lines */
    frames = frames.split("\n");

    /* Try using the LR as the PC if there is no call stack using the PC */
    if (frames.length == 1) {

        /* Fixed below */
        /* Try making a bunch of function calls to get a call stack */
        CallStack.setRegister("PC", Number(excContext.LR));
        frames = CallStack.toText();
        frames = frames.split("\n");
        if (frames.length > 1) {
            frames[0] += " (Using LR as PC)";
        }
    }

    if (frames.length > 1) {
        /*
         * Strip off "Unwind halted ... " from last frame
         */
        frames.length -= 1;
    }
    else {
        /* No call stack */
        instView = new CallStackFrame();
        instView.frame = "No call stack";
        return (instView);
    }

    var i;
    for (i = 0; i < frames.length; i++) {
        var line = frames[i];
        /* separate frame # from frame text a little */
        line = line.replace(" ", "    ");
        var file = line.substr(line.indexOf(" at ") + 4);
        file = file.replace(/\\/g, "/");
        file = file.substr(file.lastIndexOf("/")+1);
        if (file != "") {
            frames[i] = line.substring(0,
                                   line.indexOf(" at ") + 4);
            /* tack on file info */
            frames[i] += file;
        }
    }
    /*
     * Invert the frames[] array so that the strings become the index of a
     * new associative array.
     *
     * This is done because the TREE view renders the array index (field)
     * on the left and the array value on the right.
     *
     * At the same time, extract the "PC = ..." substrings and make them
     * be the value of the array who's index is the beginning of the
     * frame text.
     */
    var invframes = new Array();

    for (i = 0; i < frames.length; i++) {
        invframes[frames[i].substring(0,frames[i].indexOf("PC")-1)] =
            frames[i].substr(frames[i].indexOf("PC"));
    }

    var view = new Array();

    for (var x in invframes) {
        instView = new CallStackFrame();
        instView.frame = " " + x + invframes[x];
        view.push(instView);
    }

    return (view);
}
