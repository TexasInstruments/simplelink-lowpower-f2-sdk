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

#ifndef SALL_MGM_CORE_FACTORY_RESET_H
#define SALL_MGM_CORE_FACTORY_RESET_H

#include <sall_mgm_core_lib.h>

#include <sid_time_types.h>

#ifdef __cplusplus
extern "C" {
#endif

sid_error_t sall_mgm_core_factory_reset_event_handler(struct sid_event *event, void *data);
sid_error_t on_sall_mgm_core_factory_reset_set(const halo_hall_appv1_mgm_core_ifc *ifc,
                                               halo_hall_message_descriptor_t *mdesc,
                                               const struct sall_mgm_factory_reset *reset);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif /* SALL_MGM_CORE_FACTORY_RESET_H */
