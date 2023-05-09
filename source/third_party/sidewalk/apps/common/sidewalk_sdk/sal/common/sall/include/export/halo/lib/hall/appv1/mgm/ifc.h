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

#ifndef RNETC_PUBLIC_HALO_HALL_APPV1_MGM_IFC_H_
#define RNETC_PUBLIC_HALO_HALL_APPV1_MGM_IFC_H_

#include <halo/lib/hall/appv1/mgm/types.h>
#include <halo/lib/hall/appv1/message.h>
#include <halo/lib/hall/appv1/generic_command_ifc.h>
#include <halo/lib/hall/interface.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct halo_hall_appv1_mgm_ifc_s* halo_hall_appv1_mgm_ifc;

struct halo_hall_appv1_mgm_ifc_s {
    sid_error_t (*start_dfu_set)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_mgm_dfu_params_t* param);
    sid_error_t (*start_dfu_response)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*vendor_pkt_set)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* pkt, const uint8_t size);
    sid_error_t (*vendor_pkt_response)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*channel_survey_get)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_channel_survey_prop_t* param);
    sid_error_t (*channel_survey_response)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_channel_survey_results_t* res);

    sid_error_t (*rf_power_set)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_rf_power_t* power);
    sid_error_t (*rf_power_get)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t data);
    sid_error_t (*rf_power_response)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_rf_power_t* power);

    sid_error_t (*rf_security_settings_set)(
            const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_security_params_t* security);
    sid_error_t (*rf_security_settings_get)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*rf_security_settings_response)(
            const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_security_capabilities_t* params);

    sid_error_t (*device_profile_get)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*device_profile_set)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_dev_profile_t* data);
    sid_error_t (*device_profile_response)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_dev_profile_resp_t* data);

    sid_error_t (*max_missed_beacons_get)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*max_missed_beacons_set)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t data);
    sid_error_t (*max_missed_beacons_response)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t data);

    sid_error_t (*beacons_interval_get)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*beacons_interval_set)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t data);
    sid_error_t (*beacons_interval_response)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t data);

    sid_error_t (*ldr_channel_get)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*ldr_channel_set)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t data);
    sid_error_t (*ldr_channel_response)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t data);

    sid_error_t (*deficit_flag_get)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*deficit_flag_set)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_mgm_flag_t data);
    sid_error_t (*deficit_flag_response)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_mgm_flag_t data);

    sid_error_t (*protocol_on_get)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*protocol_on_set)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_mgm_flag_t data);
    sid_error_t (*protocol_on_response)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_mgm_flag_t data);

    sid_error_t (*pan_scan_interval_get)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*pan_scan_interval_set)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint16_t data);
    sid_error_t (*pan_scan_interval_response)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint16_t data);

    sid_error_t (*configuration_params_get)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*configuration_params_response)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_config_param_t* data);

    sid_error_t (*slot_params_get)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t data);
    sid_error_t (*slot_params_set)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_slot_params_t* data);
    sid_error_t (*slot_params_respose)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_slot_param_t* data);

    sid_error_t (*join_proc_request)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_mgm_join_proc_key_t* param);
    sid_error_t (*join_proc_response)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const halo_hall_appv1_mgm_join_proc_key_t* res);
    sid_error_t (*join_proc_notify)(const halo_hall_appv1_mgm_ifc *_this, halo_hall_message_descriptor_t *mdesc);

    sid_error_t (*dev_cls_ldr_get)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*dev_cls_ldr_set)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const struct halo_hall_appv1_dev_cls_ldr* data);
    sid_error_t (*dev_cls_ldr_response)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const struct halo_hall_appv1_dev_cls_ldr* data);

    sid_error_t (*dev_cls_fsk_get)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*dev_cls_fsk_set)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const struct halo_hall_appv1_dev_cls_fsk* data);
    sid_error_t (*dev_cls_fsk_response)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const struct halo_hall_appv1_dev_cls_fsk* data);
    sid_error_t (*goto_sideband_mode)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc);

    sid_error_t (*tmp_prtcl_exp_get)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*tmp_prtcl_exp_set)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const mgm_config_tlv_frmt_t* data);
    sid_error_t (*tmp_prtcl_exp_response)(const halo_hall_appv1_mgm_ifc* _this, halo_hall_message_descriptor_t* mdesc, const mgm_config_tlv_frmt_t* data);
};

typedef struct halo_hall_appv1_mgm_ext_ifc_s halo_hall_appv1_mgm_ext_ifc;

struct halo_hall_appv1_mgm_ext_ifc_s {
    halo_hall_appv1_mgm_ifc send;
    halo_hall_appv1_mgm_ifc receive;
    halo_hall_appv1_generic_command_ifc generic;
};

typedef struct halo_hall_dispatcher_ifc_s* halo_hall_dispatcher_ifc;

sid_error_t halo_hall_appv1_mgm_create(const halo_hall_appv1_mgm_ext_ifc** _this, const halo_hall_appv1_mgm_ifc user_notify_cb,
    halo_hall_appv1_user_ctx_t user_ctx, const void* lib_ctx);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_PUBLIC_HALO_HALL_APPV1_MGM_IFC_H_ */
