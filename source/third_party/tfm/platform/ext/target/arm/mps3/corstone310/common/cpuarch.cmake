#-------------------------------------------------------------------------------
# Copyright (c) 2021-2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# cpuarch.cmake is used to set things that related to the platform that are both
# immutable and global, which is to say they should apply to any kind of project
# that uses this platform. In practise this is normally compiler definitions and
# variables related to hardware.

# Set architecture and CPU
set(TFM_SYSTEM_PROCESSOR cortex-m85)
set(TFM_SYSTEM_ARCHITECTURE armv8.1-m.main)

set(CONFIG_TFM_FP_ARCH "fpv5-d16")
set(CONFIG_TFM_FP_ARCH_ASM "FPv5_D16")
