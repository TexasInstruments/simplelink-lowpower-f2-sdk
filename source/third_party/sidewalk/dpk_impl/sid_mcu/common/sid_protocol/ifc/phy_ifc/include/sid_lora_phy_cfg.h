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
 * @file sid_lora_phy_cfg.h
 * @brief SID LORA PHY configuration
 * @addtogroup SID_PHY
 *
 * USAGE
 * -----
 *
 * 1) Global handle for lora phy configuration.
 * 2) APIs to get and set lora modulation, packet and cad parameters.
 * 3) APIs to get and set frequency, power for lora mode
 *
 * @{
 */

#ifndef SID_LORA_PHY_CFG_H
#define SID_LORA_PHY_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sid_pal_radio_ifc.h>
#include <sid_pal_radio_lora_defs.h>

#define RNET_LORA_DEFAULT_FREQUENCY 922500000

#define RNET_LORA_DEFAULT_TX_POWER 20
#define RNET_LORA_DEFAULT_HIGH_POWER 11

#define RNET_LORA_DEFAULT_PREAMBLE_LENGTH 250

#define RNET_LORA_DEFAULT_SPREADING_FACTOR SID_PAL_RADIO_LORA_SF7
#define RNET_LORA_DEFAULT_BANDWIDTH SID_PAL_RADIO_LORA_BW_500KHZ
#define RNET_LORA_DEFAULT_CODING_RATE SID_PAL_RADIO_LORA_CODING_RATE_4_6
#define RNET_LORA_DEFAULT_HEADER_TYPE SID_PAL_RADIO_LORA_HEADER_TYPE_VARIABLE_LENGTH
#define RNET_LORA_DEFAULT_PAYLOAD_LENGTH 0
#define RNET_LORA_DEFAULT_CRC_MODE SID_PAL_RADIO_LORA_CRC_ON
#define RNET_LORA_DEFAULT_IQ_MODE SID_PAL_RADIO_LORA_IQ_NORMAL
#define RNET_LORA_DEFAULT_SYNC_WORD SID_PAL_RADIO_LORA_PRIVATE_NETWORK_SYNC_WORD
#define RNET_LORA_DEFAULT_SYMBOL_TIMEOUT 13
#define RNET_LORA_DEFAULT_CAD_SYMBOL_NUM SID_PAL_RADIO_LORA_CAD_04_SYMBOL
#ifdef HALO_WITH_MODULE_HALO_DEV_RADIO_LR1110
#define RNET_LORA_DEFAULT_CAD_DETECT_PEAK 0x32
#define RNET_LORA_DEFAULT_CAD_DETECT_MIN 0xA
#else
#define RNET_LORA_DEFAULT_CAD_DETECT_PEAK 21
#define RNET_LORA_DEFAULT_CAD_DETECT_MIN 10
#endif
#define RNET_LORA_DEFAULT_MAX_CAD_DETECT 0x32
#define RNET_LORA_DEFAULT_CAD_EXIT_MODE SID_PAL_RADIO_CAD_EXIT_MODE_CS_ONLY
#define RNET_LORA_DEFAULT_CAD_TIMEOUT 0
#define RNET_LORA_DEFAULT_RX_TIMEOUT 32768
#define RNET_LORA_NUM_CAD_PARAMS_STORED 4

/** @brief   rnet phy set frequency for LoRa mode.
 *           The API checks the regulatory settings to validate the frequency.
 *
 *  @param   frequency in hz
 *  @return  radio error code.
 */
int32_t rnet_phy_set_lora_frequency(uint32_t freq);

/** @brief   rnet phy set transmit power for LoRa mode.
 *           The API checks the regulatory settings to valid transmit power setting.
 *
 *  @param   power in dB
 *  @return  radio error code
 */
int32_t rnet_phy_set_lora_power(int8_t power);

/** @brief   rnet phy set long interleaved mode for LoRa.
 *
 *  @param   long interleaved enable
 *  @return  radio error code
 */
void rnet_phy_set_lora_long_interleaved(uint8_t li_mode);

/** @brief   set sync word for LoRa mode.
 *
 *  @param   sync_word.
 *  @return  radio error code.
 */
int32_t rnet_phy_set_lora_sync_word(uint16_t sync_word);

/** @brief   set symbol timeout for LoRa mode.
 *
 *  @param   symbol_timeout in milliseconds
 *  @return  radio error code
 */
int32_t rnet_phy_set_lora_symbol_timeout(uint8_t symbol_timeout);

/** @brief   set LoRa modulation parameters.
 *
 *  @param   sid_pal_radio_lora_modulation_params_t.
 *  @return  radio error code
 */
int32_t rnet_phy_set_lora_modulation_params(const sid_pal_radio_lora_modulation_params_t *mod_params);

/** @brief   set LoRa packet parameters.
 *
 *  @param   sid_pal_radio_lora_packet_params_t.
 *  @return  radio error code
 */
int32_t rnet_phy_set_lora_packet_params(const sid_pal_radio_lora_packet_params_t *packet_params);

/** @brief   set LoRa cad parameters.
 *
 *  @param   sid_pal_radio_lora_cad_params_t.
 *  @return  radio error code
 */
int32_t rnet_phy_set_lora_cad_params(const sid_pal_radio_lora_cad_params_t *cad_params);

/** @brief   set LoRa configuration params to radio.
 *           This API should be invoked when switching from fsk mode to LoRa mode.
 *
 *  @return radio error code
 */
int32_t rnet_phy_set_lora_phy_cfg(void);

/** @brief   set LoRa tx timeout.
 *
 *  @param   timeout in millieconds
 */
void rnet_phy_set_lora_tx_timeout(uint32_t tx_timeout);

/** @brief   get LoRa configuration params in lora config handle.
 *
 *  @return  sid_pal_radio_lora_phy_settings_t.
 */
const sid_pal_radio_lora_phy_settings_t *rnet_get_lora_phy_settings(void);

/** @brief   get frequency configured in LoRa mode.
 *
 *  @return  frequency in hz
 */
uint32_t rnet_phy_get_lora_frequency(void);

/** @brief   get tx power configured for LoRa mode.
 *
 *  @return  power in dB
 */
int8_t rnet_phy_get_lora_power(void);

/** @brief   get sync word configured for LoRa mode.
 *
 *  @return  current LoRa sync word.
 */
uint16_t rnet_phy_get_lora_sync_word(void);

/** @brief   get symbol timeout configured for LoRa mode.
 *
 *  @return  symbol timeout in milliseconds
 */
uint8_t rnet_phy_get_lora_symbol_timeout(void);

/** @brief   get LoRa modulation parameters.
 *
 *  @return  sid_pal_radio_lora_modulation_params_t.
 */
const sid_pal_radio_lora_modulation_params_t *rnet_phy_get_lora_modulation_params(void);

/** @brief   get LoRa packet parameters.
 *
 *  @return  sid_pal_radio_lora_packet_params_t.
 */
const sid_pal_radio_lora_packet_params_t *rnet_phy_get_lora_packet_params(void);

/** @brief   get LoRa cad parameters.
 *
 *  @return  sid_pal_radio_lora_cad_params_t.
 */
const sid_pal_radio_lora_cad_params_t *rnet_phy_get_lora_cad_params(void);

/** @brief   get time on air for LoRa mode for a given packet length.
 *
 *  @return  time on air in ms.
 */
uint32_t rnet_phy_get_lora_time_on_air(uint8_t len);

/**
 * @brief    get time on air for a given tx packet config
 *           The API calculates the time on air for a given packet config and payload length.
 *           It neither changes packet config nor uses reads the packet payload.
 *
 *  @param   tx_packet_cfg pointer to sid_pal_radio_tx_packet_t.
 *  @return  On success a positive number indicating the time on air in ms is returned,
 *           on error 0 is returned
 */
uint32_t rnet_phy_get_lora_time_on_air_tx_packet_cfg(const struct sid_pal_radio_tx_packet *tx_packet_cfg);

/**
 * @brief    get cad duration for a given number of symbols and lora packet cfg.
 *
 * In the case of error, an invalid value of 0 will be returned.
 *
 *  @param   Symbol timeout for the CAD configuration.
 *  @param   lora_packet_cfg pointer to lora_packet_cfg.
 *  @return  The time in microseconds for cad duration
 */
uint32_t radio_phy_get_lora_cad_duration(uint8_t symbol, const sid_pal_radio_packet_cfg_t  *lora_packet_cfg);

/**
 * @brief    Returns the number of preamble symbols needed for a given tx_config and backoff_duration.
 *           In the case of fractional symbols, rounds it up.
 *           Does not affect the radio state and can be executed without radio ownership.
 *           In the case of an error, 0 is returned.
 *
 * @param    tx_config
 * @param    Backoff time in ms.
 * @return   Number of symbols needed. In the case of error 0 is returned.
 */
uint32_t rnet_phy_get_lora_num_symbols_tx_packet_cfg(const struct sid_pal_radio_tx_packet *tx_packet_cfg, uint32_t delay_micro_secs);


/** @brief   get tx timeout configured in LoRa mode.
 *
 *  @return  tx timeout configured.
 */
uint32_t rnet_phy_get_lora_tx_timeout(void);

/** @brief   get Long Interleaved configured in LoRa mode.
 *
 *  @return  Long Interleaved configured.
 */
uint8_t rnet_phy_get_lora_li(void);

/**
 * @brief Get the time between the last bit sent (on Tx side) and the Rx done event (on Rx side)
 *
 * @param   tx_packet_cfg pointer to sid_pal_radio_tx_packet_t.
 * @return  On success a positive number indicating the delay in micro second is returned,
 *          On error a negative number indicating the error type is returned.
 */
int32_t rnet_phy_get_lora_rx_done_delay(const struct sid_pal_radio_tx_packet *tx_packet_cfg);

/**
 * @brief Get the time between Tx schedule and the first bit of Tx
 *
 * @return a positive number indicating the delay in micro second
 */
uint32_t rnet_phy_get_lora_tx_process_delay(void);

/**
 * @brief Get the time of LoRa Rx process delay
 *
 * @return a positive number indicating the delay in micro second
 */
uint32_t rnet_phy_get_lora_rx_process_delay(void);

/** @brief    Configure lora packet parameters.
 *            The API configures the packet parameters. This does
 *            not put the radio in receive or transmit mode.
 *
 *  @param    lora_packet_cfg pointer to sid_pal_radio_packet_cfg_t.
 *  @param    payload_len length of payload in bytes if in transmit mode, in receive mode
 *            this parameter is ignored
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t rnet_phy_set_lora_packet_cfg(const sid_pal_radio_packet_cfg_t *lora_packet_cfg,
                                     uint8_t payload_len);

/** @brief    rnet phy set lora transmit packet config.
 *            The API writes the packet config and payload. It does not send
 *            the packet on air.
 *  @param    tx_packet_cfg pointer to sid_pal_radio_tx_packet_t.
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t rnet_phy_set_lora_tx_packet_cfg(const sid_pal_radio_tx_packet_t *tx_packet_cfg);

/** @brief    Put rnet phy in receive mode with lora modulation set.
 *
 *  @param    timeout time in ms for the radio to be in receive mode.
 *  @param    rx_continuous if true the radio is in continuous receive and
 *            timeout value is ignored
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t rnet_phy_start_lora_receive(uint32_t timeout, bool rx_continuous);

/** @brief    Configure the radio to perform cad in lora mode.
 *
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t rnet_phy_start_lora_cad(void);

/** @brief    Configure radio to transmit a continuous wave in lora mode.
 *
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t rnet_phy_set_lora_tx_continuous_wave(void);

/** @brief    rnet phy send lora packet.
 *            The API writes payload and sends the packet on air.
 *
 *  @param    pointer to the payload that needs to be sent.
 *  @param    length of the payload in bytes.
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t rnet_phy_send_lora_packet(const uint8_t *buf, uint8_t len);

/** @brief Calculate time on air for a given length for the current lora phy settings
 *
 *  @param    length of the payload in bytes.
 *  @return   time on air in milliseconds
 */
uint32_t rnet_phy_get_lora_time_on_air(uint8_t len);

/**
 * @brief Get LoRa symbol timeout in us
 *
 * @param   num_of_symbol number of symbol.
 * @return  symbol timeout in us
 */
uint32_t rnet_phy_get_lora_symbol_timeout_us(sid_pal_radio_data_rate_t data_rate, uint8_t num_of_symbol);
#ifdef __cplusplus
}
#endif

#endif
/** @} */
