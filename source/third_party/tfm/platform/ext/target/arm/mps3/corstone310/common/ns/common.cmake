#-------------------------------------------------------------------------------
# Copyright (c) 2020-2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_policy(SET CMP0076 NEW)

set(CONFIG_TFM_FP_ARCH "fpv5-d16")
set(CONFIG_TFM_FP_ARCH_ASM "FPv5_D16")

#========================= Platform region defs ===============================#

target_include_directories(platform_region_defs
    INTERFACE
        ${CORSTONE310_COMMON_DIR}/partition
        ${CMAKE_CURRENT_SOURCE_DIR}
)

#========================= Platform common defs ===============================#

target_sources(tfm_ns
    PRIVATE
        ${CORSTONE310_COMMON_DIR}/device/source/startup_corstone310.c
        ${CORSTONE310_COMMON_DIR}/device/source/system_core_init.c
)

#========================= Device definition lib ===============================#

add_library(device_definition INTERFACE)
target_include_directories(device_definition
    INTERFACE
        ${CORSTONE310_COMMON_DIR}
        ${CORSTONE310_COMMON_DIR}/device/include
        ${CORSTONE310_COMMON_DIR}/native_drivers
        ${CORSTONE310_COMMON_DIR}/partition
        ${PLATFORM_DIR}/ext/target/arm/drivers/flash/common
        ${PLATFORM_DIR}/ext/target/arm/drivers/usart/cmsdk
        ${PLATFORM_DIR}/ext/target/arm/drivers/usart/common
        ${PLATFORM_DIR}/ext/target/arm/drivers/mpc_sie
        ${PLATFORM_DIR}/ext/target/arm/drivers/mpu/armv8m
        ${PLATFORM_DIR}/ext/target/arm/drivers/counter/armv8m
        ${PLATFORM_DIR}/ext/target/arm/drivers/timer/armv8m
        ${ETHOS_DRIVER_PATH}/src
        ${ETHOS_DRIVER_PATH}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/device/config
)

add_library(device_definition_ns STATIC)
target_sources(device_definition_ns
    PUBLIC
        ${CORSTONE310_COMMON_DIR}/device/source/platform_ns_device_definition.c
)

#========================= CMSIS lib ===============================#

add_library(cmsis_includes INTERFACE)
target_include_directories(cmsis_includes
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/device/config
        ${CORSTONE310_COMMON_DIR}/device/include
        ${CORSTONE310_COMMON_DIR}/cmsis_drivers
        ${PLATFORM_DIR}/ext/cmsis
        ${CORSTONE310_COMMON_DIR}/partition
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/driver
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/common
)

add_library(cmsis_includes_ns INTERFACE)
target_link_libraries(cmsis_includes_ns INTERFACE cmsis_includes)
target_include_directories(cmsis_includes_ns
    INTERFACE
        ${CORSTONE310_COMMON_DIR}/cmsis_drivers/config/non_secure
)

#========================= Linking ===============================#

target_link_libraries(device_definition_ns PUBLIC device_definition)
target_link_libraries(device_definition_ns PRIVATE cmsis_includes_ns)

target_link_libraries(platform_ns
    PUBLIC
        cmsis_includes_ns
    PRIVATE
        device_definition_ns
)

#========================= Platform Non-Secure ================================#

target_sources(platform_ns
    PRIVATE
        ${CORSTONE310_COMMON_DIR}/cmsis_drivers/Driver_USART.c
        ${PLATFORM_DIR}/ext/target/arm/drivers/usart/cmsdk/uart_cmsdk_drv.c
    INTERFACE
        $<$<BOOL:${TEST_NS_FPU}>:${CORSTONE300_COMMON_DIR}/device/source/corstone310_ns_init.c>
        $<$<BOOL:${TEST_NS_FPU}>:${PLATFORM_DIR}/ext/common/test_interrupt.c>
)

target_compile_definitions(platform_ns
    PUBLIC
        $<$<BOOL:${TEST_NS_FPU}>:TEST_NS_FPU>
)
