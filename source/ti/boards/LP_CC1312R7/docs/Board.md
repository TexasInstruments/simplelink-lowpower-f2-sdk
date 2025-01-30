# SimpleLink&trade; CC1312R7 LaunchPad&trade; Settings & Resources

The [__SimpleLink&trade; CC1312R7 LaunchPad&trade;__][board] contains a
[__CC1312R74T0RGZR__][device] device.

## Jumper Settings

* Close the __`LEDs`__ jumpers to enable the on-board LEDs.
* Close the __`RXD<<`__ and __`TXD>>`__ jumpers to enable UART via the XDS110 on-board USB debugger.
* In order to get the lowest shutdown current (see the
  `gpioshutdown` example), remove the jumpers for the __`TCK`__ and __`TMS`__
  JTAG pins. TMS and TCK have internal pull-up and are driven low when
  inactive from the emulator, hence after programming the device these jumpers
  should be removed to avoid the extra leakage current.

## SysConfig Board File

The [LP_CC1312R7.syscfg.json](../.meta/LP_CC1312R7.syscfg.json)
is a handcrafted file used by SysConfig. It describes the physical pin-out
and components on the LaunchPad.

## Driver Examples Resources

Examples utilize SysConfig to generate software configurations into
the __ti_drivers_config.c__ and __ti_drivers_config.h__ files. The SysConfig
user interface can be utilized to determine pins and resources used.
Information on pins and resources used is also present in both generated files.

## TI BoosterPacks&trade;

The following BoosterPacks are used with some driver examples.

### [__BOOSTXL-SHARP128 LCD & SD Card BoosterPack__][boostxl-sharp128]

* No modifications are needed.

### [__BP-BASSENSORSMKII BoosterPack__][bp-bassensorsmkii]

* No modifications are needed.

### [__CC3200 Audio BoosterPack__][cc3200audboost]

The BoosterPack's `DIN` signal pin is not compatible with this LaunchPad.
Use the following modifications to enable the CC3200 Audio BoosterPack's
usage with the __i2secho__ example.

On the LP_CC1312R7 board, bend down the following BoosterPack header pin:

* `BP.29`

Be sure that the bent pin does not make contact with the IC or any other
component, only bend it enough to make sure it doesn't connect to the CC3200
Audio BoosterPack.

Attach the CC3200 Audio BoosterPack to the LP_CC1312R7 and run jumper wires
between the following pins on the CC3200 Audio BoosterPack:

* DIN: `P3.3` and `P3.9`

See [Audio BP User Guide][cc3200audboost-user-guide] (Figure 2-1), or the
diagram below for information on where these pins are located.

![CC3200 Audio BoosterPack Jumper Wire Diagram](images/cc3200audboost_jumpers_annotated.png)

[device]: https://www.ti.com/product/CC1312R7
[board]: https://www.ti.com/tool/LP-CC1312R7
[boostxl-sharp128]: https://www.ti.com/tool/BOOSTXL-SHARP128
[bp-bassensorsmkii]: https://www.ti.com/tool/BP-BASSENSORSMKII
[cc3200audboost]: https://www.ti.com/tool/CC3200AUDBOOST
[cc3200audboost-user-guide]: https://www.ti.com/lit/pdf/swru383
