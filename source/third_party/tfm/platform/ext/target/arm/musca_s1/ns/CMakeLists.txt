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
        Device/Source/startup_musca.c
        Device/Source/system_core_init.c
        Device/Source/device_definition.c
        uart_pl011_drv.c
        Native_Driver/timer_cmsdk_drv.c
        CMSIS_Driver/Driver_USART.c
        $<$<BOOL:${PLATFORM_GPLED_ENABLED}>:services/src/tfm_ioctl_ns_api.c>
        $<$<BOOL:${PLATFORM_GPLED_ENABLED}>:services/src/tfm_gpled_api.c>
)

target_include_directories(platform_ns
    PUBLIC
        include
        ext/cmsis
        ext/common
        Native_Driver
        CMSIS_Driver/Config
        CMSIS_Driver/include
        Device/Include
        partition
        $<$<BOOL:${PLATFORM_GPLED_ENABLED}>:services/include>
)

target_include_directories(platform_region_defs
    INTERFACE
        partition
)
