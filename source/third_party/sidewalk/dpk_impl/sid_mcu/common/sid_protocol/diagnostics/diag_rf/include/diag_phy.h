/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.  This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef DIAG_PHY_H
#define DIAG_PHY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <diag_radio.h>

#define DIAG_MAX_PHY_SZ                 256
#define DIAG_MAX_FRAME_SZ               249
#define DIAG_TX_GUARD_DELAY_MS          3
#define DIAG_TX_MOD_MIN_PAYLOAD_LENGTH  8

// If user put radio to sleep mode, wake it up when next SWI context has been triggered
enum diag_radio_state {
    STANDBY = 0,
    SLEEP,
    RECEIVE,
    TRANSMIT,
    TRANSMIT_CW,
};

enum diag_radio_result {
    NONE,
    RX_DONE,
    RX_TIMEOUT,
    RX_ERR,
    TX_DONE,
    TX_TIMEOUT
};

// Helps track state of what we were currently transmitting
enum phy_pkt_type {
    TM_MOD_PKT = 1,
    RAW_PKT,
};

struct phy_pkt_data {
    uint8_t vld:1;
    struct sid_timespec rcv_tm;
    int16_t rssi;
    int8_t snr;
    uint8_t rx_status;
    int8_t rssi_sync;
    uint16_t len;
    uint8_t buf[DIAG_MAX_PHY_SZ];
};

struct diag_phy_state {
    enum diag_radio_state radio_st;
    enum diag_radio_result radio_result;
    struct phy_pkt_data pkt_data;
    enum phy_pkt_type tx_pkt_type; // What we're currently transmitting
};

struct p2p_metrics {
    uint32_t total_tx_pkts;
    uint32_t total_rx_pkts;
    uint32_t CRC_errors;
};

/**
 *  @brief Keep the payload length for each transmission packet
 *
 *  @param length packet length in bytes
 */
void diag_phy_set_payload_len(uint8_t length);

/**
 *  @brief Get the payload length
 */
uint8_t diag_phy_get_payload_len(void);

/**
 *  @brief Config the fcs type and whitening setting for each packet
 *
 *  @param fcs_type frame checksum type. 2 bytes or 4 bytes
 *  @param whiten enable or disable whitening
 */
void diag_phy_set_fsk_hdr(uint8_t fcs_type, uint8_t whiten);

/**
 *  @brief Get the noise floor for specific frequency
 *
 *  @param freq the frequency point that noise floor check perform
 */
int16_t diag_phy_get_radio_channel_noise(uint32_t freq);

/**
 *  @brief Get the return result of radio
 *
 *  @param diag_radio_result current return result of radio
 */
enum diag_radio_result diag_phy_get_radio_result(void);

/**
 *  @brief Get the configured radio state
 *
 *  @param diag_radio_state current configured state of radio
 */
enum diag_radio_state diag_phy_get_radio_state(void);

/**
 *  @brief Init radio and register notification callback function
 */
void diag_phy_init(void);

/**
 *  @brief process registered timer events in SWI
 */
void diag_phy_process_radio_events(void);

/**
 *  @brief config radio to sleep mode
 */
void diag_phy_sleep(void);

/**
 *  @brief config radio to standby mode
 */
void diag_phy_standby(void);

/**
 *  @brief config radio to receive packets
 *
 *  @param continuous enable to receive continuously and disable to receive one packet
 */
void diag_phy_start_receive(bool continuous);

/**
 *  @brief config radio to continuous wave mode
 */
void diag_phy_set_cw(void);

/**
 *  @brief send modulation packet
 */
void diag_phy_mod_pkt(void);

/**
 *  @brief config tx power for radio
 *
 *  @param pwr tx power in dBm unit
 */
int8_t diag_phy_set_tx_power(int8_t pwr);

/**
 *  @brief config max tx power for radio
 */
void diag_phy_set_max_power(void);

/**
 *  @brief reset tx and rx packet counters
 */
void diag_phy_reset_counters(void);

/**
 *  @brief read last tx or rx packet log
 */
int8_t diag_phy_read_counters(int8_t *buf);

/**
 *  @brief show last tx or rx packet log
 */
void diag_phy_print_last_packet(void);

/**
 *  @brief show last rx packet rssi
 */
void diag_print_last_rssi_records(int8_t records);

/**
 *  @brief show rssi avg for rx packets
 */
void diag_print_avg_rssi_records(void);

/**
 *  @brief show tx and rx packet counters
 */
void diag_phy_print_counters(void);

/**
 *  @brief send ping packet with specific contnet
 */
void diag_phy_send_ping(void);

/**
 *  @brief send pong packet with specific content
 */
void diag_phy_send_pong(void);

#ifdef __cplusplus
}
#endif

#endif
