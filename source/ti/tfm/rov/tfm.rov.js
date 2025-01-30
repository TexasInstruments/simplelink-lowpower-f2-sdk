/*
 * Copyright (c) 2020-2023, Texas Instruments Incorporated
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

/* eslint-disable-next-line no-unused-vars */
var viewMap = [
    {name: "Secure Partitions", fxn: "viewSPs", structName: "SP"},
    {name: "Services", fxn: "viewServices", structName: "Service"},
    {name: "Connections", fxn: "viewConnections", structName: "Connection"},
    {name: "Assets", fxn: "viewAssets", structName: "Asset"},
    {name: "Secure Logs", fxn: "viewSLog", structName: "Log"},
    {name: "Non-Secure Logs", fxn: "viewNSLog", structName: "Log"}
];

const PARTITION_EXT_INFO_LENGTH = 8;  /* Partition stack and heap addr members occupy 8-bytes */
const PARTITION_DEP_SIZE = 4;         /* Partition dependancies are defined as uint32_t */
const PSA_MAX_IOVEC = 4;              /* Max number of input/output vectors */
const PARTITION_FLAGS_PRIORITY_MASK = 0xFF;

/* SP constructor */
function SP() {
    this.pid = null;
    this.function = null;
    this.type = null;
    this.priority = null;
    this.state = null;
    this.sigAllowed = null;
    this.sigAsserted = null;
    this.stackBase = null;
    this.stackPtr = null;
    this.stackSize = null;
    this.stackPeak = null;
    this.boundary = null;
    this.nIRQs = null;
    this.nServices = null;
    this.nDeps = null;
    this.nAssets = null;
}

/* Service constructor */
function Service() {
    this.pid = null;
    this.sid = null;
    this.name = null;
    this.type = null;
    this.version = null;
    this.sfn = null;
    this.signal = null;
}

/* Connection constructor */
function Connection() {
    this.status = null;
    this.callerPID = null;
    this.service = null;
    this.msgType = null;
    this.msgHandle = null;
    this.msgClientID = null;
    this.numInVecs = null;
    this.numOutVecs = null;
}

/* Asset constructor */
function Asset() {
    this.pid = null;
    this.memStart = null;
    this.memLimit = null;
    this.ppcBank = null;
    this.ppcMask = null;
    this.attr = null;
}

/* Log constructor */
function Log() {
    this.msg = null;
}

/*
 * ======== isSecureState ========
 * Returns true if processor is in secure mode
 */
/* eslint-disable-next-line no-unused-vars */
function isSecureState() {
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
 * ======== computeStackPeak ========
 * returns the max depth the stack has reached
 */
function computeStackPeak(stackLimit, stackSize) {
    var stack = Program.fetchFromAddr(stackLimit, "uint32_t", (stackSize / 4));

    for (var i = 0; i < (stackSize / 4); i++) {
        if (stack[i] != 0) return (toHexString(stackSize - (i * 4)));
    }

    return ("Unknown");
}

/*
 * ======== decodePartitionFlags ========
 * returns string of TFM-specific partition type flags.
 */
function decodePartitionFlags(flags) {
    var type = null;

    /* The following match "PARTITION_" flag defines in TF-M code */
    type = (flags & 0x100) ? "PRoT" : "ARoT";
    type += (flags & 0x200) ? " + IPC" : " + SFN";
    /* The following flags are commented out as the info is redundant with the
     * partition name.
     */
    // type += (flags & 0x400) ? " + NS Agent" : "";
    // type += (flags & 0x800) ? " TZ" : "";

    return (type);
}

/*
 * ======== decodeBoundaryFlags ========
 * returns string of TFM-specific boundary flags.
 */
function decodeBoundaryFlags(flags) {
    var type = "";

    /* The following match "HANDLE_ATTR_xxxx_MASK" defines in TF-M code */
    type += (flags & 0x1) ? "NS + " : "";
    type += (flags & 0x2) ? "PRIVILEDGED + " : "";

    /* Strip any trailing " + " */
    type = type.replace(/\W{3}$/, '');

    return (type);
}

/*
 * ======== isIPCPartition ========
 * returns true if partition type is IPC model.
 */
function isIPCPartition(partition_flags) {
    return ((partition_flags & 0x200) ? true : false);
}

/*
 * ======== decodeServiceFlags ========
 * returns string of TFM-specific service type flags.
 */
function decodeServiceFlags(flags) {
    var type = "";

    /* The following match "SERVICE_FLAG_" defines in TF-M code */
    type += (flags & 0x100) ? "NS_ACCESS + " : "";
    type += (flags & 0x200) ? "STATELESS + " : "";
    type += (flags & 0x400) ? "STRICT_VER + " : "";
    type += (flags & 0x800) ? "MM_IOVEC" : "";

    /* Strip any trailing " + " */
    type = type.replace(/\W{3}$/, '');

    return (type);
}

/*
 * ======== decodeIPCState ========
 * returns string of TFM-specific thread state of a IPC model Secure Partition
 */
function decodeIPCState(threadState) {
    switch (threadState) {
        /* The following match "THRD_STATE_" defines in TF-M code */
        case 0:
            return "CREATING";
        case 1:
            return "RUNNABLE";
        case 2:
            return "BLOCK";
        case 3:
            return "DETACH";
        case 4:
            return "INVALID";
        case 5:
            return "RET_VAL_AVAIL";
        default:
            return threadState + ": UNKNOWN";
    }
}

/*
 * ======== decodeSFNState ========
 * returns string of TFM-specific state of a SFN model Secure Partition
 */
function decodeSFNState(state) {
    switch (state) {
        case 0:
            return "NOT_INITED";
        case 1:
            return "INITED";
        default:
            return state + ": INVALID";
    }
}

/*
 * ======== decodePriority ========
 * returns string of TFM-specific priority of a Secure Partition
 */
function decodePriority(flags) {
    switch (flags & PARTITION_FLAGS_PRIORITY_MASK) {
        /* The following match "PARTITION_PRI_" defines in TF-M code */
        case 0:
            return "HIGHEST";
        case 0xF:
            return "HIGH";
        case 0x1F:
            return "NORMAL";
        case 0x7F:
            return "LOW";
        case 0xFF:
            return "LOWEST";
        case 0xFE:
            /* This special priority is used by NS Agent */
            return "LOWEST - 1";
        default:
            return toHexString(flags & PARTITION_FLAGS_PRIORITY_MASK);
    }
}

/*
 * ======== decodeConnectionStatus ========
 * returns string of TFM-specific connection status.
 */
function decodeConnectionStatus(status) {
    switch (status) {
        /* The following match "TFM_HANDLE_STATUS_" defines in TF-M code */
        case 0:
            return "IDLE";
        case 1:
            return "ACTIVE";
        case 2:
            return "TO_FREE";
        default:
            return status + ": INVALID";
    }
}

/*
 * ======== decodeMsgType ========
 * returns string of TFM-specific message type.
 */
function decodeMsgType(type) {
    switch (type) {
        /* The following match "PSA_IPC_" defines in TF-M code */
        case -2:
            return "DISCONNECT";
        case -1:
            return "CONNECT";
        default:
            /* SP-specific msg type */
            return "CALL: " + toHexString(type);
    }
}

/*
 * ======== decodeAssetAttr ========
 * returns string of TFM-specific asset attribute flags.
 */
/* eslint-disable-next-line no-unused-vars */
function decodeAssetAttr(flags) {
    var type = "";

    /* The following match "ASSET_ATTR_" defines in TF-M code */
    type += (flags & 0x1) ? "RO + " : "";
    type += (flags & 0x2) ? "R/W + " : "";
    type += (flags & 0x4) ? "PRIV_PERIPH_BUS + " : "";
    type += (flags & 0x8) ? "NAMED MMIO + " : "";
    type += (flags & 0x10) ? "NUMBERED MMIO + " : "";

    /* Strip any trailing " + " */
    type = type.replace(/\W{3}$/, '');

    return (type);
}

/*
 * ======== isNamedMMIO ========
 * returns true if asset type is Named MMIO.
 */
function isNamedMMIO(asset_flags) {
    return ((asset_flags & 0x8) ? true : false);
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
 * ======== getPartitions ========
 */
function getPartitions() {

    var partitions = [];
    var head = Program.fetchVariable("partition_listhead");
    var sp = head;

     /* Traverse link list of partitions and fetch partition structs */
    while (sp.next != 0) {
        sp = Program.fetchFromAddr(sp.next, "partition_t");
        partitions.push(sp);
    }

    return partitions;
}

/*
 * ======== viewSPs ========
 */
/* eslint-disable-next-line no-unused-vars */
function viewSPs() {
    var view = new Array();
    var partitions = getPartitions();

    var currThrdAddr = Program.fetchVariable("p_curr_thrd");
    var currThrd = Program.fetchFromAddr(currThrdAddr, "thread_t");

    for (var i = 0; i < partitions.length; i++) {
        var instView = new SP();
        var loadInfo = Program.fetchFromAddr(partitions[i].p_ldinf, "partition_load_info_t");

        instView.pid = loadInfo.pid;
        instView.function = String(Program.lookupFuncName(loadInfo.entry));
        instView.type = decodePartitionFlags(loadInfo.flags);
        instView.priority = decodePriority(loadInfo.flags);
        instView.sigAllowed = toHexString(partitions[i].signals_allowed);
        instView.sigAsserted = toHexString(partitions[i].signals_asserted);
        instView.stackSize = toHexString(loadInfo.stack_size);

        if (isIPCPartition(loadInfo.flags)) {
            /* IPC model */
            instView.stackBase = toHexString(partitions[i].ctx_ctrl.sp_base);
            instView.stackPeak = computeStackPeak(partitions[i].ctx_ctrl.sp_limit, loadInfo.stack_size);
            instView.stackPtr = toHexString(partitions[i].ctx_ctrl.sp);
            instView.state = decodeIPCState(partitions[i].thrd.state);

            if (instView.stackPeak > loadInfo.stack_size) {
                Program.displayError(instView, "stackPeak", "Stack overflow!");
            }

            if (Number(partitions[i].thrd.p_context_ctrl) == currThrd.p_context_ctrl) {
                /* Set current thread indicator */
                instView.function = instView.function + "*";
            }
        }
        else {
            /* SFN model */
            instView.state = decodeSFNState(partitions[i].state);
        }

        instView.boundary = decodeBoundaryFlags(partitions[i].boundary);
        instView.nDeps = loadInfo.ndeps;
        instView.nServices = loadInfo.nservices;
        instView.nAssets = loadInfo.nassets;
        instView.nIRQs = loadInfo.nirqs;

        view.push(instView);
    }

    return (view);
}

/*
 * ======== viewServices ========
 */
/* eslint-disable-next-line no-unused-vars */
function viewServices() {
    var view = new Array();
    var partitions = getPartitions();
    var partLoadInfoType = Program.lookupType("partition_load_info_t");
    var servLoadInfoType = Program.lookupType("service_load_info_t");

    for (var i = 0; i < partitions.length; i++) {
        var partLoadInfo = Program.fetchFromAddr(partitions[i].p_ldinf, "partition_load_info_t");

        for (var j = 0; j < partLoadInfo.nservices; j++) {
            var instView = new Service();

            /* Get the service info for the partition */
            var servLoadInfoAddr = partitions[i].p_ldinf + partLoadInfoType.size +
                PARTITION_EXT_INFO_LENGTH + (partLoadInfo.ndeps * PARTITION_DEP_SIZE) +
                (j * servLoadInfoType.size);
            var servLoadInfo = Program.fetchFromAddr(servLoadInfoAddr, "service_load_info_t");

            instView.pid = partLoadInfo.pid;
            instView.sid = toHexString(servLoadInfo.sid);

            var nameAddr = servLoadInfo.name_strid;
            var name = Program.fetchFromAddr(nameAddr, "char", 25);
            instView.name = charArrayToString(name);

            instView.type = decodeServiceFlags(servLoadInfo.flags);
            instView.version = servLoadInfo.version;
            instView.sfn = String(Program.lookupFuncName(servLoadInfo.sfn));

            if (isIPCPartition(partLoadInfo.flags)) {
                instView.signal = servLoadInfo.signal;
            }

            view.push(instView);
        }
    }

    return (view);
}

/*
 * ======== viewConnections ========
 */
/* eslint-disable-next-line no-unused-vars */
function viewConnections() {
    var view = new Array();
    var partitions = getPartitions();

    for (var i = 0; i < partitions.length; i++) {
        var connAddr = partitions[i].p_handles;

        /* Tranverse the connection link list */
        while (connAddr != 0) {
            var instView = new Connection();
            var conn = Program.fetchFromAddr(connAddr, "connection_t");
            var clientPart = Program.fetchFromAddr(conn.p_client, "partition_t");
            var clientPartLoadInfo = Program.fetchFromAddr(clientPart.p_ldinf, "partition_load_info_t");
            var service = Program.fetchFromAddr(conn.service, "service_t");
            var servLoadInfo = Program.fetchFromAddr(service.p_ldinf, "service_load_info_t");

            instView.status = decodeConnectionStatus(conn.status);
            instView.callerPID = clientPartLoadInfo.pid;
            instView.service = toHexString(servLoadInfo.sid);

            instView.msgType = decodeMsgType(conn.msg.type);
            instView.msgHandle = toHexString(conn.msg.handle);
            instView.msgClientID = conn.msg.client_id;

            for (var j = 0; j < PSA_MAX_IOVEC; j++) {
                if (conn.msg.in_size[j] == 0) {
                    instView.numInVecs = j;
                    break;
                }
            }

            for (var k = 0; k < PSA_MAX_IOVEC; k++) {
                if (conn.msg.out_size[k] == 0) {
                    instView.numOutVecs = k;
                    break;
                }
            }

            view.push(instView);

            connAddr = conn.p_handles;
        }
    }

    if (view.length == 0) {
        /* No connections */
        var instView2 = new Connection();
        instView2.status = "No connections. Set a breakpoint inside backend_messaging() to view connections.";
        view.push(instView2);
    }

    return (view);
}

/*
 * ======== viewAssets ========
 */
/* eslint-disable-next-line no-unused-vars */
function viewAssets() {
    var view = new Array();
    var partitions = getPartitions();
    var partLoadInfoType = Program.lookupType("partition_load_info_t");
    var servLoadInfoType = Program.lookupType("service_load_info_t");
    var assetDescType = Program.lookupType("asset_desc_t");

    for (var i = 0; i < partitions.length; i++) {
        var partLoadInfo = Program.fetchFromAddr(partitions[i].p_ldinf, "partition_load_info_t");

        for (var j = 0; j < partLoadInfo.nassets; j++) {
            var instView = new Asset();

            /* Get the next asset descriptor for the partition */
            var assetDescAddr = partitions[i].p_ldinf + partLoadInfoType.size +
                PARTITION_EXT_INFO_LENGTH + (partLoadInfo.ndeps * PARTITION_DEP_SIZE) +
                (partLoadInfoType.nservices * servLoadInfoType.size) +
                (j * assetDescType.size);
            var assetDesc = Program.fetchFromAddr(assetDescAddr, "asset_desc_t");

            instView.pid = partLoadInfo.pid;

            if (isNamedMMIO(assetDesc.attr)) {
                var platData = Program.fetchFromAddr(assetDesc.dev.dev_ref, "platform_data_t");
                instView.memStart = toPaddedHexString(platData.periph_start, 8);
                instView.memLimit = toPaddedHexString(platData.periph_limit, 8);
                instView.ppcBank = platData.periph_ppc_bank;
                instView.ppcMask = toHexString(platData.periph_ppc_mask);
            }
            else {
                instView.memStart = toPaddedHexString(assetDesc.mem.start, 8);
                instView.memLimit = toPaddedHexString(assetDesc.mem.limit, 8);
            }

            view.push(instView);
        }
    }

    return (view);
}

/*
 * ======== addMsgFromBuf ========
 */
function addMsgFromBuf(view, outbuf, index, max) {
    var str = "";

    for (var i = index; i < max; i++) {
        var ch = outbuf[i];

        /* Continue if CR character (0x0d) encountered */
        if (ch == 0x0d) continue;

        /* Break if LF character (0x0a) encountered */
        if (ch == 0x0a) break;

        str += String.fromCharCode(ch);
    }

    var log = new Log();
    log.msg = str;
    view.push(log);

    /* Return the number of characters processed */
    return (i - index);
}

/*
 * ======== viewSLog ========
 */
/* eslint-disable-next-line no-unused-vars */
function viewSLog() {
    var view = new Array();

    try {
        var uartOutputBufAddr = Program.lookupSymbolValue("uartOutputBuf_s");
        var uartOutputBufIndex = Program.fetchVariable("uartOutputBufIndex_s");
    }
    catch (e) {
        var log = new Log();
        log.msg = String(e);
        log.msg += " Try accessing Secure logs from Secure image ROV";
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
/* eslint-disable-next-line no-unused-vars */
function viewNSLog() {
    var view = new Array();

    try {
        var uartOutputBufAddr = Program.lookupSymbolValue("uartOutputBuf_ns");
        var uartOutputBufIndex = Program.fetchVariable("uartOutputBufIndex_ns");
    }
    catch (e) {
        var log = new Log();
        log.msg = String(e);
        log.msg += " Try accessing Non-Secure logs from Non-Secure image ROV";
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
