# SimpleLink Instrumentation and Logging Kit

## Introduction

This subfolder contains the host-side logger tooling, used to capture and
interpret log messages. This tool supports several input/transport modules for
capturing, and modules for outputting logs. A list of available log handling
modules is displayed in the list below.

### List of Transport and Output Modules

#### Transport modules

* itm
* uart
* from-replayfile

#### Output modules

* stdout
* wireshark (requires configuration)
* to-replayfile

> **Please note there is no support for Linux in this version of the logging
> tool. Also note that some log sinks (for example, LogSinkBuf) don't require
> this host-side tool to capture and view log messages.**

## Quick Start Guide

This section will briefly explain the steps needed to setup and use the logger
tool. For more details about each transport/output module, please refer to their
respective sections in this document.

Install Python (3.8.10) from [**python.org**][python-web], and if you want to
use the Wireshark output module, install the application from
[**wireshark.org**][wireshark-web].

### Install `tilogger` command line tool

The logger modules can be installed in a local "virtualenv" so that the global
Python install is not affected, or it can simply be installed to the global
Python library.

*Note*: If you have several versions of Python installed, it is important that
the tilogger is run with the correct version (3.8). On Windows, this can be
controlled via the py launcher. To see a list of installed versions, run `py
-0p`. The instructions below will assume there are multiple versions available,
but will also work if only 3.8 is available.

#### Create and activate a venv (optional)

```powershell
# Create a virtualenv (local copy of python + packages)
$ py -3.8 -m venv .venv
# Activate virtualenv.
$ .\.venv\Scripts\activate.ps1 (or .bat)
```

When you install to a virtualenv you must `activate` it again each time you open
a new command line terminal window.

#### Installing the `tilogger` command line interface tool

If you are behind a proxy, you need to either have this configured in the global
environment variables `http_proxy` and `https_proxy` (and `no_proxy`), or you
can add `--proxy yourproxy.com` after `install` in the `pip` calls below.

Install tilogger in one of the following ways:

##### Installing the tilogger tool in the virtual environment

If you are using a virtual environment as created above, the py launcher should
not be used to install tilogger. Use instead the following command

```powershell
# Install the tilogger tool in the virtual environment created above
$ python3 -m pip install [--proxy yourproxy.com] -r requirements.txt
```

##### Installing the tilogger tool globally

If you wish to install the tilogger tool globally, change directory to
`<SDK_INSTALL_DIR>/tools/log/tiutils/`, and use the following command

```powershell
# Install the tilogger tool in the virtual environment created above (optional)
$ py -3.8 -m pip install [--proxy yourproxy.com] -r requirements.txt
```

If you install globally, `python` must be available on PATH for these scripts to
work, and the `Scripts` folder for the Python version you use to install must
also be on the PATH.

After installing you can use the `tilogger` command. Try it!

### Using the `tilogger` command line interface tool

The most up to date documentation about the options and commands is available by
running `tilogger --help`. Running `tilogger <command> --help` will display help
about the command, for example `tilogger uart --help`.

```text
$ tilogger --help
Usage: tilogger [OPTIONS] COMMAND1 [ARGS]... [COMMAND2 [ARGS]...]...

  Parse LogSinkITM and LogSinkUART log output, or use replay file
  functionality to store/replay log streams.

  This tool may be used to instantiate a serial port parser for the ITM and
  UART Log Sinks. The tool receives logs generated with the Log.h API and
  reconstructs and displays the log output by using the encoded metadata in
  the toolchain-generated ELF output.

  A simple invocation (provided Wireshark is installed) is

    tilogger --elf path/to/elf.out itm COM3 12000000 wireshark --start

Options:
  --elf PATH            Symbol file path (elf/out file) shared by all input
                        parsers  [default: ]

  --install-completion  Install completion for the current shell.
  --show-completion     Show completion for the current shell, to copy it or
                        customize the installation.

  --help                Show this message and exit.

Commands:
  from-replayfile  Replay a sequence of logs from a replay file.
  itm              Add ITM transport as input to log.
  stdout           Add log output as standard output stream.
  to-replayfile    Store log outputs to a replay file.
  uart             Add UART transport as input to log.
  wireshark        Add Wireshark to log outputs.
```

#### Extracting Logs

The tools must be started in sequence:

* Start a terminal session in the `<SDK_INSTALL_DIR>/tools/log/tiutils/` folder
* If using a virtual environment, run `.venv/Scripts/activate` to get into the
  virtual environment
* Run `tilogger [transport cmd] [transport arguments] [output cmd] [output
  module]` to launch the logger tool with specified transport(s) and output(s).
    * Example for UART to stdout: `> tilogger uart --elf myelf.out COM4 3000000 stdout`
* Flash your log-enabled firmware and reset the device.

You should now be capturing and interpreting log messages with the specified
transport and output modules.

> **Please refer to each module’s respective sections in this document for
> specific considerations.**

#### ELF Files Specification

Non-logging related sections from your elf.out file may be removed as they may
contain sensitive information. To do this, use the utilities found in your
toolchain to run the appropriate command below:

* GCC
   ```arm-none-eabi-objcopy --only-section=.log_data --only-section=.log_ptr input.out output.out```
* TICLANG and IAR projects
   ```tiarmobjcopy --only-section=.log_data --only-section=.log_ptr input.out output.out```

You can provide ELF files containing Log-string information either globally (to
all transports you add) if you have just one device or if the devices run the
same firmware, and you can provide the ELF files to each transport individually.

#### Examples

Below are more examples for launching the logger tool. These examples use uart,
but itm transport is invoked in the same way.

* Listen for UART input and pipe output to stdout.
    * `tilogger --elf myelf.out uart --alias BestDevice COM4 3000000 stdout`
* Listen for UART input and pipe output to wireshark.
    * `tilogger --elf myelf.out uart --alias BestDevice COM4 3000000 wireshark`
* Listen for UART input and pipe output to wireshark, trying to start wireshark
  and configure it to understand the tilogger protocol.
    * `tilogger --elf myelf.out uart --alias BestDevice COM4 3000000 wireshark --start`
* Listen for ITM and UART inputs and pipe outputs to stdout.
    * `tilogger itm --elf myelf.out COM3 12000000 uart --elf myelf.out COM4 3000000 stdout`
* Start two UART listeners, each using a different log statement database, and
  both will be logging to the wireshark instance that is automatically
  started.
    * `tilogger uart --alias dev0 --elf dev0.out COM3 3000000 uart --alias dev1 --elf dev1.out COM4 3000000 wireshark --start`

## Transport Modules

### ITM Transport

```text
$ tilogger itm --help
Usage: tilogger itm [OPTIONS] PORT BAUDRATE

  Add ITM transport as input to log.

  You need to specify a serial port and a baudrate (ITM driver module
  configures this). The baudrate is often quite high, such as 12000000
  (12MHz).

  You also need to specify .out/.elf files that contain symbol information
  needed to parse the log, but this may also be provided globally before
  adding transports.

Arguments:
  PORT      Serial port (eg COM12)  [required]
  BAUDRATE  TPIU baudrate  [required]

Options:
  --elf PATH    Symbol file path (elf/out file)  [default: ]
  --alias TEXT  Alias for this device in the log
  --help        Show this message and exit.
```

#### *ITM Transport Considerations*

* Use Device Manager to choose the correct COM port.
  > Be aware that when when using XDS110 Debug Probe you must use the "Auxiliary
  > Data Port".
* `tilogger` will not begin recording until it has seen a reset frame, which is
  only generated on startup.

#### Viewing RAW ITM Streams

The `tilogger_itm_transport` package also provides a raw ITM viewer, which can
for example support the use of TI's `ITM` driver directly. Follow the setup
guidelines above to create an environment and install the ITM package. You can
ignore any instructions for Wireshark.

The command line for the ITM viewer is `> tilogger_itm_viewer [COM PORT] [BAUDRATE]`.
Example: `> tilogger_itm_viewer COM5 115200`

[python-web]: https://www.python.org/downloads/
[wireshark-web]: https://www.wireshark.org/download.html

### UART Transport

```text
$ tilogger uart --help
Usage: tilogger uart [OPTIONS] PORT BAUDRATE

  Add UART transport as input to log.

  You need to specify a serial port and a baudrate (UART LogSink module
  configures this). The baudrate is often quite high, such as 3000000
  (3 MHz).

  You also need to specify .out/.elf files that contain symbol information
  needed to parse the log, but this may also be provided globally before
  adding transports.

Arguments:
  PORT      Serial port (eg COM11)  [required]
  BAUDRATE  UART baudrate  [required]

Options:
  --elf PATH    Symbol file path (elf/out file)  [default: ]
  --alias TEXT  Alias for this device in the log
  --help        Show this message and exit.
```

#### *UART Transport Considerations*

* Use Device Manager to choose the correct COM port.
  > Be aware that when when using XDS110 Debug Probe you must use the
  > "Application/User UART".
* Waiting for a reset frame is not necessary for UART transport, in this case
  `tilogger` can attach to a running data stream.

### From Replay File Transport

```text
$ tilogger from-replayfile --help
Usage: tilogger from-replayfile [OPTIONS] FILE

  Replay a sequence of logs from a replay file.

  You need to specify path to a replay file (JSON file) and an output mode
  (stdout or wireshark).

  A simple invocation that loads from replay file and outputs to stdout is

      tilogger from-replayfile path/to/replayFile.json stdout

Arguments:
  FILE  Replay file path (JSON file)  [required]

Options:
  --help  Show this message and exit.
```

#### *Replay File Transport Considerations*

* Providing an `--elf` argument is not necessary when using replay file
  transport. The replay file already contains the parsed log messages.

## Output Modules

### Stdout Output

```text
$ tilogger stdout --help
Usage: tilogger stdout [OPTIONS]

  Add log output as standard output stream.

  If custom format is wanted, use --format "<FORMAT_SPEC>" with desired
  format specifiers.

  Supported Specifiers
  {I}: Identifier (for example COM7, or the provided alias)
  {T}: Timestamp
  {M}: Module  (for example LogMod_App1)
  {L}: Log level (for example Log_INFO)
  {F}: File and line number (for example path/log.c:134)
  {D}: Log data

Options:
  -f, --format TEXT           [default: "{I} | {T} | {M} | {L} | {F} | {D}"]
  --disable-column-alignment  Disable dynamic column alignment
  --help                      Show this message and exit.
```

### Wireshark Output

>**Configuration of Wireshark is needed for the logs to be displayed correctly. See [Configure Wireshark](#configure-wireshark).**

```text
$ tilogger wireshark --help
Usage: tilogger wireshark [OPTIONS]

  Add Wireshark to log outputs.

  If you don't want to start wireshark via this tool you must configure it
  manually

  Open Capture -> Options -> Manage Interfaces -> Pipes -> +, then type in
  `\\.\pipe\tilogger-wireshark` or your own choice of pipe name.

  Add protocol parsing via  Edit -> Preferences -> Protocols -> DLT_USER ->
  Edit..., then add new row for DLT=147 and use `tilogger` as payload
  protocol.

Options:
  -s, --start  [default: False]
  --pipe TEXT  [default: \\.\pipe\tilogger-wireshark]
  --help       Show this message and exit.
```

#### Configure Wireshark

Copy `streams/wireshark/tilog_dissector.lua` to `[Wireshark install
dir]/plugins/x.y/tilogger_dissector.lua`. This will teach Wireshark to
understand the TI Logger's packet format.

>*Warning*: If you have previously
used an older version of the TI logger tool, make sure to remove the previous
`dissector.lua` containing the old TI Logger from Wireshark.

To automatically start wireshark when starting the logger, supply `--start` to
the `wireshark` command. `tilogger` adds columns to the default view for
Filename, Line-number, Level, Module and Format string. To modify these or add
new columns, right click the column headers and choose `Column Preferences`.
Note that the field type should be 'Custom'.

This has been tested with Wireshark 3.4.x, but should work with later versions
as well.

### To Replay File Output

```text
$ tilogger to-replayfile --help
Usage: tilogger to-replayfile [OPTIONS]

  Store log outputs to a replay file.

  You need to specify path to a JSON file. A replay file will be generated
  by storing the captured LogPackets in the specified JSON file. If a path
  to an existing JSON file is provided, the file will be overwritten.

  A invocation storing an ITM stream parsed with the provided ELF file to
  replayfile.json:

      tilogger --elf path/to/elf.out itm COM6 12000000 to-replayfile --file
      path/to/replayfile.json

  To replay the log events, use

      tilogger from-replayfile <FILE> <OUTPUT>

Options:
  --file TEXT  Replay file path (JSON file)  [required]
  --verbose    Print updates in terminal while writing to file
  --help       Show this message and exit.
```

## User format modules

User format modules are functional in this beta, but there are no examples
provided. You will need to create a Python package with a class that inherits
from one of the abstract base classes (ABCs) in `core/tilogger/interface.py`.
Your package must register an entry point (this is documented in the interface
classes). If placed in a subfolder of modules/, it will be installed into the
virtual environment automatically when you call one of the setup scripts.
