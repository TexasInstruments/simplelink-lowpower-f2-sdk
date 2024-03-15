# Boot Image manager for OAD On-chip ( Single Image Mode )

## Table of Contents

* [Introduction](#Introduction)
* [Hardware Prerequisites](#HardwarePrerequisites)
* [Software Prerequisites](#SoftwarePrerequisites)
* [Usage](#Usage)
* [Optional Features](#OptionalFeatures)

## <a name="Introduction"></a>Introduction

The Boot Image manager (BIM) project demonstrates how to select, copy and run an
image located in the internal flash. This project supports two variants on on-chip
OAD methodologies. One case is where there is a user application & the other image
present on internal flash is the persistent application. The other case is the
dual-image OAD case, where the internal flash is treated like two slots and there
can be two user applications, one in each slot. The dual-image BIM is explained in
more details under dual/README.md. BIM always performs sign verifcation check &
security version check, if applicable, to deem an image valid & boot into it.

This project is independent of any protocol stack.

## <a name="HardwarePrerequisites"></a>Hardware Prerequisites

The default configuration uses the
[LAUNCHXL-CC1352R1](http://www.ti.com/tool/launchxl-cc1352r1).

For custom hardware, see the **Running the SDK on Custom Hardware** section of
the [BLE5-Stack User's
Guide](../../../../../docs/ble5stack/ble_user_guide/ble5stack-users-guide.html).

## <a name="SoftwarePrerequisites"></a>Software Prerequisites

For information on what versions of Code Composer Studio and IAR Embedded
Workbench to use, see the dependencies section of the Release Notes located
in the root folder of the SDK.

For information on how to import this project into your IDE workspace and
build/run, please refer to the device's Platform section in the [BLE5-Stack
Quick Start
Guide](../../../../../docs/simplelink_mcu_sdk/html/quickstart-guide/ble5-quick-start.html).

## <a name="Usage"></a>Usage

**Note: The ``bim_onchip`` project must be flashed to the device when
using on-chip OAD or the device will not function correctly, after flashing the
persistent application and the user application images, in the same order that is
mentioned here**

This project is independent of any protocol stack.

Five build configurations are provided for on-chip BIM:

* `Release_unsecure`
* `Debug_unsecure`
* `Release`
* `Debug`

`Release_unsecure` is to be flashed when flashing the user application .bin and doing unsecure OAD
i.e. NO image authentication is done before executing the newly transferred (OAD) image.
Release is to be used when doing secure OAD i.e. Image authentication needs to be done before
executing the newly transferred image.

`Debug` versions are provided for debugging purposes i.e. when the application image is
downloaded using the debugger in the IDE. In this mode, the post build step of calculating the CRC
or signing of the image is not done and hence, the sign or CRC is not available in the header
of the image downloaded to internal flash with the debugger.
Additionally, in Debug configurations, LED indications are available
for visual feedback.

## <a name="OptionalFeatures"></a>Optional Features

The following definitions were added in the Q4 2020 release. They provide
additional security and boot time improvements. By default, they are not used
and thus backwards compatibility is maintained.

`AUTHENTICATE_PERSISTENT_IMG` - This can be used to provide additional security
to your OAD solution. If this is defined, the BIM will verify the CRC of the
persistent image, authenticate the signature and verify that the prgEntry addr
is within the bounds of the image. All of the above security improvements are
only useful if your persistent image is signed via the oad_image_tool using a
post build step.

`APP_HDR_LOC` and `PERSIST_HDR_LOC` defines can be used to pre-define the
location of the U-App and the P-App respectively. This is only useful if you
know the specific location of your U-App or P-App before launching the product
into the field. By defining `PERSIST_HDR_LOC`, you will also see improved BIM
boot up times as the BIM no longer needs to search through the internal flash
for the P-App header.
