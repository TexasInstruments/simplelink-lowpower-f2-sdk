#-------------------------------------------------------------------------------
# Copyright (c) 2020-2023, Arm Limited. All rights reserved.
# Copyright (c) 2023, Texas Instruments Incorporated. All rights reserved.
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

if(NS)
    target_sources(tfm_ns
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_core/startup_cc26x4.c
    )
    target_add_scatter_file(tfm_ns
        $<$<C_COMPILER_ID:ARMClang>:${PLATFORM_DIR}/ext/common/armclang/tfm_common_ns.sct>
        $<$<C_COMPILER_ID:GNU>:${PLATFORM_DIR}/ext/common/gcc/tfm_common_ns.ld>
        $<$<C_COMPILER_ID:IAR>:${PLATFORM_DIR}/ext/common/iar/tfm_common_ns.icf>
    )
    target_link_libraries(CMSIS_5_tfm_ns
        INTERFACE
            $<$<OR:$<BOOL:${CONFIG_TFM_ENABLE_FP}>,$<BOOL:${CONFIG_TFM_ENABLE_MVE_FP}>>:CMSIS_5_RTX_V8MMFN>
            $<$<NOT:$<OR:$<BOOL:${CONFIG_TFM_ENABLE_FP}>,$<BOOL:${CONFIG_TFM_ENABLE_MVE_FP}>>>:CMSIS_5_RTX_V8MMN>
    )
    target_compile_options(tfm_ns
        PUBLIC
            ${COMPILER_CP_FLAG}
    )
    target_link_options(tfm_ns
        PUBLIC
            ${LINKER_CP_OPTION}
    )
endif()

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
        ${SOURCE_REPO}
        ${GENERATED_FLASH_LAYOUT_DIR}
)

target_sources(platform_s
    PRIVATE
        $<$<AND:$<BOOL:${TFM_PARTITION_INITIAL_ATTESTATION}>,$<NOT:$<BOOL:${PLATFORM_DEFAULT_ATTEST_HAL}>>>:
            ${CMAKE_CURRENT_SOURCE_DIR}/attest_hal.c>
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
        ${KERNEL_REPO}/tfm/dpl/HwiP_tfm.c
)

# Link driverlib for flash functions
target_link_libraries(platform_s
    PRIVATE
        $<$<C_COMPILER_ID:ARMClang>:${SOURCE_REPO}/ti/devices/cc13x4_cc26x4/driverlib/bin/ticlang/driverlib.lib>
        $<$<C_COMPILER_ID:GNU>:${SOURCE_REPO}/ti/devices/cc13x4_cc26x4/driverlib/bin/gcc/driverlib.lib>
        $<$<C_COMPILER_ID:IAR>:${SOURCE_REPO}/ti/devices/cc13x4_cc26x4/driverlib/bin/iar/driverlib.lib>
)

target_compile_options(platform_s
    PUBLIC
        ${COMPILER_CMSE_FLAG}
)

target_compile_definitions(platform_s
    PRIVATE
        $<$<BOOL:${TFM_S_REG_TEST}>:USART_BUFFER_FOR_TEST>
)

#========================= Platform Non-Secure ================================#

target_sources(platform_ns
    PRIVATE
        native_drivers/arm_uart_drv.c
        cmsis_drivers/Driver_USART.c
        retarget/platform_retarget_dev.c
        cmsis_core/system_core_init.c
)

target_include_directories(platform_ns
    PUBLIC
        .
        retarget
        cmsis_core
        native_drivers
        ${SOURCE_REPO}
)

target_compile_definitions(platform_ns
    PRIVATE
        __DOMAIN_NS=1
        $<$<BOOL:${TFM_NS_REG_TEST}>:USART_BUFFER_FOR_TEST>
)

#========================= tfm_spm ============================================#

target_sources(tfm_spm
    PRIVATE
        target_cfg.c
        tfm_hal_isolation.c
        tfm_hal_platform.c
)
