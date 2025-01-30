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
        cmsis_core/startup_an521.c
        cmsis_core/system_core_init.c
        retarget/platform_retarget_dev.c
    INTERFACE
        $<$<BOOL:${TEST_NS_FPU}>:${CMAKE_CURRENT_LIST_DIR}/cmsis_core/an521_ns_init.c>
        $<$<BOOL:${TEST_NS_FPU}>:${CMAKE_CURRENT_LIST_DIR}/test_interrupt.c>
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

target_compile_definitions(platform_ns
    PUBLIC
        $<$<BOOL:${TEST_NS_FPU}>:TEST_NS_FPU>
)

# Include region_defs.h and flash_layout.h
target_include_directories(platform_region_defs
    INTERFACE
        include
)
