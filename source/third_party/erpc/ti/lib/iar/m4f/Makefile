#
#   Copyright (c) 2020 Texas Instruments Incorporated - http://www.ti.com
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

CC = $(IAR_ARMCOMPILER)/bin/iccarm
CC_CMD = $(CC) $(CFLAGS) $< -o $@
AR = $(IAR_ARMCOMPILER)/bin/iarchive

# these variables must match profile used in macro call below
debug = --debug -D_DEBUG_=1
release = -Ohs -DNDEBUG

CFLAGS = --c++ --cpu=Cortex-M4F --endian=little --thumb -e \
        --fpu=VFPv4_sp --thumb --aeabi --guard_calls \
        --silent --dlib_config $(IAR_ARMCOMPILER)/inc/c/DLib_Config_Normal.h \
        --diag_suppress=Pa082 \
        $($(PROFILE)) \
        -Dxdc_target_name__=M4F \
        -Dxdc_target_types__=iar/targets/arm/std.h \
        -I$(SDK_INSTALL_DIR)/kernel/tirtos/packages/ti/posix/iar \
        -I../../../tirtos $(INCS-tirtos)

ARFLAGS = --create --diag_suppress=La017

$(eval $(call TI_ERPC,tirtos,debug))
$(eval $(call TI_ERPC,tirtos,release))
