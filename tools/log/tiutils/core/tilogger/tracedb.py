import os
import coloredlogs, logging
import pickle
import hashlib
import json
import enum
import struct
import io

from pathlib import Path
from typing import Dict, Optional, List
from appdirs import AppDirs, user_data_dir
from elftools.dwarf.descriptions import describe_form_class
from elftools.elf.elffile import ELFFile, Section

from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

import threading

# Set up logger
logger = logging.getLogger("TraceDB")

TRACE_SECTION_NAMES = [".log_data", ".log_ptr"]


class Opcode(enum.Enum):
    FORMATTED_TEXT = 0
    BUFFER = 1
    RESET = 2
    REPLAY_FILE = 3


# String to opcode dictionary
log_string_to_opcode = {
    "LOG_OPCODE_FORMATED_TEXT": Opcode.FORMATTED_TEXT,
    "LOG_OPCODE_BUFFER": Opcode.BUFFER,
}


class ElfString:
    def __init__(self, value: str, trace_db: "TraceDB"):
        # Get opcode and strip from value
        self.value = value
        opcode_str, value = value.split("\x1e", 1)

        try:
            self.opcode: Opcode = log_string_to_opcode[opcode_str]
        except KeyError:
            logger.error("ElfString cannot be instantiated with unknown opcode %s", opcode_str)

        self.trace_db = trace_db

        self.file: str
        self.line: str
        self.event: str
        self.string: str
        self.module_id: str
        self.nargs: int
        self.level: str

        if self.opcode == Opcode.FORMATTED_TEXT:
            self.file, self.line, self.level, self.module_id, self.string, tmp_nargs = value.split("\x1e")

        elif self.opcode == Opcode.BUFFER:
            # Note that nargs == 0 for BUFFER strings - length is only known at runtime
            self.file, self.line, self.level, self.module_id, self.string, tmp_nargs = value.split("\x1e")

        else:
            raise Exception(f"Unexpected opcode {self.opcode}")

        self.nargs = int(tmp_nargs)

    def __getstate__(self):
        d = dict(self.__dict__)
        del d["trace_db"]
        return d


class ElfWatcherHandler(FileSystemEventHandler):
    def __init__(self, elfpaths: List[Path], event: threading.Event) -> None:
        self.elfpaths = elfpaths
        self.event = event
        super().__init__()

    def on_any_event(self, event):
        path = Path(event.src_path)
        if event.event_type in ["modified", "created"]:
            if any((ep == path for ep in self.elfpaths)):
                logger.warn(f"Change detected in {path.parts[-1]}, marking for rescan on next lookup")
                self.event.set()


class TraceDB:
    def __init__(self, elves, repickle: bool):
        self.elves = [Path(elf) for elf in elves]
        self.device = ""
        self._traceDB = {}
        self._eventDB = {}
        self.timestamp_fmt_32 = b""
        self.timestamp_fmt_64 = b""
        self.stringpointers = {}

        self.changed_event = threading.Event()
        self.change_handler = ElfWatcherHandler(self.elves, self.changed_event)
        self.change_observers = [Observer() for _ in range(len(self.elves))]
        for idx, observer in enumerate(self.change_observers):
            observer.schedule(self.change_handler, path=str(self.elves[idx].parent))
            observer.start()

        # Locate and create %AppData% directories
        dirs = AppDirs("logger", "tilogger")
        self.user_data_dir = Path(dirs.user_data_dir)
        self.user_data_dir.mkdir(parents=True, exist_ok=True)

        self.init_db(repickle)

    def init_db(self, repickle=True):
        # Clear existing info
        self._traceDB = {}
        self._eventDB = {}
        self.timestamp_fmt_32 = b""
        self.timestamp_fmt_64 = b""
        self.stringpointers = {}

        # Build current hash
        hasher = hashlib.md5()
        for elf in self.elves:
            try:
                hasher.update(elf.read_bytes())
            except FileNotFoundError as e:
                logger.error("Not able to open elf file " + elf.as_uri())
                raise e
        current_hash = hasher.hexdigest()

        trace_db_pickle_file = (self.user_data_dir / f"{current_hash}.trace_db.pkl").resolve()
        event_db_pickle_file = (self.user_data_dir / f"{current_hash}.event_db.pkl").resolve()
        timestamp_fmt_32_pickle_file = (self.user_data_dir / f"{current_hash}.timestamp_fmt_32.pkl").resolve()
        timestamp_fmt_64_pickle_file = (self.user_data_dir / f"{current_hash}.timestamp_fmt_64.pkl").resolve()

        build_trace_db = False
        if not trace_db_pickle_file.exists():
            build_trace_db = True

        # Build and pickle databases if needed
        if build_trace_db is False:
            # Load from pickled trace file, add back unpicklable reference to self
            try:
                self._traceDB = pickle.loads(trace_db_pickle_file.read_bytes())
                for elfstring in self._traceDB:
                    self._traceDB[elfstring].trace_db = self

                self._eventDB = pickle.loads(event_db_pickle_file.read_bytes())
                for elfstring in self._eventDB:
                    self._eventDB[elfstring].trace_db = self

                self.timestamp_fmt_32 = pickle.loads(timestamp_fmt_32_pickle_file.read_bytes())
                self.timestamp_fmt_64 = pickle.loads(timestamp_fmt_64_pickle_file.read_bytes())

                logger.info("Pickled TraceDB, EventDB, and TimestampFormat have been successfully loaded")
            except Exception as e:
                logger.error(e)
                build_trace_db = True  # Build anyway

        if build_trace_db is True or repickle is True:
            # Build elf information
            for elf in self.elves:
                self.parse_elf(elf)

            # Pickle trace database
            with open(trace_db_pickle_file, "wb") as f:
                pickle.dump(self._traceDB, f)
            # Pickle event database
            with open(event_db_pickle_file, "wb") as f:
                pickle.dump(self._eventDB, f)
            # Pickle timestamp format 32 information
            with open(timestamp_fmt_32_pickle_file, "wb") as f:
                pickle.dump(self.timestamp_fmt_32, f)
            # Pickle timestamp format 64 information
            with open(timestamp_fmt_64_pickle_file, "wb") as f:
                pickle.dump(self.timestamp_fmt_64, f)
            logger.info("TraceDB, EventDB, and TimestampFormat have been pickled")
        logger.info("Done configuring databases")

    @property
    def traceDB(self):
        if self.changed_event.wait(0):
            self.changed_event.clear()
            self.init_db()
        return self._traceDB

    @property
    def eventDB(self):
        if self.changed_event.wait(0):
            self.changed_event.clear()
            self.init_db()
        return self._eventDB

    def parse_elf(self, elfpath: Path):
        elf = ELFFile(io.BytesIO(elfpath.read_bytes()))
        # Find LOG sections
        trace_secs: List[Section] = []
        for secnum, sec in enumerate(elf.iter_sections()):
            if any([x in sec.name for x in TRACE_SECTION_NAMES]):
                sect = elf.get_section(secnum)
                trace_secs.append(sect)

        if len(trace_secs) == 0:
            raise ValueError(
                "Trace sections not found in elf file. Ensure that the linker file is correct and that "
                "there is at least one module and level enabled."
            )

        logger.info("Parsing elf-file...")

        def find_sect(address):
            for sect in trace_secs:
                if sect.header["sh_addr"] <= address <= sect.header["sh_addr"] + sect.header["sh_size"]:
                    return sect.header["sh_addr"], sect.header["sh_size"], sect.header["sh_offset"], sect
            return 0, 0, 0, None

        def extract_symbol_value(symbol):
            sect_base, sect_size, sect_offset, sect = find_sect(symbol.entry.st_value)
            # Find offset into section by subtracting section base address
            offset = sect_offset + (sym.entry.st_value - sect_base)
            # Seek to offset in ELF
            sect.stream.seek(offset)
            # Read until end of section
            assert symbol.entry.st_size > 0, f"ERROR: Linker has stored {symbol.name} with size 0, can't read"
            value = sect.stream.read(symbol.entry.st_size)
            return value

        # Build LOG trace database by searching in symbol table
        for sym in elf.get_section_by_name(".symtab").iter_symbols():
            sect_base, sect_size, sect_offset, sect = find_sect(sym.entry.st_value)
            if not sect:
                continue

            if "TimestampP_nativeFormat32_copy" in sym.name:
                value = extract_symbol_value(sym)
                self.timestamp_fmt_32 = value
                logger.debug(f"{sym.name} = 0x{value.hex()}")
            elif "TimestampP_nativeFormat64_copy" in sym.name:
                value = extract_symbol_value(sym)
                self.timestamp_fmt_64 = value
                logger.debug(f"{sym.name} = 0x{value.hex()}")
            elif "Ptr_LogSymbol_" in sym.name:
                value = extract_symbol_value(sym)  # Find what it points to
                sym_addr = struct.unpack("I", value)[0]
                self.stringpointers[sym_addr] = sym.entry.st_value
                logger.debug(f"{sym.name} -> 0x{sym_addr : 08X}")
                if sym_addr in self._traceDB:
                    # Make pointer address point to "elf string" if it is already found
                    self._traceDB[sym.entry.st_value] = self._traceDB[sym_addr]

            elif "LogSymbol_" in sym.name:
                value = extract_symbol_value(sym)
                # Truncate output at null character and remove quotes
                value = value.decode("utf-8").split("\0")[0].replace('"', "")
                # Create new ElfString to store in dictionary
                elf_string = ElfString(value, self)
                # Add to database
                self._traceDB[sym.entry.st_value] = elf_string
                if sym.entry.st_value in self.stringpointers:
                    # If is already found, update so that traceDB lookup for pointer resolves to this.
                    self._traceDB[self.stringpointers[sym.entry.st_value]] = elf_string

                logger.debug("0x%x --> %s", sym.entry.st_value, value.replace("\x1e", ", "))


if __name__ == "__main__":
    coloredlogs.install(level="DEBUG", fmt="%(asctime)s %(name)s[%(process)d] %(levelname)8s %(message)s")
    db = TraceDB(["your_out_file_path"], True)  # May be used to test tracedb changes and observe logs
