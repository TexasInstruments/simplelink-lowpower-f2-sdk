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

#include <halo/lib/hall/hallerr.h>
#include <halo/lib/hall/appv1/mgm_gateway/types.h>
#include <halo/lib/hall/appv1/cmd.h>

#ifndef HALO_LIB_HALL_INCLUDE_APPV1_MGM_GATEWAY_SERIALIZE_H_
#define HALO_LIB_HALL_INCLUDE_APPV1_MGM_GATEWAY_SERIALIZE_H_

/**
 * For details see: https://wiki.labcollab.net/confluence/display/HALO/Halo+Application+Layer
 * command: NOTIFY_PROXY_RESP_TIME (0x89A)
 * payload: |5B UUID|16B KEY|4B Random Key|
 */

sid_error_t halo_hall_appv1_mgm_gateway_time_proxy_serialize(
    uint8_t *buffer, const size_t len, const halo_hall_appv1_proxy_response_time_t *data, hall_size_t *out_size);

sid_error_t halo_hall_appv1_mgm_ng_discovery_serialize(
    uint8_t *buffer, const size_t len, const halo_hall_appv1_mgm_ng_discovery_params_t *data, hall_size_t *out_size);

sid_error_t halo_hall_appv1_mgm_dedicated_link_request_serialize(
    uint8_t *buffer, const size_t len, const halo_hall_appv1_mgm_dedicated_link_request_t *data, hall_size_t *out_size);
sid_error_t halo_hall_appv1_mgm_dedicated_link_response_serialize(
    uint8_t *buffer, const size_t len, const halo_hall_appv1_mgm_dedicated_link_response_t *data, hall_size_t *out_size);

sid_error_t halo_hall_appv1_mgm_gw_distress_discovery_param_serialize(
    uint8_t *buffer, const size_t len, const halo_hall_appv1_mgm_gw_distress_discovery_param_t *data, hall_size_t *out_size);

sid_error_t halo_hall_appv1_mgm_cloud_auth_serialize(
    uint8_t *buffer, const size_t len, const halo_hall_appv1_cloud_auth_token_t *data, hall_size_t *out_size);

sid_error_t halo_hall_appv1_mgm_gw_mac_serialize(
        uint8_t *buffer, const size_t len, const halo_hall_appv1_mac_t *data, hall_size_t *out_size);

sid_error_t halo_hall_appv1_mgm_gw_hw_id_info_serialize(
    uint8_t *buffer, const size_t len, const halo_hall_appv1_hw_id_info_t *data, hall_size_t *out_size);

#endif /* HALO_LIB_HALL_INCLUDE_APPV1_MGM_GATEWAY_SERIALIZE_H_ */
