## Example Summary

This application moves into and out of shutdown. The state is controlled by
buttons. `CONFIG_BUTTON_0` will send the device into shutdown, while
`CONFIG_GPIO_WAKEUP_BUTTON` will wake-up the device. A special LED toggling
sequence is used when waking from shutdown to demonstrate the get-reset-source
functionality.

## Peripherals Exercised

* `CONFIG_BUTTON_0` - Used to tell the device to enter shutdown.
* `CONFIG_GPIO_WAKEUP_BUTTON` - Used as a wake-up source.
* `CONFIG_LED_0` -  Lit when the device is active, not lit when in shutdown.
* `CONFIG_LED_1` -  Will blink when coming out of shutdown.

## BoosterPacks, Board Resources & Jumper Settings

For board specific jumper settings, resources and BoosterPack modifications,
refer to the __Board.html__ file.

> If you're using an IDE such as Code Composer Studio (CCS) or IAR, please
refer to Board.html in your project directory for resources used and
board-specific jumper settings.

The Board.html can also be found in your SDK installation:

        <SDK_INSTALL_DIR>/source/ti/boards/<BOARD>

> In order to get lowest shutdown current, the JTAG pins TMS and TCK should be
disconnected by removing the relevant jumpers. TMS and TCK have internal pull-up
and are driven low when inactive from the emulator, hence after programming the
device these jumpers should be removed to avoid the extra leakage current.

## Example Usage

* Run the example. Use `CONFIG_BUTTON_0` and `CONFIG_GPIO_WAKEUP_BUTTON` to
shutdown and wake-up the device. Try the RESET button to compare the different
start-up sequences used.

> It is not possible to do a proper shutdown sequence with the
debugger connected. For correct behaviour, this example must be run with the
debugger disconnected by resetting or performing a power-cycle of the device.

## Application Design Details

* The example consists of one task which waits for a button press and then calls
`Power_shutdown()`. Since no other power resources are active, the device will
shutdown immediately and then reset on the opposite button press.
* Demonstrates the use of `Power_getResetSource()` to determine the previous
reset source, and blinks an LED if we reset due to shutdown.

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
