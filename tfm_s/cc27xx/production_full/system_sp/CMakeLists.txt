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

add_library(ti_tfm_app_rot_partition_system STATIC)

add_dependencies(ti_tfm_app_rot_partition_system manifest_tool)

target_sources(ti_tfm_app_rot_partition_system
    PRIVATE
    #### TODO: Add system drivers here ####
#        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/system/SystemCC27XX_s.c
#        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/system/PowerCC27XX_s.c
        ${CMAKE_CURRENT_LIST_DIR}/dummy.c   # Remove dummy file this when System_sp_main() is implemented in SystemCC27XX_s.c
        ${TIDRIVERS_INSTALL_DIR}/kernel/tfm/dpl/HwiP_tfm.c
)

# The generated sources
target_sources(ti_tfm_app_rot_partition_system
    PRIVATE
        ${CMAKE_BINARY_DIR}/generated/secure_fw/partitions/ti_system/auto_generated/intermedia_system_sp.c
)
target_sources(tfm_partitions
    INTERFACE
        ${CMAKE_BINARY_DIR}/generated/secure_fw/partitions/ti_system/auto_generated/load_info_system_sp.c
)

# Set include directory
target_include_directories(ti_tfm_app_rot_partition_system
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        ${CMAKE_BINARY_DIR}/generated/secure_fw/partitions/ti_system
)
target_include_directories(tfm_partitions
    INTERFACE
        ${CMAKE_BINARY_DIR}/generated/secure_fw/partitions/ti_system
)

target_compile_options(ti_tfm_app_rot_partition_system
    PUBLIC
        $<$<C_COMPILER_ID:GNU>:-Wno-unused-parameter>
        $<$<C_COMPILER_ID:ARMClang>:-Wno-unused-parameter>
)

target_link_libraries(ti_tfm_app_rot_partition_system
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
        ti_tfm_app_rot_partition_system
)
