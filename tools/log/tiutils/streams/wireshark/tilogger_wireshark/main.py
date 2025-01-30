from pathlib import Path
import struct
import logging
import platform
from struct import pack
from threading import local
import typing
import pkg_resources
import argparse
import win32pipe
import win32file
import typer
import click
import subprocess
import os
import sys
import time

from typing import DefaultDict, Dict, List
from collections import defaultdict

from tilogger.interface import LogOutputABC, LogPacket, LoggerCliCtx, TransportABC, LogFormatterABC, LogSubscriberABC
from tilogger.tracedb import Opcode, TraceDB, ElfString


logger = logging.getLogger("Wireshark Output")


def _build_value(buf):
    """
    Helper function: Turn an iterable into a little-endian integer
    """
    value = 0
    for idx, val in enumerate(buf):
        value += val << (idx * 8)
    return value


WIRESHARK_PIPE_NAME = "\\\\.\\pipe\\tilogger-wireshark"


class WiresharkOutput(LogOutputABC):
    PCAP_MAGIC_NUMBER: int = 0xA1B2C3D4
    PCAP_MAJOR_VER: int = 2
    PCAP_MINOR_VER: int = 4
    PCAP_LINK_TYPE: int = 147

    def __init__(self, ws_pipe):
        self.ws_pipe = ws_pipe
        logger.info("Waiting for Wireshark to be ready..")
        self._pipe = None
        self._pipe_inited = False
        self._pipe_backlog = []
        self._create_try_connect_pipe()

    def start(self):
        start_wireshark(self.ws_pipe)
        # Try flushing backlog until Pipe is initialized
        while not self._pipe_inited:
            self._try_connect_backlog()
            time.sleep(0.5)  # Sleep to reduce CPU usage

    def notify_packet(self, packet: LogPacket) -> None:
        self._send(packet)

    def _send(self, packet: LogPacket):
        if packet.timestamp:
            timestamp = packet.timestamp
            timestamp_seconds = int(timestamp)
            timestamp_microsec = int(1000000 * (timestamp - timestamp_seconds))
        else:
            timestamp_seconds = 0
            timestamp_microsec = 0

        columns = f"{packet.alias}||{packet.timestamp_local:0.9f}||{str(Opcode(packet.opcode).name)}||{packet.module}||{str(packet.level.name)}||{str(packet.filename)}||{str(packet.lineno)}||{packet._str_data}"
        payload = columns.encode("utf-8")
        header = struct.pack("IIII", timestamp_seconds, timestamp_microsec, len(payload), len(payload))

        data = header + payload
        self._send_internal(data)

    def _try_connect_backlog(self):
        if not self._pipe_inited:
            if platform.system() == "Windows":
                try:
                    clientpid = win32pipe.GetNamedPipeClientProcessId(self._pipe)
                    self._connect_send_initial_pipe()
                    for x in self._pipe_backlog:
                        win32file.WriteFile(self._pipe, x)
                    self._pipe_backlog = []
                except Exception:
                    pass

    def _send_internal(self, data: bytes):
        if platform.system() == "Windows":
            if not self._pipe_inited:
                try:
                    clientpid = win32pipe.GetNamedPipeClientProcessId(self._pipe)
                    # Assume exception happens above if pipe not yet open. If open, assume below works.
                    self._connect_send_initial_pipe()
                    for x in self._pipe_backlog:
                        win32file.WriteFile(self._pipe, x)
                    win32file.WriteFile(self._pipe, data)
                except Exception:
                    logger.warn("Could not send to Wireshark - packet stored in backlog")
                    self._pipe_backlog.append(data)

            else:
                win32file.WriteFile(self._pipe, data)
        else:
            raise NotImplementedError("No support for Linux in this beta")

    def _create_try_connect_pipe(self):
        # pipename should be of the form \\.\pipe\mypipename
        self._pipe = win32pipe.CreateNamedPipe(
            WIRESHARK_PIPE_NAME,
            win32pipe.PIPE_ACCESS_OUTBOUND,
            win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_WAIT,
            1,
            65536,
            65536,
            300,
            None,
        )

        try:
            clientpid = win32pipe.GetNamedPipeClientProcessId(self._pipe)
            self._connect_send_initial_pipe()
        except Exception:
            pass

    def _connect_send_initial_pipe(self):
        self._pipe_inited = True
        win32pipe.ConnectNamedPipe(self._pipe, None)

        # Header format (rows are 32-bit words):
        # Magic number
        # Major (16b) and minor (16b) numbers
        # Reserved write 0
        # Reserved write 0
        # Max Packet Length, in bytes
        # LinkType

        header = struct.pack(
            "IHHIIII", self.PCAP_MAGIC_NUMBER, self.PCAP_MAJOR_VER, self.PCAP_MINOR_VER, 0, 0, 256, self.PCAP_LINK_TYPE
        )
        self._send_internal(header)

    def _close(self):
        logger.info("Closing pipe")
        win32file.CloseHandle(self._pipe)


def find_wireshark():
    alternatives = [
        Path(os.getenv("%PROGRAMFILES(x86)%", "")) / "Wireshark/Wireshark.exe",
        Path(os.getenv("ProgramFiles", "")) / "Wireshark/Wireshark.exe",
    ]

    for path in alternatives:
        if path.exists():
            return str(path.resolve())

    envpath = os.getenv("WIRESHARK_EXE_PATH", None)
    if envpath:
        fullpath = Path(envpath)
        if fullpath.is_file() and fullpath.exists():
            return str(fullpath.resolve())
        else:
            typer.secho(f"WIRESHARK_EXE_PATH => {envpath} does not exist or is not a file", fg=typer.colors.BRIGHT_RED)
            sys.exit(1)
    else:
        typer.secho(
            f"Could not find wireshark. If it is installed, make an environment variable `WIRESHARK_EXE_PATH` with the path",
            fg=typer.colors.BRIGHT_RED,
        )
        sys.exit(1)


def start_wireshark(ws_pipe):
    if ws_pipe is None:
        return

    ws_exec = [
        find_wireshark(),
        # Begin trace immediately
        "-k",
        # Use the specified interface (here, a named pipe)
        "-i",
        ws_pipe,
        # Modify option uat:user_dlts
        "-o",
        # Tell Wireshark that protocol 147 (USER_0) should be handled by tilogger dissector
        # "uat:user_dlts:\"User N (DLT=[N+147])\",\"payload protocol\",\"header size\",\"header protocol\",\"trailer size\",\"trailer protocol\""
        'uat:user_dlts:"User 0 (DLT=147)","tilogger","0","","0",""',
        # Modify gui columns
        "-o",
        # Add the tilogger fields as columns - each column specified as a tuple of name, type, name+1, type+1.
        'gui.column.format:"No.", "%m", "Device alias", "%Cus:tilogger.alias:0:R", "Time", "%t", "Device Time", "%Cus:tilogger.ts_local:0:R", "Level", "%Cus:tilogger.level:0:R", "Module", "%Cus:tilogger.module:0:R", "File", "%Cus:tilogger.file:0:R", "Line", "%Cus:tilogger.line:0:R", "String", "%Cus:tilogger.string:0:R"',
    ]

    subprocess.Popen(ws_exec)


# Function that adds a command to a typer instance via decorator
def output_factory_cli(app: typer.Typer):
    @app.command(name="wireshark")
    def output_factory_cb(
        ctx: typer.Context,
        start_wireshark: bool = typer.Option(False, "--start", "-s"),
        ws_pipe: str = typer.Option(WIRESHARK_PIPE_NAME, "--pipe"),
    ):
        r"""Add Wireshark to log outputs.

        If you don't want to start wireshark via this tool you must configure
        it manually

        Open Capture -> Options -> Manage Interfaces -> Pipes -> +, then type in
        `\\.\pipe\tilogger-wireshark` or your own choice of pipe name.

        Add protocol parsing via  Edit -> Preferences -> Protocols -> DLT_USER ->
        Edit..., then add new row for DLT=147 and use `tilogger` as payload
        protocol.
        """

        state = ctx.ensure_object(LoggerCliCtx)
        return WiresharkOutput(ws_pipe=ws_pipe if start_wireshark else None)


#### Local usage below


def itm_parser_main():
    # Local finalizer for command results, not exported.
    def local_cli_finalizer(transports: List[WiresharkOutput]):
        for t in transports:  # transports returned by each subcommand
            t.start()

    local_cli = typer.Typer(name="wireshark", result_callback=local_cli_finalizer, chain=True)
    transport_factory_cli(local_cli)
    local_cli()
