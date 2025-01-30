"""
This module describes the host-side interfaces for developers of transport implementations or plugins.
"""
from dataclasses import dataclass, field
from enum import Enum
from argparse import ArgumentParser
from pathlib import Path

from typing import Optional, NoReturn, List, TYPE_CHECKING
from abc import ABC, abstractmethod, abstractproperty

from tilogger.tracedb import ElfString, Opcode, TraceDB
from tilogger.helpers import build_value

if TYPE_CHECKING:
    from tilogger.logger import Logger


class LogLevel(Enum):
    Log_DEBUG = 1
    Log_VERBOSE = 4
    Log_INFO = 16
    Log_WARNING = 64
    Log_ERROR = 256


@dataclass
class LoggerCliCtx:
    """Context object shared between CLI parser handlers to store and pass along info

    May be used by transports to access symbol files provided via common --elf option
    used before command invocations.
    """

    symbol_files: List[Path] = field(default_factory=list)
    start_ws: bool = False
    ws_pipe: str = ""


class LogPacket:
    """
    LogPacket is a data class for describing log records.

    Some notes on data representation:
     - alias (str):         Alias for this log stream
     - module (str):        This string should match one of the module names used on the target.

     - opcode (int):        This opcode represents the "logging operation" this packet represents. Opcodes
                            0-9 are RESERVED for use only by Log.h APIs. Any custom log packets (e.g.
                            DWT watchpoint packets, or program counter traces) must use opcodes 10 and up.
                            If your embedded code defines specific opcodes, you should also provide a Python
                            module implementing the LogFormatterABC (below) that converts the representation
                            of those new opcodes into user-facing strings.

     - timestamp (int):     A 32-bit integer defining a timestamp. A transport layer should be able to resolve
       (optional)           this timestamp into real-time seconds, to tolerable accuracy, and (barring
                            rollovers) it must be monotonically increasing. Examples of good timestamps:
                            a timer derived directly or some division of the system clock, an RTC, or a
                            hardware timer. A 'message number' incremented with each packet would not be a
                            good timestamp because it cannot be converted into seconds.

     - data (bytes):        The data bytes for this packet, including the header that contains the ELF
                            pointer if applicable.

     - level (LogLevel):    What LogLevel the call was made with.

     - _str_data (bytes):   Internal use only. Holds the string representation of this log packet once
                            it has been through the formatting mechanism. Do not modify this value.

     - _final_str_data:     Internal use only. Holds the string representation of this log packet as it will
       (bytes)              be sent over the pipe, including the module and opcode values. Do not modify this value.
    """

    def __init__(
        self,
        alias: str,
        module: str,
        opcode: int,
        level: LogLevel,
        filename: str,
        lineno: str,
        timestamp: Optional[int],
        timestamp_local: Optional[int],
        data: bytes,
        trace_db: TraceDB,
    ):
        """
        Create a new LogPacket instance.

        Args:
            module (str): Name of the Module the user has logged from
            opcode (int): Integer 0-9 for TI Log APIs or a module-specific opcode >= 10
            level (LogLevel): One of the values from the LogLevel enum
            timestamp (int): Transport-dependent timestamp code
            data (bytes): The data bytes for this packet
            trace_db (TraceDB): Backing store of symbols/debug information if needed for further parsing
        """
        self.alias: str = alias
        self.module: str = module
        self.opcode: int = opcode
        self.timestamp: Optional[int] = timestamp
        self.timestamp_local: Optional[int] = timestamp_local
        self.level: LogLevel = level
        self.filename = filename
        self.lineno = lineno
        self.data: bytes = data
        self.trace_db: TraceDB = trace_db

        self._str_data: str = ""
        self._final_str_data: str = ""

    @classmethod
    def from_elf_string(
        cls,
        elf_string: ElfString,
        data: bytes,
        alias: str,
        timestamp: Optional[int] = None,
        timestamp_local: Optional[int] = None,
    ):
        """
        Generates a LogPacket instance from an ElfString and packet data.

        Args:
            elf_string (ElfString): The ElfString that corresponds to this log site.
            data (bytes): The data bytes associated with this packet.
            timestamp (int, optional): A timestamp value (see class docstring). Defaults to None.

        Returns:
            LogPacket: A completed LogPacket.
        """
        level = LogLevel[elf_string.level]
        return cls(
            alias,
            elf_string.module_id,
            elf_string.opcode.value,
            level,
            elf_string.file.replace(r"\\", "/"),
            elf_string.line,
            timestamp,
            timestamp_local,
            data,
            elf_string.trace_db,
        )

    def __str__(self) -> str:
        """
        This is not normally used, but builds a human-readable packet representation for debugging or verbose mode.
        """
        elf_ptr = build_value(self.data[:4])
        data_str = " ".join(["0x%x" % byte for byte in self.data[4:]])
        return f"""LogPacket(module={self.module}, opcode={self.opcode}, level={self.level}, ts={self.timestamp:0.4f}, ptr=0x{elf_ptr:08X}, data={data_str}, formatted={self._str_data}"""


class TransportABC(ABC):
    """
    This interface serves as a base class for transport layers.
    All methods marked @abstractmethod must be implemented by a transport layer.
    """

    @property
    @abstractmethod
    def alias(self) -> str:
        """Name of interface"""

    @abstractmethod
    def start(self, logger: "Logger") -> NoReturn:
        """
        start is called shortly after the 'logger' CLI is invoked. It should never return, unless
        the program indicates completion or encounters an error.

        Args:
            logger (Logger): A logger output instance that accept log frames

        Returns:
            NoReturn: This function should retain execution control and never return unless the program exits.
        """

    @abstractmethod
    def reset(self):
        """
        Handle a device or protocol reset, clearing module state as needed.
        """

    @abstractmethod
    def stop(self):
        """
        Stop processing and kill threads.
        """

    @abstractmethod
    def timestamp_to_seconds(self, timestamp: int) -> float:
        """
        Converts the input timestamp, in implementation-dependent units, into decimal seconds.

        Note that it is not elapsed time that is requested; if the clock source is an RTC and it
        happens to currently read 4 days and a few minutes, then that value should be returned (in seconds).

        Args:
            timestamp (int): A timestamp, provided as part of a LogPacket.

        Returns:
            float: An equivalent timestamp, in seconds.
        """


class LogFormatterABC(ABC):
    """
    This interface should be implemented by packages looking to provide rich log formatting.

    If you do not provide a plugin for your module, then logs will be formatted using a default
    implementation. For format_log, this simply prints the resolved format string with inserted arguments.
    For format_packet, the individual bytes are printed in hex, space-separated.

    Both format_log and format_packet must be implemented. See the notes under each function for when they
    will be called. For example, if your module provides no custom opcodes and does not use LogBuf, then
    format_packet will never be called and it can simply return an empty string.

    LogFormatter implemeters must provide an entry point formatted like this:
        entry_points={
            "tilogger.formatter": [
                "[MODULE_NAME] = YOUR_PACKAGE.YOUR_MODULE:FORMATTER_CLASS",
            ],
        },
    """

    @abstractmethod
    def format_log(self, logged_str: str, packet: LogPacket) -> str:
        """
        This method will be called for the following Log.h APIs: Log_printf, ************

        Args:
            logged_str (str): This contains the fully resolved format string including provided arguments.
            packet (LogPacket): This is the packet that generated this event, as the transport provided it
                                to Logger.Send. Please note that while packet.data is technically valid,
                                plugins are not provided with direct access to the .out file so cannot
                                perform their own format string resolution. Please consider parsing the
                                string e.g. with a regex if you need to reformat the arguments.

        Returns:
            str: The string representation of the *data* for this packet, as it should be presented to the
                 user. Do not include the module name, timestamp or opcode in this string; they are added
                 automatically by Logger.

        Default Implementation:
            The default implementation returns logged_str.
        """

    def format_packet(self, packet: LogPacket) -> str:
        """
        This method will be called when a user calls the LogBuf API from Log.h, or when a module packet
        is received with a custom opcode (>=10). These packets are completely untouched; this function must
        use packet.data to build a meaningful string representation for the user.

        Args:
            packet (LogPacket): This is the packet that generated this event, as the transport provided it
                                to Logger.Send.

        Returns:
            str: The string representation of the *data* for this packet, as it should be presented to the
                 user. Do not include the module name, timestamp or opcode in this string; they are added
                 automatically by Logger.
        """


class LogSubscriberABC(ABC):
    """
    This interface can be implemented by packages who wish to subscribe to packets logged by another module.
    A subscriber is notified about matching packets, but cannot influence their representation or prevent
    them from being emitted.

    Hooks in LogFormatter-derived classes will be called before LogSubscribers are notified.

    LogSubscriber implementers must provide an entry point formatted like this:
        entry_points={
            "tilogger.subscriber": [
                "[MODULE_NAME] = YOUR_PACKAGE.YOUR_MODULE:FORMATTER_CLASS",
            ],
        },
    """

    @abstractmethod
    def notify_packet(self, packet: LogPacket) -> None:
        """
        This method is called when a log packet matching the 'watched' module is emitted.
        It is called just prior to the log's emission over the pipe.

        Args:
            final_string (str): The string representation of the packet, either generated by Logger or
                                by a module plugin implementing LogFormatterABC. See the Logger
                                documentation for a description of the string format here (it is identical
                                to the string format in the pipe).
            packet (LogPacket): The original packet passed to Logger.
        """


class LogOutputABC(ABC):
    """
    This interface can be implemented by modules that wish to provide an output-
    stream for log packets.

    LogOutput implementers must provide an entry point formatted like this:
        entry_points={
            "tilogger.output": [
                "[MODULE_NAME] = YOUR_PACKAGE.YOUR_MODULE:CLASS_FACTORY_FUNCTION",
            ],
        },
    """

    @abstractmethod
    def notify_packet(self, packet: LogPacket) -> None:
        """
        This method is called when a log packet matching the 'watched' module is emitted.
        It is called just prior to the log's emission over the pipe.

        Args:
            final_string (str): The string representation of the packet, either generated by Logger or
                                by a module plugin implementing LogFormatterABC. See the Logger
                                documentation for a description of the string format here (it is identical
                                to the string format in the pipe).
            packet (LogPacket): The original packet passed to Logger.
        """

    @abstractmethod
    def start(self) -> None:
        """
        start is called, in a separate thread, shortly after the 'logger' CLI is invoked.

        This means that start is allowed to retain execution control and never return
        unless the program exits. But start is allowed to return, terminating the thread,
        if the output module does not require any work done in the separate thread.

        Args:
            none
        """
