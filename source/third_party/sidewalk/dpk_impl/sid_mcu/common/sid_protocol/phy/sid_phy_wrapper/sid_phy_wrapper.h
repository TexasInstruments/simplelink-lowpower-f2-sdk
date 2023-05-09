/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_PHY_WRAPPER_H
#define SID_PHY_WRAPPER_H

#include <sid_protocol_phy.h>
#include <sid_pal_radio_ifc.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * These are the set of functions used by the phy in order to perform functions per
 * radio mode. The implementation of these is should be defined on what functionality
 * a device contains (FSK/LORA or both). This is done in order to optimize for space.
 */

void sid_phy_wrapper_get_phy_settings(sid_pal_radio_lora_phy_settings_t **lora_phy_cfg,
    sid_pal_radio_fsk_phy_settings_t **fsk_phy_cfg);

int32_t sid_phy_wrapper_set_phy_settings(sid_pal_radio_modem_mode_t mode);

int32_t sid_phy_wrapper_set_packet_cfg(const sid_pal_radio_packet_cfg_t *rx_packet_cfg, sid_pal_radio_modem_mode_t mode);

int32_t sid_phy_wrapper_start_tx(uint32_t payload_length, sid_pal_radio_modem_mode_t mode);

int32_t sid_phy_wrapper_start_cad(sid_pal_radio_cad_param_exit_mode_t cad_exit_mode,
    uint32_t payload_length, void (*rnet_radio_event_notifier)(sid_pal_radio_events_t events));

int32_t sid_phy_wrapper_phy_start_rx(uint32_t timeout, bool rx_continuous, sid_pal_radio_modem_mode_t mode);

int32_t sid_phy_wrapper_set_tx_continuous_wave(sid_pal_radio_modem_mode_t mode);

int32_t sid_phy_wrapper_get_time_on_air_tx_packet(const struct sid_pal_radio_tx_packet *tx_packet_cfg, sid_pal_radio_modem_mode_t mode);

uint32_t sid_phy_wrapper_time_to_num_symbols_tx_packet(const struct sid_pal_radio_tx_packet *tx_packet_cfg,
    uint32_t backoff_micro_sec, sid_pal_radio_modem_mode_t mode);

int32_t sid_phy_wrapper_get_frequency(sid_pal_radio_modem_mode_t mode);

int32_t sid_phy_wrapper_set_frequency(uint32_t frequency, sid_pal_radio_modem_mode_t mode);

int8_t sid_phy_wrapper_get_power(sid_pal_radio_modem_mode_t mode);

uint32_t sid_phy_wrapper_set_power(uint8_t power, sid_pal_radio_modem_mode_t mode);

sid_pal_radio_data_rate_t sid_phy_wrapper_get_data_rate(sid_pal_radio_modem_mode_t mode);

int32_t sid_phy_wrapper_set_tx_packet_cfg(const sid_pal_radio_tx_packet_t *tx_packet_cfg,
    sid_pal_radio_modem_mode_t mode);

int32_t sid_phy_wrapper_carrier_sense(uint32_t timeout, sid_pal_radio_cad_param_exit_mode_t exit_mode);

uint32_t sid_phy_wrapper_get_cad_duration(const sid_pal_radio_packet_cfg_t *radio_cfg,
        sid_pal_radio_cad_param_exit_mode_t exit_mode, sid_pal_radio_modem_mode_t mode);

uint32_t sid_phy_wrapper_get_rx_process_delay(sid_pal_radio_modem_mode_t mode);

uint32_t sid_phy_wrapper_get_tx_process_delay(sid_pal_radio_modem_mode_t mode);

uint32_t sid_phy_wrapper_get_time_on_air(uint8_t payload_length, sid_pal_radio_modem_mode_t mode);

int32_t sid_phy_wrapper_toggle_features(enum mode_specific_features feature,
    bool enable, sid_pal_radio_modem_mode_t mode);

int32_t sid_phy_wrapper_get_rx_done_delay(const struct sid_pal_radio_tx_packet *tx_packet_cfg,
    sid_pal_radio_modem_mode_t mode);

uint32_t sid_phy_wrapper_get_symbol_timeout_us(sid_pal_radio_data_rate_t data_rate, uint8_t num_of_symbol, sid_pal_radio_modem_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif
