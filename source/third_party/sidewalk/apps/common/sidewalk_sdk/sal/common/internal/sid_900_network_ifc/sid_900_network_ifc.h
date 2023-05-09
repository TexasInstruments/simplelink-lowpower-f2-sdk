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

#ifndef SID_900_NETWORK_INTERFACE_H
#define SID_900_NETWORK_INTERFACE_H

#include <sid_network_control_ifc.h>
#include <sid_900_types.h>

#include <sid_event_queue_ifc.h>
#include <sid_error.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct halo_hall_resource_manager_ifc_s* halo_hall_resource_manager_ifc;
typedef struct sid_pal_event_queue_s sid_pal_event_queue_t;

typedef struct {
    uint16_t mtu;
    uint16_t metric;
} sid_900_network_interface_control_config_t;

typedef struct {
    const sid_900_network_interface_control_config_t * ni_control_config;
    const struct sid_address* local;
    const struct sid_event_queue * event_queue;
    const halo_hall_resource_manager_ifc* resource_manager;
    const sid_ni_900_config_t * ni_config;
    int   hw_link;          //!< required to implement interface on top of ringnet-hall
                            //!< should be removed/replaced after ringnet will be reworked
                            //!<
    uint32_t crystal_32khz;
} sid_900_network_interface_config;

/**
 * Create instance of Network Interface (NI) to communicate over 900MHz radio
 * @param [out]network_interface - reference to created instance
 * @param [in] cfg               - NI configuration, required to be static const,
 *                                  NI will keep a reference to configuration
 * @param [in] ctrl_ntfy_interface - interface to send notification about internal NI
 *                                  status changes, should be provided by RPM or another
 *                                  controller for NI
 * @return numerical error code
 * @retval SID_ERROR_NONE - interface is created and ready to operate
 * @retval SID_ERROR_OOM  - Not enough memory to create instance
 * @retval
 */

sid_error_t sid_900_network_ifc_create(
        const network_interface_ifc** network_interface,
        const sid_900_network_interface_config* cfg,
        const network_interface_control_notify_ifc* ctrl_ntfy_interface);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SID_900_NETWORK_IFC
