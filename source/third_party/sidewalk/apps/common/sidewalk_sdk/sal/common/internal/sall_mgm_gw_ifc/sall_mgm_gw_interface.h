/*
 * Copyright 2021-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SALL_MGM_GW_INTERFACE_H_
#define SALL_MGM_GW_INTERFACE_H_

#include <halo/lib/hall/interface.h>
#include <sall_mgm_interface.h>
#include <sid_900_types.h>
#include <sid_event_queue_ifc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sid_gw_exp_mfg_store_callbacks;

typedef struct {
    uint16_t router_metric_low;
    uint16_t router_metric_high;
    uint16_t initial_inter_msg_delay_s;
    halo_hall_appv1_mgm_gw_distress_discovery_param_t discovery;
    uint8_t offline_server_cloud_id;
} sall_mgm_gw_distress_config_t;

struct distress_params_validation {
    uint8_t version;
    uint8_t msg_max_delay_min_low;
    uint8_t msg_max_delay_min_high;
};

typedef struct {
    struct distress_params_validation distress;
} sall_mgm_gw_validation_t;

typedef struct {
    sid_ni_900_gw_config_t default_config;
    sall_mgm_gw_validation_t validation_params;
    uint16_t config_group;
} sall_mgm_gw_cmd_config_t;

typedef struct {
    sall_mgm_gw_cmd_config_t gw_commands;
    sall_mgm_config_t mgm;
    sall_mgm_gw_distress_config_t distress;
} sall_mgm_gw_config_t;

typedef const struct sall_mgm_gw_ifc_s *sall_mgm_gw_ifc;

struct sall_mgm_gw_ifc_s {
    sid_error_t (*set_mfg_store_callbacks)(const sall_mgm_gw_ifc *_this,
                                           const struct sid_gw_exp_mfg_store_callbacks *const callbacks);
    sid_error_t (*mgm_get)(const sall_mgm_gw_ifc *_this, const sall_mgm_ifc **mgm);
    sid_error_t (*config_get)(const sall_mgm_gw_ifc *_this, sid_ni_900_config_t *config);
    sid_error_t (*refresh_user_consent_flag)(const sall_mgm_gw_ifc *_this);
    void (*destroy)(const sall_mgm_gw_ifc *_this);
};

typedef struct {
    const halo_hall_lib_ctx_t sall_lib_ctx;
    const struct sid_event_queue *event_queue;
    halo_hall_appv1_bind_params_t bind_params;
    uint32_t hw_id;
} sall_mgm_gw_params_t;

sid_error_t sall_mgm_gw_create(const sall_mgm_gw_ifc **_this,
                               const sall_mgm_gw_config_t *config,
                               const sall_mgm_gw_params_t *params);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif /* SALL_MGM_GW_INTERFACE_H_ */
