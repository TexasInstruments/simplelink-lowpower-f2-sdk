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

class CloudEnvironment:
    BETA = 'beta'
    GAMMA = 'gamma'
    PROD = 'prod'

# PYTHON VARIABLE                       | ENVIRONMENT VARIABLE         | DEFAULT VALUE
# --------------------------------------------------------------------------------------------------------------------
REGISTRATION_ENVIRONMENT     = os.getenv("REGISTRATION_ENVIRONMENT",    CloudEnvironment.BETA)
# --------------------------------------------------------------------------------------------------------------------
REGISTRATION_ENVIRONMENT_URL = os.getenv("REGISTRATION_ENVIRONMENT_URL",{CloudEnvironment.BETA: 'https://ringffsproxy-beta.amazon.com/public/v1',
                                                                         CloudEnvironment.GAMMA: 'https://ringffsproxy-gamma.amazon.com/public/v1',
                                                                         CloudEnvironment.PROD: 'https://ringffsproxy.amazon.com/public/v1'}
                                                                            .get(REGISTRATION_ENVIRONMENT, 'https://ringffsproxy.amazon.com/public/v1'))
# --------------------------------------------------------------------------------------------------------------------
REGISTRATION_REQUEST_TIMEOUT = int(os.getenv("REGISTRATION_REQUEST_TIMEOUT",10))
# --------------------------------------------------------------------------------------------------------------------
