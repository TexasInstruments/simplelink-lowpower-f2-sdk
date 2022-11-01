## Example Summary

Application that registers temperature based notifications and toggles LEDs.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (\*.syscfg) present in the
project may be opened with SysConfig's graphical user interface to determine
pins and resources used.

* `CONFIG_GPIO_LED_0` - Toggled by Temperature driver notification
* `CONFIG_GPIO_LED_1` - Turned on or off by Temperature driver notification

## BoosterPacks, Board Resources & Jumper Settings

For board specific jumper settings, resources and BoosterPack modifications,
refer to the __Board.html__ file.

> If you're using an IDE such as Code Composer Studio (CCS) or IAR, please
refer to Board.html in your project directory for resources used and
board-specific jumper settings.

The Board.html can also be found in your SDK installation:

        <SDK_INSTALL_DIR>/source/ti/boards/<BOARD>


## Example Usage

* Open a serial session (e.g. [`PuTTY`](http://www.putty.org/ "PuTTY's
 Homepage"), etc.) to the appropriate COM port.
 * The COM port can be determined via Device Manager in Windows or via
 `ls /dev/tty*` in Linux.

The connection will have the following settings:
```
    Baud-rate:     115200
    Data bits:          8
    Stop bits:          1
    Parity:          None
    Flow Control:    None
```

* Run the example.

* `CONFIG_GPIO_LED_0` is toggled when the temperature of the chip changes more
than THRESHOLD_DELTA degrees C.

* `CONFIG_GPIO_LED_1` is turned on when the temperature is above
THRESHOLD_CUTOFF degrees C and turned off when the temperature falls below this
threshold.

* The current temperature in degrees C is printed to the console once per
second.

## Application Design Details

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
