#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# This file is exported to NS side during CMake installation phase and renamed
# to CMakeLists.txt. It instructs how to build a platform on non-secture side.
# The structure and sources list are fully platform specific.

add_library(platform_ns)

target_sources(platform_ns
    PRIVATE
        Driver_USART.c
        device/source/startup_an524.c
        device/source/system_core_init.c
        device_definition.c
        uart_cmsdk_drv.c
        timer_cmsdk_drv.c
)

target_include_directories(platform_ns
    PRIVATE
        retarget
    PUBLIC
        ext/cmsis
        device
        device/config
        device/include
        include
        ext/common
)

# Include region_defs.h and flash_layout.h
target_include_directories(platform_region_defs
    INTERFACE
        partition
)
