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

application_id = ['beams_legacy',
                  'halo_network_server',
                  'halo_metrics',
                  'halo_time_sync_server',
                  'halo_provisioning_server'] +\
                  ['sidewalk_reserved'] * 27 +\
                  ['tracker_application',
                   'sailboat_application',
                   'ALS',
                   'BMDS',
                   'tile',
                   'alexa_presence'] +\
                   ['sidewalk_reserved'] * 62


device_state = [
    'ble_mesh',
    'ble'
]


device_mode = [
    'normal',
    'oobe',
    'connection_request',
    'reserved'
]


battery_level = [
    'critical',
    'low',
    'normal',
    'reserved'
]
