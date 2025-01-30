# Copyright (c) 2020, Linaro. All rights reserved.
# Copyright (c) 2020, Arm Limited. All rights reserved.
# Copyright 2020-2023 NXP. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause

import os
import platform

# Move to the TF-M root directory
os.chdir('../../../../../../')

# Remove previous build folder
if os.path.isdir("build"):
    if platform.system() == 'Windows':
        os.system('rd /s /q build')
    else:
        os.system('rm -rf build')

# Generate the S and NS makefiles
os.system('cmake -S . -B build -DTFM_PLATFORM=nxp/lpcxpresso55s69 -DTFM_TOOLCHAIN_FILE=toolchain_GNUARM.cmake -G"Unix Makefiles" \
           -DTFM_PROFILE=profile_medium \
           -DTFM_SPM_LOG_LEVEL=TFM_SPM_LOG_LEVEL_INFO -DTFM_PARTITION_LOG_LEVEL=TFM_PARTITION_LOG_LEVEL_INFO \
           -DBL2=OFF \
           -DTEST_S=ON \
           -DTEST_NS=ON ')

# Build the binaries
os.chdir('build')
os.system('make install')