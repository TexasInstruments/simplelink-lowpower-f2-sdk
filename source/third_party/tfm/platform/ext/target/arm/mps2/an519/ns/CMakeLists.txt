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
        arm_uart_drv.c
        timer_cmsdk.c
        Driver_USART.c
        cmsis_core/startup_an519.c
        cmsis_core/system_core_init.c
        retarget/platform_retarget_dev.c
)

target_include_directories(platform_ns
    PRIVATE
        retarget
    PUBLIC
        include
        ext/cmsis
        cmsis_core
        ext/common
)

# Include region_defs.h and flash_layout.h
target_include_directories(platform_region_defs
    INTERFACE
        partition
)
