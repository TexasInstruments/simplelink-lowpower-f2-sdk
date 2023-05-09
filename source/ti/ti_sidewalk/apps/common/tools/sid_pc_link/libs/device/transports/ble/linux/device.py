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
import logging
import dbus

from gatt import Device

_logger_name = 'bluetooth_device'
logger = logging.getLogger(_logger_name)


class BluetoothDevice(Device):

    def __init__(self, mac_address, characteristics, manager, managed=True, notify_callback=None,
                 rx_callback=None, tx_callback=None, connect_callback=None):
        super().__init__(mac_address, manager, managed)
        self._notify_callback = notify_callback
        self._rx_callback = rx_callback
        self._tx_callback = tx_callback
        self._connect_callback = connect_callback
        self.__characteristics = characteristics
        self._queue = []
        self._rx_packet = []
        self._pkt_count = 0
        self._in_write = False
        self.mailbox_write = None
        self.mailbox_notify = None

    @property
    def manufacturer_data(self):
        try:
            data = self._properties.Get('org.bluez.Device1', 'ManufacturerData')
            # Convert from dbus to simple hex
            return bytearray(list(data.items())[0][1]).hex()
        except dbus.exceptions.DBusException as e:
            logger.error('[%s] Cannot obtain manufacturer data from device' % self.mac_address)
            return None

    def connect_succeeded(self):
        super().connect_succeeded()
        logger.info("[%s] Connected" % self.mac_address)
        self._connect_callback({'type': 'connect_succeeded'})

    def connect_failed(self, error):
        super().connect_failed(error)
        logger.error("[%s] Connection failed: %s" % (self.mac_address, str(error)))
        self._connect_callback({'type': 'connect_failed'})

    def disconnect_succeeded(self):
        super().disconnect_succeeded()
        logger.warning("[%s] Device Disconnected" % self.mac_address)
        self._connect_callback({'type': 'disconnect_succeeded'})

    def services_resolved(self):
        super().services_resolved()
        logger.info("[%s] Resolved services" % self.mac_address)
        for service in self.services:
            logger.info("[%s]  Service [%s]" % (self.mac_address, service.uuid))
            for characteristic in service.characteristics:
                logger.info("[%s]    Characteristic [%s]" % (self.mac_address, characteristic.uuid))
                if characteristic.uuid == self.__characteristics.get('mailbox_notify_char'):
                    self.mailbox_notify = characteristic
                elif characteristic.uuid == self.__characteristics.get('mailbox_write_char'):
                    self.mailbox_write = characteristic

        if self.mailbox_notify:
            self.mailbox_notify.enable_notifications()

    def characteristic_enable_notifications_succeeded(self, characteristic):
        logger.info("[%s] notifications enabled for [%s]" % (self.mac_address, characteristic.uuid))
        if self._notify_callback:
            self._notify_callback()

    def characteristic_enable_notifications_failed(self, characteristic, error):
        logger.error("[%s] notifications enabled failed for [%s]: %s" % (self.mac_address, characteristic.uuid, error))

    def characteristic_value_updated(self, characteristic, value):
        logger.info("[%s] characteristic value updated  [%s]" % (characteristic, value))
        if characteristic.uuid == self.__characteristics.get('mailbox_notify_char'):
            self.__receiver_handler(value)

    def characteristic_write_value_succeeded(self, characteristic):
        logger.info("[%s] characteristic write succeeded" % characteristic)
        if characteristic.uuid == self.__characteristics.get('mailbox_write_char'):
            self._in_write = False
            self.__trigger_send()

    def send(self, data):
        tx_packet = bytes(data)
        logger.info("Sending: {}".format(binascii.hexlify(data)))
        self._queue.append(tx_packet)
        self._pkt_count += 1
        logger.info("Packet count: {count}".format(count=self._pkt_count))
        self.__trigger_send()

    def __write(self, value):
        self.mailbox_write.write_value(value)

    def __trigger_send(self):
        if not self._in_write and len(self._queue) > 0:
            value = self._queue.pop(0)
            self.__write(value)
            self._in_write = True
        else:
            if self._tx_callback:
                self._tx_callback()

    def __receiver_handler(self, rx_packet_fragment):
        if self._rx_callback:
            self._rx_callback(rx_packet_fragment)
