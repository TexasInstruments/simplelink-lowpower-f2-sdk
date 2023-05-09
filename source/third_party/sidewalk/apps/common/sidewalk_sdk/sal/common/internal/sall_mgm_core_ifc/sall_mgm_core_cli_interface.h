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

#ifndef SALL_MGM_CORE_CLI_INTERFACE_H_
#define SALL_MGM_CORE_CLI_INTERFACE_H_

#include <sall_mgm_core_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sall_mgm_core_handle *sall_mgm_core_library_instance_get(void);
void sall_mgm_core_on_cli_config_set_helper(const sall_mgm_core_config_id_t id, void *data);
void sall_mgm_core_gcs_on_cli_time_set(struct sid_timespec *time, struct sid_timespec *drift);
void sall_mgm_core_gcs_on_cli_time_get(const struct sid_address *addr);
void sall_mgm_core_gcs_on_cli_config_set(const sall_mgm_gcs_config_t *config);
void sall_mgm_core_gcs_on_cli_config_get(sall_mgm_gcs_config_t *config);
void sall_mgm_core_gcs_on_cli_config_get_defaulte(sall_mgm_gcs_config_t *config);
void sall_mgm_core_gcs_on_cli_time_notify(void);
const char *sall_mgm_core_gcs_on_cli_state_get(void);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif /* SALL_MGM_CORE_CLI_INTERFACE_H_ */
