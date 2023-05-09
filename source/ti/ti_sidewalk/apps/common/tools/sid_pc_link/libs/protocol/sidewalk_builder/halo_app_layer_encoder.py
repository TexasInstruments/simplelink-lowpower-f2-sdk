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

import struct

from . import halo_collection as defs


class CmdDataEncoder(object):
    def __init__(self, cmd_id, fields):
        self.cmd_id = cmd_id
        self.fields = fields
        self.data = self.encode()

    def encode(self):
        cmd_id_name = self.cmd_ids.get_name_by_value(self.cmd_id)
        method_name = 'encode_{}'.format(cmd_id_name).lower()
        if hasattr(self, method_name):
            data = getattr(self, method_name)()
            return data
        else:
            return None

class SecurityCmdDataEncoder(CmdDataEncoder):
    """Encode Security commands."""

    def __init__(self, *args, **kwargs):
        self.cmd_ids = defs.SecurityCommandIdCollection
        super(SecurityCmdDataEncoder, self).__init__(*args, **kwargs)

    def encode_get_encryption_capabilities(self):
        return bytearray(
                [
                    self.fields['requester_mode']
                ]
        )

    def encode_check_cert_serial(self):
        array = []
        array.extend([self.fields['intermediate_level']])
        array.extend(struct.unpack("4B", struct.pack(">I",
                                                     self.fields['intermediate_certificate_serial'])))
        return bytearray(array)

    def encode_get_device_cert_count_nonce(self):
        return None

    def encode_get_device_serial_puk(self):
        return None

    def encode_get_device_sig(self):
        return None

    def encode_get_model_serial_puk(self):
        return None

    def encode_get_model_sig(self):
        return None

    def encode_set_remote_nonce_ecdh_puk_cert_count(self):
        array = []
        array.extend([self.fields['frame_count']])
        array.extend([self.fields['cert_count']])
        assert(len(self.fields['remote_nonce']) == 8)
        array.extend(self.fields['remote_nonce'])
        assert(len(self.fields['remote_ecdh_puk']) == 32)
        array.extend(self.fields['remote_ecdh_puk'])
        return bytearray(array)

    def encode_set_remote_ecdh_sig(self):
        array = []
        assert(len(self.fields['ecdh_sig']) == 64)
        array.extend(self.fields['ecdh_sig'])
        return bytearray(array)

    def encode_set_remote_cert_serial_puk(self):
        array = []
        assert(len(self.fields['serial_number']) == 4)
        array.extend(self.fields['serial_number'])
        assert(len(self.fields['model_puk']) == 32)
        array.extend(self.fields['model_puk'])
        return bytearray(array)

    def encode_set_remote_sig(self):
        array = []
        assert(len(self.fields('sig')) == 64)
        array.extend(self.fields('sig'))
        return bytearray(array)

    def encode_get_device_ecdh_puk(self):
        return None

    def encode_get_device_ecdh_sig(self):
        return None

    def encode_resp_prov_init_req(self):
        array = []
        array.append(self.fields['frame_count'])
        array.append(self.fields['cert_count'])
        assert(len(self.fields['nonce']) == 12)
        array.extend(self.fields['nonce'])
        assert(len(self.fields['ed25519_ecdhe_puk']) == 32)
        array.extend(self.fields['ed25519_ecdhe_puk'])
        return bytearray(array)

    def encode_prov_init_sig(self):
        array = []
        assert(len(self.fields['signature']) == 64)
        array.extend(self.fields['signature'])
        return bytearray(array)

    def encode_hand_shake(self):
        array = []
        assert(len(self.fields['dev_id']) == 5)
        array.extend(self.fields['dev_id'])
        assert(len(self.fields['rand_a']) == 8)
        array.extend(self.fields['rand_a'])
        return bytearray(array)


class CustomCmdDataEncoder(CmdDataEncoder):
    """Encode Security commands."""

    def __init__(self, *args, **kwargs):
        self.cmd_ids = defs.CustomCommandIdCollection
        super(CustomCmdDataEncoder, self).__init__(*args, **kwargs)

    def encode_custom_get_cmd(self):
        pass

    def encode_custom_set_cmd(self):
        return bytearray(self.fields['data'])

    def decode_custom_notify_cmd(self):
        return bytearray(self.fields['data'])

    def decode_custom_resp_cmd(self):
        return bytearray(self.fields['data'])

class_to_encoder = {
    defs.CommandClass.SECURITY: SecurityCmdDataEncoder,
    defs.CommandClass.CUSTOM: CustomCmdDataEncoder,
}

def get_encoder(cmd_class):
    encoder = class_to_encoder.get(cmd_class)
    return encoder
