/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef RNET_NW_LAYER_H
#define RNET_NW_LAYER_H

#include <rnet_errors.h>
#include "rnet_frames.h"
#include "rnet_time.h"
#include "rnet_app_layer.h"
#include "rnet_app_sup_layer.h"
#include <sid_protocol_defs.h>
#include <sid_protocol_opts.h>
#include "rnet_transact_store.h"
#include "rnet_dev_addr_def.h"

#define RNET_TX_BUF_SZ                  128


// Macros for different commands in RINGNET_MGMT command class
#define CMD_ID_RNET_NWL_KA                      ((0x001 << 2) | RNET_ASL_OPC_NTFY)

#define CMD_ID_RNET_SET_NG_DISCOVERY            ((0x002 << 2) | RNET_ASL_OPC_WR)

#define CMD_ID_RNET_GW_ANNOUNCEMENT             ((0x003 << 2) | RNET_ASL_OPC_NTFY)

#define CMD_ID_RNET_PING                        ((0xFF << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_RNET_PONG                        ((0xFF << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_P2P_CHNL                ((0x100 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_GET_P2P_CHNL                ((0x100 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_P2P_CHNL               ((0x100 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_GET_FW_VERSION              ((0x101 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_FW_VERSION             ((0x101 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_GET_SERIAL_NUM              ((0x102 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_SERIAL_NUM             ((0x102 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_PAN_ID                  ((0x103 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_GET_PAN_ID                  ((0x103 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_PAN_ID                 ((0x103 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_P2P_BROADCAST_KEY       ((0x104 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_GET_P2P_BROADCAST_KEY_ST    ((0x104 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_P2P_BROADCAST_KEY_ST   ((0x104 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_START_DFU                   ((0x105 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_START_DFU              ((0x105 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_RNET_NTFY_START_DFU              ((0x105 << 2) | RNET_ASL_OPC_NTFY)

#define CMD_ID_RNET_STOP_DFU                    ((0x119 << 2) | RNET_ASL_OPC_WR)

#define CMD_ID_RNET_SET_P2P_UNICAST_KEY         ((0x106 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_GET_P2P_UNICAST_KEY_ST      ((0x106 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_P2P_UNICAST_KEY_ST     ((0x106 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_MGMT_RESET_REPORT           ((0x107 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_RNET_GET_RESET_INFO              ((0x107 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_RESET_INFO             ((0x107 << 2) | RNET_ASL_OPC_RSP)


#define CMD_ID_RNET_SET_TIME                    ((0x108 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_GET_TIME                    ((0x108 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_NOTIFY_TIME                 ((0x108 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_RNET_RESP_TIME                   ((0x108 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_MULTICAST_RETRIES       ((0x109 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_SET_MULTICAST_RETRIES       ((0x109 << 2) | RNET_ASL_OPC_WR)

#define CMD_ID_RNET_SET_DBG_DEV_ID              ((0x180 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_GET_DBG_DEV_ID              ((0x180 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_DBG_DEV_ID             ((0x180 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_GROUP_NUM               ((0x10A << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_GET_GROUP_NUM               ((0x10A << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_GROUP_NUM              ((0x10A << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_KA_INTERVAL             ((0x10E << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_GET_KA_INTERVAL             ((0x10E << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_KA_INTERVAL            ((0x10E << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_START_LQ_TEST               ((0x10F << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_START_LQ_TEST          ((0x10F << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_RNET_STOP_LQ_TEST                ((0x110 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_STOP_LQ_TEST           ((0x110 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_RNET_GET_LQ_TEST_RESULT          ((0x128 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_LQ_TEST_RESULT         ((0x128 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_RNET_NOTIFY_LQ_TEST_RESULT       ((0x128 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_RNET_START_LQ_TEST2              ((0x130 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_START_LQ_TEST2         ((0x130 << 2) | RNET_ASL_OPC_RSP)


#define CMD_ID_RNET_DIAGNOSTIC_TEST_START       ((0x127 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_DIAGNOSTIC_TEST        ((0x127 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_RNET_NOTIFY_DIAGNOSTIC_TEST      ((0x127 << 2) | RNET_ASL_OPC_NTFY)

#define CMD_ID_RNET_GET_DEV_ID                  ((0x111 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_DEV_ID                 ((0x111 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_RESET                   ((0x112 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_RESET                  ((0x112 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_RNET_NOTIFY_RESET                ((0x112 << 2) | RNET_ASL_OPC_NTFY)

#define CMD_ID_RNET_SET_RF_POWER                ((0x113 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_GET_RF_POWER                ((0x113 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_RF_POWER               ((0x113 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_GET_BRIDGE_CLOUD_AUTH_TOKEN      ((0x114 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_BRIDGE_CLOUD_AUTH_TOKEN     ((0x114 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_GET_PROTOCOL_VERSION        ((0x121 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_PROTOCOL_VERSION       ((0x121 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_PAIRING_STATE           ((0x122 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_GET_PAIRING_STATE           ((0x122 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_PAIRING_STATE          ((0x122 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_GET_FIRMWARE_OTA_PARAMETERS ((0x123 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_FIRMWARE_OTA_PARAMETERS ((0x123 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_CAD_PARAMS              ((0x124 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_GET_CAD_PARAMS              ((0x124 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_CAD_PARAMS             ((0x124 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_VENDOR_DFU_PKT              ((0x125 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_VENDOR_DFU_PKT         ((0x125 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_RNET_NOTIFY_VENDOR_DFU_PKT       ((0x125 << 2) | RNET_ASL_OPC_NTFY)

#define CMD_ID_RNET_GET_RF_SECURITY_SETTINGS    ((0x126 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_SET_RF_SECURITY_SETTINGS    ((0x126 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_RF_SECURITY_SETTINGS   ((0x126 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_GET_HW_ID                   ((0x129 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_HW_ID                  ((0x129 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_WIFI_MAC                ((0x205 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_GET_WIFI_MAC                ((0x205 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_WIFI_MAC               ((0x205 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_ETH_MAC                 ((0x206 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_GET_ETH_MAC                 ((0x206 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_ETH_MAC                ((0x206 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_MFG_PROXY_FLAG          ((0x210 << 2) | RNET_ASL_OPC_WR)

#define CMD_ID_RNET_FAKE_EVENT                  ((0x215 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_RNET_INJECT_FAKE_EVENT           ((0x215 << 2) | RNET_ASL_OPC_WR)

#define CMD_ID_RNET_GET_UPTIME                  ((0x216 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_UPTIME                 ((0x216 << 2) | RNET_ASL_OPC_NTFY)

#define CMD_ID_RNET_START_CHANNEL_SURVEY        ((0x217 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_CHANNEL_SURVEY         ((0x217 << 2) | RNET_ASL_OPC_RSP)

/**************Deprecated Halo 1.0 command Ids , Please do not reuse***********/
#define CMD_ID_RNET_JOIN_REQ                    ((0x218 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_JOIN_RSP                    ((0x218 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_SET_JOIN_PARAMS                  ((0x219 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_GET_JOIN_PARAMS                  ((0x219 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RESP_JOIN_PARAMS                 ((0x219 << 2) | RNET_ASL_OPC_RSP)
/*******************************************************************************/

#define CMD_ID_GET_BRIDGE_KA_INTERVAL           ((0x21A << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RSP_BRIDGE_KA_INTERVAL           ((0x21A << 2) | RNET_ASL_OPC_RSP)

/**************Deprecated Halo 1.0 command Ids , Please do not reuse***********/
#define CMD_ID_RNET_WAKEUP_SLEEPY               ((0x21B << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RSP_WAKE_UP_SLEEPY          ((0x21B << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_SET_SD_PARAMS                    ((0x21C << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_GET_SD_PARAMS                    ((0x21C << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RESP_SD_PARAMS                   ((0x21C << 2) | RNET_ASL_OPC_RSP)
/*******************************************************************************/


#define CMD_ID_GET_DEV_PROFILE                  ((0x21D << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_SET_DEV_PROFILE                  ((0x21D << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RSP_DEV_PROFILE                  ((0x21D << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_GET_BCN_MISS                     ((0x21E << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_SET_BCN_MISS                     ((0x21E << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RSP_BCN_MISS                     ((0x21E << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_GET_BCN_INTVL                    ((0x21F << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_SET_BCN_INTVL                    ((0x21F << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RSP_BCN_INTVL                    ((0x21F << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_GET_LDR_CHNL                     ((0x220 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_SET_LDR_CHNL                     ((0x220 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RSP_LDR_CHNL                     ((0x220 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_GET_DFCT_LOGIC                   ((0x221 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_SET_DFCT_LOGIC                   ((0x221 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RSP_DFCT_LOGIC                   ((0x221 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_GET_PROBE_AUTH                   ((0x222 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_SET_PROBE_AUTH                   ((0x222 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RSP_PROBE_AUTH                   ((0x222 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_GET_PRTCL_FLAG                   ((0x223 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_SET_PRTCL_FLAG                   ((0x223 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RSP_PRTCL_FLAG                   ((0x223 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_GET_SIDEWALK_NW_FLAG             ((0x225 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_SET_SIDEWALK_NW_FLAG             ((0x225 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RSP_SIDEWALK_NW_FLAG             ((0x225 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_NTFY_PROXY_RESP_TIME             ((0x226 << 2) | RNET_ASL_OPC_NTFY)

#define CMD_ID_GET_PRTCL_CFG                    ((0x227 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RSP_PRTCL_CFG                    ((0x227 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_GET_HDR_SLOT_PARAMS              ((0x228 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_SET_HDR_SLOT_PARAMS              ((0x228 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RSP_HDR_SLOT_PARAMS              ((0x228 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_CONFIGURATION_COMPLETE  ((0x229 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_CONFIGURATION_COMPLETE ((0x229 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_SET_DEV_STATUS_ANNOUNCEMENT      ((0x22A << 2) | RNET_ASL_OPC_WR)   //https://wiki.labcollab.net/confluence/pages/viewpage.action?pageId=904994323
#define CMD_ID_RSP_DEV_STATUS_ANNOUNCEMENT      ((0x22A << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_SET_DEDICATED_LINK_INIT          ((0x22B << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RSP_DEDICATED_LINK_INIT          ((0x22B << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_DEDICATED_LINK_HND_SHK_REQ       ((0x22C << 2) | RNET_ASL_OPC_WR)

#define CMD_ID_SET_DISTRESS_GWD_PARAMS          ((0x22D << 2) | RNET_ASL_OPC_WR)

#define CMD_ID_GET_TMP_PRTCL_EXP                ((0x22E << 2) | RNET_ASL_OPC_RD) /* command for temporary protocol configuration needs */
#define CMD_ID_SET_TMP_PRTCL_EXP                ((0x22E << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RSP_TMP_PRTCL_EXP                ((0x22E << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_JOIN_PROC_REQ               ((0x22F << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_JOIN_PROC_RESP              ((0x22F << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_RNET_JOIN_PROC_AUTH ((0xD << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_SCH_ASSIGN_REQ ((0xE << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_SCH_ASSIGN_RESP ((0xE << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_RNET_SCH_LOOKUP_REQ ((0xE << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_SCH_LOOKUP_RESP CMD_ID_RNET_SCH_ASSIGN_RESP
#define CMD_ID_RNET_SCH_EXTENSION_REQ ((0xE << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_RNET_SCH_TERM_REQ ((0xF << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_SCH_TERM_RESP ((0xF << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_RNET_SCH_TERM_NTFY ((0xF << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_RNET_CS_SLOT_REQ ((0x10 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_CS_SLOT_RESP ((0x10 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_RNET_CS_SLOT_WRITE ((0x10 << 2) | RNET_ASL_OPC_WR)


// Macros for different commands in BATTERY command class
// TODO: move to BATTERY class
#define CMD_ID_BATTERY_GET_VOLTAGE              ((0x00 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_BATTERY_RESP_VOLTAGE             ((0x00 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_BATTERY_NOTIFY_VOLTAGE           ((0x00 << 2) | RNET_ASL_OPC_NTFY)

#define CMD_ID_BATTERY_GET_LEVEL                ((0x01 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_BATTERY_RESP_LEVEL               ((0x01 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_BATTERY_NOTIFY_LEVEL             ((0x01 << 2) | RNET_ASL_OPC_NTFY)

#define CMD_ID_BATTERY_GET_USAGE_REPORT         ((0x02 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_BATTERY_RESP_USAGE_REPORT        ((0x02 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_BATTERY_NOTIFY_USAGE_REPORT      ((0x02 << 2) | RNET_ASL_OPC_NTFY)

#define CMD_ID_RNET_GET_STATUS                  ((0x006 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_STATUS                 ((0x006 << 2) | RNET_ASL_OPC_RSP)
// Halo GATEWAY command class
#define CMD_ID_NTFY_RX_PKT                      ((0x00 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_SEND_PKT                         ((0x01 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_SEND_PKT_NTFY                    ((0x01 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_NTFY_RX_DFU_PKT                  ((0x02 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_SEND_DFU_PKT                     ((0x03 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_SET_DFU_PARAMS                   ((0x04 << 2) | RNET_ASL_OPC_WR)

#define CMD_ID_SET_ZWAVE_TX_FLAG                ((0x05 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RESP_ZWAVE_TX_FLAG               ((0x05 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_GET_ZWAVE_TX_FLAG                ((0x05 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_SET_ZWAVE_THRESHOLD              ((0x06 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_GET_ZWAVE_THRESHOLD              ((0x06 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RESP_ZWAVE_THRESHOLD             ((0x06 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_RNET_GET_DFU_ZWAVE_METRICS       ((0x07 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_DFU_ZWAVE_METRICS      ((0x07 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_RNET_NOTIFY_DFU_ZWAVE_METRICS    ((0x07 << 2) | RNET_ASL_OPC_NTFY)

// Halo LOGGING class
#define CMD_ID_SNIFFER_ENABLE                   ((0x00 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RESP_SNIFFER_ENABLE              ((0x00 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_SNIFFER_DISABLE                  ((0x01 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RESP_SNIFFER_DISABLE             ((0x01 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_NOTIFY_SNIFFER_RX_PKT            ((0x02 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_NOTIFY_SNIFFER_TX_PKT            ((0x03 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_ENABLE_DEVICE_LOGS               ((0x04 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RESP_ENABLE_DEVICE_LOGS          ((0x04 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_DISABLE_DEVICE_LOGS              ((0x05 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RESP_DISABLE_DEVICE_LOGS         ((0x05 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_SET_MODULE_LOGGING               ((0x06 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RESP_SET_MODULE_LOGGING          ((0x06 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_NOTIFY_POTENTIAL_COV_GAP         ((0x09 << 2) | RNET_ASL_OPC_NTFY)
// HALO LINK METRICS class
#define CMD_ID_METRICS_GET_P2P_METRICS          ((0x01 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_METRICS_RESP_P2P_METRICS         ((0x01 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_METRICS_NOTIFY_P2P_METRICS       ((0x01 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_METRICS_NOTIFY_LATENCY           ((0x02 << 2) | RNET_ASL_OPC_NTFY)

//Enhanced P2P Metrics
#define CMD_ID_GET_ENHANCED_P2P_METRICS         ((0x03 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RESP_ENHANCED_P2P_METRICS        ((0x03 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_NTFY_ENAHNCED_P2P_METRICS        ((0x03 << 2) | RNET_ASL_OPC_NTFY)

//Enhanced Metrics - Scheduler Metrics Collection
#define CMD_ID_METRICS_GET_SCHED_METRICS        ((0x04 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_METRICS_NTFY_SCHED_METRICS       ((0x04 << 2) | RNET_ASL_OPC_NTFY)

//Enhanced Metrics - Noise Floor collection
#define CMD_ID_METRICS_GET_NOISE_FLOOR          ((0x05 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_METRICS_RESP_NOISE_FLOOR         ((0x05 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_METRICS_NOTIFY_NOISE_FLOOR       ((0x05 << 2) | RNET_ASL_OPC_NTFY)

#define CMD_ID_METRICS_GET_LDR_METRICS          ((0x06 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_METRICS_RESP_LDR_METRICS         ((0x06 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_METRICS_NTFY_LDR_METRICS         ((0x06 << 2) | RNET_ASL_OPC_NTFY)

//Enhanced P2P Metrics from gateway
#define CMD_ID_GET_EP2P_GW_METRICS              ((0x07 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RESP_EP2P_GW_METRICS             ((0x07 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_NTFY_EP2P_GW_METRICS             ((0x07 << 2) | RNET_ASL_OPC_NTFY)

//Enhanced P2P Metrics from end node
#define CMD_ID_GET_EP2P_EN_METRICS              ((0x08 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RESP_EP2P_EN_METRICS             ((0x08 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_NTFY_EP2P_EN_METRICS             ((0x08 << 2) | RNET_ASL_OPC_NTFY)

//LDR End node Metrics updated version
#define CMD_ID_GET_LDR_METRICS                  ((0x09 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RESP_LDR_METRICS                 ((0x09 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_NTFY_LDR_METRICS                 ((0x09 << 2) | RNET_ASL_OPC_NTFY)

// LDR PHY Improvement Metrics Part 1 from end node
#define CMD_ID_GET_EN_LDR_PHY_IMP1_METRICS ((0x0A << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RESP_EN_LDR_PHY_IMP1_METRICS ((0x0A << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_NTFY_EN_LDR_PHY_IMP1_METRICS ((0x0A << 2) | RNET_ASL_OPC_NTFY)

// LDR PHY Improvement Metrics from gateway
#define CMD_ID_GET_GW_LDR_PHY_IMP_METRICS ((0x0B << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RESP_GW_LDR_PHY_IMP_METRICS ((0x0B << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_NTFY_GW_LDR_PHY_IMP_METRICS ((0x0B << 2) | RNET_ASL_OPC_NTFY)

//CMD_CLASS_SECURITY (0x5)
#define CMD_ID_RNET_SET_WAN_SEC_PARAMS          ((0x00 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_WAN_SEC_PARAMS         ((0x00 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_APP_SEC_PARAMS          ((0x01 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_APP_SEC_PARAMS         ((0x01 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_PROV_INIT_REQ               ((0x04 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_PROV_INIT_REQ          ((0x04 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_NTFY_PROV_INIT_REQ_SIG      ((0x05 << 2) | RNET_ASL_OPC_NTFY)

#define CMD_ID_RNET_PROV_INIT_SIG               ((0x06 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_PROV_INIT_SIG          ((0x06 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_GET_SEC_CAP                 ((0x10B << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_SEC_CAP                ((0x10B << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_CHECK_CERT_SERIAL           ((0x10C << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_CHECK_CERT_SERIAL      ((0x10C << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_HAND_SHAKE                  ((0x10D << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_HAND_SHAKE             ((0x10D << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_GET_DEVICE_CERT_COUNT_NOUNCE    ((0x115 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_DEVICE_CERT_COUNT_NOUNCE   ((0x115 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_GET_DEVICE_SERIAL_PUK       ((0x116 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_DEVICE_SERIAL_PUK      ((0x116 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_GET_DEVICE_SIG              ((0x117 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_DEVICE_SIG             ((0x117 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_GET_MODEL_SERIAL_PUK        ((0x118 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_MODEL_SERIAL_PUK       ((0x118 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_GET_MODEL_SIG               ((0x119 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_MODEL_SIG              ((0x119 << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_REMOTE_NONCE_ECDH_PUK_CERT_COUNT  ((0x11A << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_REMOTE_NONCE_ECDH_PUK_CERT_COUNT ((0x11A << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_REMOTE_ECDH_SIG         ((0x11B << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_REMOTE_ECDH_SIG        ((0x11B << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_REMOTE_CERT_SERIAL_PUK  ((0x11C << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_REMOTE_CERT_SERIAL_PUK ((0x11C << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_SET_REMOTE_SIG              ((0x11D << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_RESP_REMOTE_SIG             ((0x11D << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_GET_DEVICE_ECDH_PUK         ((0x11E << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_DEVICE_ECDH_PUK        ((0x11E << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_RNET_GET_DEVICE_ECDH_SIG         ((0x11F << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_DEVICE_ECDH_SIG        ((0x11F << 2) | RNET_ASL_OPC_RSP)

#define CMD_ID_GET_CLASS_VERSION                ((0x140 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RESP_CLASS_VERSION               ((0x140 << 2) | RNET_ASL_OPC_RSP)

/**************Deprecated Halo 1.0 command Ids , Please do not reuse***********/
#define CMD_ID_RNET_TRIGGER_JOIN_NETWORK        ((0x120 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_RNET_READ_JOIN_NETWORK_STATUS    ((0x120 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_RNET_RESP_JOIN_NETWORK           ((0x120 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_RNET_NOTIFY_JOIN_NETWORK_STATUS  ((0x120 << 2) | RNET_ASL_OPC_NTFY)
/*******************************************************************************/
#define CMD_ID_RNET_SECURITY_ERROR              ((0x200 << 2) | RNET_ASL_OPC_NTFY)

// SDB command
#define CMD_ID_SDB_CONNECT_WR                   ((0x00 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_SDB_CONNECT_RESP                 ((0x00 << 2) | RNET_ASL_OPC_RSP)
#define CMD_ID_SDB_OPEN_WR                      ((0X01 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_SDB_OPEN_RESP                    ((0X01 << 2) | RNET_ASL_OPC_RESP)
#define CMD_ID_SDB_MAINTAIN_WR                  ((0X02 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_SDB_MAINTAIN_RESP                ((0X02 << 2) | RNET_ASL_OPC_RESP)
#define CMD_ID_SDB_KA_NTFY                      ((0X03 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_SDB_KA_RESP                      ((0X03 << 2) | RNET_ASL_OPC_RESP)
#define CMD_ID_SDB_LINK_STATUS_RD               ((0X04 << 2) | RNET_ASL_OPC_RD)
#define CMD_ID_SDB_LINK_STATUS_NTFY             ((0X04 << 2) | RNET_ASL_OPC_NTFY)
#define CMD_ID_SDB_LINK_STATUS_RESP             ((0X04 << 2) | RNET_ASL_OPC_RESP)
#define CMD_ID_SDB_CLOSE_WR                     ((0X05 << 2) | RNET_ASL_OPC_WR)
#define CMD_ID_SDB_CLOSE_RESP                   ((0X05 << 2) | RNET_ASL_OPC_RESP)

#define RNET_P2P_CHNL_NUM_MIN                   0
#define RNET_P2P_CHNL_NUM_MAX                   (RNET_NUM_OF_P2P_CHNLS-1)
#define RNET_LDR_CHNL_NUM_MAX                   (RNET_NUM_OF_LDR_CHNLS-1)
#define RNET_P2P_KEY_SZ                         16
#define RNET_CLOUD_ID_SZ                        0
#define RNET_GROUP_ID_SZ                        5
#define RNET_SER_NUM_SZ                         17
#define FIRMWARE_OTA_PARAMETER_SZ               30
#define RNET_DISTRESS_GWD_SZ                    4

#define LQ_TEST_DISABLED                        0
#define LQ_TEST_ENABLED                         1
#define LQ_TEST_UNIDIRECTIONAL                  0
#define LQ_TEST_BIDIRECTIONAL                   1
#define LQ_TEST_ROLE_SNIFFER                    0
#define LQ_TEST_ROLE_RECEIVER                   1
#define LQ_TEST_ROLE_TRANSMITTER                2
#define LQ_TEST_DATARATE_HDR                    0
#define LQ_TEST_DATARATE_LDR                    1
#define LQ_TEST_DATARATE_RFU                    2 /*Reserved for the future*/
#define LQ_TEST_DATARATE_AUTOMATIC              3
#define LQ_TEST_DURATION_INFINITE               0XFFFF
#define LQ_TEST_ON_DEFAULT_CHANNEL              (-1)
#define LQ_TEST_BIT_FLAG_PARAM_SZ_BYTE          1
#define LQ_TEST_TX_INTERVAL_PARAM_SZ_BYTE       2
#define LQ_TEST_DURATION_PARAM_SZ_BYTE          2
#define LQ_TEST_ADDRESS_PARAM_SZ_BYTE           RNET_DEV_ID_SZ
#define LQ_TEST_CHANNEL_PARAN_SZ_BYTE           1
#define LQ_TEST_PARAM_MIN_DATA_SZ_BYTE          3
#define LQ_TEST_ADDITIONAL_PARAM_DATA_SZ_BYTE   10
#define LQ_TEST2_ADDITIONAL_PARAM_DATA_SZ_BYTE  14

#define RNET_DEV_PRFL_SZ                        2
#define RNET_MAX_BCN_MISS                       10
#define RNET_MIN_BCN_MISS                       1
#define RNET_MAX_BCN_INTVL                      10
#define RNET_MIN_BCN_INTVL                      1
#define RNET_PRTCL_ENABLE                       1
#define RNET_PRTCL_DISABLE                      0
#define RNET_LI_MODE_ENABLE                     1
#define RNET_LI_MODE_DISABLE                    0
#define RNET_MIN_GRP_PRD                        1
#define RNET_MAX_GRP_PRD                        RNET_MAC_LAST_HDR_SLOT_NUM
#define RNET_MIN_UNI_PRD                        1
#define RNET_MAX_UNI_PRD                        RNET_MAC_LAST_HDR_SLOT_NUM
#define RNET_MIN_GRP_OFFSET                     RNET_MAC_FIRST_HDR_SLOT_NUM
#define RNET_MAX_GRP_OFFSET                     RNET_MAC_LAST_HDR_SLOT_NUM
#define RNET_MIN_UNI_OFFSET                     RNET_MAC_FIRST_HDR_SLOT_NUM
#define RNET_MAX_UNI_OFFSET                     RNET_MAC_LAST_HDR_SLOT_NUM
#define RNET_MAX_GROUP_NUM_COUNT                8

/*Reset Argument[0]: Reset type*/
typedef enum {
    RESET_SOFT = 0,
    RESET_HARD,
    REBOOT
} rnet_reset_type_t;

/*Reset Argument[1]: Reset Target*/
typedef enum {
    RESET_TARGET_ALL = 0,
    RESET_TARGET_MCU,
    RESET_TARGET_HOST
} rnet_reset_target_t;

/*Reset Argument[2]: Reset Delay*/
/*uint8_t value*/

/*Reset command parameters*/
typedef struct{
    rnet_reset_type_t reset_type;
    rnet_reset_target_t reset_target;
    uint8_t reboot_dalay_sec;
} rnet_reset_param_t;

#define RNET_PAIRING_STATE_NOT_PAIRED           0  /*Not paired with any user account or bridge*/
#define RNET_PAIRING_STATE_PAIRED               1  /*Paired with a user account and bridge*/
#define RNET_PAIRING_STATE_PAIR_FAILED          2  /*Failed to paired*/

#define HW_ID_FORMAT_TYPE0                 0

/*
 * Structure to save the supporting parameters
 * to send the proxy RESP_TIME to the end node
 */
typedef struct{
    uint8_t prxy_sec_key[RNET_P2P_KEY_SZ];
} rnet_resp_tm_prxy_support_t;

/*
 * If size changed please update wiki and edit nrf_malloc configurations for
 * products
 * https://wiki.labcollab.net/confluence/x/C0BoQg
 */
_Static_assert(sizeof(rnet_resp_tm_prxy_support_t) == 16, "SIZEOF RNET_RESP_TM_PRXY_SUPPORT_T CHANGED");

//structure for link quality test pram
typedef struct {
   U8                      lq_test_enabled :1;      // test enable flag
   U8                      lq_test_direction :1;    // Unidirectional=0, Bidirectional=1
   U8                      lq_test_role :2;         // receiver=0, transmitter=1, or sniffer=2
   U8                      lq_test_data_rate :2;    // data rate 0 = HDR, 1 = LDR, 2 = reserved, 3 = automatic.
   U8                      lq_test_addr_frmt :2;    // specifies the addres format
   U8                      lq_test_address[RNET_DEV_ID_SZ];      // address: specify the destination address for the transmitter
                                                    // device or source address for the receiver address. Transmitter
                                                    // will send PINGs to corresponding address and receiver should
                                                    // respond back with PONGs only if source address of PING is as
                                                    // specified here. As usual, address of 0x0 implies destination/source is cloud.
   S8                      lq_test_channel;         // For specifying the channel that should be used for the test. In case of data rate
                                                    // 3 (automatic), this should be ignored and default channel(s) in protocol should be
                                                    // used and this parameter should be ignored.
   U16                     lq_tx_interval;          // in ms (max of 65.5 secs sufficient)
   U16                     lq_test_duration_sec;    // in seconds: total duration of the test to run
   U8                      lq_ack_req;              // ack request for the ping packets
   U8                      lq_tx_retries;           // tx retry count
   U16 lq_test_tx_duration_sec; // total duration to send ping
} lq_test_param_t;

//typedef RNET_FDS_RESPONSE_CODE rnet_fds_code_t;

typedef struct {
    uint16_t            total_pkts;
    uint16_t            mean_latency;
    uint8_t             success_rate;
    int8_t              mean_RSS;
    uint8_t             mean_SNR;
    uint8_t             std_RSS;
    uint8_t             std_SNR;
    uint8_t             output_power:5;
} rnet_nwl_lq_test_results_t;

typedef struct {
  U32                   mfg_version;
  U8                    p2p_chnl;
  U8                    pan_id[RNET_PAN_ID_SZ];
  U8                    dev_id[RNET_DEV_ID_SZ];
  U8                    group_num[RNET_MAX_GROUP_NUM_COUNT];
  U8                    auth_code_sz;
  U8                    multicast_retries;
  U8                    pairing_state;          // just flag to read/write, does nothing
  U8                    configuration_complete;
  U8                    mode;
#if !HALO_WITH_MODULE_HALO_LIB_HALO_MANAGEMENT
  U8                    dsn[RNET_SER_NUM_SZ];
  U8                    p2p_broadcast_key[RNET_P2P_KEY_SZ];
  U32                   ka_interval_secs;
  U8                    ka_updated;
#endif
#if !HALO_WITH_MODULE_HALO_LIB_HALO_MANAGEMENT || SID_LQ_TEST
  lq_test_param_t       lq_test_param;
#endif
#if defined(WAN_SUPPORT) && WAN_SUPPORT
  U8                    scan_interval_secs;
#endif
} rnet_nwl_settings_t;

typedef struct {
#if !HALO_WITH_MODULE_HALO_LIB_HALO_MANAGEMENT
  time_def              last_ka_time;
  time_def              fr_kickoff_tm;
#endif
#if !HALO_WITH_MODULE_HALO_LIB_HALO_MANAGEMENT || SID_LQ_TEST
  time_def              last_test_ping_tm;
  time_def              last_test_pong_tm;
  time_def              lq_test_start_tm;
  U8                    last_ping_pong_num;
#endif
  U8                    exec_pending;
  U8                    tr_idx;
  time_def              dfu_kickoff_tm;
#if defined(WAN_SUPPORT) && WAN_SUPPORT
  time_def              last_scan_time;
#endif
  U8                    lq_dst_frmt;
  U16                   lq_num_retries;
  U8                    lq_gw_devid[RNET_DEV_ID_SZ];
} rnet_nwl_data_t;

#if defined(WAN_SUPPORT) && WAN_SUPPORT
#define DEFAULT_SCAN_INTERVAL                   (1 * 60)
#define DEFAULT_KA_INTERVAL                     (1 * 60)
#else
#define DEFAULT_KA_INTERVAL                     (30 * 60)
#endif
// KA to be scheduled KA_INTERVAL +/- (RNET_KA_RAND_TIME_SEC/2)
#define RNET_KA_RAND_TIME_SEC                   (30)
// Minimum interval between two KAs
#define RNET_MIN_KA_DELTA_SEC                   1

rnet_nwl_settings_t* rnet_nwl_settings_get(void);
uint8_t         rnet_nwl_pairing_state_get(void);
void            rnet_nwl_q_ka(void);
void            rnet_nwl_on_transmit(U8 tr_idx);
rnet_error_t    rnet_create_nwl_frame(rnet_pkt_t* pkt, const nwl_opts_t* nwl_opts);
void            rnet_nwl_q_ka(void);

void            log_nwl_frame(nwl_frame_t* p_nwl_frame);
struct sid_timespec rnet_nwl_process();
void            rnet_nwl_init(rnet_nwl_settings_t *init_settings);
rnet_error_t    schedule_fake_event(U32 tm_offset_ms);
const uint8_t   *get_default_pan_id(void);
void            start_link_quality_test(lq_test_param_t* test_param);
void            start_link_quality_test2(lq_test_param_t* test_param, U16 start_rand_spread);
void            stop_link_quality_test();
rnet_error_t    rnet_nwl_cov_gap_notify(const rnet_link_t tx_link, const uint8_t ping_mun);
rnet_error_t    rnet_nwl_lq_test_result_serialize(const lq_test_param_t* order, const  rnet_nwl_lq_test_results_t * results,
                                const uint8_t buff_size, uint8_t *buff, uint8_t *out_size);
rnet_error_t    rnet_nwl_lq_test_result_notify(const lq_test_param_t* order, const  rnet_nwl_lq_test_results_t * results, const rnet_link_t tx_link);
rnet_error_t    rnet_nwl_lq_test_result_get(lq_test_param_t* order, rnet_nwl_lq_test_results_t * results);
#if !defined(GW_SUPPORT) || !GW_SUPPORT
uint8_t rnet_nwl_configuration_complete_get(void);
#endif

/*
 * @brief function get stacit network key
 */
const uint8_t*          rnet_get_static_nw_key(void);

/*
 * @brief Function to get the network key size
 */
uint8_t                 rnet_get_static_nw_key_sz(void);

/*@brief: Function to send the enhnaced noise floor metrics
 * collected over 24 hours to the cloud
 */
#if defined(GW_SUPPORT) && GW_SUPPORT
void rnet_scheduler_ep2p_metrics_ntfy(void) ;
bool rnet_noise_floor_ep2p_metrics_notify(void);
#else
void rnet_ldr_ep2p_metrics_ntfy(void);
#endif

/*
 * @brief Function to send a negative acknowledgment
 *    to the cloud in case the gateway fails to
 *    route the previous command from the cloud to the
 *    end-node.
 * @param[in] tr: pointer to the original packet
 *    that wasn't been able to send to
 * NOTE: this function is useful for the gateway.
 */
void                    rnet_send_nack_to_cloud(rnet_transact_store_t* tr);

#if defined(GW_SUPPORT) && GW_SUPPORT
/*!
 * @brief: enum to distinguish between temporary unicast and app key
 */
typedef enum {
    DEDICATED_LINK_TEMP_UNICAST_KEY = 0,
    DEDICATED_LINK_TEMP_APP_KEY,
    INVALID_KEY_TYPE
} rnet_key_type_t;

/*!
 * @brief: Function to get temporary app/unicast key
 * @param [in] key_type: app key or unicast key
 * @return if dedicated link started, return the key pointer, otherwise NULL
 */
const uint8_t* rnet_get_dedicated_link_temp_key(rnet_key_type_t key);

/*!
 * @brief: Function to set temporary app/unicast key
 * @param[in] @param [in] key_type: app key or unicast key
 * @param[in] key pointer
 */
void rnet_set_dedicated_link_temp_key(rnet_key_type_t key_type, uint8_t* key);
#endif //#if defined(GW_SUPPORT) && GW_SUPPORT

extern U8       rnet_tx_buf[RNET_TX_BUF_SZ];

#if defined(WAN_SUPPORT) && WAN_SUPPORT
/*
 * @brief Function to start gateway discovery for end-node to start home PAN
 */
void rnet_start_pan_scan(void);
#endif

#endif
