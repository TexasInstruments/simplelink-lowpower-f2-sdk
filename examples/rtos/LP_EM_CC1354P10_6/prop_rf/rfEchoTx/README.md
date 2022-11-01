# rfEchoTx

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
communication. The Echo TX example the radio to transmit a packet and then
switch over to the receiver mode and wait for its echo. This example is meant
to be used with the Echo RX example or SmartRF Studio. For every packet echo,
CONFIG_PIN_GLED is toggled. The frequency and other RF settings can be modified
using SmartRF Studio.

Peripherals Exercised
---------------------
* `CONFIG_PIN_GLED` - Blinking indicates a successful transmission and reception
  of a packet (echo)
* `CONFIG_PIN_RLED` - Indicates an abort occurred in packet reception (waiting
  for the echo)
* `CONFIG_PIN_GLED` & `CONFIG_PIN_RLED` indicate an error condition

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
will toggle when it receives a packet, and then when its able to
re-transmit it (see [figure 1]).

![perfect_echo_ref][figure 1]

If the receiver (`Board_2`) is turned off and the rfEchoTx (`Board_1`) begins
transmitting, it switches over to the receiver mode waiting for an echo that
will never come; in this situation a timeout timer is started and if no
packet is received within 300ms the receiver operation is aborted. This
condition is indicated by CONFIG_PIN_GLED being cleared and CONFIG_PIN_RLED
being set (see [figure 2]).

![missed_first_ref][figure 2]

If the receiver continues to stay turned off then the rfEchoTx example will
alternate between transmitting and aborted receiver operations. CONFIG_PIN_GLED
and CONFIG_PIN_RLED will start alternating, as seen in [figure 3].

![missed_first_few_ref][figure 3]

An error in transmission of a packet, or the reception of its echo, is
indicated by both LEDs going high (see [figure 4]).

![echo_error_ref][figure 4]

Application Design Details
--------------------------
This examples consists of a single task and the exported SmartRF Studio radio
settings.

The default frequency is 868.0 MHz (433.92 MHz for the
CC1350-LAUNCHXL-433 board and CC1352P-4-LAUNCHXL boards). In order to change frequency, modify the
ti_radio_config.c file. This can be done using the code export feature in
Smart RF Studio, or directly in the file.

When the task is executed it:

1. Configures the radio for Proprietary mode
2. Gets access to the radio via the RF drivers RF_open
3. Sets up the radio using CMD_PROP_RADIO_DIV_SETUP command
4. Set the output power to 14 dBm (requires that CCFG_FORCE_VDDR_HH = 1 in ccfg.c)
5. Sets the frequency using CMD_FS command
6. Create packet (with increasing sequence number and random content)
7. Set absolute TX time to utilize automatic power management
8. Transmit packet using CMD_PROP_TX command with blocking RF driver call, in
   which case CONFIG_PIN_GLED is toggled. The transmit command is chained with
   the receive command, CMD_PROP_RX, which runs immediately after the packet
   is transmitted
9. The radio waits for the echo and there are three possibilities
   a. echo is successfully received and CONFIG_PIN_GLED is toggled
   b. receive operation times out and CONFIG_PIN_RLED is set
   c. unexpected radio event, an error, in which case both LEDs are set
10. Transmit packets forever by repeating step 6-9


[figure 1]:rfEcho_PerfectEcho.png "Perfect Echo"
[figure 2]:rfEcho_MissedFirstPacket.png "Missed First Packet"
[figure 3]:rfEcho_MissingFirstCouplePackets.png "Missing First Couple of Packets"
[figure 4]:rfEcho_ErrorTxRx.png "Echo Error"