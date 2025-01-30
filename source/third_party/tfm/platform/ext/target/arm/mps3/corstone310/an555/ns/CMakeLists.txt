#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# This file is exported to NS side during CMake installation phase and renamed
# to CMakeLists.txt. It instructs how to build a platform on non-secture side.
# The structure and sources list are fully platform specific.

cmake_policy(SET CMP0076 NEW)

set(PLATFORM_DIR    ${CMAKE_CURRENT_LIST_DIR})

add_library(platform_ns STATIC)

include(${CORSTONE310_COMMON_DIR}/common.cmake)

