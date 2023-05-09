/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_PAL_SERIAL_BUS_IFC_H
#define SID_PAL_SERIAL_BUS_IFC_H

#include <sid_error.h>

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

enum sid_pal_serial_bus_bit_order {
    SID_PAL_SERIAL_BUS_BIT_ORDER_MSB_FIRST,
    SID_PAL_SERIAL_BUS_BIT_ORDER_LSB_FIRST,
};

struct sid_pal_serial_bus_client {
    uint32_t client_selector;
    uint32_t speed_hz;

    enum sid_pal_serial_bus_bit_order bit_order;
    uint8_t mode;

    const void *client_selector_extension;
};

struct sid_pal_serial_bus_iface;

struct sid_pal_serial_bus_iface {
    // for full duplex communication
    sid_error_t (*xfer)(const struct sid_pal_serial_bus_iface *iface,
                        const struct sid_pal_serial_bus_client *client,
                        uint8_t *tx,
                        uint8_t *rx,
                        size_t xfer_size);
    // for half-duplex
    sid_error_t (*xfer_hd)(const struct sid_pal_serial_bus_iface *iface,
                           const struct sid_pal_serial_bus_client *client,
                           uint8_t *tx,
                           uint8_t *rx,
                           size_t tx_size,
                           size_t rx_size);
    sid_error_t (*destroy)(const struct sid_pal_serial_bus_iface *iface);
};

struct sid_pal_serial_bus_factory {
    sid_error_t (*create)(const struct sid_pal_serial_bus_iface **iface, const void *config);
    const void *config;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_PAL_SERIAL_BUS_IFC_H */
