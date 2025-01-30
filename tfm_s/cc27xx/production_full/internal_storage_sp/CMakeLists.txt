#-------------------------------------------------------------------------------
# Copyright (c) 2024, Texas Instruments Incorporated. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.15)

# Allow the target_link_libraries() command to be called from any directory to
# add link dependencies and link interface libraries to targets created in other
# directories
cmake_policy(SET CMP0079 NEW)

add_library(ti_tfm_app_rot_partition_internal_storage STATIC)

add_dependencies(ti_tfm_app_rot_partition_internal_storage manifest_tool)

target_sources(ti_tfm_app_rot_partition_internal_storage
    PRIVATE
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/nvs/flash/FlashCC27XX_s.c
        ${TIDRIVERS_INSTALL_DIR}/kernel/tfm/dpl/HwiP_tfm.c
)

# The generated sources
target_sources(ti_tfm_app_rot_partition_internal_storage
    PRIVATE
        ${CMAKE_BINARY_DIR}/generated/secure_fw/partitions/ti_internal_storage/auto_generated/intermedia_internal_storage_sp.c
)
target_sources(tfm_partitions
    INTERFACE
        ${CMAKE_BINARY_DIR}/generated/secure_fw/partitions/ti_internal_storage/auto_generated/load_info_internal_storage_sp.c
)

# Set include directory
target_include_directories(ti_tfm_app_rot_partition_internal_storage
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        ${CMAKE_BINARY_DIR}/generated/secure_fw/partitions/ti_internal_storage
)
target_include_directories(tfm_partitions
    INTERFACE
        ${CMAKE_BINARY_DIR}/generated/secure_fw/partitions/ti_internal_storage
)

target_compile_options(ti_tfm_app_rot_partition_internal_storage
    PUBLIC
        $<$<C_COMPILER_ID:GNU>:-Wno-unused-parameter>
        $<$<C_COMPILER_ID:ARMClang>:-Wno-unused-parameter>
)

target_link_libraries(ti_tfm_app_rot_partition_internal_storage
    PRIVATE
        platform_s
        tfm_config
        tfm_sprt
        $<$<C_COMPILER_ID:ARMClang>:${DRIVERLIB_SOURCE_DIR}/ti/devices/cc27xx/driverlib/lib/ticlang/driverlib.a>
        $<$<C_COMPILER_ID:GNU>:${DRIVERLIB_SOURCE_DIR}/ti/devices/cc27xx/driverlib/lib/gcc/driverlib.a>
        $<$<C_COMPILER_ID:IAR>:${DRIVERLIB_SOURCE_DIR}/ti/devices/cc27xx/driverlib/lib/iar/driverlib.a>
)

############################ Partition Defs ####################################

target_link_libraries(tfm_partitions
    INTERFACE
        ti_tfm_app_rot_partition_internal_storage
)
