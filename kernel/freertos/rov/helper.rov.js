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

/* Make the functions global */
xdc.global.helperGetHexString                = helperGetHexString;
xdc.global.helperCompareAddress              = helperCompareAddress;
xdc.global.helperReadStringFromAddr          = helperReadStringFromAddr;
xdc.global.helperGetNameOfTaskByAddress      = helperGetNameOfTaskByAddress;
xdc.global.helperGetNameOfTaskByTCBObj       = helperGetNameOfTaskByTCBObj;
xdc.global.helperGetListOfAddressesInListObj = helperGetListOfAddressesInListObj;
xdc.global.helperGetCurrentDevice            = helperGetCurrentDevice;
xdc.global.helperGetPriorityGivenIndex       = helperGetPriorityGivenIndex;
xdc.global.helperGetEnabledActivePending     = helperGetEnabledActivePending;

/*
 *
 * Global helper functions that are accessed by the ROV modules
 *
 */

/* ======== helperGetHexString ========
 * Function for converting a decimal number to a padded hex string
 */
function helperGetHexString(num){
    let hexStr = Number(num).toString(16);
    while (hexStr.length < 8) {
        hexStr = "0" + hexStr;
    }
    return "0x" + hexStr;
}

/* ======== helperCompareAddress ========
 * Helper function for sorting by address
 */
function helperCompareAddress(a, b){
    let comparison = 0;
    if (a.Address > b.Address) {
      comparison = 1;
    }
    else if (a.Address < b.Address) {
      comparison = -1;
    }
    return comparison;
}

/* ======== helperReadStringFromAddr ========
 * Reads a string from memory at address ptr at most maxLen
 * characters or until the first null character
 */
function helperReadStringFromAddr(ptr, maxLen){
    let name = "";
    let arr = Program.fetchFromAddr(ptr, "char", maxLen);
    for (let i = 0; i < arr.length; i++) {
        if (arr[i] == 0) break;
        name += String.fromCharCode(arr[i]);
    }
    return name;
}

/* ======== helperGetNameOfTaskByTCBObj ========
 * Reads the task name associated by a TCB (Task Control Block)
 */
function helperGetNameOfTaskByTCBObj(task){
    var name = "";
    /* Max length of a task name is defined in FreeRTOSConfig.h
     * with the label configMAX_TASK_NAME_LEN, it is set to 12 */
    for (var j = 0; j < 12; j++) {
        if (task.pcTaskName[j] == 0) break;
        name += String.fromCharCode(task.pcTaskName[j]);
    }
    return name;
}

/* ======== helperGetNameOfTaskByAddress ======== */
function helperGetNameOfTaskByAddress(taskAddress){
    let task = Program.fetchFromAddr(taskAddress, "TCB_t");
    return helperGetNameOfTaskByTCBObj(task);
}

/* ======== helperGetListOfAddressesInListObj ========
 * Gets the addresses of the objects in a List_t.
 * These object hold a ListItem_t in their struct and thus iterating
 * through the ListItems will give us the addresses we are interested in.
 * Examples of object addresses that are fetched with
 * this function are Tasks and Timers
 */
function helperGetListOfAddressesInListObj(listObj){
    let currentItem = Program.fetchFromAddr(listObj.xListEnd.pxNext, "ListItem_t");

    let list = [];

    for (let i = 0; i < listObj.uxNumberOfItems; i++) {
        let address = currentItem.pvOwner;
        list.push(address);

        /* Traverse the list */
        currentItem = Program.fetchFromAddr(currentItem.pxNext, "ListItem_t");
    }

    return list;
}

/* ======== helperGetCurrentDevice ========
 * Looks at available variables to determine the type of device
 * that is running
 */
/* eslint-disable no-unused-vars, no-empty */
function helperGetCurrentDevice(){
    try{
        let test = Program.fetchVariable("ClockP_list");
        return "CC23XX";
    }
    catch(e){}

    try{
        let test = Program.fetchVariable("ClockP_module");
        return "CC26X2";
    }
    catch(e){}

    /* Returns unknown to represent that this is an unknown
     * dpl implementation. Thus will try to parse the generic
     * ClockP_freertos.c */
    return "Unknown";
}
/* eslint-enable no-unused-vars, no-empty */

/* ======== helperGetPriorityGivenIndex ========
 * Gets the priority of a interrupt based on its interrupt number */
function helperGetPriorityGivenIndex(interruptNum, NVIC){
    /* In the FreeRTOS implementation of dpl, IPR is just a 240 byte long
     * array. This means that we simply access the corresponding byte from
     * that array with the IRQ number. */

    if (interruptNum >= 16) {
        let IRQNumber = interruptNum - 16;
        return NVIC.IPR[IRQNumber];
    }
    else{
        /* In the documentation. Page 237 in version 1b
         * of Cortex M4 Devices Generic User Guide SHPR uses three
         * 32 bit numbers to represent 12 8 bit fields
         * the FreeRTOS dpl implementation uses 12 8 bit numbers instead */
        switch (interruptNum) {
            case 4: // MemManage
                return NVIC.SHPR[0];
            case 5: // BusFault
                return NVIC.SHPR[1];
            case 6: // Usagefault
                return NVIC.SHPR[2];
            case 11: // SVCall
                return NVIC.SHPR[7];
            case 14: // PendSV
                return NVIC.SHPR[10];
            case 15: // SysTick
                return NVIC.SHPR[11];
            default:
                return "-";
        }
    }
}

/* ======== helperGetEnabledActivePending ========
 * Gets status information about an interrupt given its interrupt number
 */
function helperGetEnabledActivePending(interruptNum, NVIC){

    /* ISER, IABR and ISPR holds 8 32 bit numbers where
     * each bit represents the state of a single interrupt.
     * Thus we find the bit that corresponds to this interrupt
     * num by first getting the correct 32-bit number and then
     * finding the relevant bit in that number. The -16 comes from that
     * the first 16 interrupt numbers are reserved.
     * For further documentation see page 19 of version 1b of
     * Cortex M4 Devices Generic User Guide */
    let enabled, active, pending;
    if (interruptNum >= 16) {
        let IRQNumber = interruptNum - 16;
        let index     = Math.floor(IRQNumber / 32);
        let bit       = 1 << (IRQNumber % 32);

        enabled = Boolean(NVIC.ISER[index] & bit);
        active  = Boolean(NVIC.IABR[index] & bit);
        pending = Boolean(NVIC.ISPR[index] & bit);
    }
    else {
        /* See page 239 of version 1b of Cortex M4 Devices Generic User Guide */
        switch (interruptNum) {
            case 4: // MemManage
                enabled = "-";
                active  = Boolean(NVIC.SHCSR & 1);
                pending = Boolean(NVIC.SHCSR & (1 << 13));
                break;
            case 5: // BusFault
                enabled = "-";
                active  = Boolean(NVIC.SHCSR & (1 << 1));
                pending = Boolean(NVIC.SHCSR & (1 << 14));
                break;
            case 6: // UsageFault
                enabled = "-";
                active  = Boolean(NVIC.SHCSR & (1 << 3));
                pending = Boolean(NVIC.SHCSR & (1 << 12));
                break;
            case 11: // SVCall
                enabled = "-";
                active  = Boolean(NVIC.SHCSR & (1 << 7));
                pending = Boolean(NVIC.SHCSR & (1 << 15));
                break;
            case 14: // PendSV
                enabled = "-";
                active  = Boolean(NVIC.SHCSR & (1 << 10));
                pending = Boolean(NVIC.SHCSR & (1 << 15));
                break;
            case 15: // SysTick
                // See the documentation
                enabled = Boolean(NVIC.STCSR & 1);
                active  = Boolean(NVIC.SHCSR & (1 << 11));
                pending = Boolean(NVIC.ICSR & (1 << 26));
                break;
            default:
                enabled = "-";
                active  = "-";
                pending = "-";
                break;
        }
    }
    return [enabled, active, pending];
}
