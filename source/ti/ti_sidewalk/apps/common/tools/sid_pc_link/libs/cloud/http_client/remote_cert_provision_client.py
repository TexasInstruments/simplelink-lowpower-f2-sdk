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

from .http_config import *
from .base_client import HttpAbstractClient
import logging

logging.basicConfig(level=logging.DEBUG)


class RemoteCertProvisionClient(HttpAbstractClient):

    def set_connection_url(self):
        return REGISTRATION_ENVIRONMENT_URL

    def set_auth(self):
        # No initial authentication is needed
        # Every request carries an Auth token and a Session ID
        pass

    def validate_connection(self):
        # The connection can be validated after the
        # Auth token and Session ID have been generated
        pass

    def post_authenticate_remote_device(self,
                                        data,
                                        gateway_id=None,
                                        auth=None,
                                        lwa_token=None):
        header = {
            'Content-Type': 'application/json'
        }
        if gateway_id is not None:
            header['X-Amz-Halo-Gateway-Id'] = gateway_id
        if auth is not None:
            header['Authorization'] = auth
        if lwa_token is not None:
            header['X-Amz-Access-Token'] = lwa_token

        return self.post('/remoteProvisioning/AuthenticateRemoteDevice',
                         json=data,
                         headers=header)


