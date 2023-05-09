/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef RNET_BLE_H
#define RNET_BLE_H

#if !PROTOCOL_STANDALONE_BUILD

#include <sid_protocol_defs.h>
#include <halo/error.h>
#include "nrf_log_ctrl.h"
#include "rnet_dbg_log.h"

#define RNET_BLE_VERSION            (0x01)

#define APP_BLE_OBSERVER_PRIO       3   /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define RNET_BLE_MAX_PKT_LEN        128
#define COMPANY_IDENTIFIER          0x0171          // Amazon Company Identifier www.bluetooth.com
#if NRF_MODULE_ENABLED(BLE_DIS)
#define MANUFACTURER_NAME           "Amazon"
#endif

void                    rnet_ble_init();
halo_error_t            rnet_ble_comm_enable();
halo_error_t            rnet_ble_comm_disable();
halo_error_t            rnet_ble_adv_disable(void);
halo_error_t            rnet_ble_send(uint8_t * p_data, uint16_t * p_length);
nrf_log_backend_nus_t * nrf_log_backend_nus_get(void);
bool                    rnet_ble_is_connected(void);

extern U8               rnet_ble_tx_buf[RNET_BLE_MAX_PKT_LEN];

#endif

#endif  // RNET_BLE_H
