from pathlib import Path
import struct
import logging
import platform
from struct import pack
import typing
import pkg_resources
import argparse
import typer
import click
import subprocess
import os
import sys
import threading
import time

from typing import DefaultDict, Dict, List
from collections import defaultdict

from tilogger.interface import LogOutputABC, LogPacket, LoggerCliCtx, TransportABC, LogFormatterABC, LogSubscriberABC
from tilogger.tracedb import Opcode, TraceDB, ElfString
from tilogger.helpers import build_value

# Upper value of opcodes used/reserved by Log.h
RESERVED_OPCODES = 10

logger = logging.getLogger("TI Logger")


class Logger:
    def __init__(self, transports: List[TransportABC], outputs: List[LogOutputABC]):
        # Locate all transports/formatters/subscribers by entry points
        # Note that any duplicate formatters are resolved to a single formatter last-come-first-served
        self._formatters: Dict[str, LogFormatterABC] = {
            entry_point.name: entry_point.load()
            for entry_point in pkg_resources.iter_entry_points("tilogger.formatter")
        }

        self.timebase = None

        self.subscribers: DefaultDict[str, List[LogSubscriberABC]] = defaultdict(list)

        # Subscribers are slightly more difficult, because we need to handle duplicates explicitly
        for entry_point in pkg_resources.iter_entry_points("tilogger.subscriber"):
            self.subscribers[entry_point.name].append(entry_point.load())

        self.transports: List[TransportABC] = transports
        self.outputs: List[LogOutputABC] = outputs

        # Start transport threads
        self.transport_threads = [
            threading.Thread(target=x.start, args=[self], daemon=True, name=x.alias) for x in self.transports
        ]
        for thread in self.transport_threads:
            thread.start()

        # Start output threads also
        self.output_threads = [
            threading.Thread(target=x.start, daemon=True, name=x.__class__.__name__) for x in self.outputs
        ]
        for thread in self.output_threads:
            thread.start()

    def get_system_time(self, device_time: float):
        """Helper for transports, resets on first request to have t0 == first
        device time received"""
        if self.timebase == None:
            self.timebase = time.time() - device_time
            return device_time
        return time.time() - self.timebase

    def log(self, packet: LogPacket):
        logger.debug("Handling %s", packet)

        # Special handling for Log.h opcodes; all opcodes except
        # REPLAY_FILE have elf strings and may need to be expanded
        # before further handling
        if packet.opcode < RESERVED_OPCODES and packet.opcode != Opcode.REPLAY_FILE.value:
            self.format_dobby_packet(packet)

        # Duplicate formatters for a module are not permitted
        if packet.module in self._formatters:
            packet._str_data = self._formatters[packet.module].format_log(packet._str_data, packet)

        logger.info(f"[{packet.timestamp : 0.3f}] {packet._str_data}")

        # Log this packet before notification, so badly behaved subscribers can't break the datapath
        for output in self.outputs:
            if hasattr(output, "_try_connect_backlog"):
                getattr(output, "_try_connect_backlog")()

            output.notify_packet(packet)

        # Duplicate subscribers are permitted
        if packet.module in self.subscribers:
            for subscriber in self.subscribers[packet.module]:
                subscriber.notify_packet(packet)

    def wait_threads(self):
        try:
            while threading.active_count() > 1:
                time.sleep(0.1)
                pass
        finally:
            typer.echo("Exiting, hoping all threads exit too")
            for transport in self.transports:
                transport.stop()

    def format_dobby_packet(self, packet: LogPacket) -> None:
        if len(packet.data) < 4:
            logger.error("Packet with less than 4 bytes of data! \n%s", packet)
            return

        address: int = build_value(packet.data[:4])

        if address not in packet.trace_db.traceDB:
            logger.error("Packet header points to %d but this address does not map to the .out file!", address)
            return

        elf_str: ElfString = packet.trace_db.traceDB[address]

        # We have used the first 32-bit word already, so strip it off
        data = packet.data[4:]

        if elf_str.opcode == Opcode.FORMATTED_TEXT:
            values = []
            while data:
                # Build the first four bytes into an int32
                values.append(build_value(data[:4]))
                # Trim the first four bytes from data
                data = data[4:]

            try:
                # Convert the list of values into a tuple so it is a valid argument to the % command
                packet._str_data = elf_str.string % tuple(values)
            except TypeError as exc:
                logger.error(
                    "Log.h elf string formatting failed: %s\nFormat string: %s, args: %s", exc, elf_str.string, values
                )
                return

        elif elf_str.opcode == Opcode.BUFFER:
            # Format buffers as a list of space-sparated hex bytes
            data_str = " ".join(["0x%x" % byte for byte in data])
            # Join them with the user's elf string
            packet._str_data = elf_str.string + data_str


# Called after all cli commands have evaluated
def logger_cli_finalizer(streams, **kwargs):
    state: LoggerCliCtx = click.get_current_context().ensure_object(LoggerCliCtx)

    input_streams = [stream for stream in streams if isinstance(stream, TransportABC)]
    output_streams = [stream for stream in streams if isinstance(stream, LogOutputABC)]
    lgr = Logger(input_streams, output_streams)
    lgr.wait_threads()


# Define CLI entry point
logger_cli = typer.Typer(chain=True, result_callback=logger_cli_finalizer)


# Use callback to collect common options
@logger_cli.callback(invoke_without_command=False)
def logger_cli_options(
    ctx: typer.Context,
    elf: List[Path] = typer.Option([], help="Symbol file path (elf/out file) shared by all input parsers"),
):
    r"""Parse LogSinkITM and LogSinkUART log output, or use replay file
    functionality to store/replay log streams.

    This tool may be used to instantiate a serial port parser for the ITM
    and UART Log Sinks. The tool receives logs generated with the Log.h API and
    reconstructs and displays the log output by using the encoded metadata in the
    toolchain-generated ELF output.

    A simple invocation (provided Wireshark is installed) is

      tilogger --elf path/to/elf.out itm COM3 12000000 wireshark --start
    """

    # Initialize parsing context
    state = LoggerCliCtx()
    ctx.obj = state
    # Add common elf files (given before subcommands) to parser context
    state.symbol_files.extend(elf)


def main():
    """
    Generates argparse parsers, reads command line arguments and starts a Logger.
    """
    # Plug in transports that are installed, add them as subcommands
    for entry_point in pkg_resources.iter_entry_points("tilogger.transport"):
        subtyper = entry_point.load()
        subtyper(logger_cli)
        # logger_cli.add_typer(subtyper, name=entry_point.name)

    # Plug in outputs that are installed, add them as subcommands
    for entry_point in pkg_resources.iter_entry_points("tilogger.output"):
        subtyper = entry_point.load()
        subtyper(logger_cli)
        # logger_cli.add_typer(subtyper, name=entry_point.name)

    logger_cli()


if __name__ == "__main__":
    main()
