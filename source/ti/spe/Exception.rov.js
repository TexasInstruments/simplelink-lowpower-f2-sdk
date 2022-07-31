/*
 * Copyright (c) 2019-2020, Texas Instruments Incorporated
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

/*
 *  ======== Exception.rov.js ========
 */

var moduleName = "ti.spe::Exception";

var viewMap = [
    {name: "Call Stack", fxn: "viewCallStack", structName: "CallStackFrame"},
    {name: "Exception Info", fxn: "viewExceptionInfo", structName: "ExceptionInfo"}
];

/*
 * CallStackFrame constructor
 */
function CallStackFrame()
{
    this.frame = null;
}

/*
 * ExceptionInfo constructor
 */
function ExceptionInfo()
{
    this.exceptionInfo = null;
}

/*
 * Exception Context constructor
 */
function ExcContext()
{
    /* Internal Registers */
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

/*
 * ======== getExcContext ========
 * returns an ExcContext filled with the register set that was valid
 * at the time an exception occurred. Registers R4-R11 and the various
 * status registers are only guranteed to be valid if the exception
 * was a secure fault.
 */
function getExcContext()
{
    try {
        var CallStack = xdc.useModule('xdc.rov.CallStack');
        CallStack.clearRegisters();
    }
    catch (e) {
        return (null);
    }

    CallStack.fetchRegisters(["xPSR"]);

    var xPSR = CallStack.getRegister("xPSR");

    if ((Number(xPSR) & 0xff) == 0) {
        return (null);
    }

    var vecNum = Number(xPSR) & 0xff;

    var excContext = new ExcContext();

    excContext.ICSR = Program.fetchFromAddr(0xe000ed04, "uint32_t", 1);

    var nvicRegs = Program.fetchFromAddr(0xe000ed24, "uint32_t", 7);

    excContext.SHCSR = nvicRegs[0];
    excContext.MMFSR = nvicRegs[1] & 0xff;
    excContext.BFSR = (nvicRegs[1] & 0x0000ff00) >> 8;
    excContext.UFSR = (nvicRegs[1] & 0xffff0000) >> 16;

    /* untouched S_Ptr types render as HEX */
    excContext.HFSR = nvicRegs[2];
    excContext.DFSR = nvicRegs[3];
    excContext.MMAR = nvicRegs[4];
    excContext.BFAR = nvicRegs[5];
    excContext.AFSR = nvicRegs[6];

    var sfRegs = Program.fetchFromAddr(0xe000ede4, "uint32_t", 6);
    excContext.SFSR = sfRegs[0];
    excContext.SFAR = sfRegs[1];

    CallStack.fetchRegisters(["R14", "PSP", "MSP", "R0", "R1", "R2"]);

    CallStack.fetchRegisters(["R4", "R5", "R6", "R7",
                              "R8", "R9", "R9", "R10",
                              "R11"]);

    var LR = CallStack.getRegister("R14");
    var SP;

    if (LR & 0x40) { /* interrupted secure mode */
        var MSP = CallStack.getRegister("MSP");
        var PSP = CallStack.getRegister("PSP");
        var S_MODE = true;
    }
    else {          /* interrupted non-secure mode */
        if (vecNum < 15) {
            /*
             * MSP_NS, PSP_NS, CONTROL_NS were copied to
             * R0, R1, R2 by the secure exception handler
             */
            var MSP = CallStack.getRegister("R0");
            var PSP = CallStack.getRegister("R1");
            CONTROL = CallStack.getRegister("R2");
            var S_MODE = false;
        }
        else {
            /*
             * Currently (12/3/2019), CCS doesn't know how
             * to fetch MSP_NS, PSP_NS, CONTROL_NS.
             */
             return (null);
        }
    }

    if (S_MODE) {
        if (LR & 0x4) {
            SP = PSP;
        }
        else {
            SP = MSP;
        }
    }
    else {
        if (CONTROL & 0x2) {
            SP = PSP;
        }
        else {
            SP = MSP;
        }
    }

    try {
        var context = Program.fetchFromAddr(SP, "uint32_t", 8);
        excContext.R0 = context[0];
        excContext.R1 = context[1];
        excContext.R2 = context[2];
        excContext.R3 = context[3];
        excContext.R12 = context[4];
        excContext.LR = context[5];
        excContext.PC = context[6];
        excContext.PSR = context[7];
    }
    catch (e) {
        Program.print(String(e));
    }

    SP += 8*4;   /* standard 8 word stack frame */

    if ((LR & 0x10) == 0) {  /* FTYPE */
        SP += 18*4;    /* Floating point regs */
    }

    excContext.R4 = CallStack.getRegister("R4");
    excContext.R5 = CallStack.getRegister("R5");
    excContext.R6 = CallStack.getRegister("R6");
    excContext.R7 = CallStack.getRegister("R7");
    excContext.R8 = CallStack.getRegister("R8");
    excContext.R9 = CallStack.getRegister("R9");
    excContext.R10 = CallStack.getRegister("R10");
    excContext.R11 = CallStack.getRegister("R11");
    excContext.SP = SP;

    return (excContext);
}

/*
 *  ======== viewCallStack ========
 */
function viewCallStack()
{
    try {
        var CallStack = xdc.useModule('xdc.rov.CallStack');
        CallStack.clearRegisters();
    }
    catch (e) {
        return (null);
    }

    var excContext = getExcContext();

    if (excContext == null) {
        var instView = new CallStackFrame();
        instView.frame = "No Interpretable Exception Pending";
        return (instView);
    }

    CallStack.setRegister("R0", excContext.R0);
    CallStack.setRegister("R1", excContext.R1);
    CallStack.setRegister("R2", excContext.R2);
    CallStack.setRegister("R3", excContext.R3);
    CallStack.setRegister("R12", excContext.R12);
    CallStack.setRegister("SP", excContext.SP);
    CallStack.setRegister("R13", excContext.SP);
    CallStack.setRegister("R14", excContext.LR);
    CallStack.setRegister("PC", excContext.PC);
    CallStack.setRegister("xPSR", excContext.PSR);

    /* fetch call stack string */
    var frames = CallStack.toText();
    /* break up into separate lines */
    frames = frames.split("\n");

    /* Try using the LR as the PC if there is no call stack using the PC */
    if (frames.length == 1) {
        CallStack.setRegister("PC", excContext.LR);
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
        var instView = new CallStackFrame();
        instView.frame = "No call stack";
        return (instView);
    }

    for (var i = 0; i < frames.length; i++) {
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

    for (var i = 0; i < frames.length; i++) {
        invframes[frames[i].substring(0,frames[i].indexOf("PC")-1)] =
            frames[i].substr(frames[i].indexOf("PC"));
    }

    var view = new Array();

    for (var x in invframes) {
        var instView = new CallStackFrame();
        instView.frame = " " + x + invframes[x];
        view.push(instView);
    }

    return (view);
}

/*
 *  ======== viewExceptionInfo ========
 */
function viewExceptionInfo()
{
    var excContext = getExcContext();

    if (excContext == null) {
        var instView = new ExceptionInfo();
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
        var instView = new ExceptionInfo();
        instView.exceptionInfo = z + " = " + toPaddedHexString(excContext[z], 8);
        view.push(instView);
    }

    return (view);
}

/*
 *  ======== viewDecodeNMI ========
 */
function viewDecodeNMI(excContext)
{
    return("NMI Exception");
}

/*
 *  ======== viewDecodeHardFault ========
 */
function viewDecodeHardFault(excContext)
{
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
function viewDecodeMemFault(excContext)
{
    var fault = ""

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
function viewDecodeBusFault(excContext)
{
    var fault = ""

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
function viewDecodeUsageFault(excContext)
{
    var fault = ""

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
function viewDecodeSecureFault(excContext)
{
    var fault = ""

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
function viewDecodeSvCall(excContext)
{
    return("SV Call Exception, pc = " + Number(excContext.pc).toString(16));
}

/*
 *  ======== viewDecodeDebugMon ========
 */
function viewDecodeDebugMon(excContext)
{
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
function viewDecodeReserved(excContext, excNum)
{
    return ("Reserved vector: " + String(excNum));
}

/*
 *  ======== viewDecodeSecureIsr ========
 */
function viewDecodeSecureIsr(excContext, excNum)
{
    var vtor = Program.fetchFromAddr(0xE000ED08, "uint32_t", 1);
    var vectorTable = Program.fetchFromAddr(vtor, "uint32_t", 120);
    var vectorFunc = Program.lookupFuncName(Number(vectorTable[excNum]));
    return ("IRQ Number: " + String(excNum) + " (" + vectorFunc + ")");
}

/*
 *  ======== viewDecodeException ========
 */
function viewDecodeException(excContext)
{
    var Program = xdc.useModule('xdc.rov.Program');
    var excNum = String(excContext.ICSR & 0xff);

    switch (excNum) {
        case "2":
            return (viewDecodeNMI(excContext));         /* NMI */
            break;
        case "3":
            return (viewDecodeHardFault(excContext));   /* Hard Fault */
            break;
        case "4":
            return (viewDecodeMemFault(excContext));    /* Mem Fault */
            break;
        case "5":
            return (viewDecodeBusFault(excContext));    /* Bus Fault */
            break;
        case "6":
            return (viewDecodeUsageFault(excContext));  /* Usage Fault */
            break;
        case "7":
            return (viewDecodeSecureFault(excContext)); /* Secure Fault */
            break;
        case "11":
            return (viewDecodeSvCall(excContext));      /* SVCall */
            break;
        case "12":
            return (viewDecodeDebugMon(excContext));    /* Debug Mon */
            break;
        case "8":
        case "9":
        case "10":
        case "13":
            return (viewDecodeReserved(excContext, excNum));/* reserved */
            break;
        default:
            return (viewDecodeSecureIsr(excContext, excNum));   /* Secure ISR */
            break;
    }
    return (null);
}
