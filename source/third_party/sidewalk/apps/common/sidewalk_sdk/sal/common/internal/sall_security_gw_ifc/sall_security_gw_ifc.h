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

#ifndef SALL_SECURITY_GW_IFC_H
#define SALL_SECURITY_GW_IFC_H

#include <halo/lib/hall/interface.h>
#include <sid_event_queue_ifc.h>

#include <sid_error.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sall_security_gw_user_cb {
    void *user_ctx;
    void (*on_device_registration_state_changed)(void *user_ctx, bool is_registered);
};

struct sall_security_gw_params {
    uint32_t timeout_seconds;
    struct sid_event_queue *event_queue;
    struct sid_address address;
    struct sall_security_gw_user_cb user_cb;
};

sid_error_t sall_security_gw_init(const struct sall_security_gw_params *const params,
                                  const halo_hall_lib_ctx_t hall_lib_ctx);

void sall_security_gw_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /*! SALL_SECURITY_GW_IFC_H */
