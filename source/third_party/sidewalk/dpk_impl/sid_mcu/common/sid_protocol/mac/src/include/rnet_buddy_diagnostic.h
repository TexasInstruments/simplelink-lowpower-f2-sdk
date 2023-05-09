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

#ifndef RNET_BUDDY_DIAGNOSTIC_H_
#define RNET_BUDDY_DIAGNOSTIC_H_

#include <rnet_errors.h>
#include "rnet_nw_layer.h"
#include "rnet_api.h"
#include "rnet_time.h"
#include "rnet_mac_util.h"

typedef enum {
    PING_TX_PENDING = 0,
    PING_TX_DONE
} ping_tx_state_t;

typedef struct rnet_buddy_txentry_t {
    RNET_LIST_ENTRY(rnet_buddy_txentry_t);
    U8  ping_seq_no;
    ping_tx_state_t state;
    time_def tx_state_time;
} rnet_buddy_txentry_t;

/*!
 * @brief Process coverage tool packet timer.
 *
 * @return RNET_SUCCESS (always)
 */
rnet_error_t rnet_buddy_pkttimer_process(void);

/*!
 * @brief initialize the buddy_diagnostic_data starts PING interval timer
 * @param[in] ping_interval_time: time in ms ping period.
 * return true if the send operation successfully initiated, false otherwise
 */
bool rnet_buddy_diagnostic_start(uint16_t ping_interval_time);

/*!
 * @brief pong receive callback
 *        update rssi,snr,latency metrics
 * @param[in] rssi: rssi of received frame
 * @param[in] snr: snr of received frame
 * @param[in] recv_time: receive timestamp
 * @param[in] pong_seq_no: seqquence number of pong
 */
void rnet_buddy_pong_rx(int8_t rssi, int8_t snr, time_def *recv_time, U8 pong_seq_no);

/*!
 * @brief ping tx callback for tx done
 *        frame manager will give this callback after the ping packet is transmitted
 * @param[in]  rnet_tr: transaction store entry for the ping packet
 * @param[in]  status:  tx done status for the packet
 */
void rnet_buddy_ping_txdone(void *tr, int32_t cmd_status);

/*!
 * @brief ping tx call. this is called when nw layer sends the ping packet
 * @param[in] ping_seq_no: sequence number of the ping
 * return true if the tx context was allocated, false if failure.
 */
bool rnet_buddy_ping_tx(U8 ping_seq_no);

/*!
 * @brief return buddy diagnostic test state
 * True: if test is in progress
 * False: if test is not in progress
 */
bool rnet_is_buddy_diagtest_enabled(void);

/*!
 * @brief ends buddy diagnostic test
 */
void rnet_buddy_test_end(void);

/*!
 * @brief Implementation of buddy diagnostic test cleanup. link quality test notification is sent to cloud.
 */
void rnet_buddy_lq_test_cleanup(void);

/*!
 * @brief ends of buddy diagnostic test. calls the rnet_buddy_test_end
 * if the pkt timer is not active
 */
void rnet_buddy_test_stop(void);

/*!
 * @brief return the last pong rssi
 */
int8_t rnet_buddy_get_pongrssi(void);

/*!
 * @brief return the last pong snr
 */
int8_t rnet_buddy_get_pongsnr(void);

/*!
 * @brief get buddy lq test pending cleanup flag.
 */
bool rnet_get_buddy_lq_test_pending_cleanup(void);

#endif //RNET_BUDDY_DIAGNOSTIC_H_
