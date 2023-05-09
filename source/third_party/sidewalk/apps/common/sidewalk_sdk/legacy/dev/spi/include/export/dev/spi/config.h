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

#ifndef HALO_DEV_SPI_EXPORT_DEV_SPI_CONFIG_H
#define HALO_DEV_SPI_EXPORT_DEV_SPI_CONFIG_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t                            gpio_sck;
    uint32_t                            gpio_mosi;
    uint32_t                            gpio_miso;
    uint32_t                            gpio_default_nss;

    const void *                        ll_instance;
} iot_spi_config_t;

typedef struct {
    const iot_spi_config_t *            list;
    size_t                              count;
} iot_spi_instances_t;

#endif /* !HALO_DEV_SPI_EXPORT_DEV_SPI_CONFIG_H */
