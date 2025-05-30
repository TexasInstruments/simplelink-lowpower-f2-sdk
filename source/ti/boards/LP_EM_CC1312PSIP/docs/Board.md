# SimpleLink&trade; CC1312PSIP LaunchPad&trade; Settings & Resources

The [__SimpleLink&trade; CC1312PSIP LaunchPad&trade;__][board] contains a
[__CC1312PSIP__][device] device.

## Jumper Settings

* Close the __`LEDs`__ jumpers to enable the on-board LEDs.

## SysConfig Board File

The [LP_EM_CC1312PSIP.syscfg.json](../.meta/LP_EM_CC1312PSIP.syscfg.json)
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

* BoosterPack pin header BP.8 (__`SD_CS`__) must be jumpered to BP.28 when using the SD card.

### [__BP-BASSENSORSMKII BoosterPack__][bp-bassensorsmkii]

* No modifications are needed.

### [__CC3200 Audio BoosterPack__][cc3200audboost]

* No modifications are needed.

[device]: https://www.ti.com/product/CC1312PSIP
[board]: https://www.ti.com/tool/LP-EM-CC1312PSIP
[boostxl-sharp128]: https://www.ti.com/tool/BOOSTXL-SHARP128
[bp-bassensorsmkii]: https://www.ti.com/tool/BP-BASSENSORSMKII
[cc3200audboost]: https://www.ti.com/tool/CC3200AUDBOOST
