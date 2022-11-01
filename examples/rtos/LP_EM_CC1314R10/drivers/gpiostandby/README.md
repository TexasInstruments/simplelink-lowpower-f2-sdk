## Example Summary

This example will cycle the device between active and standby states. The CPU
will toggle the LEDs each time it wakes up from standby, using the GPIO driver,
and then immediately return to standby.

## Peripherals Exercised

* `CONFIG_LED_0` - Toggled by Device
* `CONFIG_LED_0` - Toggled by Device

## Resources & Jumper Settings

> If you're using an IDE (such as CCS or IAR), please refer to Board.html in
your project directory for resources used and board-specific jumper settings.
Otherwise, you can find Board.html in the directory
&lt;SDK_INSTALL_DIR&gt;/source/ti/boards/&lt;BOARD&gt;.


## Example Usage

* Run the example.

* The device wakes up and toggles the LEDs at periodic intervals.
Modify the `standbyDuration` to change how long the device stays in standby,
the default is 5s.

>If the `standbyDuration` is set to < 1ms, the power policy will
choose to enter idle instead of standby, since it won't have sufficient time
to enter/exit standby.

## Application Design Details

* The example consists of one task which periodically sleeps and wakes.
Since no other work is scheduled in the application, the power policy
(run when the CPU is idle) will choose to put the device into standby during
the duration of the task sleep.

>If you are measuring power consumption during standby, it will be higher when
the LEDs are on, and will be much lower when the LEDs are off.

>Note that in order to get lowest standby current, the JTAG pins TMS and TCK
should be disconnected after programming the device. TMS and TCK have internal
pull-ups and are driven low when inactive from the emulator, hence after
programming the device these jumpers should be removed to avoid the extra
leakage current.

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
