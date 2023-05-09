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

import math
from binascii import hexlify, unhexlify
from collections import namedtuple

from ..halo_collection import (
    CommandClass, CommandId, DestinationFormat
)

LINE_SEPARATOR = '\r\n'

cmd_details_struct = namedtuple('CmdDetails', ['cls', 'id', 'cmd', 'dst'])


def to_hex(val, fill=1, prefix=''):
    fill *= 2
    return '{0}{1:0{2}x}'.format(prefix, val, fill)


def to_5B_hex(val, prefix='0x'):
    return to_hex(val, 5, prefix)


def to_full_byte_hex(val, prefix='0x'):
    vl = int(math.ceil(math.log(val if val > 1 else 2, 0xFF)))
    return to_hex(val, vl, prefix)


def mark_hex(val):
    return '0x{}'.format(val)


class RnetCliFormatter(object):
    cmd_i = namedtuple('CmdInfo', ['name', 'fmt'])
    CMD_MAP = {
        'cmd_class': cmd_i('U', to_full_byte_hex),
        'cmd_id': cmd_i('T', to_full_byte_hex),
        'data': cmd_i('V', mark_hex),
        'status_code': cmd_i('X', str),
        'dest': cmd_i('D', to_5B_hex),
        'dest_format': cmd_i('A', to_full_byte_hex),
        'app_ack_req': cmd_i('G', str),
        'seqn_sz': cmd_i('H', str),
        'seqn': cmd_i('J', str),
        'seqn_inc': cmd_i('I', str)
    }
    LINE_SEPARATOR = LINE_SEPARATOR

    @classmethod
    def build_cmd(cls, kwargs):
        # always require response
        args = []
        if 'data' in kwargs:
            if len(kwargs['data']) % 2 != 0:
                kwargs['data'] = '0' + kwargs['data']
            data = hexlify(unhexlify(kwargs['data'])[::-1])
            kwargs['data'] = data.decode('ASCII')
            # explicit data length
            data_len = int(len(data) / 2)
            args.extend(['M', str(data_len), 'Y', str(data_len + 1)])
        if 'seqn' in kwargs:
            kwargs['seqn_inc'] = '1'
        for arg, val in list(kwargs.items()):
            if arg in cls.CMD_MAP:
                args.append(cls.CMD_MAP[arg].name)
                args.append(cls.CMD_MAP[arg].fmt(val))
        cmds = ['tx rnet 0']
        while len(args):
            cmds.append('tx rnet_app ' + ' '.join(args[:6]))
            args = args[6:]
        cmds.append('tx rnet 1{}'.format(cls.LINE_SEPARATOR))
        return cls.LINE_SEPARATOR.join(cmds)


class BaseCommandBuilder(object):
    """
    Class to construct commands and validate that entered fields are valid.
    """

    def __init__(self, cmd_class, cmd_format):
        self.command_class = cmd_class
        self.destination_format = DestinationFormat
        self.__set_cmd_formatter(cmd_format)

    def __set_cmd_formatter(self, cmd_format):
        formatter_name = 'formatter_{}'.format(cmd_format)
        if not hasattr(self, formatter_name):
            raise AttributeError(
                "Command formater `{}` doesn't exist".format(cmd_format))
        self._format_command = getattr(self, formatter_name)

    def _validate_cmd_params(self, kwargs):
        try:
            self.destination_format.has_value(kwargs.get('dest_format'))
            self.command_id.has_value(kwargs.get('cmd_id'))
        except ValueError as e:
            raise ValueError(
                'Entered command cannot be formated:\n\t{}'.format(e))

    def format_command(self, **kwargs):
        self._validate_cmd_params(kwargs)
        kwargs['cmd_class'] = self.command_class
        text = self._format_command(**kwargs)
        return cmd_details_struct(
            self.command_class, kwargs['cmd_id'], text, kwargs['dest'])

    def formatter_rnet_cli(self, **kwargs):
        """Construct command in rnet_cli format."""
        return RnetCliFormatter.build_cmd(kwargs)

    def formatter_test_ringnet(self, cmd_class, cmd_id, data=None,
                               status_code=0, app_ack_req=1, dest_format=None, dest=None,
                               **kwargs):
        """
        Construct the command to test Ring Net protocol over SPI using testRingNet application.
        :param cmd_class:
        :param cmd_id:
        :param dest_format:
        :param dest:
        :param status_code:
        :return:
        """
        command = ["testRingNet",
                   "--class %x" % cmd_class,
                   "--id %x" % cmd_id]

        # Build the command
        if data:
            command.append("--data %s" % data)
        if status_code:
            command.append("--status_code %s" % status_code)
        if dest_format:
            command.append("--dst_frmt %x" % dest_format)
        if dest:
            command.append("--dst %x" % dest)
        if app_ack_req:
            command.append("--app_ack_req %x" % app_ack_req)
        return " ".join(command) + "\r\n"

    def formatter_halo_cli(self, cmd_class, cmd_id, data=None, dest=None, timeout=5, retries=1, **kwargs):
        command = ["halo-cli"]
        if dest:
            command.append("-d %x" % dest)
        command.append("-c %x" % cmd_class)
        command.append("-i %x" % cmd_id)
        if data:
            command.append("%s" % data)
        if timeout:
            command.append("-t %s" % timeout)
        if retries:
            command.append("-r %s" % retries)

        return " ".join(command) + "\r\n"


class LightCommandBuilder(BaseCommandBuilder):

    def __init__(self, cmd_format='halo_cli'):
        super(LightCommandBuilder, self).__init__(CommandClass.LIGHT,
                                                  cmd_format)
        self.command_id = CommandId.light

    def get_light_on(self, dest_format, dest_id, data=0, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.TURN_ON,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=to_hex(data, 2),
                                   timeout=timeout,
                                   retries=retries)

    def get_light_off(self, dest_format, dest_id, data=0, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.TURN_OFF,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=to_hex(data, 2),
                                   timeout=timeout,
                                   retries=retries)

    def set_light_timeout(self, dest_format, dest_id, time, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.SET_TIMEOUT,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=to_hex(time, 2),
                                   timeout=timeout,
                                   retries=retries)

    def set_light_brightness(self, dest_format, dest_id, brightness, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.SET_BRIGHTNESS_ON_MOTION,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=to_hex(brightness, 2),
                                   timeout=timeout,
                                   retries=retries)


class HaloManagementCommandBuilder(BaseCommandBuilder):

    def __init__(self, cmd_format='halo_cli'):
        super(HaloManagementCommandBuilder, self).__init__(
            CommandClass.HALO_MANAGEMENT, cmd_format)
        self.command_id = CommandId.halo_management

    def start_dfu(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.START_DFU,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def set_p2p_chnl(self, dest_format, dest_id, channel, timeout=3, retries=1):
        hex_channel = to_hex(channel, 1)
        return self.format_command(cmd_id=self.command_id.SET_P2P_CHNL,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=hex_channel,
                                   timeout=timeout,
                                   retries=retries)

    def get_p2p_chnl(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_P2P_CHNL,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def get_fw_version(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_FW_VERSION,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def get_serial_num(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_SERIAL_NUM,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def get_dev_id(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_DEV_ID,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def set_pan_id(self, dest_format, dest_id, pan_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.SET_PAN_ID,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=to_hex(pan_id, 5),
                                   timeout=timeout,
                                   retries=retries)

    def get_pan_id(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_PAN_ID,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def set_group_num(self, dest_format, dest_id, group_num, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.SET_GROUP_NUM,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=to_hex(group_num, 1),
                                   timeout=timeout,
                                   retries=retries)

    def get_group_num(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_GROUP_NUM,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def set_p2p_broadcast_key(self, dest_format, dest_id, p2p_broadcast_key, timeout=3, retries=1):
        key = '{0:032X}'.format(p2p_broadcast_key)
        if len(key) != 32:
            raise ValueError('`p2p_broadcast_key` should have 32 chars')
        return self.format_command(cmd_id=self.command_id.SET_P2P_BROADCAST_KEY,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=key,
                                   timeout=timeout,
                                   retries=retries)

    def get_p2p_broadcast_key(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_P2P_BROADCAST_KEY,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def set_p2p_unicast_key(self, dest_format, dest_id, p2p_unicast_key, timeout=3, retries=1):
        key = '{0:032X}'.format(p2p_unicast_key)
        if len(key) != 32:
            raise ValueError('`p2p_broadcast_key` should have 32 chars')
        return self.format_command(cmd_id=self.command_id.SET_P2P_UNICAST_KEY,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=key,
                                   timeout=timeout,
                                   retries=retries)

    def get_p2p_unicast_key(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_P2P_UNICAST_KEY,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def start_link_quality_test(self, dest_format, dest_id, test_role, direction, transmit_interval):
        validation_conds = (
            test_role in range(3),
            direction in range(2),
            0 <= transmit_interval <= 0xFFFF
        )
        if not all(validation_conds):
            raise ValueError(
                'Wrong argument value for start_link_quality_test')
        data = '{0:02X}{1:04X}'.format(
            test_role << 1 | direction, transmit_interval)
        return self.format_command(cmd_id=self.command_id.START_LQ_TEST,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=data)

    def stop_link_quality_test(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.STOP_LQ_TEST,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def get_status(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_STATUS,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def set_ka_interval(self, dest_format, dest_id, ka_interval, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.SET_KA_INTERVAL,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=to_hex(ka_interval, 4),
                                   timeout=timeout,
                                   retries=retries)

    def get_ka_interval(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_KA_INTERVAL,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def set_time(self, dest_format, dest_id, time, timeout=3, retries=1):
        try:
            int(time, 16)
        except (TypeError, ValueError):
            raise TypeError('`time` param needs to be hex string.')
        return self.format_command(cmd_id=self.command_id.SET_TIME,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=time,
                                   timeout=timeout,
                                   retries=retries)

    def get_time(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_TIME,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def get_reset_info(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_RESET_INFO,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def notify_reset_info(self, dest_format, dest_id, interval, timeout=3, retries=1):
        # Note: Needs more work
        return self.format_command(cmd_id=self.command_id.NOTIFY_RESET_INFO,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=to_hex(interval, 5),
                                   timeout=timeout,
                                   retries=retries)

    def factory_reset(self, dest_format, dest_id, _type, timeout=3, retries=1):
        if _type == 'soft':
            type_bit = 0
        if _type == 'hard':
            type_bit = 1
        # needs more work
        return self.format_command(cmd_id=self.command_id.FACTORY_RESET,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=to_hex(type_bit, 1),
                                   timeout=timeout,
                                   retries=retries)

    def set_rf_power(self, dest_format, dest_id, rf_power, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.SET_RF_POWER,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=to_hex(rf_power, 1),
                                   timeout=timeout,
                                   retries=retries)

    def get_rf_power(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_RF_POWER,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def get_bridge_cloud_auth_token(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_BRIDGE_CLOUD_AUTH_TOKEN,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def get_protocol_version(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_PROTOCOL_VERSION,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def get_pairing_state(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_PAIRING_STATE,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def set_pairing_state(self, dest_format, dest_id, pairing_state, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.SET_PAIRING_STATE,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=to_hex(pairing_state, 1),
                                   timeout=timeout,
                                   retries=retries)

    def get_firmware_ota_parameters(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_FIRMWARE_OTA_PARAMETERS,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)

    def inject_fake_cmd(self, dest_format, dest_id, delay, rand_offset,
                        seqn_sz, seqn):
        assert -1 < delay < 0xFFFF, 'Ivalid delay value'
        assert -1 < rand_offset < 0xFFFF, 'Ivalid rand_offset value'
        data = '{:04X}{:04X}'.format(delay, rand_offset)
        return self.format_command(
            cmd_id=self.command_id.INJECT_FAKE_CMD, dest_format=dest_format,
            dest=dest_id, data=data, seqn_sz=seqn_sz, seqn=seqn)


class AlertCommandBuilder(BaseCommandBuilder):

    def __init__(self, cmd_format='halo_cli'):
        super(AlertCommandBuilder, self).__init__(
            CommandClass.ALERT, cmd_format)
        self.command_id = CommandId.alert

    def event_motion_pir(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.MOTION_PIR_STARTED,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)


class MotionCommandBuilder(BaseCommandBuilder):

    def __init__(self, cmd_format='halo_cli'):
        super(MotionCommandBuilder, self).__init__(
            CommandClass.MOTION_SENSOR_PIR, cmd_format)
        self.command_id = CommandId.motion_sensor

    def set_motion_detection_on(self, dest_format, dest_id, data=None, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.ENABLE,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=data,
                                   timeout=timeout,
                                   retries=retries)

    def set_motion_detection_off(self, dest_format, dest_id, data=None, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.DISABLE,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=data,
                                   timeout=timeout,
                                   retries=retries)

    def set_motion_sensitivity(self, dest_format, dest_id, motion_level, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.SET_SENSITIVITY,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=to_hex(motion_level),
                                   timeout=timeout,
                                   retries=retries)

    def get_motion_sensitivity(self, dest_format, dest_id, timeout=3, retries=1):
        return self.format_command(cmd_id=self.command_id.GET_SENSITIVITY,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   timeout=timeout,
                                   retries=retries)


class SecurityCommandBuilder(BaseCommandBuilder):
    """Command builer for Security class."""

    def __init__(self, cmd_format='halo_cli'):
        super(SecurityCommandBuilder, self).__init__(
            CommandClass.SECURITY, cmd_format)
        self.command_id = CommandId.security

    def get_cert_serial(self, dest_format, dest_id, data=None, timeout=3, retries=1):
        """Format get cert serial command."""
        return self.format_command(cmd_id=self.command_id.CHECK_CERT_SERIAL,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=data,
                                   timeout=timeout,
                                   retries=retries)

    def get_device_cert_count_nonce(self, dest_format, dest_id, data=None, timeout=3, retries=1):
        """Get current device cert count nonce."""
        return self.format_command(cmd_id=self.command_id.GET_DEVICE_CERT_COUNT_NONCE,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=data,
                                   timeout=timeout,
                                   retries=retries)

    def get_device_serial_puk(self, dest_format, dest_id, data=None, timeout=3, retries=1):
        """Get device serial puk."""
        return self.format_command(cmd_id=self.command_id.GET_DEVICE_SERIAL_PUK,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=data,
                                   timeout=timeout,
                                   retries=retries)

    def get_encryption_capabilities(self, dest_format, dest_id, data=None, timeout=3, retries=1):
        """Format get encryption capabilities command."""
        return self.format_command(cmd_id=self.command_id.GET_ENCRYPTION_CAPABILITIES,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=data,
                                   timeout=timeout,
                                   retries=retries)

    def get_device_sig(self, dest_format, dest_id, data=None, timeout=3, retries=1):
        """Get device sig."""
        return self.format_command(cmd_id=self.command_id.GET_DEVICE_SIG,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=data,
                                   timeout=timeout,
                                   retries=retries)

    def get_model_serial_puk(self, dest_format, dest_id, data=None, timeout=3, retries=1):
        """Get current device model serial puk."""
        return self.format_command(cmd_id=self.command_id.GET_MODEL_SERIAL_PUK,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=data,
                                   timeout=timeout,
                                   retries=retries)

    def get_model_sig(self, dest_format, dest_id, data=None, timeout=3, retries=1):
        """Format get model sig command."""
        return self.format_command(cmd_id=self.command_id.GET_MODEL_SIG,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=data,
                                   timeout=timeout,
                                   retries=retries)

    def get_device_ecdh_sig(self, dest_format, dest_id, data=None, timeout=3, retries=1):
        """Format get device ecdh sig command."""
        return self.format_command(cmd_id=self.command_id.GET_DEVICE_ECDH_SIG,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=data,
                                   timeout=timeout,
                                   retries=retries)

    def get_device_ecdh_puk(self, dest_format, dest_id, data=None, timeout=3, retries=1):
        """Format get device ecdh puk command."""
        return self.format_command(cmd_id=self.command_id.GET_DEVICE_ECDH_PUK,
                                   dest_format=dest_format,
                                   dest=dest_id,
                                   data=data,
                                   timeout=timeout,
                                   retries=retries)
