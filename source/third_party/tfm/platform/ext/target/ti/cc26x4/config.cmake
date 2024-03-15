#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited. All rights reserved.
# Copyright (c) 2023, Texas Instruments Incorporated. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# config.cmake is the default build configuration for this platform.

# Platform-specific fixed configuration
set(CONFIG_TFM_USE_TRUSTZONE          ON )
set(TFM_MULTI_CORE_TOPOLOGY           OFF)
set(CONFIG_TFM_ENABLE_FP              ON    CACHE BOOL    "Enable/disable FP usage")

# TF-M's bundled secondary bootloader is not used.
set(BL2                               OFF   CACHE BOOL    "Whether to build BL2")

# NS app is built outside of TF-M build process.
set(NS                                OFF   CACHE BOOL    "Whether to build NS app")

# Disable FPU lazy stacking until TIRTOS7 can support it.
set(CONFIG_TFM_LAZY_STACKING          OFF   CACHE BOOL    "Enable/disable lazy stacking")

# Uncomment this one TI platform-specific attestation HAL is implemented.
# set(PLATFORM_DEFAULT_ATTEST_HAL       OFF   CACHE BOOL    "Use default attest hal implementation.")

if(NOT TEST_S AND NOT TEST_NS)
    # Disable TF-M Crypto partition as this platform implements a proprietary crypto partition.
    set(TFM_PARTITION_CRYPTO              OFF   CACHE BOOL    "Enable Crypto partition")
endif()

if(BL2)
    message(FATAL_ERROR "Only TI's standalone secondary bootloader is supported.")
else()
    # TI's secondary bootloader specifies a header size of 256-bytes and trailer of 4KB.
    set(BL2_HEADER_SIZE         0x100       CACHE STRING    "BL2 Header size")
    set(BL2_TRAILER_SIZE        0x1000      CACHE STRING    "BL2 Trailer size")
endif()
