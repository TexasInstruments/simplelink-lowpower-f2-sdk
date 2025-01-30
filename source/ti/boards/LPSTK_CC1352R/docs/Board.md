# SimpleLink&trade; LPSTK-CC1352R LaunchPad&trade; Settings & Resources

The [__SimpleLink&trade; LPSTK-CC1352R LaunchPad&trade;__][board] contains a
[__CC1352R1F3RGZR__][device] device and several on-board sensors:

* The [__HDC2080__][hdc2080] is an integrated humidity and temperature sensor that provides
  high accuracy measurements with very low power consumption, in a small DFN package
* The [__DRV5055__][drv5055] is a high accuracy 3.3 V or 5 V ratio-metric bipolar hall effect
  sensor that responds proportionally to magnetic flux density. The device can be used for
  accurate position sensing in a  wide range of applications.
* The [__ADXL362__][adxl362] is an ultralow power, 3-axis MEMS accelerometer that consumes
  less than 2 μA at a 100 Hz output data rate and 270 nA when in motion triggered wake-up mode,
* The [__OPT3001__][opt3001] is a sensor that measures the intensity of visible light.
  The spectral response of the sensor tightly matches the photopic response of the human eye
  and includes significant infrared rejection.

## Jumper Settings

* No modifications are needed.

## SysConfig Board File

The [LPSTK_CC1352R.syscfg.json](../.meta/LPSTK_CC1352R.syscfg.json)
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

[device]: https://www.ti.com/product/CC1352R
[board]: https://www.ti.com/tool/LPSTK-CC1352R
[hdc2080]: https://www.ti.com/product/HDC2080
[drv5055]: https://www.ti.com/product/DRV5055
[adxl362]: https://www.analog.com/en/products/adxl362.html
[opt3001]: https://www.ti.com/product/OPT3001
[boostxl-canfd-lin]: https://www.ti.com/tool/BOOSTXL-CANFD-LIN
[boostxl-sharp128]: https://www.ti.com/tool/BOOSTXL-SHARP128
[bp-bassensorsmkii]: https://www.ti.com/tool/BP-BASSENSORSMKII
