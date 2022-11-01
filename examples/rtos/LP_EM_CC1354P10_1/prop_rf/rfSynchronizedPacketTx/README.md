# rfSynchronizedPacketTx

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

This example project shows the transmission part. The receiver part can be
found in the Synchronized Packet RX example.

Peripherals Exercised
---------------------
* `CONFIG_PIN_BTN1` - Toggles the state of `CONFIG_PIN_GLED` and sends a message in spontaneous beacon state.
* `CONFIG_PIN_BTN2` - Toggles between periodic and spontaneous beacon state.
* `CONFIG_PIN_GLED` - Toggled when `CONFIG_PIN_BTN1` is pushed.
* `CONFIG_PIN_RLED` - On while data is being transmitted over the RF interface (PA enable), off when in standby.

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
| `CONFIG_PIN_BTN1` | `BTN_UP` (up button)                                   |
| `CONFIG_PIN_BTN2` | `BTN_DN` (down button)                                 |
| `CONFIG_PIN_GLED` | `LED1`                                                 |
| `CONFIG_PIN_RLED` | `LED2`                                                 |


### CC1310 / CC1350 / CC2640R2 Launchpad

| Resource          | Mapping / Notes                                        |
| ----------------- | ------------------------------------------------------ |
| `CONFIG_PIN_BTN1` | `BTN-1` (left button)                                  |
| `CONFIG_PIN_BTN2` | `BTN_2` (right button)                                 |
| `CONFIG_PIN_GLED` | Green LED                                              |
| `CONFIG_PIN_RLED` | Red LED                                                |

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
4. The CC2640R2 is setup to run all proprietary physical modes at a center
frequency of 2440 MHz, at a data rate of 250 Kbps

Example Usage
-------------

This section is similar for both TX and RX. You need 2 boards: one running the
`rfSynchronizedPacketTx` application (TX board) and another one running the
`rfSynchronizedPacketRx` application (RX board).


### Initial synchronization

1. Build and run the `rfSynchronizedPacketRx` example on the RX board.
   You will see `CONFIG_PIN_RLED` on the RX board being on all the time.

2. Build and run the `rfSynchronizedPacketTx` example on the TX board.
   You will see `CONFIG_PIN_RLED` on the TX board flashing with a period of 500 ms.
   On the RX board, you will see that `CONFIG_PIN_RLED` is flashing synchronously.

3. Push `CONFIG_PIN_BTN1` on the TX board. `CONFIG_PIN_GLED` will toggle immediately.
   On the RX board, `CONFIG_PIN_GLED` follows after a short delay.

4. You may push `CONFIG_PIN_BTN1` several times and will see that
   `CONFIG_PIN_GLED` on the RX board will always reflect the state
   on the TX board with some delay.

Explanation: After starting, the RX board goes into `WaitingForSync` state.
The receiver is switched on end waits for a packet. The LNA signal (`CONFIG_PIN_RLED`) is
enabled to reflect the current receiver state.

When the application on the TX board is started, it starts to send periodic
beacon messages. Once the RX board has received the first beacon message, it
switches the receiver off and goes into `SyncedRx` state. In this state, it
wakes up the receiver right before the next packet from the TX board is
expected.

When `CONFIG_PIN_BTN1` on the TX board is pushed, the current LED state is sent
in the next available time slot and is shown on the RX board as soon
as the packet has arrived.


### Sending spontaneous beacons after synchronization

5. Push `CONFIG_PIN_BTN2` on the TX board. You will see that `CONFIG_PIN_RLED` on the
   TX board stops flashing while `CONFIG_PIN_RLED` on the RX boards remains
   flashing.

6. Push `CONFIG_PIN_BTN1` on the TX board. You will see that `CONFIG_PIN_GLED` toggles
   on the TX board and with a short delay also on the RX board.
   `CONFIG_PIN_RLED` on the TX board will flash a short while after pushing
   the button.

Explanation: After pushing `CONFIG_PIN_BTN2` on the TX board, the TX application
goes into `SporadicMessage` state and stops sending periodic beacons.
The RX application remains in `SyncedRx` state and wakes up when it
expects a packet. As long as no button on the TX board is pushed,
the RX board will wake up only for a short time and go back to standby
after a very short timeout because no packet is received.

When pushing `CONFIG_PIN_BTN1` on the TX board, a packet with the new state of `CONFIG_PIN_GLED`
is transmitted. The TX board sends exactly at the same time when the RX board
expects to receive a packet. The RX board receives the message and updates the
state of its own `CONFIG_PIN_GLED`.


### Error handling: Re-synchronization due to crystal drift ===

7. Repeat step 6 for a while. After a couple of minutes, you will notice that
   `CONFIG_PIN_GLED` on the RX board is not updated properly anymore.

8. Push `CONFIG_PIN_BTN1` on the RX board. `CONFIG_PIN_RLED` will remain on permanently.

9. Push `CONFIG_PIN_BTN1` on the TX board. You will see `CONFIG_PIN_GLED` toggle on both boards
   and `CONFIG_PIN_RLED` on the RX board starting to flash again.

Explanation: Both TX and RX board predict the following wake-up events based
on the time when synchronization happened. If both clocks have a small drift,
then the wake-up time will be incorrect after some time.

By pushing `CONFIG_PIN_BTN1` on the RX board, the application goes back into
`WaitingForSync` state and re-synchronizes to the TX board.


Application Design Details
--------------------------

This examples consists of a single task and the exported SmartRF Studio radio
settings. The TX application is implemented as a state machine with 3 states:

![tx-uml-state-machine][state-machine]

In order to send synchronous packets, the transmitter uses an absolute start
trigger for the TX command. Absolute start triggers are explained in the
proprietary RF user's guide and the technical reference manual. It starts with
an arbitrarily chosen time stamp:

```c
    /* Use the current time as an anchor point for future time stamps.
     * The Nth transmission in the future will be exactly N * 500ms after
     * this time stamp.  */
    RF_cmdPropTx.startTime = RF_getCurrentTime();
```

And then adds a fixed interval for any further transmission:


```c
    /* Set absolute TX time in the future to utilize "deferred dispatching of commands with absolute timing".
     * This is explained in the proprietary RF user's guide. */
    RF_cmdPropTx.startTime += RF_convertMsToRatTicks(BEACON_INTERVAL_MS);
```

In `SpontaneousBeacon` state, the next transmission start time is calculated
based on the last value transmission start time:

```
    /* We need to find the next synchronized time slot that is far enough
     * in the future to allow the RF driver to power up the RF core.
     * We use 2 ms as safety margin. */
    uint32_t currentTime = RF_getCurrentTime() + RF_convertMsToRatTicks(2);
    uint32_t intervalsSinceLastPacket = DIV_INT_ROUND_UP(currentTime - RF_cmdPropTx.startTime, RF_convertMsToRatTicks(BEACON_INTERVAL_MS));
    RF_cmdPropTx.startTime += intervalsSinceLastPacket * RF_convertMsToRatTicks(BEACON_INTERVAL_MS);
```

That means, the transmission is not really "spontaneous", but rather "as soon
as possible" according to the interval. A safety margin of 2 ms is added
because the RF core is powered down while waiting for the RX command. If we
are close to the next slot, the RF driver would not have enough time to re-
initialize the RF core.

No further timing restrictions apply to the transmitter.


[state-machine]: state-machine.png "TX state chart"