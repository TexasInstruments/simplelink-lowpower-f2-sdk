#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

include(${CORSTONE310_COMMON_DIR}/config.cmake)

set(TEST_NS_FLIH_IRQ        OFF        CACHE BOOL      "Whether to build NS regression First-Level Interrupt Handling tests")
set(PROVISIONING_CODE_PADDED_SIZE       "0x2400"  CACHE STRING    "")
set(PROVISIONING_VALUES_PADDED_SIZE     "0x400"   CACHE STRING    "")
set(PROVISIONING_DATA_PADDED_SIZE       "0x400"   CACHE STRING    "")
