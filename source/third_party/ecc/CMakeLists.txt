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
cmake_minimum_required(VERSION 3.21.3)
include(${CMAKE_CURRENT_LIST_DIR}/CMakeSources.cmake OPTIONAL)

project(ecc LANGUAGES C ASM)
ti_init_package(NAMESPACE ThirdPartyEccLib)

if (TI_INTERNAL_BUILD)
    # Not using get_install_dir for INVIA_LIB_INSTALL_DIR as it does not ship
    list(TRANSFORM SOURCES_CC13X1_CC26X1 PREPEND ${INVIA_LIB_INSTALL_DIR}/source/third_party/ecc/src/)
    list(TRANSFORM SOURCES_CC23X0 PREPEND ${INVIA_LIB_INSTALL_DIR}/source/third_party/ecc/src/)
    list(TRANSFORM SOURCES_CC27XX PREPEND ${INVIA_LIB_INSTALL_DIR}/source/third_party/ecc/src/)

    if ("m4" IN_LIST SUPPORTED_ARCHITECTURES)
        ti_add_library(
            ecc_cc13x1_cc26x1
            SOURCES ${SOURCES_CC13X1_CC26X1} ${CMAKE_SOURCE_DIR}/source/third_party/ecc/include/ECCSW.h
            ARCH m4
            LINK_PUBLIC CMakeCommon::${TI_TOOLCHAIN_NAME}_m4
        )
    endif ()

    if ("m33f" IN_LIST SUPPORTED_ARCHITECTURES)
        ti_add_library(
            ecc_cc27xx
            OUTPUT_NAME ecc
            SOURCES ${SOURCES_CC27XX} ${CMAKE_SOURCE_DIR}/source/third_party/ecc/include/ECCSW.h
            ARCH m33f
            LINK_PUBLIC CMakeCommon::${TI_TOOLCHAIN_NAME}_m33f
        )
        target_include_directories(ecc_cc27xx INTERFACE "${CMAKE_SOURCE_DIR}/source")

        if (${TI_TOOLCHAIN_NAME} STREQUAL "ticlang" OR ${TI_TOOLCHAIN_NAME} STREQUAL "gcc")
            target_sources(ecc_cc27xx PUBLIC ${INVIA_LIB_INSTALL_DIR}/source/third_party/ecc/src/lowlevelapi_m0_gnu.s)
        elseif (${TI_TOOLCHAIN_NAME} STREQUAL "iar")
            target_sources(ecc_cc27xx PUBLIC ${INVIA_LIB_INSTALL_DIR}/source/third_party/ecc/src/lowlevelapi_m0_iar.s)
        endif ()        
    endif ()

    if ("m0p" IN_LIST SUPPORTED_ARCHITECTURES)
        ti_add_library(
            ecc_cc23x0
            OUTPUT_NAME ecc
            SOURCES ${SOURCES_CC23X0} ${CMAKE_SOURCE_DIR}/source/third_party/ecc/include/ECCSW.h
            ARCH m0p
            LINK_PUBLIC CMakeCommon::${TI_TOOLCHAIN_NAME}_m0p
        )
        target_include_directories(ecc_cc23x0 INTERFACE "${CMAKE_SOURCE_DIR}/source")

        if (${TI_TOOLCHAIN_NAME} STREQUAL "ticlang" OR ${TI_TOOLCHAIN_NAME} STREQUAL "gcc")
            target_sources(ecc_cc23x0 PUBLIC ${INVIA_LIB_INSTALL_DIR}/source/third_party/ecc/src/lowlevelapi_m0_gnu.s)
        elseif (${TI_TOOLCHAIN_NAME} STREQUAL "iar")
            target_sources(ecc_cc23x0 PUBLIC ${INVIA_LIB_INSTALL_DIR}/source/third_party/ecc/src/lowlevelapi_m0_iar.s)
        endif ()
    endif ()

    get_property(libraries DIRECTORY PROPERTY TI_PACKAGE_LIBRARIES)
    foreach (library_name ${libraries})
        target_include_directories(${library_name} PRIVATE "${CMAKE_SOURCE_DIR}/source")
    endforeach ()

    # Manually run commands for ECC header deployment
    add_custom_command(
        OUTPUT
            ${CMAKE_SOURCE_DIR}/source/third_party/ecc/include
            ${CMAKE_SOURCE_DIR}/source/third_party/ecc/include/ECCSW.h
        DEPENDS ${INVIA_LIB_INSTALL_DIR}/source/third_party/ecc/include/ECCSW.h
        COMMENT Copying ECC header files
        COMMAND bash -c "mkdir -p ${CMAKE_SOURCE_DIR}/source/third_party/ecc/include"
        COMMAND bash -c "cp ${INVIA_LIB_INSTALL_DIR}/source/third_party/ecc/include/*.h ${CMAKE_SOURCE_DIR}/source/third_party/ecc/include"
    )

    add_library(ecc_deploy INTERFACE ${CMAKE_SOURCE_DIR}/source/third_party/ecc/include)

    set_target_properties(
        ecc_deploy PROPERTIES
        ADDITIONAL_CLEAN_FILES "${CMAKE_SOURCE_DIR}/source/third_party/ecc/include"
    )
else()
    ti_add_library(ecc_cc13x1_cc26x1 INTERFACE ARCH m4)
    ti_add_library(ecc_cc23x0 INTERFACE ARCH m0p)
    ti_add_library(ecc_cc27xx INTERFACE ARCH m33f)
    ti_add_library(ecc_deploy INTERFACE ARCH all)
endif()
ti_export_package()
