# Introduction

This subfolder contains the host-side logger tooling, used to capture and
interpret log messages and send them to Wireshark to be displayed. If you would
just like to view arbitrary packets in an ITM stream, see the section at the end
of this document.

Please note there is no support for Linux in this version of the logging tool.

# Quick Start Guide

Install Python (3.8.10) from [__python.org__][python-web] and Wireshark
from [__wireshark.org__][wireshark-web].

There are three steps to displaying logs: installing the Python modules using
the setup script, configuring Wireshark to display the logs correctly, and
running the tools.

## Install `tilogger` command line tool

These modules can be installed in a local "virtualenv" so that the global Python
install is not affected, or it can simply be installed to the global Python
library.

*Note*: If you have several versions of Python installed, it is important that
the tilogger is run with the correct version (3.8). On Windows, this can be controlled
via the py launcher. To see a list of installed versions, run `py -0p`. The instructions
below will assume there are multiple versions available, but will also work if only 3.8
is available.

### Create and activate a venv (optional)

```bash
# Create a virtualenv (local copy of python + packages)
$ py -3.8 -m venv .venv
# Activate virtualenv.
$ .\.venv\Scripts\activate.ps1 (or .bat)
```

When you install to a virtualenv you must `activate` it again each time you open
a new command line terminal window.

### Installing the `tilogger` command line interface tool

If you are behind a proxy, you need to either have this configured in the global
environment variables `http_proxy` and `https_proxy` (and `no_proxy`), or you
can add `--proxy yourproxy.com` after `install` in the `pip` calls below.

#### Installing the tilogger tool in the virtual environment
If you are using a virtual environment as created above, the py launcher should not be used to install tilogger.
Use instead the following command
```bash
# Install the tilogger tool in the virtual environment created above
$ python3 -m pip install [--proxy yourproxy.com] -r requirements.txt
```

#### Installing the tilogger tool globally
If you wish to install the tilogger tool globally, use the following command
```bash
# Install the tilogger tool in the virtual environment created above (optional)
$ py -3.8 -m pip install [--proxy yourproxy.com] -r requirements.txt
```
If you install globally, `python` must be available on PATH for these scripts to
work, and the `Scripts` folder for the Python version you use to install must
also be on the PATH.

After installing you can use the `tilogger` command. Try it!

## Configure Wireshark

Copy `streams/wireshark/tilog_dissector.lua` to `[Wireshark install
dir]/plugins/x.y/tilogger_dissector.lua`. This will teach Wireshark to
understand the TI Logger's packet format. *Warning*: If you have previously
used an older version of the TI logger tool, make sure to remove the previous
`dissector.lua` containing the old TI Logger from Wireshark.

To automatically start wireshark when starting the logger, supply `--start` to the
`wireshark` command.

This has been tested with Wireshark 3.4.x, but should work with later versions as well.

## Using the `tilogger` command line interface tool

The most up to date documentation is available by running `tilogger --help` to
see what the available options and commands are, and by running `tilogger
<command> --help` to get help about the command - for example `tilogger itm
--help`.

```
$ tilogger --help
Usage: tilogger [OPTIONS] COMMAND1 [ARGS]... [COMMAND2 [ARGS]...]...

  Parse LogSinkITM and LogSinkUART log output.

  This tool may be used to instantiate a serial port parser for the ITM
    and UART Log Sinks. The tool receives logs generated with the Log.h API and
    reconstructs and displays the log output by using the encoded metadata in the
    toolchain-generated ELF output.

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
  itm        Add ITM transport as input to log.
  uart       Add UART transport as input to log.
  wireshark  Add Wireshark to log outputs.
```

You can provide ELF files containing Log-string information either globally (to
all transports you add) if you have just one device or if the devices run the
same firmware, and you can provide the ELF files to each transport individually.

Examples:
* `tilogger --elf myelf.out itm --alias BestDevice COM3 12000000`
  * Starts the logger and connects to COM3 at 12Mbps, but no output is specified
* `tilogger --elf myelf.out itm --alias BestDevice COM3 12000000 wireshark`
  * Will in addition start the wireshark output and try to connect to the
    default pipe
* `tilogger --elf myelf.out itm --alias BestDevice COM3 12000000 wireshark --start`
  * Will also try to start wireshark and configure it to understand the tilogger protocol.
* `tilogger itm --alias dev0 --elf dev0.out COM3 12000000 itm --alias dev1 --elf dev1.out COM4 12000000 wireshark --start`
  * Starts two ITM listeners, each using a different log statement database, and
    both will be logging to the wireshark instance that is automatically
    started.

### ITM transport help

```
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

### UART transport help

```
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

### Wireshark output help

```
$ tilogger wireshark --help
Usage: tilogger wireshark [OPTIONS]

  Add Wireshark to log outputs.

  If you don\'t want to start wireshark via this tool you must configure it
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

## Extracting some Logs

The tools must be started in sequence:
 - Start a terminal session in the `tools/log/tiutils/` folder
 - `> .venv/Scripts/activate` to get into the virtual environment
 - `> tilogger itm --elf [out_file] [COM_PORT] 12000000` to launch the logger tool,
   scan your .out file for log items and wait for Wireshark to pick up the pipe.
   Use Device Manager to choose the correct COM port and be aware that you must
   use the "Auxiliary Data Port" not the "Application/User UART".
 - Flash your logged firmware or reset the device. `tilogger` will not begin
   recording until it has seen a reset frame, which is currently only generated
   on startup.

Start wireshark via `tilogger wireshark --start`, which adds columns to the default
view for Filename, Line-number, Level, Module and Format string. To modify these or add new columns,
right click the column headers and choose `Column Preferences`. Note that the
field type should be 'Custom'.

*Note*: The above can be achieved with one single command:
`> tilogger itm --elf [out_file] [COM_PORT] 12000000 wireshark --start`

If all goes well, your logs should begin to appear in the Wireshark window.



## User format modules
User format modules are functional in this beta, but there are no examples
provided. You will need to create a Python package with a class that inherits
from one of the ABCs in tilogger.interface. Your package must register an entry
point (this is documented in the interface classes). If placed in a subfolder of
modules/, it will be installed into the virtual environment automatically when
you call one of the setup scripts.

# Viewing RAW ITM Streams
The `tilogger_itm_transport` package also provides a raw ITM viewer, which can
for example support the use of TI's `ITM` driver directly. Follow the setup
guidelines above to create an environment and install the ITM package. You can
ignore any instructions for Wireshark.

The command line for the ITM viewer is `> tilogger_itm_viewer [COM PORT] [BAUDRATE]`.
Example: `> tilogger_itm_viewer COM5 115200`

[python-web]: https://www.python.org/downloads/
[wireshark-web]: https://www.wireshark.org/download.html
