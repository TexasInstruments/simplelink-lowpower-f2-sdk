# Copyright (c) 2022, Texas Instruments Incorporated
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
include(CMakeSources.cmake)
ti_init_package(NAMESPACE TiUtils PACKAGE_NAME Json)

foreach(TI_ARCH ${TI_ALL_SUPPORTED_ARCHITECTURES})
    ti_add_library(json_${TI_ARCH}
        SOURCES ${json_sources}
        # Specify _release until we have a common _debug vs _release CMake configuration paradigm
        OUTPUT_NAME json_release
        # Specify the default compiler settings and other toolchain-related configurations
        LINK_PRIVATE CMakeCommon::${TI_TOOLCHAIN_NAME}_${TI_ARCH}
        # Add include path to source directory of Conan package for any downstream CMake dependencies
        LINK_PUBLIC TiUtils::tiutils
    )
    # Set defines for library build
    target_compile_definitions(json_${TI_ARCH}
    PRIVATE
        ALLOW_PARSING__TEMPLATE
        ALLOW_PARSING__JSON
        USE__STANDARD_LIBS
    )
endforeach()

# Generate public package information files needed for other CMake projects to depend on tiutils
ti_export_package()
