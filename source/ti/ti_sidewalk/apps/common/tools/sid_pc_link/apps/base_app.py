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

import sys
import os
import json

APPS_BASE_PATH = os.path.dirname(os.path.realpath(__file__))
LIBRARY_BASE_PATH = '{base}/{libs}'.format(base=APPS_BASE_PATH, libs='../libs')
CURR_WS = os.getcwd()
_libs = []
_config_map = {}

def add_lib(path):
    global _libs
    _libs.append(path)


def load_dependencies():
    global _libs
    for library in _libs:
        lib_path = '{path}/{lib}'.format(path=LIBRARY_BASE_PATH, lib=library)
        print("Adding library {}".format(lib_path))
        sys.path.append(lib_path)


def load_config():
    global _config_map
    app_config_path = '{base_path}/app_config.json'.format(base_path=CURR_WS)
    with open(app_config_path, 'r') as app_config_file:
        app_config_json = json.load(app_config_file)
    for item in app_config_json.items():
        key, value = item
        # Initialize the value as None by default
        _config_map.update({key: None})
        # Give precedence to the environment configuration
        if value is not None and os.getenv(key) is None:
            os.environ.update({key: str(value)})
            _config_map.update({key: str(value)})
        else:
            if os.getenv(key) is not None:
                _config_map.update({key: os.getenv(key)})
    # If there is any configuration present in the environment
    # Write it back to the config file
    with open(app_config_path, 'w') as app_config_file:
        app_config_file.write(json.dumps(_config_map, indent=4))


def get_config():
    return _config_map

#########################################################
# Load the configuration when this module is imported
#########################################################
# Add the base apps path
load_config()