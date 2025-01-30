#-------------------------------------------------------------------------------
# Copyright (c) 2022-2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_policy(SET CMP0076 NEW)

set(PLATFORM_DIR    ${CMAKE_CURRENT_LIST_DIR})

add_library(platform_ns STATIC)

include(${CORSTONE310_COMMON_DIR}/common.cmake)

target_include_directories(device_definition
    INTERFACE
        libraries
        native_drivers
)
