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

#ifndef DIAG_TEST_MODE_H
#define DIAG_TEST_MODE_H

#include <sid_pal_radio_ifc.h>
#include <sid_protocol_phy.h>

#ifdef __cplusplus
extern "C" {
#endif

enum diag_test_ping_pong_state {
    PP_STATE_UNKNOWN = 0,
    PP_STATE_PING_SEND = 1,
    PP_STATE_PONG_SEND = 2,
    PP_STATE_PING_SENDING = 3,
    PP_STATE_PONG_SENDING = 4,
    PP_STATE_WAIT_FOR_PING_RECEIVE = 5,
    PP_STATE_WAIT_FOR_PONG_RECEIVE = 6,
    PP_STATE_PING_RECEIVED = 7,
    PP_STATE_PONG_RECEIVED = 8,
    PP_STATE_STOP = 9,
};

struct diag_test_ping_pong_info {
    enum diag_test_ping_pong_state pp_state;
    uint8_t pp_mode;
    uint32_t ping_counter;
    uint32_t pong_counter;
    uint32_t ping_send_timeout_counter;
    uint32_t pong_send_timeout_counter;
    uint32_t pong_receive_timeout_counter;
    uint32_t ping_receive_timeout_counter;
    uint32_t pp_max_iterations;
    uint32_t period_secs;
    bool timer_fired;
};

struct diag_test_mode_state {
    bool cw_ena;
    bool mod_ena;
    bool rxContinuous;
    struct diag_test_ping_pong_info pp_test;
    sid_pal_radio_modem_mode_t modem;
    bool tx_timer_fired;
    bool rtc;
    uint32_t tx_packet_spacing;
    int16_t tx_threshold;
    uint32_t tx_sense_time;
    int32_t tx_pkt_counts;
    uint16_t freq_hop_bw;
    uint8_t freq_hop_ena;
};

#define PP_TEST_MODE_STOP                0
#define PP_TEST_MODE_PING                1
#define PP_TEST_MODE_PONG                2
#define PP_TEST_MODE_CHECK               3

#define DIAG_MOD_PROFILE_0               0
#define DIAG_MOD_PROFILE_1               1
#define DIAG_MOD_PROFILE_2               2

#define NOT_SUPPORT                      "Feature Not Support !"

/**
 *  @brief Get test mode state information sturcture
 *
 *  @return diag_test_mode_state which point to test mode state information structure
 */
struct diag_test_mode_state *diag_tm_st_get(void);

/**
 *  @brief Print modem mode in radio hal
 */
void diag_test_get_modem_mode(void);

/**
 *  @brief Print frequency setting by modem mode
 *
 *  @param modem_mode modem mode setting
 */
void diag_test_get_modem_freq(sid_pal_radio_modem_mode_t modem_mode);

/**
 *  @brief Print transmission power by modem mode
 *
 *  @param modem_mode modem mode setting
 */
void diag_test_get_modem_tx_power(sid_pal_radio_modem_mode_t modem_mode);

/**
 *  @brief Print FSK modulation parameters by modem mode (depreciated)
 *
 *  @param modem_mode modem mode setting
 */
void diag_test_get_modem_mod_params(sid_pal_radio_modem_mode_t modem_mode);

/**
 *  @brief Print packet parameters by modem mode
 *
 *  @param modem_mode modem mode setting
 */
void diag_test_get_modem_packet_params(sid_pal_radio_modem_mode_t modem_mode);

/**
 *  @brief Print LORA cad parameters by modem mode
 *
 *  @param modem_mode modem mode setting
 */
void diag_test_get_lora_cad_params(sid_pal_radio_modem_mode_t modem_mode);

/**
 *  @brief Print tx timeout by modem mode
 *
 *  @param modem_mode modem mode setting
 */
void diag_test_get_tx_timeout(sid_pal_radio_modem_mode_t modem_mode);

/**
 *  @brief Print time on the air by modem mode and specific packet length
 *
 *  @param modem_mode modem mode setting
 *  @param len packet length
 */
void diag_test_get_modem_time_on_air(sid_pal_radio_modem_mode_t modem_mode, uint8_t len);

/**
 *  @brief Print FSK modulation parameters by modem mode
 *
 *  @param modem_mode modem mode setting
 */
void diag_test_get_modem_profile(sid_pal_radio_modem_mode_t modem_mode);

/**
 *  @brief Print noise floor for a range of channels; channel index 0 is 910MHz, and
 *         increase 250KHz for next channel index, that is 910.250MHz for channel index 1
 *
 *  @param ch_start get noise floor from channel index ch_start
 *  @param ch_end get noise floor to channel index ch_end
 */
void diag_test_get_channel_noise(uint8_t ch_start, uint8_t ch_end);

/**
 *  @brief Config the modem mode to Radio hal
 *
 *  @param modem_mode modem mode for Radio hal, 3P vendors support FSK only
 */
void diag_test_set_modem(sid_pal_radio_modem_mode_t modem_mode);

/**
 *  @brief Config non-modulation continuous wave to be on or off
 *
 *  @param on_off turn on or off the continuous wave
 */
void diag_test_set_cw(uint8_t on_off);

/**
 *  @brief Config the transmission power of FSK mode
 *
 *  @param pwr_dbm to config the transmission power to pwr_dbm in dBm unit
 *  @param modem_mode modem mode setting
 */
void diag_test_set_power(int8_t pwr_dbm, uint8_t modem_mode);

/**
 *  @brief Config the frequency of FSK mode
 *
 *  @param freq to config the frequency to freq in hz unit
 */
void diag_test_set_frequency(uint32_t freq);

/**
 *  @brief Config the radio driver to sleep, standby, or receiver mode
 *
 *  @param state to define the mode sleep = 1, standby = 2, and receiver = 3
 */
void diag_test_set_radio_state(uint8_t state);

/**
 *  @brief Config the radio driver to send FSK modulation packets
 *
 *  @param ena to enable or disable FSK modulation packet transmission
 *  @param packet_spacing back off time if noise floor higher than threshold
 *  @param threshold packet will really be sent if the noise floor lower than this value
 *  @param sense_time how long need to check if the noise floor lower than threshold; unit is us
 *  @param pkt_counts how many packets will be transmitted
 */
void diag_test_set_modulate(bool ena, uint32_t packet_spacing, int16_t threshold,
                            uint32_t sense_time, int32_t pkt_counts);

/**
 *  @brief Config the diags-app to run ping-pong test
 *
 *  @param mode to instruct diags-app which action should be run, including ping
 *              or pong or stop action or show counters
 *  @param count how many times of ping-pong tests
 *  @param delay the first ping will get started after this delay seconds;
 *               the first pong will wait until this delay seconds pass
 *  @param freq time interval between each time of ping-pong test
 */
void diag_test_set_ping_pong(uint8_t mode, uint32_t count, uint32_t delay, uint32_t freq);

/**
 *  @brief Config diags-app to run frequence hop test.
 *
 *  @param en to enable or disable frequence hop function
 *  @param start config the start frequency to Low freq in Hz unit
 *  @param end config the end frequency to High freq in Hz unit
 *  @param separation channel separation in Hz
 *  @param dwell_time dwell time in ms
 */
void diag_test_set_fqhop(uint8_t en, uint32_t start, uint32_t end, uint32_t separation,
                         uint16_t dwell_time);

/**
 *  @brief Config the tx timeout in radio driver
 *
 *  @param modem_mode to specify the modem_mode
 *  @param seconds tx timeout in seconds
 */
void diag_test_set_tx_timeout(sid_pal_radio_modem_mode_t modem_mode, uint8_t seconds);

/**
 *  @brief Config the FSK modulation by profile number
 *
 *  @param profile to specify the profile number; 0,1,2 for 50,150,250kbps, respectively
 */
void diag_test_set_mod_profile(uint8_t profile);

/**
 *  @brief To init the test mode default information structure
 */
void diag_test_mode_init(void);

#ifdef __cplusplus
}
#endif

#endif
