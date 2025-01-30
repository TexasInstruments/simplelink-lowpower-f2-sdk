import logging
from dataclasses import dataclass, field
from enum import Enum

from typing import Optional
from tilogger.helpers import build_value

logger = logging.getLogger("ITM Framer")

HDR_OVERFLOW = 0x70
MAX_ITM_FRAME_SIZE = 5
ITM_RESET_TOKEN = bytes([0xFB, 0xBB, 0xBB, 0xBB, 0xBB])


class ITMStimulusPort(Enum):
    """ITM ports for software packets"""

    STIM_RAW0 = 0
    STIM_RAW1 = 1
    STIM_RAW2 = 2
    STIM_RAW3 = 3
    STIM_RAW4 = 4
    STIM_RAW5 = 5
    STIM_RAW6 = 6
    STIM_RAW7 = 7
    STIM_RAW8 = 8
    STIM_RAW9 = 9
    STIM_RAW10 = 10
    STIM_RAW11 = 11
    STIM_RAW12 = 12
    STIM_RAW13 = 13
    STIM_RAW14 = 14
    STIM_RAW15 = 15
    STIM_RESV16 = 16
    STIM_RESV17 = 17
    STIM_RESV18 = 18
    STIM_RESV19 = 19
    STIM_RESV20 = 20
    STIM_RESV21 = 21
    STIM_RESV22 = 22
    STIM_RESV23 = 23
    STIM_RESV24 = 24
    STIM_RESV25 = 25
    STIM_RESV26 = 26
    STIM_RESV27 = 27
    STIM_TRACE = 28
    STIM_HEADER = 29
    STIM_SYNC_TIME = 30
    STIM_INFO = 31
    STIM_HW = -1


class ITMOpcode(Enum):
    """Opcodes for ITM frames that are built in ITMFramer"""

    SYNCHRONIZATION = 0
    EXTENSION = 1
    OVERFLOW = 2
    TIMESTAMP = 3
    PACKET_PC = 4
    SOURCE_SW = 5
    COUNTER_WRAP = 6
    EXCEPTION = 7
    TRACE = 8
    UNPARSED = None


HW_EXC_FUNC_DICT = {
    1: "Entered exception indicated by ExceptionNumber field",
    2: "Exited exception indicated by ExceptionNumber field.",
    3: "Returned to exception indicated by ExceptionNumber field.",
}

COUNTER_DICT = {5: "CPI", 4: "Exc", 3: "Sleep", 2: "LSU", 1: "Fold", 0: "Cyc"}

ACCESS_DICT = {
    5: "at {}, Write Access, comparator: {}, value : 0x{:X} ",
    4: "at {}, Read Access, comparator: {}, value : 0x{:X} ",
    3: "at {}, Address access, comparator: {}, value : 0x{:X} ",
    2: "at {}, PC value Access, comparator: {}, value : 0x{:X} ",
}

################################################################################
################################################################################


@dataclass
class ITMFrame:
    """Base ITM frame that stores common information and should be subclassed by other frames"""

    header: int
    opcode: ITMOpcode
    ts_counter: float = 0
    # value: int = 0

    size: int = 0
    string: str = "Frame has not yet been parsed"

    def __len__(self):
        return self.size

    def __str__(self):
        return self.string


@dataclass
class ITMSyncFrame(ITMFrame):
    """
    Synchronization Frame:
    Synchronization packets are at least 47 bytes of value zero followed by 1 byte of value one
    """

    opcode: ITMOpcode = ITMOpcode.SYNCHRONIZATION
    data: list = field(default_factory=list)

    def parse(self, buf):
        """Build ITMSyncFrame from buf"""
        # Find index of 1 and add 1 since we want to remove the 1
        idx = buf.index(1) + 1

        # Store data
        self.data = buf[:idx]
        self.size = len(self.data)
        self.string = f"Synchronization packet of size {self.size}"

        # Remove sync bytes from input data
        return buf[idx:]


@dataclass
class ITMExtensionFrame(ITMFrame):
    """
    Extension Frame:
    Extension packet is continued until most significant bit is 0
    """

    opcode: ITMOpcode = ITMOpcode.EXTENSION
    data: list = field(default_factory=list)

    def parse(self, buf):
        """Build ITMExtensionFrame from buf"""
        for idx, val in enumerate(buf):
            # Look for most significant bit to be 0
            if val & 0x80 == 0:
                self.size = idx
            else:
                self.data.append(buf[idx])

        self.string = f"Extension packet of size {self.size}"
        return buf[self.size :]


@dataclass
class ITMOverflowFrame(ITMFrame):
    """
    Overflow Frame:
    Overflow packet is only one byte
    """

    opcode: ITMOpcode = ITMOpcode.OVERFLOW

    def __post_init__(self):
        """Send warning that overflow frame occurred"""
        logger.warning("ITM Frame Overflow")

        self.string = "ITM Frame overflow packet"


@dataclass
class ITMTimestampFrame(ITMFrame):
    """
    Generic Timestamp frame for all variants:
    Retrieve and parse an ITM timestamp
    """

    opcode: ITMOpcode = ITMOpcode.TIMESTAMP

    timestamp_codes = {0xC: "in sync", 0xD: "TS delayed", 0xE: "packet delayed", 0xF: "packet and timestamp delayed"}

    def parse(self, buf):
        """Build ITMTimestampFrame from buf"""
        # Find out what type of timestamp this is
        try:
            self.string = ITMTimestampFrame.timestamp_codes[self.header >> 4]
        except KeyError:
            self.string = "RESERVED"
            # TODO: Look into what the reserved codes mean and build behaviour for them
            logger.debug("Reserved code (0x%x) used for timestamp", self.header >> 4)

        # Only build a timestamp if there is a continuation bit
        if self.header & 0x80:
            self.ts_counter = 0

            for idx, val in enumerate(buf):
                # Continue adding value, shifting left each time
                self.ts_counter += (val & 0x7F) << (7 * idx)

                # No continuation bit == stop
                if not val & 0x80:
                    self.size = idx + 1
                    break

            self.string = f"TIMESTAMP {self.string}: + {self.ts_counter} cycles"
        else:
            self.string = f"Unknown/Reserved timestamp header: {self.header}"

        return buf[self.size :]


@dataclass
class ITMSourceFrame(ITMFrame):
    """Software or hardware source frame. SW / HW frames should subclass this"""

    port: Optional[ITMStimulusPort] = None

    # A packet-dependent single value e.g. current program counter
    value: int = 0

    def __post_init__(self):
        self.port = ITMStimulusPort(self.header >> 3)

        # Get size (lowest 2 bits of header)
        # 0b00 is an illegal value
        # 1 -> 1 byte, 2 -> 2 bytes, 3 -> 4 bytes
        length = self.header & 0x03
        if length == 3:
            self.size = 4
        else:
            self.size = length


@dataclass
class ITMSourceHwPcFrame(ITMSourceFrame):
    """Program Counter Hardware Source Frame"""

    opcode: ITMOpcode = ITMOpcode.PACKET_PC

    def parse(self, buf):
        """Build ITMSourceHwPcFrame from buf"""
        self.value = build_value(buf[: self.size])

        if self.size == 4:
            self.string = f"Received a PC sample @ {self.ts_counter} PC: 0x{self.value:X}"
        else:
            self.string = f"Received a IDLE PC sample @ {self.ts_counter}"

        return buf[self.size :]


@dataclass
class ITMSourceHwCntWrapFrame(ITMSourceFrame):
    """Hardware Source Frame indicating which counter(s) have wrapped."""

    opcode: ITMOpcode = ITMOpcode.COUNTER_WRAP

    def parse(self, buf):
        """Build ITMSourceHwCntWrapFrame from buf"""
        # Payload is only one byte
        self.value = buf[0]
        return buf[1:]

    def __str__(self):
        string = "At timestamp {}, the following counter(s) wrapped: ".format(self.ts_counter)
        return string + "".join([COUNTER_DICT[i] + " " if self.value & (1 << i) else "" for i in COUNTER_DICT])


@dataclass
class ITMSourceHwExceptionFrame(ITMSourceFrame):
    """Hardware source frame indicating exceptions such as interrupt entrance / exit"""

    opcode: ITMOpcode = ITMOpcode.EXCEPTION
    num_exception: int = 0
    func_exception: int = 0

    def parse(self, buf):
        """Build ITMSourceHwExceptionFrame from buf"""
        self.num_exception = buf[0] + ((buf[1] & 0x1) << 8)
        self.func_exception = (buf[1] & 0x30) >> 4
        # We used two bytes so discard them now
        return buf[2:]

    def __str__(self):
        return "An Exception has occurred @ {}, Exception Number: {}, Function done: {}".format(
            self.ts_counter, self.num_exception, HW_EXC_FUNC_DICT[self.func_exception]
        )


@dataclass
class ITMSourceHwTraceFrame(ITMSourceFrame):
    """Hardware source frame indicating all possibilities: watchpoints, etc"""

    opcode: ITMOpcode = ITMOpcode.TRACE
    hw_packet_type: int = 0
    data_trace_packet_type: int = 0
    comparator: int = 0
    direction: int = 0

    def __post_init__(self):
        """Extract information from hardware packet type"""
        self.data_trace_packet_type = self.hw_packet_type >> 3
        self.comparator = (self.hw_packet_type >> 1) & 0x3
        self.direction = self.hw_packet_type & 0x1
        self.access_type = self.direction + (self.data_trace_packet_type << 1)
        super().__post_init__()

    def parse(self, buf):
        """Build ITMSourceHwTraceFrame from buf"""
        self.value = build_value(buf[: self.size])
        return buf[self.size :]

    def __str__(self):
        return "HW Trace " + ACCESS_DICT[self.access_type].format(self.ts_counter / 1000, self.comparator, self.value)


@dataclass
class ITMSourceSWFrame(ITMSourceFrame):
    """Software source frame"""

    opcode: ITMOpcode = ITMOpcode.SOURCE_SW
    data: bytearray = field(default_factory=bytearray)

    def parse(self, buf):
        """Build ITMSourceSWFrame from buf"""
        # Store data
        self.data = buf[: self.size]
        # build string
        self.string = "SW SWIT at +{}, port {}: {}".format(
            self.ts_counter, self.port.name, " ".join((("0x{:02X}".format(i)) for i in self.data))
        )
        return buf[self.size :]

    def __str__(self):
        return self.string


################################################################################
################################################################################


class ITMFramer:
    """
    Manages parsing serial data into ITMFrames and outputs ITMFrames onto output queue q

    Args:
        q: Output queue

    """

    def __init__(self, output_queue):
        # Create the PDU stream thread.
        self._output_queue = output_queue
        self.last_ts_counter = 0
        self._first_read = True

    def parse(self, buf: bytearray):
        """
        Parse all of an input byte buffer into ITMFrames until the buffer size is <= MAX_ITM_FRAME_SIZE

        There is special funcionality to handle the reset frame (indentified by ITM_RESET_TOKEN). Initially,
        parsing of other frames will not start until the reset frame is found. After this, the buffer received for
        parsing will be searched for the reset frame. If the reset frame is found, all preceding data will be discarded
        and parsing will continue at the  software source frame containing the rest

        Args:
          buf: input buffer to parse

        Returns:
            Unparsed portion of the input buffer

        """
        if not buf:
            return buf

        # If the reset token is found
        if ITM_RESET_TOKEN in buf:
            # Discard anything before the reset token
            buf = buf[buf.index(ITM_RESET_TOKEN) :]

        # Return full buffer in case any part of the reset token was at the end, in
        # which case only part of the token may have been received. Then reparse
        # the next time data is added to the buffer
        elif self._first_read and buf[-1] == 0xBB or buf[-1] == 0xFB:
            return buf

        # If first run and no reset found yet, don't parse anything
        elif self._first_read is True:
            logger.debug("Waiting for a reset frame to begin parsing.")
            return bytearray()

        # While there is a full packet to parse...
        while len(buf) >= MAX_ITM_FRAME_SIZE:
            self._first_read = False

            # Read the header byte
            header = buf.pop(0)

            frame: Optional[ITMFrame] = None

            # Figure out what type of packet this is
            # Synchronization packet is all zeros
            if header == 0x00:
                frame = ITMSyncFrame(0)
            else:
                # Everything besides source packets have the 2 least significant bits set to 0
                if (header & 0x03) == 0x00:
                    # Header == Overflow
                    if header == HDR_OVERFLOW:
                        frame = ITMOverflowFrame(header)
                    # Least significant byte of header Header == 0 --> Local timestamp
                    elif (header & 0x0F) == 0x00:
                        frame = ITMTimestampFrame(header)
                    # Least significant byte of header with S bit masked out == Extension
                    elif (header & 0x0B) == 0x08:
                        frame = ITMExtensionFrame(header)
                # Source packets have non-zero least significant bits
                else:
                    # 3rd bit == 0 --> Software Source
                    if (header & 0x04) == 0x00:
                        frame = ITMSourceSWFrame(header, ts_counter=self.last_ts_counter)
                    # 3rd bit == 1 --> Hardware Source
                    else:
                        # Get packet type discriminator ID
                        hw_packet_type = header >> 3
                        # This is a counter Wrap
                        if hw_packet_type == 0x00:
                            frame = ITMSourceHwCntWrapFrame(header, ts_counter=self.last_ts_counter)
                        # This is an exception event
                        elif hw_packet_type == 0x01:
                            frame = ITMSourceHwExceptionFrame(header, ts_counter=self.last_ts_counter)
                        # This is a PC sampling event
                        elif hw_packet_type == 0x02:
                            frame = ITMSourceHwPcFrame(header, ts_counter=self.last_ts_counter)
                        # This is a hardware trace packet
                        elif hw_packet_type <= 0x17:
                            frame = ITMSourceHwTraceFrame(
                                header, hw_packet_type=hw_packet_type, ts_counter=self.last_ts_counter
                            )
                        # Anything else is invalid
                        else:
                            logger.error("Invalid ITM Hardware Source Packet")

            if frame is not None:
                # Parse packet based on packet type
                try:
                    if frame.opcode is not ITMOpcode.OVERFLOW:
                        # Parse buffer
                        buf = frame.parse(buf)

                        # Update timestamp if needed
                        if frame.opcode == ITMOpcode.TIMESTAMP:
                            self.last_ts_counter = frame.ts_counter

                        # Log packet that was just parsed
                        logger.debug("%s", frame)

                        # queue packet for output
                        self._output_queue.put(frame)

                except Exception as exc:  # pylint: disable=broad-except
                    logger.error("Invalid ITM Packet")
                    logger.debug(exc)

        # Return unparsed data
        return buf
