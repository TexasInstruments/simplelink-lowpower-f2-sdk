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

from .itm_framer import ITMFrame, ITMSourceSWFrame, ITMOpcode, ITMStimulusPort

logger = logging.getLogger("ItmPacketiser")

SWIT_SIZE = 4
RESET_TOKEN = bytes([0xBB, 0xBB, 0xBB, 0xBB])


class InfoOps(enum.Enum):
    TIMESTAMP_INFO = 3


class ItmLogPacketData:
    def __init__(
        self,
        elf_string: ElfString,
        header: ITMSourceSWFrame,
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

    def append(self, itm_frame: ITMSourceSWFrame) -> None:
        if self.next_frame_has_length:
            # Set remaining length to the data frame and clear the read-length flag
            self.next_frame_has_length = False
            self.remaining_length = build_value(itm_frame.data)
            logger.debug("FRAMING: Frame length set to %d", self.remaining_length)

        else:
            # Adjust remaining length and append data
            self.remaining_length -= len(itm_frame)
            self.data += bytes(itm_frame.data)

    def to_log_packet(self) -> LogPacket:
        logger.debug("FRAMING: Handing off %s Packet with %d bytes of data", self.elf_string.opcode, len(self.data))
        return LogPacket.from_elf_string(self.elf_string, self.data, self.alias, self.timestamp, self.timestamp_local)


def rat_from_rtc(rtc_s):
    """
    Turn a real-time clock value into a radio time value

    Args:
      rtc_s: real-time-clock in seconds

    Returns:
        rat_s: radio time in seconds
        rat_t: radio time in ticks

    """
    # Updated assumed RAT tick based on RTC value (magic magic)
    # Doing the same assumptions as done inside the RF  (0x100000000LL/32768)
    # RTC in ticks like on our devices
    rtc_sec = int((math.floor(rtc_s) * 32768))
    rtc_subsec = int((rtc_s - rtc_sec) * 2**32)
    new_rat = (rtc_sec << 32) + rtc_subsec
    # Conservatively assume that we are just about to increment
    # the RTC Scale with the 4 MHz that the RAT is running
    # Add the RAT offset for RTC == 0 * /
    new_rat += 4294967296 / 32768
    new_rat *= 4000000  # Scale to 4 MHz ticks
    new_rat = new_rat / 4294967296
    # Store as ticks
    rat_t = new_rat
    # Store as time
    rat_s = new_rat / 4000000
    return rat_s, rat_t


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
        multiplier = (value >> 16) & 0xFFFF
        return TimestampInfo(exponent, multiplier, int_width, frac_width)

    def parse_native(self, word: int) -> Optional[int]:
        """Based on configuration convert native words into fractional seconds

        Args:
            word (32-bit): Word comprising either the entire timestamp or a part of it (if > 32 bit total)

        Returns:
            Optional[int]: Returns a number if able to parse else None if more data needed
        """

        # If we have been provided with the 32 LSB of the timestamp already
        if self._curr_word is not None:
            word = word << 32 | self._curr_word
            self._curr_word = None

        # If we need more bits to parse the timestamp, save and return
        elif self._curr_word is None and self.int_width + self.frac_width > 32:
            self._curr_word = word
            return None

        fractional = (word & ((1 << self.frac_width) - 1)) / 2**self.frac_width
        integral = (word >> self.frac_width) & ((1 << self.int_width) - 1)
        return self.multiplier * (integral + fractional) * 10**-self.exponent


class ITMPacketiser:
    """
    Manages parsing ITM frames into LogPackets

    Stores a sorted dictionary of frames as they are being parsed.
    Stores a dictionary of watchpoint strings to match to corresponding watchpoints.
    Stores a trace database for use by members.

    Args:
        db: trace database
        clock: clock speed of embedded device

    """

    def __init__(self, db=None, logsink: Logger = None, alias="ITM0", clock=48000000, baud=12000000):
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

        self._current_packet: Optional[ItmLogPacketData] = None

    def parse(self, itm_frame: ITMFrame) -> Optional[LogPacket]:
        """
        The top-level ITMFrame parser.

        Will directly build all frames besides software source frames (these are build by build_sw_source_frame).
        When a timestamp is received from ITM, the running time values will be updated.
        After all frames are built, completed() will be called.

        Args:
          itm_frame: input ITMFrame

        Returns:

        """
        try:
            if itm_frame.opcode == ITMOpcode.TIMESTAMP:
                # Update running timestamps then discard the frame
                self._rtc_s += itm_frame.ts_counter / (self.clock / self.prescaler)
                self._rat_s, self._rat_t = rat_from_rtc(self._rtc_s)
                self.offset = 0
                return None

            packet = None
            self.offset += (len(itm_frame) + 1) * 1 / self.baudrate

            if itm_frame.opcode == ITMOpcode.SOURCE_SW:
                assert isinstance(itm_frame, ITMSourceSWFrame)

                if itm_frame.port == ITMStimulusPort.STIM_INFO:
                    self.parse_control_frame(itm_frame, self.offset)

                if itm_frame.port == ITMStimulusPort.STIM_SYNC_TIME:
                    self.parse_resync_frame(itm_frame, self.offset)

                # This may not be the entire frame. Try to build it.
                # packet will be None unless the whole frame is completed
                packet = self.append_packet(itm_frame, self.offset)

                if packet:
                    return packet.to_log_packet()

        except Exception as exc:
            exc_type, _, exc_tb = sys.exc_info()
            if exc_tb:
                fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
                logger.error("{} @ {} {}: ".format(exc_type, fname, exc_tb.tb_lineno) + str(exc))

        return None

    def handle_ts_info(self, prescaler=None, ts_format=None):
        if prescaler:
            prescaler_lut = {0: 1, 1: 4, 2: 16, 3: 64}
            self.prescaler = prescaler_lut[prescaler]
            logger.debug(f"TPIU prescaler is {self.prescaler}")

        if ts_format:
            self._ts_info = TimestampInfo.from_native_format(ts_format)
            logger.debug(f"Native timestamp format is {self._ts_info}")
            self._info_opcode = None  # done parsing this opcode

    def parse_control_frame(self, itm_frame: ITMSourceSWFrame, time_offset) -> None:
        # Size 4 is only ever continuation
        if itm_frame.size == 4:
            (word,) = struct.unpack("I", itm_frame.data)
            if word == 0xBBBBBBBB:
                logger.debug("Parsed reset control frame")
            elif self._info_opcode == InfoOps.TIMESTAMP_INFO:
                self.handle_ts_info(ts_format=itm_frame.data)  # Different unpack

        # Size 2 is start-of with immediate
        if itm_frame.size == 2:
            opcode, imm = struct.unpack("BB", itm_frame.data)
            self._info_opcode = InfoOps(opcode)
            logger.debug(f"Got opcode {InfoOps(opcode)} with immediate value {imm}")
            if InfoOps(opcode) == InfoOps.TIMESTAMP_INFO:
                self.handle_ts_info(prescaler=imm)

        # Size 1 is start-of, possibly indicating 32-bit to follow
        if itm_frame.size == 1:
            opcode = struct.unpack("B", itm_frame.data)
            self._info_opcode = InfoOps(opcode)
            logger.debug(f"Got opcode {opcode}")

    def parse_resync_frame(self, itm_frame: ITMSourceSWFrame, time_offset) -> None:
        dev_time = self._ts_info.parse_native(struct.unpack("I", itm_frame.data)[0])
        if dev_time:
            logger.debug(f"Device clock is {dev_time:0.5f}, overriding current time of {self._rtc_s:0.5f}")
            self._rtc_s = dev_time

            if self.global_timestamp_delta is None:
                # TODO: Get rid of jitter and update every time a new sync happens
                self.global_timestamp_delta = self.logger.get_system_time(dev_time) - dev_time

    def append_packet(self, itm_frame: ITMSourceSWFrame, time_offset) -> Optional[ItmLogPacketData]:
        if itm_frame.port == ITMStimulusPort.STIM_HEADER:
            # Get elf data from header
            header = build_value(itm_frame.data)

            if header in self._trace_db.traceDB:
                elf_string = self._trace_db.traceDB[header]
                self._current_packet = ItmLogPacketData(
                    elf_string,
                    itm_frame,
                    alias=self.alias,
                    timestamp_local=self._rtc_s + time_offset,
                    timestamp=self._rtc_s + time_offset + (self.global_timestamp_delta or 0),
                )
                logger.debug(
                    "FRAMING: New Frame with len %d header 0x%x", self._current_packet.remaining_length, header
                )
            else:
                # This address does not exist in the trace database
                logger.warning("FRAMING: corruption: no trace database information at 0x%x", header)
                self._current_packet = None

            if self._current_packet.remaining_length == 0:
                packet = self._current_packet
                self._current_packet = None

                return packet

        elif itm_frame.port == ITMStimulusPort.STIM_TRACE:
            if not self._current_packet:
                # Fall through and return None
                logger.warning("Unexpected trace packet with no header! Discarding.")
            else:
                # Append the incoming data to the currently assembling packet
                self._current_packet.append(itm_frame)
                logger.debug(
                    "FRAMING: %d bytes added, remaining: %d", len(itm_frame), self._current_packet.remaining_length
                )

                # Note that < 0 here means we somehow appended a trace packet to a header that said it had no data
                # This should not happen as long as all opcodes are correctly configured
                if self._current_packet.remaining_length <= 0:
                    packet = self._current_packet
                    self._current_packet = None

                    return packet

        return None

    def reset(self):
        """Handle reset frame."""
        pass
