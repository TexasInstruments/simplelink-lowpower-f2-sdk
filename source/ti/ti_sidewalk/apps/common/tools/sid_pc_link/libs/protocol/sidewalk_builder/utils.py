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

import binascii
import math


def uint_to_int_2_compl(uval, nbits=8):
    is_negative = (uval >> (nbits - 1)) == 1
    if is_negative:
        max_uval = int(math.pow(2, nbits - 1) - 1)
        resp = (uval & max_uval) - max_uval - 1
    else:
        resp = uval
    return resp


def bytearray_to_int(arr):
    return int(binascii.hexlify(arr), 16)


def bytarray_to_hex_str(ba, prefix=''):
    return prefix + binascii.hexlify(ba).decode('ASCII').upper()


def str_to_byte_arr(s):
    s = s.replace(' ', '')
    if s.startswith('0x'):
        s = s[2:]
    return bytearray.fromhex(s)


def get_presentation_from_app_layer(cmd_class, cmd_id):
    opc = cmd_id & 0x3
    cmd_id = cmd_id >> 2  # take off opcode bits from cmd_id

    if ((cmd_class <= 0xF) and (cmd_id <= 0xF)):
        addr = (cmd_class << 4) | cmd_id
        addr_enc = 0xC
    elif ((cmd_class <= 0xFF) and (cmd_id <= 0xFF)):
        addr = (cmd_class << 8) | cmd_id
        addr_enc = 0xD
    elif ((cmd_class <= 0xFFF) and (cmd_id <= 0xFFF)):
        addr = (cmd_class << 12) | cmd_id
        addr_enc = 0xE
    else:
        addr = (cmd_class << 16) | cmd_id
        addr_enc = 0xF
    return {'opc': opc, 'addr_enc': addr_enc, 'addr': addr}


def get_app_from_presentationlayer(addr_enc, addr, opc):
    cmd_class = 0
    cmd_id = 0

    if addr_enc == 0xC:
        cmd_class = addr >> 4
        cmd_id = addr & 0xF
    elif addr_enc == 0xD:
        cmd_class = addr >> 8
        cmd_id = addr & 0xFF
    elif addr_enc == 0xE:
        cmd_class = addr >> 12
        cmd_id = addr & 0xFFF
    elif addr_enc == 0xF:
        cmd_class = addr >> 16
        cmd_id = addr & 0xFFFF

    cmd_id = (cmd_id << 2) | opc
    return {'cmd_id': cmd_id, 'cmd_class': cmd_class}
