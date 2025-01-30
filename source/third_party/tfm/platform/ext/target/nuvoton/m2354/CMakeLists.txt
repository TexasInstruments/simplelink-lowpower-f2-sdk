#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited. All rights reserved.
# Copyright (c) 2020 Nuvoton Technology Corp. All rights reserved.
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
        ${CMAKE_CURRENT_SOURCE_DIR}/device/source/startup_m2354.c
)
target_add_scatter_file(tfm_s
    $<$<AND:$<VERSION_LESS:${TFM_ISOLATION_LEVEL},3>,$<C_COMPILER_ID:ARMClang>>:${CMAKE_SOURCE_DIR}/platform/ext/common/armclang/tfm_common_s.sct>
    $<$<AND:$<VERSION_LESS:${TFM_ISOLATION_LEVEL},3>,$<C_COMPILER_ID:GNU>>:${CMAKE_SOURCE_DIR}/platform/ext/common/gcc/tfm_common_s.ld>
    $<$<AND:$<VERSION_LESS:${TFM_ISOLATION_LEVEL},3>,$<C_COMPILER_ID:IAR>>:${CMAKE_SOURCE_DIR}/platform/ext/common/iar/tfm_common_s.icf>
)

if(BL2)
    target_sources(bl2
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/device/source/startup_m2354.c
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
        ../common/
        ../common/cmsis_drivers/config
        ../common/native_drivers
        ../common/retarget
        partition
        device/include
        device/config
        bsp/Device/Nuvoton/M2354/Include
        bsp/Library/StdDriver/inc
        ../common/bsp/Include
        ../common/bsp/StdDriver/inc
        ${PLATFORM_DIR}/ext/cmsis
        ${PLATFORM_DIR}/ext/driver
)

target_sources(platform_s
    PRIVATE
        device/source/system_core_init.c
        ../common/cmsis_drivers/Driver_Flash.c
        ../common/cmsis_drivers/Driver_USART.c
        ../common/native_drivers/arm_uart_drv.c
        ../common/native_drivers/mpu_armv8m_drv.c
        ../common/native_drivers/timer_cmsdk_drv.c
        ../common/native_drivers/timer_cmsdk.c
        ../common/retarget/platform_retarget_dev.c
        bsp/Library/StdDriver/src/timer.c
        bsp/Library/StdDriver/src/clk.c
        bsp/Library/StdDriver/src/sys.c
        bsp/Library/StdDriver/src/fmc.c
        bsp/Library/StdDriver/src/tamper.c
        ../common/services/src/tfm_platform_system.c
        ${PLATFORM_DIR}/ext/common/uart_stdout.c
        $<$<BOOL:${TFM_PARTITION_PLATFORM}>:${CMAKE_CURRENT_SOURCE_DIR}/plat_test.c>
)

target_compile_options(platform_s
    PUBLIC
        ${COMPILER_CMSE_FLAG}
)

#========================= Platform BL2 =======================================#

if(BL2)
    target_sources(platform_bl2
        PRIVATE
            device/source/system_core_init.c
            ../common/cmsis_drivers/Driver_Flash.c
            ../common/native_drivers/arm_uart_drv.c
            ../common/cmsis_drivers/Driver_USART.c
            ../common/retarget/platform_retarget_dev.c
            bsp/Library/StdDriver/src/fmc.c
            bsp/Library/StdDriver/src/tamper.c
    )
    target_include_directories(platform_bl2
        PUBLIC
            partition
            bsp/Device/Nuvoton/M2354/Include
            bsp/Library/StdDriver/inc
            ../common/bsp/Include
            ../common/bsp/StdDriver/inc
            device/include
            device/config
            ../common/cmsis_drivers/config
        PRIVATE
            .
            ../common/
            ${PLATFORM_DIR}/..
            ../common/native_drivers
    )
endif()

#========================= tfm_spm ============================================#

target_sources(tfm_spm
    PRIVATE
        target_cfg.c
        ../common/tfm_hal_isolation.c
        ../common/tfm_hal_platform.c
        ../common/faults.c
        $<$<OR:$<BOOL:${CONFIG_TFM_FLIH_API}>,$<BOOL:${CONFIG_TFM_SLIH_API}>>:${PLATFORM_DIR}/ext/common/tfm_interrupts.c>
)



#========================= Files for building NS platform =====================#


if(NS)
    target_sources(tfm_ns
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/device/source/startup_m2354.c
    )
    target_add_scatter_file(tfm_ns
        $<$<C_COMPILER_ID:ARMClang>:${CMAKE_CURRENT_SOURCE_DIR}/device/source/armclang/m2354_ns.sct>
        $<$<C_COMPILER_ID:GNU>:${CMAKE_CURRENT_SOURCE_DIR}/device/source/gcc/m2354_ns.ld>
        $<$<C_COMPILER_ID:IAR>:${CMAKE_CURRENT_SOURCE_DIR}/device/source/iar/m2354_ns.icf>
    )
    target_link_libraries(CMSIS_5_tfm_ns
        INTERFACE
            CMSIS_5_RTX_V8MBN
    )
endif()


install(FILES       ${TARGET_PLATFORM_PATH}/device/source/startup_m2354.c
                    ${PLATFORM_DIR}/ext/target/nuvoton/common/native_drivers/arm_uart_drv.c
                    ${PLATFORM_DIR}/ext/target/nuvoton/common/native_drivers/timer_cmsdk.c
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

install(DIRECTORY   ${TARGET_PLATFORM_PATH}/device
                    ${TARGET_PLATFORM_PATH}/bsp
                    ${TARGET_PLATFORM_PATH}/../common/cmsis_drivers
                    ${TARGET_PLATFORM_PATH}/../common/retarget
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

install(DIRECTORY   ${TARGET_PLATFORM_PATH}/../common/bsp/Include
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/bsp/Device/Nuvoton/M2354)

install(DIRECTORY   ${PLATFORM_DIR}/ext/target/nuvoton/common/bsp/StdDriver/inc
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/bsp/Library/StdDriver)

install(DIRECTORY   ${PLATFORM_DIR}/ext/driver
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/include)

install(FILES       ${PLATFORM_DIR}/ext/target/nuvoton/common/native_drivers/arm_uart_drv.h
                    ${PLATFORM_DIR}/ext/target/nuvoton/common/native_drivers/timer_cmsdk.h
                    ${PLATFORM_DIR}/ext/target/nuvoton/common/native_drivers/uart_cmsdk_drv.h
                    ${PLATFORM_DIR}/ext/target/nuvoton/common/native_drivers/timer_cmsdk_drv.h
                    ${PLATFORM_DIR}/ext/target/nuvoton/common/target_cfg.h
                    ${PLATFORM_DIR}/ext/target/nuvoton/common/tfm_peripherals_def.h
                    ${PLATFORM_DIR}/ext/target/nuvoton/common/bsp/Include/acmp_reg.h
                    ${PLATFORM_DIR}/ext/target/nuvoton/common/bsp/Include/bpwm_reg.h
                    ${PLATFORM_DIR}/ext/target/nuvoton/common/bsp/Include/crc_reg.h
                    ${PLATFORM_DIR}/ext/target/nuvoton/common/bsp/Include/dac_reg.h
                    ${PLATFORM_DIR}/ext/driver/Driver_USART.h
                    ${PLATFORM_DIR}/ext/driver/Driver_Common.h
                    ${PLATFORM_DIR}/include/tfm_plat_defs.h
                    ${CMAKE_SOURCE_DIR}/lib/fih/inc/fih.h
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/include)

install(DIRECTORY   ${TARGET_PLATFORM_PATH}/partition
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

# copy all files from active platform directory
install(DIRECTORY   ${TARGET_PLATFORM_PATH}/ns/
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

install(FILES       ${TARGET_PLATFORM_PATH}/cpuarch.cmake
                    ${TARGET_PLATFORM_PATH}/config.cmake
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

install(FILES       ${TARGET_PLATFORM_PATH}/device/source/gcc/m2354_ns.ld
                    ${TARGET_PLATFORM_PATH}/device/source/armclang/m2354_ns.sct
                    ${TARGET_PLATFORM_PATH}/device/source/iar/m2354_ns.icf
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/linker_scripts)

# Install test configs
install(DIRECTORY   ${TARGET_PLATFORM_PATH}/tests
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})