/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef __RNET_DBG_LOG__
#define __RNET_DBG_LOG__

#if !PROTOCOL_STANDALONE_BUILD

#include <sid_protocol_opts.h>
#include <sid_protocol_defs.h>
#include "rnet_ll_phy_interface.h"
#include "nordic_common.h"
#include "nrf_log_backend_interface.h"

#define RNET_LOG_LINK_SRL 0
#define RNET_LOG_LINK_BLE 1

#define SNIFFER_PKT_TX    0
#define SNIFFER_PKT_RX    1

//RX ---- |1B RF mode||4B timestamp| |2B RSSI| |1B SNR| |4B carrier offset| |xB raw packet|
typedef struct {
    uint8_t  rf_mode;
    int8_t   snr;
    int16_t  rssi;
    uint32_t time_in_sec;
    int16_t  frac_secs;
    int16_t  time_uncertainty;
    int32_t  carrier_offset; // Deprecated. keep it to be compatible with cloud
    uint8_t  raw_pkt[RNET_MAX_FRAME_SZ];
    //------------------
    uint8_t  total_len; // Total length of all fields.
}__attribute__((packed)) rnet_log_pl_t;

//TX ----- |1B RF mode| |1B output power| | 8B timestamp| |xB raw packet|
typedef struct {
    uint8_t  rf_mode;
    int8_t   out_pwr;
    uint32_t time_in_sec;
    int16_t  frac_secs;
    int16_t  time_uncertainty;
    uint8_t  raw_pkt[RNET_MAX_FRAME_SZ];
    //------------------
    uint8_t  total_len; // Total length of all fields.

}__attribute__((packed)) rnet_log_tx_pl_t;

typedef struct {
    union {
        rnet_log_pl_t rx_packet_pl;
        rnet_log_tx_pl_t tx_packet_pl;
    }pkt;
    uint8_t  dir; //TX=0,RX=1
}rnet_log_pkt_pl_t;

#define LOG_QUEUE_CAPACITY  4
typedef union {
  struct {
   uint8_t  enabled  : 1;
   uint8_t  log_link : 2;
   uint8_t  log_cfg : 2;
   uint8_t  rf_mode : 3;
  };
  uint8_t byte;
} rnet_sniffer_cfg_t;

typedef struct {
    int8_t snr;
    uint8_t buf_len;
    int16_t rssi;
    uint8_t *buf;
} rnet_sniffer_rx_data_t;

typedef struct {
    uint8_t out_pwr;
    uint8_t buf_len;
    uint8_t *buf;
} rnet_sniffer_tx_data_t;

typedef union {
  struct {
   uint8_t  enabled  : 1;
   uint8_t  send_link : 2;
   uint8_t  mode : 2;
   uint8_t  global_level : 3;
  };
  uint8_t byte;
} rnet_logging_cfg_t;

typedef struct {
    nrf_log_backend_t backend;
} nrf_log_backend_nus_t;

void rnet_sniffer_set_config(uint8_t cfg);
void rnet_sniffer_log_pkt(rnet_sniffer_rx_data_t *rx_data);
void rnet_sniffer_log_tx_pkt(rnet_sniffer_tx_data_t *tx_data);
ret_code_t rnet_dbg_log_process(void);
void enable_nus_logging();
void disable_nus_logging();
void set_rnet_ble_log_backend_id(uint32_t id);

#if RNET_LOG_RAW_PKT
void rnet_logging_set_config(uint8_t cfg);
#else
#define rnet_logging_set_config(...)
#endif

#endif

#endif //__RNET_DBG_LOG__
