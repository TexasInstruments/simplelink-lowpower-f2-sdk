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

#ifndef SID_PROTOCOL_PHY_INTERNAL_H
#define SID_PROTOCOL_PHY_INTERNAL_H

#include <sid_protocol_metrics.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief    Update time spent in each radio state.
 *            Update the radio state to metrics subsystem to capture time spent by
 *            radio in CAD, Standby, Sleep, TX and RX states
 *
 *  @param    state metrics state
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_phy_update_radio_time_metrics(metrics_time_types_t state);

/** @brief    Get tx power that needs to be configured for the current packet
 *
 *  @param    pointer to the tx packet cfg
 *  @param    modem mode
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_phy_set_pkt_tx_power(const sid_pal_radio_tx_packet_t *tx_packet_cfg, sid_pal_radio_modem_mode_t mode);

/** @brief    Set frequency by channel
 *
 *  @param    channel channel
 *  @param    modem mode
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_phy_set_frequency_by_channel(const uint8_t channel, sid_pal_radio_modem_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif

