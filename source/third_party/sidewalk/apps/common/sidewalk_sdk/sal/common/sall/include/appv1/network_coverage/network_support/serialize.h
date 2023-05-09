/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef RNETC_INCLUDE_APPV1_NETWORK_COVERAGE_SUPPORT_SERIALIZE_H_
#define RNETC_INCLUDE_APPV1_NETWORK_COVERAGE_SUPPORT_SERIALIZE_H_

#include <halo/lib/hall/hallerr.h>
#include <halo/lib/hall/appv1/network_coverage/network_support/types.h>
#include <halo/lib/hall/appv1/cmd.h>
#include <appv1/mgm_core/serialize.h>

#ifdef __cplusplus
extern "C" {
#endif

sid_error_t halo_hall_appv1_start_network_test_serialize(uint8_t* buffer, const size_t len, const halo_hall_appv1_start_network_test_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_start_network_test_notify_serialize(uint8_t* buffer, const size_t len, const halo_hall_appv1_notify_network_test_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_stop_network_test_serialize(uint8_t* buffer, const size_t len, const halo_hall_appv1_stop_network_test_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_network_test_results_serialize(uint8_t* buffer, const size_t len, const halo_hall_appv1_network_test_results_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_stop_network_test_serialize(uint8_t* buffer, const size_t len, const halo_hall_appv1_stop_network_test_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_rx_event_serialize(uint8_t* buffer, const size_t len, const halo_hall_appv1_rx_event_notify_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_tx_event_serialize(uint8_t* buffer, const size_t len, const halo_hall_appv1_tx_event_notify_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_network_missed_pong_serialize(uint8_t* buffer, const size_t len, const halo_hall_appv1_network_missed_pong_notify_t* data, hall_size_t* out_size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_INCLUDE_APPV1_NETWORK_COVERAGE_SUPPORT_SERIALIZE_H_ */
