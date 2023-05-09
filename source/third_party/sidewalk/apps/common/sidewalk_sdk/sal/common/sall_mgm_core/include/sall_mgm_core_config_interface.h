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

#ifndef SALL_MGM_CORE_CONFIG_INTERFACE_H
#define SALL_MGM_CORE_CONFIG_INTERFACE_H

#include <sall_mgm_core_lib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    sid_error_t (*validate)(const struct sall_mgm_core_handle *const handle, const void *data);
    sid_error_t (*set)(struct sall_mgm_core_handle *handle, const void *data);
    void (*get)(const struct sall_mgm_core_handle *const handle, void *data);
    sid_error_t (*response_send)(const struct sall_mgm_core_handle *const handle,
                                 halo_hall_message_descriptor_t *mdesc,
                                 const void *data);
    const sall_mgm_core_config_id_t id;
} sall_mgm_core_config_ifc_t;

sid_error_t sall_mgm_core_config_ifc_get(const sall_mgm_core_config_ifc_t **handle, const sall_mgm_core_config_id_t id);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif /* SALL_MGM_CORE_CONFIG_INTERFACE_H */
