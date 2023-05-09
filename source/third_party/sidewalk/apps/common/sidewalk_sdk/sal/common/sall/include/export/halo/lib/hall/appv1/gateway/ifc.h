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

#ifndef RNETC_PUBLIC_HALO_HALL_APPV1_GATEWAY_IFC_H_
#define RNETC_PUBLIC_HALO_HALL_APPV1_GATEWAY_IFC_H_

#include <halo/lib/hall/appv1/gateway/types.h>
#include <halo/lib/hall/appv1/mgm/types.h>
#include <halo/lib/hall/appv1/message.h>
#include <halo/lib/hall/appv1/generic_command_ifc.h>
#include <halo/lib/hall/interface.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct halo_hall_appv1_gateway_ifc_s* halo_hall_appv1_gateway_ifc;

struct halo_hall_appv1_gateway_ifc_s {
    sid_error_t (*send_pkt)(const halo_hall_appv1_gateway_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_all_gateway_pkt_t* pkt);
    sid_error_t (*send_pkt_response)(const halo_hall_appv1_gateway_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*rx_pkt_notify)(const halo_hall_appv1_gateway_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_all_gateway_pkt_t* pkt);
    sid_error_t (*send_dfu_pkt)(const halo_hall_appv1_gateway_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_all_gateway_pkt_t* pkt);
    sid_error_t (*rx_dfu_pkt_notify)(
        const halo_hall_appv1_gateway_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_all_gateway_pkt_t* pkt);
    sid_error_t (*set_dfu_params)(
        const halo_hall_appv1_gateway_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_mgm_dfu_params_t* params);
    sid_error_t (*dfu_params_response)(const halo_hall_appv1_gateway_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*zwave_tx_flag_get)(const halo_hall_appv1_gateway_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*zwave_tx_flag_set)(const halo_hall_appv1_gateway_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t enable);
    sid_error_t (*zwave_tx_flag_response)(const halo_hall_appv1_gateway_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t enable);
    sid_error_t (*zwave_threshold_get)(const halo_hall_appv1_gateway_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*zwave_threshold_set)(
        const halo_hall_appv1_gateway_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_all_gateway_zwave_threshold_t* threshold);
    sid_error_t (*zwave_threshold_response)(
        const halo_hall_appv1_gateway_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_all_gateway_zwave_threshold_t* threshold);
    sid_error_t (*dfu_zwave_params_get)(const halo_hall_appv1_gateway_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*dfu_zwave_params_response)(
        const halo_hall_appv1_gateway_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_all_gateway_zwave_params_t* params);
    sid_error_t (*fsk_wan_auth_response)(const halo_hall_appv1_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc, const halo_hall_appv1_mgm_join_proc_key_t *data);
    sid_error_t (*fsk_wan_sch_get)(const halo_hall_appv1_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc, const halo_hall_appv1_mgm_join_proc_key_t *data);
    sid_error_t (*fsk_wan_sch_set)(const halo_hall_appv1_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc, const halo_hall_appv1_mgm_join_proc_key_t *data);
    sid_error_t (*fsk_wan_sch_notify)(const halo_hall_appv1_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc, const halo_hall_appv1_mgm_join_proc_key_t *data);
    sid_error_t (*fsk_wan_sch_response)(
        const halo_hall_appv1_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc, const halo_hall_appv1_mgm_join_proc_key_t *data);
    sid_error_t (*fsk_wan_sch_term_notify)(const halo_hall_appv1_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc, const halo_hall_appv1_mgm_join_proc_key_t *data);
};

typedef struct halo_hall_appv1_gateway_ext_ifc_s halo_hall_appv1_gateway_ext_ifc;

struct halo_hall_appv1_gateway_ext_ifc_s {
    halo_hall_appv1_gateway_ifc send;
    halo_hall_appv1_gateway_ifc receive;
    halo_hall_appv1_generic_command_ifc generic;
};

typedef struct halo_hall_dispatcher_ifc_s* halo_hall_dispatcher_ifc;

sid_error_t halo_hall_appv1_gateway_create(const halo_hall_appv1_gateway_ext_ifc** _this, const halo_hall_appv1_gateway_ifc user_notify_cb,
    halo_hall_appv1_user_ctx_t user_ctx, const void* lib_ctx);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_PUBLIC_HALO_HALL_APPV1_GATEWAY_IFC_H_ */
