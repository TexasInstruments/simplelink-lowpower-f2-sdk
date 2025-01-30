#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited. All rights reserved.
# Copyright (c) 2021 STMicroelectronics. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

########################## BL2 #################################################
set(MCUBOOT_IMAGE_NUMBER                2           CACHE STRING    "Whether to combine S and NS into either 1 image, or sign each seperately")
set(BL2_HEADER_SIZE                     0x400       CACHE STRING    "Header size")
set(BL2_TRAILER_SIZE                    0x2000      CACHE STRING    "Trailer size")
set(MCUBOOT_ALIGN_VAL                   16          CACHE STRING    "Align option to build image with imgtool")
set(MCUBOOT_UPGRADE_STRATEGY            "SWAP_USING_SCRATCH"      CACHE STRING    "Upgrade strategy for images")
set(TFM_PARTITION_FIRMWARE_UPDATE       OFF          CACHE BOOL      "Enable firmware update partition")
set(TFM_PARTITION_PLATFORM              ON          CACHE BOOL      "Enable platform partition")
set(MCUBOOT_DATA_SHARING                ON          CACHE BOOL      "Enable Data Sharing")
set(MCUBOOT_BOOTSTRAP                   ON          CACHE BOOL      "Allow initial state with images in secondary slots(empty primary slots)")
set(MCUBOOT_ENC_IMAGES                  ON          CACHE BOOL      "Enable encrypted image upgrade support")
set(MCUBOOT_ENCRYPT_RSA                 ON          CACHE BOOL      "Use RSA for encrypted image upgrade support")
################################## Dependencies ################################
set(TFM_PARTITION_INTERNAL_TRUSTED_STORAGE ON       CACHE BOOL      "Enable Internal Trusted Storage partition")
set(TFM_PARTITION_CRYPTO                ON          CACHE BOOL      "Enable Crypto partition")
set(CRYPTO_HW_ACCELERATOR               ON          CACHE BOOL      "Whether to enable the crypto hardware accelerator on supported platforms")
set(MBEDCRYPTO_BUILD_TYPE               minsizerel  CACHE STRING    "Build type of Mbed Crypto library")
set(PS_CRYPTO_AEAD_ALG                  PSA_ALG_GCM CACHE STRING    "The AEAD algorithm to use for authenticated encryption in Protected Storage")
set(MCUBOOT_FIH_PROFILE                 LOW         CACHE STRING    "Fault injection hardening profile [OFF, LOW, MEDIUM, HIGH]")
# LOG LEVEL
set(TFM_SPM_LOG_LEVEL                   TFM_SPM_LOG_LEVEL_DEBUG         CACHE STRING    "Set default SPM log level as INFO level")
set(TFM_PARTITION_LOG_LEVEL             TFM_PARTITION_LOG_LEVEL_INFO    CACHE STRING    "Set default Secure Partition log level as INFO level")

set(MCUBOOT_HW_ROLLBACK_PROT            ON          CACHE BOOL      "Enable security counter validation against non-volatile HW counters")
# Added for H5
set(CONFIG_TFM_USE_TRUSTZONE            ON)
set(TFM_PARTITION_PROTECTED_STORAGE     ON         CACHE BOOL      "Disable Protected Storage partition")
set(TFM_PARTITION_INITIAL_ATTESTATION   ON         CACHE BOOL      "Disable Initial Attestation partition")
set(STSAFEA                             ON          CACHE BOOL      "Activate ST SAFE SUPPORT")
