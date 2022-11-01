# rfcarrierwave

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
---------------
The carrier wave (CW) example sends a continuous carrier wave or pseudo-random
modulated signal on a fixed frequency. The frequency and other RF settings can
be modified using SmartRF Studio or the System Configuration Tool (SysConfig).

Peripherals Exercised
---------------------
N/A

Resources & Jumper Settings
---------------------------
> If you're using an IDE (such as CCS or IAR), please refer to Board.html in your project
directory for resources used and board-specific jumper settings. Otherwise, you can find
Board.html in the directory &lt;SDK_INSTALL_DIR&gt;/source/ti/boards/&lt;BOARD&gt;.

Board Specific Settings
-----------------------
1. The default frequency is:
    - 433.92 MHz for the CC1350-LAUNCHXL-433 and the CC1352P-4-LAUNCHXL
    - 2440 MHz on the CC2640R2-LAUNCHXL and CC26X2R1-LAUNCHXL
    - 868.0 MHz for other launchpads
In order to change frequency, modify the ti_radio_config.c file. This can be
done using the code export feature in Smart RF Studio, or directly in the file.
If the project is SysConfig enabled, the frequency can be adjusted in the Custom
RF Stack Module.
2. On the CC1352P1 the high PA is enabled (high output power) for all
Sub-1 GHz modes by default.
3. On the CC1352P-2 and CC1352P-4 the high PA operation for Sub-1 GHz modes is not supported
5. The CC2640R2 is setup to run all proprietary physical modes at a center
frequency of 2440 MHz, at a data rate of 250 Kbps

Example Usage
-------------
Run the example.

Application Design Details
--------------------------
This examples consists of a single task and the exported SmartRF Studio radio
settings.

To switch between carrier wave (1) and modulated signal (0) set the following
in the code (CW is set as default):

    RF_cmdTxTest.config.bUseCw = 1; // CW
    RF_cmdTxTest.config.bUseCw = 0; // modulated signal

In order to achieve +14 dBm output power, make sure that the define
CCFG_FORCE_VDDR_HH = 0x1 in ccfg.c. This requirement holds for CC13x2P boards
when using the default PA.

When the task is executed it:

1. Configures the radio for Proprietary mode
2. Explicitly configures CW (1) or Modulated (0). Default modulated mode is
   PRBS-15
3. Gets access to the radio via the RF drivers RF_open
4. Sets up the radio using CMD_PROP_RADIO_DIV_SETUP command
5. Sets the frequency using CMD_FS command
6. Sends the CMD_TX_TEST command to start sending the CW or Pseudo-random
   signal forever
