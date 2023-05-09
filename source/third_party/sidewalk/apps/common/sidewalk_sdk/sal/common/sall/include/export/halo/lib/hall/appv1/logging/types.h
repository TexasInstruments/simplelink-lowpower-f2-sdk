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

#ifndef HALO_ALL_APPV1_LOGGING_TYPES_H_
#define HALO_ALL_APPV1_LOGGING_TYPES_H_

#include <halo/lib/hall/platform.h>
#include <sid_time_types.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HALO_ALL_LOGGING_MAX_PKT_SIZE 256
#define HALO_ALL_LOGGING_MAX_NE 5
#define HALO_ALL_LOGGING_LOG_DAT_SIZE 5
typedef enum {
    HALO_ALL_LOGGING_SNIFFER_CFG_LOG_RECEIVED = 0x1,
    HALO_ALL_LOGGING_SNIFFER_CFG_LOG_TRANSMITTED = 0x2,
    HALO_ALL_LOGGING_SNIFFER_CFG_LOG_ALL = 0x3,
} halo_all_logging_sniffer_cfg;

typedef enum {
    HALO_ALL_LOGGING_LOGGER_CFG_COMPRESSED = 0x1,
    HALO_ALL_LOGGING_LOGGER_CFG_HUMAN_READABLE = 0x2,
} halo_all_logging_logger_cfg;

typedef enum {
    HALO_ALL_LOGGING_LINK_SRL = 0x0,
    HALO_ALL_LOGGING_LINK_BLE = 0x1,
    HALO_ALL_LOGGING_LINK_RTT = 0x2,
} halo_all_logging_link;

typedef enum {
    halo_all_logging_level_OFF = 0x0,
    halo_all_logging_level_ERROR = 0x1,
    halo_all_logging_level_WARNING = 0x2,
    halo_all_logging_level_INFO = 0x3,
    halo_all_logging_level_DEBUG = 0x4,
} halo_all_logging_level;

/*|3b RF mode| |2b logging cfg| |2b logging link| |1b enable/diasble|*/
typedef struct {
    uint8_t rf_mode;
    halo_all_logging_sniffer_cfg logging_cfg;
    halo_all_logging_link logging_link;
    bool enable;
} halo_all_logging_sniffer_cfg_t;

/*|3b logging level| |2b logging cfg| |2b logging link| |1b enable/disable|*/
typedef struct {
    halo_all_logging_level logging_level;
    halo_all_logging_logger_cfg logging_cfg;
    halo_all_logging_link logging_link;
    bool enable;
} halo_all_logging_log_cfg_t;

/*|1B RF mode || 1B SNR || 2B RSSI || 8B timestamp | | xB raw packet ||*/
typedef struct {
    uint8_t raw_pkt[HALO_ALL_LOGGING_MAX_PKT_SIZE];
    struct sid_timespec timestamp;
    uint16_t rssi;
    size_t raw_pkt_sz;
    uint8_t rf_mode;
    uint8_t snr;
} halo_all_logging_pkt_rx_info_t;

/*|1B RF mode||1B output power|| 8B timestamp| |xB raw packet|*/
typedef struct {
    uint8_t raw_pkt[HALO_ALL_LOGGING_MAX_PKT_SIZE];
    struct sid_timespec timestamp;
    size_t raw_pkt_sz;
    uint8_t rf_mode;
    uint8_t output_power;
} halo_all_logging_pkt_tx_info_t;

/* |1B module ID| |2b reserved||4b logging level||2b logging cfg| */
typedef struct {
    halo_all_logging_level logging_level;
    halo_all_logging_logger_cfg logging_cfg;
    uint8_t module_id;
} halo_all_logging_module_cfg_t;

/* |1B num of cfgs (N)| Array of configs|*/
typedef struct {
    halo_all_logging_module_cfg_t cfgs[HALO_ALL_LOGGING_MAX_NE];
    uint8_t num_of_cfgs;
} halo_all_logging_module_cfgs_t;

/*|2B starting log entry number (SLN)| |2B num of entries to clear/delete (NE)|*/
typedef struct {
    uint16_t start_entrie_number;
    uint16_t num_of_entries;
} halo_all_logging_entries_clear_t;

/*|1B module ID| |4b log level| |12b Log ID| |5B log data| |8B Timestamp|*/
typedef struct {
    uint8_t log_data[HALO_ALL_LOGGING_LOG_DAT_SIZE];
    struct sid_timespec timestamp;
    halo_all_logging_level log_level;
    uint16_t log_id;
    uint8_t module_id;
} halo_all_logging_log_entrie_t;

/* |1B num of entries (NE)| |NEx16B log entry format data|*/
typedef struct {
    uint8_t num_of_entries;
    halo_all_logging_log_entrie_t entries[HALO_ALL_LOGGING_MAX_NE];
} halo_all_logging_log_entries_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HALO_ALL_APPV1_LOGGING_TYPES_H_ */
