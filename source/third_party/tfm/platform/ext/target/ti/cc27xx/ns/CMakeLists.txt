#-------------------------------------------------------------------------------
# Copyright (c) 2024, Texas Instruments Incorporated. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

add_library(platform_ns)

target_sources(platform_ns
    PRIVATE
        arm_uart_drv.c
        Driver_USART.c
        cmsis_core/system_core_init.c
        retarget/platform_retarget_dev.c
    PUBLIC
        cmsis_core/startup_cc27xx.c
        driverlib/ccfg.c
        driverlib/scfg.c
)

target_include_directories(platform_ns
    PRIVATE
        retarget
    PUBLIC
        driverlib/inc
        include
        cmsis_core
        ext/common
        ext/cmsis
        ${DRIVERLIB_SOURCE_DIR}
)

target_compile_definitions(platform_ns
    PRIVATE
        __DOMAIN_NS=1
        $<$<BOOL:${TFM_NS_REG_TEST}>:USART_BUFFER_FOR_TEST>
)

# Include dir for region_defs.h and flash_layout.h
target_include_directories(platform_region_defs
    INTERFACE
        include
)
