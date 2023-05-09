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

from apps.base_app import *
import logging
import coloredlogs

# Add needed libraries
add_lib('protocol')
add_lib('device')
add_lib('cloud')

# Load all added libraries
load_dependencies()

logging.basicConfig(format="%(asctime)s %(name)-12s %(levelname)-8s [%(filename)s:%(lineno)d] %(message)s")
coloredlogs.install(level=logging.DEBUG)
