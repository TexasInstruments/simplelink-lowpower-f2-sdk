#-------------------------------------------------------------------------------
# Copyright (c) 2020-2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.15)

include(${CMAKE_CURRENT_LIST_DIR}/tfm_t_cose.cmake)

############################ t_cose secure #####################################

add_library(tfm_t_cose_s STATIC EXCLUDE_FROM_ALL)

target_link_libraries(tfm_t_cose_s
    PUBLIC
        tfm_t_cose_common
        tfm_t_cose_defs
        psa_interface
        qcbor
)

target_compile_options(tfm_t_cose_s
    PUBLIC
        ${COMPILER_CP_FLAG}
)
