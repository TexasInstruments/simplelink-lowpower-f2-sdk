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

#ifndef RNETC_INCLUDE_APPV1_MGM_DESERIALIZE_H_
#define RNETC_INCLUDE_APPV1_MGM_DESERIALIZE_H_

#include <halo/lib/hall/hallerr.h>
#include <halo/lib/hall/appv1/mgm_diagnostics/types.h>
#include <halo/lib/hall/appv1/cmd.h>
#include <appv1/mgm_core/deserialize.h>

#ifdef __cplusplus
extern "C" {
#endif

sid_error_t halo_hall_appv1_mgm_start_lq_deserialize(
    const uint8_t* buffer, const size_t len, halo_hall_appv1_lq_test_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_mgm_start_lq_results_deserialize(
    const uint8_t* buffer, const size_t len, halo_hall_appv1_lq_test_results_t* data, hall_size_t* out_size);
sid_error_t halo_hall_appv1_mgm_ping_deserialize(const uint8_t* buffer, const size_t len, halo_hall_appv1_ping_t* data, hall_size_t* out_size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_INCLUDE_APPV1_MGM_DESERIALIZE_H_ */
