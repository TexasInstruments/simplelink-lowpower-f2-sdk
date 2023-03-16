#-------------------------------------------------------------------------------
# Copyright (c) 2019 Texas Instruments Incorporated
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

include(Common/CompilerTIClangArmCommon)
include(Common/Utils)

if (NOT DEFINED ARM_CPU_TYPE)
    set(_NO_ARM_CPU_TYPE true)
elseif (${ARM_CPU_TYPE} STREQUAL "Cortex-M33F")
    string_append_unique_item(STRING CMAKE_C_FLAGS_CPU KEY "-mcpu=" VAL "-mcpu=cortex-m33")
    string_append_unique_item(STRING CMAKE_C_FLAGS_CPU KEY "-mfloat-abi=" VAL "-mfloat-abi=hard")
    string_append_unique_item(STRING CMAKE_C_FLAGS_CPU KEY "-mfpu=" VAL "-mfpu=fpv5-sp-d16")
    string_append_unique_item(STRING CMAKE_CXX_FLAGS_CPU KEY "-mcpu=" VAL "-mcpu=cortex-m33")
    string_append_unique_item(STRING CMAKE_CXX_FLAGS_CPU KEY "-mfloat-abi=" VAL "-mfloat-abi=hard")
    string_append_unique_item(STRING CMAKE_CXX_FLAGS_CPU KEY "-mfpu=" VAL "-mfpu=fpv5-sp-d16")
    string_append_unique_item(STRING CMAKE_ASM_FLAGS_CPU KEY "-mcpu=" VAL "-mcpu=cortex-m33")
    string_append_unique_item(STRING CMAKE_ASM_FLAGS_CPU KEY "-mfloat-abi=" VAL "-mfloat-abi=hard")
    string_append_unique_item(STRING CMAKE_ASM_FLAGS_CPU KEY "-mfpu=" VAL "-mfpu=fpv5-sp-d16")
    string_append_unique_item(STRING CMAKE_LINK_FLAGS_CPU KEY "-mcpu=" VAL "-mcpu=cortex-m33")
    string_append_unique_item(STRING CMAKE_LINK_FLAGS_CPU KEY "-mfloat-abi=" VAL "-mfloat-abi=hard")
    string_append_unique_item(STRING CMAKE_LINK_FLAGS_CPU KEY "-mfpu=" VAL "-mfpu=fpv5-sp-d16")
else ()
    message(FATAL_ERROR "Unknown ARM cpu setting.")
endif ()

if (_NO_ARM_CPU_TYPE)
    message(FATAL_ERROR "can not set cpu specific compiler flags")
endif ()
