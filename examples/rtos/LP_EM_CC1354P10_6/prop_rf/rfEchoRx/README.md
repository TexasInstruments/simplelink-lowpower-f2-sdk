# rfEchoRx

---

Project Setup using the System Configuration Tool (SysConfig)
-------------------------
The purpose of SysConfig is to provide an easy to use interface for configuring
drivers, RF stacks, and more. The .syscfg file provided with each example
project has been configured and tested for that project. Changes to the .syscfg
file may alter the behavior of the example away from default. Some parameters
configured in SysConfig may require the use of specific APIs or additional
modifications in the application source code. More information can be found in
SysConfig by hovering over a configurable and clicking the question mark (?)
next to it's name.

Example Summary
---------------
In this example you will learn how to setup the radio to do bi-directional
communication. The Echo RX example start off by putting the radio in receiver
mode and waits for a packet to be transmitted. When it receives a packet it
switches the radio to transmitter mode and re-transmits the received
packet (echo). This example is meant to be used with the Echo TX example or
SmartRF Studio. For every packet echo, CONFIG_PIN_RLED is toggled. The
frequency and other RF settings can be modified using SmartRF Studio.

Peripherals Exercised
---------------------
* `CONFIG_PIN_RLED` - Blinking indicates a successful reception and
  re-transmission of a packet (echo)
* `CONFIG_PIN_GLED` - Indicates an error in either reception of a packet or
  in its re-transmission

Resources & Jumper Settings
---------------------------
> If you're using an IDE (such as CCS or IAR), please refer to Board.html in
your project directory for resources used and board-specific jumper settings.
Otherwise, you can find Board.html in the directory
&lt;SDK_INSTALL_DIR&gt;/source/ti/boards/&lt;BOARD&gt;.

Example Usage
-------------
The user will require two launchpads, one running rfEchoTx (`Board_1`),
another running rfEchoRx (`Board_2`). Run Board_2 first, followed by
Board_1. Board_1 is set to transmit a packet every second while Board_2 is
set to receive the packet and then turnaround and transmit it after a delay of
100ms. CONFIG_PIN_GLED on Board_1 will toggle when it's able to successfully
transmits a packet, and when it receives its echo. CONFIG_PIN_RLED on Board_2
will toggle when it receives a packet, and once again when it's able to
re-transmit it (see [figure 1]).

![perfect_echo_ref][figure 1]

If there is an issue in receiving a packet then CONFIG_PIN_GLED on Board_2 is
toggled while CONFIG_PIN_RLED is cleared, to indicate an error condition

![echo_error_ref][figure 2]

Application Design Details
--------------------------
This examples consists of a single task and the exported SmartRF Studio radio
settings.

The default frequency is 868.0 MHz (433.92 MHz for the
CC1350-LAUNCHXL-433 and CC1352P-4-LAUNCHXL boards). In order to
change frequency, modify the ti_radio_config.c file. This can be done using
the code export feature in Smart RF Studio, or directly in the file.

When the task is executed it:

1. Configures the radio for Proprietary mode
2. Gets access to the radio via the RF drivers RF_open
3. Sets up the radio using CMD_PROP_RADIO_DIV_SETUP command
4. Set the output power to 14 dBm (requires that CCFG_FORCE_VDDR_HH = 1 in ccfg.c)
5. Sets the frequency using CMD_FS command
6. Run the CMD_PROP_RX command and wait for a packet to be transmitted. The
   receive command is chained with a transmit command, CMD_PROP_TX, which runs
   once a packet is received
7. When a packet is successfully received CONFIG_PIN_RLED is toggled, the radio
   switches over to the transmit mode and schedules the packet for transmission
   100 ms in the future
8. If there is an issue either with the receive or transmit, an error, both
   LEDs are set
9. The devices repeat steps 6-8 forever.


[figure 1]:rfEcho_PerfectEcho.png "Perfect Echo"
[figure 2]:rfEcho_ErrorTxRx.png "Echo Error"