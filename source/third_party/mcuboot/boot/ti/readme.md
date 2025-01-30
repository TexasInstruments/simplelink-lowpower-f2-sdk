# Required
1. git
2. [UniFlash](https://www.ti.com/tool/UNIFLASH), If using UniFlash to download images
3. The [Arm GCC Compiler](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads), if using makefiles to build images
4. [CCS](https://software-dl.ti.com/ccs/esd/documents/ccs_downloads.html) if using CCS to build images
5. The [CC1352 SimpleLink SDK](https://www.ti.com/tool/SIMPLELINK-CC13X2-26X2-SDK)
6. Python 3

Clone the repo and initialize the submodules:
```
git clone https://github.com/mcu-tools/mcuboot.git
git submodule update —-init —-recursive
```

# CC1352 Port
The CC1353 port of mcu boot consists of:

* The mcuboot application
* The cc1552 porting files
* A Blinky application that can be booted
* A submodule containing a [Python based boot loader](https://github.com/JelmerT/cc2538-bsl)

## Flash Layout and Configuration

The example provided supports XIP mode of mcuboot with 1 primary and secondary slot. The flash layout in the example is:

|     Slot     |     Address      |
|:----------------:|:------------------ |
|   mcuboot slot   | <ul><li>0x00000000</li><li>    </li></ul><li>0x00015FFF</li> |
|   primary slot     | <ul><li>0x00016000</li><li>    </li></ul><li>0x00035FFF</li> |
|  secondary slot  | <ul><li>0x00036000</li><li>    </li></ul><li>0x00055FFF</li> |
|  ......  | <ul><li>0x00056000</li><li>    </li></ul><li>0x00057fa7</li> |
|  CCFG | <ul><li>0x00057fa8</li><li>    </li></ul><li>0x00058000</li> |

The ccfg is used to configure the cc1352, it is contained in the mcuboot_app image and uses default values accept for enabling the serial bootloader.

# McuBoot Application

## Building the McuBoot Application with the makefile

* Set up environmental variables used in the makefile:
```
$ export SIMPLELINK_CC13X2_26X2_SDK_INSTALL_DIR=/Applications/ti/simplelink_cc13x2_26x2_sdk_4_40_00_44
export GCC_ARMCOMPILER=/Applications/ti/ccs1020/ccs/tools/compiler/gcc-arm-none-eabi-7-2017-q4-major/
```
* Build the mcuboot-app
```
$ cd boot/ti/projects/CC1352R1_LAUNCHXL/mcuboot_app/gcc/
$ make
Building build/obj/mcuboot_app.obj
Building build/obj/main_nortos.obj
Building build/obj/trace.obj
Building build/obj/flash_map_backend.obj
Building build/obj/ti_drivers_config.obj
Building build/obj/ti_devices_config.obj
Building build/obj/boot_record.obj
Building build/obj/image_ed25519.obj
Building build/obj/bootutil_misc.obj
Building build/obj/image_rsa.obj
Building build/obj/image_validate.obj
Building build/obj/bootutil_public.obj
Building build/obj/loader.obj
Building build/obj/caps.obj
Building build/obj/swap_misc.obj
Building build/obj/encrypted.obj
Building build/obj/swap_move.obj
Building build/obj/fault_injection_hardening.obj
Building build/obj/fault_injection_hardening_delay_rng_mbedtls.obj
Building build/obj/swap_scratch.obj
Building build/obj/image_ec.obj
Building build/obj/tlv.obj
Building build/obj/image_ec256.obj
Building build/obj/aes_decrypt.obj
Building build/obj/ccm_mode.obj
Building build/obj/ctr_prng.obj
Building build/obj/ecc_dsa.obj
Building build/obj/hmac_prng.obj
Building build/obj/aes_encrypt.obj
Building build/obj/cmac_mode.obj
Building build/obj/ecc.obj
Building build/obj/ecc_platform_specific.obj
Building build/obj/sha256.obj
Building build/obj/cbc_mode.obj
Building build/obj/ctr_mode.obj
Building build/obj/ecc_dh.obj
Building build/obj/hmac.obj
Building build/obj/utils.obj
linking build/mcuboot_app.out
```
## Building the McuBoot Application with CCS

* Open CCS and import the boot/ti/projects/CC1352R1_LAUNCHXL/mcuboot_app/gcc/mcuboot_app_CC1352R1_LAUNCHXL_nortos_gcc.projectspec

* Click the build icon. The built application will be located in you workspace at mcuboot_app_CC1352R1_LAUNCHXL_nortos_gcc/Debug/mcuboot_app_CC1352R1_LAUNCHXL_nortos_gcc.out

* CCS can be used to download and run mcuboot using the debug icon, alternatively you can use Uniflash as instructed below

## Running the McuBoot Application

* Download mcuboot-app.out using uniflash.

* Connect a serial terminal to the Launch pad with 3000000 board and 8N1:
```
$ picocom --imap lfcrlf -b 3000000 /dev/tty.usbmodemL41005JA1
```

* Reset the device and observe that mcuboot runs and can not find a valid image:
```
[INFO][MCB ]: mcuboot_app
[INFO][MCB ]: Primary slot: Image not found
[INFO][MCB ]: Secondary slot: Image not found
[INFO][MCB ]: bootRsp: no image
```

# Blinky Application

## Building the Blinky Application with the makefile for the Primary Slot

* Build the blinky-app:
```
$ cd ../../blinky_app/gcc
$ make
Building build/obj/blinky.obj
Building build/obj/trace.obj
Building build/obj/main_nortos.obj
Building build/obj/ti_drivers_config.obj
linking build/primary_blinky.out
linking build/secondary_blinky.out
```

⋅⋅⋅Note: As this example is for XIP both the mages have been built the primary and secondary slot.

* Convert the primary image to an mcuboot binary with V1.0.0 header:
```
$ arm-none-eabi-objcopy build/primary_blinky.out build/primary_blinky_app-no-header.bin -O binary
$ python3 ../../../../../../scripts/imgtool.py sign     --header-size 0x200     --align 4     --slot-size 0x20000     --version 1.0.0     --pad-header build/primary_blinky_app-no-header.bin     --key ../../../../../../root-ec-p256.pem     build/primary_blinky_app.bin
```
## Building the Blinky Application with CCS for the Primary Slot

* Open CCS and import the boot/ti/projects/CC1352R1_LAUNCHXL/blinky_app/gcc/blinky_CC1352R1_LAUNCHXL_nortos_gcc.projectspec

* Click the build icon. The blinky mcuboot binary will be located in you workspace at blinky_CC1352R1_LAUNCHXL_nortos_gcc/Debug/blinky_app.bin

* The blinky mcuboot binary should be downloaded with Uniflash as instructed below

## Running the Primary Slot Blinky Application

* Download the primary slot blinky application image using UniFlash. Make sure to check the binary checkbox and set the offset to 0x16000 
  * If built with the makefiles the image is `<git dir>boot/ti/projects/CC1352R1_LAUNCHXL/blinky_app/gcc/build/primary_blinky_app.bin`  
  * If built with the CCS the image is `<worksapce dir>/blinky_CC1352R1_LAUNCHXL_nortos_gcc/Debug/blinky_app.bin`  

* Reset the device and observe that mcuboot runs and finds a valid v1.0.0 image:
```
[INFO][MCB ]: mcuboot_app
[INFO][MCB ]: Primary   slot: version=1.0.0+0
[INFO][MCB ]: Secondary slot: Image not found
[INFO][MCB ]: Booting image from the primary slot
[INFO][MCB ]: bootRsp: slot = 0, offset = 8000, ver=1.0.0.0
[INFO][MCB ]: Starting Main Application
[INFO][MCB ]:   Image Start Offset: 0x8000
[INFO][MCB ]:   Vector Table Start Address: 0x8200
[INFO][APP ]: blinky_app ver=1.0.0.0
```

## Building the Blinky Application with the makefile for the Secondary Slot

* Convert the secondary image to an mcuboot binary with V2.0.0 header:
```
$ arm-none-eabi-objcopy build/secondary_blinky.out build/secondary_blinky_app-no-header.bin -O binary
$ python3 ../../../../../../scripts/imgtool.py sign     --header-size 0x200     --align 4     --slot-size 0x20000     --version 2.0.0     --pad-header build/secondary_blinky_app-no-header.bin     --key ../../../../../../root-ec-p256.pem     build/secondary_blinky_app.bin
```
## Building the Blinky Application with CCS for the Secondary Slot

* In the imported blinky project open the project options in and change:
  * The version number in the Post build step, under `Project->Properties->Build`
```
python ${GIT_ROOT}/scripts/imgtool.py sign     --header-size 0x200     --align 4     --slot-size 0x20000     --version 2.0.0     --key ../../../mcuboot/root-ec-p256.pem     --pad-header blinky_no-header.bin blinky_app.bin
```
  * Remove the `x` from the secondary image linker define under `Project->Properties->Build->GNU->Linker->Symbols`
```
SEC_IMG=1
```

* Click the build icon. The blinky mcuboot binary will be located in the workspace at blinky_CC1352R1_LAUNCHXL_nortos_gcc/Debug/blinky_app.bin

* The blinky mcuboot binary should be downloaded with Uniflash as instructed below

## Running the Secondary Slot Blinky Application

* Download the secondary slot blinky application image using UniFlash. Make sure to check the binary checkbox and set the offset to 0x36000
  * If built with the makefiles the image is located at`<git dir>boot/ti/projects/CC1352R1_LAUNCHXL/blinky_app/gcc/build/secondary_blinky_app.bin`  
  * If built with the CCS the image is located `<worksapce dir>/blinky_CC1352R1_LAUNCHXL_nortos_gcc/Debug/blinky_app.bin`  

* Reset the device and observe that mcuboot runs and finds a valid v2.0.0 image:
```
[INFO][MCB ]: mcuboot_app
[INFO][MCB ]: Primary   slot: version=1.0.0+0
[INFO][MCB ]: Secondary slot: version=2.0.0+0
[INFO][MCB ]: Booting image from the secondary slot
[INFO][MCB ]: bootRsp: slot = 1, offset = 28000, ver=2.0.0.0
[INFO][MCB ]: Starting Main Application
[INFO][MCB ]:   Image Start Offset: 0x28000
[INFO][MCB ]:   Vector Table Start Address: 0x28200
[INFO][APP ]: blinky_app ver=2.0.0.0
```

New versions can then be loaded using the above process.

Pressing BTN-1 while the blinky_app is running will set the version number in the mcuboot header to 0 and reset the device. mcuboot_app will then run a previous version if present in the primary or secondary slot.

# Serial Bootloader

## Using the serial boot loader to program the mcuboot_app image

⋅⋅⋅NOTE: This will only work if the CCFG has been previously configured to enable the bootloader. If not then the mcuboot image must initially be loaded through JTAG using Uniflash or CCS.

* Convert the mcuboot_app.out file into a binary file:
```
arm-none-eabi-objcopy build/mcuboot_app.out build/mcuboot_app.bin -O binary
```
* Disconnect the serial terminal, hold BTN-2 (right button) and press reset to invoke the ROM serial boot loader

* Hold BTN-2 (right button) and press reset to invoke the ROM serial boot loader

* Mass erase, program and verify the mcuboot_app binary image:
```
$ python3 ../../../../tools/serial_boot/cc2538-bsl.py -e -w -v build/mcuboot_app.bin -d /dev/tty.usbmodemL41005JA1 

Opening port /dev/tty.usbmodemL41005JA1, baud 500000
Reading data from build/mcuboot_app.bin
Cannot auto-detect firmware filetype: Assuming .bin
Connecting to target...
CC1350 PG2.0 (7x7mm): 352KB Flash, 20KB SRAM, CCFG.BL_CONFIG at 0x00057FD8
Primary IEEE Address: 00:12:4B:00:14:F4:35:81
    Performing mass erase
Erasing all main bank flash sectors
    Erase done
Writing 360448 bytes starting at address 0x00000000
Write 104 bytes at 0x00057F980
    Write done                                
Verifying by comparing CRC32 calculations.
    Verified (match: 0xf111e6e4)
```
⋅⋅⋅NOTE: This will take several minutes as the image includes mcuboot at the start of flash and the CCFG at the end of Flash.

## Using the serial boot loader to program the blinky_app image

* Program and verify the mcuboot_app binary image:
```
$ python3 ../../../../tools/serial_boot/cc2538-bsl.py -a 0x8000 -w -v build/primary_blinky_app.bin -d /dev/tty.usbmodemL41005JA1 
```

⋅⋅⋅Note: This will only work if the sectors being programmed are erased. It should be possible to erase the primary slot sectors by using the -E a,0x00008000,0x00028000 option. However at the time of writing this feature in cc2538-bsl.py does not function correctly on the cc1352.

