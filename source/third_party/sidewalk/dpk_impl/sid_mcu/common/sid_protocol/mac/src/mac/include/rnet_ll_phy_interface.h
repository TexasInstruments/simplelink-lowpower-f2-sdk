/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef LIB_RINGNET_WAN_SRC_PHY_RNET_LL_PHY_INTERFACE_H_
#define LIB_RINGNET_WAN_SRC_PHY_RNET_LL_PHY_INTERFACE_H_

#include <sid_protocol_phy.h>
#include <rnet_errors.h>
#include "rnet_time.h"

////////////// ENUM DECLERATIONS //////////////
/** Rnet Phy Radio Modem Mode*/

#define RNET_MAX_PHY_SZ         256

typedef struct rnet_rx_packet_phy_metadata{
    struct sid_timespec* rcv_tm;
    sid_pal_radio_data_rate_t data_rate;
    int16_t rssi;
    int16_t signal_rssi;
    int8_t snr;
    sid_pal_radio_modem_mode_t mode;
} rnet_rx_packet_phy_metadata_t;

typedef struct rnet_ll_rx_packet {
    uint8_t* rcv_payload;
    uint8_t payload_len;
    rnet_rx_packet_phy_metadata_t phy_metadata;
} rnet_ll_rx_packet_t;

//DS needed by sniffer and legacy applications
//will be removed once apps move to HALL

typedef struct {
    U8 vld:1;
    time_def rcv_tm;
    S16 rssi;
    S8 snr;
    U16 len;
    U8 buf[RNET_MAX_PHY_SZ];
} phy_rx_data_t;


typedef struct {
    U8 vld:1;
    U8 buf[RNET_MAX_PHY_SZ];
    time_def rcv_tm;
    U8 out_pwr;
    U16 len;
} phy_tx_data_t;

typedef enum
{
    STANDBY = 0,
    SLEEP,
    CAD_DETECT,
    RECEIVE,
    TRANSMIT,
    TRANSMIT_CW,
} RnetRadioState_t;

////////////// METHOD DECLERATIONS //////////////
/** @brief Retrieve the received packet.
 *
 *  On receiving rx done event this api would needs to be called to get the received packet.
 *  @param void.
 *  @return pointer to rnet_ll_rx_packet_t.
 */
rnet_ll_rx_packet_t *rnet_get_rx_ll_packet(void);

/**
 * @brief Returns the minimum number of preamble symbols needed for a completely
 * time synchronous tx/tx  operation to succeeed considering crystal tolerances
 * and the modulation
 *
 * In the case of fractional symbols, rounds it up.
 * Does not affect the radio state and can be executed without radio ownership.
 * In the case of an error, 0 is returned.
 *
 * @param radio_cfg Pointer to the radio_config
 * @param tx_packet_cfg modem mode
 * @return Number of symbols needed. In the case of error 0 is returned.
 */
uint32_t rnet_phy_min_num_symbols_radio_cfg(const sid_pal_radio_packet_cfg_t *radio_cfg, sid_pal_radio_modem_mode_t m );

/**
 * @brief set phy modem to fsk or lora mode
 * it will set the modulation and packet parameter to the specified phy mode.
 * @param  sid_pal_radio_modem_mode_t mode RNET_RADIO_MODEM_MODE_FSK/RNET_RADIO_MODEM_MODE_LORA
 * @return success/failure
 */
rnet_error_t rnet_phy_set_modem(sid_pal_radio_modem_mode_t mode);

/*
 * @brief map Halo1 channels indices to Halo2 channel indices.
 * Halo1 channels use channel index 0-7 to map to RF frequencies shown below.
 *
 * halo1 channel | halo2 channel | RF Frequency
 *      0        |      0        |   902.5MHz
 *      1        |      1        |   903.3MHz
 *      2        |      25       |   922.5MHz
 *      3        |      26       |   923.3MHz
 *      4        |      27       |   924.1MHz
 *      5        |      28       |   924.9MHz
 *      6        |      29       |   925.7MHz
 *      7        |      30       |   926.5MHz
 *
 * @params: halo1_ch: Halo1 channel index from 0-7
 * @return: Halo2 channel mapped to corresponding indices shown in table above
 */
uint8_t rnet_halo1_to_halo2_channel_map(uint8_t halo1_ch);

//APIs to be rmeoved once applications move to HALL
//TODO:HALO-2916

int16_t rnet_phy_get_chan_noise(sid_pal_radio_modem_mode_t modem, uint32_t freq);

rnet_error_t rnet_phy_get_freq_by_halo1_channel(uint8_t halo1_ch, uint32_t *freq);

int32_t rnet_phy_init(void);

RnetRadioState_t RnetPhyRadioStateGet(void);

#endif /* LIB_RINGNET_WAN_SRC_PHY_RNET_LL_PHY_INTERFACE_H_ */
