/*
 * Copyright (c) 2020-2022, Texas Instruments Incorporated
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
 *  ======== TFM.rov.js ========
 */

var moduleName = "ti.tfm::TFM";

var viewMap = [
    {name: "SPM", fxn: "viewSPM", structName: "SPM"},
    {name: "Secure Partitions", fxn: "viewSPs", structName: "SP"},
    {name: "Services", fxn: "viewServices", structName: "Service"},
    {name: "Messages", fxn: "viewMessages", structName: "Message"},
    {name: "Secure Logs", fxn: "viewSLog", structName: "Log"},
    {name: "Non Secure Logs", fxn: "viewNSLog", structName: "Log"}
];

/* SPM constructor */
function SPM() {
    this.is_init = 0;
    this.partition_count = 0;
}

/* SP constructor */
function SP() {
    this.part_id = 0;
    this.func = null;
    this.type = null;
    this.status = null;
    this.signal = null;
    this.stackBase = null;
    this.stackSize = null;
    this.stackPeak = null;
    this.address = null;
}

/* Service constructor */
function Service() {
    this.part_id = 0;
    this.sid = null;
    this.name = null;
    this.msgs = 0;
    this.msgHandle = null;
    this.msgType = null;
    this.numInVecs = 0;
    this.numOutVecs = 0;
    this.address = null;
}

/* Message constructor */
function Message() {
    this.part_id = 0;
    this.sid = null;
    this.msgHandle = null;
    this.msgType = null;
    this.numInVecs = 0;
    this.numOutVecs = 0;
}

/* Log constructor */
function Log() {
    this.msg = null;
}

/* Context constructor */
function Context() {
    this.contextId = null;
    this.core = null;       /* SOC Core name */
    this.s_ns = null;       /* Secure/NonSecure */
    this.p_up = null;       /* Priveleged/UnPrivileged */
    this.baseAddr = null;   /* Region Base address */
    this.endAddr = null;    /* Region end address */
}

/*
 * ======== inSecureState ========
 * Returns true if processor is in secure mode
 */
function inSecureState() {
    var DSCSR = Program.fetchFromAddr(0xe000ee08, "uint32_t", 1);

    if (DSCSR & 0x10000) {
        return (true);
    }
    else {
        return (false);
    }
}

/*
 * ======== toHexString ========
 * converts a number to a hex string with "0x" prefix.
 */
function toHexString(num) {
    return ("0x" + Number(num).toString(16));
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
 * ======== viewSPM ========
 */
function viewSPM() {
    var instView = new SPM();

    if (!inSecureState()) {
        return (instView);
    }

    var g_spm_partion_db = Program.fetchVariable("g_spm_partition_db");
    instView.is_init = g_spm_partion_db.is_init;
    instView.partition_count = g_spm_partion_db.partition_count;
    return (instView);
}

/*
 * ======== computeStackPeak ========
 * returns the max depth the stack has reached
 */
function computeStackPeak(stackBase, stackSize) {
    var stack = Program.fetchFromAddr(stackBase, "uint32_t", stackSize/4);

    for (var i = 0; i < stackSize/4; i++) {
        if (stack[i] != 0) return (toHexString(stackSize - i*4));
    }

    return ("Unknown");
}

/*
 * ======== decodeFlags ========
 * returns the set of partition type flags.
 */
function decodeFlags(flags) {
    var type = null;
    if (flags & 1) type = "APP_ROT";
    if (flags & 2) type = "PSA_ROT";
    if (flags & 4) type += " + IPC";
    return (type);
}

/*
 * ======== decodeStatus ========
 * returns TFM specific state of a Secure Partion
 */
function decodeStatus(status) {
    switch (status) {
        case 0:
            return "THRD_STAT_CREATING";
        case 1:
            return "THRD_STAT_RUNNING";
        case 2:
            return "THRD_STAT_BLOCK";
        case 3:
            return "THRD_STAT_DETACH";
        case 4:
            return "THRD_STAT_INVALID";
    }
}

/*
 * ======== charArrayToString ========
 * convert a C array of chars into a string
 */
function charArrayToString(charArray){
    var s = "";
    var i = 0;
    while (charArray[i] && (i < charArray.length)) {
        s += String.fromCharCode(charArray[i]);
        i++;
    }
    return s;
}

/*
 * ======== decodeMsgType ========
 * SP-specific msg.type decoder
 */
function decodeMsgType(type) {
    switch (type) {
        case -2: return ("CLOSE            ");
        case -1: return ("CONNECT          ");
        default: return ("CALL: " + toHexString(type));
    }
}

/*
 * ======== decodeSignal ========
 * generic SP signal decoder
 */
function decodeSignal(signal, srv_signal) {
    if (signal == srv_signal) {
        return ("IPC");
    }
    if (signal & srv_signal) {
        return ("IPC + " + toHexString(signal & ~srv_signal));
    }
    return (toHexString(signal));
}

/*
 * ======== viewSPs ========
 */
function viewSPs() {
    var view = new Array();

    if (!inSecureState()) {
        var instView = new SP();
        view.push(instView);
        return (view);
    }

    var g_spm_partion_db = Program.fetchVariable("g_spm_partition_db");
    var partitionsAddr = g_spm_partion_db.partitions;
    var partionType = Program.lookupType("spm_partition_desc_t");
    var partition_count = g_spm_partion_db.partition_count;
    var memory_data_listAddr = Program.lookupSymbolValue("memory_data_list");
    var service_dbAddr = Program.lookupSymbolValue("service_db");
    var service_db = Program.fetchFromAddr(
                            service_dbAddr,
                            "tfm_spm_service_db_t",
                            partition_count-1);
    var partitions = Program.fetchFromAddr(
                            partitionsAddr,
                            "spm_partition_desc_t",
                            partition_count);
    var memory_data_list = Program.fetchFromAddr(
                            memory_data_listAddr,
                            "tfm_spm_partition_memory_data_t",
                            partition_count);
    for (var i = 0; i < partition_count; i++) {
        var instView = new SP();
        var static_data = Program.fetchFromAddr(
                            partitions[i].static_data,
                            "spm_partition_static_data_t", 1);
        var runtime_data = partitions[i].runtime_data;
        instView.part_id = static_data.partition_id;
        instView.func = String(Program.lookupFuncName(
                            Number(static_data.partition_init)));
        instView.type = decodeFlags(static_data.partition_flags);
        instView.status = decodeStatus(runtime_data.sp_thrd.status);
        if (i > 0) {
            if (partition_count == 2) {
                instView.signal = decodeSignal(runtime_data.signals,
                                    service_db.signal);
            }
            else {
                instView.signal = decodeSignal(runtime_data.signals,
                                    service_db[i-1].signal);
            }
        }
        instView.stackBase = toHexString(memory_data_list[i].stack_bottom);
        instView.stackSize = toHexString(
                                memory_data_list[i].stack_top -
                                memory_data_list[i].stack_bottom);
        instView.stackPeak = computeStackPeak(
                                memory_data_list[i].stack_bottom,
                                Number(instView.stackSize));
        instView.address = toHexString(partitionsAddr + i*partionType.size);
        view.push(instView);
    }

    return (view);
}

/*
 * ======== viewServices ========
 */
function viewServices() {
    var view = new Array();

    if (!inSecureState()) {
        var instView = new Service();
        view.push(instView);
        return (view);
    }

    var service_db = Program.fetchVariable("service_db");
    var serviceAddr = Program.lookupSymbolValue("service");
    var serviceType = Program.lookupType("tfm_spm_service_t");
    var service = Program.fetchVariable("service");

    for (var x in service) {
        var instView = new Service();
        instView.part_id = service_db[x].partition_id;
        instView.sid = toHexString(service_db[x].sid);
        var nameAddr = service_db[x].name;
        var name = Program.fetchFromAddr(nameAddr, "char", 20);
        instView.name = charArrayToString(name);
        instView.msgs = service[x].msg_queue.size;
        if (instView.msgs) {
            var msgBody = Program.fetchFromAddr(
                            service[x].msg_queue.head, "tfm_msg_body_t", 1);
            instView.msgHandle = toHexString(msgBody.msg.handle);
            instView.msgType = decodeMsgType(msgBody.msg.type);

            /*
             * work around ROV not knowing the size of
             * in_size/out_size array elements
             */
            var msgBodyType = Program.lookupType("tfm_msg_body_t");
            var in_size_offset =
                            msgBodyType.member.msg.offset +
                            msgBodyType.member.msg.member.in_size.offset;
            var out_size_offset =
                            msgBodyType.member.msg.offset +
                            msgBodyType.member.msg.member.out_size.offset;
            var in_size_addr = service[x].msg_queue.head + in_size_offset;
            var in_size = Program.fetchFromAddr(in_size_addr, "uint32_t", 4);
            var out_size_addr = service[x].msg_queue.head + out_size_offset;
            var out_size = Program.fetchFromAddr(out_size_addr, "uint32_t", 4);
            /* end work around */

            for (var j = 0; j < 4; j++) {
                if (in_size[j] == 0) {
                    instView.numInVecs = j;
                    break;
                }
            }
            for (var j = 0; j < 4; j++) {
                if (out_size[j] == 0) {
                    instView.numOutVecs = j;
                    break;
                }
            }
        }
        instView.address = toHexString(serviceAddr + x*serviceType.size);
        view.push(instView);
    }

    return (view);
}

/*
 * ======== viewMessages ========
 */
function viewMessages() {
    var view = new Array();

    if (!inSecureState()) {
        var instView = new Message();
        view.push(instView);
        return (view);
    }

    var serviceAddr = Program.lookupSymbolValue("service");
    var serviceType = Program.lookupType("tfm_spm_service_t");
    var service = Program.fetchVariable("service");

    for (var x in service) {
        var service_db = Program.fetchFromAddr(
                            service[x].service_db,
                            "tfm_spm_service_db_t", 1);
        var msgBodyAddr = service[x].msg_queue.head;
        while (msgBodyAddr != 0) {
            var msgBody = Program.fetchFromAddr(msgBodyAddr,
                            "tfm_msg_body_t", 1);
            var instView = new Message();
            instView.part_id = service_db.partition_id;
            instView.sid = toHexString(service_db.sid);
            instView.msgHandle = toHexString(msgBody.msg.handle);
            instView.msgType = decodeMsgType(msgBody.msg.type);

            /*
             * work around ROV not knowing the size of
             * in_size/out_size array elements
             */
            var msgBodyType = Program.lookupType("tfm_msg_body_t");
            var in_size_offset =
                            msgBodyType.member.msg.offset +
                            msgBodyType.member.msg.member.in_size.offset;
            var out_size_offset =
                            msgBodyType.member.msg.offset +
                            msgBodyType.member.msg.member.out_size.offset;
            var in_size_addr = msgBodyAddr + in_size_offset;
            var in_size = Program.fetchFromAddr(in_size_addr, "uint32_t", 4);
            var out_size_addr = msgBodyAddr + out_size_offset;
            var out_size = Program.fetchFromAddr(out_size_addr, "uint32_t", 4);
            /* end work around */

            for (var j = 0; j < 4; j++) {
                if (in_size[j] == 0) {
                    instView.numInVecs = j;
                    break;
                }
            }
            for (var j = 0; j < 4; j++) {
                if (out_size[j] == 0) {
                    instView.numOutVecs = j;
                    break;
                }
            }
            view.push(instView);
            msgBodyAddr = msgBody.next;
        }
    }

    /* return null message instance if no messages to decode */
    if (view.length == 0) {
        var instView = new Message();
        view.push(instView);
    }

    return (view);
}

/*
 * ======== viewSLog ========
 */
function viewSLog() {
    var view = new Array();

    /* BQ - commenting this out to avoid clearing Secure log when entering NS state */
    /*if (!inSecureState()) {
        var log = new Log();
        view.push(log);
        return (view);
    }*/

    try {
        var uartOutputBufAddr = Program.lookupSymbolValue("uartOutputBuf_s");
        var uartOutputBufIndex = Program.fetchVariable("uartOutputBufIndex_s");
    }
    catch (e) {
        var log = new Log();
        log.msg = String(e);
        view.push(log);
        return (view);
    }

    var outbuf = Program.fetchFromAddr(uartOutputBufAddr, "char", uartOutputBufIndex);
    var index = 0;

    while (index < uartOutputBufIndex) {
        index += addMsgFromBuf(view, outbuf, index, uartOutputBufIndex);
    }

    return (view);
}

/*
 * ======== viewNSLog ========
 */
function viewNSLog() {
    var view = new Array();

    try {
        var uartOutputBufAddr = Program.lookupSymbolValue("uartOutputBuf_ns");
        var uartOutputBufIndex = Program.fetchVariable("uartOutputBufIndex_ns");
    }
    catch (e) {
        var log = new Log();
        log.msg = String(e);
        view.push(log);
        return (view);
    }

    var outbuf = Program.fetchFromAddr(uartOutputBufAddr, "char", uartOutputBufIndex);
    var index = 0;

    while (index < uartOutputBufIndex) {
        index += addMsgFromBuf(view, outbuf, index, uartOutputBufIndex);
    }

    return (view);
}

/*
 * ======== viewSLog ========
 */
function addMsgFromBuf(view, outbuf, index, max) {
    var str = "";
    var count = index;
    while (1) {
        var ch = outbuf[index++];
        if (ch == 0x0d) continue;
        if (ch == 0x0a) break;
        if (index >= max) break;
        str += String.fromCharCode(ch);
    }
    log = new Log();
    log.msg = str;
    view.push(log);

    count = index - count;
    return (count);
}
