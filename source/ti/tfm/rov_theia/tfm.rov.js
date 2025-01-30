/*
 * Copyright (c) 2020-2024, Texas Instruments Incorporated
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

const PARTITION_EXT_INFO_LENGTH = 8;  /* Partition stack and heap addr members occupy 8-bytes */
const PARTITION_DEP_SIZE = 4;         /* Partition dependencies are defined as uint32_t */
const PSA_MAX_IOVEC = 4;              /* Max number of input/output vectors */
const PARTITION_FLAGS_PRIORITY_MASK = 0xFF;

class SP {
    constructor() {
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
}

class Service  {
    constructor() {
        this.pid = null;
        this.sid = null;
        this.name = null;
        this.type = null;
        this.version = null;
        this.sfn = null;
        this.signal = null;
    }
}

class Connection  {
    constructor() {
        this.status = null;
        this.callerPID = null;
        this.service = null;
        this.msgType = null;
        this.msgHandle = null;
        this.msgClientID = null;
        this.numInVecs = null;
        this.numOutVecs = null;
    }
}

class Asset {
    constructor() {
        this.pid = null;
        this.memStart = null;
        this.memLimit = null;
        this.ppcBank = null;
        this.ppcMask = null;
        this.attr = null;
    }
}
class Log  {
    constructor() {
        this.msg = null;
    }
}
class TFM {
    constructor(ctx) {
        this.ctx = ctx;
        this.viewMap = [
            { name: 'Secure Partitions', fxn: this.viewSPs.bind(this), structName: SP },
            { name: 'Services', fxn: this.viewServices.bind(this), structName: Service },
            { name: 'Connections', fxn: this.viewConnections.bind(this), structName: Connection },
            { name: 'Assets', fxn: this.viewAssets.bind(this), structName: Asset },
            { name: 'Secure Logs', fxn: this.viewSLog.bind(this), structName: Log },
            { name: 'Non-Secure Logs', fxn: this.viewNSLog.bind(this), structName: Log }
        ];
        this.Program = this.ctx.getProgram();
    }
    getModuleName() {
        return 'TFM';
    }

    /*
     * ======== isSecureState ========
     * Returns true if processor is in secure mode
     */
    async isSecureState() {
        const DSCSR = await this.ProgramfetchFromAddr(0xe000ee08, "uint32_t", 1);

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
    toHexString(num) {
        return ("0x" + Number(num).toString(16));
    }

    /*
     * ======== toPaddedHexString ========
     * converts a number to a hex string with "0x" prefix.
     * inserts up to (len - 1) leading zeros.
     * up to a max of 7 zeros.
     */
    toPaddedHexString(number, len)
    {
        return ("0x" + ("0000000" + number.toString(16)).substr(-len));
    }

    /*
     * ======== computeStackPeak ========
     * returns the max depth the stack has reached
     */
    async computeStackPeak(stackLimit, stackSize) {
        const stack = await this.Program.fetchFromAddr(stackLimit, "uint32_t", (stackSize / 4));

        for (let i = 0; i < (stackSize / 4); i++) {
            if (stack[i] != 0) {
                return (this.toHexString(stackSize - (i * 4)));
            }
        }

        return ("Unknown");
    }

    /*
     * ======== decodePartitionFlags ========
     * returns string of TFM-specific partition type flags.
     */
    decodePartitionFlags(flags) {
        let type = null;

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
    decodeBoundaryFlags(flags) {
        let type = "";

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
    isIPCPartition(partition_flags) {
        return ((partition_flags & 0x200) ? true : false);
    }

    /*
     * returns string of TFM-specific service type flags.
     */
    decodeServiceFlags(flags) {
        let type = "";

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
    decodeIPCState(threadState) {
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
    decodeSFNState(state) {
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
    decodePriority(flags) {
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
                return this.toHexString(flags & PARTITION_FLAGS_PRIORITY_MASK);
        }
    }

    /*
     * ======== decodeConnectionStatus ========
     * returns string of TFM-specific connection status.
     */
    decodeConnectionStatus(status) {
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
    decodeMsgType(type) {
        switch (type) {
            /* The following match "PSA_IPC_" defines in TF-M code */
            case -2:
                return "DISCONNECT";
            case -1:
                return "CONNECT";
            default:
                /* SP-specific msg type */
                return "CALL: " + this.toHexString(type);
        }
    }

    /*
     * ======== decodeAssetAttr ========
     * returns string of TFM-specific asset attribute flags.
     */
    decodeAssetAttr(flags) {
        let type = "";

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
    isNamedMMIO(asset_flags) {
        return ((asset_flags & 0x8) ? true : false);
    }

    /*
     * ======== charArrayToString ========
     * convert a C array of chars into a string
     */
    charArrayToString(charArray){
        let s = "";
        let i = 0;
        while (charArray[i] && (i < charArray.length)) {
            s += String.fromCharCode(charArray[i]);
            i++;
        }
        return s;
    }

    /*
     * ======== getPartitions ========
     */
    async getPartitions() {
        let partitions = [];
        const head = await this.Program.fetchVariable("partition_listhead");
        let sp = head;

        /* Traverse link list of partitions and fetch partition structs */
        while (sp.next != 0) {
            sp = await this.Program.fetchFromAddr(sp.next, "partition_t");
            partitions.push(sp);
        }

        return partitions;
    }

    /*
     * ======== viewSPs ========
     */
    async viewSPs() {
        let view = new Array();
        const partitions = this.getPartitions();

        const currThrdAddr = await this.Program.fetchVariable("p_curr_thrd");
        const currThrd = await this.Program.fetchFromAddr(currThrdAddr, "thread_t");

        for (let i = 0; i < partitions.length; i++) {
            let instView = new SP();
            const loadInfo = await this.Program.fetchFromAddr(partitions[i].p_ldinf, "partition_load_info_t");

            instView.pid = loadInfo.pid;
            instView.function = String(await this.Program.lookupFuncName(loadInfo.entry));
            instView.type = this.decodePartitionFlags(loadInfo.flags);
            instView.priority = this.decodePriority(loadInfo.flags);
            instView.sigAllowed = this.toHexString(partitions[i].signals_allowed);
            instView.sigAsserted = this.toHexString(partitions[i].signals_asserted);
            instView.stackSize = this.toHexString(loadInfo.stack_size);

            if (this.isIPCPartition(loadInfo.flags)) {
                /* IPC model */
                instView.stackBase = this.toHexString(partitions[i].ctx_ctrl.sp_base);
                instView.stackPeak = this.computeStackPeak(partitions[i].ctx_ctrl.sp_limit, loadInfo.stack_size);
                instView.stackPtr = this.toHexString(partitions[i].ctx_ctrl.sp);
                instView.state = this.decodeIPCState(partitions[i].thrd.state);

                if (instView.stackPeak > loadInfo.stack_size) {
                    await this.Program.displayError(instView, "stackPeak", "Stack overflow!");
                }

                if (Number(partitions[i].thrd.p_context_ctrl) == currThrd.p_context_ctrl) {
                    /* Set current thread indicator */
                    instView.function = instView.function + "*";
                }
            }
            else {
                /* SFN model */
                instView.state = this.decodeSFNState(partitions[i].state);
            }

            instView.boundary = this.decodeBoundaryFlags(partitions[i].boundary);
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
    async viewServices() {
        let view = new Array();
        const partitions = this.getPartitions();
        const partLoadInfoType = await this.Program.lookupType("partition_load_info_t");
        const servLoadInfoType = await this.Program.lookupType("service_load_info_t");

        for (let i = 0; i < partitions.length; i++) {
            const partLoadInfo = await this.Program.fetchFromAddr(partitions[i].p_ldinf, "partition_load_info_t");

            for (let j = 0; j < partLoadInfo.nservices; j++) {
                let instView = new Service();

                /* Get the service info for the partition */
                const servLoadInfoAddr = partitions[i].p_ldinf + partLoadInfoType.size +
                    PARTITION_EXT_INFO_LENGTH + (partLoadInfo.ndeps * PARTITION_DEP_SIZE) +
                    (j * servLoadInfoType.size);
                const servLoadInfo = await this.Program.fetchFromAddr(servLoadInfoAddr, "service_load_info_t");

                instView.pid = partLoadInfo.pid;
                instView.sid = this.toHexString(servLoadInfo.sid);

                const nameAddr = servLoadInfo.name_strid;
                const name = await this.Program.fetchFromAddr(nameAddr, "char", 25);
                instView.name = this.charArrayToString(name);

                instView.type = this.decodeServiceFlags(servLoadInfo.flags);
                instView.version = servLoadInfo.version;
                instView.sfn = String(await this.Program.lookupFuncName(servLoadInfo.sfn));

                if (this.isIPCPartition(partLoadInfo.flags)) {
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
    async viewConnections() {
        let view = new Array();
        const partitions = this.getPartitions();

        for (let i = 0; i < partitions.length; i++) {
            let connAddr = partitions[i].p_handles;

            /* Tranverse the connection link list */
            while (connAddr != 0) {
                let instView = new Connection();
                const conn = await this.Program.fetchFromAddr(connAddr, "connection_t");
                const clientPart = await this.Program.fetchFromAddr(conn.p_client, "partition_t");
                const clientPartLoadInfo = await this.Program.fetchFromAddr(clientPart.p_ldinf, "partition_load_info_t");
                const service = await this.Program.fetchFromAddr(conn.service, "service_t");
                const servLoadInfo = await this.Program.fetchFromAddr(service.p_ldinf, "service_load_info_t");

                instView.status = this.decodeConnectionStatus(conn.status);
                instView.callerPID = clientPartLoadInfo.pid;
                instView.service = this.toHexString(servLoadInfo.sid);

                instView.msgType = this.decodeMsgType(conn.msg.type);
                instView.msgHandle = this.toHexString(conn.msg.handle);
                instView.msgClientID = conn.msg.client_id;

                for (let j = 0; j < PSA_MAX_IOVEC; j++) {
                    if (conn.msg.in_size[j] == 0) {
                        instView.numInVecs = j;
                        break;
                    }
                }

                for (let k = 0; k < PSA_MAX_IOVEC; k++) {
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
            let instView = new Connection();
            instView.status = "No connections. Set a breakpoint inside backend_messaging() to view connections.";
            view.push(instView);
        }

        return (view);
    }

    /*
     * ======== viewAssets ========
     */
    async viewAssets() {
        let view = new Array();
        const partitions = this.getPartitions();
        const partLoadInfoType = await this.Program.lookupType("partition_load_info_t");
        const servLoadInfoType = await this.Program.lookupType("service_load_info_t");
        const assetDescType = await this.Program.lookupType("asset_desc_t");

        for (let i = 0; i < partitions.length; i++) {
            const partLoadInfo = await this.Program.fetchFromAddr(partitions[i].p_ldinf, "partition_load_info_t");

            for (let j = 0; j < partLoadInfo.nassets; j++) {
                let instView = new Asset();

                /* Get the next asset descriptor for the partition */
                const assetDescAddr = partitions[i].p_ldinf + partLoadInfoType.size +
                    PARTITION_EXT_INFO_LENGTH + (partLoadInfo.ndeps * PARTITION_DEP_SIZE) +
                    (partLoadInfoType.nservices * servLoadInfoType.size) +
                    (j * assetDescType.size);
                const assetDesc = await this.Program.fetchFromAddr(assetDescAddr, "asset_desc_t");

                instView.pid = partLoadInfo.pid;

                if (this.isNamedMMIO(assetDesc.attr)) {
                    const platData = await this.Program.fetchFromAddr(assetDesc.dev.dev_ref, "platform_data_t");
                    instView.memStart = this.toPaddedHexString(platData.periph_start, 8);
                    instView.memLimit = this.toPaddedHexString(platData.periph_limit, 8);
                    instView.ppcBank = platData.periph_ppc_bank;
                    instView.ppcMask = this.toHexString(platData.periph_ppc_mask);
                }
                else {
                    instView.memStart = this.toPaddedHexString(assetDesc.mem.start, 8);
                    instView.memLimit = this.toPaddedHexString(assetDesc.mem.limit, 8);
                }

                view.push(instView);
            }
        }

        return (view);
    }

    /*
     * ======== addMsgFromBuf ========
     */
    addMsgFromBuf(view, outbuf, index, max) {
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
    async viewSLog() {
        let view = new Array();

        try {
            const uartOutputBufAddr = await this.Program.lookupSymbolValue("uartOutputBuf_s");
            const uartOutputBufIndex = await this.Program.fetchVariable("uartOutputBufIndex_s");

            let outbuf = await this.Program.fetchFromAddr(uartOutputBufAddr, "char", uartOutputBufIndex);
            let index = 0;

            while (index < uartOutputBufIndex) {
                index += this.addMsgFromBuf(view, outbuf, index, uartOutputBufIndex);
            }

            return (view);
        }
        catch (e) {
            let log = new Log();
            log.msg = String(e);
            log.msg += " Try accessing Secure logs from Secure image ROV";

            view.push(log);
            return (view);
        }
    }

    /*
     * ======== viewNSLog ========
     */
    async viewNSLog() {
        let view = new Array();

        try {
            const uartOutputBufAddr = await this.Program.lookupSymbolValue("uartOutputBuf_ns");
            const uartOutputBufIndex = await this.Program.fetchVariable("uartOutputBufIndex_ns");

            let outbuf = await this.Program.fetchFromAddr(uartOutputBufAddr, "char", uartOutputBufIndex);
            let index = 0;

            while (index < uartOutputBufIndex) {
                index += this.addMsgFromBuf(view, outbuf, index, uartOutputBufIndex);
            }

            return (view);
        }
        catch (e) {
            let log = new Log();
            log.msg = String(e);
            log.msg += " Try accessing Non-Secure logs from Non-Secure image ROV";

            view.push(log);
            return (view);
        }
    }
}
exports.classCtor = TFM;
