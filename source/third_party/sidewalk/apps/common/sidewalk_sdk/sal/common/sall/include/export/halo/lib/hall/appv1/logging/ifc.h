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

#ifndef RNETC_PUBLIC_HALO_HALL_APPV1_LOGGING_IFC_H_
#define RNETC_PUBLIC_HALO_HALL_APPV1_LOGGING_IFC_H_

#include <halo/lib/hall/appv1/logging/types.h>
#include <halo/lib/hall/appv1/message.h>
#include <halo/lib/hall/appv1/generic_command_ifc.h>
#include <halo/lib/hall/interface.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct halo_hall_appv1_logging_ifc_s* halo_hall_appv1_logging_ifc;

struct halo_hall_appv1_logging_ifc_s {
#ifdef FULL_LOGGING_CLASS
    sid_error_t (*sniffer_cfg_get)(const halo_hall_appv1_logging_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*sniffer_cfg_set)(
        const halo_hall_appv1_logging_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_all_logging_sniffer_cfg_t* status);
    sid_error_t (*sniffer_cfg_response)(
        const halo_hall_appv1_logging_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_all_logging_sniffer_cfg_t* status);

    sid_error_t (*notify_sniffer_pkt_rx)(
        const halo_hall_appv1_logging_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_all_logging_pkt_rx_info_t* status);
    sid_error_t (*notify_sniffer_pkt_tx)(
        const halo_hall_appv1_logging_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_all_logging_pkt_tx_info_t* status);

    sid_error_t (*module_logging_set)(
        const halo_hall_appv1_logging_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_all_logging_module_cfgs_t* cfg);
    sid_error_t (*module_logging_response)(const halo_hall_appv1_logging_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*log_entries_clear)(
        const halo_hall_appv1_logging_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_all_logging_entries_clear_t* req);
    sid_error_t (*log_entries_get)(const halo_hall_appv1_logging_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint16_t num_of_entries);
    sid_error_t (*log_entries_response)(
        const halo_hall_appv1_logging_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_all_logging_log_entries_t* entries);
    sid_error_t (*log_entries_notify)(
        const halo_hall_appv1_logging_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_all_logging_log_entries_t* entries);

    sid_error_t (*missed_pong_notify)(const halo_hall_appv1_logging_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t ping_num);
#endif
    sid_error_t (*error_notify)(
        const halo_hall_appv1_logging_ifc* _this, halo_hall_message_descriptor_t* mdesc, const sall_app_error_t err_code);

    sid_error_t (*logging_cfg_get)(const halo_hall_appv1_logging_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*logging_cfg_set)(
        const halo_hall_appv1_logging_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_all_logging_log_cfg_t* status);
    sid_error_t (*logging_cfg_response)(
        const halo_hall_appv1_logging_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_all_logging_log_cfg_t* status);

};

typedef struct halo_hall_appv1_logging_ext_ifc_s halo_hall_appv1_logging_ext_ifc;

struct halo_hall_appv1_logging_ext_ifc_s {
    halo_hall_appv1_logging_ifc send;
    halo_hall_appv1_logging_ifc receive;
    halo_hall_appv1_generic_command_ifc generic;
};

typedef struct halo_hall_dispatcher_ifc_s* halo_hall_dispatcher_ifc;

sid_error_t halo_hall_appv1_logging_create(const halo_hall_appv1_logging_ext_ifc** _this, const halo_hall_appv1_logging_ifc user_notify_cb,
    halo_hall_appv1_user_ctx_t user_ctx, const void* lib_ctx);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_PUBLIC_HALO_HALL_APPV1_LOGGING_IFC_H_ */
