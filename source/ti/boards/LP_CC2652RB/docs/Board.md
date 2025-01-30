# SimpleLink&trade; CC2652RB LaunchPad&trade; Settings & Resources

The [__SimpleLink&trade; CC2652RB LaunchPad&trade;__][board] contains a
[__CC2652RB1FRGZR__][device] device.

## Jumper Settings

* Close the __`LEDs`__ jumpers to enable the on-board LEDs.
* Close the __`RXD<<`__ and __`TXD>>`__ jumpers to enable UART via the XDS110 on-board USB debugger.
* In order to get the lowest shutdown current (see the
  `gpioshutdown` example), remove the jumpers for the __`TCK`__ and __`TMS`__
  JTAG pins. TMS and TCK have internal pull-up and are driven low when
  inactive from the emulator, hence after programming the device these jumpers
  should be removed to avoid the extra leakage current.

## SysConfig Board File

The [LP_CC2652RB.syscfg.json](../.meta/LP_CC2652RB.syscfg.json)
is a handcrafted file used by SysConfig. It describes the physical pin-out
and components on the LaunchPad.

## Driver Examples Resources

Examples utilize SysConfig to generate software configurations into
the __ti_drivers_config.c__ and __ti_drivers_config.h__ files. The SysConfig
user interface can be utilized to determine pins and resources used.
Information on pins and resources used is also present in both generated files.

## TI BoosterPacks&trade;

The following BoosterPacks are used with some driver examples.

### [__BOOSTXL-CANFD-LIN SPI to CAN FD + LIN BoosterPack__][boostxl-canfd-lin]

The BoosterPack requires an external voltage supply connected to the barrel jack __`J2`__ in the range of 6 V to 24 V.

The following modifications are required:

* On the BoosterPack, remove jumpers __`J11`__ and __`J12`__ to disconnect the BoosterPack's 3.3 V and 5 V rails if the
  LaunchPad is powered from the debugger.
* On the LaunchPad, bend down header pin BP.33 (__`TCK`__) to prevent the signal from interfering with the BoosterPack
  __`nWKRQ`__ signal.
* On the LaunchPad, remove the __`TDI`__ jumper to disconnect the debugger from LP.31 and prevent interference with the
  BoosterPack __`nINT`__ signal.

Place the BoosterPack directly atop the LaunchPad.

### [__BOOSTXL-SHARP128 LCD & SD Card BoosterPack__][boostxl-sharp128]

* No modifications are needed.

### [__BP-BASSENSORSMKII BoosterPack__][bp-bassensorsmkii]

* No modifications are needed.

### [__CC3200 Audio BoosterPack__][cc3200audboost]

* No modifications are needed.

[device]: https://www.ti.com/product/CC2652RB
[board]: https://www.ti.com/tool/LP-CC2652RB
[boostxl-canfd-lin]: https://www.ti.com/tool/BOOSTXL-CANFD-LIN
[boostxl-sharp128]: https://www.ti.com/tool/BOOSTXL-SHARP128
[bp-bassensorsmkii]: https://www.ti.com/tool/BP-BASSENSORSMKII
[cc3200audboost]: https://www.ti.com/tool/CC3200AUDBOOST
