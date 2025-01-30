#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# In the new split build this file defines a platform specific parameters
# like mcpu core, arch etc and to be included by NS toolchain file.
# A platform owner is free to configure toolchain here for building NS side.

# Set architecture and CPU
set(TFM_SYSTEM_PROCESSOR cortex-m23)
set(TFM_SYSTEM_ARCHITECTURE armv8-m.base)
