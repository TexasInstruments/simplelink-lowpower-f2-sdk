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

class AmaPktDecoder(object):
    """
    Ama Packet Decoder
    """

    def __init__(self, pkt):
        self._pkt = pkt
        assert(len(pkt) > 0x2)
        if self._is_initial_pkt():
            assert(len(pkt) > 0x5)
        assert(self.type != 0x3)

    def _is_initial_pkt(self):
        return self.type == 0x0

    @property
    def stream_id(self):
        return (self._pkt[0] & 0xf0) >> 4

    @property
    def trxn(self):
        return self._pkt[0] & 0x0f

    @property
    def seq(self):
        return (self._pkt[1] & 0xf0) >> 4

    @property
    def type(self):
        return (self._pkt[1] & 0x0c) >> 2

    @property
    def ack(self):
        return self._pkt[1] & 0x02 >> 1

    @property
    def ext(self):
        return self._pkt[1] & 0x01

    @property
    def total_trxn_len(self):
        return int.from_bytes(self._pkt[3:5], byteorder='big') if self._is_initial_pkt() else 0

    @property
    def pkt_len(self):
        return self._pkt[5] if self._is_initial_pkt() else self._pkt[2]

    @property
    def data(self):
        return self._pkt[6:] if self._is_initial_pkt() else self._pkt[3:]

    def __repr__(self):
        return str({
            "stream_id": self.stream_id,
            "trxn": self.trxn,
            "seq": self.seq,
            "type": self.type,
            "ack": self.ack,
            "ext": self.ext,
            "total_trxn_len": self.total_trxn_len,
            "pkt_len": self.pkt_len,
            "data": self.data
        })
