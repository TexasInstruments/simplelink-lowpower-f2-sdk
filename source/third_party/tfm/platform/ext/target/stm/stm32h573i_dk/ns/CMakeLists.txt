#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_policy(SET CMP0076 NEW)


#========================= Platform Non-Secure ================================#

add_library(platform_ns)

target_sources(platform_ns
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/Device/Source/startup_stm32h5xx_ns.c
)

target_include_directories(platform_ns
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/boards
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/Device/Include
        ${CMAKE_CURRENT_LIST_DIR}/ext/cmsis
        ${CMAKE_CURRENT_LIST_DIR}/ext/common
        ${CMAKE_CURRENT_LIST_DIR}/ext/driver
        ${CMAKE_CURRENT_LIST_DIR}/hal/Inc
)

target_sources(platform_ns
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/hal/Src/stm32h5xx_hal.c
        ${CMAKE_CURRENT_SOURCE_DIR}/Device/Source/system_stm32h5xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hal/CMSIS_Driver/low_level_com.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hal/Src/stm32h5xx_hal_dma.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hal/Src/stm32h5xx_hal_dma_ex.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hal/Src/stm32h5xx_hal_pwr.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hal/Src/stm32h5xx_hal_pwr_ex.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hal/Src/stm32h5xx_hal_rcc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hal/Src/stm32h5xx_hal_gpio.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hal/Src/stm32h5xx_hal_uart.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hal/Src/stm32h5xx_hal_uart_ex.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hal/Src/stm32h5xx_hal_cortex.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hal/Src/stm32h5xx_hal_rcc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hal/Src/stm32h5xx_hal_rcc_ex.c
)

#========================= Platform region defs ===============================#

target_include_directories(platform_region_defs
    INTERFACE
        include
        ${CMAKE_CURRENT_LIST_DIR}/boards
)

target_compile_definitions(platform_region_defs
    INTERFACE
        $<$<OR:$<BOOL:${TEST_NS}>,$<BOOL:${TEST_S}>>:FLASH_LAYOUT_FOR_TEST>
)
