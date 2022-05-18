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
#  Build EmbeddedRPC library for Linux on ARM Cortex-A architecture
#

SDK_INSTALL_DIR = $(abspath ../../../../../../..)
ERPC_ROOT = ../../../..

# target-independent definitions
ERPC_TI = $(SDK_INSTALL_DIR)/source/third_party/erpc/ti
include $(ERPC_TI)/defs.mak

CC = $(GCC_ARMCOMPILER)/usr/bin/arm-linux-gnueabihf-gcc --sysroot=$(GCC_ARMSYSROOT) -c
CC_CMD = $(CC) $(CFLAGS) $< -o $@
AR = $(GCC_ARMCOMPILER)/usr/bin/arm-linux-gnueabihf-ar

# these variables must match profile used in macro call below
debug = -Og -g -gdwarf-3 -gstrict-dwarf -D_DEBUG_=1
release = -O2 -DNDEBUG

CFLAGS = $($(PROFILE)) \
        -Wall -fno-threadsafe-statics \
        -DLINUX \
        -I../../../linux $(INCS-linux) \
        "-I$(GCC_ARMSYSROOT)/usr/include"

ARFLAGS = -rc

$(eval $(call TI_ERPC,linux,debug))
$(eval $(call TI_ERPC,linux,release))
