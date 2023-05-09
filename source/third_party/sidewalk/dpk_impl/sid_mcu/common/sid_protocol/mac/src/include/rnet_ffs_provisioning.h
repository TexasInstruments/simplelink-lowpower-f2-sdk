/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef RNET_FFS_PROVISIONING_H
#define RNET_FFS_PROVISIONING_H
#include <sid_protocol_opts.h>

#if !GW_SUPPORT && RNET_SECURITY_BUILD && HALO_FFS_PROVISIONING
/**
 * @brief Function to initialize ffs provisioning process
 */
void rnet_ffs_prov_init(void);
/**
 * @brief Function to process ffs provisioning events
 */
void rnet_ffs_prov_process(void);
/**
 * @brief Function to set provisioning init resp result
 *
 * @param[in]  result  Result of resp of provisioning init
 *
 */
void rnet_ffs_prov_request_set_result(bool result);
/**
 * @brief Function to notify ffs provisioning cmd eror
 */
void rnet_ffs_prov_notify_error(void);
/**
 * @brief Function to indicate provisioning is complete
 */
void rnet_ffs_prov_set_config_complete(void);
/**
 * @brief Function to check whether FFS provisioning is active
 *
 * @retval  true if FFS provisioning is active
 */
bool rnet_ffs_prov_is_active(void);
/**
 * @brief Function to disable FFS provisioning attempts
 */
void rnet_ffs_prov_disable(void);
#endif
#endif
