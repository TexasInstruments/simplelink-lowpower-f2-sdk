#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited. All rights reserved.
# Copyright (c) 2023, Texas Instruments Incorporated. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# preload.cmake is used to set things that related to the platform that are both
# immutable and global, which is to say they should apply to any kind of project
# that uses this platform. In practice this is normally compiler definitions and
# variables related to hardware.

# Set CPU and architecture
set(TFM_SYSTEM_PROCESSOR cortex-m33)
set(TFM_SYSTEM_ARCHITECTURE armv8-m.main)
set(CONFIG_TFM_FP_ARCH "fpv5-sp-d16")
set(CONFIG_TFM_FP_ARCH_ASM "FPv5-SP")

# Add define for TI custom memory layout for region_defs.h
add_definitions(-DTI_CUSTOM_MEMORY_LAYOUT)

# Add defines necessary to build TI drivers
add_definitions(-DDeviceFamily_CC26X4)
add_definitions(-DTFM_ENABLED=1)
add_definitions(-DTFM_BUILD)