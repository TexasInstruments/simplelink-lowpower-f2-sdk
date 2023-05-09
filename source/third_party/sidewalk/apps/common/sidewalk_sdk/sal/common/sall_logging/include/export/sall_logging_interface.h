/*
 * Copyright 2021-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SALL_LOGGING_INTERFACE_H_
#define SALL_LOGGING_INTERFACE_H_

#include <halo/lib/hall/appv1/logging/ifc.h>
#include <sall_mgm_core_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sall_logging_config {
    uint32_t notify_interval_s;
};

struct  sall_logging_params {
    const halo_hall_lib_ctx_t lib_ctx;
    const struct sid_event_queue *event_queue;
    halo_hall_appv1_bind_params_t bind_params;
};

typedef const struct sall_logging_ifc_s *sall_logging_ifc_t;

struct sall_logging_ifc_s {
    void (*destroy)(const sall_logging_ifc_t *ifc);
};

sid_error_t sall_logging_create(const sall_logging_ifc_t **ifc,
                                const struct sall_logging_config *config,
                                const struct sall_logging_params *params);

#ifdef __cplusplus
}
#endif

#endif /* SALL_LOGGING_INTERFACE_H_ */
