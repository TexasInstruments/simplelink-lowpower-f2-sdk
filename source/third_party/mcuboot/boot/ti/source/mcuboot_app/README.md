# Texas Instruments MCUBOOT Example Quick-start Guide

## Disclaimer
Mcuboot is a open source bootloader. This example is the result of Texas Instruments forking the original [mcuboot repo](https://github.com/mcu-tools/mcuboot) and providing support for [CC13XX_CC26XX](https://www.ti.com/wireless-connectivity/overview.html) class devices.
The following information is for reference only. In a production environment, the end user is must to do the proper modifications that best serve the needs of the particular application. 

## Configuration (mcuboot_config.h)
This release supports two configurations. See more details and change the current configuration within SysConfig.

* `MCUBOOT_DIRECT_XIP` (default):
In this configuration mcuboot runs an image directly from either the primary or the secondary slot (without moving and or copying it into the primary slot). In other words, mcuboot e**X**ecutes **I**n **P**lace (**XIP**).
The image update client, which downloads the new images must be aware which slot contains the active image and which acts as a staging area.
In addition, the image update client is responsible for loading the proper images into the proper slot. Because mcuboot runs the image directly from the
slot it resides in, the image must be compiled/linked to execute within the slot it will be programmed to.
</br></br>
At boot time the mcuboot first looks for images in both slots and then inspects the version numbers in the image headers.
It selects the newest image (with the highest version number) and then checks its validity (integrity check, signature verification etc.). If the image 
is invalid, MCUboot erases its memory slot and starts to validate the other image. After a successful validation of the selected image the mcuboot executes it.
This mode only allows `MCUBOOT_IMAGE_NUMBER = 1`.

* `MCUBOOT_OVERWRITE_ONLY`:
In this configuration mcuboot only runs an image from the primary slot. If during boot time the mcuboot identifies the secondary slot to contain the image it needs to execute.
Mcuboot will first copy the image from the secondary to the primary slot, as long as the image in secondary slot has a version higher or equal to the one in primary. Afterward it will erase the secondary slot. This mode supports `MCUBOOT_IMAGE_NUMBER = 1` or `MCUBOOT_IMAGE_NUMBER = 2`. See [Image Slots](#image-slots) section for further explaination.

Note: Mcuboot also supports swap-based image upgrades. This is not currently supported by the TI but is planned in future releases.

## HW Antirollback Protection

MCUBoot has a hardware-based downgrade prevention feature by using a security counter that is stored in each image's protected TLV area. If MCUBoot is built with `MCUBOOT_HW_ROLLBACK_PROT` configuration enabled
in mcuboot_config.h, in the project post-build steps for the target application, add option `-s <val>`, where `<val>` specifies the value of the security counter for the image. In this scenario, besides comparing
the version stored in the header of each image, MCUBoot will compare the value of the image security counter against the current security counter stored in the last sector of the MCUBoot region, and accept the new
image if its security counter has the same or higher value.</br>

**Note**: This feature is not supported in CC13x2x7/CC26x2x7 devices.

## Image Slots
The flash memory is partitioned into two image slots: a primary slot and a secondary slot. Each slot must have a fixed location and associated length, which is set up in SysConfig.
Default values are provided, but user is free to modify the partitions defined for a particular device if needed. Note that the address of a partition is required to be aligned to a sector boundary, and its length must be multiple
of sector size. 

There is also an option to configure `MCUBOOT_IMAGE_NUMBER = 2` in mcuboot_config.h to enable dual primary and secondary slots; that is, 2 primary slots, and 2 secondary slots. This is required for applications that make use of secure features.
In such case, primary 1 is for the secure application, and primary 2 for non secure. If an upgrade is to occur, then the secondary 1 image will overwrite primary 1, and secondary 2 will overwrite
primary 2. Then, if both primary 1 and 2 are successfully verified, MCUBoot will proceed to boot primary 1. 


## How to build an image that is compatible with MCUBOOT

Refer to example `mcuboot_blinky` for different linker definitions depending on the current device and toolchain in use. 

* Linker variable `FLASH_BASE` controls the address for which the application will be linked for. `MCUBOOT_HDR_BASE` defines the start address of the MCUBoot image header.
In the examples provided in this SDK, a length of 0x80 is used for the MCUBoot header. 

* If application needs to access the contents of the MCUBoot header, add the following code as shown below. Note that symbol `MCUBOOT_HDR_BASE` has been defined in the linker file, and is accessed by the C source code to initialize 
the address of variable `mcubootHdr` so that it can be used to access the current application's MCUBoot header. 
</br>

```
 #include "bootutil/bootutil.h"
 #include "bootutil/image.h"

extern int MCUBOOT_HDR_BASE;
struct image_header *mcubootHdr = (struct image_header *)&MCUBOOT_HDR_BASE;
```
* Add post processing in CCS

    * Extract the binary image `<proj_name>_noheader.bin` from ELF file `<proj_name>.out`.       
    * Process `<proj_name>_noheader.bin` with imgtool (`${COM_TI_SIMPLELINK_SDK_INSTALL_DIR}/tools/common/mcuboot/imgtool.exe`) to get `<proj_name>.bin`, which is the compatible MCUBoot image format.

See example below:
</br></br>NOTE: The key root-ec-p256.pem is for example use only. Provide your own key for your application.
	
```
 ${CG_TOOL_ROOT}/bin/arm-none-eabi-objcopy ${ProjName}.out -O binary ${ProjName}-noheader.bin
 ${COM_TI_SIMPLELINK_SDK_INSTALL_DIR}/tools/common/mcuboot/imgtool sign --header-size 0x80 --align 4 --slot-size 0x2A000 --version 2.0.0 --pad-header --key ${COM_TI_SIMPLELINK_SDK_INSTALL_DIR}/source/third_party/mcuboot/root-ec-p256.pem ${ProjName}-noheader.bin ${ProjName}.bin
```

Refer to example `mcuboot_blinky` for different post-build steps depending on the current device and toolchain in use.

See [MCUBoot Project imgtool](https://github.com/mcu-tools/mcuboot/blob/main/docs/imgtool.md) description for further reference on how to create an image compatible with MCUBoot.

## MCUBoot Image Upgrade in OVERWRITE mode for devices that support TZ_Enabled and TZ_Disabled configurations

For 1 upgradeable image, MCUBoot will overwrite the Primary slot image with that one from Secondary slot if:

* Secondary image is valid and Primary image is invalid.
* Secondary image is valid and its version is higher or equal than that in Primary.

MCUBoot will attempt to boot Primary after performing an update, if any. 

**NOTE**: When using the **TZ Disabled** build configuration, make sure to set **Address of Flash Vector Table** to 0x00000000, under SysConfig **Device Configuration**.


For 2 upgradeable images:

The same procedure as for 1 upgradeable image described above is followed, but in this case Primary 0 is compared against Secondary 0, and Primary 1 will be compared against Secondary 1.

MCUBoot will attempt to boot Primary 0 after performing an update, if any.

**NOTE 1**: When using the **TZ Enabled** build configuration, make sure to set **Address of Flash Vector Table** to 0x00000800, under SysConfig **Device Configuration**.</br>
**NOTE 2**: The SDK includes the **Secure Image** ELF file (tfm_s.axf) and a **Non-Secure example** project called tfm_aescbc, which can be imported into CCS, so that both secure and non-secure images be used in MCUBoot 2-image upgrade mode. </br>
**NOTE 3**: HW Antirollback Protection is not supported in this mode.</br></br>

Per section **How to build an image that is compatible with MCUBOOT** above, find file `${COM_TI_SIMPLELINK_SDK_INSTALL_DIR}/tfm_s/build/cc26x4/production_full/tfm_s.axf` and run the following steps:<br><br>

```
${CG_TOOL_ROOT}/bin/arm-none-eabi-objcopy -O binary tfm_s.axf tfm_s-noheader.bin
${COM_TI_SIMPLELINK_SDK_INSTALL_DIR}/tools/common/mcuboot/imgtool.exe sign --header-size 0x100 --align 4 --slot-size 0x2b000 --version 1.0.0 --pad-header --pad --key ${COM_TI_SIMPLELINK_SDK_INSTALL_DIR}/source/third_party/mcuboot/root-ec-p256.pem tfm_s-noheader.bin tfm_s.bin

${CG_TOOL_ROOT}/bin/arm-none-eabi-objcopy tfm_aescbc.out -O binary tfm_aescbc-noheader.bin --remove-section=.ccfg
${COM_TI_SIMPLELINK_SDK_INSTALL_DIR}/tools/common/mcuboot/imgtool.exe sign --header-size 0x100 --align 4 --slot-size 0x4e800 --version 1.0.0 --pad-header --pad --key ${COM_TI_SIMPLELINK_SDK_INSTALL_DIR}/source/third_party/mcuboot/root-ec-p256.pem tfm_aescbc-noheader.bin tfm_aescbc.bin
```
The resulting files tfm_s.bin and tfm_aescbc.bin can now be flashed in slots Primary 1 and 2, accordingly.

## Example Usage

This project is an example implementation of MCUBoot while `mcuboot_blinky` is an example implementation of an application to be programmed and run by MCUBoot. If MCUBoot finds a valid image to boot, it will blink the Green LED 3 times and the proceed to boot the target image. Otherwise, it will blink the Red LED repeatedly.

Refer to SDK example `mcuboot_blinky` for additional details.

