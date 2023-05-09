/*
 * Copyright 2020-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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
#ifndef SALL_MGM_TYPES_H
#define SALL_MGM_TYPES_H

#include <halo/lib/hall/appv1/mgm/ifc.h>

#include <sid_event_queue_ifc.h>
#include <sid_900_types.h>
#include <sall_mgm_core_types.h>

#define DFLT_GRP_PRDCITY 1
#define DFLT_UNI_PRDCITY 4
#define SALL_MGM_FFS_RETRY_THRESHOLDS_SIZE 4

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int8_t rf_power_max[HALO_HALL_APPV1_MAX_DATA_RATES];
    uint8_t device_mode_max;
    uint8_t missed_beacons_max;
    uint8_t ldr_channel_max;
} sall_mgm_validation_t;

typedef struct {
    sid_ni_900_internal_config_t default_config;
    sall_mgm_validation_t validation_param;
    uint16_t config_group;
} sall_mgm_config_cmd_ctx_t;

typedef struct {
    sall_mgm_config_cmd_ctx_t config_900;
    sall_mgm_core_config_t config_core;
} sall_mgm_config_t;

typedef struct {
    sid_error_t (*on_sall_mgm_ffs_start)(bool is_key_refresh);
} sall_mgm_ffs_callbacks_t;

typedef struct {
    sall_mgm_ffs_callbacks_t ffs_callbacks;
    int16_t rssi_threshold;
    uint8_t retry_threshold[SALL_MGM_FFS_RETRY_THRESHOLDS_SIZE];
    uint16_t retry_wait_time_s[SALL_MGM_FFS_RETRY_THRESHOLDS_SIZE];
    uint8_t max_retries;
    uint32_t periodicity_s;
    bool enable;
} sall_mgm_ffs_config_t;

typedef struct {
    const struct sid_event_queue *event_queue;
    const halo_hall_lib_ctx_t lib_ctx;
    halo_hall_appv1_bind_params_t bind_params;
    const config_provider_t gw_cfg_access;
    sall_mgm_core_callbacks_t core_callbacks;
#if SID_SDK_CONFIG_ENABLE_LINK_TYPE_2_REGISTRATION
    sall_mgm_ffs_config_t *ffs_config;
#endif
} sall_mgm_params_t;

typedef enum {
    MGM_CONFIG_OUT_RANGE_BEFORE_FIRST = SALL_MGM_CORE_CONFIG_LAST,
    MGM_CONFIG_FIRST,
    MGM_CONFIG_ID_RF_POWER = MGM_CONFIG_FIRST,
    MGM_CONFIG_ID_CONFIG_COMPLETE,
    MGM_CONFIG_ID_DEVICE_MODE,
    MGM_CONFIG_ID_UNICAST_PARAM,
    MGM_CONFIG_ID_GROUP_PARAM,
    MGM_CONFIG_ID_PROTOCOL_ON,
    MGM_CONFIG_ID_MAX_MISSED_BEACONS,
    MGM_CONFIG_ID_UNUSED_1,
    MGM_CONFIG_ID_BEACONS_INTERVAL,
    MGM_CONFIG_ID_LDR_CHANNEL,
    MGM_CONFIG_ID_DEFICIT_FLAG,
    MGM_CONFIG_ID_GW_CONSENT_FLG,
    MGM_CONFIG_ID_PAN_SCAN_INTERVAL,
    MGM_CONFIG_ID_DEV_CLS_FSK_PARAMS,
    MGM_CONFIG_ID_DEV_CLS_LDR_PARAMS,
    MGM_CONFIG_ID_TMP_PRTCL_EXP,
    MGM_CONFIG_ID_DEV_FSK_WAN_UL_PARAM,
    MGM_CONFIG_ID_DEV_FSK_WAN_DL_PARAM,
    MGM_CONFIG_ID_DEV_FSK_WAN_TRAFFIC,
    MGM_CONFIG_ID_DEV_PROFILE_SYNC,
    MGM_CONFIG_ID_DEV_PROFILE_ASYNC,
    MGM_CONFIG_LAST = MGM_CONFIG_ID_DEV_PROFILE_ASYNC
} sall_mgm_config_id_t;
_Static_assert(sizeof(sall_mgm_config_id_t) == sizeof(sall_mgm_core_config_id_t), "sall_mgm_core_config_id_t should be type typecastable to sall_mgm_config_id_t");
_Static_assert((sall_mgm_config_id_t)MGM_CONFIG_FIRST > (sall_mgm_config_id_t)SALL_MGM_CORE_CONFIG_LAST, "sall_mgm_config_t should not overlap with sall_mgm_core_config_id_t");

enum {
    WAKEUP_TX_ENABLED = 0x1,
    WAKEUP_RX_ENABLED = 0x2,
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* SALL_MGM_TYPES_H */
