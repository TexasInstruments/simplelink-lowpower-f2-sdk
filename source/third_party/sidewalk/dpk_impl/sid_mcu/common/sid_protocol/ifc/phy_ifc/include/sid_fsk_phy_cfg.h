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

/**
 * @file sid_fsk_phy_cfg.h
 * @brief SID FSK PHY configuration
 * @addtogroup SID_PHY
 *
 * USAGE
 * -----
 *
 * 1) Global handle for fsk phy configuration.
 * 2) APIs to get and set fsk modulation, packet params.
 * 3) APIs to get and set frequency, power for fsk mode
 *
 * @{
 */

#ifndef SID_FSK_PHY_CFG_H
#define SID_FSK_PHY_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <sid_pal_radio_ifc.h>
#include <sid_pal_radio_fsk_defs.h>

#define RNET_FSK_DEFAULT_FREQUENCY  908000000

#define RNET_FSK_DEFAULT_TX_POWER               20
#define RNET_FSK_DEFAULT_HIGH_POWER             11

#define RNET_FSK_DEFAULT_BIT_RATE               50000
#define RNET_FSK_DEFAULT_MOD_SHAPING            SID_PAL_RADIO_FSK_MOD_SHAPING_G_BT_05
#define RNET_FSK_DEFAULT_BANDWIDTH              SID_PAL_RADIO_FSK_BW_150KHZ
#define RNET_FSK_DEFAULT_FREQ_DIVISION          25000
#define RNET_FSK_DEFAULT_PREAMBLE_LENGTH        50
#define RNET_FSK_DEFAULT_PREAMBLE_MIN_DETECT    SID_PAL_RADIO_FSK_PREAMBLE_DETECTOR_08_BITS
#define RNET_FSK_DEFAULT_SYNC_WORD_LENGTH       3
#define RNET_FSK_DEFAULT_ADD_COMP               SID_PAL_RADIO_FSK_ADDRESSCOMP_FILT_OFF
#define RNET_FSK_DEFAULT_HEADER_TYPE            SID_PAL_RADIO_FSK_RADIO_PACKET_FIXED_LENGTH
#define RNET_FSK_DEFAULT_CRC_TYPE               SID_PAL_RADIO_FSK_CRC_OFF
#define RNET_FSK_DEFAULT_RADIO_WHITENING_MODE   SID_PAL_RADIO_FSK_DC_FREE_OFF
#define RNET_FSK_DEFAULT_WHITENING_SEED         SID_PAL_RADIO_FSK_WHITENING_SEED
#define RNET_FSK_DEFAULT_PAYLOAD_LENGTH         250
#define RNET_FSK_MAX_PDU_LENGTH                 255 /* MAX pdu length allowed by semtech radio in fsk */
#define RNET_FSK_MAX_SDU_LENGTH                 249 /* fsk phy adds 2 byte header & 4/2 byte crc */

#define RNET_FSK_DEFAULT_CAD_RSSI_THRESHOLD     -80
#define RNET_FSK_DEFAULT_CAD_DURATION_MUS       1200
#define RNET_FSK_DEFAULT_CS_DURATION_MUS        1200

#define MSEC_TO_USEC(X)                         ((X) * 1000)

#define USECS_IN_SEC                            (1000000UL)
#define SEC_TO_USEC(X)                          ((X) * USECS_IN_SEC)
//This is accounting for the radio driver to finish its operation and generate the response/start next operation.
//The value is set experimentally by comparing the theoretical numbers with the CAD duration by recording SPI
//transactions.
#define FSK_CAD_TRANS_TIME_US 1200


/** @brief  set frequency for fsk mode.
 *          The API checks the regulatory settings to validate the frequency.
 *
 *  @param  frequency in hz
 *  @return radio error code
 */
int32_t rnet_phy_set_fsk_frequency(uint32_t freq);

/** @brief  set transmit power for fsk mode.
 *          The API checks the regulatory settings to set valid transmit power setting.
 *
 *  @param  power in dB
 *  @return radio error code
 */
int32_t rnet_phy_set_fsk_power(int8_t power);

/** @brief  get fsk cad parameters.
 *
 *  @return radio_cad_params_t.
 */
const sid_pal_radio_fsk_cad_params_t * rnet_phy_get_fsk_cad_params(void);

/** @brief  set cad packet parameters.
 *
 *  @param  radio_fsk_cad_params_t.
 *  @return radio error code
 */

int32_t rnet_phy_set_fsk_cad_params(const sid_pal_radio_fsk_cad_params_t *cad_params);

/** @brief  set sync word for fsk mode.
 *
 *  @param  sync_word.
 *  @return radio error code
 */
int32_t rnet_phy_set_fsk_sync_word(const uint8_t *sync_word);

/** @brief  set whitening seed for fsk mode.
 *
 *  @param  whitening seed.
 *  @return radio error code
 */
int32_t rnet_phy_set_fsk_whitening_seed(uint16_t seed);

/** @brief  set fsk modulation parameters.
 *
 *  @param  sid_pal_radio_fsk_modulation_params_t.
 *  @return radio error code
 */
int32_t rnet_phy_set_fsk_modulation_params(const sid_pal_radio_fsk_modulation_params_t *mod_params);

/** @brief  set fsk packet parameters.
 *
 *  @param  sid_pal_radio_fsk_packet_params_t.
 *  @return radio error code
 */
int32_t rnet_phy_set_fsk_packet_params(const sid_pal_radio_fsk_packet_params_t *packet_params);

/** @brief  set fsk configuration params to radio.
 *          This API should be invoked when switching from lora mode to fsk mode.
 *
 *  @return radio error code
 */
int32_t rnet_phy_set_fsk_phy_cfg(void);

/** @brief  set fsk tx timeout.
 *
 *  @param  tx_timeout in milliseconds
 */
void rnet_phy_set_fsk_tx_timeout(uint32_t tx_timeout);

/** @brief  get fsk configuration params
 *
 *  @return sid_pal_radio_fsk_phy_settings_t.
 */
sid_pal_radio_fsk_phy_settings_t *rnet_get_fsk_phy_settings(void);

/** @brief  get frequency configured in fsk mode.
 *
 *  @return frequency in hz
 */
uint32_t rnet_phy_get_fsk_frequency(void);

/** @brief  get tx power configured for fsk mode.
 *
 *  @return power in dB.
 */
int8_t rnet_phy_get_fsk_power(void);

/** @brief  get sync word configured for fsk mode.
 *
 *  @return sync word.
 */
const uint8_t *rnet_phy_get_fsk_sync_word(void);

/** @brief  get whitening seed configured for fsk mode.
 *
 *  @return whitening seed.
 */
uint16_t rnet_phy_get_fsk_whitening_seed(void);

/** @brief  get fsk modulation parameters.
 *
 *  @return sid_pal_radio_fsk_modulation_params_t.
 */
const sid_pal_radio_fsk_modulation_params_t *rnet_phy_get_fsk_modulation_params(void);

/** @brief  get fsk packet parameters.
 *
 *  @return sid_pal_radio_fsk_packet_params_t.
 */
const sid_pal_radio_fsk_packet_params_t *rnet_phy_get_fsk_packet_params(void);

/**
 * @brief   get time on air for a given tx packet config
 *          The API calculates the time on air for a given packet config and payload length.
 *          It neither changes packet config nor uses reads the packet payload.
 *
 *  @param  tx_packet_cfg pointer to sid_pal_radio_tx_packet_t.
 *  @return  On success a positive number indicating the time on air in ms is returned,
 *           on error 0 is returned
 */
uint32_t rnet_phy_get_fsk_time_on_air_tx_packet_cfg(const struct sid_pal_radio_tx_packet *tx_packet_cfg);


/** @brief  rnet phy get tx timeout configured in fsk mode.
 *
 *  @return tx timeout configured.
 */
uint32_t rnet_phy_get_fsk_tx_timeout(void);

/**
 * @brief   Returns the number of preamble symbols needed for a given tx_config and backoff_duration.
 *          In the case of fractional symbols, rounds it up.
 *          Does not affect the radio state and can be executed without radio ownership.
 *
 * @param   tx_packet_cfg Pointer to the tx_config
 * @param   Backoff time in ms.
 * @return  Number of symbols needed. In the case of error 0 is returned.
 */
uint32_t rnet_phy_get_fsk_num_symbols_tx_packet_cfg(const struct sid_pal_radio_tx_packet *tx_packet_cfg,
                                                    uint32_t delay_micro_secs);

/**
 * @brief    get time on air for a given length for the current fsk phy settings
 *
 *  @param    length of the payload.
 *  @return   time on air in milliseconds
 */
uint32_t rnet_phy_get_fsk_time_on_air(uint8_t len);

/** @brief    Configure fsk packet parameters.
 *            The API writes the configures the packet parameters. This does
 *            not put the radio in receive or transmit mode.
 *
 *  @param    fsk_packet_cfg pointer to sid_pal_radio_packet_cfg_t.
 *  @param    fsk_tx_packet_cfg pointer to sid_pal_radio_tx_packet_t.
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */

int32_t rnet_phy_set_fsk_packet_cfg(const sid_pal_radio_packet_cfg_t *fsk_packet_cfg, const
                                    sid_pal_radio_tx_packet_t *tx_packet_cfg);


/** @brief    prepare fsk parameters to be in recive mode
 *            The API writes the configures the packet parameters. This does
 *            not put the radio in receive or transmit mode.
 *
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t rnet_phy_prepare_fsk_receive(void);

/** @brief    rnet phy set fsk transmit packet config.
 *
 *            The API writes the packet config and payload. It does not send
 *            the packet on air.
 *
 *  @param    tx_packet_cfg
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t rnet_phy_set_fsk_tx_packet_cfg(const sid_pal_radio_tx_packet_t *tx_packet_cfg);

/** @brief    Put rnet phy in receive mode with fsk modulation set.
 *
 *  @param    timeout time in ms the radio to be in receive mode.
 *  @param    rx_continuous if set the radio is in continuous receive and
 *            timeout value is ignored
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t rnet_phy_start_fsk_receive(uint32_t timeout, bool rx_continuous);

/** @brief    Puts the radio in Rx mode and senses for preamble detection for upto timeout
 *            duration. If preamble is detected, the Rx mode is terminated. If preamble is not
 *            detected, then depending upon exit mode, either a packet is transmitted or the
 *            function simply terminates
 *  @param    exit_mode of carrier sense
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t rnet_phy_start_fsk_carrier_sense(sid_pal_radio_cad_param_exit_mode_t exit_mode);

/** @brief    Configure radio to transmit a continuous wave in fsk mode.
 *
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t rnet_phy_set_fsk_tx_continuous_wave(void);

/** @brief    rnet phy send fsk packet.
 *            The API writes payload and sends the packet on air.
 *
 *  @param    Rnet Phy transmit packet configuration
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t rnet_phy_send_fsk_packet(sid_pal_radio_tx_packet_t *tx_packet_cfg);

/** @brief    rnet phy prepare fsk packet.
 *            The API prepares PPDU and puts it into radio FIFO without transmitting over the air
 *
 *  @param    Rnet Phy transmit packet configuration
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t rnet_phy_prepare_fsk_packet(sid_pal_radio_tx_packet_t *tx_packet_cfg);



#ifdef __cplusplus
}
#endif

#endif
/** @} */
