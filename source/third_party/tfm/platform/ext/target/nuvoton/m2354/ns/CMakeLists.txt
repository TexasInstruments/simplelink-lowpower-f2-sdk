#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
# Copyright (c) 2023, Nuvoton Technology Corp. All rights reserved.
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
        cmsis_drivers/Driver_USART.c
        startup_m2354.c
        retarget/platform_retarget_dev.c
        device/source/system_core_init.c
)

target_include_directories(platform_ns
    PUBLIC
        bsp/Library/StdDriver/inc
        bsp/Device/Nuvoton/M2354/Include
        retarget
        partition
        include
        ext/cmsis
        ext/common
        cmsis_drivers/config
        device/config
        device/include
)


# Include region_defs.h and flash_layout.h
target_include_directories(platform_region_defs
    INTERFACE
        partition
)
