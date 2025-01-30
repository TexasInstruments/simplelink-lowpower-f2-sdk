#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_policy(SET CMP0076 NEW)
set(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../core nrf_common)

target_sources(tfm_ns
    PRIVATE
        $<$<C_COMPILER_ID:GNU>:${CMAKE_CURRENT_LIST_DIR}/gcc/startup_nrf5340.c>
)

target_include_directories(platform_ns
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(platform_ns
    PRIVATE
        ${HAL_NORDIC_PATH}/nrfx/mdk/system_nrf5340_application.c
)

target_compile_definitions(platform_ns
    PUBLIC
        NRF_TRUSTZONE_NONSECURE
        DOMAIN_NS=1
)
