/*
 * Copyright 2021-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_PROTOCOL_OPTS_H
#define SID_PROTOCOL_OPTS_H

// Does not effect existing products
#if PROTOCOL_STANDALONE_BUILD
// Only is enabled for gateways
#if SID_PROTOCOL_OPTS_PROVIDED
#include <sid_protocol_ext_opts.h>
#else
#include <sid_sdk_internal_config.h>
#endif
#endif

#include <sid_utils.h>

/*
 * @brief: Distinguish between gateway and peripheral firmware
 */

#ifndef GW_SUPPORT
#define GW_SUPPORT 0
#endif

/*
 * @brief: Protocol configuration  flag to control
 * the use of LDR submodule: i.e.: some product family
 * may have LDR disabled (eg lights). In this case, the project
 * configuration file of those products will not include this flag
 * and consequently be assigned 0. This flag will enable and compile
 * in LDR code when set to 1. This flag will disable and compile out
 * LDR code when set to 0.
 */

#ifndef SID_MAC_LDR_ENABLED
#define SID_MAC_LDR_ENABLED 0
#endif

/*
 * @brief: LDR channel access timing design aiming to:
 * 1. Allow devices to transmit and receive without colliding with each other
 *    due to overlap of the events in time.
 * 2. Allow devices to retry packet transmissions in both directions with lower
 *    latency.
 */

#ifndef LDR_TIME_DIVERSITY
#define LDR_TIME_DIVERSITY 0
#endif

/*
 * @brief: Enable AAD extension to include LDR header fields
 *
 *  This is for feature integration, it will only be applied when end-to-end
 *  is ready. At that time, RNET_RF_PROTOCOL_VERSION will be incremented to
 *  indicate the change and SID_LL_RF_AAD_EXTENSION will then be removed.
 */

#ifndef SID_LL_RF_AAD_EXTENSION
#define SID_LL_RF_AAD_EXTENSION 0
#endif

/*
 * @brief: Protocol configuration flag to control
 * the use of dynamic TX_UUID
 */
#ifndef HALO_PROTCL_USE_UUID
#define HALO_PROTCL_USE_UUID 1
#endif

/*
 * @brief: Enable support for PAN
 */
#ifndef PAN_SUPPORT
#define PAN_SUPPORT 0
#endif

#if PAN_SUPPORT || GW_SUPPORT
#undef HALO_PROTCL_USE_UUID
#define HALO_PROTCL_USE_UUID 0
#endif

/*
 * @brief: Enable the join process
 */

#ifndef HALO_JOIN_PROCESS
#define HALO_JOIN_PROCESS 1
#endif

/*
 * @brief: Enable FSK Data Rate
 */

#ifndef RNET_MAC_MODE_FSK
#define RNET_MAC_MODE_FSK 1
#endif

/*
 * @brief: Enable FSK feature.
 *
 * FSK feature can be disabled only on EN.
 * FSK feature is always enabled by default.
 * If FSK feature has to be disabled on an EN
 * platform, below flag has to set to a value
 * of zero in that platform specific makefile
 * or config file.
 */
#ifndef SID_MAC_FSK_ENABLED
#define SID_MAC_FSK_ENABLED 1
#endif

/*
 * @brief: Enable FSK feature.
 *
 * On GW, FSK feature should always enabled.
 * If FSK feature has been disabled, throw
 * an error.
 */
#if defined(GW_SUPPORT) && GW_SUPPORT
#if !SID_MAC_FSK_ENABLED
#error FSK cannot be disabled on GW
#endif
#endif

/*
 * @brief: FSK WAN support
 */

#ifndef SID_FSK_WAN_SUPPORT
#define SID_FSK_WAN_SUPPORT 0
#endif

/*
 * @brief: Disable time sync over PAN
 */

#ifndef HALO_GCS_DISABLE_P2P
#define HALO_GCS_DISABLE_P2P 0
#endif

/*
 * @brief: Carry seed info
 */

#ifndef RNET_MAC_PROBE_CARRY_SEED_INFO
#define RNET_MAC_PROBE_CARRY_SEED_INFO 0
#endif


/*
 * @brief: GW Wakeup schedule behavior
 */

#ifndef RNET_GW_DEFAULT_WAKEUP_PERIOD
#define RNET_GW_DEFAULT_WAKEUP_PERIOD 1
#endif

#ifndef RNET_GW_DEFAULT_WAKEUP_OFFSET
#define RNET_GW_DEFAULT_WAKEUP_OFFSET 0
#endif

/*
 * @brief: End-node assumption for CS slots for GWs with undefined CS slots (non matching Beacon format) in its TLV
 */

#ifndef RNET_FSK_WAN_CS_UL_PERIOD_FOR_LEGACY_HALO_GW
#define RNET_FSK_WAN_CS_UL_PERIOD_FOR_LEGACY_HALO_GW 8
#endif

#ifndef RNET_FSK_WAN_CS_UL_OFFSET_FOR_LEGACY_HALO_GW
#define RNET_FSK_WAN_CS_UL_OFFSET_FOR_LEGACY_HALO_GW 50
#endif

#ifndef RNET_FSK_WAN_CS_DL_PERIOD_FOR_LEGACY_HALO_GW
#define RNET_FSK_WAN_CS_DL_PERIOD_FOR_LEGACY_HALO_GW RNET_GW_DEFAULT_WAKEUP_PERIOD
#endif

#ifndef RNET_FSK_WAN_CS_DL_OFFSET_FOR_LEGACY_HALO_GW
#define RNET_FSK_WAN_CS_DL_OFFSET_FOR_LEGACY_HALO_GW RNET_GW_DEFAULT_WAKEUP_OFFSET
#endif

/*
 * @brief: Enable Serial host MCU interface (SPI/UART)
 */

#ifndef RNET_SRL_HOST_INTERFACE
#define RNET_SRL_HOST_INTERFACE 0
#endif

/*
 * @brief: Serial host interface will use SPI
 */

#ifndef RNET_HOST_SPI_MCU
#define RNET_HOST_SPI_MCU 0
#endif

/*
 * @brief: Compile with spi master commands to test spi host interface
 */

#ifndef RNET_SPI_MASTER_DEBUG_BUILD
#define RNET_SPI_MASTER_DEBUG_BUILD 0
#endif

/*
 * @brief: Enable protocol security
 */

#ifndef RNET_SECURITY_BUILD
#define RNET_SECURITY_BUILD 0
#endif

/*
 * @brief: Debug message with received RAW packet
 */

#ifndef RNET_LOG_RAW_PKT
#define RNET_LOG_RAW_PKT 1
#endif

/*
 * @brief: BLE NUS Enabled
 */

#ifndef BLE_NUS_ENABLED
#define BLE_NUS_ENABLED 0
#endif

/*
 * Link Layer Logging Options
 */

#ifndef RNET_LL_CONFIG_LOG_ENABLED
#define RNET_LL_CONFIG_LOG_ENABLED 1
#endif

#ifndef RNET_LL_CONFIG_LOG_LEVEL
#define RNET_LL_CONFIG_LOG_LEVEL 4
#endif

#ifndef RNET_LL_CONFIG_INFO_COLOR
#define RNET_LL_CONFIG_INFO_COLOR 0
#endif

#ifndef RNET_LL_CONFIG_DEBUG_COLOR
#define RNET_LL_CONFIG_DEBUG_COLOR 0
#endif

/*
 * Network Layer Logging Options
 */
#ifndef RNET_NWL_CONFIG_LOG_ENABLED
#define RNET_NWL_CONFIG_LOG_ENABLED 1
#endif

#ifndef RNET_NWL_CONFIG_LOG_LEVEL
#define RNET_NWL_CONFIG_LOG_LEVEL 4
#endif

#ifndef RNET_NWL_CONFIG_INFO_COLOR
#define RNET_NWL_CONFIG_INFO_COLOR 0
#endif

#ifndef RNET_NWL_CONFIG_DEBUG_COLOR
#define RNET_NWL_CONFIG_DEBUG_COLOR 0
#endif

/*
 * Security Logging Options
 */
#ifndef RNET_SEC_CONFIG_LOG_ENABLED
#define RNET_SEC_CONFIG_LOG_ENABLED 1
#endif

#ifndef RNET_SEC_CONFIG_LOG_LEVEL
#define RNET_SEC_CONFIG_LOG_LEVEL 4
#endif

#ifndef RNET_SEC_CONFIG_INFO_COLOR
#define RNET_SEC_CONFIG_INFO_COLOR 0
#endif

#ifndef RNET_SEC_CONFIG_DEBUG_COLOR
#define RNET_SEC_CONFIG_DEBUG_COLOR 0
#endif

/*
 * Transaction Store Logging Options
 */
#ifndef TR_STORE_CONFIG_LOG_ENABLED
#define TR_STORE_CONFIG_LOG_ENABLED 1
#endif

#ifndef TR_STORE_CONFIG_LOG_LEVEL
#define TR_STORE_CONFIG_LOG_LEVEL 4
#endif

#ifndef TR_STORE_CONFIG_INFO_COLOR
#define TR_STORE_CONFIG_INFO_COLOR 0
#endif

#ifndef TR_STORE_CONFIG_DEBUG_COLOR
#define TR_STORE_CONFIG_DEBUG_COLOR 0
#endif

#ifndef SYSTEM_RST_REPORTS
#define SYSTEM_RST_REPORTS    1
#endif

/*
 * Feature to pass dfu settings from app to bootloader berfore DFU start
 */

#ifndef DFU_SETTINGS_ENABLED
#define DFU_SETTINGS_ENABLED   1
#endif

/*
 * @brief: Priority with which BLE events are dispatched to the RingNet Service.
 */

#ifndef BLE_RNET_OBSERVER_PRIO
#define BLE_RNET_OBSERVER_PRIO 2
#endif

/*
 * Mac scheduler settings
 */

/*
 * Build and release version tags
 * TODO: Only used by app layer, remove when using SALL
 */
#ifndef RING_APP_MAJOR_VERSION
#define RING_APP_MAJOR_VERSION 0
#endif

#ifndef RING_APP_MINOR_VERSION
#define RING_APP_MINOR_VERSION 0
#endif

#ifndef RING_APP_PATCH_VERSION
#define RING_APP_PATCH_VERSION 0
#endif

#ifndef RING_APP_BUILD_VERSION
#define RING_APP_BUILD_VERSION 0
#endif

#ifndef RELEASE_TAG
#define RELEASE_TAG "na"
#endif

#ifndef OTA_POOL
#define OTA_POOL "na"
#endif

#ifndef BUILD_VARIANT
#define BUILD_VARIANT 0
#endif

#ifndef HALO1_MIGRATION
#define HALO1_MIGRATION 0
#endif

#ifndef SID_GW_BUILD
#define SID_GW_BUILD 0
#endif

#ifndef SID_ENABLE_SALL_SECURITY
#define SID_ENABLE_SALL_SECURITY 0
#endif

#endif
