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

#ifndef SALL_LINK_METRIC_INTERFACE_H_
#define SALL_LINK_METRIC_INTERFACE_H_

#include <halo/lib/hall/appv1/link_metrics/ifc.h>
#include <sall_mgm_core_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t notify_interval_s;
    uint8_t cloud_id;
} sall_link_metric_config_t;

typedef struct {
    struct sall_mgm_core_handle *mgm_core_handle;
    const halo_hall_lib_ctx_t lib_ctx;
    const struct sid_event_queue *event_queue;
    halo_hall_appv1_bind_params_t bind_params;
} sall_link_metric_parems_t;

typedef const struct sall_link_metric_ifc_s *sall_link_metric_ifc_t;

struct sall_link_metric_ifc_s {
    void (*destroy)(const sall_link_metric_ifc_t *ifc);
};

sid_error_t sall_link_metric_create(const sall_link_metric_ifc_t **ifc,
        const sall_link_metric_config_t *config, const sall_link_metric_parems_t *params);

#ifdef __cplusplus
}
#endif

#endif /* SALL_LINK_METRIC_INTERFACE_H_ */
