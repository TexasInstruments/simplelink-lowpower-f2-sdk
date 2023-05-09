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
import queue
import backoff
import threading
import asyncio
import time

from bleak.exc import BleakError
from bleak import BleakClient
from bleak import discover
from threading import Event
import binascii

from ..exceptions import *
from ..ble_config import *


_logger_name = 'bluetooth_device'
logger = logging.getLogger(_logger_name)


class BluetoothDevice(object):

    def __init__(self, device_identifier=None, characteristics={}, manager=None, **kwargs):
        self.__manufacturer_data = None
        self.__manager = manager
        self.__characteristics = characteristics
        self.__loop = None
        self.__device_identifier = device_identifier
        self.__client = None
        self.__receiver_queue = queue.Queue()
        self.__transmitter_queue = []
        self.__packet_count = 0
        self.__is_connected = Event()
        self.__notify_char_handler = None
        self.__write_char_handler = None
        # Assign the manufacturer data at creation time
        if kwargs.get('manufacturer_data') is not None:
            self.__manufacturer_data = kwargs.get('manufacturer_data')

    def __repr__(self):
        return self.__device_identifier

    def __receiver_callback(self, sender, data):
        if sender == self.__notify_char_handler.handle:
            self.__receiver_queue.put(item=bytes(data), block=False)

    def __disconnected_callback(self, client):
        logger.info('Device {} is disconnected'.format(self))
        self.__is_connected.clear()

    def __resolve_characteristic(self, char_str):
        char_handle = None
        for service in self.__manager.await_bleak(self.__client.get_services()):
            char_handle = service.get_characteristic(char_str)
            if char_handle is not None:
                break
        return char_handle

    def __write(self):
        while len(self.__transmitter_queue) > 0:
            data = self.__transmitter_queue.pop(0)
            self.__manager.await_bleak(self.__client.write_gatt_char(self.__write_char_handler,
                                                                     bytes(data)))

    @property
    def mac_address(self):
        if self.__device_identifier is not None:
            return self.__device_identifier
        else:
            self.__device_identifier = self.__client.address
            return self.__device_identifier

    @property
    def manufacturer_data(self):
        # There can be a manual setter of this property, if set, return the existing data
        if self.__manufacturer_data is not None:
            return self.__manufacturer_data
        else:
            try:
                data = self.__client.device.metadata.get('manufacturer_data')
                # Convert from dbus to simple hex
                if len(data) == 0:
                    return None
                self.__manufacturer_data = bytearray(list(data.items())[0][1]).hex()
                return self.__manufacturer_data
            except AttributeError as e:
                logger.info(e)
                logger.info('Cannot obtain manufacturer data from device')
                return None

    @backoff.on_exception(backoff.constant,
                          SidDeviceNotConnected,
                          max_tries=5,
                          interval=5)
    def connect(self):
        # If the client is already connected, return. Avoid trying to connect again
        if self.__is_connected.is_set():
            return

        # Initialize the client with the given device identifier
        self.__client = BleakClient(
            self.__device_identifier,
            adapter=BLUETOOTH_ADAPTER
        )
        try:
            # And attempt to connect
            self.__manager.await_bleak(self.__client.connect())
            # Register the handlers after connecting
            self.__notify_char_handler = self.__resolve_characteristic(self.__characteristics.get('mailbox_notify_char'))
            self.__write_char_handler = self.__resolve_characteristic(self.__characteristics.get('mailbox_write_char'))
            self.__client.set_disconnected_callback(self.__disconnected_callback)
            self.__manager.await_bleak(self.__client.start_notify(self.__notify_char_handler,
                                                                  self.__receiver_callback))
        except BleakError as err:
            logger.info("Couldn't connect to the device due to: {}".format(err))
            # Try to disconnect
            # Disregard if there is any exception while disconnecting
            # This is to prevent BLE adapter to keep stale connections and try
            # in a clean state before retry
            try:
                self.__manager.await_bleak(self.__client.disconnect())
            except Exception as err:
                logger.info('Device failed to disconnect: {err}')
            raise SidDeviceNotConnected(err)
        # If the device finally connects, set the connection flag
        try:
            # Try first with the is_connected method
            if self.__manager.await_bleak(self.__client.is_connected()):
                self.__is_connected.set()
        except TypeError as err:
            # If we get a TypeError exception, the user has installed bleak>0.10.0 library
            # We need to check for is_connected property
            logger.debug('There was an error while checking if '
                        'the device is connected: {}'.format(err))
            logger.debug('Checking for is_connected property')
            if self.__client.is_connected:
                self.__is_connected.set()

    def read(self, timeout=1, delay=1):
        data = None
        while True:
            try:
                if not self.__is_connected.is_set():
                    raise SidDeviceNotConnected('{} not connected'.format(self))
                time.sleep(delay)
                data = self.__receiver_queue.get(block=False, timeout=timeout)
                break
            except queue.Empty:
                break
        return data

    def write(self, data):
        if not self.__is_connected.is_set():
            raise SidDeviceNotConnected('{} not connected'.format(self))
        packet = bytes(data)
        logger.info("Sending: {data}".format(data=binascii.hexlify(data)))
        self.__transmitter_queue.append(packet)
        self.__packet_count += 1
        logger.info("Packet count: {count}".format(count=self.__packet_count))
        self.__write()

    @backoff.on_exception(backoff.constant,
                          (SidDeviceNotConnected, SidDeviceConnected),
                          max_tries=5,
                          interval=5)
    def disconnect(self):
        if not self.__is_connected.is_set():
            return
        self.__manager.await_bleak(self.__client.disconnect())
        if self.__is_connected.is_set():
            raise SidDeviceConnected('{} cannot be disconnected'.format(self))
