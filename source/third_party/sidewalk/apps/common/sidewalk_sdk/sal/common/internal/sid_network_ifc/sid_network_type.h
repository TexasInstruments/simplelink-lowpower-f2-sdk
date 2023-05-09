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

#ifndef SID_NETWORK_TYPE_H
#define SID_NETWORK_TYPE_H

#include <halo/lib/hall/appv1/mgm/types.h>
#include <sid_network_address.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SID_MAX_GROUP_NUM_COUNT            8   // = RNET_MAX_GROUP_NUM_COUNT

typedef struct {
    uint32_t                                ka_interval_s;
    halo_hall_appv1_pan_id_t                pan_id;
    uint8_t                                 dev_id[SID_ADDRESS_SIZE_MAX];
    uint8_t                                 group_id[SID_MAX_GROUP_NUM_COUNT];
    uint8_t                                 pairing_state;
    uint8_t                                 configuration_state;
    uint8_t                                 p2p_channel;
} sid_ni_config_t;

#ifdef __cplusplus
}
#endif

#endif /* SID_NETWORK_TYPE_H */
