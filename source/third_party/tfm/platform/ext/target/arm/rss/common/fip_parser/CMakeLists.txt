#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#========================= Platform BL2 =======================================#

target_sources(platform_bl2
    PRIVATE
        fip_parser.c
        host_flash_atu.c
        $<$<BOOL:${RSS_GPT_SUPPORT}>:${CMAKE_CURRENT_SOURCE_DIR}/gpt.c>
)

target_include_directories(platform_bl2
    PUBLIC
        .
)

#========================= Platform BL1 =======================================#

target_sources(platform_bl1_1
    PRIVATE
        fip_parser.c
        host_flash_atu.c
        $<$<BOOL:${RSS_GPT_SUPPORT}>:${CMAKE_CURRENT_SOURCE_DIR}/gpt.c>
)

target_include_directories(platform_bl1_1_interface
    INTERFACE
        .
)
