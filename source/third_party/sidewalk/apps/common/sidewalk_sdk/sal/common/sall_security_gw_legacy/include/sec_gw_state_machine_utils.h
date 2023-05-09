/*
* Copyright 2021-2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
*
* AMAZON PROPRIETARY/CONFIDENTIAL
*
* You may not use this file except in compliance with the terms and conditions
* set forth in the accompanying LICENSE.TXT file.
*
* THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
* DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
*/

#ifndef SEC_GW_STATE_MACHINE_UTILS_H
#define SEC_GW_STATE_MACHINE_UTILS_H

#include <sid_error.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct in_out_offsets {
    uint8_t *in_data;
    size_t in_size;
    uint8_t *out_data;
    size_t out_size;
    size_t len;
};

sid_error_t get_app_in_out_offset_without_payload_size(struct in_out_offsets *const offsets);

#ifdef __cplusplus
}
#endif

#endif /*! SEC_GW_STATE_MACHINE_UTILS_H */
