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

from .exceptions import DecodingError
from ..utils import str_to_byte_arr, bytarray_to_hex_str


class PktDecoder:
    def __call__(self):
        try:
            self.decode()
        except Exception as e:
            raise DecodingError('Packet could not be decoded. {}'.format(e))
        return self

    def _format_pkt(self, pkt):
        if not isinstance(pkt, bytearray):
            try:
                pkt = pkt.replace('0x', '')
            except AttributeError:
                raise ValueError(
                    'Packet type should be either bytearray or string')
            pkt = str_to_byte_arr(pkt)
        return pkt

    def _dict_to_repr_chunk(self, d, title, prefix=''):
        if not d:
            return []
        rchunk = ['']
        if prefix:
            rchunk = []
        rchunk.append(title)
        for k, v in list(d.items()):
            if isinstance(v, bytearray):
                v = bytarray_to_hex_str(v)
            if isinstance(v, int):
                v = '{} ({})'.format(v, hex(v))
            if isinstance(v, dict):
                rchunk.extend(
                    self._dict_to_repr_chunk(v, '- {}:: '.format(k), '\t'))
            else:
                rchunk.append('{}- {}: {}'.format(prefix, k, v))
        return rchunk
