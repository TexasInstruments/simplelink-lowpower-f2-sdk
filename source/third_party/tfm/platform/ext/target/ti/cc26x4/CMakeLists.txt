#-------------------------------------------------------------------------------
# Copyright (c) 2020-2023, Arm Limited. All rights reserved.
# Copyright (c) 2023-2024, Texas Instruments Incorporated. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_policy(SET CMP0076 NEW)
set(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

#========================= Platform region defs ===============================#
target_include_directories(platform_region_defs
    INTERFACE
        partition
        ${GENERATED_FLASH_LAYOUT_DIR}
)

#========================= Platform common defs ===============================#

# Specify the location of platform specific build dependencies.
target_sources(tfm_s
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_core/startup_cc26x4.c
)

target_add_scatter_file(tfm_s
    $<$<C_COMPILER_ID:ARMClang>:${PLATFORM_DIR}/ext/common/armclang/tfm_common_s.sct>
    $<$<C_COMPILER_ID:GNU>:${PLATFORM_DIR}/ext/common/gcc/tfm_common_s.ld>
    $<$<C_COMPILER_ID:IAR>:${PLATFORM_DIR}/ext/common/iar/tfm_common_s.icf>
)

if(BL2)
    message(FATAL_ERROR "BL2 build is not suppported.")
endif()

#========================= Platform Secure ====================================#

target_include_directories(platform_s
    PUBLIC
        .
        cmsis_core
        cmsis_drivers
        native_drivers
        partition
        retarget
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/../..
        ${DRIVERLIB_SOURCE_DIR}
        ${TIDRIVERS_INSTALL_DIR}/source/
        ${GENERATED_FLASH_LAYOUT_DIR}
)

target_sources(platform_s
    PRIVATE
        $<$<AND:$<BOOL:${TFM_PARTITION_INITIAL_ATTESTATION}>,$<NOT:$<BOOL:${PLATFORM_DEFAULT_ATTEST_HAL}>>>:
            ${CMAKE_CURRENT_SOURCE_DIR}/attest_hal.c>
        $<$<BOOL:${TI_CRYPTO_ITS_INTEGRATION}>:${CMAKE_SOURCE_DIR}/platform/ext/common/tfm_hal_its.c>
        platform_svc.c
        cmsis_core/system_core_init.c
        cmsis_drivers/Driver_Flash.c
        cmsis_drivers/Driver_USART.c
        native_drivers/arm_uart_drv.c
        native_drivers/mpu_armv8m_drv.c
        retarget/platform_retarget_dev.c
        $<$<OR:$<BOOL:${TFM_S_REG_TEST}>,$<BOOL:${TFM_NS_REG_TEST}>>:${CMAKE_CURRENT_SOURCE_DIR}/plat_test.c>
        $<$<BOOL:${TFM_PARTITION_PLATFORM}>:${CMAKE_CURRENT_SOURCE_DIR}/services/src/tfm_platform_system.c>

        # Driver Porting Layer needed for Driver_Flash.c
        ${TIDRIVERS_INSTALL_DIR}/kernel/tfm/dpl/HwiP_tfm.c
)

# Link driverlib for flash functions
target_link_libraries(platform_s
    PRIVATE
        $<$<C_COMPILER_ID:ARMClang>:${DRIVERLIB_SOURCE_DIR}/ti/devices/cc13x4_cc26x4/driverlib/bin/ticlang/driverlib.lib>
        $<$<C_COMPILER_ID:GNU>:${DRIVERLIB_SOURCE_DIR}/ti/devices/cc13x4_cc26x4/driverlib/bin/gcc/driverlib.lib>
        $<$<C_COMPILER_ID:IAR>:${DRIVERLIB_SOURCE_DIR}/ti/devices/cc13x4_cc26x4/driverlib/bin/iar/driverlib.lib>
)

target_compile_options(platform_s
    PUBLIC
        ${COMPILER_CMSE_FLAG}
)

target_compile_definitions(platform_s
    PRIVATE
        $<$<BOOL:${TFM_S_REG_TEST}>:USART_BUFFER_FOR_TEST>
)

#========================= tfm_spm ============================================#

target_sources(tfm_spm
    PRIVATE
        target_cfg.c
        tfm_hal_isolation.c
        tfm_hal_platform.c
)

#========================= Files for building NS platform =====================#

install(FILES       ${PLATFORM_DIR}/ext/common/test_interrupt.c
                    ${TARGET_PLATFORM_PATH}/native_drivers/arm_uart_drv.c
                    ${TARGET_PLATFORM_PATH}/cmsis_drivers/Driver_USART.c
                    ${TARGET_PLATFORM_PATH}/retarget/platform_retarget_dev.c
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

install(DIRECTORY   ${TARGET_PLATFORM_PATH}/cmsis_core
                    ${TARGET_PLATFORM_PATH}/retarget
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

install(DIRECTORY   ${PLATFORM_DIR}/ext/driver
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/include)

install(FILES       ${TARGET_PLATFORM_PATH}/partition/region_defs.h
                    ${GENERATED_FLASH_LAYOUT_DIR}/flash_layout.h
                    ${TARGET_PLATFORM_PATH}/native_drivers/arm_uart_drv.h
                    ${TARGET_PLATFORM_PATH}/retarget/platform_retarget.h
                    ${TARGET_PLATFORM_PATH}/target_cfg.h
                    ${TARGET_PLATFORM_PATH}/device_cfg.h
                    ${TARGET_PLATFORM_PATH}/tfm_peripherals_def.h
                    ${TARGET_PLATFORM_PATH}/cmsis_driver_config.h
                    ${TARGET_PLATFORM_PATH}/RTE_Device.h
                    ${PLATFORM_DIR}/ext/common/test_interrupt.h
                    ${PLATFORM_DIR}/ext/driver/Driver_USART.h
                    ${PLATFORM_DIR}/ext/driver/Driver_Common.h
                    ${PLATFORM_DIR}/include/tfm_plat_defs.h
                    ${CMAKE_SOURCE_DIR}/lib/fih/inc/fih.h
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/include)

install(FILES       ${PLATFORM_DIR}/ext/common/gcc/tfm_common_ns.ld
                    ${PLATFORM_DIR}/ext/common/armclang/tfm_common_ns.sct
                    ${PLATFORM_DIR}/ext/common/iar/tfm_common_ns.icf
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/linker_scripts)

# Copy all files from active platform directory
install(DIRECTORY   ${TARGET_PLATFORM_PATH}/ns/
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

# Copy the platform specific CPU architecture
install(FILES       ${TARGET_PLATFORM_PATH}/cpuarch.cmake
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

# Copy the platform specific config
install(FILES       ${TARGET_PLATFORM_PATH}/config.cmake
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})
