# rfDiagnostics

---

Project Setup using the System Configuration Tool (SysConfig)
-------------------------
The purpose of SysConfig is to provide an easy to use interface for configuring drivers, RF stacks, and more. The .syscfg file provided with each example
project has been configured and tested for that project. Changes to the .syscfg file may alter the behavior of the example away from default. Some parameters configured in SysConfig may require the use of specific APIs or additional modifications in the application source code. More information can be found in SysConfig by hovering over a configurable and clicking the question mark (?) next to it's name.

Example Summary
---------------
Some of the most common Proprietary and BLE API commands has been exposed over an AT Command Interface such that it can be exercised by Host SW (running on an PC, MPU or MCU) or by a human using a serial terminal emulator.

Peripherals Exercised
---------------------
* `CONFIG_DISPLAY_UART` - Used to access the AT interface from a host serial session

Resources & Jumper Settings
---------------------------
> If you're using an IDE (such as CCS or IAR), please refer to Board.html in your project directory for resources used and board-specific jumper settings. Otherwise, you can find Board.html in the directory &lt;SDK_INSTALL_DIR&gt;/source/ti/boards/&lt;BOARD&gt;.

Board Specific Settings
-----------------------
1. If the example is initialized to the default mode, i.e. the custom setting,
the center frequency is set to:
    - 433.92 MHz for the CC1352P-4-LAUNCHXL
    - 868.0 MHz for other launchpads
In order to change frequency, use SysConfig

Example Usage
-------------
Open a serial session (e.g. HyperTerminal,puTTY, etc.) to the appropriate COM port. Note: the COM port can be determine via Device Manager in Windows or via ls /dev/tty* in Linux.

The connection should have the following settings:

    Baud-rate:    115200
    Data bits:    8
    Stop bits:    1
    Parity:       None
    Flow Control: None

By default the target echoes back any character that is typed in the serial session.

Note: Two boards are recommended in order to see the effect of these commands


AT API
-------------

The AT Command Interface uses ASCII characters so that a terminal emulator can send the commands, but also uses framing so that SW can format and parse the AT commands.

The frame format is shown below

    |----------------|-----------------------------|-------------|---------------------------|---------------|--------------|
    | Start of Frame | Command Type                | Command ID  | Get/Set Parameter         | Parameters    | End Of Frame |
    |                |                             |             | (parameter command only)  |               |              |
    |================|=============================|=============|===========================|===============|==============|
    | 'AT' / 'at'    | '+'       (Control Command) |             |  '=' (set)                |               |  '\n' or     |
    |                | 'P' / 'p' (Parameter)       |             |  '?' (get)                |               |  '\r'        |
    |----------------|-----------------------------|-------------|---------------------------|---------------|--------------|

The AT Command Interface uses 2 command types:

1. +x: Control Commands
1. Pxx: Parameters

__AT Control Commands__


    |----------|------------------------------------|-----------------------------------------------------|-----------------------------------------------------|
    | Command  | Description                        | Parameter(s)                                        | Example                                             |
    |==========|====================================|=====================================================|=====================================================|
    |   I/i    | Initialize the Radio               | 2.4GHz-only devices                                 | Initialize Proprietary 250kbps for CC26X2R1: AT+I 0 |
    |          |                                    |  00: Proprietary 250kbps (2440MHz)                  | Initialize IEEE 250kbps for CC26X2R1: AT+I 1        |
    |          |                                    |  01: IEEE 250kbps (2405MHz)                         |                                                     |
    |          |                                    |  02-09: To be defined if more PHYs                  |                                                     |
    |          |                                    |  are manually added                                 |                                                     |
    |          |                                    |                                                     |                                                     |
    |          |                                    | Sub-1GHz-only devices                               | Initialize 50kbps for CC1312R1: AT+I 0              |
    |          |                                    |  00: 50kbps  (433MHz for CC1352P-4/CC1352P7-4       |                                                     |
    |          |                                    |               868MHz for other devices)             |                                                     |
    |          |                                    |  01-09: To be defined if more PHYs                  |                                                     |
    |          |                                    |  are manually added                                 |                                                     | 
    |          |                                    |                                                     |                                                     |
    |          |                                    | Devices that support 2.4GHz and                     | Initialize 50kbps for CC1352R1: AT+I 0              |
    |          |                                    | Sub-1GHz                                            | Initialize Proprietary 250kbps for CC1352R1: AT+I 1 |
    |          |                                    |  00: 50kbps  (433MHz for CC1352P-4/CC1352P7-4       | Initialize IEEE 250kbps for CC1352R1: AT+I 2        |
    |          |                                    |               868MHz for other devices)             |                                                     |
    |          |                                    |  01: Proprietary 250kbps (2440MHz)                  |                                                     |
    |          |                                    |  02: IEEE 250kbps (2405MHz)                         |                                                     |
    |          |                                    |  03-09: To be defined if more PHYs                  |                                                     |
    |          |                                    |  are manually added                                 |                                                     |
    |----------|------------------------------------|-----------------------------------------------------|-----------------------------------------------------|
    |   RS/rs  | System reset                       | None                                                | AT+RS                                               |
    |----------|------------------------------------|-----------------------------------------------------|-----------------------------------------------------|

Please see PhySettings.c for a detailed description on how to manually add a PHY parameter.

__AT Parameters__

Parameters offer set (=) and get (?) functionality.

The supported parameters are:

    |-----------------------------------------------------------------------------|--------------------------|
    | Param | R/W | Description                      | Parameter(s)               | Example                  | 
    |=======|=====|==================================|============================|==========================|
    |  FR   | R/W | Read/Write frequency in Hz       | Frequency (4 bytes)        | - Read frequency: ATPFR? |
    |  fr   |     |                                  |                            | - Write Frequency:       |
    |       |     |                                  |                            |   ATPFR=<Frequency>      |
    |-------|-----|----------------------------------|----------------------------|--------------------------|
    |  PW   | R/W | Read/Write tx power in dBm       | Power in decimal between   | - Read TX power: ATPPW?  |
    |  pw   |     |                                  | -10 to 14 dBm (20 dBm on   | - Write TX power:        |
    |       |     |                                  | 1352P). Note ccfg changes  |   ATPPW=<TX POWER>       |
    |       |     |                                  | are required for 14 dBm    |                          |
    |       |     |                                  | output power; this applies |                          |
    |       |     |                                  | to CC13x2P running the     |                          |
    |       |     |                                  | default PA                 |                          |
    |-------|-----|----------------------------------|----------------------------|--------------------------|
    |  CR   |  R  | Read Current RSSI when the radio | None                       | ATPCR?                   |
    |  cr   |     | is in cont. RX (Test Mode 3)     |                            |                          |
    |-------|-----|----------------------------------|----------------------------|--------------------------|
    |  TM   | R/W | Read/Write test mode             | Test mode                  | - Read Test Mode: ATPTM? |
    |  tm   |     |                                  | 0: Exit                    | - Set Test Mode:         |
    |       |     |                                  | 1: Carrier Wave            |   ATPTM=<Test Mode>      |
    |       |     |                                  | 2: Modulated               |                          |
    |       |     |                                  | 3: cont. RX                |                          |
    |       |     |                                  | 4: PER TX                  |                          |
    |       |     |                                  | 5: PER RX                  |                          |
    |-------|-----|----------------------------------|----------------------------|--------------------------|
    |  PP   | R/W | Read/Write Number of PER Tx/Rx   | Number of Tx/Rx Packets    | - Read number of PER     |
    |  pp   |     | Packets                          | (0 - 65535)                |   packets: ATPPP?        |
    |       |     |                                  |                            | - Write number of PER    |
    |       |     |                                  |                            |   packets:               |
    |       |     |                                  |                            | ATPPP=<Number of Packets>|
    |-------|-----|----------------------------------|----------------------------|--------------------------|
    |  PL   | R/W | Read/Write PER Tx/Rx Packet      | Tx/Rx Payload Length       | - Read packet length:    |
    |  pl   |     | Length                           | (2 - 255)*                 |   ATPPL?                 |
    |       |     |                                  |                            | - Write packet length:   |
    |       |     |                                  |                            |   ATPPL=<Packet Length>  |
    |-------|-----|----------------------------------|----------------------------|--------------------------|
    |  PN   | R   | Read the PHY names for the       | None                       | ATPPN?                   |
    |  pn   |     | available PHYs                   |                            |                          |
    |-------|-----|----------------------------------|----------------------------|--------------------------|
    |  AR   | R   | Read average RSSI during PER RX  | None                       | ATPAR?                   |
    |  ar   |     |                                  |                            |                          |
    |-------|-----|----------------------------------|----------------------------|--------------------------|
    |  MR   | R   | Read maximum RSSI during PER RX  | None                       | ATPMR?                   |
    |  mr   |     |                                  |                            |                          |
    |-------|-----|----------------------------------|----------------------------|--------------------------|
    |  LR   | R   | Read lowest/minimum RSSI during  | None                       | ATPMR?                   |
    |  lr   |     | PER RX                           |                            |                          |
    |-------|-----|----------------------------------|----------------------------|--------------------------|
    |  AE   | R/W | Read/Write AT command echo       | 0: Disable AT command echo | - Read echo setting:     |
    |  ae   |     | ( echo is enabled by default)    | 1: Enable AT command echo  |   ATPAE?                 |
    |       |     |                                  |                            | - Write echo setting:    |
    |       |     |                                  |                            |   ATPAE=<Echo Setting>   |
    |-------|-----|----------------------------------|----------------------------|--------------------------|
       
    * For BLE packets, the length set is the PDU Payload Length (12 - 37 bytes). If the length is set outside this, a default
      PDU Payload Length of 37 will be used

It is important to note that most of the parameters (except PP and PL)  requires that the initialization command has been run ( "AT+I x").

The format of a parameter read command to get the Tx Power parameter is:

        ATPPW?<CR>

The response to a read will be of the format shown below, depending on the parameter it will be hex or decimal:

        -10<CR>

The format of an AT command to set the Frequency Parameter to 868 MHz is:

        ATPFR=868000000<CR>

The response of a write will be of the format:

        OK<CR>

__Command Responses__

    |-----------------------------------------------------------------|
    | Response                        | Description                   |
    |=================================|===============================|
    | OK<CR>                          | Command executed successfully |
    |---------------------------------|-------------------------------|
    | AtProcess_Status_CmdIdError<CR> | Command ID not valid          |
    |---------------------------------|-------------------------------|
    | AtProcess_Status_ParamError<CR> | Command failed due to         |
    |                                 | a parameter error             |
    |---------------------------------|-------------------------------|
    | AtProcess_Status_Error<CR>      | Command cound not be executed |
    |-----------------------------------------------------------------|

## Test Modes

The AT interface supports the following test modes:

- Carrier Wave
- Modulated Signal
- Cont. RX
- PER Tx
- PER Rx

### Carrier Wave:

This is enabled by setting test mode to 1. To configure the carrier wave test mode use:

        ATPTM=1<CR>

To exit the carrier wave test mode:

        ATPTM=0<CR>

### Modulated Signal:

This is enabled by setting test mode to 2. To configure the Modulated Signal test mode use:

        ATPTM=2<CR>

To exit the Modulated Signal test mode:

        ATPTM=0<CR>

### Continuous RX:

This is enabled by setting test mode to 3. To configure the cont. RX test mode use:

        ATPTM=3<CR>

The user may request the RSSI data from the radio by polling the CR (Current RSSI), while the radio is running in receive mode, by

        ATPCR?<CR>

A value of -128 indicates an error in the RSSI value.

To exit this test mode:

        ATPTM=0<CR>

### PER TX:

This is enabled by setting test mode to 4, it will run for the number of packets configure in the `PP` Parameter. To configure PER Tx test mode to run for 100 packets:

        ATPPP=100<CR>
        ATPTM=4<CR>

While the test is running the below responses should be expected:

        Packet: 0
        Packet: 1
        Packet: 2
        ...
        ...
        Packet: 99

Once the test completes the below response should be expected.

        OK<CR>

Other related parameters to this test is:

- PPL: PER Tx Packet Length

### PER Rx:

This is enabled by setting test mode to 5. It will run until test mode is set back to 0 (exit). To start PER RX:

        ATPTM=5<CR>

While the test is running the below responses should be expected:

       RX in Progress....
       Exit by setting Test Mode = 0 (ATPTM=0)
       
       Note: Packet Length filtering is utilized, so the packet length
       on the receiver must be larger or equal to the packet length on the transmitter
       OK

Once the test completes (ATPTM=0) the below response should be expected.

        Packets Received:    x
        Average RSSI:        -y dBm
        Max RSSI:            -n dBm
        Min RSSI:            -m dBm
        OK


Where x is the number of packets received with CRC OK, and y is the average RSSI of all of the received packets (CRC OK)

