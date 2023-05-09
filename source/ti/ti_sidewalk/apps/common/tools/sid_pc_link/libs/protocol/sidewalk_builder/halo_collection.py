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

from . import utils


class Collection(object):
    """
    Class based container

    Implements has_class method which raises an exception if a given value
    doesn't exist among uppercase class attributes.
    """
    @classmethod
    def has_value(cls, val):
        attrs = (v for k, v in list(cls.__dict__.items()) if k.isupper())
        if val not in attrs:
            raise ValueError(
                'Value `{}` not found in {}'.format(val, cls.__name__))

    @classmethod
    def get_name_by_value(cls, val, default=None):
        v_2_k_dict = {v: k for k, v in list(cls.__dict__.items()) if k.isupper()}
        return v_2_k_dict.get(val, default)


class CommandClass(Collection):
    HALO_MANAGEMENT = 0x0
    ALERT = 0x1
    LIGHT = 0x2
    MOTION_SENSOR_PIR = 0x3
    LIGHT_SENSOR = 0x4
    SECURITY = 0x5
    BEAMS_MODES = 0x10
    BATTERY = 0x6
    CUSTOM = 0x8
    HALO_LINK_METRICS = 0xB
    HALO_GATEWAY = 0xFF
    HALO_LOGGING = 0xA


class DestinationFormat(Collection):
    SINGLE_DEVICE = 0x1
    GROUP = 0x3


class LightCommandIdCollection(Collection):
    TURN_ON = 0x1
    RESP_TURN_ON = 0x3
    NOTIFY_TURN_ON = 0x2
    TURN_OFF = 0x5
    NOTIFY_TURN_OFF = 0x6
    RESP_TURN_OFF = 0x7
    SET_TIMEOUT = 0x9
    RESP_TIMEOUT = 0xB
    SET_BRIGHTNESS_ON_MOTION = 0xD
    RESP_SET_BRIGHTNESS_ON_MOTION = 0xF
    CUSTOM_CMD = 0x1D


class BeamsModesCommandIdCollection(Collection):
    TURN_ON_D2D_MODE = 0x9
    RESP_TURN_ON_D2D_MODE = 0xB
    TURN_OFF_D2D_MODE = 0xD
    RESP_TURN_OFF_D2D_MODE = 0xF


class AlertCommandIdCollection(Collection):
    MOTION_PIR_STARTED = 0x2
    MOTION_PIR_STOPPED = 0x6
    LOW_BATTERY = 0xA
    MOTION_IN_PROGRESS = 0xE


class BatteryCommandIdCollection(Collection):
    GET_VOLTAGE = 0x0
    RESP_VOLTAGE = 0x3
    NOTIFY_VOLTAGE = 0x2
    GET_LEVEL = 0x4
    RESP_LEVEL = 0x7
    NOTIFY_LEVEL = 0x6
    GET_USAGE_REPORT = 0x8
    RESP_USAGE_REPORT = 0xB
    NOTIFY_USAGE_REPORT = 0xA


class LightSensorCommandIdCollection(Collection):
    ENABLE = 0x1
    RESP_ENABLE = 0x3
    DISABLE = 0x5
    RESP_DISABLE = 0x7
    SET_THRESHOLD = 0x9
    GET_THRESHOLD = 0x8
    RESP_THRESHOLD = 0xB


class MotionSensorCommandIdCollection(Collection):
    ENABLE_BRIGHTNESS = 0x29
    ENABLE = 0x1
    RESP_ENABLE = 0x3
    DISABLE = 0x5
    RESP_DISABLE = 0x7
    SET_SENSITIVITY = 0x9
    GET_SENSITIVITY = 0x8
    RESP_SENSITIVITY = 0xB


class ManagementCommandIdCollection(Collection):
    START_DFU = 0x415
    RESP_START_DFU = 0x417
    SET_P2P_CHNL = 0x401
    GET_P2P_CHNL = 0x400
    RESP_P2P_CHNL = 0x403
    GET_FW_VERSION = 0x404
    RESP_FW_VERSION = 0x407
    GET_SERIAL_NUM = 0x408
    RESP_SERIAL_NUM = 0x40B
    GET_DEV_ID = 0x444
    RESP_DEV_ID = 0x447
    SET_PAN_ID = 0x40D
    GET_PAN_ID = 0x40C
    RESP_PAN_ID = 0x40F
    SET_GROUP_NUM = 0x429
    GET_GROUP_NUM = 0x428
    RESP_GROUP_NUM = 0x42B
    SET_P2P_BROADCAST_KEY = 0x411
    GET_P2P_BROADCAST_KEY = 0x410
    RESP_P2P_BROADCAST_KEY = 0x413
    SET_P2P_UNICAST_KEY = 0x419
    GET_P2P_UNICAST_KEY = 0x418
    RESP_P2P_UNICAST_KEY = 0x41B
    START_LQ_TEST = 0x43D
    STOP_LQ_TEST = 0x441
    RESP_STOP_LQ_TEST = 0x443
    GET_STATUS = 0x18
    RESP_STATUS = 0x1B
    NOTIFY_KEEP_ALIVE = 0x6
    SET_KA_INTERVAL = 0x439
    GET_KA_INTERVAL = 0x438
    RESP_KA_INTERVAL = 0x43B
    SET_TIME = 0x421
    GET_TIME = 0x420
    RESP_TIME = 0x423
    GET_RESET_INFO = 0x41C
    RESP_RESET_INFO = 0x41F
    NOTIFY_RESET_INFO = 0x41E
    FACTORY_RESET = 0x449
    RESP_FACTORY_RESET = 0x44B
    SET_RF_POWER = 0x44D
    GET_RF_POWER = 0x44C
    RESP_RF_POWER = 0x44F
    GET_PAIRING_STATE = 0x488
    SET_PAIRING_STATE = 0x489
    RESP_PAIRING_STATE = 0x48B
    GET_FIRMWARE_OTA_PARAMETERS = 0x48C
    RESP_FIRMWARE_OTA_PARAMETERS = 0x48F
    GET_PROTOCOL_VERSION = 0x484
    RESP_PROTOCOL_VERSION = 0x487
    GET_BRIDGE_CLOUD_AUTH_TOKEN = 0x450
    RESP_BRIDGE_CLOUD_AUTH_TOKEN = 0x453
    VENDOR_DFU_PKT = 0x495
    RNET_RESP_VENDOR_DFU_PKT = 0x497
    INJECT_FAKE_CMD = 0x855
    NOTIFY_FAKE_CMD = 0x856
    RESP_FAKE_CMD = 0x857
    PING = 0x3FE
    PONG = 0x3FF


class HaloLoggingCommandIdCollection(Collection):
    SNIFFER_CFG = 0x1
    RESP_SNIFFER_CFG = 0x3
    NOTIFY_SNIFFER_RX_PKT = 0xA
    ENABLE_DEVICE_LOGS = 0x11


class HaloLinkMetricsCommandIdCollection(Collection):
    GET_P2P_METRICS = 0x4
    RESP_P2P_METRICS = 0x7
    NOTIFY_P2P_METRICS = 0x6
    NOTIFY_CMD_LATENCY = 0xA
    GET_SCHED_METRICS = 0x10
    RESP_SCHED_METRICS = 0x13
    NOTIFY_SCHED_METRICS = 0x12
    GET_NOISE_METRICS = 0x14
    RESP_NOISE_METRICS = 0x17
    NOTIFY_NOISE_METRICS = 0x16
    GET_LDR_METRICS = 0x18
    RESP_LDR_METRICS = 0x1B
    NOTIFY_LDR_METRICS = 0x1A


class HaloGatewayCommandIdCollection(Collection):
    NOTIFY_RX_PKT = 0x2
    SEND_PKT = 0x5
    RESP_SEND_PKT = 0x7
    NOTIFY_RX_DFU_PKT = 0xA
    SEND_DFU_PKT = 0xD
    SET_DFU_PARAMS = 0x11


class SecurityCommandIdCollection(Collection):
    PROV_INIT_REQ = 0x11
    RESP_PROV_INIT_REQ = 0x13
    NOTIFY_PROV_INIT_SIG = 0x16
    PROV_INIT_SIG = 0x19
    RESP_PROV_INIT_SIG = 0x1b
    GET_ENCRYPTION_CAPABILITIES = 0x42C
    RESP_ENCRYPTION_CAPABILITIES = 0x42F
    CHECK_CERT_SERIAL = 0x431
    RESP_CHECK_CERT_SERIAL = 0x433
    GET_DEVICE_CERT_COUNT_NONCE = 0x454
    RESP_DEVICE_CERT_COUNT_NONCE = 0x457
    GET_DEVICE_SERIAL_PUK = 0x458
    RESP_DEVICE_SERIAL_PUK = 0x45B
    GET_DEVICE_SIG = 0x45C
    RESP_DEVICE_SIG = 0x45F
    GET_MODEL_SERIAL_PUK = 0x460
    RESP_MODEL_SERIAL_PUK = 0x463
    GET_MODEL_SIG = 0x464
    RESP_MODEL_SIG = 0x467
    SET_REMOTE_NONCE_ECDH_PUK_CERT_COUNT = 0x469
    RESP_REMOTE_NONCE_ECDH_PUK_CERT_COUNT = 0x46B
    SET_REMOTE_ECDH_SIG = 0x46D
    RESP_REMOTE_ECDH_SIG = 0x46F
    SET_REMOTE_CERT_SERIAL_PUK = 0x471
    RESP_REMOTE_CERT_SERIAL_PUK = 0x473
    SET_REMOTE_SIG = 0x475
    RESP_REMOTE_SIG = 0x477
    GET_DEVICE_ECDH_PUK = 0x478
    RESP_DEVICE_ECDH_PUK = 0x47B
    GET_DEVICE_ECDH_SIG = 0x47C
    RESP_DEVICE_ECDH_SIG = 0x47F
    HAND_SHAKE = 0x435
    RESP_HAND_SHAKE = 0x437
    TRIGGER_JOIN_NETWORK = 0x481
    READ_JOIN_NETWORK_STATUS = 0x480
    RESP_JOIN_NETWORK = 0x483
    NOTIFY_JOIN_NETWORK_STATUS = 0x482
    SECURITY_ERROR = 0x802


class ResponseStatusCode(Collection):
    SUCCESS = 0x00
    INTERNAL_ERROR = 0x03
    NO_MEMORY = 0x04
    CMD_NOT_SUPPORTED = 0x06
    INSUFFICIENT_DATA = 0x09
    MISSING_FLEX_FIELDS = 0x0A
    INVALID_DATA = 0x0B
    TIMEOUT = 0x0D
    HARDWARE_ERROR = 0x19
    SECURITY_ERROR = 0x1A
    VERSION_MISMATCH = 0x1B


class CustomCommandIdCollection(Collection):
    CUSTOM_GET_CMD = 0x0
    CUSTOM_SET_CMD = 0x1
    CUSTOM_NOTIFY_CMD = 0x2
    CUSTOM_RESP_CMD = 0x3


class CommandId(object):
    light = LightCommandIdCollection
    alert = AlertCommandIdCollection
    light_sensor = LightSensorCommandIdCollection
    motion_sensor = MotionSensorCommandIdCollection
    halo_management = ManagementCommandIdCollection
    security = SecurityCommandIdCollection
    beams_modes = BeamsModesCommandIdCollection
    battery = BatteryCommandIdCollection
    halo_gateway = HaloGatewayCommandIdCollection
    halo_logging = HaloLoggingCommandIdCollection
    custom = CustomCommandIdCollection


def get_names(cmd_cls, cmd_id):
    if isinstance(cmd_cls, bytearray):
        cmd_cls = utils.bytearray_to_int(cmd_cls)
    if isinstance(cmd_id, bytearray):
        cmd_id = utils.bytearray_to_int(cmd_id)

    cc = CommandClass
    cmd_cls_name = cc.get_name_by_value(cmd_cls, 'UNKNOWN_CMD_CLASS')
    cmd_id_name = None
    IdCollection = {
        cc.HALO_MANAGEMENT: ManagementCommandIdCollection,
        cc.ALERT: AlertCommandIdCollection,
        cc.LIGHT: LightCommandIdCollection,
        cc.MOTION_SENSOR_PIR: MotionSensorCommandIdCollection,
        cc.LIGHT_SENSOR: LightSensorCommandIdCollection,
        cc.SECURITY: SecurityCommandIdCollection,
        cc.BEAMS_MODES: BeamsModesCommandIdCollection,
        cc.BATTERY: BatteryCommandIdCollection,
        cc.HALO_LINK_METRICS: HaloLinkMetricsCommandIdCollection,
        cc.HALO_GATEWAY: HaloGatewayCommandIdCollection,
        cc.HALO_LOGGING: HaloLoggingCommandIdCollection,
        cc.CUSTOM: CustomCommandIdCollection
    }
    IdCollection = IdCollection.get(cmd_cls)
    if IdCollection is None:
        return 'UNKNOWN_CMD_CLASS', 'UNKNOWN_CMD_ID'
    if IdCollection:
        cmd_id_name = IdCollection.get_name_by_value(cmd_id, 'UNKNOWN_CMD_ID')
    return cmd_cls_name, cmd_id_name
