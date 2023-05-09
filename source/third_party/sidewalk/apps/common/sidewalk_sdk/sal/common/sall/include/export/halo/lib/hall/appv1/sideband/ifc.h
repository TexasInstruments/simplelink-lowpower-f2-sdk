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

#ifndef RNETC_PUBLIC_HALO_HALL_APPV1_SIDEBAND_IFC_H_
#define RNETC_PUBLIC_HALO_HALL_APPV1_SIDEBAND_IFC_H_

#include <halo/lib/hall/appv1/sideband/types.h>
#include <halo/lib/hall/appv1/message.h>
#include <halo/lib/hall/appv1/generic_command_ifc.h>
#include <halo/lib/hall/interface.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct halo_hall_appv1_sideband_ifc_s* halo_hall_appv1_sideband_ifc;

struct halo_hall_appv1_sideband_ifc_s {
    sid_error_t (*sdb_control_set)(
        const halo_hall_appv1_sideband_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_sdb_control_t* param);
    sid_error_t (*sdb_control_response)(
        const halo_hall_appv1_sideband_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_sdb_control_t* param);

    sid_error_t (*sdb_open_set)(
        const halo_hall_appv1_sideband_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_sdb_open_t* param);
    sid_error_t (*sdb_open_response)(
        const halo_hall_appv1_sideband_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_sdb_open_t* param);

    sid_error_t (*sdb_maintain_set)(
        const halo_hall_appv1_sideband_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_sdb_maintain_t* param);
    sid_error_t (*sdb_maintain_response)(
        const halo_hall_appv1_sideband_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_sdb_maintain_t* param);

    sid_error_t (*sdb_keep_alive_notify)(
        const halo_hall_appv1_sideband_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_sdb_keep_alive_t* param);
    sid_error_t (*sdb_keep_alive_response)(
        const halo_hall_appv1_sideband_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_sdb_keep_alive_t* param);

    sid_error_t (*sdb_link_status_get)(
        const halo_hall_appv1_sideband_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_sdb_link_status_t* param);
    sid_error_t (*sdb_link_status_notify)(
        const halo_hall_appv1_sideband_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_sdb_link_status_t* param);
    sid_error_t (*sdb_link_status_response)(
        const halo_hall_appv1_sideband_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_sdb_link_status_t* param);

    sid_error_t (*sdb_close_set)(
        const halo_hall_appv1_sideband_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_sdb_close_t* param);
    sid_error_t (*sdb_close_response)(
        const halo_hall_appv1_sideband_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_sdb_close_t* param);
};

typedef struct halo_hall_appv1_sideband_ext_ifc_s halo_hall_appv1_sideband_ext_ifc;

struct halo_hall_appv1_sideband_ext_ifc_s {
    halo_hall_appv1_sideband_ifc send;
    halo_hall_appv1_sideband_ifc receive;
    halo_hall_appv1_generic_command_ifc generic;
};

typedef struct halo_hall_dispatcher_ifc_s* halo_hall_dispatcher_ifc;

sid_error_t halo_hall_appv1_sideband_create(const halo_hall_appv1_sideband_ext_ifc** _this, const halo_hall_appv1_sideband_ifc user_notify_cb,
    halo_hall_appv1_user_ctx_t user_ctx, const void* lib_ctx);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_PUBLIC_HALO_HALL_APPV1_SIDEBAND_IFC_H */
