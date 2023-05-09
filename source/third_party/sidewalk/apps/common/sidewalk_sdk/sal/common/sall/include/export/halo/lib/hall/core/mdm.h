/*
 * Copyright 2019-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef HALO_HALS_MDM_H_
#define HALO_HALS_MDM_H_

#include <sid_network_data_ifc.h>
#include <sid_memory_pool.h>
#include <sid_time_types.h>

#include <halo/lib/hall/hallerr.h>
#include <sid_network_address.h>
#include <halo/lib/hall/sequence_number.h>
#include <halo/lib/hall/core/dispatcher.h>
#include <halo/lib/hall/core/router.h>
#include <halo/lib/hall/core/stats.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t max_addresses_number;
    uint8_t max_messages_per_address;
} halo_hall_deduplicator_config_t;

typedef struct {
    uint8_t max_policy_number;
    uint32_t no_give_up_retry_period;
    halo_hall_policy_params_t default_policy_params;
    halo_resource_manager_config resource_manager_config;
    halo_hall_deduplicator_config_t deduplicator_config;
} halo_hall_mdm_config_t;

typedef void* timer_ctx_t;
typedef void* mdm_ctx_t;
typedef sid_error_t (*on_timer_t)(const mdm_ctx_t, struct sid_timespec*);
typedef sid_error_t (*set_timer_t)(timer_ctx_t, struct sid_timespec*, const mdm_ctx_t, on_timer_t);
typedef uint32_t (*get_sequence_number_t)(void* ctx);

typedef struct {
    const halo_hall_dispatcher_ifc * dispatcher;
    const struct sid_memory_pool * mem_pool;
    const halo_lib_sequence_number_ifc * sequence_number;
    const halo_hall_stats_ifc * stats;
    set_timer_t set_timer_cb;
    timer_ctx_t tctx;
} halo_hall_mdm_create_param_t;

/**
 * @brief halo_hall_policy_init
 *
 * Set all policy components to the defult state
 * counted as not specified
 */
sid_error_t halo_hall_policy_key_init(halo_hall_policy_key_t* policy);

typedef const struct halo_hall_mdm_ifc_s* halo_hall_mdm_ifc;

struct halo_hall_mdm_ifc_s {
    void (*destroy)(const halo_hall_mdm_ifc* _this);
    const halo_hall_resource_manager_ifc* (*get_resource_manager)(const halo_hall_mdm_ifc* _this);
    sid_error_t (*send)(const halo_hall_mdm_ifc* _this, halo_all_cmd_t* msg);
    sid_error_t (*set_policy)(const halo_hall_mdm_ifc* _this, const halo_hall_policy_key_t* policy_key, const halo_hall_policy_params_t* params);
    sid_error_t (*set_downlink)(const halo_hall_mdm_ifc* _this, const network_interface_data_ifc* data_ifc);
    const network_interface_data_notify_ifc* (*get_downlink_notify)(const halo_hall_mdm_ifc* _this);
};

sid_error_t halo_hall_mdm_create(const halo_hall_mdm_ifc** _this,
                                  const halo_hall_mdm_config_t* cfg,
                                  const halo_hall_mdm_create_param_t* param);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HALO_HALS_MDM_H_ */
