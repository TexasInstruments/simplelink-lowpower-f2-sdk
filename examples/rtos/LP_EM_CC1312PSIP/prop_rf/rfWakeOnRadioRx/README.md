# rfWakeOnRadioRx

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
This examples showcases the Wake-on-Radio (WoR) functionality of the CC1310 to
significantly lower the power consumption of an RF link. It shows how to use the
RF Driver to schedule automatic wake-ups in the future and do a Carrier Sense
as quickly as possible using RSSI and Preamble Quality (PQT).

This example is intended to be used together with the rfWakeOnRadioTx example.

Peripherals Exercised
---------------------
* `CONFIG_PIN_RLED` - The LED is on when the radio is in active RX
* `CONFIG_PIN_GLED` - Toggled when data is transmitted over the RF interface

Resources & Jumper Settings
---------------------------
> If you're using an IDE (such as CCS or IAR), please refer to Board.html in your project
directory for resources used and board-specific jumper settings. Otherwise, you can find
Board.html in the directory &lt;SDK_INSTALL_DIR&gt;/source/ti/boards/&lt;BOARD&gt;.

Example Usage
-------------
Run the example on one of the boards above, this will be the RX board.
CONFIG_PIN_RLED will blink on this board for every wake-up. By default the example
is set up to wake up two times per second, so every 500 ms.

Start the rfWakeOnRadioTx companion example on another board (TX board)
and press CONFIG_PIN_BUTTON0 on that board to send a packet.

CONFIG_PIN_GLED on the RX board should now toggle for every button press.

Basic configuration
--------------------------
It is possible to use the Code Export feature of SmartRF Studio with the
Packet TX / RX tab selected to export settings for this example.
This example has been tested mainly with 50kbit/s.

The wake-up interval is set using the WOR_WAKEUPS_PER_SECOND define at the top
of the rfWakeOnRadioRx.c file. Make sure that this is set to the same in both
the RX and TX part of the Wake-on-Radio example.

Application Design Details
--------------------------
Note: For IAR users using any SensorTag(STK) Board, the XDS110 debugger must be
selected with the 4-wire JTAG connection within your projects' debugger
configuration.

The Wake-on-Radio example is based on the principle of duty-cycling the radio
and entering RX just as much as necessary to detect a packet.

The application has one task which, besides initializing the application,
re-submits a CMD_PROP_RX_SNIFF command to the radio with a given interval.

Whenever the RX Sniff command has been run, it also logs the status of the
command. Some statistics regarding this status may be found in the
"worStatistics" structure. Here it's possible to see why the command finished,
which can be one of four reasons:

1. Returned Idle because no RSSI was found
2. Returned Idle because RSSI was found, but not PQT
3. Timed out because we found RSSI and PQT, but not a valid sync word
4. Received a packet

In the application, the internal LNA control signal has been routed to
CONFIG_PIN_RLED. This means that when the radio is in active RX, CONFIG_PIN_RLED will be
on. If RSSI is not found to be over the configured threshold, then it will be
on for a shorter time than if it is found to be over the threshold. The reason
for this is because if it is over the threshold, then it will also check for a valid preamble.

If the RSSI and a valid preamble is found, then it will also continue and look
for the sync word. If the sync word is found, and the CRC of the packet is also
correct, then CONFIG_PIN_GLED will toggle.

Packet Preamble
---------------

Below is a typical radio physical layer packet format.

    |  Preamble  |  Sync Word  |  Length Byte  |  Payload  |    CRC    |
    --------------------------------------------------------------------
    |   4 byte   |   4 byte    |    1 byte     |  X bytes  |  2 bytes  |

The preamble is usually set to a repeating 10101010 pattern, as the beginning
of the packet is used for several purposes in a modern radio. This usually
involves settling the Automatic Gain Control, estimating frequency error etc.
In addition to this, it can also be used for detecting the presence of a signal.

If we use the preamble to detect the presence of a signal, then the receiver
has to wake up often enough to not miss the preamble. This means that the
length of the preamble directly affects how often the receiver has to wake up.

At 50kbit/s 2-GFSK a 4 byte preamble is only 650 us long. This means that to
not miss a packet, the receiver would have to wake up more than 1500 times
per second. This is generally not a feasible solution, and does not save a lot
of power.

If we instead configure the transmitter to send a 100 ms long preamble, this
means that the receiver only have to wake up 10 times per second to be
guaranteed to receive the packet. This increases the latency in the system,
but significantly reduces the average power consumption.

In this Wake-on-Radio example, the default setting is to send a 500 ms preamble
and so wake up approximately two times per second to check for it.

RSSI and PQT
------------
There are generally two ways to check for the presence of a signal on the air
with a receiver. One is to check the Received Signal Strength Indicator (RSSI)
which simply indicates the energy received. The other is to check for the
presence of a valid preamble and check the Preamble Quality (PQT).

RSSI is usually quicker to check, but also gives less information. How long it
takes to get a valid RSSI read to compare against a given threshold depends
mainly on the configured receiver bandwidth. The information you get from an
RSSI reading is only that there is a signal present, it give no qualitative
information.

PQT takes a bit longer to check than RSSI. The main reason being that the
receiver has to receive a certain number of symbols before it can take a look
at the received data and check that it does indeed look like a valid preamble.
How long this takes mainly depends on the symbol rate.

It is also possible to check both RSSI and PQT. In that case one would normally
first check RSSI, preferably at the sensitivity level, and then if the RSSI is
above it, check for PQT.

The RX Sniff Commands
---------------------
The CC13xx have dedicated radio commands for entering RX and for doing
Carrier Sense. To enter receive mode, one would send either CMD_PROP_RX or
CMD_PROP_ADV. To do a carrier sense, one would send CMD_PROP_CS. There are also
two commands which combine a carrier sense with entering receive in the
presence of a signal. These are the CMD_PROP_RX_SNIFF and
CMD_PROP_RX_ADV_SNIFF.

The sniff commands themselves consist of two parts. The first part are bit
fields that are identical to the respective RX command, and the second part
is identical to the carrier sense command. In practice, the sniff commands
behave much like chaining a CMD_PROP_CS with a CMD_PROP_RX(_ADV), but with
less overhead.

The sniff command can be configured to use either RSSI, PQT or both as a
criterion for detecting the presence of a signal. In this example, both are
used by default.

Wake-on-Radio configuration of the RX Sniff command
---------------------------------------------------
There are several parameters that are important when configuring the RX Sniff
command for use with a long preamble for Wake-on-Radio.

This example has configured the RX Sniff command in the following way by
default:

- Use RSSI and PQT:

    rxSniffCmd->csConf.bEnaRssi = 1;
    rxSniffCmd->csConf.bEnaCorr = 1;

- Report Idle directly if RSSI is not valid, no need to test PQT then:

    rxSniffCmd->csConf.operation = 1;

- End command straight away if Idle is reported:

    rxSniffCmd->csConf.idleOp = 1;

- Continuously check RSSI and PQT for the entire length of the preamble:

    rxSniffCmd->csConf.busyOp = 0;

- The RSSI threshold is set in a define in the example. Default is sensitivity
   of default 50kbit/s SmartRF Studio settings:

    rxSniffCmd->rssiThr    = (int8_t)WOR_RSSI_THRESHOLD;

- A single RSSI above threshold is enough to report Busy / Idle:

    rxSniffCmd->numRssiBusy = 1;
    rxSniffCmd->numRssiIdle = 1;

- A single valid PQT is enough to report Busy / Invalid:

    rxSniffCmd->corrConfig.numCorrBusy = 1;
    rxSniffCmd->corrConfig.numCorrInv  = 1;

Except for the functional settings, there are also several timing parameters
that need to be configured correctly. These are based on the length of the
preamble and/or the selected data rate:

# Start Trigger #
To be able to have a deterministic interval between wake-ups, absolute
timing is used. This is done by setting the start trigger type to be
TRIG_ABSTIME. This means that the start time itself is given in
Radio Timer (RAT) ticks, and this is incremented with the given interval,
(i.e. 500ms for waking up twice per second) for each wake-up.

The start trigger also has the "past trigger" feature enabled. This means that
if the start time has already passed, it will trigger immediately. The reason
for this is because when we increment the start time for the wake-up after
we receive a packet, the new start time might already have passed. This means
that we will simply check the RSSI directly after we receive a packet, and then
start incrementing to the given interval again. Since the interval will simply
be shorter when this happens, no packets can be lost.

# End Trigger #
The end trigger of the RX Sniff command is set to slightly longer than the
entire preamble plus the length of the sync word. This means that if we wake
up at the very beginning of the preamble, we wait until we would have received
the entire preamble and sync word, and if we have not gotten a sync word by
then, we leave RX.

Note: This is critical for not getting stuck in RX in the presence of an
    interferer which is sending a valid preamble.

# Correlation Period #
The correlation period is the time window in which the radio have to have
detected a valid preamble. This time window will be moved forward for every
valid PQT detected, which is done in the underlying hardware via correlation
peaks, hence the name. The very first peak will happen after about 24 symbols
of preamble has been received. Subsequent peaks will happen every two symbols.
If there is no correlation peak within the Correlation Period, then there is no
valid preamble on the air.

One caveat of this period is that it has to be both wide enough to get the very
first correlation peak. Another is that it also has to be wide enough to be
able to fit the entire sync word, with some margin. The reason for this is
because if it is not, then the command will report back that it can no longer
sense preamble as it is receiving the sync word.

# Carrier Sense End Time #
The RX Sniff command does not normally take a decision on the presence of a
valid preamble until the end of the correlation period. And as mentioned above,
the correlation period has to be wide enough to fit the sync word with some
margin. With the carrier sense end time, it is possible to force an early
check.

The earliest we can force this check is when we know that we would have gotten
a correlation peak if there is any preamble present. This will be after about
24 symbols plus 150us. The subject of this, and other timing parameters, will
be covered in a future Wake-on-Radio application note from Texas Instruments.
