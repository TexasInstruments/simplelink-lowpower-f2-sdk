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

#ifndef SID_PAL_RADIO_IFC_H
#define SID_PAL_RADIO_IFC_H

/** @file sid_pal_radio_ifc.h
 *
 * @defgroup sid_pal_lib_radio sid Radio interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details     Provides radio interface to be implemented by platform
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <sid_pal_radio_lora_defs.h>
#include <sid_pal_radio_fsk_defs.h>

#include <sid_time_types.h>

#include <stdint.h>
#include <stdbool.h>

#define SID_PAL_RADIO_RX_PAYLOAD_MAX_SIZE 255

/** Radio Error Codes*/
#define    RADIO_ERROR_NONE                0
#define    RADIO_ERROR_NOT_SUPPORTED       -1  // Feature not supported
#define    RADIO_ERROR_INVALID_PARAMS      -2  // parameters not valid
#define    RADIO_ERROR_IO_ERROR            -3  // Bus errors
#define    RADIO_ERROR_BUSY                -4  // Radio is busy and cannot process commands
#define    RADIO_ERROR_NOMEM               -5  // target buffer has insufficient memory allocated
#define    RADIO_ERROR_HARDWARE_ERROR      -6  // Radio not responding to power up, reset, change of state etc
#define    RADIO_ERROR_INVALID_STATE       -7  // Operation not valid in current radio state
#define    RADIO_ERROR_GENERIC             -8  // Errors reported by library calls
#define    RADIO_ERROR_PKT_CHECK_DCR_LIMIT -9  // DCR check fail
#define    RADIO_ERROR_PKT_CHECK_REG_LIMIT -10 // Regulatory check fail
#define    RADIO_ERROR_VENDOR_FIRST        -64 // Add vendor error codes between -64 and -255
#define    RADIO_ERROR_VENDOR_LAST         -255

/** Sidewalk Phy Radio State*/
#define    SID_PAL_RADIO_UNKNOWN              0
#define    SID_PAL_RADIO_STANDBY              1
#define    SID_PAL_RADIO_SLEEP                2
#define    SID_PAL_RADIO_RX                   3
#define    SID_PAL_RADIO_TX                   4
#define    SID_PAL_RADIO_CAD                  5
#define    SID_PAL_RADIO_STANDBY_XOSC         6
#define    SID_PAL_RADIO_RX_DC                7
#define    SID_PAL_RADIO_BUSY                 8

/** Sidewalk Phy Radio Region For Regional Config*/
typedef enum {
    SID_PAL_RADIO_RC_NONE = 0, // Region none
    SID_PAL_RADIO_RC_NA = 1, // Region North America
    SID_PAL_RADIO_RC_EU = 2, // Region EU
    SID_PAL_RADIO_RC_JP = 3, // Region JP
    SID_PAL_RADIO_RC_MAX // Region max
} sid_pal_radio_region_code_t;

/** Sidewalk Phy Radio Modem Mode*/
typedef enum sid_pal_radio_modem_mode {
    SID_PAL_RADIO_MODEM_MODE_FSK      = 0,
    SID_PAL_RADIO_MODEM_MODE_LORA     = 1,
} sid_pal_radio_modem_mode_t;

/** Sidewalk Phy Radio Event*/
typedef enum {
    SID_PAL_RADIO_EVENT_UNKNOWN       = 0,
    SID_PAL_RADIO_EVENT_TX_DONE       = 1,
    SID_PAL_RADIO_EVENT_RX_DONE       = 2,
    SID_PAL_RADIO_EVENT_CAD_DONE      = 3,
    SID_PAL_RADIO_EVENT_CAD_TIMEOUT   = 4,
    SID_PAL_RADIO_EVENT_RX_ERROR      = 5,
    SID_PAL_RADIO_EVENT_TX_TIMEOUT    = 6,
    SID_PAL_RADIO_EVENT_RX_TIMEOUT    = 7,
    SID_PAL_RADIO_EVENT_CS_DONE       = 8,
    SID_PAL_RADIO_EVENT_CS_TIMEOUT    = 9,
    SID_PAL_RADIO_EVENT_HEADER_ERROR  = 10,
} sid_pal_radio_events_t;

/** Sidewalk Phy Radio Data Rate*/
typedef enum sid_pal_radio_data_rate {
    SID_PAL_RADIO_DATA_RATE_INVALID   = 0,
    SID_PAL_RADIO_DATA_RATE_2KBPS     = 1,
    SID_PAL_RADIO_DATA_RATE_22KBPS    = 2,
    SID_PAL_RADIO_DATA_RATE_50KBPS    = 3,
    SID_PAL_RADIO_DATA_RATE_150KBPS   = 4,
    SID_PAL_RADIO_DATA_RATE_250KBPS   = 5,
    SID_PAL_RADIO_DATA_RATE_12_5KBPS  = 6,
    SID_PAL_RADIO_DATA_RATE_MAX_NUM   = SID_PAL_RADIO_DATA_RATE_12_5KBPS, // 0 is not a valid data rate
} sid_pal_radio_data_rate_t;

typedef enum {
    SID_PAL_RADIO_CAD_EXIT_MODE_CS_ONLY    = 0x00, // Carrier sense only
    SID_PAL_RADIO_CAD_EXIT_MODE_CS_RX      = 0x01, // Carrier sense followed by Rx
    SID_PAL_RADIO_CAD_EXIT_MODE_CS_LBT     = 0x10, // Carrier sense followed by Tx
    SID_PAL_RADIO_CAD_EXIT_MODE_ED_ONLY    = 0x100,// Energy detect only
    SID_PAL_RADIO_CAD_EXIT_MODE_ED_RX      = 0x101,// Energy detect followed by Rx
    SID_PAL_RADIO_CAD_EXIT_MODE_ED_LBT     = 0x110,// Energy detect followed by Tx
    SID_PAL_RADIO_CAD_EXIT_MODE_NONE       = 0x10000,// No CAD mode set
} sid_pal_radio_cad_param_exit_mode_t;

/*!
 * Radio IRQ MASK
 */
typedef enum {
    RADIO_IRQ_NONE                 = (0 << 0),
    RADIO_IRQ_TX_DONE              = (1 << 0),
    RADIO_IRQ_RX_DONE              = (1 << 1),
    RADIO_IRQ_PREAMBLE_DETECT      = (1 << 2),
    RADIO_IRQ_VALID_SYNC_WORD      = (1 << 3),
    RADIO_IRQ_VALID_HEADER         = (1 << 4),
    RADIO_IRQ_ERROR_HEADER         = (1 << 5),
    RADIO_IRQ_ERROR_CRC            = (1 << 6),
    RADIO_IRQ_CAD_DONE             = (1 << 7),
    RADIO_IRQ_CAD_DETECT           = (1 << 8),
    RADIO_IRQ_TXRX_TIMEOUT         = (1 << 9),
    RADIO_IRQ_ALL                  = ((1 << 10 ) - 1),
} sid_pal_radio_irq_mask_t;

/** Sidewalk Phy received LoRa packet status*/
typedef struct sid_pal_radio_rx_packet {
    sid_pal_radio_data_rate_t data_rate;
    sid_pal_radio_lora_rx_packet_status_t lora_rx_packet_status;
    sid_pal_radio_fsk_rx_packet_status_t fsk_rx_packet_status;
    struct sid_timespec rcv_tm;
    uint8_t rcv_payload[SID_PAL_RADIO_RX_PAYLOAD_MAX_SIZE];
    uint8_t payload_len;
} sid_pal_radio_rx_packet_t;

/** Sidewalk Phy settings to configure radio prior to transmission and reception*/
typedef struct sid_pal_radio_packet_cfg {
    sid_pal_radio_data_rate_t data_rate;
    uint8_t channel;
    uint8_t invert_IQ;
    uint16_t preamble_len;
    uint32_t symbol_timeout;
    bool crc_enabled;
} sid_pal_radio_packet_cfg_t;

/** Sidewalk Phy transmit packet configuration*/
typedef struct sid_pal_radio_tx_packet {
    sid_pal_radio_packet_cfg_t packet_cfg;
    int8_t tx_power_in_dbm;
    uint8_t *tx_payload;
    uint8_t payload_len;
    uint32_t timeout;
} sid_pal_radio_tx_packet_t;

/** Sidewalk radio state transition timings*/
typedef struct sid_pal_radio_state_transition_timings {
    uint32_t sleep_to_full_power_us;
    uint32_t full_power_to_sleep_us;
    uint32_t rx_to_tx_us;
    uint32_t tx_to_rx_us;
    uint32_t tcxo_delay_us;
} sid_pal_radio_state_transition_timings_t;

/** Radio event callback*/
typedef void (*sid_pal_radio_event_notify_t)(sid_pal_radio_events_t events);

/** Radio interrupt callback*/
typedef void ( *sid_pal_radio_irq_handler_t )( void );

/** @brief Initializes the radio.
 *
 *  Registers the radio event callback, interrupt callback and pointer to the
 *  protocol's receive packet buffer.
 *  Sets and enables the radio interrupts.
 *
 *  @param[in]   notify routine called as part of bottom half processing of the radio
 *           interrupt.
 *  @param[in]   dio_irq_handler irq handler to notify the protocol that interrupt has
 *           occurred. The protocol switches context to a task or a software interrupt to continue
 *           with the bottom half processing of the interrupt.
 *  @param[in]   rx_packet to protocol's receive packet buffer.
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_init(sid_pal_radio_event_notify_t notify, sid_pal_radio_irq_handler_t dio_irq_handler, sid_pal_radio_rx_packet_t *rx_packet);

/** @brief Configure irq mask.
 *
 *  Configure the interrupts that the low level driver has to generate.
 *  The protocol configures the interrupts it is interested in based on
 *  modem mode. The radio driver has a default mask that can be retrieved
 *  with sid_pal_radio_get_current_config_irq_mask.
 *
 *  @param[in]   irq_mask
 *
 *  @retval  updated configured irq mask settings.
 */
sid_pal_radio_irq_mask_t sid_pal_radio_configure_irq_mask(sid_pal_radio_irq_mask_t irq_mask);

/** @brief Get current irq mask settings.
 *
 *  @retval  current configured irq mask settings.
 */
sid_pal_radio_irq_mask_t sid_pal_radio_get_current_config_irq_mask(void);

/** @brief Radio irq processing.
 *
 *  The function reads the irq status register and reports radio event to the
 *  phy layer through the callback routine registered as part of sid_pal_radio_init.
 *  The protocol after being notified on receiving a radio interrupt switches
 *  from hardware isr to software isr context to continue with bottom half
 *  processing of the radio interrupt.
 *  sid_pal_radio_irq_process should determine the cause of interrupt and notify the
 *  protocol of the phy event through the event handler registered as part of
 *  sid_pal_radio_init.
 *  On packet reception, this API has to copy the received packet from radio
 *  buffers to the rx packet registered as part of sid_pal_radio_init
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_irq_process(void);

/** @brief Set the frequency for the radio.
 *
 *  @param[in]   freq frequency in Hz
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_set_frequency(uint32_t freq);

/** @brief Set the radio transmit power.
 *
 *  @param[in]   power tx power in dB
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_set_tx_power(int8_t power);

/** @brief Get the radio max transmit power setting for a given data rate
 *
 *  @param[in]   data rate
 *  @param[out]  tx_power tx power populated by this API
 *
 *  @retval  On success RADIO_ERROR_NONE, RADIO_ERROR_INVALID_PARAMS otherwise
 */
int32_t sid_pal_radio_get_max_tx_power(sid_pal_radio_data_rate_t data_rate, int8_t *tx_power);

/** @brief Set the radio region
 *
 *  @param[in]   region region
 *
 *  @retval  On success RADIO_ERROR_NONE, RADIO_ERROR_INVALID_PARAMS otherwise
 */
int32_t sid_pal_radio_set_region(sid_pal_radio_region_code_t region);

/** @brief Set the radio to sleep.
 *
 *  The protocol expects the radio to be set in
 *  lowest power consumption state possible.
 *
 *  @param[in] sleep_ms time period for which the radio should be put in sleep mode
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_sleep(uint32_t sleep_ms);

/** @brief Set the radio to standby.
 *
 *  The protocol sets the radio to standby mode in the following scenarios:
 *  To wake the radio from sleep state
 *  To configure modulation parameters, packet parameters, cad, frequency, power etc
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_standby(void);

/** @brief Set the radio to busy state.
 *
 *  Busy state is used for concurrent stacks operation. Set the state of radio to busy to block
 *  another stack from using the radio peripheral. Calling sid_pal_radio_sleep()
 *  replaces SID_PAL_RADIO_BUSY state with SID_PAL_RADIO_SLEEP state.
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_set_radio_busy(void);

/** @brief Set the radio in preamble detect mode
 *
 *  @param[in]   timeout in microseconds for how long radio is in
 *           receive mode. The upper bound of the timeout value is specific to
 *           each vendor's driver implementation.
 *  @param[in]   exit_mode exit mode of carrier sense operation
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_start_carrier_sense(uint32_t timeout, sid_pal_radio_cad_param_exit_mode_t exit_mode);

/** @brief Set the radio in receive mode
 *
 *  @param[in]   timeout in microseconds for how long radio is in
 *           receive mode. The upper bound of the timeout value is specific to
 *           each vendor's driver implementation.
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_start_rx(uint32_t timeout);

/** @brief Set the radio to continuous receive
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_start_continuous_rx(void);

/** @brief Set the receive duty cycle.
 *
 *  Configures the radio receive duty cycle. The protocol uses
 *  this API to set the radio to alternate between receive and sleep
 *  states. The radio in this mode should not interrupt the protocol
 *  unless it detects a valid packet.
 *
 *  @param[in]   rx_time time in milliseconds spent by radio in receive.
 *  @param[in]   sleep_time time in milliseconds spent by radio in sleep.
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_set_rx_duty_cycle(uint32_t rx_time, uint32_t sleep_time);

/** @brief Set the transmit continuous wave.
 *
 *  Confiure the radio to transmit a continuous wave. This API is used for
 *  diagnostics mode only
 *
 *  @param[in]   freq frequency in Hz on which to transmit a continuous wave.
 *  @param[in]   power power in dB at which the continuous wave has to be
 *           transmitted
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_set_tx_continuous_wave(uint32_t freq, int8_t power);

/** @brief Set transmit payload for the radio to transmit.
 *
 *  Writes the payload and payload length to radio buffers but the packet is
 *  not transmitted on the air.
 *
 *  @param[in]   buffer pointer to the buffer containing the tx packet.
 *  @param[in]   size length of the packet that needs to be transmitted.
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_set_tx_payload(const uint8_t *buffer, uint8_t size);

/** @brief Start packet transmission
 *
 *  Starts the packet transmission. This needs to be invoked after all
 *  the radio configuration viz modulation params, packet params, freq,
 *  power, payload and payload length are set atleast once.
 *  The radio should be able to transmit the packet within the timeout
 *  specfied through this API. If it fails to transmit the packet within
 *  the stipulated timeout value, the radio driver should generate a
 *  interrupt with tx timeout as the reason
 *
 *  @param[in]   timeout timeout in microseconds
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_start_tx(uint32_t timeout);

/** @brief Get the radio state.
 *
 *  The protocol uses this API to query its current state
 *
 *  @retval current radio state
 */
uint8_t sid_pal_radio_get_status(void);

/** @brief Get the current radio modem mode.
 *
 *  Supported modem modes are LoRa and FSK
 *  @retval  current configured modem mode
 *           LoRa  = 1 or FSK = 0
 */
sid_pal_radio_modem_mode_t sid_pal_radio_get_modem_mode(void);

/** @brief Set radio modem mode.
 *
 *  The driver should configure all the parameters
 *  to operate in the desired mode.
 *  Supported modem modes are LoRa and FSK
 *
 *  @param[in]   mode LoRa = 1 or FSK = 0
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_set_modem_mode(sid_pal_radio_modem_mode_t mode);

/** @brief Check the channel noise level for a given rssi.
 *
 *  The radio should sense the medium in the frequency specified by parameter freq for a period
 *  specified by delay_us, compute the rssi, and compare the computed rssi with the passed
 *  threshold specified by the parameter threshold.
 *  If the computed rssi is greater than threshold, the parameter is_channel_free
 *  is set to false otherwise it is set to true.
 *  If any of the above operations fail, appropriate error value will be returned.
 *
 *  @param[in]    freq frequency in Hz to measure noise level.
 *  @param[in]    threshold rssi threshold in dBm
 *  @param[in]    delay_us period in microseconds for radio to sense the medium
 *  @param[out]   is_channel_free boolean to store the result of the operation
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_is_channel_free(uint32_t freq, int16_t threshold, uint32_t delay_us, bool *is_channel_free);

/** @brief Compute the noise sensed by radio at a particular frequency
 *
 *  @param[in]    freq frequency in Hz on which noise level is to be measured.
 *  @param[out]   noise pointer to variable to store the avg noise in dBm
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_get_chan_noise(uint32_t freq, int16_t *noise);

/** @brief Get RSSI at radio's current configured frequency
 *
 *  The frequency on which RSSI needs to be measured needs to be set before
 *  calling this API.
 *
 *  @retval  signed integer indicating RSSI in dBm
 */
int16_t sid_pal_radio_rssi(void);

/** @brief Get a random number from radio.
 *
 *  @param[out]   random pointer to store the random number
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_random(uint32_t *random);

/** @brief Get antenna gain in dBi
 *
 *  @retval  antenna gain in dBi.
 */
int16_t sid_pal_radio_get_ant_dbi(void);

/** @brief Get the cca adjustment in dB
 *
 *  @param[in]   data rate
 *  @param[out]  adj_level pointer to CCA level adjustment in dB get by this API
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_get_cca_level_adjust(sid_pal_radio_data_rate_t data_rate, int8_t *adj_level);

/** @brief Get the delay in microseconds to switch between different radio states
 *
 *  @param[out] configuration of switching delay between different radio states
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_get_radio_state_transition_delays(sid_pal_radio_state_transition_timings_t *state_delay);

/** Radio LoRa Modulation specific APIs*/

/** @brief Set LoRa symbol timeout.
 *
 *  @param[in]   num_of_symbols number of symbols the radio has to detect before reporting
 *           a rx timeout interrupt
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_set_lora_symbol_timeout(uint8_t num_of_symbols);

/** @brief Set LoRa sync word
 *
 *  @param[in]   sync_word
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_set_lora_sync_word(uint16_t sync_word);

/** @brief Set LoRa modulation parameters.
 *
 *  @param[in]   mod_params pointer to Sidewalk LoRa modulation params.
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_set_lora_modulation_params(const sid_pal_radio_lora_modulation_params_t *mod_params);

/** @brief Set LoRa packet parameters.
 *
 *  @param[in]   packet_params pointer to Sidewalk LoRa packet params.
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_set_lora_packet_params(const sid_pal_radio_lora_packet_params_t *packet_params);

/** @brief Set LoRa CAD parameters.
 *
 *  @param[in]   cad_params pointer to Sidewalk CAD params.
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_set_lora_cad_params(const sid_pal_radio_lora_cad_params_t *cad_params);

/** @brief Set the radio in CAD mode.
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_lora_start_cad(void);

/** @brief convert LoRa modulation params to data rate.
 *
 *  @param[in]   mod_params pointer to LoRa modulation params.
 *
 *  @retval  data rate
 */
sid_pal_radio_data_rate_t
sid_pal_radio_lora_mod_params_to_data_rate(const sid_pal_radio_lora_modulation_params_t *mod_params);

/** @brief Convert data rate to LoRa modulation parameters.
 *
 *  @param[out]   mod_params pointer to LoRa modulation params.
 *  @param[in]    data_rate.
 *  @param[in]    li_enable enable/disable long interleaver mode
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_lora_data_rate_to_mod_params(sid_pal_radio_lora_modulation_params_t *mod_params,
                                           sid_pal_radio_data_rate_t data_rate, uint8_t li_enable);

/** @brief Get time on air for a LoRa packet.
 *
 *  @param[in]   mod_params pointer to LoRa modulation params.
 *  @param[in]   packet_params pointer to LoRa packet params.
 *  @param[in]   packet_len length of the packet that needs to be transmitted.
 *
 *  @retval  On success a positive number indicating the time on air in ms is returned,
 *           on error 0 is returned
 */
uint32_t sid_pal_radio_lora_time_on_air(const sid_pal_radio_lora_modulation_params_t *mod_params,
                               const sid_pal_radio_lora_packet_params_t *packet_params, uint8_t packet_len);

/**
 * @brief    get CAD duration for a given number of symbols and lora mod_params
 *
 *  @param[in]   symbol symbol timeout for the CAD configuration.
 *  @param[in]   mod_params pointer to LoRa modulation params.
 *
 *  @retval  The time in microseconds for CAD duration
 */
uint32_t sid_pal_radio_lora_cad_duration(uint8_t symbol, const sid_pal_radio_lora_modulation_params_t *mod_params);

/**
 * @brief Calculates the minimum number of symbols that takes more than delay_micro_sec of air time.
 *
 * In the case of a fractional number of symbols, the return value is rounded up to the next integer.
 * Does not affect the radio state and can be executed without radio ownership.
 * In the case of an error, 0 is returned.
 *
 *  @param[in]   mod_params Current modulation parameters that phy uses. If null, zero will be returned.
 *  @param[in]   delay_micro_sec Input amount of time that will be translated to number of symbols.
 *
 *  @retval  number of symbols
 *
 */
uint32_t sid_pal_radio_lora_get_lora_number_of_symbols(const sid_pal_radio_lora_modulation_params_t *mod_params,
                                               uint32_t delay_micro_sec);

/**
 * @brief Get the time between the last bit sent (on Tx side) and the Rx done event (on Rx side)
 *
 * @param[in] mod_params Pointer to a structure holding the LoRa modulation parameters used for the computation
 * @param[in] pkt_params Pointer to a structure holding the LoRa packet parameters used for the computation
 *
 * @retval Delay in microsecond
 */
uint32_t sid_pal_radio_get_lora_rx_done_delay(const sid_pal_radio_lora_modulation_params_t* mod_params,
                                            const sid_pal_radio_lora_packet_params_t* pkt_params);

/**
 * @brief Get the time between Tx schedule and the first bit of Tx
 *
 * @retval Delay in microsecond
 */
uint32_t sid_pal_radio_get_lora_tx_process_delay(void);

/**
 * @brief Get the time of LoRa Rx processing delay
 *
 * @retval Delay in microsecond
 */
uint32_t sid_pal_radio_get_lora_rx_process_delay(void);

/** @brief Get LoRa symbol timeout in us
 *
 *  @param[in]  mod_params pointer to Sidewalk LoRa modulation params.
 *  @param[in]  number_of_symbol number of symbol
 *
 *  @retval  Symbol timeout in us
 */
uint32_t sid_pal_radio_get_lora_symbol_timeout_us(sid_pal_radio_lora_modulation_params_t *mod_params, uint8_t number_of_symbol);

/** Radio FSK  Modulation specific APIs*/

/** @brief Set fsk sync word
 *
 *  @param[in]   sync_word
 *  @param[in]   sync_word_length
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_set_fsk_sync_word(const uint8_t *sync_word, uint8_t sync_word_length);

/** @brief Set fsk whitening seed
 *
 *  @param[in]   seed
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_set_fsk_whitening_seed(uint16_t seed);

/** @brief Set fsk modulation parameters.
 *
 *  @param[in]   mod_params pointer to Sidewalk fsk modulation params.
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_set_fsk_modulation_params(const sid_pal_radio_fsk_modulation_params_t *mod_params);

/** @brief Set fsk packet parameters.
 *
 *  @param[in]   packet_params pointer to Sidewalk fsk packet params.
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_set_fsk_packet_params(const sid_pal_radio_fsk_packet_params_t *packet_params);

/** @brief convert fsk modulation params to data rate.
 *
 *  @param[in]   mp pointer to fsk modulation params.
 *
 *  @retval  data rate
 */
sid_pal_radio_data_rate_t sid_pal_radio_fsk_mod_params_to_data_rate(const sid_pal_radio_fsk_modulation_params_t *mp);

/** @brief Convert data rate to fsk modulation parameters.
 *
 *  @param[out]   mod_params pointer to fsk modulation params.
 *  @param[in]    data_rate  data rate.
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_fsk_data_rate_to_mod_params(sid_pal_radio_fsk_modulation_params_t *mod_params,
                                          sid_pal_radio_data_rate_t  data_rate);

/** @brief Get time on air for a fsk packet.
 *
 *  @param[in]   mod_params pointer to fsk modulation params.
 *  @param[in]   packet_params poiner to fsk packet params.
 *  @param[in]   packet_len length of the packet that needs to be transmitted in bytes.
 *
 *  @retval  On success a positive number indicating the time on air in ms is returned,
 *           on error 0 is returned.
 */
uint32_t sid_pal_radio_fsk_time_on_air(const sid_pal_radio_fsk_modulation_params_t *mod_params,
                              const sid_pal_radio_fsk_packet_params_t *packet_params, uint8_t packet_len);

/**
 * @brief Calculates the minimum number of symbols that takes more than delay_micro_sec of air time.
 *
 * In the case of a fractional number of symbols, the return value is rounded up to the next integer.
 * Does not affect the radio state and can be executed without radio ownership.
 * In the case of an error, 0 is returned.
 *
 *  @param[in]   mod_params Current modulation parameters that phy uses. If null, zero will be returned.
 *  @param[in]   delay_micro_sec Input amount of time in uS that will be translated to number of symbols.
 *
 *  @retval  number of symbols
 *
 */
uint32_t sid_pal_radio_fsk_get_fsk_number_of_symbols(const sid_pal_radio_fsk_modulation_params_t *mod_params,
                                             uint32_t delay_micro_secs);

/** @brief Setup transmit in fsk mode.
 *
 *  This API is used to configure the sync word, packet params
 *  perform crc, and data whitening on the payload, and determine the packet length.
 *  This API needs to be called before calling sid_pal_radio_set_payload
 *  and sid_pal_radio_start_tx in FSK mode.
 *
 *  @param[in/out]   fsk tx packet config
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_prepare_fsk_for_tx(sid_pal_radio_fsk_pkt_cfg_t *tx_pkt_cfg);

/** @brief Setup receive in fsk mode.
 *
 *  This API is used to configure the sync word and packet params.
 *  This API needs to be called before calling sid_pal_radio_start_rx in FSK mode.
 *
 *  @param[in]   rx_pkt_cfg pointer to fsk packet config
 *
 *  @retval  On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t sid_pal_radio_prepare_fsk_for_rx(sid_pal_radio_fsk_pkt_cfg_t *rx_pkt_cfg);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
