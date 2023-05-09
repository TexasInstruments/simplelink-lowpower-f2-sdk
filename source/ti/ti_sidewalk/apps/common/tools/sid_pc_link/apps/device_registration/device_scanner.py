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

from sidewalk_builder.ble_beacon.decoder import BleBeaconDecoder
import logging

logger = logging.getLogger(__name__)


def scan(transport):
    return transport.scan()


def scan_for_device(transport, sidewalk_id):
    discovered = scan(transport)
    found = []
    for device in discovered:
        if device.manufacturer_data is not None:
            decoded_beacon = BleBeaconDecoder(device.manufacturer_data)
            try:
                if decoded_beacon.application_data.get('tx_id') is not None:
                    if decoded_beacon.application_data.get('tx_id').lower() == sidewalk_id.lower():
                        found.append(device)
            except IndexError as err:
                logger.error('Cannot obtain TX ID for device. Skipping it')
    return {
        'discovered': discovered,
        'found': found
    }
