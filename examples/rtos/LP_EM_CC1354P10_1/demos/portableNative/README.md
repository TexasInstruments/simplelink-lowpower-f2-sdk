## Example Summary

This example is the same as the "portable" example but instead of using
POSIX APIs, it uses the native RTOS APIs. This example is still portable
across devices (because of TI Drivers). This project requires less code/data
space since there is no POSIX layer. This example is described in more detail
in the _SDK User's Guide_ at
&lt;SDK_INSTALL_DIR&gt;/docs/simplelink_mcu_sdk/Users_Guide.html.

## Peripherals & Pin Assignments

When this project is built, the SysConfig tool will generate the TI-Driver
configurations into the __ti_drivers_config.c__ and __ti_drivers_config.h__
files. Information on pins and resources used is present in both generated
files. Additionally, the System Configuration file (\*.syscfg) present in the
project may be opened with SysConfig's graphical user interface to determine
pins and resources used.


* `CONFIG_GPIO_LED_0` - Indicates that temperature has exceeded the alert
threshold
* `CONFIG_GPIO_BUTTON_1` - Used to restart the UART console. Not used for
CC32xx; for these devices the restart button GPIO is specified as
Power->Wakeup GPIO Source LPDS in the .syscfg file
* `CONFIG_UART2_0` - Used for a simple console
* `CONFIG_I2C_TMP` - Used to read temperature

If using the [__Building Automation Sensors BoosterPack__][bp-bassensorsmkii], `CONFIG_GPIO_TMP_EN`
is used to power on the BoosterPack's temperature sensor.

## BoosterPacks, Board Resources & Jumper Settings

* For boards with an on-board TMP sensor (e.g. CC32XX LaunchPads), the on-board
TMP sensor is used.

* For boards without an on-board TMP sensor, This example requires a
[__BP-BASSENSORSMKII BoosterPack__][bp-bassensorsmkii]. It has the TMP117
sensor along with several I2C sensors. This example may be compatible with
with previous BoosterPack revisions containing the TMP116. CC32XX LaunchPads
will default to the BoosterPack's temperature sensor if present.

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

The connection should have the following settings
```
    Baud-rate:  115200
    Data bits:       8
    Stop bits:       1
    Parity:       None
    Flow Control: None
```

* Run the example.

* The target displays a simple console on the serial session.
```
    Valid Commands
    --------------
    h: help
    q: quit and shutdown UART
    c: clear the screen
    t: display current temperature
```

* The DIE temperature is displayed. You'll need to change the temperature of
the actual TMP sensor to see a change. Please note to keep the example short
and sweet, the TMP sensor was not calibrated.

* If you quit the console, the device will go into a lower power mode.
To start the console back up, hit the restart button. Note: depending on the
TMP sensor you are using, the initial readings of the TMP sensor might be
invalid when coming out of the low power mode. Please refer to the TMP sensor
data sheet for more details.

## Application Design Details

* This example shows how to initialize the UART driver in blocking read
and write mode.

* A thread, `consoleThread`, is a simple console. When the UART is closed
the device can go into a lower power mode.

* A thread, `temperatureThread`, reads the temperature via I2C. The thread reads
the sensor every second. The timing is controlled by a timer and
semaphore. This approach was used to eliminate time drift (which would have
occurred with simply using sleep(1)).

* The disabling of interrupts/tasking (depending on the kernel you are using)
is used to protect the reading/writing of the
temperature variables. The reading/writing of these two variables must be
done atomically. For example without critical region management, the
temperature thread (which is higher priority) could interrupt the reading of
the variables by the console thread. This could result with the console thread
printing a Celsius value that did not match up with the Fahrenheit value.

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.

SimpleLink Sensor and Actuator Plugin:

* This example uses a TMP sensor. It interfaces via the I2C Driver. For a
richer API set for the TMP sensors, please refer to the SimpleLink
Sensor and Actuator Plugin.

[bp-bassensorsmkii]: http://www.ti.com/tool/bp-bassensorsmkii
