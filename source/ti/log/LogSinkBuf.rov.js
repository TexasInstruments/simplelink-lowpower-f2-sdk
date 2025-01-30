/*
 * Copyright (c) 2022-2024, Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== LoggerBuf.rov.js ========
 */

/* global xdc */
/* global java */
/* global utils */
var Program = xdc.module('xdc.rov.Program');
var Monitor = xdc.module('xdc.rov.runtime.Monitor');

/* eslint-disable-next-line no-unused-vars */
var moduleName = "ti.loggers.utils::LogSinkBuf";
var rovMetaDataFile = "ti_utils_runtime_rov.js.xs";

const LOG_EVT_FORMAT_STR_OFFSET = 5;
const LOG_EVT_MODULE_OFFSET = 4;
const LOG_EVT_OFFSET = 3;
const NUM_BYTES_PER_WORD = 4;

const LoggerBuf_EVENT = 0;
const LoggerBuf_PRINTF = 1;
const LoggerBuf_BUFFER_START = 2;
const LoggerBuf_BUFFER_CONTINUED = 3;
const LoggerBuf_WordsPerRecord = 5;

/* eslint-disable-next-line no-unused-vars */
var viewMap = [
    {
        name: "Instance",
        fxn: "viewInitInstance",
        structName: "InstanceView",
        viewType: "Table"
    },
    {
        name: "Records",
        fxn: "viewInitRecord",
        structName: "RecordView",
        viewType: "Pages"
    }
];

function InstanceView()
{
    this.Address = "0x00000000";
    this.Serial = 0;
    this.NumEntries = 0;
    this.Type = "";
}

/* eslint-disable-next-line no-unused-vars */
function viewInitInstance()
{
    /* compute name of meta-data file */
    var cfgPathname = metaDataFilename();

    if (cfgPathname == null) {
        return ([]);
    }

    /* load ROV generated configuration file */
    var cap = xdc.loadCapsule(cfgPathname);
    var icount = cap.LogSinkBuf_settings.count;
    var table = new Array();
    var view = new InstanceView();

    var typespecInst = Program.lookupType("LogSinkBuf_Instance");

    for (var i = 0; i < icount; i++) {
        view = new InstanceView();
        var iaddr =
            Program.lookupSymbolValue("LogSinkBuf_CONFIG_ti_log_LogSinkBuf_0_config");
        if (iaddr < 0) continue;
        iaddr += (i * typespecInst.size);
        var inst = Program.fetchFromAddr(iaddr, "LogSinkBuf_Instance");
        view.Address = "0x" + iaddr.toString(16);
        view.Serial = inst.serial;
        view.NumEntries = inst.numEntries;
        view.Type = (inst.bufType == 1 ? "Linear" : "Circular");
        table.push(view);
    }

    /* if no instances on target, render and headings only */
    if (table.length == 0) {
        view = new InstanceView();
        view.Address = "";
        view.Serial = "";
        view.NumEntries = "";
        view.Type = "";
        table.push(view);
    }

    return (table);
}

function RecordView()
{
    this.Serial = 0;
    this.Timestamp = 0.0;
    this.Type = "";
    this.SubType = "N/A";
    this.Module = "";
    this.Message = "";
    this.Data = "";
}


/* Helper function to extract unsigned bytes from a uint32 */
function toBytesUInt32 (num, numBytes) {
    var arr = new Array();

    if (numBytes >= 1 ) {
        arr.push(num & 0x000000ff);
    }
    if(numBytes >= 2 ) {
        arr.push((num & 0x0000ff00) >>> 8);
    }
    if(numBytes >= 3) {
        arr.push((num & 0x00ff0000) >>> 16);
    }
    if(numBytes >= 4) {
        arr.push((num & 0xff000000) >>> 24);
    }
    return arr;
}

/* eslint-disable-next-line no-unused-vars */
function viewInitRecord()
{
    /* compute name of meta-data file */
    var cfgPathname = metaDataFilename();

    if (cfgPathname == null) {
        return ([]);
    }

    /* load ROV generated configuration file */
    var cap = xdc.loadCapsule(cfgPathname);

    /* collect metrics needed for data processing */
    var icount = cap.LogSinkBuf_settings.count;
    var tsInst = Program.lookupType("LogSinkBuf_Instance");
    var recSize = Program.lookupType("LogSinkBuf_Rec").size;

    /* ROV page view object */
    var pages = new Array();
    /* table of records per page */
    var table = new Array();

    function addEventToTable(rec, fmt, mdata, recView)
    {
        var args = new Array();
        args.push(rec.data[1]);
        args.push(rec.data[2]);
        args.push(rec.data[3]);
        args.push(rec.data[4]);

        recView.Type = "EVENT";
        recView.SubType = mdata[LOG_EVT_OFFSET];
        recView.Message = rsprintf(fmt, args);
        recView.Module = mdata[LOG_EVT_MODULE_OFFSET];
        recView.Data = ("").concat("Args: ", args.toString());
        table.push(recView);
    }

    function addPrintfToTable(rec, fmt, mdata, recView)
    {
        var args = new Array();
        args.push(rec.data[1]);
        args.push(rec.data[2]);
        args.push(rec.data[3]);
        args.push(rec.data[4]);

        recView.Type = "FORMATTED_TEXT";
        recView.SubType = "N/A";
        recView.Serial = rec.serial;
        recView.Message = rsprintf(fmt, args);
        recView.Module = mdata[LOG_EVT_MODULE_OFFSET];

        recView.Data = ("").concat("Args: ", args.toString());
        table.push(recView);
    }

    function addBufToTable(rec, fmt, mdata, recView)
    {
        /* This function is stateful and assumes that buf records are
            * placed in the buffer contiguously and are not interleaved.
            * This is an area that has been identified for improvement.
            */
        if( typeof addBufToTable.numWords == 'undefined' ||
            typeof addBufToTable.remainder == 'undefined') {
            addBufToTable.numWords = 0;
            addBufToTable.remainder = 0;
        }

        if(rec.type == LoggerBuf_BUFFER_START) {
            /* The first record is purely informational, use it to set state */
            recView.Message = fmt;
            recView.Module = mdata[LOG_EVT_MODULE_OFFSET];
            recView.Type = "BUFFER_START";
            recView.SubType = "N/A";

            var size = rec.data[1];
            recView.Data = ("").concat("size: ", size);
            addBufToTable.numWords = Math.floor(size/NUM_BYTES_PER_WORD);
            addBufToTable.remainder = size % NUM_BYTES_PER_WORD;
        }
        else if (rec.type == LoggerBuf_BUFFER_CONTINUED) {
            recView.Type = "BUFFER_CONTINUED";
            recView.SubType = "N/A";

            /* Protect against data loss from wraparound in circular buf */
            if ((addBufToTable.numWords == -1) && (addBufToTable.remainder == -1)) {
                recView.Message = "Found continue without start the rest " +
                                    "of the buffer will not be parsed";
                return;
            }

            /* The next record stores the user data */
            var outputArray = new Array();
            Monitor.println(String(addBufToTable.numWords));
            Monitor.println(String(addBufToTable.remainder));

            /* Target memory is arranged as uint32_t array, reformat this
                * to an array of bytes, ignoring any padding entries
                */
            for (var i = 0;
                    (i < LoggerBuf_WordsPerRecord)
                    && (addBufToTable.numWords >= 0);
                    i++, addBufToTable.numWords--) {

                if(addBufToTable.numWords == 0) {
                    outputArray.push(toBytesUInt32(rec.data[i], addBufToTable.remainder));
                    /* Reset the control variables */
                    addBufToTable.numWords = -1;
                    addBufToTable.remainder = -1;
                }
                else {
                    outputArray.push(toBytesUInt32(rec.data[i], NUM_BYTES_PER_WORD));
                }
            }
            recView.Data = outputArray.toString();
        }
        table.push(recView);
    }

    /* add a new entry to table for given log record */
    function makeRecord(addr)
    {

        /* Read a record from the buffer */
        var rec = Program.fetchFromAddr(addr, "LogSinkBuf_Rec");
        /* deter what type of log statement the record originated from */
        var recType = Number(rec.type);

        var dataAddr;
        var ldata;
        var mdata;

        /* Events are also transported with printf - if the type is PRINTF, check if the magic string indicates an event */
        if(recType == LoggerBuf_PRINTF)
        {
            dataAddr = Number(rec.data[0]);
            ldata = String(Program.ofReader.findString(dataAddr, ".log_data"));
            mdata = ldata.split("\x1e");
            if(mdata[0] == "LOG_OPCODE_EVENT")
            {
                recType = LoggerBuf_EVENT;
            }
        }

        /* The continued data structure doesn't have a format string as this
            * place is used for storage of user data
            */
        if(recType != LoggerBuf_BUFFER_CONTINUED) {
            dataAddr = Number(rec.data[0]);
            /* This function returns a Java String object, must convert
                * it into a JavaScript String object. XDCTOOLS-415
                */
            ldata = String(Program.ofReader.findString(dataAddr, ".log_data"));
            mdata = ldata.split("\x1e");

            /* remove leading and trailing double-quote (") in format string */
            var fmt = mdata[LOG_EVT_FORMAT_STR_OFFSET].replace(/^"/, "").replace(/"$/, "");

            /* If the record is an event, get the formatted string from the event symbol */
            if (recType == LoggerBuf_EVENT) {
                var evtAddr = Program.lookupSymbolValue("LogSymbol_" + fmt);
                ldata = String(Program.ofReader.findString(evtAddr, ".log_data"));
                mdata = ldata.split("\x1e");
                fmt = mdata[LOG_EVT_FORMAT_STR_OFFSET].replace(/^"/, "").replace(/"$/, "");
            }
        }

        /* Create a new record for the log statement */
        var recView = new RecordView();

        /* Capture the timestamp and sequence number */
        recView.Serial = rec.serial;
        var iaddr = Program.lookupSymbolValue("TimestampP_nativeFormat32");
        var timeStampFormat = Program.fetchFromAddr(iaddr, "TimestampP_Format");

        /* The variable parser is not able to interpret bitfields. Recreate the struct manually below */
        var fracBytes = timeStampFormat.format.fracBytes & 0xF;
        var intBytes = (timeStampFormat.format.fracBytes >> 4) & 0xF;
        var exponent = (timeStampFormat.format.fracBytes >> 8) & 0xFF;
        var multiplier = (timeStampFormat.format.fracBytes >> 16) & 0xFFFF;

        /* Recreate timestamp as per the TimestampP_Format format */
        var timeStampFractional = 0;
        var timestampInteger = 0;

        if (fracBytes == 4) {
            timeStampFractional = rec.timestampLow;
        }
        else {
            timeStampFractional = (rec.timestampLow & ((1 << (fracBytes*8)) - 1)) / (1 << (fracBytes*8));
        }

        if (intBytes == 4) {
            timestampInteger = rec.timestampLow;
        }
        else {
            timestampInteger = (rec.timestampLow >> (fracBytes*8)) & ((1 << (intBytes*8)) - 1);
        }

        recView.Timestamp = multiplier * (timeStampFractional + timestampInteger) * Math.pow(10, -exponent);

        /* Set fixed number of decimals on timestamp */
        recView.Timestamp = recView.Timestamp.toFixed(8);

        /* Parse the record based on log type */
        if (recType == LoggerBuf_EVENT) {
            addEventToTable(rec, fmt, mdata, recView);
        }
        else if (recType == LoggerBuf_PRINTF) {
            addPrintfToTable(rec, fmt, mdata, recView);
        }
        else if (recType == LoggerBuf_BUFFER_START ||
                    recType == LoggerBuf_BUFFER_CONTINUED) {
            addBufToTable(rec, fmt, mdata, recView);
        }
    }

    for (let idx = 0; idx < icount; idx++) {

        table = new Array();
        /* read requested instance from target memory */
        var iaddr = Program.lookupSymbolValue(
            "LogSinkBuf_CONFIG_ti_log_LogSinkBuf_0_config") + (idx * tsInst.size);
        var inst = Program.fetchFromAddr(iaddr, "LogSinkBuf_Instance");

        /* create page for logger instance */
        pages.push({ label: "0x" + iaddr.toString(16), elements: table });

        /* check for empty log buffer */
        if (inst.serial == 0) {
            continue;
        }

        var addr;

        if (inst.bufType == 1) { /* linear buffer  */
            for (var i = 0; i < inst.serial; i++) {
                addr = inst.buffer + (i * recSize);
                makeRecord(addr);
            }
        }
        else { /* circular buffer */
            var count;

            if (inst.serial < inst.numEntries) {
                addr = inst.buffer;
                count = inst.serial;
            }
            else {
                addr = (inst.curEntry == inst.endEntry ? inst.buffer :
                    inst.curEntry + recSize);
                count = inst.numEntries - 1;
            }

            for (var r = 0; r < count; r++) {
                makeRecord(addr);
                addr = (addr == inst.endEntry ? inst.buffer : addr + recSize);
            }
        }
    }

    return (pages);
}
/*
 *  ======== metaDataFilename ========
 *  Compute name of meta-data file for this component
 */
function metaDataFilename()
{
    var cfgPathname = genDir() + "/" + rovMetaDataFile;

    var exGrCfg = new java.io.File(cfgPathname);

    if (!exGrCfg.exists()) {
        Monitor.println(
            "Error: ROV file not found (" + rovMetaDataFile + ")");
        return (null);
    }

    return (cfgPathname);
}

/*
 *  ======== genDir ========
 *  Find the code generation directory used by SysConfig
 *
 *  XDCtools exports utils as global object from the file xdc/utils.js.
 */
function genDir()
{

    /* look for the file syscfg_c.rov.xs in the hash of all loaded files */
    var dir = null;
    var keys = Object.keys(utils.loadedFiles);

    for (var i = 0; i < keys.length; i++) {
        var tokens = keys[i].match(/(.*)[\\/]syscfg_c\.rov\.xs$/);
        if (tokens) {
            dir = tokens[1].replace(/\\/g, "/");
            break;
        }
    }

    return (dir);
}

/*
 *  ======== rsprintf ========
 *  Restricted string print format
 *
 *  Scan format string and replace control characters with supplied
 *  argument. Only a limited set of control characters are supported:
 *
 *      %s - string
 *      %c - character
 *      %d - decimal
 *      %f - float
 *      %x - hexadecimal
 *
 *  Parameters
 *      fmt - format string
 *      args - string array of arguments
 */
function rsprintf(fmt, args)
{
    var regex = /%([scdfx%])/g;
    var i = -1;

    function reparg(match, p)
    {
        if (match == '%%') return ('%');
        if (args[++i] === undefined) return (undefined);
        var val;
        switch (p) {
            case 's': val = String(Program.ofReader.findString(args[i])); break;
            case 'c': val = String.fromCharCode(args[i]); break;
            case 'd': val = parseInt(args[i]); break;
            case 'f': val = parseFloat(args[i]); break;
            case 'x': val = '0x' + parseInt(args[i]).toString(16); break;
        }
       return (val);
    }

    return (fmt.replace(regex, reparg));
}
