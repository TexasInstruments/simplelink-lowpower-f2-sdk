# rfPacketErrorRate

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
-------------------------
The Packet Error Rate (PER) example showcases different RF transfer modes of the
CC13xx and CC2640R2. It combines tasks, events and several peripherals to a platform-
independent application and runs on all CC13xx and CC2640R2 launchpads as well as the
CC1310EM/SmartRF06.

Test cases are provided for the following RF modes:

- `Custom` : Custom settings to be exported from SmartRF Studio to the
ti_radio_config.c file
- `2-GFSK` : Gaussian frequency shift keying with binary symbols, 50 kBit/s for
all launchpads, 250 KBits/s for the CC2640R2
- `GFSK 200K` : Gaussian frequency shift keying with binary symbols, 200 kBit/s
  IEEE 802.15.4g for all CC13x2 launchpads
- `GFSK 100K` : Gaussian frequency shift keying with binary symbols, 100 kBit/s
  (CC2640R2 only)
- `GFSK 250K` : Gaussian frequency shift keying with binary symbols, 250 kBit/s
  (CC2640R2 only)
- `LR Mode` : Legacy Long Range (625 bps)
- `OOK` : On-off keying
- `HS Mode` : High-speed mode with up to 4 MBit/s
- `SL LR` : SimpleLink Long Rang (5 kbps)
- `BLE` : Bluetooth Low Energy (CC135X, CC2640R2 only)
- `BLE5` : Bluetooth Low Energy 5 (CC1352, CC2640R2 only)

Note that
- `HSM` and `OOK` are not supported on the CC1310-CC1190, CC1350 (433 MHz), CC1352
(R1/P1/P-2/P-4) or CC2640R2 Launchpads
- Legacy Long Range Mode (`LRM`) is not supported on the CC1352(R1/P1/P-2/P-4).
- For the `GFSK 200K` mode, if transmitting from Smart RF Studio set the PSDU
  octet transmission order to **MSbit First**

Peripherals Exercised
---------------------
This example uses the following CC13xx peripherals:

- RF core for radio communication
- 2 GPIOs for buttons
- 3 GPIOs for controlling the CC1190 when running CC1310-CC1190 Launchpad
- SPI to drive the LCD displays on the boards
- UART0 (115200 Baud) as an alternative display on VT100-compatible
  terminal emulators



  | Peripheral        | Identifier        | CC13xxEM/SmartRF06 | CC13xx Launchpad     | CC1310-CC1390 Launchpad |
  | ----------------- | ----------------- | ------------------ | -------------------- |-------------------------|
  | Select button     | CONFIG_PIN_BTN1   | UP                 | BTN-1                | BTN-1                   |
  | Navigate button   | CONFIG_PIN_BTN2   | DOWN               | BTN-2                | BTN-2                   |
  | Display           | --                | LCD Dogm1286       | Display Booster Pack | Display Booster Pack    |
  | CC1190 Control    | CONFIG_HGM        | --                 | --                   | DIO28                   |
  | signals           | CONFIG_LNA_EN     | --                 | --                   | DIO29                   |
  |                   | CONFIG_PA_EN      | --                 | --                   | DIO30                   |

Resources & Jumper Settings
---------------------------

> If you're using an IDE (such as CCS or IAR), please refer to Board.html in
your project directory for resources used and board-specific jumper settings.
Otherwise, you can find Board.html in the directory
&lt;SDK_INSTALL_DIR&gt;/source/ti/boards/&lt;BOARD&gt;.

Board Specific Settings
-----------------------
1. On the CC1352P1 the user has the ability to turn on the high PA (high
output power) for all Sub-1 GHz modes, but not for the BLE/BLE5 modes.
2. On the CC1352P-2 and CC1352P-4 the the user has the ability to turn on the high PA (high
output power) for the BLE/BLE5 modes but not the Sub-1 GHz modes.
  - On CC1352P-4 the The center frequency for all Sub-1 GHz modes is 433.92 MHz
3. The CC2640R2 is setup to run all proprietary physical modes at a center
frequency of 2440 MHz, at a data rate of either 250 Kbps or 100 Kbps

Example Usage
-------------

This example requires two boards, each running the PER Test application.
However, the packet format is identical to the default one in SmartRF Studio, so
that any compatible hardware can be used as well.

1. Connect a display to the board or alternatively, use the UART and hook it to
   a VT100-compatible terminal emulator at 115200 Baud. Use PuTTY or
   TeraTerm on Microsoft Windows. On Linux, use the terminal emulator that is
   shipped with your distribution. After a splash screen, you will see the
   main menu (the range extender option (CC1190) is only shown for CC1310
   LAUNCHXL):


        Main Menu
        >Test: Rx
         Mode: 2-GFSK
         Freq: 868.0
         Pkts: 10
         Interval: --
         Length: --
         CC1190: Disable

         Start...


2. Navigate through the rows with BTN-2/DOWN, modify a value or start the
 selected test with BTN-1/UP. Note that it is the user's responsibility to
 enable CC1190 from the menu when running CC1310-CC1190 LAUNCHXL. The
 P variants of the CC1352 have an in-built PA that allow for higher transmission
 for either Sub-1 or 2.4 GHz bands; there is a menu line option to enable
 or disable the high PA for these devices

3. Use a second board with the PER test application as test companion for
   transmissions. Once started, the current progress is shown with these menus
   (TX mode on the left side, RX on the right):

        Sending...      |  Receiving...
        2-GFSK  868.0   |  HS Mode 868.0
                        |  Pkts ok   : 39
        Pkts sent: 47   |  RSSI [dBm]: -74
        Pwr[dBm] : 10   |  PER  [%]  : 17.01
        DR[bps]  : 50000|  TP[bps]: 20867
        Interval : 60 ms|
                        |  Push a button
                        |  to abort.

   The receiver prints the amount of successfully received packets (Pkts ok),
   the Signal strength of the current packet (RSSI), the observed packet
   error rate (PER) in percent, and the observed throughput in bits per second.
   Please note that the PER is n/a when sending more packets than configured in
   the receiver.

5. You can always abort a running test case by pushing any button and go back
   to the main menu.


Application Design Details
--------------------------

The PER test application contains of one main task and an event handler to
synchronize the task with buttons. After setting up all resources, the menu task
is started and runs in an endless loop. It shows the menu and invokes test cases
in either `rx.c` or `tx.c`.

Changelog
---------

### Version 1.1

- released with TI-RTOS 2.17
- add version number on the splash screen
- hide cursor on VT100 terminals


### Version 1.0

 - shipped along with the CC1310 launchpads as default application

### Version 2.0

 - released with TI-RTOS 2.21
 - added support for CC2650_LAUNCHXL and CC2650DK_7ID
 - added BLE mode for CC1350 and CC2650 platforms. Note that the default
frequency is 2.402MHz, which is an advertisement channel and may result in
packet errors due to BLE devices advertising.
 - added custom mode which takes settings exported from SmartRf Studio into
rfPacketErrorRate_\<platform\>\ti_radio_config

### Version 2.1

 - released with simplelink_cc13x0_sdk_1_10
 - added support for SimpleLink Long Range (5 kbps)
 - added support for CC1310-CC1190 LAUNCHXL. When running on this HW, it is
the user's responsibility to enable CC1190 from the menu. Note that HSM and
OOK is not supported in the PER test when running on this HW

### Version 2.2

 - released with simplelink_cc13x0_sdk_1_50_00_00
 - added support for CC1352 and CC1350 (433MHz) LAUNCHXL. These boards support
   a reduced set of modes
 - added a No-RTOS version of the code

### Version 2.3

 - released with simplelink_cc13x0_sdk_1_60_00_00
 - added support for BLE5 for CC1352 LAUNCHXL
 - added support for CC1312 LAUNCHXL

### Version 2.4

 - released with simplelink_cc13x0_sdk_1_70_00_00
 - added throughput calculation
 - reworked PER calculation, the receiver estimates the missing packets based
   on time elapsed with no received packets, and then corrects that estimate if
   packets are received later on
 - packet transmission happens back-to-back for highest throughput
 - packet length is now configurable in back-to-back transmission mode
 - data rate and transmission power are displayed when transmitting packets

### Version 2.40.01

- released with simplelink_cc13xx_sdk_2_20_00_00
- added support for CC1352P board variants
- CC2640R2, additional phy modes to support 250Kbps and 100Kbps data rates

### Version 2.40.02

- released with simplelink_cc13xx_sdk_2_30_00_00
- fixed error in OOK and HSM Tx Power display on CC1310+1190 when the range
  extender is enabled
- fixed error in Tx Power display for the CC1350; the max power displayed
  was incorrect when the boost mode is off


### Version 2.40.03
- added support for run-time PA selection (High or Default)
- added support for 200 Kbps 2-GFSK mode for all CC13x2

### Version 2.80.00
- added support for CC26X2R1_LAUNCHXL
- added support for 2.4GHz 250 Kbps and 100 kbps 2-GFSK mode on all CC1352 and
  CC2652 devices

### Version 3.10.00
- added support for CC1352P_4_LAUNCHXL
- added support for custom boards in SysConfig

### Version 3.20.00

- Added support for LP-CC2652RSIP and LP-CC2652PSIP modules

### Version 3.30.00

- Added support for limited migration between devices/boards in SysConfig

### Version 3.30.01

- Added support for LP_CC1312R7, LP_CC1352P7_1, LP_CC1352P7_4, and LP_CC2652R7

### Version 4.10.00

- Added TI Clang compiler support

### Version 4.20.00

- Added support for CC1311P3, CC2651P3, and CC2651R3

### Version 4.30.00

- GPIO driver has replaced pin driver 
- TIRTOS7 is used for CC13X2X7, CC26X1, and CC13X1 devices

### Version 4.40.00

- Added support for FreeRTOS
- Added CC2652PSIP suppport
- TIRTOS7 is used for CC13X2 and CC26X2 devices

### Version 5.10.00

- Added CC2651R3SIPA support

### Version 5.20.00

- Removal of TI ARM CCS compiler support (TI Clang still supported)

### Version 5.30.00

- Support for CC1312PSIP device
- NoRTOS application supports CC13X4 devices
