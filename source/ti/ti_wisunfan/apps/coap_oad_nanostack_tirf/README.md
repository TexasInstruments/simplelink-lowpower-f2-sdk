TI Wi-SUN FAN CoAP OAD Node Examples
==================================

Example Summary
---------------

TI Wi-SUN FAN CoAP OAD Node examples (`ns_coap_oad` in the installed `<SDK_INSTALL_DIR>/examples/rtos/<board>/ti_wisunfan`) are extensions of the CoAP Node examples. It shares all functionality with the CoAP Node examples, but also provides additional OAD (Over-the-Air Download) functionality for firmware upgrades. Refer to the README in the TI WiSUN FAN border router example for network setup and CoAP node functionality.

The CoAP OAD example requires MCUBoot, an open source bootloader, to support OAD capabilities. TI provides a fork of the open source repo that this example uses to successfully boot the CoAP OAD application and boot into subsequent firmware upgrades. Please refer to the MCUBoot project README for more details.

Two types of OAD applications are available for Wi-SUN examples. Onchip OAD allocates spare onchip flash memory as a buffer to store the incoming image firmware upgrade. Offchip OAD allocates this same buffer on offchip flash memory. On CC13X2X7 LaunchPads, the CoAP OAD example only supports offchip OAD due to memory limitations. Both onchip and offchip CoAP OAD are available on CC13X4 LaunchPads.

MCUBoot Configuration and Build
-------------------------------

### Build and Configure
The CoAP OAD example requires two images to be flashed onto the device (MCUBoot and Wi-SUN application). MCUBoot can be built from the following project: `<SDK_INSTALL_DIR>/examples/nortos/<board_type>/mcuboot_app/mcuboot`. Configure MCUboot using the MCUBoot project sysconfig file as follows:

* For both offchip and onchip OAD, verify the following configurations in the MCUBoot project:
    * For the `Upgrade Mode` configurable, choose `Overwrite`
    * In the `Image 1` module:
        * If using a CC13X2X7 platform, no changes are required.
        * If using a CC13X4 platform, modify the following configurables:
            * Primary image - Base address:     0x00006000
            * Primary image - Image size:       0x00056000
            * Secondary image - Base address:   0x0005C000 for on-chip OAD, 0x0 for off-chip OAD
            * Secondary image - Image size:     0x00056000

* For onchip OAD projects:
    * For the `Enable External Flash` configurable, choose false (unchecked)

* For offchip OAD projects:
    * For the `Enable External Flash` configurable, choose true (checked)

Application Configuration and Build
-----------------------------------

The Wi-SUN application image is the second image to be flashed to the device. This image created as part of the post build process in the `<SDK_INSTALL_DIR>/examples/rtos/board_type/ti_wisunfan/ns_coap_oad_<offchip/onchip>` project. Buildng the project with out-of-box settings will automatically create a version 1.0.0.0 Wi-SUN application binary image (`ns_coap_oad_<onchip/offchip>_<board>_<rtos>_<compiler>.bin` file). This file should be found alongside the .out and .hex files.

A no-header verison of this binary will also be found at this location, but this should only be used for reference and not used to flash the device.

To update the firmware version in the MCUBoot header, check the post-build steps (Right click project -> Properties -> Build menu -> Steps tab -> Post-build steps). By default the post-build step is the following:

`<SDK_INSTALL_DIR>/tools/common/mcuboot/imgtool sign --header-size 0x80 --align 4 --slot-size 0x56000 --pad --version 1.0.0 --pad-header --key <SDK_INSTALL_DIR>/source/third_party/mcuboot/root-ec-p256.pem ${ProjName}-noheader.bin ${ProjName}.bin`

To change the version number, update the --version 1.0.0 argument (e.g. --version 2.0.0). Note that MCUBoot will only boot into an image with a higher version number. Please refer to online MCUBoot documentation for details on how MCUBoot decides what image to boot into.

Flashing Devices
----------------

Flash MCUBoot and the Wi-SUN application image with Uniflash as follows:
1. Add the MCUBoot image built above to the image list
2. Add the Wi-SUN application image to the image list at address 0x0
   * If using a CC13X2X7 platform, flash at address 0x0
   * If using a CC13X4 platform, flash at address 0x6000
3. Load the images onto the device

Example Usage
-------------

As noted previously, the CoAP OAD node device will operate with the same functionality as the CoAP node example. Refer to the README in the TI WiSUN FAN border router example for network setup and CoAP node functionality.

OAD can be performed to upgrade the CoAP OAD node device image with an image of a higher firmware version. First, build another CoAP OAD node image with a higher firmware version (see Application Configuration and Build for how to change built image firmware version).

**Note that devices running offchip OAD images should only be upgraded with the same type of image (offchip OAD), and similarly for onchip OAD. OAD type cannot be swapped with an OAD upgrade as the MCUBoot image must be reflashed and cannot be updated with OAD.**

Form a network between a Wi-SUN border router device and the CoAP OAD device. In the pySpinel python application of the border router, three OAD commands are enabled: `getoadfwver`, `startoad`, and `getoadstatus`. `getoadfwver` will query for the firmware version of a connected CoAP OAD device. `startoad` can be used to upgrade the firmware of a connected CoAP OAD device. `getoadstatus` will get the current block transfer status of the active OAD session. Note that you can perform other commands like ping and CoAP requests while OAD is in progress. OAD progress will be logged in the specified logfile. For more information, refer to the README at https://github.com/TexasInstruments/ti-wisunfan-pyspinel or type `help <command>` to get more information on the comand parameters.

The typical upgrade process is as follows:
1. Query the firmware version of the CoAP OAD device with the `getoadfwver` command. Verify your upgrade image has a higher firmware version
2. Perform OAD firmware upgrade with the `startoad` command. Wait for OAD completion. Use the `getoadstatus` command to get the current block transfer status.
3. After OAD completion, wait for the CoAP OAD device to reconnect to the network. After reconnection, perform another firmware version query with getoadfwver to confirm the upgrade was successful.

