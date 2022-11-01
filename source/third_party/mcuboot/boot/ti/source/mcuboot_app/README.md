# Texas Instruments MCUBOOT Example Quick-start Guide

## Disclaimer
Mcuboot is a open source bootloader. This example is the result of Texas Instruments forking the original [mcuboot repo](https://github.com/mcu-tools/mcuboot) and providing support for [CC13XX_CC26XX](https://www.ti.com/wireless-connectivity/overview.html) class devices.
The following information is for reference only. In a production environment, the end user is must to do the proper modifications that best serve the needs of the particular application. 

## Configuration (mcuboot_config.h)
This release supports two configurations. See more details and change the current configuration within the `<example_root>/mcuboot_config/mcuboot_config.h` file.

1. `MCUBOOT_DIRECT_XIP` (default):
In this configuration mcuboot runs an image directly from either the primary or the secondary slot (without moving and or copying it into the primary slot). In other words, mcuboot e**X**ecutes **I**n **P**lace (**XIP**).
The image update client, which downloads the new images must be aware which slot contains the active image and which acts as a staging area.
In addition, the image update client is responsible for loading the proper images into the proper slot. Because mcuboot runs the image directly from the
slot it resides in, the image must be compiled/linked to execute within the slot it will be programmed to.
</br></br>
At boot time the mcuboot first looks for images in both slots and then inspects the version numbers in the image headers.
It selects the newest image (with the highest version number) and then checks its validity (integrity check, signature verification etc.). If the image 
is invalid, MCUboot erases its memory slot and starts to validate the other image. After a successful validation of the selected image the mcuboot executes it.

1. `MCUBOOT_OVERWRITE_ONLY`:
In this configuration mcuboot only runs an image from the primary slot. If during boot time the mcuboot identifies the secondary slot to contain the image it needs to execute.
Mcuboot will first copy the image from the secondary to the primary slot. Afterward it will erase the secondary page to keep it clean.


Note: Mcuboot also supports swap-based image upgrades. This is not currently supported by the TI but is planned in future releases.

## Image Slots (flash_map_backend.c)
The flash memory is partitioned into two image slots: a primary slot and a secondary slot. Each slot must have a fixed location, which is set up in the `<example_root>/flash_map_backend/flash_map_backend.c` file.
See the image slot configuration below, configured according to the device in use.

```

#if defined(DeviceFamily_CC13X2) || defined(DeviceFamily_CC26X2)
    #define BOOTLOADER_BASE_ADDRESS     0x00054000
    #define BOOT_BOOTLOADER_SIZE        0x0003FA8
    #define BOOT_PRIMARY_1_SIZE         0x2A000
    #define BOOT_SECONDARY_1_SIZE       0x2A000
#elif defined(DeviceFamily_CC13X2X7) || defined(DeviceFamily_CC26X2X7)
    #define BOOTLOADER_BASE_ADDRESS     0x000AC000
    #define BOOT_BOOTLOADER_SIZE        0x0003FA8
    #define BOOT_PRIMARY_1_SIZE         0x56000
    #define BOOT_SECONDARY_1_SIZE       0x56000
#else
    #define BOOTLOADER_BASE_ADDRESS     0
    #define BOOT_BOOTLOADER_SIZE        0x06000
    #define BOOT_PRIMARY_1_SIZE         0x7D000
    #define BOOT_SECONDARY_1_SIZE       0x7D000
#endif
```

## How to build an image that is compatible with MCUBOOT
1. Apply the image location (primary or secondary slot) into your application's linker file. Assuming a CC13X2 is being used, see an example shown below (make sure to leave at least 32 bytes for MCUBOOT header).
Note that symbol `PRIMARY_SLOT` is used to control the linking address, depending if the target application is intended to be executed from the primary or secondary slot. 
To link against the secondary slot address, just remove the `PRIMARY_SLOT = 1;` symbol declaration.
 
```
PRIMARY_SLOT = 1;
SLOT_SIZE = 0x0002A000;
MCUBOOT_HDR_BASE = DEFINED(PRIMARY_SLOT) ? 0x00000000 : SLOT_SIZE;

MCUBOOT_HDR_SIZE = 0x80;
FLASH_BASE = MCUBOOT_HDR_BASE + MCUBOOT_HDR_SIZE;
FLASH_LENGTH = SLOT_SIZE - MCUBOOT_HDR_SIZE;
```
</br></br>
```
 MEMORY
 {
    /* Application stored in and executes from internal flash */

    FLASH (RX) : origin = FLASH_BASE,         length = FLASH_LENGTH
    ...
 }
```

2. Add the following code as shown below. Note that symbol `MCUBOOT_HDR_BASE` has been defined in the linker file, and is accessed by the C source code to initialize 
the address of variable `mcubootHdr` so that it can be used to access the current application's MCUBoot header. 
</br>
```
 #include "bootutil/bootutil.h"
 #include "bootutil/image.h"

extern int MCUBOOT_HDR_BASE;
struct image_header *mcubootHdr = (struct image_header *)&MCUBOOT_HDR_BASE;
```

3. Add post processing in CCS

      1. convert `<proj_name>.out` to `<proj_name>_noheader.bin`
      1. convert `<proj_name>_noheader.bin` to `<proj_name>.bin` with MCUBOOT header info using MCUBOOT image tool (`<sdk_root>/source/third_party/mcuboot/scripts/imgtool.py`)

See example below:
	
```
 ${CG_TOOL_ROOT}/bin/armobjcopy ${ProjName}.out --output-target binary ${ProjName}-noheader.bin
 python ${COM_TI_SIMPLELINK_CC13X2_26X2_SDK_154_DEV_INSTALL_DIR}/source/third_party/mcuboot/scripts/imgtool.py sign --header-size 0x80 --align 4 --slot-size 0x2A000 --version 2.0.0 --pad-header ${ProjName}-noheader.bin ${ProjName}.bin
```

See [MCUBoot Project imgtool](https://github.com/mcu-tools/mcuboot/blob/main/docs/imgtool.md) description for further reference on how to create an image compatible with MCUBoot.
