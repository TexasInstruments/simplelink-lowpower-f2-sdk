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

#ifndef SID_PAL_BLE_ADAPTER_IFC_H
#define SID_PAL_BLE_ADAPTER_IFC_H

#include <sid_ble_config_ifc.h>
#include <sid_error.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct sid_pal_ble_adapter_interface * sid_pal_ble_adapter_interface_t;

typedef void (* sid_pal_ble_data_callback_t)(sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length);
typedef void (* sid_pal_ble_notify_callback_t)(sid_ble_cfg_service_identifier_t id, bool state);
typedef void (* sid_pal_ble_connection_callback_t)(bool state, uint8_t *addr);
typedef void (* sid_pal_ble_indication_callback_t)(bool status);
typedef void (* sid_pal_ble_mtu_callback_t)(uint16_t size);
typedef void (* sid_pal_ble_adv_start_callback_t)(void);

typedef struct
{
    sid_pal_ble_data_callback_t       data_callback;
    sid_pal_ble_notify_callback_t     notify_callback;
    sid_pal_ble_connection_callback_t conn_callback;
    sid_pal_ble_indication_callback_t ind_callback;
    sid_pal_ble_mtu_callback_t        mtu_callback;
    sid_pal_ble_adv_start_callback_t  adv_start_callback;
} sid_pal_ble_adapter_callbacks_t;

struct sid_pal_ble_adapter_interface {
    sid_error_t (*init)          (const sid_ble_config_t *cfg);
    sid_error_t (*start_service) (void);
    sid_error_t (*set_adv_data)  (uint8_t *data, uint8_t length);
    sid_error_t (*start_adv)     (void);
    sid_error_t (*stop_adv)      (void);
    sid_error_t (*send)          (sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length);
    sid_error_t (*set_callback)  (const sid_pal_ble_adapter_callbacks_t *cb);
    sid_error_t (*disconnect)    (void);
    sid_error_t (*deinit)        (void);
};

sid_error_t sid_pal_ble_adapter_create(sid_pal_ble_adapter_interface_t *handle);

#endif /* SID_PAL_BLE_ADAPTER_IFC_H */
