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
from .decoder import PktDecoder
from .. import halo_collection as defs
from ..halo_app_layer_decoder import get_decoder
from ..utils import (
    str_to_byte_arr, bytearray_to_int, bytarray_to_hex_str,
    get_presentation_from_app_layer,
)

flex_header_key_name_to_code = {
    'dst': 0x1,
    'dst_frmt': 0x2,
    'ack_req': 0x3,
    'encr_ena': 0x4,
    'auth_code_inc': 0x5,
    'resp_req': 0x6,
    'seqn': 0x8,
    'retries': 0xa,
    'rssi': 0xb,
    'snr': 0xc,
    'carrier_offset': 0xd,
    'rx_tstamp': 0xe,
    'sec_enabled': 0xf,
    'src': 0x10,
    'src_frmt': 0x11,
    'proto_ver_num': 0x12,
    'auth_tag': 0x23,
    'is_data_unified_asl': 0x2b
}

flex_header_key_code_to_name = {
    v: k for k, v in list(flex_header_key_name_to_code.items())
}


class FlexPktEncoder(object):
    '''Flex Format Packet Encoder'''
    def __init__(self, header_fields, cmd_class, cmd_id, cmd_data=None):
        self.encoded = None
        self.header_len = 0
        self.header = bytearray()
        self.cmd_class = cmd_class
        self.cmd_id = cmd_id
        self.cmd_data = cmd_data
        self.header_fields = header_fields
        self._unify_formats()

    def __call__(self):
        self._encode_pkt()
        return self

    def __repr__(self):
        cmd_data_repr = None
        if self.cmd_data:
            cmd_data_repr = bytarray_to_hex_str(self.cmd_data)
        cls_name, id_name = defs.get_names(self.cmd_class, self.cmd_id)
        return '\n'.join([
            'Flex Header Data: {}'.format(
                {
                    k: bytarray_to_hex_str(v)
                    for k, v in self.header_fields.items()
                }
            ),
            'Command Class: {} (0x{})'.format(
                cls_name, bytarray_to_hex_str(self.cmd_class)),
            'Command Id: {} (0x{})'.format(
                id_name, bytarray_to_hex_str(self.cmd_id)),
            'Command Data: {}'.format(cmd_data_repr)
        ])

    @property
    def encoded_hex(self):
        return bytarray_to_hex_str(self.encoded).upper()

    def _unify_formats(self):
        if isinstance(self.cmd_class, int):
            self.cmd_class = '{0:04X}'.format(self.cmd_class)
        if isinstance(self.cmd_id, int):
            self.cmd_id = '{0:04X}'.format(self.cmd_id)

        if isinstance(self.cmd_class, str):
            self.cmd_class = str_to_byte_arr(self.cmd_class)
        if isinstance(self.cmd_id, str):
            self.cmd_id = str_to_byte_arr(self.cmd_id)
        if isinstance(self.cmd_data, str):
            self.cmd_data = str_to_byte_arr(self.cmd_data)
        for k, v in self.header_fields.items():
            if isinstance(v, str):
                self.header_fields[k] = str_to_byte_arr(v)

    def _add_header_field(self, _key, val):
        key = flex_header_key_name_to_code.get(_key)
        if not key:
            raise Exception('{} flex header key is not supported'.format(key))
        lng = len(val)
        flex_hdr_f_lng = {1: 0x0, 2: 0x1, 4: 0x2}.get(lng, 0x3)
        flex_hdr_f = (flex_hdr_f_lng << 6) | key
        flex_hdr_f = bytearray([flex_hdr_f])
        if flex_hdr_f_lng == 0x3:
            flex_hdr_f.append(lng)
        flex_hdr_f.extend(val)
        self.header.extend(flex_hdr_f)
        self.header_len += len(flex_hdr_f)

    def _generate_header(self):
        for k, v in self.header_fields.items():
            self._add_header_field(k, v)

    def _encode_pkt(self):
        self._generate_header()
        self.encoded = bytearray([self.header_len])
        self.encoded.extend(self.header)
        self.encoded.extend(self.cmd_class)
        self.encoded.extend(self.cmd_id)
        if self.cmd_data:
            # nothing in the cmd_data_hdr
            self.encoded.append(0x0)
            self.encoded.append(len(self.cmd_data))
            self.encoded.extend(self.cmd_data)


class FlexPktDecoder(PktDecoder):
    '''Flex Format Packet Decoder'''
    def __init__(self, enc_pkt):
        self.application_layer = None
        self.enc_pkt = self._format_pkt(enc_pkt)
        self.header_fields = {}
        self.is_decoded = False
        self.cmd_class = None
        self.cmd_id = None
        self.status_code = None
        self.cmd_data_raw = None
        self.cmd_data = None
        self.src = None
        self.dst = None
        self.is_unified_asl = False
        self.used_length = None
        self.encr_ena = False
        self.application_encrypted = False

    def __call__(self):
        self.decode()
        self.is_decoded = True
        return self

    def __repr__(self):
        if not self.is_decoded:
            return 'Frame not decoded.'
        appl = self.application_layer.copy()
        cls_name, id_name = defs.get_names(appl['cmd_class'], appl['cmd_id'])
        _repr = []
        _repr.extend(
            self._dict_to_repr_chunk(self.header_fields, 'Flex Header Data:')
        )
        appl.update({
            'cmd_class': '{} ({})'.format(cls_name, hex(appl['cmd_class'])),
            'cmd_id': '{} ({})'.format(id_name, hex(appl['cmd_id'])),
        })
        _repr.extend(
            self._dict_to_repr_chunk(appl, 'Application Layer:')
        )
        return '\n'.join(_repr)

    def decode(self):
        idx = 0
        p = self.enc_pkt
        flex_hdr_len = p[idx]
        idx += 1
        self._decode_flex_header_fields(p[idx:idx + flex_hdr_len])
        idx += flex_hdr_len

        if self.encr_ena:
            return

        if self.is_unified_asl:
            self.used_length = 0
            self._decode_unified_asl_fields(p[idx:idx + len(self.enc_pkt)], self.used_length)
            if self.application_encrypted:
                return

            idx += self.used_length
            if len(p) > idx:
                self.cmd_data_raw = p[idx:]
                self.application_layer['cmd_data_raw'] = self.cmd_data_raw
                cmd_data_decoder = get_decoder(self.cmd_class)
                if cmd_data_decoder:
                    cmd_data = cmd_data_decoder(
                        self.cmd_id, self.application_layer['cmd_data_raw'], self.is_unified_asl)
                    self.application_layer['cmd_data'] = cmd_data.data
                    self.cmd_data = cmd_data.data
        else:
            if self.encr_ena:
                return

            cmd_class = bytearray_to_int(p[idx:idx + 2])
            idx += 2
            cmd_id = bytearray_to_int(p[idx:idx + 2])
            idx += 2
            self.application_layer = {
                'cmd_class': cmd_class,
                'cmd_id': cmd_id,
            }
            self.cmd_class = cmd_class
            self.cmd_id = cmd_id

            if len(p) > idx:
                self.cmd_data_raw = p[idx:]
                self.application_layer['cmd_data_raw'] = self.cmd_data_raw
                cmd_data_decoder = get_decoder(cmd_class)
                if cmd_data_decoder:
                    cmd_data = cmd_data_decoder(
                        cmd_id, self.application_layer['cmd_data_raw'], self.is_unified_asl)
                    self.application_layer['cmd_data'] = cmd_data.data
                    self.status_code = cmd_data.status_code
                    self.cmd_data = cmd_data.data
            self.presentation_layer = get_presentation_from_app_layer(
                cmd_class,
                cmd_id
            )
        try:
            self.presentation_layer['pld'] = self.application_layer['cmd_data_raw']
        except:
            pass
        self.is_decoded = True

    def _decode_flex_header_fields(self, flex_hdr_b):
        idx = 0
        while idx < len(flex_hdr_b):
            key = flex_header_key_code_to_name.get(
                flex_hdr_b[idx] & 0b111111
            )
            flex_hdr_f_lng = flex_hdr_b[idx] >> 6
            flex_hdr_f_lng = {0: 1, 1: 2, 2: 4}.get(flex_hdr_f_lng, 0b11)
            if flex_hdr_f_lng == 0b11:
                idx += 1
                flex_hdr_f_lng = flex_hdr_b[idx]
            idx += 1
            val = flex_hdr_b[idx:idx + flex_hdr_f_lng]
            if key:
                self.header_fields[key] = bytearray_to_int(val)
            idx += flex_hdr_f_lng
        if self.header_fields.get('src'):
            self.src = self.header_fields.get('src')
        if self.header_fields.get('dst'):
            self.dst = self.header_fields.get('dst')
        if self.header_fields.get('is_data_unified_asl'):
            self.is_unified_asl = self.header_fields.get('is_data_unified_asl')
        if self.header_fields.get('encr_ena'):
            self.encr_ena = self.header_fields.get('encr_ena')

    def _decode_unified_asl_fields(self, pkt_b, used_length):
        idx = 0
        asl_hdr = {}
        b = pkt_b[idx]
        asl_hdr['version_inc'] = (b >> 7) & 1
        asl_hdr['resp_req'] = (b >> 6) & 1
        asl_hdr['seqn_inc'] = (b >> 5) & 1
        asl_hdr['app_enc_ena'] = (b >> 4) & 1
        asl_hdr['status_hdr_inc'] = (b >> 3) & 1
        asl_hdr['nw_data_blob_inc'] = (b >> 2) & 1
        idx += 1

        if asl_hdr['version_inc'] == 1:
            b = pkt_b[idx]
            asl_hdr['asl_version'] = (b >> 6) & 0x3
            asl_hdr['class_major_minor'] = b & 0x3F
            idx += 1

        b = pkt_b[idx]
        seqn_sz = b >> 6
        asl_hdr['seqn_num'] = int(binascii.hexlify(pkt_b[idx:idx + seqn_sz]), 16)
        asl_hdr['seqn_num'] &= (2 ** (8 * seqn_sz - 2) - 1)
        idx += seqn_sz

        if asl_hdr['nw_data_blob_inc'] == 1:
            b = pkt_b[idx]
            nw_blob_id = b >> 4
            idx += 1
            if nw_blob_id == 0:
                if b & 0xF == 0xF:
                    nw_data_blob_size = pkt_b[idx] << 0x8 | pkt_b[idx+1]
                    idx += 2
                else:
                    if pkt_b[idx] == 0xFE:
                        nw_data_blob_size = (b & 0xF) * 2 - 1
                    else:
                        nw_data_blob_size = (b & 0xF) * 2
            else:
                return
            idx += nw_data_blob_size

        if asl_hdr['app_enc_ena'] == 1:
            self.application_encrypted = True
            return

        if asl_hdr['status_hdr_inc'] == 1:
            b = pkt_b[idx]
            asl_hdr['status_code_inc'] = (b >> 7) & 1
            asl_hdr['additional_app_data_inc'] = (b >> 6) & 1
            if asl_hdr['status_code_inc'] == 1 and asl_hdr['additional_app_data_inc'] == 0:
                asl_hdr['status_code']  = b & 0x3F
                idx +=1
            if asl_hdr['status_code_inc'] == 0 and asl_hdr['additional_app_data_inc'] == 1:
                additional_app_data_len = b & 0x3F
                idx += additional_app_data_len
            if asl_hdr['status_code_inc'] == 1 and asl_hdr['additional_app_data_inc'] == 1:
                asl_hdr['status_code']  = b & 0x3F
                idx += 1
                additional_app_data_len = b & 0x3F
                idx += additional_app_data_len

        if asl_hdr['status_hdr_inc'] == 1 and asl_hdr['status_code_inc'] == 1:
            self.status_code = asl_hdr['status_code']

        b = pkt_b[idx]

        asl_hdr['opc'] = (b >> 6) & 0x3
        asl_hdr['addr_enc'] = b & 0x3F

        if asl_hdr['addr_enc'] <= 0x3B:
            asl_hdr['class'] = (b >> 3) & 0x3
            asl_hdr['cmd_id'] = b & 0x3
        else:
            if asl_hdr['addr_enc'] > 0x3B and asl_hdr['addr_enc'] <= 0x3F:
                addr_size = asl_hdr['addr_enc'] - 0x3B
                if addr_size == 1:
                    idx += 1
                    b = pkt_b[idx]
                    asl_hdr['class'] = (b >> 4) & 0xF
                    asl_hdr['cmd_id'] = b & 0xF
                if addr_size == 2:
                    idx += 1
                    b = pkt_b[idx]
                    asl_hdr['class'] = b
                    idx += 1
                    b = pkt_b[idx]
                    asl_hdr['cmd_id'] = b
                if addr_size == 3:
                    idx += 1
                    class_id = pkt_b[idx]
                    idx += 1
                    b = pkt_b[idx]
                    asl_hdr['class'] = (class_id << 4) | ((b >> 4) & 0xF)
                    cmd_id = b & 0xF
                    idx += 1
                    b = pkt_b[idx]
                    asl_hdr['cmd_id'] = (cmd_id << 8) | b
                if addr_size == 4:
                    idx += 1
                    class_id = pkt_b[idx]
                    idx += 1
                    b = pkt_b[idx]
                    asl_hdr['class'] = (class_id << 8) | b
                    idx += 1
                    cmd_id = pkt_b[idx]
                    idx += 1
                    b = pkt_b[idx]
                    asl_hdr['cmd_id'] = (cmd_id << 8) | b

        idx += 1

        self.used_length = idx
        cmd_class = asl_hdr['class']
        cmd_id = asl_hdr['cmd_id'] << 2 | asl_hdr['opc']
        self.application_layer = {
            'cmd_class': cmd_class,
            'cmd_id': cmd_id,
        }
        self.cmd_class = cmd_class
        self.cmd_id = cmd_id
