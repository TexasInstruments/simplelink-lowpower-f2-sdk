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

#ifndef RNETC_PUBLIC_HALO_HALL_APPV1_LINK_METRICS_IFC_H_
#define RNETC_PUBLIC_HALO_HALL_APPV1_LINK_METRICS_IFC_H_

#include <halo/lib/hall/appv1/message.h>
#include <halo/lib/hall/appv1/generic_command_ifc.h>
#include <halo/lib/hall/interface.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct halo_hall_appv1_link_metrics_ifc_s* halo_hall_appv1_link_metrics_ifc;

struct halo_hall_appv1_link_metrics_ifc_s {
    sid_error_t (*metrics_get)(const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*metrics_response)(
        const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* data, const uint8_t size);
    sid_error_t (*metrics_notify)(
        const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* data, const uint8_t size);
    sid_error_t (*ep2p_metrics_get)(const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*ep2p_metrics_response)(
        const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* data, const uint8_t size);
    sid_error_t (*ep2p_metrics_notify)(
        const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* data, const uint8_t size);
    sid_error_t (*metrics_sched_get)(const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*metrics_sched_response)(
        const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* data, const uint8_t size);
    sid_error_t (*metrics_sched_notify)(
        const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* data, const uint8_t size);
    sid_error_t (*metrics_noise_get)(const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*metrics_noise_response)(
        const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* data, const uint8_t size);
    sid_error_t (*metrics_noise_notify)(
        const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc,const uint8_t* data, const uint8_t size);
    sid_error_t (*metrics_ldr_get)(const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*metrics_ldr_response)(
        const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* data, const uint8_t size);
    sid_error_t (*metrics_ldr_notify)(
        const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* data, const uint8_t size);
    sid_error_t (*metrics_en_ldr_phy_imp1_get)(const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc);
    sid_error_t (*metrics_en_ldr_phy_imp1_response)(
        const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* data, const uint8_t size);
    sid_error_t (*metrics_en_ldr_phy_imp1_notify)(
        const halo_hall_appv1_link_metrics_ifc* _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* data, const uint8_t size);
    sid_error_t (*metrics_gw_ldr_phy_imp_get)(const halo_hall_appv1_link_metrics_ifc *_this, halo_hall_message_descriptor_t *mdesc);
    sid_error_t (*metrics_gw_ldr_phy_imp_response)(
        const halo_hall_appv1_link_metrics_ifc *_this, halo_hall_message_descriptor_t *mdesc, const uint8_t *data, const uint8_t size);
    sid_error_t (*metrics_gw_ldr_phy_imp_notify)(
        const halo_hall_appv1_link_metrics_ifc *_this, halo_hall_message_descriptor_t *mdesc, const uint8_t *data, const uint8_t size);
    sid_error_t (*all_metrics_get)(
        const halo_hall_appv1_link_metrics_ifc *_this, halo_hall_message_descriptor_t *mdesc, const uint8_t *data, const uint8_t size);
    sid_error_t (*all_metrics_response)(
        const halo_hall_appv1_link_metrics_ifc *_this, halo_hall_message_descriptor_t *mdesc, const uint8_t *data, const uint8_t size);
};

typedef struct halo_hall_appv1_link_metrics_ext_ifc_s halo_hall_appv1_link_metrics_ext_ifc;

struct halo_hall_appv1_link_metrics_ext_ifc_s {
    halo_hall_appv1_link_metrics_ifc send;
    halo_hall_appv1_link_metrics_ifc receive;
    halo_hall_appv1_generic_command_ifc generic;
};

typedef struct halo_hall_dispatcher_ifc_s* halo_hall_dispatcher_ifc;

sid_error_t halo_hall_appv1_link_metrics_create(const halo_hall_appv1_link_metrics_ext_ifc** _this,
    const halo_hall_appv1_link_metrics_ifc user_notify_cb, halo_hall_appv1_user_ctx_t user_ctx, const void* lib_ctx);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_PUBLIC_HALO_HALL_APPV1_LINK_METRICS_IFC_H_ */
