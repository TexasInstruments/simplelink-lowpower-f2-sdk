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
get_install_dir(MBEDCRYPTO_INSTALL_DIR)
get_install_dir(TFM_INSTALL_DIR)

include(${CMAKE_CURRENT_LIST_DIR}/CMakeSources_common.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/CMakeSources_cc13xx_cc26xx.cmake OPTIONAL)
include(${CMAKE_CURRENT_LIST_DIR}/CMakeSources_cc23xx.cmake OPTIONAL)
include(${CMAKE_CURRENT_LIST_DIR}/CMakeSources_cc32xx.cmake OPTIONAL)

project(drivers LANGUAGES C ASM)
ti_init_package(NAMESPACE Drivers DEPENDS_ON DriversUtils TrustZone ThirdPartyEccLib Tirtos7Generic)
foreach (TI_PLATFORM ${SUPPORTED_PLATFORMS})
    if (${TI_PLATFORM} MATCHES "cc(13|26).1")
        ti_add_package_dependency(DriverlibCC13XXCC26XX)
        ti_add_library(
            drivers_${TI_PLATFORM}
            SOURCES ${SOURCES_CC13X1_CC26X1}
            ADD_SDK_INCLUDE_PATH source
            LINK_PUBLIC Driverlib::${TI_PLATFORM} CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}}
            # This is only needed for PDMCC26XX to find xdc/std.h
            LINK_PRIVATE Tirtos7Generic::tirtos7
        )

    elseif (${TI_PLATFORM} MATCHES "cc(13|26).2")
        ti_add_package_dependency(DriverlibCC13XXCC26XX)
        ti_add_library(
            drivers_${TI_PLATFORM}
            SOURCES ${SOURCES_CC13X2_CC26X2}
            ADD_SDK_INCLUDE_PATH source
            LINK_PUBLIC Driverlib::${TI_PLATFORM} CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}}
            # This is only needed for PDMCC26XX to find xdc/std.h
            LINK_PRIVATE Tirtos7Generic::tirtos7
        )

        # C26X3 is deliberately excluded from this regex, as the 26x4 libraries are good enough
    elseif (${TI_PLATFORM} MATCHES "cc(13|26).4")
        ti_add_package_dependency(DriverlibCC13XXCC26XX)
        ti_add_library(
            drivers_${TI_PLATFORM}
            SOURCES ${SOURCES_SEC_ONLY_CC13X4_CC26X3_CC26X4}
            ADD_SDK_INCLUDE_PATH source
            LINK_PUBLIC Driverlib::${TI_PLATFORM} CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}}
            # This is only needed for PDMCC26XX to find xdc/std.h
            LINK_PRIVATE Tirtos7Generic::tirtos7
        )

        ti_add_library(
            drivers_${TI_PLATFORM}_ns SOURCES ${SOURCES_NONSEC_CC13X4_CC26X3_CC26X4} ADD_SDK_INCLUDE_PATH source
            LINK_PUBLIC Driverlib::${TI_PLATFORM} DriversUtils::tfm_enabled TrustZone::tfm_dependencies_cc26x4
                        CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}} LINK_PRIVATE Tirtos7Generic::tirtos7
        )

    elseif (${TI_PLATFORM} STREQUAL "cc23x0")
        ti_add_package_dependency(DriverlibCC23X0)
        ti_add_library(
            drivers_${TI_PLATFORM} SOURCES ${SOURCES_CC23X0} ADD_SDK_INCLUDE_PATH source
            LINK_PUBLIC ThirdPartyEccLib::ecc_cc23xx Driverlib::${TI_PLATFORM}
                        CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}}
        )
        add_dependencies(drivers_${TI_PLATFORM} ecc_deploy)

    elseif (${TI_PLATFORM} STREQUAL "cc23x0r2")
        ti_add_package_dependency(DriverlibCC23X0R2)
        ti_add_library(
            drivers_${TI_PLATFORM} SOURCES ${SOURCES_CC23X0R2} ADD_SDK_INCLUDE_PATH source
            LINK_PUBLIC ThirdPartyEccLib::ecc_cc23xx Driverlib::${TI_PLATFORM}
                        CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}}
        )
        add_dependencies(drivers_${TI_PLATFORM} ecc_deploy)

    elseif (${TI_PLATFORM} MATCHES "cc32")
        ti_add_package_dependency(DriverlibCC32XX)
        ti_add_library(
            drivers_${TI_PLATFORM} SOURCES ${SOURCES_CC32XX} ADD_SDK_INCLUDE_PATH source
            LINK_PUBLIC Driverlib::${TI_PLATFORM} CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}}
        )
    endif ()
endforeach ()
ti_export_package()

#
# A couple of simple utilities used by the drivers to portably export compile options
#
ti_init_package(NAMESPACE DriversUtils)

ti_add_library(secure_key_storage_enabled INTERFACE ARCH all)
target_compile_definitions(secure_key_storage_enabled INTERFACE $<$<COMPILE_LANGUAGE:C>:ENABLE_KEY_STORAGE=1>)

ti_add_library(tfm_enabled INTERFACE ARCH all)
target_compile_definitions(
    tfm_enabled INTERFACE $<$<COMPILE_LANGUAGE:C>:TFM_ENABLED=1> $<$<COMPILE_LANGUAGE:C>:ITS_CREATE_FLASH_LAYOUT=1>
                          $<$<COMPILE_LANGUAGE:C>:ENABLE_KEY_STORAGE=1>
)

ti_export_package()

#
# Targets for TrustZone support
#
ti_init_package(NAMESPACE TrustZone)
ti_add_library(tfm_dependencies_cc26x4 INTERFACE ARCH all)

# This must appear ahead of the TFM includes to find the correct sid.h
target_include_directories(tfm_dependencies_cc26x4 BEFORE INTERFACE ${CMAKE_SOURCE_DIR}/tfm_s/cc26x4/build_dependencies)

if (TI_INTERNAL_BUILD)
    target_include_directories(
        tfm_dependencies_cc26x4
        INTERFACE ${TFM_INSTALL_DIR}/source/ ${TFM_INSTALL_DIR}/source/third_party/tfm/interface/include
                  ${MBEDCRYPTO_INSTALL_DIR}/source ${MBEDCRYPTO_INSTALL_DIR}/source/third_party/mbedcrypto/include
                  ${MBEDCRYPTO_INSTALL_DIR}/source/third_party/mbedcrypto/ti
    )
else ()
    target_include_directories(
        tfm_dependencies_cc26x4
        INTERFACE ${CMAKE_SOURCE_DIR}/source/ ${CMAKE_SOURCE_DIR}/source/third_party/tfm/interface/include
                  ${CMAKE_SOURCE_DIR}/source/third_party/mbedcrypto/include
                  ${CMAKE_SOURCE_DIR}/source/third_party/mbedcrypto/ti
    )
endif ()

ti_export_package()
