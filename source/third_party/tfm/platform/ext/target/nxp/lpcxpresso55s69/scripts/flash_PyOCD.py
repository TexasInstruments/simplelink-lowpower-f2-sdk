# Copyright 2020-2023 NXP. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause

import os

# Move to the TFM build folder
os.chdir('../../../../../../build/bin')

target = 'LPC55S69'

# Flash with PyOCD
os.system('pyocd erase --mass -t ' + target)                   # erase the flash memory
os.system('pyocd flash tfm_s.hex tfm_ns.hex -t ' + target)     # flash images into the target
