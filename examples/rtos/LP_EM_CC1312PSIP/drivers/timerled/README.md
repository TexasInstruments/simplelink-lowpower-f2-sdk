## Example Summary

Sample application to periodically toggle an LED based off a timer.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (\*.syscfg) present in the
project may be opened with SysConfig's graphical user interface to determine
pins and resources used.

* `CONFIG_TIMER_0` - Timer instance that toggles the LED.
* `CONFIG_GPIO_LED_0` - LED toggled in the callback function.

## BoosterPacks, Board Resources & Jumper Settings

For board specific jumper settings, resources and BoosterPack modifications,
refer to the __Board.html__ file.

> If you're using an IDE such as Code Composer Studio (CCS) or IAR, please
refer to Board.html in your project directory for resources used and
board-specific jumper settings.

The Board.html can also be found in your SDK installation:

        <SDK_INSTALL_DIR>/source/ti/boards/<BOARD>


## Example Usage

* The example performs general initialization in `mainThread`.

## Application Design Details

This application uses one thread, `mainThread`. The timer is operating in
Timer_CONTINUOUS_CALLBACK mode which causes the callback function,
`timerCallback`, to be called at a rate specified by `period`. A `period` of
1,000,000 microseconds or 1 second is used. Because the LED is toggled each
time `timerCallback` is called, the observed frequency the LED blinks is once
every 2 seconds.

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
