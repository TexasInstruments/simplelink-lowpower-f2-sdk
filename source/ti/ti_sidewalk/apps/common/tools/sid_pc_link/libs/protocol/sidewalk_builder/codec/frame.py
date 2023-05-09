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

from .. import halo_collection as defs
from ..halo_app_layer_decoder import get_decoder
from ..utils import (
    bytarray_to_hex_str, bytearray_to_int, get_app_from_presentationlayer,
)
from .decoder import PktDecoder


IS_CLOUD = 0

FRM_TYPE_BEACON = 0
FRM_TYPE_DATA = 1
FRM_TYPE_NETWORK_DATA = 2
FRM_TYPE_ACK = 3


def get_dev_id(raw):
    dev_id = None
    sz = raw[0] >> 5
    if sz > 0:
        dev_id = bytearray_to_int(raw[:sz])
    return dev_id, sz


class FrameDecoder(PktDecoder):
    def __init__(self, raw_frm):
        self.link_layer = None
        self.network_layer = None
        self.presentation_layer = None
        self.application_layer = None
        self.src = None
        self.dst = None
        self.frm = self._format_pkt(raw_frm)
        self.is_decoded = False
        self.network_layer_raw = None
        self.link_layer_raw = None
        self.presentation_layer_raw = None

    def __repr__(self):
        if not self.is_decoded:
            return 'Frame not decoded.'
        _repr = []
        _repr.extend(
            self._dict_to_repr_chunk(
                self.link_layer, 'Link Layer:'))
        _repr.extend(
            self._dict_to_repr_chunk(
                self.network_layer, 'Network Layer:'))
        _repr.extend(
            self._dict_to_repr_chunk(
                self.presentation_layer, 'Presentation Layer:'))
        cmd_class = self.application_layer['cmd_class']
        cmd_id = self.application_layer['cmd_id']
        cls_name, id_name = defs.get_names(cmd_class, cmd_id)
        app_layer = self.application_layer.copy()
        app_layer.update({
            'cmd_class': '{} ({})'.format(cls_name, hex(cmd_class)),
            'cmd_id': '{} ({})'.format(id_name, hex(cmd_id)),
        })
        _repr.extend(self._dict_to_repr_chunk(app_layer, 'Application Layer:'))
        return '\n'.join(_repr)

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

    def decode(self):
        self.link_layer, idx = self.decode_link_layer(self.frm)
        self.network_layer, _idx = self.decode_network_layer(self.frm[idx:])
        idx += _idx
        self.presentation_layer = self.decode_presentation_layer(
            self.frm[idx:])
        self.application_layer = self.get_app_layer_data(
            self.presentation_layer)
        self.is_decoded = True

    def decode_link_layer(self, pkt):
        idx = 0
        ll = {}
        b = pkt[idx]
        ll['frame_type'] = b >> 5
        ll['protocol_version'] = b & 0xF
        idx += 1
        b = pkt[idx]
        ll['src_frmt'] = (b >> 6) & 3
        ll['dst_frmt'] = (b >> 4) & 3
        # ll['nw_frame_ctrl_inc'] = bool((b >> 3) & 1)
        if ll['frame_type'] is not FRM_TYPE_ACK:
            ll['ack_req'] = ((b >> 2) & 1) == 1
        ll['broadcast_ack'] = ((b >> 1) & 1) == 1
        idx += 1

        if ll['src_frmt'] != IS_CLOUD:
            ll['src'], sz = get_dev_id(pkt[idx:])
            idx += sz
            self.src = ll['src']
        if ll['dst_frmt'] != IS_CLOUD:
            ll['dst'], sz = get_dev_id(pkt[idx:])
            idx += sz
            self.dst = ll['dst']

        b = pkt[idx]
        seqn_sz = b >> 6
        ll['seqn_num'] = int(
            binascii.hexlify(pkt[idx:idx + seqn_sz]), 16)
        ll['seqn_num'] &= (2 ** (8 * seqn_sz - 2) - 1)
        idx += seqn_sz
        self.link_layer_raw = bytarray_to_hex_str(pkt[:idx])
        return ll, idx

    def decode_network_layer(self, pkt):
        nl = {}
        idx = 0
        b = pkt[idx]
        nl['is_fragment'] = bool(b >> 7)
        nl['frame_pld_type'] = (b >> 6) & 0x1
        nl['encr_ena'] = bool((b >> 5) & 0x1)
        nl['auth_code_size'] = {
            0: 0, 1: 2, 2: 4, 3: 8, 4: 12, 5: 16
        }[(b >> 2) & 0b111]
        idx += 1

        if nl['is_fragment']:
            nl['fragment_info'] = hex(pkt[idx])
            idx += 1

        if nl['auth_code_size'] > 0:
            auth_code = binascii.hexlify(pkt[idx:(idx + nl['auth_code_size'])])
            nl['auth_code'] = auth_code.upper()
            idx += nl['auth_code_size']
        self.network_layer_raw = bytarray_to_hex_str(pkt[:idx])
        return nl, idx

    def decode_presentation_layer(self, pkt):
        asl = {}
        idx = 0
        b = pkt[idx]
        asl['opc'] = b >> 6
        asl['seqn_inc'] = (b >> 5) & 0x1 == 1
        asl['rsp_req'] = (b >> 4) & 0x1 == 1
        asl['addr_enc_uint'] = (b & 0xF)
        asl['addr_enc'] = '0x{0:01X}'.format(asl['addr_enc_uint'])
        idx += 1
        if asl['addr_enc_uint'] < 0xC:
            asl['addr'] = asl['addr_enc_uint']
        else:
            addr_sz = asl['addr_enc_uint'] - 0xB
            asl['addr'] = bytearray_to_int(pkt[idx:idx + addr_sz])
            idx += addr_sz

        if asl['seqn_inc']:
            sz = (pkt[idx] >> 6)
            asl['seqn'] = int(binascii.hexlify(pkt[idx:idx + sz]), 16)
            asl['seqn'] &= (2 ** (8 * sz - 2) - 1)
            idx += sz
        asl['pld'] = pkt[idx:]
        self.presentation_layer_raw = bytarray_to_hex_str(pkt[:idx])
        return asl

    def get_app_layer_data(self, presentation_layer):
        pl = presentation_layer
        ald = get_app_from_presentationlayer(
            pl['addr_enc_uint'], pl['addr'], pl['opc'])
        cmd_data_decoder = get_decoder(ald['cmd_class'])
        ald.update({
            'cmd_data_raw': pl['pld'],
            'cmd_data': None
        })
        if cmd_data_decoder:
            decoded = cmd_data_decoder(ald['cmd_id'], pl['pld'])
            ald['cmd_data'] = decoded.data
            self.status_code = decoded.status_code
            ald['status_code'] = self.status_code
        return ald
