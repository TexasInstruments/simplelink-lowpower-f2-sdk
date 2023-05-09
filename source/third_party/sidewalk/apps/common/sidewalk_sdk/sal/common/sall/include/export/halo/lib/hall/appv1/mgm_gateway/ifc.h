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

#ifndef HALO_LIB_HALL_INCLUDE_EXPORT_HALO_LIB_HALL_APPV1_MGM_GATEWAY_IFC_H_
#define HALO_LIB_HALL_INCLUDE_EXPORT_HALO_LIB_HALL_APPV1_MGM_GATEWAY_IFC_H_

#include <halo/lib/hall/appv1/mgm_gateway/types.h>
#include <halo/lib/hall/appv1/message.h>
#include <halo/lib/hall/appv1/generic_command_ifc.h>
#include <halo/lib/hall/interface.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct halo_hall_appv1_mgm_gateway_ifc_s* halo_hall_appv1_mgm_gateway_ifc;

struct halo_hall_appv1_mgm_gateway_ifc_s {
    sid_error_t (*time_proxy_notify)(
        const halo_hall_appv1_mgm_gateway_ifc *self, halo_hall_message_descriptor_t *mdesc,
        const halo_hall_appv1_proxy_response_time_t *data);
    sid_error_t (*mfg_proxy_flag_set)(
        const halo_hall_appv1_mgm_gateway_ifc *self, halo_hall_message_descriptor_t *mdesc,
        const halo_hall_appv1_mfg_proxy_flag *data);

    sid_error_t (*gw_consent_flag_get)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc);
    sid_error_t (*gw_consent_flag_set)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const halo_hall_appv1_mgm_flag_t data);
    sid_error_t (*gw_consent_flag_response)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const halo_hall_appv1_mgm_flag_t data);

    sid_error_t (*ng_discovery_set)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const halo_hall_appv1_mgm_ng_discovery_params_t *data);

    sid_error_t (*gw_announcement_notify)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const uint8_t operating_channel);

    sid_error_t (*dedicated_link_init_request)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const halo_hall_appv1_mgm_dedicated_link_request_t *data);
    sid_error_t (*dedicated_link_init_response)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const halo_hall_appv1_mgm_dedicated_link_response_t *data);

    sid_error_t (*dedicated_link_hand_shake_req)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const uint8_t channel_num);

    sid_error_t (*dev_status_announcement)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const halo_hall_appv1_mgm_dev_status_announcement_t data);

    sid_error_t (*dev_status_announcement_response)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const halo_hall_appv1_mgm_dev_status_announcement_t data);

    sid_error_t (*gw_distress_discovery_set)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const halo_hall_appv1_mgm_gw_distress_discovery_param_t *data);

    sid_error_t (*cloud_auth_token_get)(
        const halo_hall_appv1_mgm_gateway_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*cloud_auth_token_response)(
        const halo_hall_appv1_mgm_gateway_ifc* _this, halo_hall_message_descriptor_t* mdesc,
        const halo_hall_appv1_cloud_auth_token_t* power);

    sid_error_t (*wifi_mac_get)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc);
    sid_error_t (*wifi_mac_set)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const halo_hall_appv1_mac_t *data);
    sid_error_t (*wifi_mac_response)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const halo_hall_appv1_mac_t *data);

    sid_error_t (*ethernet_mac_get)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc);
    sid_error_t (*ethernet_mac_set)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const halo_hall_appv1_mac_t *data);
    sid_error_t (*ethernet_mac_response)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const halo_hall_appv1_mac_t *data);

    sid_error_t (*p2p_unicast_key_set)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const uint8_t* pkt, const uint8_t size);
    sid_error_t (*p2p_unicast_key_response)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const uint8_t* pkt, const uint8_t size);

    sid_error_t (*hw_id_get)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const uint8_t format);
    sid_error_t (*hw_id_response)(
        const halo_hall_appv1_mgm_gateway_ifc *_this, halo_hall_message_descriptor_t *mdesc,
        const halo_hall_appv1_hw_id_info_t *hw_id_info);
};

typedef struct halo_hall_appv1_mgm_gateway_ext_ifc_s halo_hall_appv1_mgm_gateway_ext_ifc;

struct halo_hall_appv1_mgm_gateway_ext_ifc_s {
    halo_hall_appv1_mgm_gateway_ifc send;
    halo_hall_appv1_mgm_gateway_ifc receive;
    halo_hall_appv1_generic_command_ifc generic;
};

typedef struct halo_hall_dispatcher_ifc_s* halo_hall_dispatcher_ifc;

sid_error_t halo_hall_appv1_mgm_gateway_create(
    const halo_hall_appv1_mgm_gateway_ext_ifc **_this,
    const halo_hall_appv1_mgm_gateway_ifc user_notify_cb,
    halo_hall_appv1_user_ctx_t user_ctx,
    const void* lib_ctx);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HALO_LIB_HALL_INCLUDE_EXPORT_HALO_LIB_HALL_APPV1_MGM_GATEWAY_IFC_H_ */
