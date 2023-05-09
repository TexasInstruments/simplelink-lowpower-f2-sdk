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

from datetime import datetime, timedelta

from . import halo_collection as defs
from .utils import (
    uint_to_int_2_compl, bytearray_to_int, bytarray_to_hex_str
)


class CmdDataDecoder(object):
    def __init__(self, cmd_id, cmd_data_wrapped, is_unified_asl):
        self.cmd_id = cmd_id
        if not is_unified_asl:
            self.cmd_data, self.status_code = self._parse_cmd_data_wrapped(
                cmd_data_wrapped)
        else:
            self.cmd_data = cmd_data_wrapped
        self.data = self.decode()

    def decode(self):
        if not len(self.cmd_data):
            return {}
        cmd_id_name = self.cmd_ids.get_name_by_value(self.cmd_id)
        method_name = 'decode_{}'.format(cmd_id_name).lower()
        if hasattr(self, method_name):
            return getattr(self, method_name)()
        else:
            return {}

    def _parse_cmd_data_wrapped(self, cmd_data_wrapped):
        cmd_data = []
        status_code = None
        idx = 0
        status_code_included = cmd_data_wrapped[idx] >> 7
        idx += 1
        if status_code_included:
            status_code = cmd_data_wrapped[idx]
            idx += 1
        cmd_data_len = cmd_data_wrapped[idx]
        if cmd_data_len:
            idx += 1
            cmd_data = cmd_data_wrapped[idx:idx + cmd_data_len]
        return cmd_data, status_code

    def _decode_time(self, time_raw):
        seconds = bytearray_to_int(time_raw[0:4])
        fraction_time = time_raw[6]
        units = time_raw[7] >> 6
        uncertainty = ((time_raw[7] & 0b111111) << 8) | time_raw[8]
        random_key = bytearray_to_int(time_raw[8:12])
        ts_resolution = [30500, 97e+4, 3125e+4, 25e+7][units]
        microseconds = bytearray_to_int(time_raw[4:6]) * ts_resolution
        timestamp = (
            datetime.fromtimestamp(seconds) +
            timedelta(microseconds=microseconds)
        )
        return {
            'seconds': seconds,
            'fraction_time': fraction_time,
            'units': units,
            'uncertainty': uncertainty,
            'random_key': random_key,
            'ts_resolution': ts_resolution,
            'microseconds': microseconds,
            'timestamp': timestamp
        }


class AlertCmdDataDecoder(CmdDataDecoder):
    def __init__(self, *args, **kwargs):
        self.cmd_ids = defs.AlertCommandIdCollection
        super(AlertCmdDataDecoder, self).__init__(*args, **kwargs)

    def _decode_motion_pir(self):
        return {'ali': self.cmd_data[0] >> 6}

    def decode_motion_pir_started(self):
        return self._decode_motion_pir()

    def decode_motion_pir_stopped(self):
        return self._decode_motion_pir()

    def decode_low_battery(self):
        return {'battery_level': self.cmd_data[0]}

    def decode_motion_in_progress(self):
        return self._decode_motion_pir()


class BatteryCmdDataDecoder(CmdDataDecoder):
    def __init__(self, *args, **kwargs):
        self.cmd_ids = defs.BatteryCommandIdCollection
        super(BatteryCmdDataDecoder, self).__init__(*args, **kwargs)

    def _decode_voltage(self):
        return {'battery_voltage': bytearray_to_int(self.cmd_data[:2])}

    def _decode_level(self):
        return {'battery_level': bytearray_to_int(self.cmd_data[:1])}

    def _decode_usage_report(self):
        return {
            'tx_time': bytearray_to_int(self.cmd_data[:2]),
            'rx_time': bytearray_to_int(self.cmd_data[2:4]),
            'ble_advertising_time': bytearray_to_int(self.cmd_data[4:7]),
            'ble_connection_time': bytearray_to_int(self.cmd_data[7:9]),
            'led_on_time': bytearray_to_int(self.cmd_data[9:11])
        }

    def resp_voltage(self):
        return self._decode_voltage()

    def notify_voltage(self):
        return self._decode_voltage()

    def resp_level(self):
        return self._decode_level()

    def notify_level(self):
        return self._decode_level()

    def resp_usage_report(self):
        return self._decode_usage_report()

    def notify_usage_report(self):
        return self._decode_usage_report()


class LightSensorCmdDataDecoder(CmdDataDecoder):
    def __init__(self, *args, **kwargs):
        self.cmd_ids = defs.LightSensorCommandIdCollection
        super(LightSensorCmdDataDecoder, self).__init__(*args, **kwargs)

    def decode_resp_threshold(self):
        return {'threshold': self.cmd_data[0]}


class ManagementCmdDataDecoder(CmdDataDecoder):
    def __init__(self, *args, **kwargs):
        self.cmd_ids = defs.ManagementCommandIdCollection
        super(ManagementCmdDataDecoder, self).__init__(*args, **kwargs)

    def decode_resp_p2p_chnl(self):
        return {'channel_number': self.cmd_data[0]}

    def decode_resp_fw_version(self):
        return {'fw_version': '{}.{}.{}-{}'.format(*self.cmd_data)}

    def decode_resp_serial_num(self):
        return {'serial_number': self.cmd_data.decode('utf-8')}

    def decode_get_dev_id(self):
        return {}

    def decode_resp_dev_id(self):
        return {'dev_id': bytearray_to_int(self.cmd_data)}

    def decode_resp_pan_id(self):
        return {'pan_id': bytearray_to_int(self.cmd_data)}

    def decode_resp_group_num(self):
        return {'group_num': self.cmd_data[0]}

    def decode_resp_p2p_broadcast_key(self):
        return {'p2p_broadcast_key': bytearray_to_int(self.cmd_data)}

    def decode_resp_p2p_unicast_key(self):
        return {'p2p_unicast_key': bytearray_to_int(self.cmd_data)}

    def decode_resp_stop_lq_test(self):
        return {}

    def decode_resp_status(self):
        data = {}
        curr_byte_n = 0
        data['rssi'] = uint_to_int_2_compl(self.cmd_data[curr_byte_n])
        curr_byte_n += 1
        data['voltage'] = bytearray_to_int(
            self.cmd_data[curr_byte_n:curr_byte_n + 2])
        curr_byte_n += 2
        data['app_status_type'] = self.cmd_data[curr_byte_n]
        curr_byte_n += 1
        if data['app_status_type'] == 0x1:
            curr_byte = self.cmd_data[curr_byte_n]
            data['led_on'] = bool((curr_byte >> 3) & 0x1)
            data['motion_enabled'] = bool((curr_byte >> 2) & 0x1)
            data['glow_mode_on'] = bool((curr_byte >> 1) & 0x1)
            data['asl_enabled'] = bool(curr_byte & 0x1)
            curr_byte_n += 1
            data['timeout'] = bytearray_to_int(
                self.cmd_data[curr_byte_n: curr_byte_n + 4])
            curr_byte_n += 4
            data['brightness'] = self.cmd_data[curr_byte_n]
            curr_byte_n += 1
            data['motion_sensitivity'] = self.cmd_data[curr_byte_n]
            curr_byte_n += 1
            data['asl_threshold'] = self.cmd_data[curr_byte_n]
        return data

    def decode_resp_ka_interval(self):
        return {'ka_interval': bytearray_to_int(self.cmd_data)}

    def decode_resp_time(self):
        return self._decode_time(self.cmd_data)

    def decode_resp_reset_info(self):
        return {
            'version': self.cmd_data[0],
            'reset_reason': self.cmd_data[1],
            'program_counter': bytearray_to_int(self.cmd_data[2:5])
        }

    def decode_notify_reset_info(self):
        return {
            'version': self.cmd_data[0],
            'reset_reason': self.cmd_data[1],
            'program_counter': bytearray_to_int(self.cmd_data[2:5])
        }

    def decode_resp_factory_reset(self):
        return {}

    def decode_resp_rf_power(self):
        return {'rf_power': self.cmd_data[0]}

    def decode_resp_brge_cld_auth_tokn(self):
        return {'brigde_auth_token': bytarray_to_hex_str(self.cmd_data)}

    def decode_resp_pairing_state(self):
        return {'pairing_state': self.cmd_data[0]}

    def decode_resp_firmware_ota_parameters(self):
        data = {}
        cb_n = 0
        data['release_tag_len'] = self.cmd_data[cb_n]
        cb_n += 1
        if data['release_tag_len']:
            data['release_tag'] = (
                self.cmd_data[cb_n:cb_n + data['release_tag_len']]
                .decode('utf-8')
            )
            cb_n += data['release_tag_len']
        data['ota_pool_len'] = self.cmd_data[cb_n]
        cb_n += 1
        if data['ota_pool_len']:
            data['ota_pool'] = (
                self.cmd_data[cb_n:cb_n + data['ota_pool_len']]
                .decode('utf-8')
            )
            cb_n += data['ota_pool_len']
        data['build_variant'] = self.cmd_data[cb_n]
        return data

    def decode_resp_protocol_version(self):
        return {'protocol_version': self.cmd_data[0]}

    def decode_vendor_dfu_pkt(self):
        return {'vendor_dfu_pkt': self.cmd_data}

    def decode_rnet_resp_vendor_dfu_pkt(self):
        return {'vendor_dfu_pkt': self.cmd_data}

    def decode_ping(self):
        return {
            'ping_num': self.cmd_data[0],
            'rssi': uint_to_int_2_compl(self.cmd_data[1])
        }

    def decode_pong(self):
        return {
            'pong_num': self.cmd_data[0],
            'rssi': uint_to_int_2_compl(self.cmd_data[1])
        }

    def decode_inject_fake_cmd(self):
        return {
            'delay': bytearray_to_int(self.cmd_data[:2]),
            'rand_offset': bytearray_to_int(self.cmd_data[2:4])
        }


class LightCmdDataDecoder(CmdDataDecoder):
    def __init__(self, *args, **kwargs):
        self.cmd_ids = defs.LightCommandIdCollection
        super(LightCmdDataDecoder, self).__init__(*args, **kwargs)

    def _decode_light_state_reason(self):
        val = self.cmd_data[0]
        if val not in list(range(1, 4)):
            return {'reason': 'unsupported: {}'.format(val)}
        reason = [None, 'motion', 'alert', 'command'][val]
        return {'reason': reason}

    def decode_resp_turn_on(self):
        return {}

    def decode_notify_turn_on(self):
        return self._decode_light_state_reason()

    def decode_resp_turn_off(self):
        return {}

    def decode_notify_turn_off(self):
        return self._decode_light_state_reason()

    def decode_resp_set_brightness_on_motion(self):
        return {}


class MotionSensorCmdDataDecoder(CmdDataDecoder):
    def __init__(self, *args, **kwargs):
        self.cmd_ids = defs.MotionSensorCommandIdCollection
        super(MotionSensorCmdDataDecoder, self).__init__(*args, **kwargs)

    def decode_resp_enable(self):
        return {}

    def decode_resp_disable(self):
        return {}

    def decode_resp_sensitivity(self):
        return {'sensitivity': self.cmd_data[0]}


class HaloGatewayCmdDataDecoder(CmdDataDecoder):
    def __init__(self, *args, **kwargs):
        self.cmd_ids = defs.HaloGatewayCommandIdCollection
        super(HaloGatewayCmdDataDecoder, self).__init__(*args, **kwargs)

    def decode_notify_rx_pkt(self):
        return {'pkt': self.cmd_data}

    def decode_notify_rx_dfu_pkt(self):
        return {'flex_dfu_pkt': self.cmd_data}


class HaloLoggingCmdDataDecoder(CmdDataDecoder):
    def __init__(self, *args, **kwargs):
        self.cmd_ids = defs.HaloLoggingCommandIdCollection
        super(HaloLoggingCmdDataDecoder, self).__init__(*args, **kwargs)

    def decode_resp_sniffer_cfg(self):
        data = self.cmd_data[0]
        return {
            'rf_mode': data >> 5,
            'logging_cfg': (data >> 3) & 0b11,
            'logging_link': (data >> 1) & 0x11,
            'enable': bool(data & 0b1),
        }

    def decode_notify_sniffer_rx_pkt(self):
        return {
            'rf_mode': self.cmd_data[0],
            'snr': uint_to_int_2_compl(self.cmd_data[1]),
            'rssi': uint_to_int_2_compl(
                bytearray_to_int(self.cmd_data[2:4]),
                16),
            'ts': self._decode_time(self.cmd_data[4:12])['timestamp'],
            'carrier_offset': bytearray_to_int(self.cmd_data[12:16]),
            'pkt': self.cmd_data[16:]
        }


class SecurityCmdDataDecoder(CmdDataDecoder):
    """Decode responses of Security commands."""
    def __init__(self, *args, **kwargs):
        self.cmd_ids = defs.SecurityCommandIdCollection
        super(SecurityCmdDataDecoder, self).__init__(*args, **kwargs)

    def _decode_join_network(self):
        """Decode join network response."""
        if (self.cmd_data[0] < 1) or (self.cmd_data[0] > 3):
            return {
                'join_network_status': 'unsupported {}'.format(
                    self.cmd_data[0])
            }
        status = [None, 'success', 'failed', 'pending'][self.cmd_data[0]]
        return {
            'join_network_status': status
        }


    def decode_prov_init_req(self):
        """Decode prov init req"""
        return {
            'product_id': bytarray_to_hex_str(self.cmd_data[:4]),
            'manufacturing_vers': self.cmd_data[4],
            "device_id_type": [
                "SIDEWALK_ID",
                "SMSN",
            ][self.cmd_data[5] & 0x80 > 7],
            'algorithm' : [
                            'disabled',
                            'ECDH_25519_ED_25519_AES_CMAC_GCM_128',
                            'ECDH_256R1_ED_25519_AES_CMAC_GCM_128'
                        ][self.cmd_data[5] & 0x7F],
            'nonce': bytarray_to_hex_str(self.cmd_data[6:18]),
            'hash_chop': bytarray_to_hex_str(self.cmd_data[18:23]),
            'cloud_cert':bytarray_to_hex_str(self.cmd_data[23:27]),
            'ed25519_ecdhe_puk':bytarray_to_hex_str(self.cmd_data[27:59])
        }

    def decode_resp_prov_init_sig(self):
        return {}

    def decode_get_encryption_capabilities(self):
        """Decode get encryption capabilities."""
        return {
            "requester_model": self.cmd_data[0]
        }

    def decode_resp_encryption_capabilities(self):
        """Decode received encryption capabilities."""
        return {
            'manufacturing_vers': self.cmd_data[0],
            'algorithm': [
                'disabled',
                'ECDH_25519_ED_25519_AES_CMAC_GCM_128',
                'ECDH_256R1_ED_25519_AES_CMAC_GCM_128'
            ][self.cmd_data[1]]
        }

    def decode_resp_device_cert_count_nonce(self):
        """Decode device cert count nonce."""
        return {
            'frame_count': self.cmd_data[0],
            'cloud_cert_serial_count': self.cmd_data[1],
            'device_cert_count': self.cmd_data[2],
            'cloud_cert_serial': bytarray_to_hex_str(self.cmd_data[3:15]),
            'nonce': bytarray_to_hex_str(self.cmd_data[15:23]),
        }

    def decode_resp_device_serial_puk(self):
        """Decode device serial puk"""
        return {
            'serial_number': bytarray_to_hex_str(self.cmd_data[:5]),
            'device_puk': bytarray_to_hex_str(self.cmd_data[5:37]),
        }

    def decode_resp_device_sig(self):
        """Decode device sig response."""
        return {'device_sig': bytarray_to_hex_str(self.cmd_data)}

    def decode_resp_model_serial_puk(self):
        """Decode model serial puk response."""
        return {
            'serial_number': bytearray_to_int(self.cmd_data[:4]),
            'model_puk': bytarray_to_hex_str(self.cmd_data[4:36]),
        }

    def decode_resp_model_sig(self):
        """Decode model sig response."""
        return {'device_signature': bytarray_to_hex_str(self.cmd_data)}

    def decode_resp_device_ecdh_puk(self):
        """Decode device ecdh puk."""
        return {'device_ecdh_puk': bytarray_to_hex_str(self.cmd_data[:32])}

    def decode_resp_device_ecdh_sig(self):
        """Decode device ecdh sig."""
        return {'device_ecdh_sig': bytarray_to_hex_str(self.cmd_data)}

    def decode_resp_join_network(self):
        """Decode response to join network command."""
        self._decode_join_network()

    def decode_notify_join_network_status(self):
        """Decode join network status response."""
        self._decode_join_network()

    def decode_security_error(self):
        """Decode if any security error."""
        return {
            'type': ['unknown', 'encrypt', 'decrypt', 'authentication',
                     'security'][self.cmd_data[0]],
            'error_code': self.cmd_data[1]
        }


class LinkMetricsCmdDataDecoder(CmdDataDecoder):

    def __init__(self, *args, **kwargs):
        self.cmd_ids = defs.HaloLinkMetricsCommandIdCollection
        super(LinkMetricsCmdDataDecoder, self).__init__(*args, **kwargs)

    def _decode_p2p_metrics(self):
        resp = {
            'total_cads': bytearray_to_int(self.cmd_data[:4]),
            'total_rx_packets': bytearray_to_int(self.cmd_data[4:8]),
            'total_tx_packets': bytearray_to_int(self.cmd_data[8:12]),
            'total_retransmissions': bytearray_to_int(self.cmd_data[12:16]),
            'total_tx_packets_from_other_pans': bytearray_to_int(
                self.cmd_data[16:20]),
            'num_of_channel_busy_events': bytearray_to_int(
                self.cmd_data[20:24]),
            'num_of_crc_errors': bytearray_to_int(self.cmd_data[24:28]),
            'num_of_bad_decodes': bytearray_to_int(self.cmd_data[28:32]),
            'num_of_security_errors': bytearray_to_int(self.cmd_data[32:36]),
            'num_of_decrypt_errors': bytearray_to_int(self.cmd_data[36:40]),
            'num_of_auth_errors': bytearray_to_int(self.cmd_data[40:44]),
        }
        if len(self.cmd_data) > 44:
            resp.update({
                'gcs_errors': bytearray_to_int(
                    self.cmd_data[44:48]),
            })
        return resp

    def decode_resp_p2p_metrics(self):
        return self._decode_p2p_metrics()

    def decode_notify_p2p_metrics(self):
        return self._decode_p2p_metrics()

    def decode_notify_cmd_latency(self):
        return {
            'seq_num': self.cmd_data[0],
            'cmd_class': bytearray_to_int(self.cmd_data[1:3]),
            'cmd_id': bytearray_to_int(self.cmd_data[3:5]),
            'latency_ms': bytearray_to_int(self.cmd_data[5:9]),
        }

    def _decode_sched_metrics(self):
        resp = {
            'num_of_events[0]': bytearray_to_int(self.cmd_data[:4]),
            'num_of_events[1]': bytearray_to_int(self.cmd_data[4:8]),
            'num_of_events[2]': bytearray_to_int(self.cmd_data[8:12]),
            'num_of_events[3]': bytearray_to_int(self.cmd_data[12:16]),
            'num_of_events[4]': bytearray_to_int(self.cmd_data[16:20]),
            'num_of_events[5]': bytearray_to_int(self.cmd_data[20:24]),
            'deficit_conflict[0]': bytearray_to_int(self.cmd_data[24:28]),
            'deficit_conflict[1]': bytearray_to_int(self.cmd_data[28:32]),
            'deficit_conflict[2]': bytearray_to_int(self.cmd_data[32:36]),
            'deficit_conflict[3]': bytearray_to_int(self.cmd_data[36:40]),
            'deficit_conflict[4]': bytearray_to_int(self.cmd_data[40:44]),
            'deficit_conflict[5]': bytearray_to_int(self.cmd_data[40:44]),
            'priority_conflict[0]': bytearray_to_int(self.cmd_data[44:48]),
            'priority_conflict[1]': bytearray_to_int(self.cmd_data[48:52]),
            'priority_conflict[2]': bytearray_to_int(self.cmd_data[52:56]),
            'priority_conflict[3]': bytearray_to_int(self.cmd_data[56:60]),
            'priority_conflict[4]': bytearray_to_int(self.cmd_data[60:64]),
            'priority_conflict[5]': bytearray_to_int(self.cmd_data[64:68]),
            'hdr_process_time[0]': bytearray_to_int(self.cmd_data[68:70]),
            'hdr_process_time[1]': bytearray_to_int(self.cmd_data[70:72]),
            'hdr_process_time[2]': bytearray_to_int(self.cmd_data[72:74]),
            'hdr_process_time[3]': bytearray_to_int(self.cmd_data[74:76]),
            'ldr_process_time[0]': bytearray_to_int(self.cmd_data[76:78]),
            'ldr_process_time[1]': bytearray_to_int(self.cmd_data[78:80]),
            'ldr_process_time[2]': bytearray_to_int(self.cmd_data[80:82]),
            'ldr_process_time[3]': bytearray_to_int(self.cmd_data[82:84]),
        }
        return resp

    def decode_resp_sched_metrics(self):
        return self._decode_sched_metrics()

    def decode_notify_sched_metrics(self):
        return self._decode_sched_metrics()

    def _decode_noise_metrics(self):
        return {'payload': bytarray_to_hex_str(self.cmd_data[:])}

    def decode_resp_noise_metrics(self):
        return self._decode_noise_metrics()

    def decode_notify_noise_metrics(self):
        return self._decode_noise_metrics()

    def _decode_ldr_metrics(self):
        resp = {
            'ldr_rx_error': bytearray_to_int(self.cmd_data[:4]),
            'ldr_rx_pkts': bytearray_to_int(self.cmd_data[4:6]),
            'ldr_tx_pkts': bytearray_to_int(self.cmd_data[6:8]),
            'ldr_tx_fail': bytearray_to_int(self.cmd_data[8:10]),
            'ldr_avg_rssi': bytearray_to_int(self.cmd_data[10:12]),
        }
        return resp

    def decode_resp_ldr_metrics(self):
        return self._decode_ldr_metrics()

    def decode_notify_ldr_metrics(self):
        return self._decode_ldr_metrics()

class_to_decoder = {
    defs.CommandClass.HALO_MANAGEMENT: ManagementCmdDataDecoder,
    defs.CommandClass.LIGHT: LightCmdDataDecoder,
    defs.CommandClass.LIGHT_SENSOR: LightSensorCmdDataDecoder,
    defs.CommandClass.MOTION_SENSOR_PIR: MotionSensorCmdDataDecoder,
    defs.CommandClass.HALO_GATEWAY: HaloGatewayCmdDataDecoder,
    defs.CommandClass.ALERT: AlertCmdDataDecoder,
    defs.CommandClass.BATTERY: BatteryCmdDataDecoder,
    defs.CommandClass.HALO_LOGGING: HaloLoggingCmdDataDecoder,
    defs.CommandClass.SECURITY: SecurityCmdDataDecoder,
    defs.CommandClass.HALO_LINK_METRICS: LinkMetricsCmdDataDecoder,
}


def get_decoder(cmd_class):
    decoder = class_to_decoder.get(cmd_class)
    return decoder
