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

#ifndef HALO_ALL_APPV1_LOGGING_SERIALIZE_H_
#define HALO_ALL_APPV1_LOGGING_SERIALIZE_H_

#include <stddef.h>
#include <halo/lib/hall/hallerr.h>
#include <halo/lib/hall/appv1/logging/types.h>
#include <halo/lib/hall/appv1/cmd.h>

#ifdef __cplusplus
extern "C" {
#endif

sid_error_t halo_all_appv1_logging_sniffer_cfg_serialize(
    uint8_t* buffer, const size_t len, const halo_all_logging_sniffer_cfg_t* data, hall_size_t* out_size);
sid_error_t halo_all_appv1_logging_log_cfg_serialize(uint8_t* buffer, const size_t len, const halo_all_logging_log_cfg_t* data, hall_size_t* out_size);
sid_error_t halo_all_appv1_logging_log_cfg_serialize(uint8_t* buffer, const size_t len, const halo_all_logging_log_cfg_t* data, hall_size_t* out_size);
sid_error_t halo_all_appv1_logging_pkt_rx_info_serialize(
    uint8_t* buffer, const size_t len, const halo_all_logging_pkt_rx_info_t* data, hall_size_t* out_size);
sid_error_t halo_all_appv1_logging_pkt_tx_info_serialize(
    uint8_t* buffer, const size_t len, const halo_all_logging_pkt_tx_info_t* data, hall_size_t* out_size);
sid_error_t halo_all_appv1_logging_module_cfgs_serialize(
    uint8_t* buffer, const size_t len, const halo_all_logging_module_cfgs_t* data, hall_size_t* out_size);
sid_error_t halo_all_appv1_logging_entries_clear_serialize(
    uint8_t* buffer, const size_t len, const halo_all_logging_entries_clear_t* data, hall_size_t* out_size);
sid_error_t halo_all_appv1_logging_entries_serialize(
    uint8_t* buffer, const size_t len, const halo_all_logging_log_entries_t* data, hall_size_t* out_size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // HALO_ALL_PARSER_APPV1_LOGGING_H_
