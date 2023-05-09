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

class SidTransportNotConnected(Exception):
    def __init__(self, message):
        self.message = message


class SidTransportConnected(Exception):
    def __init__(self, message):
        self.message = message


class SidDeviceNotConnected(Exception):
    def __init__(self, message):
        self.message = message


class SidDeviceConnected(Exception):
    def __init__(self, message):
        self.message = message
