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

class BaseTransport(object):

    def __init__(self, device_descriptor=None):
        raise NotImplementedError('Implement constructor method in child class')

    def write(self, data):
        raise NotImplementedError('Implement write method in child class')

    def read(self):
        raise NotImplementedError('Implement read method in child class')

    def open(self, device_descriptor):
        raise NotImplementedError('Implement open method in child class')

    def close(self):
        raise NotImplementedError('Implement close method in child class')