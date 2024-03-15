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
This mode only allows `MCUBOOT_IMAGE_NUMBER = 1`.

1. `MCUBOOT_OVERWRITE_ONLY`:
In this configuration mcuboot only runs an image from the primary slot. If during boot time the mcuboot identifies the secondary slot to contain the image it needs to execute.
Mcuboot will first copy the image from the secondary to the primary slot. Afterward it will erase the secondary page to keep it clean. This mode supports `MCUBOOT_IMAGE_NUMBER = 1` or `MCUBOOT_IMAGE_NUMBER = 2`. See [Image Slots](#image-slots-flashmapbackendc) section for further explaination.

Note: Mcuboot also supports swap-based image upgrades. This is not currently supported by the TI but is planned in future releases.

## HW Antirollback Protection

MCUBoot has a hardware-based downgrade prevention feature by using a security counter that is stored in each image's protected TLV area. If MCUBoot is built with `MCUBOOT_HW_ROLLBACK_PROT` configuration enabled
in mcuboot_config.h, in the project post-build steps for the target application, add option `-s <val>`, where `<val>` specifies the value of the security counter for the image. In this scenario, besides comparing
the version stored in the header of each image, MCUBoot will compare the value of the image security counter against the current security counter stored in the last sector of the MCUBoot region, and accept the new
image if its security counter has the same or higher value.</br>

**Note**: This feature is not supported in CC13x2x7/CC26x2x7 devices.

## Image Slots (flash_map_backend.c)
The flash memory is partitioned into two image slots: a primary slot and a secondary slot. Each slot must have a fixed location, which is set up in the `<example_root>/flash_map_backend/flash_map_backend.c` file.
See the image slot configuration below, configured according to the device in use.

There is also an option to configure `MCUBOOT_IMAGE_NUMBER = 2` in mcuboot_config.h to enable dual primary and secondary slots; that is, 2 primary slots, and 2 secondary slots. This is required for applications that make use of secure features.
In such case, primary 1 is for the secure application, and primary 2 for non secure. If an upgrade is to occur, then the secondary 1 image will overwrite primary 1, and secondary 2 will overwrite
primary 2. Then, if both primary 1 and 2 are successfully verified, MCUBoot will proceed to boot primary 1. 

This is an example of how to use multiple images, but in general, MCUBoot is agnostic of the contents of the slots, so the user is free to use secure or non secure images in any slot, to fit its 
particular needs. 

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
#elif defined DeviceFamily_CC23X0R5
    #define BOOTLOADER_BASE_ADDRESS     0
    #define BOOT_BOOTLOADER_SIZE        0x06000
    #define BOOT_PRIMARY_1_SIZE         0x3d000
    #define BOOT_SECONDARY_1_SIZE       0x3d000
#else

    #if (MCUBOOT_IMAGE_NUMBER == 2)

        #define BOOTLOADER_BASE_ADDRESS         0x00800
        #define BOOT_BOOTLOADER_SIZE            0x06000

        #define BOOT_PRIMARY_1_BASE_ADDRESS     0x06800
        #define BOOT_PRIMARY_1_SIZE             0x2b000

        #define BOOT_PRIMARY_2_BASE_ADDRESS     0x38000
        #define BOOT_PRIMARY_2_SIZE             0x4e800

        #define BOOT_SECONDARY_1_BASE_ADDRESS   0x86800
        #define BOOT_SECONDARY_1_SIZE           0x2b000

        #define BOOT_SECONDARY_2_BASE_ADDRESS   0xb1800
        #define BOOT_SECONDARY_2_SIZE           0x4e800

    #else

        #define BOOTLOADER_BASE_ADDRESS     0
        #define BOOT_BOOTLOADER_SIZE        0x06000
        #define BOOT_PRIMARY_1_SIZE         0x2b000
        #define BOOT_SECONDARY_1_SIZE       0x2b000

    #endif
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
      1. convert `<proj_name>_noheader.bin` to `<proj_name>.bin` with MCUBOOT header info using MCUBOOT imgtool (`<SDK_DIR>/tools/common/mcuboot/imgtool.exe`)

See example below:
</br></br>NOTE: The key root-ec-p256.pem is for example use only. Provide your own key for your application.
	
```
 ${CG_TOOL_ROOT}/bin/arm-none-eabi-objcopy ${ProjName}.out -O binary ${ProjName}-noheader.bin
 <SDK_DIR>/tools/common/mcuboot/imgtool.exe sign --header-size 0x80 --align 4 --slot-size 0x2A000 --version 2.0.0 --pad-header --key root-ec-p256.pem ${ProjName}-noheader.bin ${ProjName}.bin
```

See [MCUBoot Project imgtool](https://github.com/mcu-tools/mcuboot/blob/main/docs/imgtool.md) description for further reference on how to create an image compatible with MCUBoot.

## MCUBoot Image Upgrade Permutations in OVERWRITE mode

For 1 upgradeable image:

**NOTE**: When using the **TZ Disabled** build configuration, make sure to set **Address of Flash Vector Table** to 0x00000000, under syscfg **Device Configuration**.

Images: 1, slots: 2, states: 4, permutations: 16

Conventions:
<br>&emsp;First half of slots are Primary; second half are Secondary.
<br>&emsp;For instance, when 2 images are used, 4 slots are needed: slots 0 and 1 are Primary; slots 2 and 3 are Secondary
<br>

Symbol terminology:<br>
&emsp;Empty     : Slot is clear (blank).
<br>&emsp;Corrupted : Slot has a corrupted or invalid image.
<br>&emsp;A         : Slot has a valid image.
<br>&emsp;B         : Slot has a valid and newer version image
<br>&emsp;NU        : No update will occur.
<br>&emsp;UP        : Update occurred.
<br>&emsp;FAIL      : Primary image will not be booted.
<br>&emsp;BOOT      : Primary image will be booted.
<br>&emsp;[a, b]    : Comparing slot index 'a' with slot index 'b'.
<br>


<style type="text/css">
table, th, td {
  border: 1px solid black;
}
</style>

| # | Slot 0 (Prim) before | Slot 1 (Sec) before | Boot status | Slot 0 (Prim) after | Slot 1 (Sec) after |
|---|---|---|---|---|---|
|   0 |  Empty     | Empty     | [0, 1]  NU  FAIL | Empty     | Empty     |
|   1 |  A         | Empty     | [0, 1]  NU  BOOT | A         | Empty     |
|   2 |  B         | Empty     | [0, 1]  NU  BOOT | B         | Empty     |
|   3 |  Corrupted | Empty     | [0, 1]  NU  FAIL | Empty     | Empty     |
|   4 |  Empty     | A         | [0, 1]  UP  BOOT | A         | Empty     |
|   5 |  A         | A         | [0, 1]  NU  BOOT | A         | Empty     |
|   6 |  B         | A         | [0, 1]  NU  BOOT | B         | Empty     |
|   7 |  Corrupted | A         | [0, 1]  UP  BOOT | A         | Empty     |
|   8 |  Empty     | B         | [0, 1]  UP  BOOT | B         | Empty     |
|   9 |  A         | B         | [0, 1]  UP  BOOT | B         | Empty     |
|  10 |  B         | B         | [0, 1]  NU  BOOT | B         | Empty     |
|  11 |  Corrupted | B         | [0, 1]  UP  BOOT | B         | Empty     |
|  12 |  Empty     | Corrupted | [0, 1]  NU  FAIL | Empty     | Empty     |
|  13 |  A         | Corrupted | [0, 1]  NU  BOOT | A         | Empty     |
|  14 |  B         | Corrupted | [0, 1]  NU  BOOT | B         | Empty     |
|  15 |  Corrupted | Corrupted | [0, 1]  NU  FAIL | Empty     | Empty     |

For 2 upgradeable images:

**NOTE 1**: When using the **TZ Enabled** build configuration, make sure to set **Address of Flash Vector Table** to 0x00000800, under syscfg **Device Configuration**.</br>
**NOTE 2**: The SDK includes the **Secure Image** ELF file (tfm_s.axf) and a **Non-Secure example** project called tfm_aescbc, which can be imported into CCS, so that both secure and non-secure images be used in MCUBoot 2-image upgrade mode. </br>
**NOTE 3**: HW Antirollback Protection is not supported in this mode.</br></br>

Per section **How to build an image that is compatible with MCUBOOT** above, find file `<SDK_DIR>/tfm_s/build/cc26x4/production_full/tfm_s.axf` and run the following steps:<br><br>

```
${CG_TOOL_ROOT}/bin/arm-none-eabi-objcopy -O binary tfm_s.axf tfm_s-noheader.bin
<SDK_DIR>/tools/common/mcuboot/imgtool.exe sign --header-size 0x100 --align 4 --slot-size 0x2b000 --version 1.0.0 --pad-header --pad --key root-ec-p256.pem tfm_s-noheader.bin tfm_s.bin

${CG_TOOL_ROOT}/bin/arm-none-eabi-objcopy tfm_aescbc.out -O binary tfm_aescbc-noheader.bin --remove-section=.ccfg
<SDK_DIR>/tools/common/mcuboot/imgtool.exe sign --header-size 0x100 --align 4 --slot-size 0x4e800 --version 1.0.0 --pad-header --pad --key root-ec-p256.pem tfm_aescbc-noheader.bin tfm_aescbc.bin
```
The resulting files tfm_s.bin and tfm_aescbc.bin can now be flashed in slots Primary 1 and 2, accordingly.

Images: 2, slots: 4, states: 4, permutations: 256

<style type="text/css">
table, th, td {
  border: 1px solid black;
}
</style>

| # | Slot 0 (Prim) before | Slot 1 (Prim) before | Slot 2 (Sec) before | Slot 3 (Sec) before | Boot status | Slot 0 (Prim) after | Slot 1 (Prim) after | Slot 2 (Sec) after | Slot 3 (Sec) after |
|---|---|---|---|---|---|---|---|---|---|
|   0 |  Empty     | Empty     | Empty     | Empty     | [0, 2]  [1, 3]  NU  FAIL | Empty     | Empty     | Empty     | Empty     |
|   1 |  A         | Empty     | Empty     | Empty     | [0, 2]  [1, 3]  NU  FAIL | A         | Empty     | Empty     | Empty     |
|   2 |  B         | Empty     | Empty     | Empty     | [0, 2]  [1, 3]  NU  FAIL | B         | Empty     | Empty     | Empty     |
|   3 |  Corrupted | Empty     | Empty     | Empty     | [0, 2]  [1, 3]  NU  FAIL | Empty     | Empty     | Empty     | Empty     |
|   4 |  Empty     | C         | Empty     | Empty     | [0, 2]  [1, 3]  NU  FAIL | Empty     | C         | Empty     | Empty     |
|   5 |  A         | C         | Empty     | Empty     | [0, 2]  [1, 3]  NU  BOOT | A         | C         | Empty     | Empty     |
|   6 |  B         | C         | Empty     | Empty     | [0, 2]  [1, 3]  NU  BOOT | B         | C         | Empty     | Empty     |
|   7 |  Corrupted | C         | Empty     | Empty     | [0, 2]  [1, 3]  NU  FAIL | Empty     | C         | Empty     | Empty     |
|   8 |  Empty     | D         | Empty     | Empty     | [0, 2]  [1, 3]  NU  FAIL | Empty     | D         | Empty     | Empty     |
|   9 |  A         | D         | Empty     | Empty     | [0, 2]  [1, 3]  NU  BOOT | A         | D         | Empty     | Empty     |
|  10 |  B         | D         | Empty     | Empty     | [0, 2]  [1, 3]  NU  BOOT | B         | D         | Empty     | Empty     |
|  11 |  Corrupted | D         | Empty     | Empty     | [0, 2]  [1, 3]  NU  FAIL | Empty     | D         | Empty     | Empty     |
|  12 |  Empty     | Corrupted | Empty     | Empty     | [0, 2]  [1, 3]  NU  FAIL | Empty     | Empty     | Empty     | Empty     |
|  13 |  A         | Corrupted | Empty     | Empty     | [0, 2]  [1, 3]  NU  FAIL | A         | Empty     | Empty     | Empty     |
|  14 |  B         | Corrupted | Empty     | Empty     | [0, 2]  [1, 3]  NU  FAIL | B         | Empty     | Empty     | Empty     |
|  15 |  Corrupted | Corrupted | Empty     | Empty     | [0, 2]  [1, 3]  NU  FAIL | Empty     | Empty     | Empty     | Empty     |
|  16 |  Empty     | Empty     | A         | Empty     | [0, 2]  [1, 3]  UP  FAIL | A         | Empty     | Empty     | Empty     |
|  17 |  A         | Empty     | A         | Empty     | [0, 2]  [1, 3]  NU  FAIL | A         | Empty     | Empty     | Empty     |
|  18 |  B         | Empty     | A         | Empty     | [0, 2]  [1, 3]  NU  FAIL | B         | Empty     | Empty     | Empty     |
|  19 |  Corrupted | Empty     | A         | Empty     | [0, 2]  [1, 3]  UP  FAIL | A         | Empty     | Empty     | Empty     |
|  20 |  Empty     | C         | A         | Empty     | [0, 2]  [1, 3]  UP  BOOT | A         | C         | Empty     | Empty     |
|  21 |  A         | C         | A         | Empty     | [0, 2]  [1, 3]  NU  BOOT | A         | C         | Empty     | Empty     |
|  22 |  B         | C         | A         | Empty     | [0, 2]  [1, 3]  NU  BOOT | B         | C         | Empty     | Empty     |
|  23 |  Corrupted | C         | A         | Empty     | [0, 2]  [1, 3]  UP  BOOT | A         | C         | Empty     | Empty     |
|  24 |  Empty     | D         | A         | Empty     | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
|  25 |  A         | D         | A         | Empty     | [0, 2]  [1, 3]  NU  BOOT | A         | D         | Empty     | Empty     |
|  26 |  B         | D         | A         | Empty     | [0, 2]  [1, 3]  NU  BOOT | B         | D         | Empty     | Empty     |
|  27 |  Corrupted | D         | A         | Empty     | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
|  28 |  Empty     | Corrupted | A         | Empty     | [0, 2]  [1, 3]  UP  FAIL | A         | Empty     | Empty     | Empty     |
|  29 |  A         | Corrupted | A         | Empty     | [0, 2]  [1, 3]  NU  FAIL | A         | Empty     | Empty     | Empty     |
|  30 |  B         | Corrupted | A         | Empty     | [0, 2]  [1, 3]  NU  FAIL | B         | Empty     | Empty     | Empty     |
|  31 |  Corrupted | Corrupted | A         | Empty     | [0, 2]  [1, 3]  UP  FAIL | A         | Empty     | Empty     | Empty     |
|  32 |  Empty     | Empty     | B         | Empty     | [0, 2]  [1, 3]  UP  FAIL | B         | Empty     | Empty     | Empty     |
|  33 |  A         | Empty     | B         | Empty     | [0, 2]  [1, 3]  UP  FAIL | B         | Empty     | Empty     | Empty     |
|  34 |  B         | Empty     | B         | Empty     | [0, 2]  [1, 3]  NU  FAIL | B         | Empty     | Empty     | Empty     |
|  35 |  Corrupted | Empty     | B         | Empty     | [0, 2]  [1, 3]  UP  FAIL | B         | Empty     | Empty     | Empty     |
|  36 |  Empty     | C         | B         | Empty     | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
|  37 |  A         | C         | B         | Empty     | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
|  38 |  B         | C         | B         | Empty     | [0, 2]  [1, 3]  NU  BOOT | B         | C         | Empty     | Empty     |
|  39 |  Corrupted | C         | B         | Empty     | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
|  40 |  Empty     | D         | B         | Empty     | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
|  41 |  A         | D         | B         | Empty     | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
|  42 |  B         | D         | B         | Empty     | [0, 2]  [1, 3]  NU  BOOT | B         | D         | Empty     | Empty     |
|  43 |  Corrupted | D         | B         | Empty     | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
|  44 |  Empty     | Corrupted | B         | Empty     | [0, 2]  [1, 3]  UP  FAIL | B         | Empty     | Empty     | Empty     |
|  45 |  A         | Corrupted | B         | Empty     | [0, 2]  [1, 3]  UP  FAIL | B         | Empty     | Empty     | Empty     |
|  46 |  B         | Corrupted | B         | Empty     | [0, 2]  [1, 3]  NU  FAIL | B         | Empty     | Empty     | Empty     |
|  47 |  Corrupted | Corrupted | B         | Empty     | [0, 2]  [1, 3]  UP  FAIL | B         | Empty     | Empty     | Empty     |
|  48 |  Empty     | Empty     | Corrupted | Empty     | [0, 2]  [1, 3]  NU  FAIL | Empty     | Empty     | Empty     | Empty     |
|  49 |  A         | Empty     | Corrupted | Empty     | [0, 2]  [1, 3]  NU  FAIL | A         | Empty     | Empty     | Empty     |
|  50 |  B         | Empty     | Corrupted | Empty     | [0, 2]  [1, 3]  NU  FAIL | B         | Empty     | Empty     | Empty     |
|  51 |  Corrupted | Empty     | Corrupted | Empty     | [0, 2]  [1, 3]  NU  FAIL | Empty     | Empty     | Empty     | Empty     |
|  52 |  Empty     | C         | Corrupted | Empty     | [0, 2]  [1, 3]  NU  FAIL | Empty     | C         | Empty     | Empty     |
|  53 |  A         | C         | Corrupted | Empty     | [0, 2]  [1, 3]  NU  BOOT | A         | C         | Empty     | Empty     |
|  54 |  B         | C         | Corrupted | Empty     | [0, 2]  [1, 3]  NU  BOOT | B         | C         | Empty     | Empty     |
|  55 |  Corrupted | C         | Corrupted | Empty     | [0, 2]  [1, 3]  NU  FAIL | Empty     | C         | Empty     | Empty     |
|  56 |  Empty     | D         | Corrupted | Empty     | [0, 2]  [1, 3]  NU  FAIL | Empty     | D         | Empty     | Empty     |
|  57 |  A         | D         | Corrupted | Empty     | [0, 2]  [1, 3]  NU  BOOT | A         | D         | Empty     | Empty     |
|  58 |  B         | D         | Corrupted | Empty     | [0, 2]  [1, 3]  NU  BOOT | B         | D         | Empty     | Empty     |
|  59 |  Corrupted | D         | Corrupted | Empty     | [0, 2]  [1, 3]  NU  FAIL | Empty     | D         | Empty     | Empty     |
|  60 |  Empty     | Corrupted | Corrupted | Empty     | [0, 2]  [1, 3]  NU  FAIL | Empty     | Empty     | Empty     | Empty     |
|  61 |  A         | Corrupted | Corrupted | Empty     | [0, 2]  [1, 3]  NU  FAIL | A         | Empty     | Empty     | Empty     |
|  62 |  B         | Corrupted | Corrupted | Empty     | [0, 2]  [1, 3]  NU  FAIL | B         | Empty     | Empty     | Empty     |
|  63 |  Corrupted | Corrupted | Corrupted | Empty     | [0, 2]  [1, 3]  NU  FAIL | Empty     | Empty     | Empty     | Empty     |
|  64 |  Empty     | Empty     | Empty     | C         | [0, 2]  [1, 3]  UP  FAIL | Empty     | C         | Empty     | Empty     |
|  65 |  A         | Empty     | Empty     | C         | [0, 2]  [1, 3]  UP  BOOT | A         | C         | Empty     | Empty     |
|  66 |  B         | Empty     | Empty     | C         | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
|  67 |  Corrupted | Empty     | Empty     | C         | [0, 2]  [1, 3]  UP  FAIL | Empty     | C         | Empty     | Empty     |
|  68 |  Empty     | C         | Empty     | C         | [0, 2]  [1, 3]  NU  FAIL | Empty     | C         | Empty     | Empty     |
|  69 |  A         | C         | Empty     | C         | [0, 2]  [1, 3]  NU  BOOT | A         | C         | Empty     | Empty     |
|  70 |  B         | C         | Empty     | C         | [0, 2]  [1, 3]  NU  BOOT | B         | C         | Empty     | Empty     |
|  71 |  Corrupted | C         | Empty     | C         | [0, 2]  [1, 3]  NU  FAIL | Empty     | C         | Empty     | Empty     |
|  72 |  Empty     | D         | Empty     | C         | [0, 2]  [1, 3]  NU  FAIL | Empty     | D         | Empty     | Empty     |
|  73 |  A         | D         | Empty     | C         | [0, 2]  [1, 3]  NU  BOOT | A         | D         | Empty     | Empty     |
|  74 |  B         | D         | Empty     | C         | [0, 2]  [1, 3]  NU  BOOT | B         | D         | Empty     | Empty     |
|  75 |  Corrupted | D         | Empty     | C         | [0, 2]  [1, 3]  NU  FAIL | Empty     | D         | Empty     | Empty     |
|  76 |  Empty     | Corrupted | Empty     | C         | [0, 2]  [1, 3]  UP  FAIL | Empty     | C         | Empty     | Empty     |
|  77 |  A         | Corrupted | Empty     | C         | [0, 2]  [1, 3]  UP  BOOT | A         | C         | Empty     | Empty     |
|  78 |  B         | Corrupted | Empty     | C         | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
|  79 |  Corrupted | Corrupted | Empty     | C         | [0, 2]  [1, 3]  UP  FAIL | Empty     | C         | Empty     | Empty     |
|  80 |  Empty     | Empty     | A         | C         | [0, 2]  [1, 3]  UP  BOOT | A         | C         | Empty     | Empty     |
|  81 |  A         | Empty     | A         | C         | [0, 2]  [1, 3]  UP  BOOT | A         | C         | Empty     | Empty     |
|  82 |  B         | Empty     | A         | C         | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
|  83 |  Corrupted | Empty     | A         | C         | [0, 2]  [1, 3]  UP  BOOT | A         | C         | Empty     | Empty     |
|  84 |  Empty     | C         | A         | C         | [0, 2]  [1, 3]  UP  BOOT | A         | C         | Empty     | Empty     |
|  85 |  A         | C         | A         | C         | [0, 2]  [1, 3]  NU  BOOT | A         | C         | Empty     | Empty     |
|  86 |  B         | C         | A         | C         | [0, 2]  [1, 3]  NU  BOOT | B         | C         | Empty     | Empty     |
|  87 |  Corrupted | C         | A         | C         | [0, 2]  [1, 3]  UP  BOOT | A         | C         | Empty     | Empty     |
|  88 |  Empty     | D         | A         | C         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
|  89 |  A         | D         | A         | C         | [0, 2]  [1, 3]  NU  BOOT | A         | D         | Empty     | Empty     |
|  90 |  B         | D         | A         | C         | [0, 2]  [1, 3]  NU  BOOT | B         | D         | Empty     | Empty     |
|  91 |  Corrupted | D         | A         | C         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
|  92 |  Empty     | Corrupted | A         | C         | [0, 2]  [1, 3]  UP  BOOT | A         | C         | Empty     | Empty     |
|  93 |  A         | Corrupted | A         | C         | [0, 2]  [1, 3]  UP  BOOT | A         | C         | Empty     | Empty     |
|  94 |  B         | Corrupted | A         | C         | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
|  95 |  Corrupted | Corrupted | A         | C         | [0, 2]  [1, 3]  UP  BOOT | A         | C         | Empty     | Empty     |
|  96 |  Empty     | Empty     | B         | C         | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
|  97 |  A         | Empty     | B         | C         | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
|  98 |  B         | Empty     | B         | C         | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
|  99 |  Corrupted | Empty     | B         | C         | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
| 100 |  Empty     | C         | B         | C         | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
| 101 |  A         | C         | B         | C         | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
| 102 |  B         | C         | B         | C         | [0, 2]  [1, 3]  NU  BOOT | B         | C         | Empty     | Empty     |
| 103 |  Corrupted | C         | B         | C         | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
| 104 |  Empty     | D         | B         | C         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 105 |  A         | D         | B         | C         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 106 |  B         | D         | B         | C         | [0, 2]  [1, 3]  NU  BOOT | B         | D         | Empty     | Empty     |
| 107 |  Corrupted | D         | B         | C         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 108 |  Empty     | Corrupted | B         | C         | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
| 109 |  A         | Corrupted | B         | C         | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
| 110 |  B         | Corrupted | B         | C         | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
| 111 |  Corrupted | Corrupted | B         | C         | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
| 112 |  Empty     | Empty     | Corrupted | C         | [0, 2]  [1, 3]  UP  FAIL | Empty     | C         | Empty     | Empty     |
| 113 |  A         | Empty     | Corrupted | C         | [0, 2]  [1, 3]  UP  BOOT | A         | C         | Empty     | Empty     |
| 114 |  B         | Empty     | Corrupted | C         | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
| 115 |  Corrupted | Empty     | Corrupted | C         | [0, 2]  [1, 3]  UP  FAIL | Empty     | C         | Empty     | Empty     |
| 116 |  Empty     | C         | Corrupted | C         | [0, 2]  [1, 3]  NU  FAIL | Empty     | C         | Empty     | Empty     |
| 117 |  A         | C         | Corrupted | C         | [0, 2]  [1, 3]  NU  BOOT | A         | C         | Empty     | Empty     |
| 118 |  B         | C         | Corrupted | C         | [0, 2]  [1, 3]  NU  BOOT | B         | C         | Empty     | Empty     |
| 119 |  Corrupted | C         | Corrupted | C         | [0, 2]  [1, 3]  NU  FAIL | Empty     | C         | Empty     | Empty     |
| 120 |  Empty     | D         | Corrupted | C         | [0, 2]  [1, 3]  NU  FAIL | Empty     | D         | Empty     | Empty     |
| 121 |  A         | D         | Corrupted | C         | [0, 2]  [1, 3]  NU  BOOT | A         | D         | Empty     | Empty     |
| 122 |  B         | D         | Corrupted | C         | [0, 2]  [1, 3]  NU  BOOT | B         | D         | Empty     | Empty     |
| 123 |  Corrupted | D         | Corrupted | C         | [0, 2]  [1, 3]  NU  FAIL | Empty     | D         | Empty     | Empty     |
| 124 |  Empty     | Corrupted | Corrupted | C         | [0, 2]  [1, 3]  UP  FAIL | Empty     | C         | Empty     | Empty     |
| 125 |  A         | Corrupted | Corrupted | C         | [0, 2]  [1, 3]  UP  BOOT | A         | C         | Empty     | Empty     |
| 126 |  B         | Corrupted | Corrupted | C         | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
| 127 |  Corrupted | Corrupted | Corrupted | C         | [0, 2]  [1, 3]  UP  FAIL | Empty     | C         | Empty     | Empty     |
| 128 |  Empty     | Empty     | Empty     | D         | [0, 2]  [1, 3]  UP  FAIL | Empty     | D         | Empty     | Empty     |
| 129 |  A         | Empty     | Empty     | D         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 130 |  B         | Empty     | Empty     | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 131 |  Corrupted | Empty     | Empty     | D         | [0, 2]  [1, 3]  UP  FAIL | Empty     | D         | Empty     | Empty     |
| 132 |  Empty     | C         | Empty     | D         | [0, 2]  [1, 3]  UP  FAIL | Empty     | D         | Empty     | Empty     |
| 133 |  A         | C         | Empty     | D         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 134 |  B         | C         | Empty     | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 135 |  Corrupted | C         | Empty     | D         | [0, 2]  [1, 3]  UP  FAIL | Empty     | D         | Empty     | Empty     |
| 136 |  Empty     | D         | Empty     | D         | [0, 2]  [1, 3]  NU  FAIL | Empty     | D         | Empty     | Empty     |
| 137 |  A         | D         | Empty     | D         | [0, 2]  [1, 3]  NU  BOOT | A         | D         | Empty     | Empty     |
| 138 |  B         | D         | Empty     | D         | [0, 2]  [1, 3]  NU  BOOT | B         | D         | Empty     | Empty     |
| 139 |  Corrupted | D         | Empty     | D         | [0, 2]  [1, 3]  NU  FAIL | Empty     | D         | Empty     | Empty     |
| 140 |  Empty     | Corrupted | Empty     | D         | [0, 2]  [1, 3]  UP  FAIL | Empty     | D         | Empty     | Empty     |
| 141 |  A         | Corrupted | Empty     | D         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 142 |  B         | Corrupted | Empty     | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 143 |  Corrupted | Corrupted | Empty     | D         | [0, 2]  [1, 3]  UP  FAIL | Empty     | D         | Empty     | Empty     |
| 144 |  Empty     | Empty     | A         | D         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 145 |  A         | Empty     | A         | D         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 146 |  B         | Empty     | A         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 147 |  Corrupted | Empty     | A         | D         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 148 |  Empty     | C         | A         | D         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 149 |  A         | C         | A         | D         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 150 |  B         | C         | A         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 151 |  Corrupted | C         | A         | D         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 152 |  Empty     | D         | A         | D         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 153 |  A         | D         | A         | D         | [0, 2]  [1, 3]  NU  BOOT | A         | D         | Empty     | Empty     |
| 154 |  B         | D         | A         | D         | [0, 2]  [1, 3]  NU  BOOT | B         | D         | Empty     | Empty     |
| 155 |  Corrupted | D         | A         | D         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 156 |  Empty     | Corrupted | A         | D         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 157 |  A         | Corrupted | A         | D         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 158 |  B         | Corrupted | A         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 159 |  Corrupted | Corrupted | A         | D         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 160 |  Empty     | Empty     | B         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 161 |  A         | Empty     | B         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 162 |  B         | Empty     | B         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 163 |  Corrupted | Empty     | B         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 164 |  Empty     | C         | B         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 165 |  A         | C         | B         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 166 |  B         | C         | B         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 167 |  Corrupted | C         | B         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 168 |  Empty     | D         | B         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 169 |  A         | D         | B         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 170 |  B         | D         | B         | D         | [0, 2]  [1, 3]  NU  BOOT | B         | D         | Empty     | Empty     |
| 171 |  Corrupted | D         | B         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 172 |  Empty     | Corrupted | B         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 173 |  A         | Corrupted | B         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 174 |  B         | Corrupted | B         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 175 |  Corrupted | Corrupted | B         | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 176 |  Empty     | Empty     | Corrupted | D         | [0, 2]  [1, 3]  UP  FAIL | Empty     | D         | Empty     | Empty     |
| 177 |  A         | Empty     | Corrupted | D         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 178 |  B         | Empty     | Corrupted | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 179 |  Corrupted | Empty     | Corrupted | D         | [0, 2]  [1, 3]  UP  FAIL | Empty     | D         | Empty     | Empty     |
| 180 |  Empty     | C         | Corrupted | D         | [0, 2]  [1, 3]  UP  FAIL | Empty     | D         | Empty     | Empty     |
| 181 |  A         | C         | Corrupted | D         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 182 |  B         | C         | Corrupted | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 183 |  Corrupted | C         | Corrupted | D         | [0, 2]  [1, 3]  UP  FAIL | Empty     | D         | Empty     | Empty     |
| 184 |  Empty     | D         | Corrupted | D         | [0, 2]  [1, 3]  NU  FAIL | Empty     | D         | Empty     | Empty     |
| 185 |  A         | D         | Corrupted | D         | [0, 2]  [1, 3]  NU  BOOT | A         | D         | Empty     | Empty     |
| 186 |  B         | D         | Corrupted | D         | [0, 2]  [1, 3]  NU  BOOT | B         | D         | Empty     | Empty     |
| 187 |  Corrupted | D         | Corrupted | D         | [0, 2]  [1, 3]  NU  FAIL | Empty     | D         | Empty     | Empty     |
| 188 |  Empty     | Corrupted | Corrupted | D         | [0, 2]  [1, 3]  UP  FAIL | Empty     | D         | Empty     | Empty     |
| 189 |  A         | Corrupted | Corrupted | D         | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 190 |  B         | Corrupted | Corrupted | D         | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 191 |  Corrupted | Corrupted | Corrupted | D         | [0, 2]  [1, 3]  UP  FAIL | Empty     | D         | Empty     | Empty     |
| 192 |  Empty     | Empty     | Empty     | Corrupted | [0, 2]  [1, 3]  NU  FAIL | Empty     | Empty     | Empty     | Empty     |
| 193 |  A         | Empty     | Empty     | Corrupted | [0, 2]  [1, 3]  NU  FAIL | A         | Empty     | Empty     | Empty     |
| 194 |  B         | Empty     | Empty     | Corrupted | [0, 2]  [1, 3]  NU  FAIL | B         | Empty     | Empty     | Empty     |
| 195 |  Corrupted | Empty     | Empty     | Corrupted | [0, 2]  [1, 3]  NU  FAIL | Empty     | Empty     | Empty     | Empty     |
| 196 |  Empty     | C         | Empty     | Corrupted | [0, 2]  [1, 3]  NU  FAIL | Empty     | C         | Empty     | Empty     |
| 197 |  A         | C         | Empty     | Corrupted | [0, 2]  [1, 3]  NU  BOOT | A         | C         | Empty     | Empty     |
| 198 |  B         | C         | Empty     | Corrupted | [0, 2]  [1, 3]  NU  BOOT | B         | C         | Empty     | Empty     |
| 199 |  Corrupted | C         | Empty     | Corrupted | [0, 2]  [1, 3]  NU  FAIL | Empty     | C         | Empty     | Empty     |
| 200 |  Empty     | D         | Empty     | Corrupted | [0, 2]  [1, 3]  NU  FAIL | Empty     | D         | Empty     | Empty     |
| 201 |  A         | D         | Empty     | Corrupted | [0, 2]  [1, 3]  NU  BOOT | A         | D         | Empty     | Empty     |
| 202 |  B         | D         | Empty     | Corrupted | [0, 2]  [1, 3]  NU  BOOT | B         | D         | Empty     | Empty     |
| 203 |  Corrupted | D         | Empty     | Corrupted | [0, 2]  [1, 3]  NU  FAIL | Empty     | D         | Empty     | Empty     |
| 204 |  Empty     | Corrupted | Empty     | Corrupted | [0, 2]  [1, 3]  NU  FAIL | Empty     | Empty     | Empty     | Empty     |
| 205 |  A         | Corrupted | Empty     | Corrupted | [0, 2]  [1, 3]  NU  FAIL | A         | Empty     | Empty     | Empty     |
| 206 |  B         | Corrupted | Empty     | Corrupted | [0, 2]  [1, 3]  NU  FAIL | B         | Empty     | Empty     | Empty     |
| 207 |  Corrupted | Corrupted | Empty     | Corrupted | [0, 2]  [1, 3]  NU  FAIL | Empty     | Empty     | Empty     | Empty     |
| 208 |  Empty     | Empty     | A         | Corrupted | [0, 2]  [1, 3]  UP  FAIL | A         | Empty     | Empty     | Empty     |
| 209 |  A         | Empty     | A         | Corrupted | [0, 2]  [1, 3]  NU  FAIL | A         | Empty     | Empty     | Empty     |
| 210 |  B         | Empty     | A         | Corrupted | [0, 2]  [1, 3]  NU  FAIL | B         | Empty     | Empty     | Empty     |
| 211 |  Corrupted | Empty     | A         | Corrupted | [0, 2]  [1, 3]  UP  FAIL | A         | Empty     | Empty     | Empty     |
| 212 |  Empty     | C         | A         | Corrupted | [0, 2]  [1, 3]  UP  BOOT | A         | C         | Empty     | Empty     |
| 213 |  A         | C         | A         | Corrupted | [0, 2]  [1, 3]  NU  BOOT | A         | C         | Empty     | Empty     |
| 214 |  B         | C         | A         | Corrupted | [0, 2]  [1, 3]  NU  BOOT | B         | C         | Empty     | Empty     |
| 215 |  Corrupted | C         | A         | Corrupted | [0, 2]  [1, 3]  UP  BOOT | A         | C         | Empty     | Empty     |
| 216 |  Empty     | D         | A         | Corrupted | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 217 |  A         | D         | A         | Corrupted | [0, 2]  [1, 3]  NU  BOOT | A         | D         | Empty     | Empty     |
| 218 |  B         | D         | A         | Corrupted | [0, 2]  [1, 3]  NU  BOOT | B         | D         | Empty     | Empty     |
| 219 |  Corrupted | D         | A         | Corrupted | [0, 2]  [1, 3]  UP  BOOT | A         | D         | Empty     | Empty     |
| 220 |  Empty     | Corrupted | A         | Corrupted | [0, 2]  [1, 3]  UP  FAIL | A         | Empty     | Empty     | Empty     |
| 221 |  A         | Corrupted | A         | Corrupted | [0, 2]  [1, 3]  NU  FAIL | A         | Empty     | Empty     | Empty     |
| 222 |  B         | Corrupted | A         | Corrupted | [0, 2]  [1, 3]  NU  FAIL | B         | Empty     | Empty     | Empty     |
| 223 |  Corrupted | Corrupted | A         | Corrupted | [0, 2]  [1, 3]  UP  FAIL | A         | Empty     | Empty     | Empty     |
| 224 |  Empty     | Empty     | B         | Corrupted | [0, 2]  [1, 3]  UP  FAIL | B         | Empty     | Empty     | Empty     |
| 225 |  A         | Empty     | B         | Corrupted | [0, 2]  [1, 3]  UP  FAIL | B         | Empty     | Empty     | Empty     |
| 226 |  B         | Empty     | B         | Corrupted | [0, 2]  [1, 3]  NU  FAIL | B         | Empty     | Empty     | Empty     |
| 227 |  Corrupted | Empty     | B         | Corrupted | [0, 2]  [1, 3]  UP  FAIL | B         | Empty     | Empty     | Empty     |
| 228 |  Empty     | C         | B         | Corrupted | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
| 229 |  A         | C         | B         | Corrupted | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
| 230 |  B         | C         | B         | Corrupted | [0, 2]  [1, 3]  NU  BOOT | B         | C         | Empty     | Empty     |
| 231 |  Corrupted | C         | B         | Corrupted | [0, 2]  [1, 3]  UP  BOOT | B         | C         | Empty     | Empty     |
| 232 |  Empty     | D         | B         | Corrupted | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 233 |  A         | D         | B         | Corrupted | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 234 |  B         | D         | B         | Corrupted | [0, 2]  [1, 3]  NU  BOOT | B         | D         | Empty     | Empty     |
| 235 |  Corrupted | D         | B         | Corrupted | [0, 2]  [1, 3]  UP  BOOT | B         | D         | Empty     | Empty     |
| 236 |  Empty     | Corrupted | B         | Corrupted | [0, 2]  [1, 3]  UP  FAIL | B         | Empty     | Empty     | Empty     |
| 237 |  A         | Corrupted | B         | Corrupted | [0, 2]  [1, 3]  UP  FAIL | B         | Empty     | Empty     | Empty     |
| 238 |  B         | Corrupted | B         | Corrupted | [0, 2]  [1, 3]  NU  FAIL | B         | Empty     | Empty     | Empty     |
| 239 |  Corrupted | Corrupted | B         | Corrupted | [0, 2]  [1, 3]  UP  FAIL | B         | Empty     | Empty     | Empty     |
| 240 |  Empty     | Empty     | Corrupted | Corrupted | [0, 2]  [1, 3]  NU  FAIL | Empty     | Empty     | Empty     | Empty     |
| 241 |  A         | Empty     | Corrupted | Corrupted | [0, 2]  [1, 3]  NU  FAIL | A         | Empty     | Empty     | Empty     |
| 242 |  B         | Empty     | Corrupted | Corrupted | [0, 2]  [1, 3]  NU  FAIL | B         | Empty     | Empty     | Empty     |
| 243 |  Corrupted | Empty     | Corrupted | Corrupted | [0, 2]  [1, 3]  NU  FAIL | Empty     | Empty     | Empty     | Empty     |
| 244 |  Empty     | C         | Corrupted | Corrupted | [0, 2]  [1, 3]  NU  FAIL | Empty     | C         | Empty     | Empty     |
| 245 |  A         | C         | Corrupted | Corrupted | [0, 2]  [1, 3]  NU  BOOT | A         | C         | Empty     | Empty     |
| 246 |  B         | C         | Corrupted | Corrupted | [0, 2]  [1, 3]  NU  BOOT | B         | C         | Empty     | Empty     |
| 247 |  Corrupted | C         | Corrupted | Corrupted | [0, 2]  [1, 3]  NU  FAIL | Empty     | C         | Empty     | Empty     |
| 248 |  Empty     | D         | Corrupted | Corrupted | [0, 2]  [1, 3]  NU  FAIL | Empty     | D         | Empty     | Empty     |
| 249 |  A         | D         | Corrupted | Corrupted | [0, 2]  [1, 3]  NU  BOOT | A         | D         | Empty     | Empty     |
| 250 |  B         | D         | Corrupted | Corrupted | [0, 2]  [1, 3]  NU  BOOT | B         | D         | Empty     | Empty     |
| 251 |  Corrupted | D         | Corrupted | Corrupted | [0, 2]  [1, 3]  NU  FAIL | Empty     | D         | Empty     | Empty     |
| 252 |  Empty     | Corrupted | Corrupted | Corrupted | [0, 2]  [1, 3]  NU  FAIL | Empty     | Empty     | Empty     | Empty     |
| 253 |  A         | Corrupted | Corrupted | Corrupted | [0, 2]  [1, 3]  NU  FAIL | A         | Empty     | Empty     | Empty     |
| 254 |  B         | Corrupted | Corrupted | Corrupted | [0, 2]  [1, 3]  NU  FAIL | B         | Empty     | Empty     | Empty     |
| 255 |  Corrupted | Corrupted | Corrupted | Corrupted | [0, 2]  [1, 3]  NU  FAIL | Empty     | Empty     | Empty     | Empty     |
