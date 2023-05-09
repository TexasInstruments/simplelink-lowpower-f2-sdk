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

import threading
import queue
import time
import logging

from .manager import BluetoothManager
from ..ble_config import *
from ..exceptions import *
from ..device_descriptor import BluetoothDeviceDescriptor
from .device import BluetoothDevice
from ...base_transport import BaseTransport

_logger_name = 'bluetooth_transport'
logger = logging.getLogger(_logger_name)


class BluetoothTransport(BaseTransport):

    def __init__(self, device_descriptor):
        self._init_device_descriptor(device_descriptor)
        self.__manager = BluetoothManager(adapter_name=BLUETOOTH_ADAPTER,
                                          characteristics=self.__characteristics)
        self.__device = None
        self.__tq = queue.Queue()
        self.__send_queue = queue.Queue()
        self.__rx_queue = queue.Queue()
        self.__ama_queue = queue.Queue()
        self.__is_connected = threading.Event()
        self.__thread = threading.Thread(target=BluetoothTransport._ble_event_loop_thread,
                                         args=(self.__manager, self.__tq))
        self.__thread.daemon = True
        self.__tq.put({'type': 'run'})
        self.__thread.start()

    @property
    def manufacturer_data(self):
        if self.__manufacturer_data is not None:
            return self.__manufacturer_data
        return self.__device.manufacturer_data

    def scan(self, timeout=BLUETOOTH_SCAN_TIMEOUT):
        self.__manager.start_discovery()
        logger.info(f'Scanning for {timeout} seconds')
        time.sleep(timeout)
        self.__manager.stop_discovery()
        time.sleep(2)
        discovered_devices = self.__manager.get_discovered_devices()
        return discovered_devices

    def open(self, device_descriptor=None):
        if device_descriptor is not None:
            self._init_device_descriptor(device_descriptor)
        self.__device = BluetoothDevice(mac_address=self.__device_identifier,
                                        characteristics=self.__characteristics,
                                        manager=self.__manager,
                                        notify_callback=self._notify_callback,
                                        rx_callback=self._rx_callback,
                                        tx_callback=self._tx_callback,
                                        connect_callback=self._connect_callback)
        self.__device.connect()
        time.sleep(1)
        if self.__device.is_connected():
            self.__is_connected.set()
        else:
            raise SidDeviceNotConnected('{} not connected'.format(self.__device_identifier))

    def close(self):
        self.__is_connected.clear()
        if self.__device:
            self.__device.disconnect()
        self.__tq.put({'type': 'quit'})

    def _read(self, timeout):
        data = None
        try:
            data = self.__rx_queue.get(True, timeout)
        except queue.Empty:
            pass
        return data

    def read(self):
        data = None
        while True:
            try:
                data = self.__rx_queue.get(True, timeout=1)
            except queue.Empty:
                pass
            break
        self._check_device_connected()
        return data

    def write(self, data):
        self._send(data)

    def _notify_callback(self):
        pass

    def _rx_callback(self, data):
        self.__rx_queue.put(data)

    def _tx_callback(self):
        pass

    def _connect_callback(self, status):
        if status.get('type', None) == 'disconnect_succeeded':
            self.close()
        elif status.get('type', None) == 'connect_succeeded':
            self.__is_connected.set()

    def _send(self, data):
        self._check_device_connected()
        self.__device.send(data)

    def _check_device_connected(self):
        if not self.__is_connected.is_set():
            raise SidDeviceNotConnected('{} not connected'.format(self.__device_identifier))

    @staticmethod
    def _ble_event_loop_thread(_manager, _queue):
        while True:
            _ = _queue.get()
            _ = _['type']
            if _ == 'run':
                try:
                    _manager.run()
                except KeyboardInterrupt:
                    logger.info('Ctrl+C hit, quitting')
                    exit(os.EX_USAGE)
            elif _ == 'quit':
                _manager.stop()
                break
            else:
                pass
        logger.info("Exit BLE Loop")

    def _init_device_descriptor(self, device_descriptor):
        # Check for the device descriptor first
        self.__device_identifier = device_descriptor.device_identifier
        self.__characteristics = device_descriptor.characteristics
        self.__manufacturer_data = device_descriptor.manufacturer_data
