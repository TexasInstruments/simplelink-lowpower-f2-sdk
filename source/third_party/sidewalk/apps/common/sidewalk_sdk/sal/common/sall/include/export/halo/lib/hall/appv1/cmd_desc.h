/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef HALO_LIB_HALL_APPV1_CMD_DESC_H
#define HALO_LIB_HALL_APPV1_CMD_DESC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hall_cmd_descriptor_s {
    uint16_t cls;
    uint16_t id;
    uint8_t type;
    uint8_t version;
    uint8_t priority;
} hall_cmd_descriptor_t;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* HALO_LIB_HALL_APPV1_CMD_DESC_H */
