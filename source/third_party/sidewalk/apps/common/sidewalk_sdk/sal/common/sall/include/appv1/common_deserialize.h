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

#ifndef RNETC_INCLUDE_APPV1_COMMON_DESERIALIZE_H_
#define RNETC_INCLUDE_APPV1_COMMON_DESERIALIZE_H_

#include <halo/lib/hall/hallerr.h>
#include <halo/lib/hall/appv1/common_types.h>
#include <halo/lib/hall/appv1/cmd.h>
#include <appv1/wrappers/on_message.h>

#ifdef __cplusplus
extern "C" {
#endif

// CMD Header

sid_error_t halo_hall_appv1_cmd_from_flex_deserialize(const uint8_t* buffer, const size_t len, halo_all_cmd_t*, hall_size_t* out_size);
sid_error_t halo_hall_appv1_cmd_exheader_deserialize(const uint8_t* buffer, const size_t len, halo_all_cmd_t*, hall_size_t* out_size);

// utility functions

sid_error_t halo_hall_appv1_raw_buffer_deserialize(const uint8_t* buffer, const size_t len, halo_hall_appv1_raw_buffer_t*, hall_size_t* out_size);
sid_error_t halo_hall_appv1_uint8_deserialize(const uint8_t* buffer, const size_t len, uint8_t* val, hall_size_t* out_size);
sid_error_t halo_hall_appv1_uint16_deserialize(const uint8_t* buffer, const size_t len, uint16_t* val, hall_size_t* out_size);
sid_error_t halo_hall_appv1_uint32_deserialize(const uint8_t* buffer, const size_t len, uint32_t* pld, hall_size_t* out_size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_INCLUDE_APPV1_COMMON_DESERIALIZE_H_ */
