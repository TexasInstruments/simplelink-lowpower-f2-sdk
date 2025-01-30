#-------------------------------------------------------------------------------
# Copyright (c) 2024, Texas Instruments Incorporated
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.15)

# Allow the target_link_libraries() command to be called from any directory to
# add link dependencies and link interface libraries to targets created in other
# directories
cmake_policy(SET CMP0079 NEW)

target_sources(tfm_s
    PRIVATE
#        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/
)

target_link_libraries(tfm_s
    PRIVATE
        $<$<C_COMPILER_ID:ARMClang>:${DRIVERLIB_SOURCE_DIR}/ti/devices/cc27xx/driverlib/lib/ticlang/driverlib.a>
        $<$<C_COMPILER_ID:GNU>:${DRIVERLIB_SOURCE_DIR}/ti/devices/cc27xx/driverlib/lib/gcc/driverlib.a>
        $<$<C_COMPILER_ID:IAR>:${DRIVERLIB_SOURCE_DIR}/ti/devices/cc27xx/driverlib/lib/iar/driverlib.a>
)
