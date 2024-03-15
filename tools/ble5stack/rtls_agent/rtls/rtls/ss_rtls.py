#
#  Copyright (c) 2018-2019, Texas Instruments Incorporated
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#  *  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#
#  *  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
#  *  Neither the name of Texas Instruments Incorporated nor the names of
#     its contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
#  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
#  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
#  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

import enum

from construct import Struct, Enum, Int8ul, Int8sl, Int32ul, Int32sl, Int16ul, Int16sl, Byte, this, Float64l, FlagsEnum, \
    GreedyRange, CString

from unpi.npirequest_mixins import AsyncReq, FromNwp, FromAp, SyncRsp, SyncReq
from unpi.serialnode import builder_class
from unpi.unpiparser import NpiSubSystem, NpiRequest, NpiSubSystems, NiceBytes, ReverseBytes


class Commands(enum.IntEnum):
    RTLS_CMD_IDENTIFY = 0x00
    RTLS_CMD_CONN_PARAMS = 0x02
    RTLS_CMD_CONNECT = 0x03
    RTLS_CMD_SCAN = 0x04
    RTLS_CMD_SCAN_STOP = 0x05
    RTLS_CMD_RESERVED0 = 0x06
    RTLS_CMD_RESERVED1 = 0x07
    RTLS_CMD_RESERVED2 = 0x08
    RTLS_CMD_CREATE_SYNC = 0X09
    RTLS_CMD_CREATE_SYNC_CANCEL = 0x0A
    RTLS_CMD_TERMINATE_SYNC = 0x0B
    RTLS_CMD_PERIODIC_RECEIVE_ENABLE = 0x0C
    RTLS_CMD_ADD_DEVICE_ADV_LIST = 0x0D
    RTLS_CMD_REMOVE_DEVICE_ADV_LIST = 0x0E
    RTLS_CMD_READ_ADV_LIST_SIZE = 0x0F
    RTLS_CMD_CLEAR_ADV_LIST = 0x10
    RTLS_CMD_RESERVED3 = 0x11
    RTLS_CMD_RESERVED31 = 0x12
    RTLS_CMD_AOA_SET_PARAMS = 0x13
    RTLS_CMD_AOA_ENABLE = 0x14
    RTLS_CMD_CL_AOA_ENABLE = 0x15
    RTLS_CMD_RESET_DEVICE = 0x20
    RTLS_CMD_TERMINATE_LINK = 0x22
    RTLS_CMD_RESERVED4 = 0x23
    RTLS_CMD_AOA_RESULT_RAW = 0x24
    RTLS_CMD_RESERVED5 = 0x25
    RTLS_CMD_RESERVED6 = 0x26
    RTLS_CMD_CONN_INFO = 0x27
    RTLS_CMD_SET_RTLS_PARAM = 0x28
    RTLS_CMD_GET_RTLS_PARAM = 0x29
    RTLS_CMD_RESERVED7 = 0x30
    RTLS_CMD_RESERVED8 = 0x31
    RTLS_CMD_GET_ACTIVE_CONN_INFO = 0x32
    RTLS_CMD_CL_AOA_RESULT_RAW = 0x33
    RTLS_CMD_RESERVED9 = 0x34
    RTLS_CMD_RESERVED10 = 0x35
    RTLS_CMD_HEAP_SIZE = 0x36

    RTLS_EVT_ASSERT = 0x80
    RTLS_EVT_ERROR = 0x81
    RTLS_EVT_DEBUG = 0x82
    RTLS_EVT_CONN_INFO = 0x83
    RTLS_EVT_SYNC_EST = 0x84
    RTLS_EVT_SYNC_LOST = 0x85
    RTLS_EVT_PERIODIC_ADV_RPT = 0x86
    RTLS_EVT_TERMINATE_SYNC = 0x87
    RTLS_EVT_CL_AOA_ENABLE = 0x88


# Rtls param types for RTLS_CMD_SET_RTLS_PARAM command
class RtlsParamType(enum.IntFlag):
    RTLS_PARAM_CONNECTION_INTERVAL = 1
    RTLS_PARAM_2 = 2
    RTLS_PARAM_3 = 3


class Capabilities(enum.IntFlag):
    CM = 1
    AOA_TX = 2
    AOA_RX = 4
    RESERVED1 = 8
    RTLS_CAP_CL_AOA = 16
    RESERVED2 = 32
    RTLS_RESPONDER = 64
    RTLS_COORDINATOR = 128
    RTLS_PASSIVE = 256
    RTLS_CONNECTION_MONITOR = 512


RtlsStatus = Enum(Int8ul,
                  RTLS_SUCCESS=0,
                  RTLS_FAIL=1,
                  RTLS_LINK_ESTAB_FAIL=2,
                  RTLS_LINK_TERMINATED=3,
                  RTLS_OUT_OF_MEMORY=4,
                  RTLS_CONFIG_NOT_SUPPORTED=5,
                  RTLS_ILLEGAL_CMD=6,
                  RTLS_SYNC_CANCELED_BY_HOST=68,
                  RTLS_SYNC_FAILED_TO_BE_EST=62
                  )

AssertCause = Enum(Int8ul,
                   HAL_ASSERT_CAUSE_FALSE=0,
                   HAL_ASSERT_CAUSE_TRUE=1,
                   HAL_ASSERT_CAUSE_INTERNAL_ERROR=2,
                   HAL_ASSERT_CAUSE_HW_ERROR=3,
                   HAL_ASSERT_CAUSE_OUT_OF_MEMORY=4,
                   HAL_ASSERT_CAUSE_ICALL_ABORT=5,
                   HAL_ASSERT_CAUSE_ICALL_TIMEOUT=6,
                   HAL_ASSERT_CAUSE_WRONG_API_CALL=7,
                   HAL_ASSERT_CAUSE_HARDWARE_ERROR=8,
                   HAL_ASSERT_CAUSE_RF_DRIVER_ERROR=9,
                   )


class AoaRole(enum.IntEnum):
    AOA_RESPONDER = 0
    AOA_COORDINATOR = 1
    AOA_PASSIVE = 2


class AoaResultMode(enum.IntEnum):
    AOA_MODE_RAW = 0


class DeviceFamily(enum.IntEnum):
    DeviceFamily_ID_CC13X0 = 1
    DeviceFamily_ID_CC26X0 = 2
    DeviceFamily_ID_CC26X0R2 = 3
    DeviceFamily_ID_CC13X2 = 4
    DeviceFamily_ID_CC26X2 = 5
    DeviceFamily_ID_CC3200 = 6
    DeviceFamily_ID_CC3220 = 7
    DeviceFamily_ID_CC13X1 = 8
    DeviceFamily_ID_CC26X1 = 9
    DeviceFamily_ID_CC13X2X7 = 10
    DeviceFamily_ID_CC26X2X7 = 11
    DeviceFamily_ID_CC13X1_CC26X1 = 12
    DeviceFamily_ID_CC13X2_CC26X2 = 13
    DeviceFamily_ID_CC13X2X7_CC26X2X7 = 14
    DeviceFamily_ID_CC13X4_CC26X4 = 15
    DeviceFamily_ID_CC13X4 = 16
    DeviceFamily_ID_CC26X4 = 17
    DeviceFamily_ID_CC26X3 = 18
    DeviceFamily_ID_CC23X0R5 = 19
    DeviceFamily_ID_CC23X0R2 = 20
    DeviceFamily_ID_CC27XX = 21
    DeviceFamily_ID_CC35XX = 22


# noinspection PyPep8Naming
class RTLS(NpiSubSystem):
    type = NpiSubSystems.RTLS.value

    def __init__(self, sender):
        self.sender = sender

    #
    # Responses
    #
    class IdentifyRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_IDENTIFY
        struct = Struct(
            "capabilities" / FlagsEnum(Int16ul, Capabilities),
            "revNum" / Int16ul,
            "devId" / Enum(Int8ul, DeviceFamily),
            "identifier" / NiceBytes(ReverseBytes(Byte[6])),
        )

    class ConnRsp(NpiRequest, AsyncReq, FromNwp):
        command = Commands.RTLS_CMD_CONNECT
        struct = Struct(
            "connHandle" / Int16ul,
            "status" / RtlsStatus,
        )

    class AssertRsp(NpiRequest, AsyncReq, FromNwp):
        command = Commands.RTLS_EVT_ASSERT
        struct = Struct(
            "cause" / AssertCause,
        )

    class ErrorRsp(NpiRequest, AsyncReq, FromNwp):
        command = Commands.RTLS_EVT_ERROR
        struct = Struct(
            "status" / RtlsStatus,
        )

    class DebugRsp(NpiRequest, AsyncReq, FromNwp):
        command = Commands.RTLS_EVT_DEBUG
        struct = Struct(
            "debug_value" / Int32ul,
            "debug_string" / CString("utf8"),
        )

    class ConnInfoEvtRsp(NpiRequest, AsyncReq, FromNwp):
        command = Commands.RTLS_EVT_CONN_INFO
        struct = Struct(
            "connHandle" / Int16ul,
            "rssi" / Int8sl,
            "channel" / Int8ul,
        )

    class DeviceInfoRsp(NpiRequest, AsyncReq, FromNwp):
        command = Commands.RTLS_CMD_SCAN
        struct = Struct(
            "eventType" / Int8ul,
            "addrType" / Enum(Int8ul),
            "addr" / NiceBytes(ReverseBytes(Byte[6])),
            "rssi" / Int8sl,
            "advSID" / Int8ul,
            "periodicAdvInt" / Int16ul,
            "dataLen" / Int8ul,
            "data" / NiceBytes(Byte[this.dataLen])
        )

    class ScanRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_SCAN
        struct = Struct(
            "status" / RtlsStatus,
        )

    class ResetDeviceRes(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_RESET_DEVICE
        struct = Struct(
            "status" / RtlsStatus,
        )

    class ScanStopRsp(NpiRequest, AsyncReq, FromNwp):
        command = Commands.RTLS_CMD_SCAN_STOP
        struct = Struct(
            "status" / RtlsStatus,
        )

    class ConnectRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_CONNECT
        struct = Struct(
            "status" / RtlsStatus,
        )

    class SetConnParamsRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_CONN_PARAMS
        struct = Struct(
            "status" / RtlsStatus,
        )

    class GetActiveConnInfoRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_GET_ACTIVE_CONN_INFO
        struct = Struct(
            "status" / RtlsStatus,
        )

    class ConnParamsRsp(NpiRequest, AsyncReq, FromNwp):
        command = Commands.RTLS_CMD_CONN_PARAMS
        struct = Struct(
            "connHandle" / Int16ul,
            "accessAddress" / Int32ul,
            "connRole" / Int8ul,
            "connInterval" / Int16ul,
            "hopValue" / Int8ul,
            "mSCA" / Int16ul,
            "currChan" / Int8ul,
            "chanMap" / Byte[5],
            "crcInit" / Int32ul,
            "peerAddr" / NiceBytes(ReverseBytes(Byte[6])),
        )

    class AoaStartRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_AOA_ENABLE
        struct = Struct(
            "status" / RtlsStatus,
        )

    class AoaSetParamsRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_AOA_SET_PARAMS
        struct = Struct(
            "status" / RtlsStatus,
        )

    class AoaResultRaw(NpiRequest, AsyncReq, FromNwp):
        command = Commands.RTLS_CMD_AOA_RESULT_RAW
        struct = Struct(
            "connHandle" / Int16ul,
            "rssi" / Int8sl,
            "antenna" / Int8ul,
            "channel" / Int8ul,
            "offset" / Int16ul,
            "samplesLength" / Int16ul,
            "samples" / GreedyRange(Struct(
                "i" / Int16sl,
                "q" / Int16sl,
            )),
        )

    class ClAoaResultRaw(NpiRequest, AsyncReq, FromNwp):
        command = Commands.RTLS_CMD_CL_AOA_RESULT_RAW
        struct = Struct(
            "syncHandle" / Int16ul,
            "rssi" / Int8sl,
            "antenna" / Int8ul,
            "channel" / Int8ul,
            "offset" / Int16ul,
            "samplesLength" / Int16ul,
            "samples" / GreedyRange(Struct(
                "i" / Int16sl,
                "q" / Int16sl,
            )),
        )

    class ConnInfoRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_CONN_INFO
        struct = Struct(
            "status" / RtlsStatus,
        )

    class SetRtlsParamRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_SET_RTLS_PARAM
        struct = Struct(
            "connHandle" / Int16ul,
            "rtlsParamType" / Enum(Int8ul, RtlsParamType),
            "status" / RtlsStatus,
        )

    class CreateSyncRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_CREATE_SYNC
        struct = Struct(
            "status" / RtlsStatus,
        )

    class CreateSyncCancelRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_CREATE_SYNC_CANCEL
        struct = Struct(
            "status" / RtlsStatus,
        )

    class TerminateSyncRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_TERMINATE_SYNC
        struct = Struct(
            "status" / RtlsStatus,
        )

    class PeriodicReceiveEnableRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_PERIODIC_RECEIVE_ENABLE
        struct = Struct(
            "status" / RtlsStatus,
        )

    class AddDeviceToPeriodicAdvListRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_ADD_DEVICE_ADV_LIST
        struct = Struct(
            "status" / RtlsStatus,
        )

    class RemoveDeviceFromPeriodicAdvListRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_REMOVE_DEVICE_ADV_LIST
        struct = Struct(
            "status" / RtlsStatus,
        )

    class ReadPeriodicAdvListSizeRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_READ_ADV_LIST_SIZE
        struct = Struct(
            "status" / RtlsStatus,
        )

    class ClearPeriodicAdvListRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_CLEAR_ADV_LIST
        struct = Struct(
            "status" / RtlsStatus,
        )

    class ReadAdvListSizeRsp(NpiRequest, AsyncReq, FromNwp):
        command = Commands.RTLS_CMD_READ_ADV_LIST_SIZE
        struct = Struct(
            "status" / RtlsStatus,
            "listSize" / Int8ul
        )

    class SyncEstRsp(NpiRequest, AsyncReq, FromNwp):
        command = Commands.RTLS_EVT_SYNC_EST
        struct = Struct(
            "opcode" / Int8ul,
            "status" / RtlsStatus,
            "syncHandle" / Int16ul,
            "advSid" / Int8ul,
            "advAddrType" / Int8ul,
            "advAddress" / NiceBytes(ReverseBytes(Byte[6])),
            "advPhy" / Int8ul,
            "periodicAdvInt" / Int16ul,
            "advClockAccuracy" / Int8ul
        )

    class SyncLostRsp(NpiRequest, AsyncReq, FromNwp):
        command = Commands.RTLS_EVT_SYNC_LOST
        struct = Struct(
            "opcode" / Int8ul,
            "syncHandle" / Int16ul
        )

    class TermSyncRsp(NpiRequest, AsyncReq, FromNwp):
        command = Commands.RTLS_EVT_TERMINATE_SYNC
        struct = Struct(
            "status" / RtlsStatus
        )

    class EnableClAoaRsp(NpiRequest, AsyncReq, FromNwp):
        command = Commands.RTLS_EVT_CL_AOA_ENABLE
        struct = Struct(
            "status" / RtlsStatus,
            "syncHandle" / Int16ul
        )

    class PeriodicAdvReportRsp(NpiRequest, AsyncReq, FromNwp):
        command = Commands.RTLS_EVT_PERIODIC_ADV_RPT
        struct = Struct(
            "opcode" / Int8ul,
            "syncHandle" / Int16ul,
            "txPower" / Int8sl,
            "rssi" / Int8sl,
            "cteType" / Int8ul,
            "dataStatus" / Int8ul,
            "dataLen" / Int8ul,
            "data" / NiceBytes(Byte[this.dataLen])
        )

    class ConnectionlessAoaEnableRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_CL_AOA_ENABLE
        struct = Struct(
            "status" / RtlsStatus
        )

    #
    # Requests
    #

    class IdentifyReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_IDENTIFY
        struct = None

    class ScanReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_SCAN
        struct = None

    class ResetDeviceReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_RESET_DEVICE
        struct = None

    class ConnectReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_CONNECT
        struct = Struct(
            'addrType' / Enum(Int8ul),
            'peerAddr' / NiceBytes(ReverseBytes(Byte[6])),
            'connInterval' / Int16ul,
        )

    class GetActiveConnInfoReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_GET_ACTIVE_CONN_INFO
        struct = Struct(
            'connHandle' / Int16ul,
        )

    class TerminateLinkReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_TERMINATE_LINK
        struct = Struct(
            'connHandle' / Int16ul,
        )

    class AoaStartReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_AOA_ENABLE
        struct = Struct(
            "connHandle" / Int16ul,
            "enable" / Int8ul,  # Enable/Disable
            "cteInterval" / Int16ul,  # 0 = run once, > 0 = sample CTE every cteInterval until told otherwise
            "cteLength" / Int8ul,  # Length of the CTE (2 - 20), used for AoA receiver
        )

    class AoaSetParamsReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_AOA_SET_PARAMS
        struct = Struct(
            "aoaRole" / Enum(Int8ul, AoaRole),  # AOA_COORDINATOR, AOA_RESPONDER, AOA_PASSIVE
            "aoaResultMode" / Enum(Int8ul, AoaResultMode),  # AOA_MODE_ANGLE, AOA_MODE_PAIR_ANGLES, AOA_MODE_RAW
            "connHandle" / Int16ul,
            "slotDurations" / Int8ul,  # 1us/2us sampling slots
            "sampleRate" / Int8ul,  # 1Mhz (BT5.1 spec), 2Mhz, 3Mhz or 4Mhz - this enables oversampling
            "sampleSize" / Int8ul,  # 8 bit sample (as defined by BT5.1 spec), 16 bit sample (higher accuracy)
            "sampleCtrl" / Int8ul,  # sample control flags 0x00-default filtering, 0x01-RAW_RF no filtering
            "samplingEnable" / Int8ul,
            # 0 = mask CTE even if enabled, 1 = don't mask CTE, even if disabled (support Unrequested CTE)
            "numAnt" / Int8ul,  # Number of antennas in antenna array
            "antArray" / Int8ul[this.numAnt],  # GPIO's of antennas
        )

    class SetConnInfoReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_CONN_PARAMS
        struct = Struct(
            "connHandle" / Int16ul,
            "accessAddress" / Int32ul,
            "connRole" / Int8ul,
            "connInterval" / Int16ul,
            "hopValue" / Int8ul,
            "mSCA" / Int16ul,
            "currChan" / Int8ul,
            "chanMap" / Byte[5],
            "crcInit" / Int32ul,
        )

    class GetConnInfoReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_CONN_INFO
        struct = Struct(
            "connHandle" / Int16ul,
            "enable" / Int8ul,
        )

    class SetRtlsParamReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_SET_RTLS_PARAM
        struct = Struct(
            "connHandle" / Int16ul,
            "rtlsParamType" / Enum(Int8ul, RtlsParamType),
            "len" / Int8ul,
            "data" / Byte[this.len]
        )

    class CreateSyncReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_CREATE_SYNC
        struct = Struct(
            "advSID" / Int8ul,
            "options" / Int8ul,
            "advAddrType" / Int8ul,
            "advAddress" / NiceBytes(ReverseBytes(Byte[6])),
            "skip" / Int16ul,
            "syncTimeout" / Int16ul,
            "syncCteType" / Int8ul
        )

    class CreateSyncCancelReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_CREATE_SYNC_CANCEL
        struct = None

    class TerminateSyncReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_TERMINATE_SYNC
        struct = Struct(
            "syncHandle" / Int16ul
        )

    class HeapReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_HEAP_SIZE
        struct = None

    class HeapRsp(NpiRequest, SyncRsp, FromNwp):
        command = Commands.RTLS_CMD_HEAP_SIZE
        struct = Struct(
            "totalHeap" / Int32ul,
            "freeHeap" / Int32ul
        )

    class PeriodicReceiveEnableReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_PERIODIC_RECEIVE_ENABLE
        struct = Struct(
            "syncHandle" / Int16ul,
            "enable" / Int8ul
        )

    class AddDeviceToPeriodicAdvListReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_ADD_DEVICE_ADV_LIST
        struct = Struct(
            "advAddrType" / Int8ul,
            "advAddress" / NiceBytes(ReverseBytes(Byte[6])),
            "advSID" / Int8ul
        )

    class RemoveDeviceFromPeriodicAdvListReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_REMOVE_DEVICE_ADV_LIST
        struct = Struct(
            "advAddrType" / Int8ul,
            "advAddress" / NiceBytes(ReverseBytes(Byte[6])),
            "advSID" / Int8ul
        )

    class ConnectionlessAoaEnableReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_CL_AOA_ENABLE
        struct = Struct(
            "aoaRole" / Enum(Int8ul, AoaRole),
            "aoaResultMode" / Enum(Int8ul, AoaResultMode),
            "syncHandle" / Int16ul,
            "enable" / Int8ul,
            "slotDuration" / Int8ul,
            "sampleRate" / Int8ul,  # 1Mhz (BT5.1 spec), 2Mhz, 3Mhz or 4Mhz - this enables oversampling
            "sampleSize" / Int8ul,  # 8 bit sample (as defined by BT5.1 spec), 16 bit sample (higher accuracy)
            "sampleCtrl" / Int8ul,  # sample control flags 0x00-default filtering, 0x01-RAW_RF no filtering
            "maxSampleCte" / Int8ul,
            "numAnt" / Int8ul,
            "pAntPattern" / Int8ul[this.numAnt],
        )

    class ReadPeriodicAdvListSizeReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_READ_ADV_LIST_SIZE
        struct = None

    class ClearPeriodicAdvListReq(NpiRequest, SyncReq, FromAp):
        command = Commands.RTLS_CMD_CLEAR_ADV_LIST
        struct = None

    @builder_class(IdentifyReq)
    def identify(self): pass

    @builder_class(ScanReq)
    def scan(self): pass

    @builder_class(ConnectReq)
    def connect(self, addrType, peerAddr): pass

    @builder_class(TerminateLinkReq)
    def terminate_link(self, connHandle): pass

    @builder_class(AoaStartReq)
    def aoa_start(self, connHandle, enable, cteInterval, cteLength): pass

    @builder_class(AoaSetParamsReq)
    def aoa_set_params(self, aoaRole, aoaResultMode, connHandle, slotDurations, sampleRate, sampleSize, sampleCtrl,
                       samplingEnable, numAnt, antArray): pass

    @builder_class(SetConnInfoReq)
    def set_ble_conn_info(self, connHandle, accessAddress, connInterval, hopValue, mSCA, currChan, chanMap,
                          crcInit): pass

    @builder_class(ResetDeviceReq)
    def reset_device(self): pass

    @builder_class(GetConnInfoReq)
    def get_conn_info(self, connHandle, enable): pass

    @builder_class(SetRtlsParamReq)
    def set_rtls_param(self, connHandle, rtlsParamType, len, data): pass

    @builder_class(GetActiveConnInfoReq)
    def get_active_conn_info(self, connHandle): pass

    @builder_class(CreateSyncReq)
    def create_sync(self, advSID, options, advAddrType, advAddress, skip, syncTimeout, syncCteType): pass

    @builder_class(CreateSyncCancelReq)
    def create_sync_cancel(self): pass

    @builder_class(TerminateSyncReq)
    def terminate_sync(self, syncHandle): pass

    @builder_class(PeriodicReceiveEnableReq)
    def periodic_receive_enable(self, syncHandle, enable): pass

    @builder_class(AddDeviceToPeriodicAdvListReq)
    def add_device_to_periodic_adv_list(self, advAddrType, advAddress, advSID): pass

    @builder_class(RemoveDeviceFromPeriodicAdvListReq)
    def remove_device_from_periodic_adv_list(self, advAddrType, advAddress, advSID): pass

    @builder_class(ReadPeriodicAdvListSizeReq)
    def read_periodic_adv_list_size(self): pass

    @builder_class(ClearPeriodicAdvListReq)
    def clear_periodic_adv_list_size(self): pass

    @builder_class(CreateSyncReq)
    def create_sync(self, advSID, options, advAddrType, advAddress, skip, syncTimeout, syncCteType): pass

    @builder_class(CreateSyncCancelReq)
    def create_sync_cancel(self): pass

    @builder_class(TerminateSyncReq)
    def terminate_sync(self, syncHandle): pass

    @builder_class(PeriodicReceiveEnableReq)
    def periodic_receive_enable(self, syncHandle, enable): pass

    @builder_class(AddDeviceToPeriodicAdvListReq)
    def add_device_to_periodic_adv_list(self, advAddrType, advAddress, advSID): pass

    @builder_class(RemoveDeviceFromPeriodicAdvListReq)
    def remove_device_from_periodic_adv_list(self, advAddrType, advAddress, advSID): pass

    @builder_class(ReadPeriodicAdvListSizeReq)
    def read_periodic_adv_list_size(self): pass

    @builder_class(ClearPeriodicAdvListReq)
    def clear_periodic_adv_list(self): pass

    @builder_class(ConnectionlessAoaEnableReq)
    def connectionless_aoa_enable(self, aoaRole, aoaResultMode, syncHandle, enable, slotDuration, sampleRate,
                                  sampleSize, sampleCtrl, maxSampleCte, numAnt, pAntPattern): pass

    @builder_class(HeapReq)
    def heap_req(self): pass
