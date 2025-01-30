#-------------------------------------------------------------------------------
# Copyright (c) 2019-2023 Texas Instruments Incorporated
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.15)

add_subdirectory(crypto_sp)
add_subdirectory(internal_storage_sp)
add_subdirectory(tdc)

# Install files for Conan package
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/deploy.cmake)

# Install README file
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/README.md DESTINATION ${CMAKE_INSTALL_PREFIX})
