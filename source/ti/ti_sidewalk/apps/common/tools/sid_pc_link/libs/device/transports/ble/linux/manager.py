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

from gatt import DeviceManager
from .device import BluetoothDevice
from ..device_descriptor import BluetoothDeviceDescriptor



class BluetoothManager(DeviceManager):

    def __init__(self, adapter_name, characteristics):
        self.__characteristics = characteristics
        self.__discovered_devices = set()
        super().__init__(adapter_name=adapter_name)

    def make_device(self, mac_address):
        return BluetoothDevice(mac_address=mac_address, manager=self,
                               characteristics=self.__characteristics)

    def device_discovered(self, device):
        device_descriptor = BluetoothDeviceDescriptor(
            device_identifier=str(device.mac_address).upper(),
            characteristics=None,
            manufacturer_data=device.manufacturer_data
        )
        self.__discovered_devices.add(device_descriptor)

    def get_discovered_devices(self):
        return self.__discovered_devices
