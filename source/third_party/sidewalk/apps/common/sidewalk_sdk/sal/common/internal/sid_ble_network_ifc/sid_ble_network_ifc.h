/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_BLE_NETWORK_IFC_H
#define SID_BLE_NETWORK_IFC_H

#include <sid_ble_link_config_ifc.h>

#include <sid_network_control_ifc.h>
#include <sid_network_data_ifc.h>

#include <sid_event_queue_ifc.h>
#include <halo/lib/hall/core/resource_manager.h>

#include <sid_error.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sid_ble_network_interface_config {
    const halo_hall_resource_manager_ifc *const resource_manager;
    const struct sid_address *const local;
    const sid_ble_link_config_t *const link_cfg;
    const struct sid_event_queue *const event_queue;
    uint16_t metric;
    uint8_t application_service_id;
    uint8_t provisioning_state;
    uint8_t battery_level;
    uint32_t time_sync_periodicity_seconds;
};

/*
 * @brief   Create the BLE network interface
 *
 * @return  SID_ERROR_NONE on success otherwise appropriate error is returned
 */
sid_error_t sid_ble_network_ifc_create(const network_interface_ifc **network_interface,
                                       const struct sid_ble_network_interface_config *const cfg,
                                       const network_interface_control_notify_ifc *const ctrl_ntfy_interface);

#ifdef __cplusplus
}
#endif

#endif /* SID_BLE_NETWORK_IFC_H */
