# rfSynchronizedPacketRx

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

In this example you will learn how to build a time-synchronized connection
between one transmitter and a receiver. Time-synchronization enables both
communication partners to transfer data quickly at predictable time points.
Unlike the wake-on-radio example, the transmitter does not need to send a very
long preamble and the receiver does not need to wait and check for a signal on
air. This leads to the lowest possible power consumption on both sides. It
also fits very well to the SimpleLink Long-range mode. Time synchronization
builds also the foundation for Frequency and Time Division Multiple Access,
FDMA and TDMA respectively.

This example project shows the receiver part. The transmission part can be
found in the Synchronized Packet TX example.

Peripherals Exercised
---------------------

* `CONFIG_PIN_BTN1` - Toggles between periodic and spontaneous beacon mode.
* `CONFIG_PIN_RLED` - Reflects the status of `CONFIG_PIN_RLED` on the transmitter.
* `CONFIG_PIN_GLED` - On while the receiver is enabled (LNA enable), off when in standby.

Resources & Jumper Settings
---------------------------

This section explains the resource mapping across various boards. If you're
using an IDE (such as CCS or IAR), please refer to Board.html in your project
directory for resources used and board-specific jumper settings. Otherwise,
you can find Board.html in the directory
\<SDK_INSTALL_DIR\>/source/ti/boards/\<BOARD\>.


### SmartRF06 in combination with one of the CC13x0 evaluation modules

| Resource          | Mapping / Notes                                        |
| ----------------- | ------------------------------------------------------ |
| `BUTTON1`         | `BTN_UP` (up button)                                   |
| `BUTTON1`         | `BTN_DN` (down button)                                 |
| `LED1`            | `LED1`                                                 |
| `LED2`            | `LED2`                                                 |


### CC1310 / CC1350 / CC2640R2 Launchpad

| Resource          | Mapping / Notes                                        |
| ----------------- | ------------------------------------------------------ |
| `BUTTON1`         | `BTN-1` (left button)                                  |
| `BUTTON2`         | `BTN_2` (right button)                                 |
| `LED1`            | Green LED                                              |
| `LED2`            | Red LED                                                |

Board Specific Settings
-----------------------
1. The default frequency is:
    - 433.92 MHz for the CC1350-LAUNCHXL-433 and the CC1352P-4-LAUNCHXL
    - 2440 MHz on the CC2640R2-LAUNCHXL and the CC26X2R1-LAUNCHXL
    - 868.0 MHz for other launchpads
In order to change frequency, modify the ti_radio_config.c file. This can be
done using the code export feature in Smart RF Studio, or directly in the file
2. On the CC1352P1 the high PA is enabled (high output power) for all
Sub-1 GHz modes by default.
3. On the CC1352P-2 and CC1352P-4 the high PA operation for Sub-1 GHz modes is not supported
5. The CC2640R2 is setup to run all proprietary physical modes at a center
frequency of 2440 MHz, at a data rate of 250 Kbps

Example Usage
-------------

This section is similar for both TX and RX. You need 2 boards: one running the
`rfSynchronizedPacketTx` application (TX board) and another one running the
`rfSynchronizedPacketRx` application (RX board).


### Initial synchronization

1. Build and run the `rfSynchronizedPacketRx` example on the RX board.
   You will see `LED2` on the RX board being on all the time.

2. Build and run the `rfSynchronizedPacketTx` example on the TX board.
   You will see `LED2` on the TX board flashing with a period of 500 ms.
   On the RX board, you will see that `LED2` is flashing synchronously.

3. Push `BUTTON1` on the TX board. `LED1` will toggle immediately.
   On the RX board, `LED1` follows after a short delay.

4. You may push `BUTTON1` several times and will see that
   `LED1` on the RX board will always reflect the state
   on the TX board with some delay.

Explanation: After starting, the RX board goes into `WaitingForSync` state.
The receiver is switched on end waits for a packet. The LNA signal (`LED2`) is
enabled to reflect the current receiver state.

When the application on the TX board is started, it starts to send periodic
beacon messages. Once the RX board has received the first beacon message, it
switches the receiver off and goes into `SyncedRx` state. In this state, it
wakes up the receiver right before the next packet from the TX board is
expected.

When `BUTTON1` on the TX board is pushed, the current LED state is sent
in the next available time slot and is shown on the RX board as soon
as the packet has arrived.


### Sending spontaneous messages after synchronization

5. Push `BUTTON2` on the TX board. You will see that `LED2` on the
   TX board stops flashing while `LED2` on the RX boards remains
   flashing.

6. Push `BUTTON1` on the TX board. You will see that `LED1` toggles
   on the TX board and with a short delay also on the RX board.
   `LED2` on the TX board will flash a short while after pushing
   the button.

Explanation: After pushing `BUTTON2` on the TX board, the TX application
goes into `SporadicMessage` state and stops sending periodic beacons.
The RX application remains in `SyncedRx` state and wakes up when it
expects a packet. As long as no button on the TX board is pushed,
the RX board will wake up only for a short time and go back to standby
after a very short timeout because no packet is received.

When pushing `BUTTON1` on the TX board, a packet with the new state of `LED1`
is transmitted. The TX board sends exactly at the same time when the RX board
expects to receive a packet. The RX board receives the message and updates the
state of its own `LED1`.


### Error handling: Re-synchronization due to crystal drift

7. Repeat step 6 for a while. After a couple of minutes, you will notice that
   `LED1` on the RX board is not updated properly anymore.

8. Push `BUTTON1` on the RX board. `LED2` will remain on permanently.

9. Push `BUTTON1` on the TX board. You will see `LED1` toggle on both boards
   and `LED2` on the RX board starting to flash again.

Explanation: Both TX and RX board predict the following wake-up events based
on the time when synchronization happened. If both clocks have a small drift,
then the wake-up time will be incorrect after some time.

By pushing `BUTTON1` on the RX board, the application goes back into
`WaitingForSync` state and re-synchronizes to the TX board.


Application Design Details
--------------------------

This examples consists of a single task and the exported SmartRF Studio radio
settings. The RX application is implemented as a state machine with 3 states:

![rx-uml-state-machine][state-machine]


In order to send synchronous packets, the receiver uses absolute start
triggers. Absolute start triggers are explained in the proprietary RF user's
guide and the technical reference manual.

In `WaitingForSync` state, the receiver waits an infinite time in RX state.
That means, the power consumption is high. As soon as it receives a beacon
packet from the transmitter and the packet is valid, it uses the RX time stamp
as an anchor time:

```c
    /* rxTime contains a calculated time stamp when the first preamble byte
     * was sent on air. As a time base for the next wake ups, we calculate
     * the time when this RX command would have been started for a synchronized
     * wake up. */
    RF_cmdPropRx.startTime = rxTime - RX_START_TO_SETTLE_TICKS - RX_START_MARGIN;
```

The RX time stamp is the time when the first preamble bit was on air. The RF
core calculates this time based on the internal "sync word found" signal and
the RF settings provided in the SETUP command. The `RX_START_TO_SETTLE_TICKS`
needs to be subtracted because it takes some time between command start and
the receiver being ready to receive. In addition, `RX_START_MARGIN` is
subtracted to start the RX operation a bit earlier. This is necessary to
compensate possible crystal drifts.

Finally, the application goes into the `SyncedRx` state where it calculates the
time for the next RX start based on the latest received packet:

```c
    /* Start RX command to receive a single packet. Use an absolute start trigger
     * and a predicted start time. The end time is calculated to be as tight as possible.
     * The compensation margin for RAT drift in both directions needs to be taken into account.  */
    RF_cmdPropRx.startTrigger.triggerType = TRIG_ABSTIME;
    RF_cmdPropRx.startTime += beacon.beaconInterval;
    RF_cmdPropRx.endTrigger.triggerType = TRIG_REL_START;
    RF_cmdPropRx.endTime = RX_START_TO_SETTLE_TICKS + RX_TIMEOUT_TICKS + RX_TIMEOUT_MARGIN + RX_START_MARGIN;
```

The RF driver takes care that the RF core is powered up and running before the
start trigger is due. For the end trigger, we use a time relative to the
command start. That means, the timeout is determined by:

- the time needed for transmitting a whole preamble and sync word,
- the margin subtracted from the start trigger,
- an additional timeout margin to compensate possible errors in the calculation


[state-machine]: state-machine.png "RX state chart"