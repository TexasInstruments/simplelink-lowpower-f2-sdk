TI Wi-SUN FAN RCP example
================================================================

Example Summary
---------------

TI Wi-SUN RCP (Radio Co-processor) examples are specific examples meant to work in conjunction with a Wi-SUN Linux Host example to operate as a full Wi-SUN Border Router. This configuration offloads the majority of the WI-SUN stack to the Linux host, leaving only radio/PHY layer, frequency hopping, and encryption/decryption functionality in the RCP embedded system. A Border Router configured in this manner expands the memory available to the host Wi-SUN stack, allowing for large network support up to 1000+ nodes. For detailed instructions of how to setup the Linux Host example together with the RCP example, please refer to the Linux host setup documentation in the TI-wfantund GitHub page: (https://github.com/TexasInstruments/ti-wisunfantund)

Steps for building the examples
--------------------------------

Currently the projects are provided for CCS and IAR tool chain. 
No additional steps need to be taken while importing the projects. CCS will recognize the SDK installed automatically.

Peripherals Exercised
---------------------

> UART is used for communicating with the Linux host for the RCP example. The following UART settings are set by the example code, during initialization:
```
    Baud-rate:     115200
    Data bits:          8
    Stop bits:          1
    Parity:          None
    Flow Control:    None
```

Configuration using SysConfig
-----------------------------

To configure settings different from the Out of Box settings, System Configuration Tool (SysConfig) should be used as follows.
* Double click on .syscfg file from the application in the IDE to launch the GUI of SysConfig. CCS IDE has SysConfig Tool integrated into it and thus, will automatically open the file.
* Following are the Out of Box Settings for some the parameters. Please refer to the SysConfig file for the full list of parameters configurable through SysConfig GUI.

* Important note: The RCP example shares some SysConfig configurations of WI-SUN parameters with Linux Host examples. For Linux Host to be compatible with RCP example, you must use the same parameters for both. These parameters are as follows:
    * Region/Regulatory Domain
    * PHY ID
    * Operating Mode ID/Class
    * Channel Plan ID

Secure Boot Configuration
-------------------------

All non-OAD projects can be configured to be built as a secure boot project. This will configure the project to be built into a `<project_name>.bin` file, meant to be securely booted into via MCUBoot. To configure a project to be build in this mode, change the project build configuration of the imported project to the SecureBoot configurationbefore building the project. This can be done in both CCS and IAR IDEs. A sysconfig option for secure boot configuration is present in current projects, but is only used to inform users to change the project build configuration.

Similar to OAD projects, the secure boot project must be booted into with the MCUBoot bootloader already flashed onto the device. MCUBoot can be built from the following project: `<SDK_INSTALL_DIR>/examples/nortos/<board_type>/mcuboot_app/mcuboot`. Configure MCUboot using the MCUBoot project sysconfig file as follows:

* For the `Upgrade Mode` configurable, choose `Overwrite`
* For the `Enable External Flash` configurable, choose false (unchecked)
* In the `Image 1` module:
    * If using a CC13X2X7 platform, modify the following configurables:
        * Primary image - Base address:     0x00000000
        * Primary image - Image size:       0x000A8000
        * Secondary image - Base address:   0x000A8000
        * Secondary image - Image size      0x00000000
    * If using a CC13X4 platform, modify the following configurables:
        * Primary image - Base address:     0x00006000
        * Primary image - Image size:       0x000A8000
        * Secondary image - Base address:   0x000AE000
        * Secondary image - Image size      0x00000000

Once both MCUBoot is configured and flashed on the device, the Wi-SUN secure boot project image (.bin file) must be loaded onto the device. This file can either be flashed to the device with CCS/TI-flash or with a serial bootloader application. Currently, the 15.4 stack Linux Gateway provides a serial bootloader application to faciliate this. Please refer to the 15.4 documentation on how to download and use the serial bootloader.

Whether flashing normally or using the serial bootloader application, you must specify a specific starting address depending on the platform used.
    * If using a CC13X2X7 platform, you must load the .bin file to address 0x0.
    * If using a CC13X4 platform, you must load the .bin file to address 0x6000.

With both MCUBoot and the Wi-SUN secure boot image loaded, the device should successfully boot into the Wi-SUN application via MCUBoot.

Example Usage
-------------

The Wi-SUN RCP example works in conjunction with the Linux host. Refer to the wfantund repo (https://github.com/TexasInstruments/ti-wisunfantund) for setup details. For the RCP setup example, make sure to configure Radio settings as specified in the SysConfig section above. Once configuration is done, simply build, flash to LaunchPad, and connect the LaunchPad to the Linux host via USB.