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
include(${CMAKE_CURRENT_LIST_DIR}/CMakeSources.cmake)

get_install_dir(MBEDTLS_INSTALL_DIR)

project(psa_crypto LANGUAGES C ASM)
ti_init_package(NAMESPACE PSACrypto DEPENDS_ON Drivers DriversUtils)

# Pull "all supported architectures" from cmake-common
foreach(TI_PLATFORM ${SUPPORTED_PLATFORMS})
    if (${TI_PLATFORM} MATCHES "cc(13|26).4")
        ti_add_library(
            psa_crypto_${TI_PLATFORM}_ns
            SOURCES ${SOURCES}
            LINK_PUBLIC Drivers::drivers_${TI_PLATFORM} DriversUtils::tfm_enabled CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}}
        )
        ti_export_sdk_include(psa_crypto_${TI_PLATFORM}_ns PUBLIC source/third_party/psa_crypto/library)
        ti_export_sdk_include(psa_crypto_${TI_PLATFORM}_ns PUBLIC source/third_party/psa_crypto/include/psa)
        ti_export_sdk_include(psa_crypto_${TI_PLATFORM}_ns PUBLIC source/third_party/psa_crypto/include)

        target_include_directories(psa_crypto_${TI_PLATFORM}_ns PRIVATE ${MBEDTLS_INSTALL_DIR}/source/third_party/mbedtls/ti)
        target_include_directories(psa_crypto_${TI_PLATFORM}_ns PRIVATE ${MBEDTLS_INSTALL_DIR}/source/third_party/mbedtls/include)

    endif ()
endforeach()

ti_export_package()
