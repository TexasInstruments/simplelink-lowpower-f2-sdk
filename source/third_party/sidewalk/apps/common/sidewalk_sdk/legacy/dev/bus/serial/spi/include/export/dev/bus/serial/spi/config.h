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

#ifndef HALO_DEV_BUS_SERIAL_SPI_INCLUDE_EXPORT_DEV_BUS_SERIAL_SPI
#define HALO_DEV_BUS_SERIAL_SPI_INCLUDE_EXPORT_DEV_BUS_SERIAL_SPI

#ifdef __cplusplus
extern "C" {
#endif

#include <dev/bus/serial/serial.h>

#include <halo/error.h>

#include <stdint.h>

typedef struct {
    int32_t instance_id;
    uint8_t dummy_byte;
} halo_serial_bus_spi_config_t;

halo_error_t bus_serial_spi_create(const halo_serial_bus_iface_t ** iface, const void * const context);

#ifdef __cplusplus
}
#endif

#endif /* HALO_DEV_BUS_SERIAL_SPI_INCLUDE_EXPORT_DEV_BUS_SERIAL_SPI */
