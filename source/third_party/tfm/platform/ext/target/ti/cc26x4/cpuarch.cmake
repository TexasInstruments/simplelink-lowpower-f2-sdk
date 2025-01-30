#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
# Copyright (c) 2023, Texas Instruments Incorporated. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# In the new split build this file defines a platform specific parameters
# like mcpu core, arch etc and to be included by toolchain files.

# Set architecture and CPU
set(TFM_SYSTEM_PROCESSOR cortex-m33)
set(TFM_SYSTEM_ARCHITECTURE armv8-m.main)
set(CONFIG_TFM_FP_ARCH "fpv5-sp-d16")
set(CONFIG_TFM_FP_ARCH_ASM "FPv5-D16")
