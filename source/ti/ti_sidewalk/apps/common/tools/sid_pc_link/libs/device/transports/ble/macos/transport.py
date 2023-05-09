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

import logging
import time

from bleak import BleakScanner
from .device import BluetoothDevice
from .manager import BluetoothManager
from ..ble_config import *
from ..exceptions import *
from ...base_transport import BaseTransport
from ..device_descriptor import BluetoothDeviceDescriptor

_logger_name = 'bluetooth_transport'
logger = logging.getLogger(_logger_name)


class BluetoothTransport(BaseTransport):

    def __init__(self, device_descriptor=None):
        # Check for the device descriptor first
        self._init_device_descriptor(device_descriptor)
        self.__is_connected = False
        self.__device = None
        self.__manager = BluetoothManager()
        self.__manager.run()

    @property
    def manufacturer_data(self):
        if self.__manufacturer_data is not None:
            return self.__manufacturer_data
        return self.__device.manufacturer_data

    def scan(self, timeout=BLUETOOTH_SCAN_TIMEOUT):
        logger.info(f'Scanning for {timeout} seconds')
        scanned = self.__manager.await_bleak(
            BleakScanner.discover(
                timeout=timeout,
                adapter=BLUETOOTH_ADAPTER)
        )
        time.sleep(2)
        discovered = set()
        for device in scanned:
            logger.debug(f"Scanned device is {device}")
            data = device.metadata.get('manufacturer_data')
            formatted = None
            if len(data) > 0:
                logger.debug(f"Found scanned data {data}")
                formatted = bytearray(list(data.items())[0][1]).hex()
                logger.debug(f"Hex format data is {formatted}")
            discovered.add(
                BluetoothDeviceDescriptor(
                    device_identifier=device.address,
                    characteristics=None,
                    manufacturer_data=formatted
                )
            )
        return discovered

    def open(self, device_descriptor=None):
        if device_descriptor is not None:
            self._init_device_descriptor(device_descriptor)
        self.__device = BluetoothDevice(self.__device_identifier,
                                        characteristics=self.__characteristics,
                                        manager=self.__manager)
        self.__device.connect()
        # Send AMA connection request messages
        time.sleep(5)
        self.__is_connected = True

    def close(self):
        try:
            self.__is_device_connected()
            self.__device.disconnect()
        except SidTransportNotConnected:
            # Do nothing
            pass
        finally:
            # Stop the manager
            self.__manager.stop()
        self.__is_connected = False

    def read(self):
        self.__is_device_connected()
        return self.__device.read()

    def write(self, data):
        self.__is_device_connected()
        self.__device.write(data)

    def __is_device_connected(self):
        if not self.__is_connected:
            raise SidTransportNotConnected('{} not connected'.format(self.__device))

    def _init_device_descriptor(self, device_descriptor):
        # Check for the device descriptor first
        self.__device_identifier = device_descriptor.device_identifier
        self.__characteristics = device_descriptor.characteristics
        self.__manufacturer_data = device_descriptor.manufacturer_data