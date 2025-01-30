#-------------------------------------------------------------------------------
# Copyright (c) 2020-2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_policy(SET CMP0076 NEW)

include(${CORSTONE300_COMMON_DIR}/common.cmake)

add_library(flash_drivers INTERFACE)
add_library(flash_drivers_s STATIC)
add_library(flash_drivers_bl2 STATIC)

target_include_directories(flash_drivers
    INTERFACE
        ${PLATFORM_DIR}/ext/driver
        ${CMAKE_CURRENT_SOURCE_DIR}/partition
        ${CORSTONE300_COMMON_DIR}/partition
        ${PLATFORM_DIR}/ext/target/arm/drivers/flash/common
        ${PLATFORM_DIR}/ext/target/arm/drivers/flash/emulated
)

target_sources(flash_drivers
    INTERFACE
        cmsis_drivers/Driver_Flash.c
        ${PLATFORM_DIR}/ext/target/arm/drivers/flash/emulated/emulated_flash_drv.c
)

include(${CORSTONE300_COMMON_DIR}/libflash_drivers.cmake)

#========================= Files for building NS side platform ================#

install(FILES       ${TARGET_PLATFORM_PATH}/partition/platform_base_address.h
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/partition)

install(FILES       ${TARGET_PLATFORM_PATH}/ns/cpuarch_ns.cmake
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}
        RENAME      cpuarch.cmake)

install(FILES       ${TARGET_PLATFORM_PATH}/ns/CMakeLists.txt
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

install(FILES       ${TARGET_PLATFORM_PATH}/config.cmake
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

install(FILES       ${TARGET_PLATFORM_PATH}/cmsis_drivers/Driver_Flash.c
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})
