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

#ifndef HALO_HALL_APPV1_GENERIC_COMMAND_IFC_H_
#define HALO_HALL_APPV1_GENERIC_COMMAND_IFC_H_

#include <halo/lib/hall/hallerr.h>
#include <sid_network_address.h>
#include <halo/lib/hall/appv1/cmd.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* halo_hall_appv1_user_ctx_t;
typedef struct halo_hall_appv1_generic_command_ifc_s* halo_hall_appv1_generic_command_ifc;

typedef sid_error_t (*halo_hall_appv1_on_message_t)(const halo_hall_appv1_generic_command_ifc *cmd_generic,
                                                     halo_all_cmd_t* msg);

typedef struct {
    const network_interface_data_ifc* route;
    const struct sid_address* remote;
} halo_hall_appv1_bind_params_t;

struct halo_hall_appv1_generic_command_ifc_s {
    void (*destroy)(const halo_hall_appv1_generic_command_ifc*);
    sid_error_t (*bind)(const halo_hall_appv1_generic_command_ifc* cmd_this, const halo_hall_appv1_bind_params_t* params);
    halo_hall_appv1_user_ctx_t user_ctx;
    halo_hall_appv1_on_message_t on_message;
    halo_hall_appv1_class_version_t class_version;
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HALO_HALL_APPV1_GENERIC_COMMAND_IFC_H_ */
