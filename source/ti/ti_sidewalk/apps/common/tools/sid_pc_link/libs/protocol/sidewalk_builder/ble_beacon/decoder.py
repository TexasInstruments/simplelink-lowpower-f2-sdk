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

from .collection import *

MAX_LEN = 20


class BleBeaconDecoder(object):

    def __init__(self, packet):
        self.__raw_packet = packet
        self.__device_state = {
            'device_state': None,
            'device_mode': None,
            'battery_level': None,
            'advertisement_version_format': None
        }
        self.__application_data = {
            'frame_indicator': None,
            'tx_id': None
        }
        self.__is_valid = True
        self.__raw_packet_bytes = bytearray.fromhex(self.__raw_packet)
        if len(self.__raw_packet_bytes) > MAX_LEN:
            self.__is_valid = False

    @property
    def application_id(self):
        if self.__is_valid:
            return application_id[self.__raw_packet_bytes[0]]
        return None

    @property
    def device_state(self):
        if self.__is_valid:
            return {
                'device_state': device_state[self.__raw_packet_bytes[1] & 0x01],
                'device_mode': device_mode[(self.__raw_packet_bytes[1] >> 1) & 0x3],
                'battery_level': battery_level[(self.__raw_packet_bytes[1] >> 3) & 0x3],
                'advertisement_version_format': (self.__raw_packet_bytes[1] >> 5) & 0x7
            }
        return self.__device_state

    @property
    def application_data(self):
        if self.__is_valid:
            return {
                'frame_indicator': {
                    'tx_id_format': (self.__raw_packet_bytes[2] >> 6) & 0x3,
                    'asl_version': (self.__raw_packet_bytes[2] >> 4) & 0x3,
                    'payload_data_indicator': self.__raw_packet_bytes[2] & 0xF
                },
                'tx_id': self.__raw_packet_bytes[3:8].hex()
            }
        return self.__application_data

    @property
    def decoded(self):
        return {
            'application_id': self.application_id,
            'device_state': self.device_state,
            'application_data': self.application_data
        }

    def __repr__(self):
        return str(self.decoded)
