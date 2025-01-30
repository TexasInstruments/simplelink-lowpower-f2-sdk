#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(TFM_PARTITION_MEASURED_BOOT               OFF  CACHE BOOL   "Disable Measured boot partition")
set(TFM_PARTITION_DELEGATED_ATTESTATION       OFF  CACHE BOOL   "Disable Delegated Attestation partition")
set(TFM_PARTITION_PROTECTED_STORAGE            ON  CACHE BOOL   "Enable Protected Storage partition")
set(TFM_PARTITION_INTERNAL_TRUSTED_STORAGE    OFF  CACHE BOOL   "Disable Internal Trusted Storage partition")

set(TFM_EXTRAS_REPO_PATH                       ""  CACHE PATH   "Path to tf-m-extras repo (or DOWNLOAD to fetch automatically")
set(TFM_EXTRAS_REPO_VERSION                    ""  CACHE STRING "The version of tf-m-extras to use")
set(TFM_EXTRAS_REPO_EXTRA_PARTITIONS           ""  CACHE STRING "List of extra secure partition directory name(s)")
set(TFM_EXTRAS_REPO_EXTRA_MANIFEST_LIST        ""  CACHE STRING "List of extra secure partition manifests")

set(CONFIG_TFM_BOOT_STORE_ENCODED_MEASUREMENTS ON  CACHE BOOL   "Enable storing of encoded measurements in boot.")

set(TFM_MBEDCRYPTO_PLATFORM_EXTRA_CONFIG_PATH  ""  CACHE PATH   "Config to append to standard Mbed Crypto config, used by platforms to cnfigure feature support")

set(TFM_MANIFEST_LIST                          "${CMAKE_CURRENT_LIST_DIR}/manifest/tfm_manifest_list.yaml" CACHE PATH "Config to append to standard Mbed Crypto config, used by platforms to cnfigure feature support")

set(PLATFORM_HAS_PS_NV_OTP_COUNTERS            ON  CACHE BOOL   "Platform supports nvm counters for PS in OTP")

# Once all options are set, set common options as fallback
include(${CMAKE_CURRENT_LIST_DIR}/../common/config.cmake)
