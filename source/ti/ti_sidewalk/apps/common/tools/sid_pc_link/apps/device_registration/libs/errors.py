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

class AuthenticationError(Exception):
    pass


class GetBleCapabilityError(Exception):
    pass


class StartBleRegistrationError(Exception):
    pass


class PlatformKeyExchangeError(Exception):
    pass


class HaloCredentialsError(Exception):
    pass


class RegisterEdgeError(Exception):
    pass


class RegistrationCloudError(Exception):
    pass


class PerformHandshakeError(Exception):
    pass


class BleCommandNotFoundError(Exception):
    pass


class AuthorizationCodeGrantError(Exception):
    pass


class RefreshTokenError(Exception):
    pass


class DeviceNotFoundError(Exception):
    pass


class DeviceStateError(Exception):
    pass

class AmaConnectFailedError(Exception):
    pass
