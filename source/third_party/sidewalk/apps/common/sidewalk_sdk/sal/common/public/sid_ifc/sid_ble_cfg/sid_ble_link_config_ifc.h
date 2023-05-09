/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_BLE_LINK_CONFIG_IFC_H
#define SID_BLE_LINK_CONFIG_IFC_H

#include <sid_ble_config_ifc.h>
#include <sid_error.h>
#include <sid_pal_ble_adapter_ifc.h>

typedef struct sid_ble_link_config {
    sid_error_t (*create_ble_adapter)(struct sid_pal_ble_adapter_interface **handle);
    const struct sid_ble_config *config;
} sid_ble_link_config_t;

#endif