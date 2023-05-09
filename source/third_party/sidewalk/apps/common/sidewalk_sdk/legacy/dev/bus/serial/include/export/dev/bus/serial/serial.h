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

#ifndef HALO_DEV_BUS_SERIAL_INCLUDE_EXPORT_DEV_BUS_SERIAL
#define HALO_DEV_BUS_SERIAL_INCLUDE_EXPORT_DEV_BUS_SERIAL

#ifdef __cplusplus
extern "C" {
#endif

#include <halo/error.h>

#include <stdint.h>
#include <stddef.h>

typedef struct halo_serial_bus_iface_t halo_serial_bus_iface_t;

typedef enum {
    HALO_SERIAL_BIT_ORDER_MSB_FIRST,
    HALO_SERIAL_BIT_ORDER_LSB_FIRST,
} halo_serial_bit_order_t;

typedef struct {
    uint32_t                    client_selector;
    uint32_t                    speed_hz;

    halo_serial_bit_order_t     bit_order;
    uint8_t                     mode;

    const void *                client_selector_extention;
} halo_serial_bus_client_t;

struct halo_serial_bus_iface_t {
    // for full duplex communication
    halo_error_t (*xfer     ) (const halo_serial_bus_iface_t * iface, const halo_serial_bus_client_t * client, uint8_t *tx, uint8_t *rx, size_t xfer_size);
    // for half-duplex
    halo_error_t (*xfer_hd  ) (const halo_serial_bus_iface_t * iface, const halo_serial_bus_client_t * client, uint8_t *tx, uint8_t *rx, size_t tx_size, size_t rx_size);
    halo_error_t (*destroy  ) (const halo_serial_bus_iface_t * iface);
};

typedef struct {
    halo_error_t (*create)(const halo_serial_bus_iface_t ** iface, const void * config);
    const void * config;
} halo_serial_bus_factory_t;

#ifdef __cplusplus
}
#endif

#endif /* HALO_DEV_BUS_SERIAL_INCLUDE_EXPORT_DEV_BUS_SERIAL */
