/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SALL_SIDEBAND_INTERFACE_H_
#define SALL_SIDEBAND_INTERFACE_H_

#include <sall_mgm_core_interface.h>
#include <halo/lib/hall/appv1/sideband/types.h>
#include <halo/lib/hall/appv1/sideband/ifc.h>
#include <sid_network_control_ifc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sall_sideband_gw_params {
    const halo_hall_lib_ctx_t lib_ctx;
    const struct sid_event_queue *event_queue;
    halo_hall_appv1_bind_params_t bind_params;
    const network_interface_data_ifc *sdb_destination;
};

typedef const struct sall_sideband_gw_ifc_s *sall_sideband_gw_ifc_t;

struct sall_sideband_gw_ifc_s {
    void (*destroy)(const sall_sideband_gw_ifc_t *ifc);
    sid_error_t (*on_tx_prepare)(const sall_sideband_gw_ifc_t *ifc, const void *data);
};

sid_error_t sall_sideband_gw_create(const sall_sideband_gw_ifc_t **ifc,
                                    const struct sall_sideband_gw_params *params);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif /* SALL_SIDEBAND_INTERFACE_H_ */
