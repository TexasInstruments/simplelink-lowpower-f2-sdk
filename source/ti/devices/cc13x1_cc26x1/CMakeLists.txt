# Copyright (c) 2023, Texas Instruments Incorporated
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
include(${CMAKE_CURRENT_LIST_DIR}/CMakeSources.cmake)

# All targets in this file are for Cortex-M4F
set(TI_ARCH m4)

# For customer builds, this package is defined by platform imports, so the
# targets already exist. There is no SDK that only has cc13x1 but not cc26x1
if (NOT TARGET Device::cc13x1)
    ti_init_package(NAMESPACE Device PACKAGE_NAME CC13X1CC26X1 PACKAGE_TYPE PLATFORM)

    # Add compile definition if the DRIVERLIB_DEBUG option is ON
    if (DRIVERLIB_DEBUG)
        add_compile_definitions(DRIVERLIB_DEBUG)
        message(STATUS "Option DRIVERLIB_DEBUG=ON: Debug asserts enabled (cc13x1, cc26x1)")
    endif (DRIVERLIB_DEBUG)

    # Meta-library for device, this causes DeviceFamily to be defined for downstream consumers
    ti_add_library(device_cc13x1 INTERFACE EXPORT_NAME cc13x1 LINK_INTERFACE CMakeCommon::${TI_TOOLCHAIN_NAME}_${TI_ARCH})
    target_compile_definitions(device_cc13x1 INTERFACE DeviceFamily_CC13X1)

    # Meta-library for device, this causes DeviceFamily to be defined for downstream consumers
    ti_add_library(device_cc26x1 INTERFACE EXPORT_NAME cc26x1 LINK_INTERFACE CMakeCommon::${TI_TOOLCHAIN_NAME}_${TI_ARCH})
    target_compile_definitions(device_cc26x1 INTERFACE DeviceFamily_CC26X1)

    ti_export_package()
endif ()

# Driverlib package is a component
ti_init_package(NAMESPACE Driverlib PACKAGE_NAME CC13X1CC26X1)

# Driverlib libraries currently still use the .lib file ending. We default to
# .a, so explicitly set it
set(CMAKE_STATIC_LIBRARY_SUFFIX ".lib")

# We need to add a common library that actually builds the .a since it is shared
# by the other device library interfaces
ti_add_library(
    driverlib_cc13x1_cc26x1
    SOURCES ${driverlib_sources}
    # The resulting library will be named Driverlib::cc13x1_cc26x1
    EXPORT_NAME cc13x1_cc26x1
    OUTPUT_NAME driverlib
    # Pass on DeviceFamily from above
    LINK_PRIVATE
        # We do not want to link against a specific device since those are
        # created below. So we link against the architecture + toolchain instead
        CMakeCommon::${TI_TOOLCHAIN_NAME}_${TI_ARCH}
        # Emit the final files into a bin/ folder directly, with no architecture/tool component
        OUTPUT_FOLDER driverlib/bin/${TI_TOOLCHAIN_NAME}
    ADD_SDK_INCLUDE_PATH source
)

# We are not actually building any libraries for this interface
ti_add_library(
    driverlib_cc13x1
    # The resulting library will be named Driverlib::cc13x1
    EXPORT_NAME cc13x1
    INTERFACE
    # Pass on DeviceFamily and library content from above
    LINK_INTERFACE
        Device::cc13x1
        # Link against actual .a library
        driverlib_cc13x1_cc26x1
        # Emit the final files into a bin/ folder directly, with no architecture/tool component
        OUTPUT_FOLDER driverlib/bin/${TI_TOOLCHAIN_NAME}
    ADD_SDK_INCLUDE_PATH source
)

# We are not actually building any libraries for this interface
ti_add_library(
    driverlib_cc26x1
    # The resulting library will be named Driverlib::cc26x1
    EXPORT_NAME cc26x1
    INTERFACE
    # Pass on DeviceFamily and library content from above
    LINK_INTERFACE
        Device::cc26x1
        # Link against actual .a library
        driverlib_cc13x1_cc26x1
        # Emit the final files into a bin/ folder directly, with no architecture/tool component
        OUTPUT_FOLDER driverlib/lib/${TI_TOOLCHAIN_NAME}
    ADD_SDK_INCLUDE_PATH source
)

ti_export_package()
