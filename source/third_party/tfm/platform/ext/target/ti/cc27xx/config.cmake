#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited. All rights reserved.
# Copyright (c) 2024, Texas Instruments Incorporated. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# config.cmake is the default build configuration for this platform.

# Platform-specific compile defines
add_compile_definitions(
    # Define for TI custom memory layout for region_defs.h
    TI_CUSTOM_MEMORY_LAYOUT

    # Defines to build TI drivers
    DeviceFamily_CC27XX
    TFM_ENABLED=1
    TFM_BUILD
)

# Platform-specific fixed configuration
set(CONFIG_TFM_USE_TRUSTZONE          ON )
set(TFM_MULTI_CORE_TOPOLOGY           OFF)
set(CONFIG_TFM_ENABLE_FP              ON    CACHE BOOL    "Enable/disable FP usage")

# TF-M's bundled secondary bootloader is not used.
set(BL2                               OFF   CACHE BOOL    "Whether to build BL2")

# Disable FPU lazy stacking until TIRTOS7 can support it.
set(CONFIG_TFM_LAZY_STACKING          OFF   CACHE BOOL    "Enable/disable lazy stacking")

# Uncomment this once TI platform-specific attestation HAL is implemented.
# set(PLATFORM_DEFAULT_ATTEST_HAL       OFF   CACHE BOOL    "Use default attest hal implementation.")

if(NOT TEST_S AND NOT TEST_NS)
    # Disable TF-M Crypto partition as this platform implements a proprietary crypto partition.
    set(TFM_PARTITION_CRYPTO              OFF   CACHE BOOL    "Enable Crypto partition")

    # Set to "ON" or comment out the following to enable platform-specific built-in key support.
    # See docs/design_docs/tfm_builtin_keys.rst for prerequisites.
    set(CRYPTO_TFM_BUILTIN_KEYS_DRIVER    OFF   CACHE BOOL    "Whether to allow crypto service to store builtin keys. Without this, ALL builtin keys must be stored in a platform-specific location")
endif()

if(BL2)
    message(FATAL_ERROR "Only TI's standalone secondary bootloader is supported.")
else()
    # TI's secondary bootloader specifies a header size of 256-bytes and trailer of 4KB.
    set(BL2_HEADER_SIZE         0x100       CACHE STRING    "BL2 Header size")
    set(BL2_TRAILER_SIZE        0x1000      CACHE STRING    "BL2 Trailer size")
endif()
