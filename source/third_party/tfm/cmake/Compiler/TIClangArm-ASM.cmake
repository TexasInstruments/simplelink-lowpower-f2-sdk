#-------------------------------------------------------------------------------
# Copyright (c) 2019 Texas Instruments Incorporated
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

include(Compiler/TIClangArm)
set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS s;S;asm)
__compiler_armclang(ASM)
