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

#ifndef RNETC_INCLUDE_APPV1_COMMON_SERIALIZE_H_
#define RNETC_INCLUDE_APPV1_COMMON_SERIALIZE_H_

#include <halo/lib/hall/hallerr.h>
#include <halo/lib/hall/appv1/common_types.h>
#include <halo/lib/hall/appv1/cmd.h>

#include <appv1/wrappers/send_message.h>

#ifdef __cplusplus
extern "C" {
#endif

// CMD Header

sid_error_t halo_hall_appv1_cmd_to_flex_serialize(uint8_t* buffer, const size_t len, const halo_all_cmd_t*, hall_size_t* out_size);
sid_error_t halo_hall_appv1_cmd_to_flex_serialize_reverse(uint8_t* buffer, const size_t len, const halo_all_cmd_t*, hall_size_t* out_size);

sid_error_t halo_hall_appv1_cmd_exheader_serialize(uint8_t* buffer, const size_t len, const halo_all_cmd_t*, hall_size_t* out_size);
sid_error_t halo_hall_appv1_cmd_exheader_serialize_reverse(uint8_t* buffer, const size_t len, const halo_all_cmd_t*, hall_size_t* out_size);

// Utility functions

sid_error_t halo_hall_appv1_raw_buffer_serialize(uint8_t* buffer, size_t len, const halo_hall_appv1_raw_buffer_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_pld_buffer_serialize(uint8_t* buffer, const size_t len, const uint8_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_uint8_serialize(uint8_t* buffer, const size_t len, const uint8_t pld, hall_size_t* out_size);
sid_error_t halo_hall_appv1_uint16_serialize(uint8_t* buffer, const size_t len, const uint16_t pld, hall_size_t* out_size);
sid_error_t halo_hall_appv1_uint32_serialize(uint8_t* buffer, size_t len, const uint32_t pld, hall_size_t* out_size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_INCLUDE_APPV1_COMMON_SERIALIZE_H_ */
