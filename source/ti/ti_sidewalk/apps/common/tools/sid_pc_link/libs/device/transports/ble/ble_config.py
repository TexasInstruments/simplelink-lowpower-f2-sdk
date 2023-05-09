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

# PYTHON VARIABLE                       | ENVIRONMENT VARIABLE         | DEFAULT VALUE
# --------------------------------------------------------------------------------------------------------------------
BLUETOOTH_ADAPTER            = os.getenv("BLUETOOTH_ADAPTER",           'hci0')
# --------------------------------------------------------------------------------------------------------------------
BLUETOOTH_SCAN_TIMEOUT       = int(os.getenv("BLUETOOTH_SCAN_TIMEOUT",      15))
# --------------------------------------------------------------------------------------------------------------------
