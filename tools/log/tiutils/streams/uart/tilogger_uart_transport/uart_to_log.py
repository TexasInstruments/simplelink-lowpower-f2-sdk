from ctypes import c_int32
import logging
import enum
from collections import deque
import math
from abc import ABC, abstractmethod
import os
import sys
from dataclasses import *
import struct

from typing import Optional

from tilogger.interface import LogPacket
from tilogger.logger import Logger
from tilogger.tracedb import ElfString, Opcode
from tilogger.helpers import build_value

from .uart_framer import UARTFrame, UARTDataFrame, UARTOpcode

py_logger = logging.getLogger("UartPacketiser")
# To enable debug output, uncomment the following line
# logging.basicConfig(level=logging.DEBUG)

SWIT_SIZE = 4


class InfoOps(enum.Enum):
    TIMESTAMP_INFO = 3


class UartLogPacketData:
    def __init__(
        self,
        elf_string: ElfString,
        header: UARTDataFrame,
        alias: str,
        timestamp: Optional[int] = None,
        timestamp_local: Optional[int] = None,
    ):
        self.remaining_length: int
        self.elf_string = elf_string
        self.alias = alias
        self.timestamp: Optional[int] = timestamp
        self.timestamp_local: Optional[int] = timestamp_local
        self.data: bytes = bytes(header.data)
        self.next_frame_has_length: bool = False

        if self.elf_string.opcode == Opcode.FORMATTED_TEXT:
            self.remaining_length = int(elf_string.nargs) * SWIT_SIZE

        elif self.elf_string.opcode == Opcode.BUFFER:
            self.remaining_length = 1024
            self.next_frame_has_length = True

        else:
            self.remaining_length = 0

    def append(self, uart_frame: UARTDataFrame) -> None:
        if self.next_frame_has_length:
            # Set remaining length to the data frame and clear the read-length flag
            self.next_frame_has_length = False
            self.remaining_length = build_value(uart_frame.data)
            py_logger.debug("FRAMING: Frame length set to %d", self.remaining_length)

        else:
            # Adjust remaining length and append data
            self.remaining_length -= len(uart_frame)
            self.data += bytes(uart_frame.data)

    def to_log_packet(self) -> LogPacket:
        py_logger.debug("FRAMING: Handing off %s Packet with %d bytes of data", self.elf_string.opcode, len(self.data))
        return LogPacket.from_elf_string(self.elf_string, self.data, self.alias, self.timestamp, self.timestamp_local)


@dataclass
class TimestampInfo:
    """Module for converting from native timestamp format to seconds

    The format is natively provided as a struct,
    ```
        struct {
            uint32_t fracBytes:4;  //<! Octets (LSB) used for fractional part (if any)
            uint32_t intBytes:4;   //<! Octets (MSB) used for integer part
            uint32_t exponent:8;   //<! How much to scale native time to get seconds.
            int32_t multiplier:16; //<! Signed 16-bit multiplier, eg 8 if one tick is 8 time units
        } format;
        uint32_t value;
    ```
    """

    exponent: int
    multiplier: int
    int_width: int
    frac_width: int
    _curr_word: Optional[int] = field(default=None, repr=False)

    @staticmethod
    def from_native_format(ts_format: bytearray):
        value = struct.unpack("L", ts_format)[0]
        frac_width = value & 0xF
        int_width = (value >> 4) & 0xF
        exponent = (value >> 8) & 0xFF
        # Decode the multiplier as a signed integer
        multiplier = (value >> 16) & 0x7FFF
        if (value >> 28) & 0x8FFF == 0x8:
            multiplier -= 0x8000
        return TimestampInfo(exponent, multiplier, int_width, frac_width)

    def parse_native(self, word: int):
        """Based on configuration convert native words into fractional seconds

        Args:
            word (32-bit): Word comprising either the entire timestamp or a part of it (if > 32 bit total)

        Returns:
            int: Returns a timestamp in seconds
        """

        fractional = (word & ((1 << (self.frac_width * 8)) - 1)) / 2 ** (self.frac_width * 8)
        integral = (word >> (self.frac_width * 8)) & ((1 << (self.int_width * 8)) - 1)
        if self.multiplier > 0:
            return self.multiplier * (integral + fractional) * 10**-self.exponent
        elif self.multiplier < 0:
            return (1 / abs(self.multiplier)) * (integral + fractional) * (10**-self.exponent)


class UARTPacketiser:
    """
    Manages parsing UART frames into LogPackets

    Stores a sorted dictionary of frames as they are being parsed.
    Stores a dictionary of watchpoint strings to match to corresponding watchpoints.
    Stores a trace database for use by members.

    Args:
        db: trace database
        clock: clock speed of embedded device

    """

    def __init__(self, db=None, logsink: Logger = None, alias="UART0", clock=48000000, baud=3000000):
        self._trace_db = db
        self._event_sets = {}
        self._watchpoints = [None] * 4
        self._info_opcode = None
        self._ts_info = None
        self._rat_t = 0
        self._rat_s = 0
        self._rtc_s = 0
        self.prescaler = 1
        self.clock = clock
        self.global_timestamp_delta = None
        self.offset = 0
        self.baudrate = int(baud)
        self.logger = logsink
        self.alias = alias
        self.timestamp_format = 0

        self._current_packet: Optional[UartLogPacketData] = None

    def parse(self, uart_frame: UARTFrame) -> Optional[LogPacket]:
        """
        The top-level UARTFrame parser.

        Will directly build all frames besides software source frames (these are build by build_sw_source_frame).
        After all frames are built, completed() will be called.

        Args:
          uart_frame: input UARTFrame

        Returns:

        """
        try:
            packet = None

            if uart_frame.opcode == UARTOpcode.TIMESTAMP_FORMAT:
                self._ts_info = TimestampInfo.from_native_format(uart_frame.data[0:4])
                py_logger.debug("Native timestamp format is {self._ts_info}")
                return None

            if uart_frame.opcode == UARTOpcode.DATA:
                assert isinstance(uart_frame, UARTDataFrame)

                # This may not be the entire frame. Try to build it.
                # packet will be None unless the whole frame is completed
                packet = self.append_packet(uart_frame, self.offset)

                if packet:
                    return packet.to_log_packet()

        except Exception as exc:
            exc_type, _, exc_tb = sys.exc_info()
            if exc_tb:
                fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
                py_logger.error("{} @ {} {}: ".format(exc_type, fname, exc_tb.tb_lineno) + str(exc))

        return None

    def append_packet(self, uart_frame: UARTDataFrame, time_offset) -> Optional[UartLogPacketData]:

        # Get elf data from header
        header = build_value(uart_frame.data[0:4])
        device_timestamp = self._ts_info.parse_native(build_value(uart_frame.data[4:8]))
        py_logger.debug(
            "timestamp raw: %d, formatted: %f",
            build_value(uart_frame.data[4:8]),
            self._ts_info.parse_native(build_value(uart_frame.data[4:8])),
        )
        # Delete the timestamp from the data, to align with expected format: [ptr arg0 arg1 ... argn]
        del uart_frame.data[4:8]

        if header in self._trace_db.traceDB:
            elf_string = self._trace_db.traceDB[header]
            self._current_packet = UartLogPacketData(
                elf_string,
                uart_frame,
                alias=self.alias,
                timestamp_local=device_timestamp,
                timestamp=device_timestamp,
            )
            py_logger.debug(
                "FRAMING: New Frame with len %d header 0x%x, timestamp %f",
                self._current_packet.remaining_length,
                header,
                device_timestamp,
            )
        else:
            # This address does not exist in the trace database
            py_logger.warning("FRAMING: corruption: no trace database information at 0x%x", header)
            self._current_packet = None

        packet = self._current_packet
        self._current_packet = None
        return packet

    def reset(self):
        pass
