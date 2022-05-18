#
#   Copyright (c) 2019-2020 Texas Instruments Incorporated - http://www.ti.com
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions
#   are met:
#
#   *  Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#   *  Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   *  Neither the name of Texas Instruments Incorporated nor the names of
#      its contributors may be used to endorse or promote products derived
#      from this software without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
#   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
#   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
#   EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

#
#  ======== Makefile ========
#  Build EmbeddedRPC library for SYS/BIOS on M4F architecture
#

SDK_INSTALL_DIR = $(abspath ../../../../../../..)
ERPC_ROOT = ../../../..

# target-independent definitions
ERPC_TI = $(SDK_INSTALL_DIR)/source/third_party/erpc/ti
include $(ERPC_TI)/defs.mak

CC = $(CCS_ARMCOMPILER)/bin/armcl -c
CC_CMD = $(CC) $(CFLAGS) --obj_directory=$(@D) $<
AR = $(CCS_ARMCOMPILER)/bin/armar

# these variables must match profile used in macro call below
debug = --symdebug:dwarf -D_DEBUG_=1
release = -O3 -DNDEBUG

CFLAGS = --little_endian --silicon_version=7M4 --abi=eabi \
        --float_support=FPv4SPD16 --code_state=16 --gen_func_subsections=on \
        --display_error_number --diag_wrap=off \
        $($(PROFILE)) \
        -Dxdc_target_name__=M4F \
        -Dxdc_target_types__=ti/targets/arm/elf/std.h \
        -I../../../tirtos $(INCS-tirtos) \
        "-I$(CCS_ARMCOMPILER)/include"

ARFLAGS = -rq

$(eval $(call TI_ERPC,tirtos,debug))
$(eval $(call TI_ERPC,tirtos,release))
