# Copyright (c) 2022-2023, Texas Instruments Incorporated
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# *  Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# *  Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# *  Neither the name of Texas Instruments Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
cmake_minimum_required(VERSION ${TI_MIN_CMAKE_VERSION})

# Set FREERTOS_INSTALL_DIR to SDK root if it is not explicitly defined
get_install_dir(FREERTOS_INSTALL_DIR)

include(CMakeSources.cmake)
ti_init_package(NAMESPACE FreeRTOS)

# Add a top-level FreeRTOS library
ti_add_library(freertos INTERFACE ARCH all)
target_include_directories(
    freertos
    INTERFACE "$<BUILD_INTERFACE:${FREERTOS_INSTALL_DIR}/source/third_party/freertos/include>"
              # Note: we cannot set the INSTALL_INTERFACE include here, because it depends on
              # FREERTOS_INSTALL_DIR It is configured in ConfigFreeRTOS.cmake.in, after the CMake
              # package magic
)

set(TI_LIBRARIES)

# The IAR libraries have sources, and therefore need to link against the toolchain libraries
if(${TI_TOOLCHAIN_NAME} STREQUAL iar)
    ti_add_library(
        freertos_m0p ARCH m0p OUTPUT_NAME freertos SOURCES ${SOURCES_M0P_IAR} LINK_PRIVATE CMakeCommon::iar_m0p
        LINK_PUBLIC FreeRTOS::freertos
    )
    # Internal include needed for FreeRTOSConfig.h
    target_include_directories(
        freertos_m0p PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/internal/fpu_0_max_syscall_pri_1>"
        # There is no INSTALL_INTERFACE include here, as it depends on FREERTOS_INSTALL_DIR It is
        # configured in ConfigFreeRTOSMxx.cmake.in, after the CMake package magic
    )

    ti_add_library(
        freertos_m4 ARCH m4 OUTPUT_NAME freertos SOURCES ${SOURCES_M4_IAR} LINK_PRIVATE CMakeCommon::iar_m4
        LINK_PUBLIC FreeRTOS::freertos
    )
    # Internal include needed for FreeRTOSConfig.h
    target_include_directories(
        freertos_m4 PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/internal/fpu_0_max_syscall_pri_1>"
        # There is no INSTALL_INTERFACE include here, as it depends on FREERTOS_INSTALL_DIR It is
        # configured in ConfigFreeRTOSMxx.cmake.in, after the CMake package magic
    )

    ti_add_library(
        freertos_m4f ARCH m4f OUTPUT_NAME freertos SOURCES ${SOURCES_M4F_IAR} LINK_PRIVATE CMakeCommon::iar_m4f
        LINK_PUBLIC FreeRTOS::freertos
    )
    # Internal include needed for FreeRTOSConfig.h
    target_include_directories(
        freertos_m4f PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/internal/fpu_1_max_syscall_pri_1>"
        # There is no INSTALL_INTERFACE include here, as it depends on FREERTOS_INSTALL_DIR It is
        # configured in ConfigFreeRTOSMxx.cmake.in, after the CMake package magic
    )

    ti_add_library(
        freertos_m33f ARCH m33f OUTPUT_NAME freertos SOURCES ${SOURCES_M33F_IAR} LINK_PRIVATE CMakeCommon::iar_m33f
        LINK_PUBLIC FreeRTOS::freertos
    )
    # Internal include needed for FreeRTOSConfig.h
    target_include_directories(
        freertos_m33f PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/internal/fpu_1_max_syscall_pri_1>"
        # There is no INSTALL_INTERFACE include here, as it depends on FREERTOS_INSTALL_DIR It is
        # configured in ConfigFreeRTOSMxx.cmake.in, after the CMake package magic
    )

    ti_add_library(
        freertos_m33f_tfm ARCH m33f OUTPUT_NAME freertos_tfm SOURCES ${SOURCES_M33F_IAR}
        LINK_PRIVATE CMakeCommon::iar_m33f LINK_PUBLIC FreeRTOS::freertos
    )
    # Internal include needed for FreeRTOSConfig.h
    target_include_directories(
        freertos_m33f_tfm PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/internal/fpu_1_max_syscall_pri_2>"
        # There is no INSTALL_INTERFACE include here, as it depends on FREERTOS_INSTALL_DIR It is
        # configured in ConfigFreeRTOSMxx.cmake.in, after the CMake package magic
    )
else()
    # The other libraries are interfaces primarily used for including the correct portable implementation
    ti_add_library(freertos_m0p ARCH m0p INTERFACE LINK_INTERFACE FreeRTOS::freertos)
    ti_add_library(freertos_m4 ARCH m4 INTERFACE LINK_INTERFACE FreeRTOS::freertos)
    ti_add_library(freertos_m4f ARCH m4f INTERFACE LINK_INTERFACE FreeRTOS::freertos)
    ti_add_library(freertos_m33f ARCH m33f INTERFACE LINK_INTERFACE FreeRTOS::freertos)
    ti_add_library(freertos_m33f_tfm ARCH m33f INTERFACE LINK_INTERFACE FreeRTOS::freertos)

    # Since we usually clean the ticlang variant of the build, we need to explicitly clean the lib folder here
    # Interface libraries do not appear to contribute ADDITIONAL_CLEAN_FILES if set as a target property
    set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES lib/)
endif()

# These variables are used in ConfigFreeRTOSMxx.cmake.in to set the INSTALL_INTERFACE includes These
# variables only get interpolated inside the IAR-specific exported Targets files
set(freertos_m0p_PORT_TOOL_FOLDER "IAR/ARM_CM0")
set(freertos_m4_PORT_TOOL_FOLDER "IAR/ARM_CM3")
set(freertos_m4f_PORT_TOOL_FOLDER "IAR/ARM_CM4F")
set(freertos_m33f_PORT_TOOL_FOLDER "IAR/ARM_CM33F_NTZ/non_secure")
set(freertos_m33f_tfm_PORT_TOOL_FOLDER "IAR/ARM_CM33F_NTZ/non_secure")

ti_export_package(CUSTOM_TEMPLATE)
