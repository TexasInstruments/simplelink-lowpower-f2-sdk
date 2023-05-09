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

class AmaPktEncoder(object):
    """
    Ama Packet Encoder
    """

    def __init__(self, headers, data):
        _stream_id = self._get_field(headers, "stream_id")
        assert(_stream_id < 0x10)
        _stream_id = _stream_id << 4

        _trxn = self._get_field(headers, "trxn")
        assert(_trxn < 0x10)
        _trxn = _trxn

        _seq = self._get_field(headers, "seq")
        assert(_seq < 0x10)
        _seq = _seq << 4

        _type = self._get_field(headers, "type")
        assert(_type < 0x4)
        assert(_type != 0x3)
        _type = _type << 2

        _ack = self._get_field(headers, "ack")
        assert(_ack < 0x2)
        _ack = _ack << 1

        _ext = self._get_field(headers, "ext")
        assert(_ext < 0x2)

        self._encoded = bytearray([_stream_id | _trxn, _seq | _type | _ack | _ext])

        if _type == 0x00:
            _total_trxn_len = self._get_field(headers, "total_trxn_len")
            assert(_total_trxn_len < (2**16) + 1)
            # Reserved
            self._encoded += b"\00"
            self._encoded += _total_trxn_len.to_bytes(2, byteorder='big')

        _pkt_len = len(data)
        self._encoded += bytes([_pkt_len])

        assert(isinstance(data, bytes) or isinstance(data, bytearray))
        self._encoded += data


    def _get_field(self, headers, name):
        if hasattr(headers, name):
            return headers.name
        if name in headers:
            return headers[name]
        return 0

    @property
    def encoded(self):
        return self._encoded
