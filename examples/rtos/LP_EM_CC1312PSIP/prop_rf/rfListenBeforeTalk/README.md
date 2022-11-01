# rfListenBeforeTalk

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

The Listen Before Talk (LBT) TX example illustrates how to implement a simple,
proprietary LBT algorithm using command chaining.

When sending a packet, the radio first enters RX mode using CMD_PROP_CS. If
the channel is IDLE (the RSSI is below RSSI_THRESHOLD) for IDLE_TIME_US, the
the radio enters TX and transmits a packet. If the channel is BUSY (RSSI above
RSSI_THRESHOLD), the radio enters RX again to check the channel once more.
This is repeated max CS_RETRIES_WHEN_BUSY number of times. The command chain
will either finish with a packet being sent (if the channel is IDLE), or after
checking the channel CS_RETRIES_WHEN_BUSY times.

Default Settings:

 - CS_RETRIES_WHEN_BUSY: 10
 - RSSI_THRESHOLD:       -90 dBm
 - IDLE_TIME_US:         5000 us


Peripherals Exercised
---------------------

* `CONFIG_PIN_GLED` - Toggled for every packet sent.

Resources & Jumper Settings
---------------------------
> If you're using an IDE (such as CCS or IAR), please refer to Board.html in your project
directory for resources used and board-specific jumper settings. Otherwise, you can find
Board.html in the directory &lt;SDK_INSTALL_DIR&gt;/source/ti/boards/&lt;BOARD&gt;.

Example Usage
-------------
The example code can be tested with SmartRF Studio or the rfPacketRX examples
as the receiver, and a jammer at 868.0 MHz (433.92 MHz for the
CC1350-LAUNCHXL-433 and CC1352P-4-LAUNCHXL boards).


Application Design Details
--------------------------
The command chain contains the following commands:

 - CMD_NOP
 - CMD_PROP_CS
 - CMD_COUNT_BRANCH
 - CMD_PROP_TX

The following image shows the control flow:

--------------------------------------------------------------------------------

                                   -------------
                                   |           |
                                   |  CMD_NOP  |
                                   |           |
                                   -------------
                                         |
                                         |
                                         |
                                 -----------------
         PROP_DONE_IDLE          |               |
    ---------<-------------------|  CMD_PROP_CS  |----<---
    |                            |               |       |
    |                            -----------------       |
    |                                     |              |
    |                      PROP_DONE_BUSY |    RF_cmdCountBranch.counter-- > 0
    |                                     |              |
    |                          ----------------------    |
    |                          |                    |    |
    |                          |  CMD_COUNT_BRANCH  |-->--
    |                          |                    |    |
    |                          ----------------------    |
    |                                    .               |
    |                                    .    RF_cmdCountBranch.counter-- = 0
    |                                    .               |
    |                            -----------------       |
    |                            |               |       |
    --------->-------------------|  CMD_PROP_TX  |       |
                                 |               |       |
                                 -----------------       |
                                        |                |
                                        |--------<--------
                                        |
                            Last command in chain done

--------------------------------------------------------------------------------

The carrier sense command CMD_PROP_CS is prepended by a CMD_NOP in order to
set an absolute trigger time TRIG_ABSTIME for the whole chain. All other
commands trigger immediately after each other. If CMD_PROP_CS indicates a busy
channel it is restarted by CMD_COUNT_BRANCH for maximum
RF_cmdCountBranch.counter times. Only if CMD_PROP_CS observes an IDLE channel,
CMD_PROP_TX is executed.

On a successful transmission, CONFIG_PIN_GLED toggles and the sequence number in
the packet is incremented. If CMD_PROP_TX can not be executed due to an always
BUSY channel, the command chain is restarted again after PACKET_INTERVAL_MS
which is 200 ms by default.

Once triggered, all commands runs entirely on the RF core and do not involve
the main CPU. The CC13xx enters Standby mode in-between packet
transmissions. The following image shows the characteristic current
consumption profile when channel is IDLE (the radio is in RX for 5 ms):

              TX                                             TX
             ----                                           ----
             |  |                                           |  |
             |  |                                           |  |
             |  |                                           |  |
             |  |                                           |  |
          RX |  |                                        RX |  |
          ----  |                                        ----  |
          |     |                                        |     |
          |     |             Standby                    |     |
    ------      ----------------------------------------      ----------------
          <--------------------------------------------->
                              200 ms


If the channel is BUSY, the CMD_PROP_CS command is called 10 times. The
synthesizer is kept on in between the commands.

The time it takes to determine if a channel is BUSY or not depends on RF settings and will 
also differ between devices. For example, the time will be different between CC13x0 and CC13x2. 
To determine the time this takes, and also the time to execute the complete command chain, EnergyTrace can be used.
