/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef NETWORK_INTERFACE_MOCK_H
#define NETWORK_INTERFACE_MOCK_H

#include <sid_network_control_ifc.h>
#include <halo/lib/hall/core/resource_manager.h>

typedef struct {
    const halo_hall_resource_manager_ifc* resource_manager;
    const struct sid_address* local;
    int hw_link;
} network_interface_config;

sid_error_t network_interface_mock_create(
        const network_interface_ifc** network_interface,
        const network_interface_config* cfg);

#endif // NETWORK_INTERFACE_MOCK_H

