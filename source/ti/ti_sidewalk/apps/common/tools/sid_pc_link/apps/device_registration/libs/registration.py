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

from apps.device_registration.libs.sid_ble import write, read_until, read
from apps.device_registration.libs.errors import *
from sidewalk_builder.halo_collection import CommandClass, CommandId
from sidewalk_builder.ble_beacon.decoder import BleBeaconDecoder
from sidewalk_builder.ble_beacon.collection import application_id

import logging
import os

logger = logging.getLogger(__name__)


class Command(object):

    def __init__(self, command=None, response=None):
        self.command = command
        self.response = response


class DeviceRegister(object):

    def __init__(self, device, client):
        self.device = device
        self.client = client
        self.sidewalk_endpoint_id = None
        self.endpoint_id = None
        self.is_smsn = False
        self.asl_format = False
        self.asl_version = None
        self.__session_id = None
        self.__validate_init_environment()

    def __validate_init_environment(self):
        lwa_token = os.getenv('LWA_TOKEN')
        auth_token = os.getenv('AUTH_TOKEN')
        gateway_id = os.getenv('GATEWAY_ID')
        self.endpoint_id = os.getenv('ENDPOINT_ID')
        manufacturer_data = BleBeaconDecoder(self.device.manufacturer_data)
        if manufacturer_data.application_id != application_id[0x4]:
            raise DeviceStateError('Device is already registered. Device needs a factory reset.')
        self.asl_format = bool(manufacturer_data
                               .device_state
                               .get('advertisement_version_format'))
        if self.asl_format:
            self.asl_version = int(manufacturer_data
                                   .application_data
                                   .get('frame_indicator')
                                   .get('asl_version'))
        if lwa_token is not None:
            logger.info('LWA Token was provided')
        else:
            logger.info('LWA Token was not provided. Checking for Z-Asset Auth token and Gateway ID')
            if auth_token is None or gateway_id is None:
                raise AuthenticationError('Z-Asset Token or Gateway ID were not provided')
        # Either endpoint ID or SMSN can be provided in the configuration file
        if self.endpoint_id:
            # If SMSN was provided truncate to 5B and encode
            # Otherwise, Sidewalk ID doesnt need further processing, return it as it is
            self.is_smsn,\
            self.sidewalk_endpoint_id\
                = DeviceRegister.get_device_identifier(endpoint_id=self.endpoint_id)
        else:
            raise ValueError('No ENDPOINT_ID value')

        # If ASL_FORMAT is true, then asl_version is mandatory in the configuration file
        if self.asl_format:
            if self.asl_version >= 0:
                logger.info('ASL format registration with ASL Version %d', self.asl_version)
            else:
                raise ValueError('No ASL_VERSION value')
        else:
            logger.info('Non ASL format registration')

    def __post_encrypted_comm(self, halo_commands, cloud_func, get_from=None, get_to=None):
        # Format the incoming halo commands to be introduced in register_edge endpoint
        halo_command_map = {command.command: command.response for command in halo_commands}
        response = cloud_func(auth=os.getenv('AUTH_TOKEN'),
                              lwa_token=os.getenv('LWA_TOKEN'),
                              session_id=self.__session_id,
                              gateway_id=os.getenv('GATEWAY_ID'),
                              endpoint_id=self.sidewalk_endpoint_id,
                              halo_command_map=halo_command_map)
        # As incoming commands are encrypted
        # there is no current method to decode them
        # Store them in a list within their responses to
        # carry on the next API
        resp_device = []
        command_counter = 0
        if response.ok:
            # If get_from and get_to comes as None, assign the default list len as lower and upper
            # limits
            if not get_from:
                get_from = 0
            if not get_to:
                get_to = len(response.json()['haloCommands'])
            for encrypted_command in response.json()['haloCommands']:
                write(self.device, encrypted_command)
                device_response = read(self.device)
                # We care about command responses that are in the range(get_from, get_to)
                # If the current command count is not in the range, do not raise an exception
                if not device_response and command_counter in range(get_from, get_to):
                    raise BleCommandNotFoundError('Device response not found')
                resp_device.append(device_response)
                command_counter +=1
        else:
            # Raise a generic exception with the cloud name function
            raise RegistrationCloudError('Request to {} endpoint failed. '
                                         'Status: {}. '
                                         'Reason: {}. '
                                         'Body: {}'.format(cloud_func.__name__,
                                                           response.status_code,
                                                           response.reason,
                                                           response.text))
        return [Command(command=command_map[0], response=command_map[1])
                for command_map in zip(response.json()['haloCommands'], resp_device)
        ][get_from:get_to]


    @staticmethod
    def get_device_identifier(endpoint_id):
        # Check if the provided identifier is SMSN or Device ID
        endpoint_id_bytes = bytearray.fromhex(endpoint_id)
        if len(endpoint_id_bytes) == 32:
            # It is SMSN
            logger.info('SMSN was provided')
            # Truncate to 5B and encode
            endpoint_id_bytes = endpoint_id_bytes[0:5]
            endpoint_id_bytes[0] = (endpoint_id_bytes[0] & 0x1F) | 0xA0
            return True,\
                   endpoint_id_bytes.hex().upper()
        elif len(endpoint_id_bytes) != 32 and len(endpoint_id_bytes) != 5:
            raise ValueError("""Got unexpected endpoint ID length.
                             Actual length: {val} bytes. Expected length can be:
                             32 Bytes for SMSN or 5 Bytes for device ID""".format(val=len(endpoint_id_bytes)))
        else:
            # No further processing is needed for Endpoint ID
            logger.info('Device ID was provided')
            return False,\
                   endpoint_id

    def get_ble_capabilities(self):
        response = self.client.post_get_ble_capability(auth=os.getenv('AUTH_TOKEN'),
                                                       lwa_token=os.getenv('LWA_TOKEN'),
                                                       gateway_id=os.getenv('GATEWAY_ID'),
                                                       endpoint_id=self.sidewalk_endpoint_id,
                                                       asl_format=self.asl_format,
                                                       asl_version=self.asl_version)
        if response.ok:
            encryption_capabilities_command = response.json()['haloCommands'][0]
        else:
            raise GetBleCapabilityError('Request to BleCapability endpoint failed. '
                                        'Status: {}. '
                                        'Reason: {}. '
                                        'Body: {}'.format(response.status_code, response.reason, response.text))
        # Write and wait for all the commands to be returned
        write(self.device, encryption_capabilities_command)
        prov_init_req_cmd = read_until(self.device,
                                       CommandClass.SECURITY,
                                       CommandId.security.PROV_INIT_REQ)
        if not prov_init_req_cmd:
            raise BleCommandNotFoundError('PROV_INIT_REQ not found')

        prov_init_sig_cmd = read_until(self.device,
                                       CommandClass.SECURITY,
                                       CommandId.security.NOTIFY_PROV_INIT_SIG)
        if not prov_init_sig_cmd:
            raise BleCommandNotFoundError('PROV_INIT_SIG not found')
        return {
            'prov_init_req': Command(command=prov_init_req_cmd),
            'prov_init_sig': Command(command=prov_init_sig_cmd)
        }

    def start_ble_registration(self, prov_init_req, prov_init_req_sig):
        response = self.client.post_start_ble_registration(auth=os.getenv('AUTH_TOKEN'),
                                                           lwa_token=os.getenv('LWA_TOKEN'),
                                                           gateway_id=os.getenv('GATEWAY_ID'),
                                                           asl_format=self.asl_format,
                                                           asl_version=self.asl_version,
                                                           endpoint_id=self.sidewalk_endpoint_id,
                                                           prov_init_req=prov_init_req.command,
                                                           prov_init_sig=prov_init_req_sig.command)
        if response.ok:
            self.__session_id = response.headers['X-Amz-Halo-Provisioning-Session-Id']
            resp_prov_init_req = response.json()['haloCommands'][0]
            prov_init_sig_cmd = response.json()['haloCommands'][1]
            get_device_ecdh_sig_cmd = response.json()['haloCommands'][2]
            self.sidewalk_endpoint_id = response.json().get('sidewalkId')
            # Assign the returned sidewalk ID to the device identifier that will be used along the rest of the flow
            # If the Sidewalk Device ID was provided, the field 'sidewalkID' is not returned
            if self.sidewalk_endpoint_id is None:
                # Check if SMSN was provided
                if not self.is_smsn:
                    self.sidewalk_endpoint_id = self.endpoint_id
                else:
                    # There is no proceeding after this point, raise an exception and interrupt
                    raise StartBleRegistrationError('Request to StartBleRegistration endpoint failed. '
                                                    'Status: {}. '
                                                    'Reason: Expected sidewalkID field to be returned. '
                                                    'Body: {}. '
                                                    'Headers: {}.'.format(response.status_code,
                                                                          response.text,
                                                                          response.headers))

        else:
            raise StartBleRegistrationError('Request to StartBleRegistration endpoint failed. '
                                            'Status: {}. '
                                            'Reason: {}. '
                                            'Body: {}'.format(response.status_code, response.reason, response.text))

        write(self.device, resp_prov_init_req)

        # Write and wait for all the commands to be returned
        write(self.device, prov_init_sig_cmd)
        resp_prov_init_sig = read_until(self.device,
                                        CommandClass.SECURITY,
                                        CommandId.security.RESP_PROV_INIT_SIG)
        if not resp_prov_init_sig:
            raise BleCommandNotFoundError('RESP_PROV_INIT_SIG not found')

        write(self.device, get_device_ecdh_sig_cmd)
        resp_device_ecdh_sig = read_until(self.device,
                                          CommandClass.SECURITY,
                                          CommandId.security.RESP_DEVICE_ECDH_SIG)
        if not resp_device_ecdh_sig:
            raise BleCommandNotFoundError('RESP_DEVICE_ECDH_SIG not found')

        return {
            'prov_init_sig': Command(prov_init_sig_cmd,
                                     resp_prov_init_sig),
            'device_ecdh_sig': Command(get_device_ecdh_sig_cmd,
                                       resp_device_ecdh_sig)
        }

    def platform_key_exchange(self, prov_init_sig, device_ecdh_sig):
        response = self.client.post_channel_platform_key_exchange(auth=os.getenv('AUTH_TOKEN'),
                                                                  lwa_token=os.getenv('LWA_TOKEN'),
                                                                  session_id=self.__session_id,
                                                                  gateway_id=os.getenv('GATEWAY_ID'),
                                                                  endpoint_id=self.sidewalk_endpoint_id,
                                                                  prov_init_sig_cmd=prov_init_sig.command,
                                                                  device_ecdh_cmd=device_ecdh_sig.command,
                                                                  resp_prov_init_sig=prov_init_sig.response,
                                                                  resp_device_ecdh_sig=device_ecdh_sig.response)
        if response.ok:
            handshake_cmd = response.json()['haloCommands'][0]
        else:
            raise PlatformKeyExchangeError('Request to PlatformKeyExchange endpoint failed. '
                                           'Status: {}. '
                                           'Reason: {}. '
                                           'Body: {}'.format(response.status_code, response.reason, response.text))
        # Write and wait for all the commands to be returned
        write(self.device, handshake_cmd)

        resp_handshake = read(self.device)

        if not resp_handshake:
            raise BleCommandNotFoundError('RESP_HAND_SHAKE not found')
        return {'handshake': Command(handshake_cmd,
                                     resp_handshake)}

    def perform_handshake(self, handshake):
        response = self.client.post_channel_perform_handshake(auth=os.getenv('AUTH_TOKEN'),
                                                              lwa_token=os.getenv('LWA_TOKEN'),
                                                              session_id=self.__session_id,
                                                              gateway_id=os.getenv('GATEWAY_ID'),
                                                              endpoint_id=self.sidewalk_endpoint_id,
                                                              handshake_cmd=handshake.command,
                                                              resp_handshake=handshake.response)
        if response.ok:
            return True
        else:
            return False

    def halo_credentials(self):
        response = self.client.post_halo_credentials(auth=os.getenv('AUTH_TOKEN'),
                                                     lwa_token=os.getenv('LWA_TOKEN'),
                                                     session_id=self.__session_id,
                                                     gateway_id=os.getenv('GATEWAY_ID'),
                                                     endpoint_id=self.sidewalk_endpoint_id)
        # As incoming commands are encrypted
        # there is no current method to decode them
        # Store them in a list within their responses to
        # carry on the next API
        halo_commands = []
        if response.ok:
            for encrypted_command in response.json()['haloCommands']:
                write(self.device, encrypted_command)
                device_response = read(self.device)
                if not device_response:
                    raise BleCommandNotFoundError('Device response not found')
                halo_commands.append(Command(encrypted_command, device_response))
        else:
            raise HaloCredentialsError('Request to PlatformKeyExchange endpoint failed. '
                                       'Status: {}. '
                                       'Reason: {}. '
                                       'Body: {}'.format(response.status_code, response.reason, response.text))
        return halo_commands

    def register_edge(self, halo_commands):
        # First command in the response should be RESP_PROV_INIT_REQ, which is not required
        # leave it out of the command map
        return self.__post_encrypted_comm(halo_commands=halo_commands,
                                          cloud_func=self.client.post_register_edge,
                                          get_from=1)

    def application_perform_key_exchange(self, halo_commands):
        return self.__post_encrypted_comm(halo_commands=halo_commands,
                                          cloud_func=self.client.post_application_perform_key_exchange)

    def application_perform_handshake(self, halo_commands):
        return self.__post_encrypted_comm(halo_commands=halo_commands,
                                          cloud_func=self.client.post_application_perform_handshake)

    def application_register_edge(self, halo_commands):
        return self.__post_encrypted_comm(halo_commands=halo_commands,
                                          cloud_func=self.client.post_application_register_edge)
