/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SALL_MGM_INTERFACE_H
#define SALL_MGM_INTERFACE_H

#include <halo/lib/hall/appv1/mgm/ifc.h>

#include <sid_error.h>
#include <sall_mgm_types.h>
#include <sall_mgm_core_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct sall_mgm_ifc_s * sall_mgm_ifc;

/**
 * Refers to instance of Management Library Extended implementation
 * It contains set of functionalities which works as a services and encapsulates core implementation.
 *
 * @note: no need to create instance of @ref halo_management_core_ifc in case if halo_management_ifc created
 */
struct sall_mgm_ifc_s {
    void (* destroy)(const sall_mgm_ifc *ifc);
    sid_error_t (*config_get)(const sall_mgm_ifc *ifc, sid_ni_900_config_t *config);
    sid_error_t (*get_mgm_core)(const sall_mgm_ifc *ifc, struct sall_mgm_core_handle **mgm_core);
    sid_error_t (*protocol_enable)(const sall_mgm_ifc *ifc, const bool on);
    sid_error_t (*get_config_by_id)(const sall_mgm_ifc *ifc, const sall_mgm_config_id_t id, void *data);
    sid_error_t (*set_config_by_id)(const sall_mgm_ifc *ifc, const sall_mgm_config_id_t id, const void *data);
    sid_error_t (*get_ni_control_notify)(const sall_mgm_ifc *ifc, const network_interface_control_notify_ifc **ctrl_ntfy_interface);
    sid_error_t (*ffs_start)(const sall_mgm_ifc *ifc);
    void (*ffs_stop)(const sall_mgm_ifc *ifc);
    void (*ffs_handle_error)(const sall_mgm_ifc *ifc);
    void (*ffs_handle_config_complete)(const sall_mgm_ifc *ifc, bool complete);
    void (*ffs_key_refresh_start)(const sall_mgm_ifc *ifc);
    void (*ffs_key_refresh_stop)(const sall_mgm_ifc *ifc);
};

sid_error_t sall_mgm_create(const sall_mgm_ifc **ifc,
        const sall_mgm_config_t *config,
        const sall_mgm_params_t *params);

sid_error_t sall_mgm_library_instance_get(const sall_mgm_ifc **ifc);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* SALL_MGM_INTERFACE_H */
