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
 * @file sid_protocol_phy.h
 * @brief SID PHY layer Interface
 * @addtogroup SID_PHY
 *
 * USAGE
 * -----
 *
 * 1) sid_phy_init initializes the sid phy layer.
 * 2) Register a callback with sid phy layer to receive radio events.
 * 2) Set sid modem mode and set configuration to the selected radio
 *    configuration.
 * 3) Set the tx payload and payload length after configuring modulation and
 *    packet params and trigger sending of a packet by called radio_start_tx.
 * 4) Put the radio in receive, sleep, cad, energy detect and standby mode.
 * 5) In order to support FSK only mode, LORA only mode and FSK and LORA mode
 *    at build time, similar APIs are provided separately.
 *
 * @{
 */

#ifndef SID_PROTOCOL_PHY_H
#define SID_PROTOCOL_PHY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sid_pal_radio_ifc.h>
#include <sid_lora_phy_cfg.h>
#include <sid_fsk_phy_cfg.h>

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

enum mode_specific_features {
    LONG_INTERLEAVED_SUPPORT = 0,
};

/** sid phy event callback*/
typedef void (*sid_phy_event_notify_t)(sid_pal_radio_events_t events);

/** sid phy callback interface */
typedef struct sid_phy_callback {
    sid_pal_radio_irq_handler_t irq_handler; // Radio HW to SW
    sid_phy_event_notify_t radio_event_notify; // PHY to MAC
    // Notify sid process callback
    void (*process_notify)(void); // PHY to PHY process
    // SWI event handler callback
    void (*swi_event_notify)(void); // PHY process to PHY
} sid_phy_callback_t;

/** @brief    Initialize sid phy.
 *            Initializes global sidh_radio handle.
 *            Initializes and registers with vendor radio driver.
 *            Sets the default modem mode to LORA
 *
 *  @param    phy_callback pointer to sid_phy_callback_t
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_phy_init(const sid_phy_callback_t *phy_callback);

/** @brief Sid phy process
 *
 *  The process to handle radio event which can not be done in interrupt context
 *
 */
void sid_phy_process(void);

/** @brief Sid phy swi event notifier
 *
 *  SWI event notifier to MAC layer
 *
 */
void sid_phy_swi_event_notifier(void);

/** @brief    Register callback with sid phy to receive phy events.
 *
 *  @param    radio_event_callback callback of type sid_phy_event_notify_t.
 *
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_phy_register_phy_event_callback(sid_phy_event_notify_t radio_event_callback);

/** @brief    Set the phy layer in sleep state.
 *            If the phy state is not in sleep, radio is set to sleep mode.
 *  @param    period in milliseconds the phy should be in sleep.
 *
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_phy_sleep(uint32_t sleep_ms);

/** @brief    Set the phy layer in standby state.
 *            If the phy state is not in standby, radio is set to standby mode.
 *
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_phy_standby(void);

/** @brief    Get sid phy state.
 *
 *  @return   current radio state
 */
uint8_t sid_phy_get_radio_state(void);

/** @brief    Get sid phy last radio event
 *
 *  @return   last radio event
 */
sid_pal_radio_events_t sid_phy_get_last_radio_event(void);

/** @brief    Retrieve the received packet.
 *            On receiving rx done event this api would need to be called
 *            to get the received packet.
 *
 *  @return   sid_pal_radio_rx_packet_t.
 */
sid_pal_radio_rx_packet_t *sid_phy_get_rx_packet(void);

/** @brief    Get radio channel noise.
 *
 *  @param    freq Frequency in Hz on which noise is measured.
 *  @return   noise measured
 */
int16_t sid_phy_get_radio_channel_noise(uint32_t freq);

/** @brief    sid phy set transmit packet config.
 *            The API writes the packet config and payload. It does not send
 *            the packet on air.
 *
 *  @param    tx_packet_cfg pointer to sid_pal_radio_tx_packet_t.
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_phy_set_tx_packet_config(const sid_pal_radio_tx_packet_t *tx_packet_cfg);

/** @brief    Send the packet on air
 *            The API sends the packet on air after tx packet config is set using
 *            sid_phy_set_tx_paacket_config.
 *
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_phy_start_tx(void);

/** @brief    Puts the radio in Rx mode and senses for preamble detection for upto timeout
 *            duration. If preamble is detected, the Rx mode is terminated. If preamble is not
 *            detected, then depending upon exit mode, either a packet is transmitted or the
 *            function simply terminates
 *  @param    Time duration of carrier sense in microseconds
 *  @param    exit_mode of carrier sense
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_phy_carrier_sense(uint32_t timeout, sid_pal_radio_cad_param_exit_mode_t exit_mode);

/** @brief    Compares the channel noise level with a threshold to declare whether the channel is free.
 *            The config should be set before this method is called.
 *            For free channels, the actual time this method takes can be slightly
 *            larger than this delay but not shorter.
 *            For busy channels, the time it takes can be shorter as busy signal
 *            is returned as soon as an activity is detected.
 *
 *  @param    threshold in dBm to declare the channel busy
 *  @param    Time duration in milliseconds
 *  @return   true if the channel is free, false otherwise.
 */
bool sid_phy_is_channel_free(int16_t threshold, size_t timedelay_us);

/** @brief    Configure the radio to perform cad operation.
 *            The radio, channel and modulation should be set to
 *            the desired values before this method is invoked.
 *            The duration this method takes is dicatated by the CAD
 *            parameters of the underlying PHY.
 *
 *  @param    cad_exit_mode Defines the exit mode for the cad operation
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_phy_start_cad(sid_pal_radio_cad_param_exit_mode_t cad_exit_mode);

/** @brief    Put sid phy in receive mode.
 *
 *  @param    timeout time in microseconds the radio will be in receive mode.
 *  @param    rx_continuous if true the radio is in continuous receive and
 *            timeout value is ignored
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_phy_start_rx(uint32_t timeout, bool rx_continuous);

/** @brief    Configure radio to transmit a continuous wave.
 *
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_phy_set_tx_continuous_wave(void);

/** @brief    Configure receive mode parameters.
 *            The API configures the radio receive parameters. This does
 *            not put the radio in receive mode.
 *
 *  @param    rx_packet_cfg pointer to sid_pal_radio_packet_cfg_t.
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_phy_set_rx_packet_config(const sid_pal_radio_packet_cfg_t *rx_packet_cfg);

/** @brief    Set the rx duty cycle for the radio.
 *            Configure the radio to go to sleep and turn on the receiver without the
 *            intervention of the host. The radio reports an event if cad is detected.
 *
 *  @param    rx_time time in ms the radio spends in receive mode.
 *  @param    sleep_time time in ms the radio spends sleeping.
 *  @return   On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_phy_set_rx_duty_cycle(uint32_t rx_time, uint32_t sleep_time);

/** @brief    returns the instantenous rssi reading from the radio
 *
 *  @return   rssi in dBm
 */
int16_t sid_phy_radio_rssi();

/**
 * @brief     Get time on air for a given tx packet config
 *            The API calculates the time on air for a given packet config and payload length.
 *            It neither changes packet config nor reads the packet payload.
 *
 *  @param   tx_packet_cfg
 *  @param   modem mode
 *  @return  On success a positive number indicating the time on air in ms is returned,
 *           on error 0 is returned
 */
uint32_t sid_phy_get_time_on_air_tx_packet_cfg_mode(const struct sid_pal_radio_tx_packet *tx_packet_cfg, sid_pal_radio_modem_mode_t mode);

/**
 * @brief     Get time on air for a given tx packet config
 *            The API calculates the time on air for a given packet config and payload length.
 *            It neither changes packet config nor reads the packet payload.
 *
 *  @param    tx_packet_cfg
 *  @return   The time in ms for a given packet payload.
 */
uint32_t sid_phy_get_time_on_air_tx_packet_cfg(const struct sid_pal_radio_tx_packet *tx_packet_cfg);

/**
 * @brief     Returns the number of preamble symbols needed for a given tx_config and backoff_duration.
 *            In the case of fractional symbols, rounds it up.
 *            Does not affect the radio state and can be executed without radio ownership.
 *            In the case of an error, 0 is returned.
 *
 * @param     tx_packet_cfg
 * @param     Backoff amount in ms.
 * @return    Number of symbols needed. In the case of error 0 is returned.
 */
uint32_t sid_phy_time_to_num_symbols_tx_packet_cfg(const struct sid_pal_radio_tx_packet *tx_packet_cfg,
                                                    uint32_t backoff_micro_sec);

/**
 * @brief     Returns the number of preamble symbols needed for a given tx_config and backoff_duration.
 *            In the case of fractional symbols, rounds it up.
 *            Does not affect the radio state and can be executed without radio ownership.
 *            In the case of an error, 0 is returned.
 *
 * @param     tx_packet_cfg
 * @param     Backoff amount in ms.
 * @param     modem mode.
 * @return    Number of symbols needed. In the case of error 0 is returned.
 */
uint32_t sid_phy_time_to_num_symbols_tx_packet_cfg_mode(const struct sid_pal_radio_tx_packet *tx_packet_cfg,
                                                         uint32_t backoff_micro_sec, sid_pal_radio_modem_mode_t mode);

/**
 * @brief     Calculates the maximum transmit power allowed for a given data rate.
 * The value returned includes external PA if the platform has one but excludes the antenna gain.
 * The value returned complies with the reguatory settings based on previously set channel group.
 *
 *  @param    data_rate: The data rate for which the max power is calculated
 *  @param[out] tx_power_ptr: Pointer to which the calculated power is written.
 *  @return  On success RADIO_ERROR_NONE is returned
 *           On failure, an error condition indicating the error will be returned.
 */
int32_t sid_phy_get_max_tx_power(sid_pal_radio_data_rate_t data_rate, int8_t *tx_power_ptr);

/**
 * @brief     Get cad duration for a given radio config and modem mode setting.
 *            The API calculates the time required for a given packet config and modem mode
 *            without modifying actual radio configuration. For failures, a value of zero is
 *            returned.
 *
 *  @param[in]    radio_cfg The radio configuration for which CAD will be performed
 *  @param    exit_mode The exit mode of CAD operation
 *  @param    mode The radio mode for which CAD will be performed
 *  @return   The expected cad time in microseconds.
 */
uint32_t sid_phy_get_cad_duration(const sid_pal_radio_packet_cfg_t *radio_cfg,
        sid_pal_radio_cad_param_exit_mode_t exit_mode, sid_pal_radio_modem_mode_t mode);

/**
 * @brief Get the rx processing delay for the specific radio mode. RX processing delay is defined as
 *        the time that it takes the radio from scheduling of RX window to opening it.
 *
 * @param  sid_pal_radio_modem_mode_t mode SID_PAL_RADIO_MODEM_MODE_FSK/SID_PAL_RADIO_MODEM_MODE_LORA
 * @return success/failure
 */
uint32_t sid_phy_get_rx_process_delay(sid_pal_radio_modem_mode_t mode);

/**
 * @brief Get the TX processing delay for the specific radio mode. TX processing delay is defined as
 *        the time that it takes the radio from scheduling of TX window to sending it over the air.
 *
 * @param  sid_pal_radio_modem_mode_t mode SID_PAL_RADIO_MODEM_MODE_FSK/SID_PAL_RADIO_MODEM_MODE_LORA
 * @return success/failure
 */
uint32_t sid_phy_get_tx_process_delay(sid_pal_radio_modem_mode_t mode);

/**
 * @brief Get RX done delay for specific radio mode
 *
 * @param  tx_power - power to set
 * @param  sid_pal_radio_modem_mode_t mode SID_PAL_RADIO_MODEM_MODE_FSK/SID_PAL_RADIO_MODEM_MODE_LORA
 * @return success/failure
 */
int32_t sid_phy_get_rx_done_delay(const struct sid_pal_radio_tx_packet *tx_packet_cfg,
    sid_pal_radio_modem_mode_t mode);

/**
 * @brief Get the time a packet will spend in the air given the radio mode
 *
 * @param  payload length - length of payload in bytes
 * @param  sid_pal_radio_modem_mode_t mode SID_PAL_RADIO_MODEM_MODE_FSK/SID_PAL_RADIO_MODEM_MODE_LORA
 * @return success/failure
 */
uint32_t sid_phy_get_time_on_air(uint8_t payload_length, sid_pal_radio_modem_mode_t mode);

/**
 * @brief Toggle features for the radio which may be dependent on radio mode
 *
 * @param  mode_specific_features - name of feature
 * @param  enable - state to set
 * @param  sid_pal_radio_modem_mode_t mode SID_PAL_RADIO_MODEM_MODE_FSK/SID_PAL_RADIO_MODEM_MODE_LORA
 * @return success/failure
 */
int32_t sid_phy_toggle_features(enum mode_specific_features feature, bool enable, sid_pal_radio_modem_mode_t mode);

/**
 * @brief Get symbol timeout in us
 *
 * @param     data_rate data rate.
 * @param     num_of_symbol number of symbol.
 * @param     modem mode.
 * @return    symbol timeout in us. In the case of error 0 is returned.
 */
uint32_t sid_phy_get_symbol_timeout_us(sid_pal_radio_data_rate_t data_rate, uint8_t num_of_symbol, sid_pal_radio_modem_mode_t mode);

/**
 * @brief sid phy update regulatory region to radio driver
 *
 *  @return success/failure
 */
int32_t sid_phy_update_radio_region(void);

/**
 * @brief  Get radio state transition timings.
 *
 * @param  sid_pal_radio_state_transition_timings_t delays
 * @return 0 is returned success or a negative value otherwise
 */
int32_t sid_phy_get_radio_state_transition_delays(sid_pal_radio_state_transition_timings_t *delay);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
