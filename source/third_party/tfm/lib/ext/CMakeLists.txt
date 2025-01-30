#-------------------------------------------------------------------------------
# Copyright (c) 2020-2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/lib/ext CACHE STRING "" FORCE)

add_subdirectory(qcbor)
add_subdirectory(t_cose)
add_subdirectory(mbedcrypto)
if(BL2)
    add_subdirectory(mcuboot)
endif()
if (${PLATFORM_PSA_ADAC_SECURE_DEBUG})
    add_subdirectory(psa-adac)
endif()
if(DEFINED ETHOSU_ARCH)
    add_subdirectory(ethos_u_core_driver)
endif()

add_subdirectory(tf-m-extras)
