# Boot Image Manager for OAD On-chip (Dual-Image Mode)

## Table of Contents

* [Introduction](#Introduction)
* [BIM Mechanism](#BIMMechanism)
* [Usage](#Usage)
* [Hardware Prerequisites](#HardwarePrerequisites)
* [Software Prerequisites](#SoftwarePrerequisites)
* [Optional Features](#OptionalFeatures)

## <a name="Introduction"></a>Introduction

The Boot Image Manager (BIM) in Dual-Image Mode project demonstrates how to select, copy and run an
image located in the internal flash. This project supports dual-image Mode OAD case.
The internal flash is treated like two slots and there can be two user applications,
one in each slot. The dual-image BIM variant provides anti-roll back protection, by
preventing booting into an image with lower security version than the most recently ran valid
image. BIM in dual-image also supports restricted rollback, erasing invalid images and image version
verification. BIM always performs sign verification check & security version check, if applicable,
to deem an image valid & boot into it.

This project is independent of any protocol stack.

## <a name="BIMMechanism"></a>BIM Mechanism

The Boot Image Manager is comprised of mainly two phases:

* *Image Identification and Validation Phase*
* *Image Selection and Boot-in Phase*

### <a name="IDVP"></a>Image Identification and Validation Phase

This phase answers two main questions: How many images exist on flash and whether they are valid? BIM does this by scanning the two pre-defined slot locations and determines whether an image exists in either slot. If an image exists, BIM verifies it and marks it as either `IMG_TYPE_IMAGE_PRESENT & IMG_TYPE_VALID_APPSTACKLIB_IMG` or `IMG_TYPE_IMAGE_PRESENT & IMG_TYPE_INVALID_APPSTACKLIB_IMG`. If an image does not exist in a slot, then that slot is marked as `IMG_TYPE_NO_IMAGE_PRESENT`

The Verification checks that BIM performs on each slot are the following:

* Image Detection
* Image Integrity Checks (Mainly CRC32)
* Image Authenticity Checks
* Image Security Checks (Ex. ECDSA-SHA256) *Note: only in Release build configuration*

### <a name="ISBP"></a>Image Selection and Boot-in Phase

This phase picks up the image(s) that the first phase marked as valid and Boots into one.

* If one image was marked as valid then, BIM boots into it.
* If two images were marked as valid then, BIM identifies the newly OADed image and moves to compare it against the most recent running image.

Baseline BIM compares the security version of the new image against the security version of the most recently ran image. When Both images have the same security versions? The old image is downgraded but NOT invalidated. Effectively making it possible to boot into it in the future.

| secVer_new vs. secVer_old |   New Image Status    | Old Image             |
|    :---:                  |       :---:           |   :--:                |
| secVer_new > secVer_old   |       ACCEPTED        |   Erase or Invalidate |
| secVer_new < secVer_old   |        DENIED         |  Boot into it         |
| secVer_new == secVer_old  |       ACCEPTED        | Keep Old              |

BIM with the `BIM_VERIFY_VERSION_IMAGE` macro enabled expands baseline checks to include both images' version numbers comparison. This is an optional feature, read Optional Features.

| imgVer_new vs. imgVer_old |   New Image Status    |  Old Image            |
|    :---:                  |     :---:             |    :--:               |
| imgVer_new > imgVer_old   | ACCEPTED              |  Erase or Invalidate  |
| imgVer_new < imgVer_old   | DENIED                |   Boot Into           |
| imgVer_new == imgVer_old  | DENIED                |   Boot Into           |

## <a name="Usage"></a>Usage

**Note: The ``bim_dual_onchip`` project must be flashed to the device when
using on-chip OAD or the device will not function correctly, after flashing the
persistent application and the user application images, in the same order that is
mentioned here**

This project is independent of any protocol stack.

Two build configurations are provided for on-chip BIM:

* `Release`
* `Debug`

`Debug` versions are provided for debugging purposes i.e. when the application image is
downloaded using the debugger in the IDE. In this mode, the post build step of calculating the CRC
or signing of the image is not done and hence, the sign or CRC is not available in the header
of the image downloaded to internal flash with the debugger.
Additionally, in Debug configurations, LED indications are available
for visual feedback.

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

## <a name="OptionalFeatures"></a>Optional Features

The following definitions were added in the Q3 2023 release. They provide
additional security and boot time improvements. By default, they are not used
and thus backwards compatibility is maintained.

* `BIM_ERASE_INVALID_IMAGE` - By enabling this macro, invalid images will be erased by BIM. By default this feature is disabled.


* `BIM_VERIFY_VERSION_IMAGE` - By enabling this macro, Boot Image Manager will enable Image Version checks. By default this feature is disabled.

    **NOTE: When enabling this macro, BIM will always accept one image and throw up the other one either by invalidating it so it can't be booted into in the future or by completely wiping it from flash. Unlike with baseline bim, where loading an image with the same security version will boot into it but also preserve the old image so it can be booted into in the future**

    With image version and security version, both numbers are related to each other. When upgrading one, the user has to consider whether it is necessary to upgrade the other. A general Rule of thumb is: if updating an image's security version, the image's version number has to be updated as well. However, an upgrade in an image's version number does not require an upgrade to the image's security version since the upgrade might not be security-related


* `BIM_RESTRICTED_ROLLBACK_VERIFY_COMMIT_IMAGE` - By enabling this macro, BIM will delay invalidating the old image once the new image has been verified until the application gives control back to BIM and indicates whether it wants to be committed or rejected. By default this feature is disabled.

    Assumptions:
    * A new OAD image has to pass BIM's *Image Selection and Boot-in Phase* Checks and ready to be booted into
    * The application is responsible to "P.O.S.T" and mark itself as committed or rejected and force a rest to give control back to BIM

    When BIM takes control:
    * If image is committed, BIM will invalidate (or erase) old image and the security and image versions of the new image become the standard
    * If image is rejected, BIM will invalidate the new image during the *Image Identification and Validation Phase*

    A very likely scenario:
    * When an app with a higher security version marks itself as rejected, BIM will boot into a previously committed image that **MIGHT** have a lower security version
    Rollback is not possible once an image has been committed
