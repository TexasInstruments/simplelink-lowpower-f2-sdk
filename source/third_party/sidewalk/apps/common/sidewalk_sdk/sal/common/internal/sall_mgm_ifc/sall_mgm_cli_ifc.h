/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SALL_MGM_CLI_IFC_H
#define SALL_MGM_CLI_IFC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sall_mgm_types.h>

void sall_mgm_on_cli_set_helper(const sall_mgm_config_id_t id, void * data);
void sall_mgm_on_cli_get_helper(const sall_mgm_config_id_t id, void * data);
void sall_mgm_on_cli_factory_reset(void);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* SALL_MGM_CLI_IFC_H */
