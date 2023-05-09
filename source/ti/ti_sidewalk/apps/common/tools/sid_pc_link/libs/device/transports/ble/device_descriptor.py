import json


class BluetoothDeviceDescriptor:

    def __init__(self, device_identifier=None, characteristics=None, manufacturer_data=None):
        self.__device_identifier = device_identifier
        self.__characteristics = characteristics
        self.__manufacturer_data = manufacturer_data

    @property
    def device_identifier(self):
        return self.__device_identifier

    @property
    def characteristics(self):
        return self.__characteristics

    @property
    def manufacturer_data(self):
        return self.__manufacturer_data

    @device_identifier.setter
    def device_identifier(self, val):
        self.__device_identifier = val

    @characteristics.setter
    def characteristics(self, val):
        self.__characteristics = val

    @manufacturer_data.setter
    def manufacturer_data(self, val):
        self.__manufacturer_data = val

    def __repr__(self):
        return json.dumps(
            {'device_id': self.device_identifier,
             'characteristics': self.characteristics,
             'manufacturer_data': self.manufacturer_data},
            indent=4
        )