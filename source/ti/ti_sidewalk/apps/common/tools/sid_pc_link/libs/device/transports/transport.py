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

from .transport_config import *

if SELECT_TRANSPORT == TRANSPORT_TYPE.BLE:
    if TRANSPORT_LIBRARY == PLATFORM_TYPE.LINUX:
        from .ble.linux.transport import BluetoothTransport as Transport

    elif TRANSPORT_LIBRARY == PLATFORM_TYPE.MAC_OS or TRANSPORT_LIBRARY == PLATFORM_TYPE.WINDOWS:
        from .ble.macos.transport import BluetoothTransport as Transport

    else:
        raise ImportError('Transport library for platform {} is not defined'.format(TRANSPORT_LIBRARY))

else:
    raise ImportError('Transport {} is not defined'.format(SELECT_TRANSPORT))


class DeviceTransport(Transport):
    pass
