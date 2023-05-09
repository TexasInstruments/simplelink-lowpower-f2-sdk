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

#ifndef RNETC_PUBLIC_HALO_HALL_APPV1_MGM_CORE_IFC_H_
#define RNETC_PUBLIC_HALO_HALL_APPV1_MGM_CORE_IFC_H_

#include <halo/lib/hall/appv1/mgm_core/types.h>
#include <halo/lib/hall/appv1/message.h>
#include <halo/lib/hall/appv1/generic_command_ifc.h>
#include <halo/lib/hall/interface.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct halo_hall_appv1_mgm_core_ifc_s* halo_hall_appv1_mgm_core_ifc;

struct halo_hall_appv1_mgm_core_ifc_s {
    sid_error_t (*fw_version_get)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*fw_version_response)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_mgm_firmware_version_t* fw_version);

    sid_error_t (*serial_num_get)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*serial_num_response)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* serial, uint8_t size);

    sid_error_t (*device_id_get)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*device_id_response)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_device_id_t* id);

    sid_error_t (*pan_id_set)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_pan_id_t* id);
    sid_error_t (*pan_id_get)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*pan_id_response)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_pan_id_t* id);

    sid_error_t (*group_id_set)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t id);
    sid_error_t (*group_id_get)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*group_id_response)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t id);

    sid_error_t (*p2p_unicast_key_set)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_beams_security_key_t* key);
    sid_error_t (*p2p_unicast_key_get)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*p2p_unicast_key_response)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_beams_security_key_t* key);

    sid_error_t (*p2p_bcast_key_set)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_beams_security_key_t* key);
    sid_error_t (*p2p_bcast_key_get)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*p2p_bcast_key_response)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_beams_security_key_t* key);

    sid_error_t (*keep_alive_v1_notify)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* data, const uint8_t size);
    sid_error_t (*keep_alive_v2_notify)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* data, const uint8_t size);
    sid_error_t (*keep_alive_interval_set)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint32_t interval_s);
    sid_error_t (*keep_alive_interval_get)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*keep_alive_interval_response)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint32_t interval_s);

    sid_error_t (*rset_info_get)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*rset_info_response)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_reset_info_t* info);
    sid_error_t (*rset_info_notify)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_reset_info_t* info);

    sid_error_t (*factory_reset_set)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const struct sall_mgm_factory_reset *reset);
    sid_error_t (*factory_reset_response)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*factory_reset_notify)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*pairing_state_set)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t state);
    sid_error_t (*pairing_state_get)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*pairing_state_response)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t state);
    sid_error_t (*pairing_state_notify)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t state);

    sid_error_t (*class_version_get)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint16_t);
    sid_error_t (*class_version_response)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_class_versions_t* v);

    sid_error_t (*config_complete_set)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t state);
    sid_error_t (*config_complete_response)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*status_get)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*status_response)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* data, uint8_t size);

    sid_error_t (*fw_ota_params_get)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*fw_ota_params_response)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_ota_params_t* params);

    /**
     * P2P channel command must be in mgm_core to support the same provisioning flow
     * for BLE Only and 900MHz SDK implementation
     */
    sid_error_t (*p2p_channel_set)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_mgm_p2p_channel_t* channel_info);
    sid_error_t (*p2p_channel_get)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*p2p_channel_response)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t channel);

    sid_error_t (*time_set)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_gcs_time_t* tm);
    sid_error_t (*time_get)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_gcs_time_wkey_t* tm);
    sid_error_t (*time_response)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_gcs_time_wkey_t* tm);
    sid_error_t (*time_notify)(const halo_hall_appv1_mgm_core_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_gcs_time_t* tm);
};

typedef struct halo_hall_appv1_mgm_core_ext_ifc_s halo_hall_appv1_mgm_core_ext_ifc;

struct halo_hall_appv1_mgm_core_ext_ifc_s {
    halo_hall_appv1_mgm_core_ifc send;
    halo_hall_appv1_mgm_core_ifc receive;
    halo_hall_appv1_generic_command_ifc generic;
};

typedef struct halo_hall_dispatcher_ifc_s* halo_hall_dispatcher_ifc;

sid_error_t halo_hall_appv1_mgm_core_create(
        const halo_hall_appv1_mgm_core_ext_ifc** _this,
        const halo_hall_appv1_mgm_core_ifc user_notify_cb,
        halo_hall_appv1_user_ctx_t user_ctx,
        const void* lib_ctx);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_PUBLIC_HALO_HALL_APPV1_MGM_CORE_IFC_H_ */
