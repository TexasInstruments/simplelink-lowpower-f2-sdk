# Split Image Tool

## Table of Contents

* [Usage Note](#Usage Note)
* [Introduction](#Introduction)
* [Software Prerequisites](#Software Prerequisites)
* [Usage](#Usage)

## <a name="Usage Note"></a>Usage Note

Note that this tool is not meant to be used with the CC13X2/CC26X2 devices. The
rest of this document uses the CC13X4/CC26X4 devices as an example of correct
usage. But the usage of this tool is intended for any device which has the CCFG
outside of the main flash region.

## <a name="Introduction"></a>Introduction

Due to the memory mapping on the CC13X4/CC26X4 and beyond devices, the CCFG is
no longer in the main flash region. This causes a change in functionality as
compared to the previous CC13X2/CC26X2 device families. As a result, a
CC13X4/CC26X4 binary image would span an obnoxious range and end in failure. To
fix this we have made a convenience tool to split the code from the CCFG.

The split_image_tool will split an application into two appropriately sized
images. One for the application in the main flash region and one for the CCFG in
the non main flash region.

For most use cases this tool will not be necessary as the .out/.hex file format
can be understood by TI tools for programming the devices. However the ROM
Bootloader cannot understand these formats. The ROM Bootloader only understands
a simple binary format which will require the usage of this tool.


## <a name="Software Prerequisites"></a>Software Prerequisites

The split_image_tool is shipped in both source and binary form. It is recommended
to run the tool in binary form unless source changes are absolutely required.


The following is required to run the binary image:

**Linux:**
 - The binary is 64-bit, will run without modification on 64-bit systems
 - 32-bit distributions are not supported at this time

**Windows**
 - The binary is 32-bit, but is supported on 32-bit or 64-bit versions of
   Windows.

**MacOS**
 - WARNING: MacOS support is experimental.
 - The binary is 64-bit, as of MacOS Sierra this is the only supported ArchType.

In order to run the tool from source, the following dependencies are needed:

 - Python 3.6
 - The Python packages listed in the requirements.txt in this folder. These can
   be installed via pip.

## <a name="Usage"></a>Usage

Invoke the tool with the `-h` option to display the help menu and to
see required and optional arguments.

Example:
```
 $ python split_image_tool.py \
 --tc /c/ti/ccs1100/ccs/tools/compiler/gcc-arm-none-eabi-9-2019-q4-major \
 --input /c/ti/workspace_thor_fcs/mcuboot_app/Debug/mcuboot_app.out
```

It could be helpful to add this to your post build steps in CCS as well.
This can be done by right clicking your project (properties->Build->Steps) and
then adding a call to the split image tool.

Example:
```
$ split_image_tool.py \
--tc ${CG_TOOL_ROOT} \
--input ${PROJECT_LOC}/Debug/mcuboot_app_onchip_CC1354R10_LAUNCHXL_nortos_gcc.out
```