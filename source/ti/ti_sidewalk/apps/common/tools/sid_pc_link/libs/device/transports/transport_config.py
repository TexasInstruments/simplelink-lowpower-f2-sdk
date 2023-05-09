#
# Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
#
# AMAZON PROPRIETARY/CONFIDENTIAL
#
# You may not use this file except in compliance with the terms and conditions
# set forth in the accompanying LICENSE.txt file.
#
# THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
# DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
#

import os
import platform


class TRANSPORT_TYPE:
    BLE = 'BLE'


class PLATFORM_TYPE:
    MAC_OS = 'Darwin'
    LINUX = 'Linux'
    WINDOWS = 'Windows'


# PYTHON VARIABLE                       | ENVIRONMENT VARIABLE         | DEFAULT VALUE
# --------------------------------------------------------------------------------------------------------------------
SELECT_TRANSPORT             = os.getenv("SELECT_TRANSPORT",            TRANSPORT_TYPE.BLE)
# --------------------------------------------------------------------------------------------------------------------
TRANSPORT_LIBRARY            = os.getenv("BLUETOOTH_LIBRARY",           platform.system())
# --------------------------------------------------------------------------------------------------------------------
