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


class RegistrationClient(HttpAbstractClient):

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

    def post_get_ble_capability(self,
                                endpoint_id,
                                asl_format=False,
                                asl_version=None,
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

        # Initialize default data
        data = {
            'endpointId': endpoint_id
        }
        # Check ASL format is enabled and ASL version is provided
        if asl_format and asl_version is not None:
            data['dataFormat'] = {
                'type': 'ASL',
                'version': asl_version
            }

        response = self.post('/GetBleCapability',
                             json=data,
                             headers=header,
                             timeout=REGISTRATION_REQUEST_TIMEOUT)
        return response

    def post_start_ble_registration(self,
                                    endpoint_id,
                                    prov_init_req,
                                    prov_init_sig,
                                    asl_format=False,
                                    asl_version=None,
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

        # Initialize default data
        data = {
            'endpointId': endpoint_id,
            'haloCommands': [prov_init_req, prov_init_sig]
        }
        # Check ASL format is enabled and ASL version is provided
        if asl_format and asl_version is not None:
            data['dataFormat'] = {
                'type': 'ASL',
                'version': asl_version
            }

        response = self.post('/StartBleRegistration',
                             json=data,
                             headers=header,
                             timeout=REGISTRATION_REQUEST_TIMEOUT)
        return response

    def post_channel_platform_key_exchange(self,
                                           session_id,
                                           endpoint_id,
                                           prov_init_sig_cmd,
                                           device_ecdh_cmd,
                                           resp_prov_init_sig,
                                           resp_device_ecdh_sig,
                                           gateway_id=None,
                                           auth=None,
                                           lwa_token=None):
        header = {
            'Content-Type': 'application/json',
            'X-Amz-Halo-Provisioning-Session-Id': session_id
        }
        if gateway_id is not None:
            header['X-Amz-Halo-Gateway-Id'] = gateway_id
        if auth is not None:
            header['Authorization'] = auth
        if lwa_token is not None:
            header['X-Amz-Access-Token'] = lwa_token

        data = {
            'endpointId': endpoint_id,
            'haloCommandMap': {prov_init_sig_cmd: resp_prov_init_sig,
                               device_ecdh_cmd: resp_device_ecdh_sig}
        }
        response = self.post('/ChannelSetup/PerformKeyExchange',
                             json=data,
                             headers=header,
                             timeout=REGISTRATION_REQUEST_TIMEOUT)
        return response

    def post_channel_perform_handshake(self,
                                       session_id,
                                       endpoint_id,
                                       handshake_cmd,
                                       resp_handshake,
                                       gateway_id=None,
                                       auth=None,
                                       lwa_token=None):
        header = {
            'Content-Type': 'application/json',
            'X-Amz-Halo-Provisioning-Session-Id': session_id
        }
        if gateway_id is not None:
            header['X-Amz-Halo-Gateway-Id'] = gateway_id
        if auth is not None:
            header['Authorization'] = auth
        if lwa_token is not None:
            header['X-Amz-Access-Token'] = lwa_token

        data = {
            'endpointId': endpoint_id,
            'haloCommandMap': {handshake_cmd: resp_handshake}
        }
        response = self.post('/ChannelSetup/PerformHandshake',
                             json=data,
                             headers=header,
                             timeout=REGISTRATION_REQUEST_TIMEOUT)
        return response

    def post_halo_credentials(self,
                              session_id,
                              endpoint_id,
                              gateway_id=None,
                              auth=None,
                              lwa_token=None):
        header = {
            'Content-Type': 'application/json',
            'X-Amz-Halo-Provisioning-Session-Id': session_id
        }
        if gateway_id is not None:
            header['X-Amz-Halo-Gateway-Id'] = gateway_id
        if auth is not None:
            header['Authorization'] = auth
        if lwa_token is not None:
            header['X-Amz-Access-Token'] = lwa_token

        data = {
            'endpointId': endpoint_id,
        }
        response = self.post('/HaloCredentials',
                             json=data,
                             headers=header,
                             timeout=REGISTRATION_REQUEST_TIMEOUT)
        return response

    def post_register_edge(self,
                           session_id,
                           endpoint_id,
                           pan_id_cmd=None,
                           p2p_channel_cmd=None,
                           p2p_broadcast_key_cmd=None,
                           wan_sec_params_cmd=None,
                           resp_pan_id=None,
                           resp_p2p_channel=None,
                           resp_p2p_broadcast_key=None,
                           resp_set_wan_sec_params=None,
                           halo_command_map=None,
                           gateway_id=None,
                           auth=None,
                           lwa_token=None):
        header = {
            'Content-Type': 'application/json',
            'X-Amz-Halo-Provisioning-Session-Id': session_id
        }
        if gateway_id is not None:
            header['X-Amz-Halo-Gateway-Id'] = gateway_id
        if auth is not None:
            header['Authorization'] = auth
        if lwa_token is not None:
            header['X-Amz-Access-Token'] = lwa_token

        data = {'endpointId': endpoint_id,
                'haloCommandMap': {}}
        if halo_command_map is not None:
            data['haloCommandMap'].update(halo_command_map)
        else:
            data['haloCommandMap'][pan_id_cmd] = resp_pan_id
            data['haloCommandMap'][p2p_channel_cmd] = resp_p2p_channel
            data['haloCommandMap'][p2p_broadcast_key_cmd] = resp_p2p_broadcast_key
            data['haloCommandMap'][wan_sec_params_cmd] = resp_set_wan_sec_params

        response = self.post('/RegisterEdge',
                             json=data,
                             headers=header,
                             timeout=REGISTRATION_REQUEST_TIMEOUT)
        return response

    def post_application_perform_key_exchange(self,
                                              session_id,
                                              endpoint_id,
                                              prov_init_sig_cmd=None,
                                              get_device_ecdh_sig_cmd=None,
                                              resp_prov_init_sig=None,
                                              resp_get_device_ecdh_sig=None,
                                              halo_command_map=None,
                                              gateway_id=None,
                                              auth=None,
                                              lwa_token=None):
        header = {
            'Content-Type': 'application/json',
            'X-Amz-Halo-Provisioning-Session-Id': session_id
        }
        if gateway_id is not None:
            header['X-Amz-Halo-Gateway-Id'] = gateway_id
        if auth is not None:
            header['Authorization'] = auth
        if lwa_token is not None:
            header['X-Amz-Access-Token'] = lwa_token

        data = {'endpointId': endpoint_id,
                'haloCommandMap': {}}
        if halo_command_map is not None:
            data['haloCommandMap'].update(halo_command_map)
        else:
            data['haloCommandMap'][prov_init_sig_cmd] = resp_prov_init_sig
            data['haloCommandMap'][get_device_ecdh_sig_cmd] = resp_get_device_ecdh_sig

        response = self.post('/ApplicationSetup/PerformKeyExchange',
                             json=data,
                             headers=header,
                             timeout=REGISTRATION_REQUEST_TIMEOUT)
        return response

    def post_application_perform_handshake(self,
                                           session_id,
                                           endpoint_id,
                                           handshake_cmd=None,
                                           resp_handshake=None,
                                           halo_command_map=None,
                                           gateway_id=None,
                                           auth=None,
                                           lwa_token=None):
        header = {
            'Content-Type': 'application/json',
            'X-Amz-Halo-Provisioning-Session-Id': session_id
        }
        if gateway_id is not None:
            header['X-Amz-Halo-Gateway-Id'] = gateway_id
        if auth is not None:
            header['Authorization'] = auth
        if lwa_token is not None:
            header['X-Amz-Access-Token'] = lwa_token

        data = {'endpointId': endpoint_id,
                'haloCommandMap': {}}
        if halo_command_map is not None:
            data['haloCommandMap'].update(halo_command_map)
        else:
            data['haloCommandMap'][handshake_cmd] = resp_handshake

        response = self.post('/ApplicationSetup/PerformHandshake',
                             json=data,
                             headers=header,
                             timeout=REGISTRATION_REQUEST_TIMEOUT)
        return response

    def post_application_register_edge(self,
                                       session_id,
                                       endpoint_id,
                                       set_config_complete_cmd=None,
                                       resp_set_config_complete=None,
                                       halo_command_map=None,
                                       gateway_id=None,
                                       auth=None,
                                       lwa_token=None):
        header = {
            'Content-Type': 'application/json',
            'X-Amz-Halo-Provisioning-Session-Id': session_id
        }
        if gateway_id is not None:
            header['X-Amz-Halo-Gateway-Id'] = gateway_id
        if auth is not None:
            header['Authorization'] = auth
        if lwa_token is not None:
            header['X-Amz-Access-Token'] = lwa_token

        data = {'endpointId': endpoint_id,
                'haloCommandMap': {}}
        if halo_command_map is not None:
            data['haloCommandMap'].update(halo_command_map)
        else:
            data['haloCommandMap'][set_config_complete_cmd] = resp_set_config_complete

        response = self.post('/ApplicationSetup/RegisterEdge',
                             json=data,
                             headers=header,
                             timeout=REGISTRATION_REQUEST_TIMEOUT)
        return response
