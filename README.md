# SIMPLELINK CC13XX/CC26XX SDK
This repository contains TI's SimpleLink Low Power F2 (previously CC13XX/CC26XX) Software Development Kit (SDK).
 
## Summary
 
The SimpleLink™ Low Power F2 Software Development Kit (previously SimpleLink™ CC13xx and CC26xx SDK) provides a comprehensive software package for the development of Sub-1 GHz and 2.4 GHz applications, including support for Bluetooth® Low Energy, Mesh, Zigbee®, Matter, Thread, 802.15.4-based, proprietary, and multiprotocol solutions on the SimpleLink CC13x1, CC13x2, CC13x4, CC26x1, CC26x2, and CC26x4 wireless MCUs.
 
An overview of the full SDK is hosted on TI Resource Explorer.
 
## Background
This repository contains a subset of TI's [SimpleLink CC13XX/CC26XX SDK](https://www.ti.com/tool/SIMPLELINK-CC13XX-CC26XX-SDK).  It is provided via a Git repo to meet several use cases, including:
 
* Reduces download size
* Enables CI/CD tooling to use Git rather than installers
* Increases visibility of changes, better enabling benefit/risk assessments
* Simplifies the workflow for rebasing user changes to different releases
 
It intentionally omits documentation (the **docs/** directory) and examples (**examples/**) to reduce size.  Regarding those omissions:
 
* [Documentation is available online](https://dev.ti.com/tirex/explore/node?node=A__ABWp1w.mXU7n8HjfEIyLGQ__com.ti.SIMPLELINK_CC13XX_CC26XX_SDK__BSEc4rl__LATEST&placeholder=true).  Please match your git version to the documentation version in the TI Developer Zone.
* Examples are available in separate repositories, as noted in [Examples](#example-repos).
 
## Features
 
- Supports proprietary  Sub-1 GHz and 2.4 GHz applications using basic RX and TX examples
- TI 15.4-Stack, an IEEE 802.15.4-based star topology networking solution for the Sub-1 GHz and 2.4 GHz bands
- Certified Wi-SUN® FAN 1.0 compliant Sub-1 GHz mesh stack (learn more at [TI.com/wisun](https://www.ti.com/wisun))
- Bluetooth Low Energy (BLE) software protocol stack supporting Bluetooth 5.2 and Bluetooth mesh (learn more at [TI.com/ble](https://www.ti.com/ble))
- Matter 1.0 and Thread 1.3 networking protocol based on open-source Matter project and OpenThread (learn more at [TI.com/matter](https://www.ti.com/matter) or in the [TI Matter Repo](https://github.com/TexasInstruments/matter))
- Zigbee software protocol stack (Z-stack - learn more at [TI.com/Zigbee](https://www.ti.com/Zigbee))
- Supports the concurrent operation of BLE + Sub-1 GHz (TI 15.4-Stack or proprietary Sub-1 GHz) and BLE + Zigbee using the Dynamic Multi-Protocol
- Supports Amazon Sidewalk Sub-1 GHz FSK and Bluetooth Low Energy
 
## Example Repos
 
- [Bluetooth Low Energy (BLE)](https://github.com/TexasInstruments/simplelink-ble5stack-examples)
- [Proprietary RF](https://github.com/TexasInstruments/simplelink-prop_rf-examples)
- [Amazon Sidewalk](https://github.com/TexasInstruments/simplelink-sidewalk-examples)
- [DMM](https://github.com/TexasInstruments/simplelink-dmm-examples)
- [TI 15.4](https://github.com/TexasInstruments/simplelink-ti154stack-examples)
- [ZStack](https://github.com/TexasInstruments/simplelink-zstack-examples)
- [TI WISunFan](https://github.com/TexasInstruments/simplelink-ti_wisunfan-examples)


 
## Setup Instructions
 
### Edit **imports.mak**
The imports.mak file, located at the root of the SDK, is a generic sample and will not work out of the box.   Please update imports.mak with the tools (compilers, cmake, etc.) paths installed on your system.
 
For a Linux build, settings must be updated to match your build system's setup.  The only outlier may be Python, as most python3.6+ interpreters will work.  Please note cmake must be 3.21+, or the builds will fail.  If using CCS ccs1220, the SysConfig installed is incompatible with the SDKs.  Either upgrade CCS to ccs1230 or install SysConfig 1.16.1 from [https://www.ti.com/tool/SYSCONFIG](https://www.ti.com/tool/SYSCONFIG]). Please note XDC_INSTALL_DIR was required in older SDKs but is no longer needed.
 
By default TICLANG and GCC toolchains are enabled.  If a toolchain is not needed, unset the compiler, for example, `GCC_ARMCOMPILER ?=`.
 
### Default imports.mak
 
`XDC_INSTALL_DIR`        ?= /home/username/ti/xdctools_3_62_01_15_core (Not required for 7.10+ SDK's)
 
`SYSCONFIG_TOOL`         ?= /home/username/ti/ccs1230/ccs/utils/sysconfig_1.16.1/sysconfig_cli.sh
 
`FREERTOS_INSTALL_DIR`   ?= /home/username/FreeRTOSv202104.00
 
`CMAKE`                 ?= /home/username/cmake-3.21.3/bin/cmake
 
`PYTHON`                 ?= python3
 
`TICLANG_ARMCOMPILER`    ?= /home/username/ti/ccs1230/ccs/tools/compiler/ti-cgt-armllvm_2.1.3.LTS-0
 
`GCC_ARMCOMPILER`        ?= /home/username/ti/ccs1230/ccs/tools/compiler/9.2019.q4.major-0
 
Edit **imports.mak** and update all of the above tool location variables to reflect the paths on your build system.
 
## Build SDK Libraries
SDK libraries must exist before building any examples.  After editing imports.mak, build SDK libraries from the root  of the SDK (the same directory where imports.mak resides) by:
 
$ `make`
 
The make will go through the SDK and build all libraries.  Depending on the build machine, the make will run for a few minutes.  
 
 
## Supported Devices
 
All devices supported by the SimpleLink Low Power F2 SDK.
 
## More Information
For licensing information, please refer to licensing.
 
## Technical Support
Please consider creating a post on [TI's E2E forum](https://e2e.ti.com). Additionally, we welcome any feedback.
