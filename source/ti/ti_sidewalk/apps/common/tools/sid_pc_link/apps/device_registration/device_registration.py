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

from apps.device_registration import app_config
from transports.transport import DeviceTransport
from transports.ble.device_descriptor import BluetoothDeviceDescriptor
from http_client.registration_client import RegistrationClient
from apps.device_registration.libs.registration import DeviceRegister
from apps.device_registration.libs.errors import *
from apps.device_registration.device_scanner import scan_for_device
from sidewalk_builder.ble_beacon.decoder import BleBeaconDecoder


import logging
import backoff
import json

logger = logging.getLogger(__name__)

GET_DEVICE_INFO = '0000000002020814'
GET_DEVICE_FEATURES = '010000000202081C'

DEVICE_REGISTRATION_CHARACTERISTICS = {
    'mailbox_write_char' : '74f996c9-7d6c-4d58-9232-0427ab61c53c',
    'mailbox_notify_char': 'b32e83c0-fece-47c1-9015-53b7e7f0d2fe'
}

def register_device(register):
    commands = register.get_ble_capabilities()
    commands = register.start_ble_registration(prov_init_req=commands.get('prov_init_req'),
                                               prov_init_req_sig=commands.get('prov_init_sig'))
    commands = register.platform_key_exchange(prov_init_sig=commands.get('prov_init_sig'),
                                              device_ecdh_sig=commands.get('device_ecdh_sig'))
    is_handshake_ok = register.perform_handshake(handshake=commands.get('handshake'))
    if not is_handshake_ok:
        raise PerformHandshakeError('Error in PerformHandshake endpoint')
    halo_commands = register.halo_credentials()
    halo_commands = register.register_edge(halo_commands=halo_commands)
    halo_commands = register.application_perform_key_exchange(halo_commands=halo_commands)
    halo_commands = register.application_perform_handshake(halo_commands=halo_commands)
    halo_commands = register.application_register_edge(halo_commands=halo_commands)
    if len(halo_commands) == 0:
        logger.info("Device registration succeeded")
    else:
        logger.error("Device registration failed. Application/RegisterEdge endpoint returned failure status")


def get_device_identifier(config, transport):
    retries = 0
    found = list()
    discovered = list()
    _, device_identifier = DeviceRegister.get_device_identifier(config.get('ENDPOINT_ID'))
    logger.info('Looking for SMSN: {smsn}'.format(smsn=device_identifier))
    while len(found) == 0 and retries < int(config.get('SCAN_RETRIES', 5)):
        scanned = scan_for_device(transport, device_identifier)
        found = scanned.get('found')
        discovered.extend(scanned.get('discovered'))
        retries+=1
    # Close the transport and return the list of devices
    if len(found) == 0:
        seen = set()
        logger.info('These devices were discovered:')
        for device in discovered:
            decoded_device = json.loads(str(device))
            if decoded_device.get('manufacturer_data') is not None:
                device_id = decoded_device.get('device_id')
                try:
                    tx_id = BleBeaconDecoder(
                        decoded_device.get('manufacturer_data')
                    ).application_data.get('tx_id')
                except IndexError:
                    tx_id = None
                if device_id is not None\
                    and tx_id is not None\
                        and tx_id not in seen:
                    logger.info(
                        'Device: {mac_address} with Sidewalk ID: {tx_id}'.format(
                            mac_address=device_id,
                            tx_id=tx_id
                        )
                    )
                    seen.add(tx_id)
        raise DeviceNotFoundError(f'Device with Sidewalk ID {device_identifier} cannot be found')
    else:
        logger.info(f'{len(found)} device(s) found with the Sidewalk ID {device_identifier}. '
                    f'Using the first occurrence')
        return found[0]


@backoff.on_exception(backoff.constant,
                      (AmaConnectFailedError),
                      max_tries=2,
                      interval=2)
def establish_ama_connection(device):
    logger.info('Attempt AMA connection')
    # The following 2 commands are needed to establish an ama connection
    # with the device. The cmds are hard coded, since they don't really
    # change. Once the device receives the responses for each of the send
    # command the script can safely assume that the connection has been
    # established.
    conn_req_resp = list()
    conn_req_resp.append(device.read())
    try:
        device.write(bytearray.fromhex(GET_DEVICE_INFO))
        conn_req_resp.append(device.read())
        device.write(bytearray.fromhex(GET_DEVICE_FEATURES))
        conn_req_resp.append(device.read())
        if not all(conn_req_resp):
            raise AmaConnectFailedError('{} AMA not connected'.format(device))
    except Exception as err:
        raise AmaConnectFailedError('{} AMA communication error'.format(device))


def main():
    # Initialize the environment
    # Get the configuration
    registration_config = app_config.get_config()
    # Start the registration HTTP client
    registration_client = RegistrationClient()
    try:
        # Start the device
        # Initialize the Device Transport and an empty device descriptor, we are
        # not connecting to a device immediately
        device_descriptor = BluetoothDeviceDescriptor()
        device = DeviceTransport(device_descriptor)
        device_descriptor = get_device_identifier(registration_config, device)
        # From the device descriptor we got from the scanning append the
        # characteristics and connect
        device_descriptor.characteristics = DEVICE_REGISTRATION_CHARACTERISTICS
        device.open(device_descriptor)
        # Establish Ama connection
        establish_ama_connection(device)
        # Start the Registrar
        device_register = DeviceRegister(device=device,
                                         client=registration_client)
        register_device(device_register)
        # Close the BLE connection
        device.close()
    except Exception as err:
        logger.error('An error occurred during device registration')
        logger.error('Error: {}'.format(err))
        registed_message = "Please make sure device is not registered"
        logger.error("="*len(registed_message))
        logger.error(registed_message)
        logger.error("="*len(registed_message))

        logger.error('Closing device connection')
        device.close()
