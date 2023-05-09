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

#ifndef SALL_MGM_CORE_TYPES_H
#define SALL_MGM_CORE_TYPES_H

#include <halo/lib/hall/appv1/mgm_core/ifc.h>
#include <sid_event_queue_ifc.h>
#include <sid_network_type.h>
#include <sid_network_control_ifc.h>

#include <stddef.h>

#define SALL_MGM_GCS_SUSPEND_INTERVALS_SIZE 5

#ifdef __cplusplus
extern "C" {
#endif

typedef void * sall_mgm_core_user_ctx_t;

struct sall_mgm_core_handle;

typedef struct {
    sall_mgm_core_user_ctx_t user_ctx;
    sid_error_t (*halo_user_app_status_v1_get)(sall_mgm_core_user_ctx_t *user_ctx,
                        void *buffer, uint8_t *size, const struct sall_mgm_core_handle *const handle,
                        halo_hall_message_descriptor_t *mdesc);
    sid_error_t (*halo_user_app_status_v2_get)(sall_mgm_core_user_ctx_t *user_ctx,
                        void *buffer, uint8_t *size, const struct sall_mgm_core_handle *const handle,
                        halo_hall_message_descriptor_t *mdesc);
    sid_error_t (*halo_user_battery_level_get)(sall_mgm_core_user_ctx_t *user_ctx,
                        uint8_t *battery_level);
    sid_error_t (*user_reset_info_get)(sall_mgm_core_user_ctx_t *user_ctx,
                        halo_hall_appv1_reset_info_t *reset_info);
    void (*on_factory_reset)(sall_mgm_core_user_ctx_t *user_ctx,
                        const uint8_t reset_type);
    void (*on_pairing_state_change)(sall_mgm_core_user_ctx_t *user_ctx,
                        const uint8_t state);
    void (*on_configuration_complete)(sall_mgm_core_user_ctx_t *user_ctx,
                        const bool state);
    void (*on_time_sync)(sall_mgm_core_user_ctx_t *user_ctx,
                         const halo_hall_message_descriptor_t *const mdesc,
                         const bool state);
    void (*on_connection_state_change) (sall_mgm_core_user_ctx_t *user_ctx,
                        const network_interface_ifc *net_ifc,
                        const bool is_connected);
    void (*sall_user_app_on_sideband_set)(sall_mgm_core_user_ctx_t *user_ctx);
    void (*on_radio_tx)(sall_mgm_core_user_ctx_t *user_ctx,
                        const network_interface_ifc *net_ifc,
                        const void *info);
    sid_error_t (*on_tx_prepare)(sall_mgm_core_user_ctx_t *user_ctx,
                                 const network_interface_ifc *net_ifc,
                                 const void *data);
    bool (*is_gw_registered)(void *user_ctx);
} sall_mgm_core_user_entry_t;

typedef struct {
    const network_interface_control_notify_ifc *ctrl_ntfy_mgm;
    const network_interface_control_notify_ifc *ctrl_ntfy_gateway;
} sall_mgm_core_ni_control_notify_entry_t;

typedef struct {
    void (*on_ni_configured)(void *context, const network_interface_control_ifc *ni_control);
    void *context;
} sall_mgm_core_callbacks_t;

typedef struct {
    void (*on_group_id_update)(void *context, const uint8_t id);
    sid_error_t (*get_config)(void *context, void *buffer, const size_t size, size_t *used);
    void (* connection_state_change) (void* context, const network_interface_control_ifc *ni_control,
            const network_interface_control_connection_state connection_state);
    void *context;
    uint32_t size;
} config_provider_t;

typedef struct {
    uint32_t ka_interval_max;
    uint8_t p2p_channel_max;
} sall_mgm_core_validation_t;

typedef struct {
    sid_ni_config_t default_config;
    sall_mgm_core_validation_t validation_param;
    uint16_t config_group;
    uint8_t applying_config_delay_s;
} sall_mgm_core_config_ctx_t;

typedef struct {
    uint32_t cloud_sync_interval_s;
    uint16_t request_suspend_interval_s[SALL_MGM_GCS_SUSPEND_INTERVALS_SIZE];
    uint8_t request_retry_interval_s;
    uint8_t try_count_max;
    uint8_t max_drift_s;
} sall_mgm_gcs_config_t;

typedef struct {
    sall_mgm_core_config_ctx_t config_cmd_ctx;
    sall_mgm_gcs_config_t gcs_config;
    uint16_t factory_reset_delay_ms;
    uint8_t cloud_addr[SID_ADDRESS_SIZE_MAX];
    uint8_t app_status_size;
    uint8_t gcs_cloud_id;
    uint8_t reset_info_cloud_id;
    uint8_t ni_max_count;
    uint8_t boot_reset_report;
} sall_mgm_core_config_t;

typedef struct {
    const struct sid_event_queue *event_queue;
    const halo_hall_lib_ctx_t lib_ctx;
    config_provider_t cfg_access;
    halo_hall_appv1_bind_params_t bind_params;
    sall_mgm_core_callbacks_t callbacks;
} sall_mgm_core_params_t;

typedef enum {
    SALL_MGM_CORE_CONFIG_FIRST = 0x00,
    SALL_MGM_CORE_CONFIG_ID_PAN_ID = SALL_MGM_CORE_CONFIG_FIRST,
    SALL_MGM_CORE_CONFIG_ID_BRK_KEY,
    SALL_MGM_CORE_CONFIG_ID_UNI_KEY,
    SALL_MGM_CORE_CONFIG_ID_P2P_CHANNEL,
    SALL_MGM_CORE_CONFIG_ID_GROUP_ID,
    SALL_MGM_CORE_CONFIG_ID_PAIRING_ST,
    SALL_MGM_CORE_CONFIG_ID_KA_INTERVAL,
    SALL_MGM_CORE_CONFIG_ID_CONFIG_COMPLETE,
    SALL_MGM_CORE_CONFIG_ID_DEVICE_ID,
    SALL_MGM_CORE_CONFIG_LAST = SALL_MGM_CORE_CONFIG_ID_DEVICE_ID
} sall_mgm_core_config_id_t;

typedef enum {
    SALL_FACTORY_RESET_TYPE_SOFT = 0,
    SALL_FACTORY_RESET_TYPE_HARD = 1,
} sall_factory_reset_type_t;

#ifdef __cplusplus
} // extern "C"
#endif


#endif /* SALL_MGM_CORE_TYPES_H */
