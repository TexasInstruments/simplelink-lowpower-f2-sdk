#-------------------------------------------------------------------------------
# Copyright (c) 2020-2023, Arm Limited. All rights reserved.
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
)

#========================= Platform common defs ===============================#

# Specify the location of platform specific build dependencies.
target_sources(tfm_s
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Device/Source/startup_musca.c
)
target_add_scatter_file(tfm_s
    $<$<C_COMPILER_ID:ARMClang>:${PLATFORM_DIR}/ext/common/armclang/tfm_common_s.sct>
    $<$<C_COMPILER_ID:GNU>:${PLATFORM_DIR}/ext/common/gcc/tfm_common_s.ld>
    $<$<C_COMPILER_ID:IAR>:${PLATFORM_DIR}/ext/common/iar/tfm_common_s.icf>
)

if(BL2)
    target_sources(bl2
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/Device/Source/startup_musca.c
    )
    target_add_scatter_file(bl2
            $<$<C_COMPILER_ID:ARMClang>:${PLATFORM_DIR}/ext/common/armclang/tfm_common_bl2.sct>
            $<$<C_COMPILER_ID:GNU>:${PLATFORM_DIR}/ext/common/gcc/tfm_common_bl2.ld>
            $<$<C_COMPILER_ID:IAR>:${PLATFORM_DIR}/ext/common/iar/tfm_common_bl2.icf>
    )
endif()

#========================= Platform Secure ====================================#

target_include_directories(platform_s
    PUBLIC
        .
        CMSIS_Driver
        CMSIS_Driver/Config
        Device/Config
        Device/Include
        Native_Driver
        partition
        services/include
        Libraries
        driver
        ${PLATFORM_DIR}/ext/target/arm/drivers/usart/pl011
    INTERFACE
        cc312
)

target_sources(platform_s
    PRIVATE
        CMSIS_Driver/Driver_QSPI_Flash.c
        CMSIS_Driver/Driver_Flash_MRAM.c
        CMSIS_Driver/Driver_MPC.c
        CMSIS_Driver/Driver_PPC.c
        CMSIS_Driver/Driver_USART.c
        Device/Source/device_definition.c
        Device/Source/system_core_init.c
        Native_Driver/mpc_sie200_drv.c
        Native_Driver/mpu_armv8m_drv.c
        Native_Driver/ppc_sse200_drv.c
        Native_Driver/gpio_cmsdk_drv.c
        Native_Driver/musca_s1_scc_drv.c
        Native_Driver/qspi_ip6514e_drv.c
        Native_Driver/cache_drv.c
        ${PLATFORM_DIR}/ext/target/arm/drivers/usart/pl011/uart_pl011_drv.c
        ${CMAKE_CURRENT_SOURCE_DIR}/Native_Driver/timer_cmsdk_drv.c
        Libraries/mt25ql_flash_lib.c
        ${CMAKE_CURRENT_SOURCE_DIR}/plat_test.c
        $<$<BOOL:${TFM_PARTITION_PLATFORM}>:${CMAKE_CURRENT_SOURCE_DIR}/services/src/tfm_platform_system.c>
)

target_compile_options(platform_s
    PUBLIC
        ${COMPILER_CMSE_FLAG}
)

# To configure S and NS timer in S side for FP interrupt test
target_compile_definitions(platform_s
    PUBLIC
        $<$<BOOL:${TEST_NS_FPU}>:TEST_NS_FPU>
        $<$<BOOL:${TEST_S_FPU}>:TEST_S_FPU>
)

#========================= Platform BL2 =======================================#

if(BL2)
    target_sources(platform_bl2
        PRIVATE
            Device/Source/system_core_init.c
            Device/Source/device_definition.c
            CMSIS_Driver/Driver_Flash_MRAM.c
            ${PLATFORM_DIR}/ext/target/arm/drivers/usart/pl011/uart_pl011_drv.c
            Native_Driver/cache_drv.c
            Native_Driver/musca_s1_scc_drv.c
            CMSIS_Driver/Driver_USART.c
    )

    target_include_directories(platform_bl2
        PUBLIC
            partition
            Device/Include
            Libraries
        INTERFACE
            cc312
        PRIVATE
            .
            CMSIS_Driver/Config
            Device/Config
            Native_Driver
            ${PLATFORM_DIR}/ext/target/arm/drivers/usart/pl011
            ${CMAKE_SOURCE_DIR}/bl2/ext/mcuboot/include # for fih.h interop only
            ${MCUBOOT_PATH}/boot/bootutil/include # for fault_injection_hardening.h only
            ${CMAKE_BINARY_DIR}/bl2/ext/mcuboot # for mcuboot_config.h only
    )

    target_compile_options(platform_bl2
        PRIVATE
            ${BL2_COMPILER_CP_FLAG}
    )
endif()

#========================= tfm_spm ============================================#

target_sources(tfm_spm
    PRIVATE
        target_cfg.c
        tfm_hal_isolation.c
        tfm_hal_platform.c
        faults.c
        tfm_interrupts.c
)

#========================= Files for building NS side platform ================#

install(FILES       ${PLATFORM_DIR}/ext/target/arm/drivers/usart/pl011/uart_pl011_drv.c
                    Libraries/mt25ql_flash_lib.c
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}
)

install(DIRECTORY   Native_Driver
                    CMSIS_Driver
                    services
                    partition
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}
)

install(FILES       target_cfg.h
                    Libraries/mt25ql_flash_lib.h
                    tfm_peripherals_def.h
                    Device/Config/device_cfg.h
                    ${PLATFORM_DIR}/ext/target/arm/drivers/usart/pl011/uart_pl011_drv.h
                    ${PLATFORM_DIR}/ext/accelerator/cc312/crypto_accelerator_config.h
                    ${PLATFORM_DIR}/include/tfm_plat_defs.h
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/include
)

install(DIRECTORY   Device/Include
                    Device/Source
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/Device
)

install(FILES       ${PLATFORM_DIR}/ext/driver/Driver_Common.h
                    ${PLATFORM_DIR}/ext/driver/Driver_Flash.h
                    ${PLATFORM_DIR}/ext/driver/Driver_USART.h
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/include)

# copy all files from active platform directory
install(DIRECTORY   ${TARGET_PLATFORM_PATH}/ns/
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

install(FILES       ${TARGET_PLATFORM_PATH}/cpuarch.cmake
                    ${TARGET_PLATFORM_PATH}/config.cmake
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

# Install linker scripts
install(FILES       ${PLATFORM_DIR}/ext/common/armclang/tfm_common_ns.sct
                    ${PLATFORM_DIR}/ext/common/gcc/tfm_common_ns.ld
                    ${PLATFORM_DIR}/ext/common/iar/tfm_common_ns.icf
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/linker_scripts)

# Install test configs
install(DIRECTORY   ${TARGET_PLATFORM_PATH}/tests
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})
