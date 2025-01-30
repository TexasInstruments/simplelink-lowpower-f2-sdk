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

target_sources(flash_drivers_s
    PRIVATE
        cmsis_drivers/Driver_Flash.c
)

target_sources(flash_drivers_bl2
    PRIVATE
        cmsis_drivers/Driver_Flash_bl2.c
)

target_include_directories(flash_drivers
    INTERFACE
        ${PLATFORM_DIR}/ext/target/arm/drivers/qspi/xilinx_pg153_axi
        ${PLATFORM_DIR}/ext/target/arm/drivers/flash/sst26vf064b
        ${PLATFORM_DIR}/ext/target/arm/drivers/flash/common
        ${PLATFORM_DIR}/ext/driver
        device/include
        ${CORSTONE300_COMMON_DIR}/device/config
)

target_sources(flash_drivers
    INTERFACE
        ${PLATFORM_DIR}/ext/target/arm/drivers/flash/sst26vf064b/spi_sst26vf064b_flash_lib.c
        ${PLATFORM_DIR}/ext/target/arm/drivers/qspi/xilinx_pg153_axi/xilinx_pg153_axi_qspi_controller_drv.c
        device/source/flash_device_definition.c
)

include(${CORSTONE300_COMMON_DIR}/libflash_drivers.cmake)

#========================= Files for building NS side platform ================#

install(FILES       ${TARGET_PLATFORM_PATH}/partition/platform_base_address.h
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/partition)

install(DIRECTORY   ${TARGET_PLATFORM_PATH}/device
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

install(FILES       ${TARGET_PLATFORM_PATH}/ns/cpuarch_ns.cmake
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}
        RENAME      cpuarch.cmake)

install(FILES       ${TARGET_PLATFORM_PATH}/ns/CMakeLists.txt
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

install(FILES       ${TARGET_PLATFORM_PATH}/config.cmake
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})
