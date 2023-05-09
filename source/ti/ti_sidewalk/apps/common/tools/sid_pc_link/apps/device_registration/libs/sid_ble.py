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

from sidewalk_builder.codec.flex import FlexPktDecoder
from sidewalk_builder.ama.encoder import AmaPktEncoder
from sidewalk_builder.ama.decoder import AmaPktDecoder

import os
import time
import logging

logger = logging.getLogger(__name__)


def write(device, data):
    packet_decoded = FlexPktDecoder(data)()
    logger.info("Write Raw data {}".format(data))
    logger.info("Write Application Layer: {}".format(packet_decoded.application_layer))
    logger.info("Write Headers: {}".format(packet_decoded.header_fields))
    data = bytearray.fromhex(data)
    ama_headers = {'stream_id': 0b0111, 'total_trxn_len': len(data)}
    ama_encoded = AmaPktEncoder(ama_headers, data)
    device.write(ama_encoded.encoded)

def reaseemble_ama_get_data(device):
    aggregate_data = bytearray()
    total_len = None
    while True:
        data = device.read()
        if data is None:
            return None
        logger.info("Raw data {}".format(data.hex()))

        try:
            ama_decoded = AmaPktDecoder(data)
        except TypeError as err:
            logger.warning("Not an AMA packet: {}. Error: {}".format(data, err))
            return None

        if total_len is None:
            total_len = ama_decoded.total_trxn_len

        assert(ama_decoded.data)
        aggregate_data += ama_decoded.data

        if len(aggregate_data) == total_len:
            break

    return aggregate_data

def read(device):
    data = reaseemble_ama_get_data(device)
    if data is None:
        return data

    try:
        packet_decoded = FlexPktDecoder(bytearray(data))()
    except ValueError as err:
        logger.warning("Not a flex packet: {}. Error: {}".format(data.hex(), err))
        return None
    logger.info("Application Layer: {}".format(packet_decoded.application_layer))
    logger.info("Headers: {}".format(packet_decoded.header_fields))
    return packet_decoded.enc_pkt.hex()


def read_until(device, command_class, command_id, timeout=None):
    packet = None
    if timeout is None:
        timeout = int(os.getenv('COMMAND_TIMEOUT', 60)) # 60 seconds timeout
    t_init = time.time()
    while time.time() - t_init < timeout:
        packet = read(device)
        if packet is not None:
            decoded_packet = FlexPktDecoder(packet)()
            resp_command_class = decoded_packet.cmd_class
            resp_command_id = decoded_packet.cmd_id
            if resp_command_class == command_class and resp_command_id == command_id:
                break
        logger.warning("No response from device")
    return packet
