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

#ifndef SALL_MGM_CONFIGURATION_INTERFACE_H
#define SALL_MGM_CONFIGURATION_INTERFACE_H

#include <sall_mgm_lib.h>
#include <sall_mgm_cli_ifc.h>
#include <sid_error.h>

typedef struct {
    sid_error_t (* validate)(const sall_mgm_config_cmd_ctx_t * config_cmd_ctx, const void * data);
    sid_error_t (* set)(const sall_mgm_config_cmd_ctx_t * config_cmd_ctx, const void * data);
    sid_error_t (* response_send)(sall_mgm_lib_impl_t * impl,
            halo_hall_message_descriptor_t * mdesc, const void * data);
    void (* get)(const sall_mgm_config_cmd_ctx_t * config_cmd_ctx, void * data);
    const sall_mgm_config_id_t id;
} sall_mgm_config_ifc_t;

sid_error_t sall_mgm_config_ifc_get(const sall_mgm_config_ifc_t ** handle,
                                const sall_mgm_config_id_t id);
sid_error_t sall_mgm_config_get(const sall_mgm_ifc * ifc, sid_ni_900_config_t * config);
sid_error_t
sall_mgm_config_set_and_apply_by_id(const sall_mgm_ifc *ifc, const sall_mgm_config_id_t id, const void *data);
sid_error_t sall_mgm_config_get_by_id(const sall_mgm_ifc *ifc, const sall_mgm_config_id_t id, void *data);
#endif /* SALL_MGM_CONFIGURATION_INTERFACE_H */
