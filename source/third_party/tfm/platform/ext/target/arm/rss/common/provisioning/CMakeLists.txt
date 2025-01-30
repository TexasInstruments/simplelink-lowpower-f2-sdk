#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

add_subdirectory(bundle_cm)
add_subdirectory(bundle_dm)

target_sources(platform_s
    PRIVATE
        ./runtime_stub_provisioning.c
)

target_sources(platform_bl2
    PRIVATE
        ./bl2_stub_provisioning.c
)

target_sources(bl1_1_lib
    INTERFACE
        ./bl1_provisioning.c
)

target_include_directories(bl1_1_lib
    INTERFACE
        ./bundle_common
)
