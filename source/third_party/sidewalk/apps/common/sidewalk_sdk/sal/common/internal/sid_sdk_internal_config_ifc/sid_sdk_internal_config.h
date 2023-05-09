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

#ifndef SID_SDK_INTERNAL_CONFIG_H
#define SID_SDK_INTERNAL_CONFIG_H

#include <sid_utils.h>

#if IS_ENABLED(SID_GW_BUILD) || IS_ENABLED(GW_SUPPORT)
#include <sid_sdk_gw_config.h>
#else
#include <sid_sdk_config.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Configure Link Type 2
#ifndef SID_MAC_FSK_ENABLED
#define SID_MAC_FSK_ENABLED SID_SDK_CONFIG_ENABLE_LINK_TYPE_2
#endif

#ifndef SID_FSK_WAN_SUPPORT
#define SID_FSK_WAN_SUPPORT SID_SDK_CONFIG_ENABLE_LINK_TYPE_2
#endif

// Configure Link type 3
#ifndef SID_MAC_LDR_ENABLED
#define SID_MAC_LDR_ENABLED SID_SDK_CONFIG_ENABLE_LINK_TYPE_3
#endif

// Set common flags
#if SID_SDK_CONFIG_ENABLE_LINK_TYPE_2 || SID_SDK_CONFIG_ENABLE_LINK_TYPE_3
#ifndef WAN_SUPPORT
#define WAN_SUPPORT 1
#endif
#endif

// Undefine defaults defined in protocol
// These are sidewalk sdk defaults
#ifndef HALO_GCS_DISABLE_P2P
#define HALO_GCS_DISABLE_P2P 1
#endif

#ifndef RNET_SECURITY_BUILD
#define RNET_SECURITY_BUILD 1
#endif

#ifndef RNET_LOG_RAW_PKT
#define RNET_LOG_RAW_PKT 0
#endif

#ifndef SYSTEM_RST_REPORTS
#define SYSTEM_RST_REPORTS 0
#endif

#ifndef DFU_SETTINGS_ENABLED
#define DFU_SETTINGS_ENABLED 0
#endif

#ifndef SID_ENABLE_SALL_SECURITY
#define SID_ENABLE_SALL_SECURITY 1
#endif

#ifndef PROTOCOL_STANDALONE_BUILD
#define PROTOCOL_STANDALONE_BUILD 1
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SID_SDK_INTERNAL_CONFIG_H */
