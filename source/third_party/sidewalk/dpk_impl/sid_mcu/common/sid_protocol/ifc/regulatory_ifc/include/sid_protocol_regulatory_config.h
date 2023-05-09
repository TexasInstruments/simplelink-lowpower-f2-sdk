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
 * @file sid_protocol_regulatory_config.h
 * @brief SID PROTOCOL REGULATORY configuration
 * @addtogroup SID_PROTOCOL_REGULATORY
 *
 * USAGE
 * -----
 *
 * 1) Add region in sid_reg_region_code_t of current header file
 * 2) Add channel group index in current header file if needed by MAC
 * 3) Create channel group config for new region
 * 4) Fill channel group config in sid_reg_init() per region according to MAC design
 *
 * @{
 */

#ifndef SID_PROTOCOL_REGULATORY_CONFIG_H
#define SID_PROTOCOL_REGULATORY_CONFIG_H

#include <sid_pal_radio_ifc.h>
#include <sid_time_types.h>
#include <rnet_errors.h>

#ifdef __cplusplus
extern "C" {
#endif

// The size for fsk channel info. It's equal to the maximum number among
// the channel number of supported fsk bands. This needs to be updated
// accordingly if new band is added for new regions
#define SID_REG_FSK_CHANNELINFO_SIZE            SID_REG_NA_NUM_OF_CHANNELS_125KHZ

/** Region North America */
#define SID_REG_NA_START_FREQUENCY_125KHZ       902200000
#define SID_REG_NA_MAX_TX_POWER_DBM_125KHZ      30
#define SID_REG_NA_NUM_OF_CHANNELS_125KHZ       69
#define SID_REG_NA_START_FREQUENCY_500KHZ       902500000
#define SID_REG_NA_MAX_TX_POWER_DBM_500KHZ      27
#define SID_REG_NA_NUM_OF_CHANNELS_500KHZ       31
#define SID_REG_NA_DWELL_TIME_125KHZ_MS         400

/** Region EU */
#define SID_REG_EU_START_FREQUENCY_M_BAND       868100000
#define SID_REG_EU_MAX_TX_POWER_DBM_M_BAND      14
#define SID_REG_EU_NUM_OF_CHANNELS_M_BAND       3
#define SID_REG_EU_DUTY_CYCLE_M_BAND            1.00
#define SID_REG_EU_START_FREQUENCY_L_BAND       865100000
#define SID_REG_EU_MAX_TX_POWER_DBM_L_BAND      14
#define SID_REG_EU_NUM_OF_CHANNELS_L_BAND       15
#define SID_REG_EU_LBT_CCA_L_BAND               1200
#define SID_REG_EU_LBT_LEVEL_DBM_L_BAND         -80
#define SID_REG_EU_DUTY_CYCLE_L_BAND            2.77

/** Macro for data size */
#define SID_REG_ISO_COUNTRY_STRING_SIZE         3
#define SID_REG_PAUSE_SETTING_SIZE              3

// The maximum number among the channel number of the EU bands, which
// needs to be updated accordingly if DC/DCT is applied for new regions
#define SID_REG_TIME_INFO_NUM_OF_CHANNELS       SID_REG_EU_NUM_OF_CHANNELS_L_BAND

#ifdef SID_REG_SLIDING_DWELL_TIME
// Time series segmentation
// {X0, X1 ...} => X[0:m][0:249n] = {{X0, X0+1n, X0+2n, X0+3n, ..., X0+249n}, ...}
// One byte reserved, a time segment is 250 milli second
// Observed window = Number of segment in observed window * Length of time segment in milli second
#define SID_REG_DT_MAX_TRANSMISSION             (318) // 317.46 63ms-slots within 20s observed window
#define SID_REG_DT_BUF_SIZE                     (SID_REG_DT_MAX_TRANSMISSION + 1) // one element reserved for buffer full condition (rear + 1 == front)
#define SID_REG_DT_OBSERVED_WINDOW_SEC          (20) // Observed window
#define SID_REG_DT_NUMBER_TIME_SEGMENT          (80) // Number of time segment in observed window
#define SID_REG_DT_TIME_SEGMENT_LENGTH_MS       (250) // Length of time segment in milli second
#endif

/** Macro for unit conversion */
#define SID_REG_KHZ_TO_HZ(x)                    ((x) * 1000)

/**@def SID_REG_GET_CHANNEL_SPACING

   Preprocessor Macro to get channel spacing in a channel group.
*/
#define SID_REG_GET_CHANNEL_SPACING(x)        SID_REG_KHZ_TO_HZ((x)->config->channel_spacing)

/**@def SID_REG_GET_CHANNEL_FREQUENCY

   Preprocessor Macro to get frequency in a channel group from channel number.
*/
#define SID_REG_GET_CHANNEL_FREQUENCY(x, y)   ((x)->config->start_frequency + ((y) * SID_REG_GET_CHANNEL_SPACING(x)))

/**@def SID_REG_GET_LBT_CCA

   Preprocessor Macro to get listen-before-talk CCA interval in micro second in a channel group.
*/
#define SID_REG_GET_LBT_CCA(x)                ((x)->config->lbt.cca_us)

/**@def SID_REG_GET_LBT_LEVEL_DBM

   Preprocessor Macro to get listen-before-talk signal level in dBm in a channel group.
*/
#define SID_REG_GET_LBT_LEVEL_DBM(x)          ((x)->config->lbt.level_dbm)

/**@def SID_REG_GET_NUM_OF_CHANNELS

   Preprocessor Macro to get the number of channels in a channel group.
*/
#define SID_REG_GET_NUM_OF_CHANNELS(x)        ((x)->config->num_of_chnls)

/**@def SID_REG_GET_REGION_PARAM

   Preprocessor Macro to get region param in a channel group.
*/
#define SID_REG_GET_REGION_PARAM(x)           ((x)->config->region_param)

/** sid regulatory region codes */
typedef enum {
    SID_REG_RC_NONE = SID_PAL_RADIO_RC_NONE, // Region none
    SID_REG_RC_NA = SID_PAL_RADIO_RC_NA, // Region North America
    SID_REG_RC_EU = SID_PAL_RADIO_RC_EU, // Region EU
    SID_REG_RC_JP = SID_PAL_RADIO_RC_JP, // Region Japan
    SID_REG_RC_MAX // Region Max
} sid_reg_region_code_t;

/** sid regulatory channel group index */
typedef enum {
    SID_REG_CGI_1    = 0, // Channel group one
    SID_REG_CGI_2    = 1, // Channel group two
    SID_REG_CGI_3    = 2, // Channel group three
    SID_REG_CGI_MAX       // Max channel group index
} sid_reg_channel_group_index_t;

/** sid regulatory region parameters which describe channel group bahaviour */
typedef enum {
    SID_REG_REGION_PARAM_NONE     = 0b00000000, // None
    SID_REG_REGION_PARAM_FH       = 0b00000001, // Frequency hopping
    SID_REG_REGION_PARAM_LBT      = 0b00000010, // Listen before talk
    SID_REG_REGION_PARAM_DC_CG    = 0b00000100, // Duty cycle per channel group
    SID_REG_REGION_PARAM_DT       = 0b00001000, // Dwelling time
    SID_REG_REGION_PARAM_DCT_CG   = 0b00010000, // Duty cycle template per channel group
    SID_REG_REGION_PARAM_PT       = 0b00100000, // Pause time
    SID_REG_REGION_PARAM_DC_CH    = 0b01000000, // Duty cycle per channel
    SID_REG_REGION_PARAM_DCT_CH   = 0b10000000  // Duty cycle template per channel
} sid_reg_region_param_t;

/** sid regulatory channel spacing */
// Using KHZ value instead of HZ is to reduce the size of enum to 2 bytes
typedef enum {
    SID_REG_CHANNEL_SPACING_NONE      = 0, // None
    SID_REG_CHANNEL_SPACING_200KHZ    = 200, // 200KHz channel spacing, used for FSK 50Kbps, LoRa SFx/125Khz
    SID_REG_CHANNEL_SPACING_400KHZ    = 400, // 400KHz channel spacing, used for FSK 150Kbps
    SID_REG_CHANNEL_SPACING_500KHZ    = 500, // 500KHz channel spacing, used for FSK 250Kbps
    SID_REG_CHANNEL_SPACING_800KHZ    = 800 // 800KHz channel spacing, used for LoRa SFx/500Khz
} sid_reg_channel_spacing_t;

/** sid regulatory pause time setting */
typedef enum {
    SID_REG_PARAM_PAUSE_NONE     = 0,
    SID_REG_PARAM_PAUSE_10_TIMES = 1, // Pause ten times of time on air
    SID_REG_PARAM_PAUSE_2MS      = 2, // Pause 2ms
    SID_REG_PARAM_PAUSE_50MS     = 50, // Pause 50ms
    SID_REG_PARAM_PAUSE_100MS    = 100 // Pause 100ms
} sid_reg_param_pause_t;

/** sid regulatory regional info */
typedef struct sid_reg_regional {
    sid_reg_region_code_t region; // Region of channel group
    char iso_country[SID_REG_ISO_COUNTRY_STRING_SIZE]; // ISO 3166 Alpha2 country code
} sid_reg_regional_t;

/** sid regulatory listen-before-talk info */
typedef struct sid_reg_lbt {
    uint16_t cca_us; // Clear channel assessment in usec
    int16_t level_dbm; // Listen before talk level
} sid_reg_lbt_t;

/** sid regulatory pause time info */
typedef struct sid_reg_pause {
    struct {
        uint16_t lower_bound; // Lower bound (exclusive)
        uint16_t upper_bound; // Upper bound (exclusive)
    } sending_duration; // Sending duration in ARIB-T108 part 2
    sid_reg_param_pause_t pause_type; // Pause type in ARIB-T108 part 2
} sid_reg_pause_t;

/** sid regulatory channel group configuration */
typedef struct sid_reg_channel_group_config {
    uint32_t start_frequency; // Start frequency in Hz
    sid_reg_channel_spacing_t channel_spacing; // Channel spacing in Hz
    uint8_t num_of_chnls; // Number of channels
    int8_t tx_max_power; // Tx max power
    uint16_t region_param; // Region parameter
    uint16_t dwell_time; // Dwell time in ms
    uint16_t duty_cycle; // Duty cycle
    sid_reg_lbt_t lbt; // Lbt
    sid_reg_pause_t pause[SID_REG_PAUSE_SETTING_SIZE]; // Pause
} sid_reg_channel_group_config_t;

/** sid regulatory DC time infomation */
typedef struct sid_reg_dc_time_info {
    struct sid_timespec start_time; // DC start TX aggregation time
    int32_t allow_tx_time_ms; // Allow tx time in millisecond
} sid_reg_dc_time_info_t;

/** sid regulatory DCT time infomation */
typedef struct sid_reg_dct_time_info {
    struct sid_timespec available_time; // DCT absolute available time
} sid_reg_dct_time_info_t;

/** sid regulatory PT time infomation */
typedef struct sid_reg_pt_time_info {
    struct sid_timespec available_time; // PT absolute available time
} sid_reg_pt_time_info_t;

/** sid regulatory channel group info */
typedef struct sid_reg_channel_group {
    sid_reg_channel_group_index_t index; // Index of channel group
    const sid_reg_channel_group_config_t *config; // Channel group config
    struct {
        // DC/DCT parameters are exclusive
        union {
            sid_reg_dc_time_info_t dc_channel_group; // Channel group DC time information
            sid_reg_dc_time_info_t dc_channels[SID_REG_TIME_INFO_NUM_OF_CHANNELS]; // Per channel DC time information
            sid_reg_dct_time_info_t dct_channel_group; // Channel group DCT time information
            sid_reg_dct_time_info_t dct_channels[SID_REG_TIME_INFO_NUM_OF_CHANNELS]; // Per channel DCT time information
        };
        sid_reg_pt_time_info_t pt_channel_group; // Channel group PT time information
    } time_info;
} sid_reg_channel_group_t;

/** sid lbt cca level */
typedef struct sid_reg_lbt_cca_level {
    sid_reg_channel_group_t *channel_group;
    int16_t ant_dbi;
    int8_t tx_pwr;
    int8_t adj_level;
} sid_reg_lbt_cca_level_t;

#ifdef SID_REG_SLIDING_DWELL_TIME
typedef struct sid_reg_dt_transmission {
    uint8_t ltd : 1; // Limited bit raised if new dwell time set
    uint8_t chnl : 7;
    uint8_t toa;
    uint8_t rel_time_ms;
} sid_reg_dt_transmission_t;

typedef struct sid_reg_dt_window {
    sid_reg_dt_transmission_t tx[SID_REG_DT_BUF_SIZE];
    uint8_t tx_count[SID_REG_DT_NUMBER_TIME_SEGMENT];
    struct sid_timespec start_time;
    int16_t front;
    int16_t rear;
} sid_reg_dt_window_t;

typedef struct sid_reg_dt_param {
    sid_reg_channel_group_t *channel_group;
    uint8_t channel;
    uint32_t time_on_air_ms;
} sid_reg_dt_param_t;

typedef struct sid_reg_dt_limit {
    int8_t id;
    uint16_t dwell_time;
} sid_reg_dt_limit_t;
#endif

/** @brief Set region ready
 *
 *  The API set region ready to regulatory.
 *
 *  @param ready set true if ready, else false
 */
void sid_reg_set_region_ready(bool ready);

/** @brief Get region ready flag from regulatory.
 *
 *  The API get region ready flag from regulatory.
 *
 *  @return sid_reg_region_ready.
 */
bool sid_reg_get_region_ready(void);

/** @brief sid regulatory set regional info, including region code and ISO country.
 *
 *  @param region Supported region code
 *  @param country 2 bytes in ISO3166 Alpha2 format.
 *  @return RNET_SUCCESS.
 */
rnet_error_t sid_reg_set_regional_info(const sid_reg_region_code_t region, const char *country);

/** @brief sid regulatory get regional info, including region code and ISO country.
 *
 *  @return pointer to sid_reg_regional_info.
 */
sid_reg_regional_t *sid_reg_get_regional_info(void);

/** @brief Select channel group.
 *
 *  The API select channel group from regulatory for MAC.
 *
 *  @param idx of sid_reg_channel_group_index_t.
 *  @return rnet error code.
 */
rnet_error_t sid_reg_select_channel_group(sid_reg_channel_group_index_t idx);

/** @brief Get current group index from regulatory.
 *
 *  The API get channel group index from regulatory.
 *
 *  @return current group index of type sid_reg_channel_group_index_t.
 */
sid_reg_channel_group_index_t sid_reg_get_selected_channel_group_index(void);

/** @brief Get channel group from channel group index.
 *
 *  The API get channel group from regulatory.
 *
 *  @param idx of sid_reg_channel_group_index_t.
 *  @return channel group of type sid_reg_channel_group_t.
 */
sid_reg_channel_group_t *sid_reg_get_channel_group(sid_reg_channel_group_index_t idx);

/** @brief Set current packet time on air in ms to regulatory.
 *
 *  The API set current packet time on air in ms to regulatory.
 *
 *  @param time_on_air_ms time on air of a transmit packet.
 */
void sid_reg_set_current_packet_time_on_air(uint32_t time_on_air_ms);

/** @brief sid regulatory get frequency when channel number is passed.
 *
 *  The API returns valid frequency by channel.
 *
 *  @param channel_group channel group pointer.
 *  @param channel channel number.
 *  @param pointer to frequency that will be filled.
 *  @return rnet error code
 */
rnet_error_t sid_reg_get_freq_by_channel(sid_reg_channel_group_t *channel_group, uint8_t channel, uint32_t *freq);

/** @brief sid regulatory get maximum tx power limit based on channel group
 *
 *  @param channel_group channel group pointer.
 *  @param ant_dbi antenna gain in dBi
 *  @return power.
 */
int8_t sid_reg_get_max_tx_power(sid_reg_channel_group_t *channel_group, int16_t ant_dbi);

/** @brief Get LBT cca level dynamically.
 *
 *  The API return success with cca level in dBm.
 *
 *  @param cca_cfg cca level configuration pointer.
 *  @param pointer to cca level in dBm.
 *  @return rnet error code
 */
rnet_error_t sid_reg_get_lbt_cca_level(sid_reg_lbt_cca_level_t *cca_cfg, int16_t *level_dbm);

#ifdef SID_REG_SLIDING_DWELL_TIME
/** @brief Limit the dwell time of particular channel group
 *
 *  The API return success when limitation set correctly.
 *
 *  @param channel_group channel group pointer.
 *  @param dwell_time dwelling time.
 *  @return rnet error code
 */
rnet_error_t sid_reg_set_dt_limited_dwell_time(sid_reg_channel_group_t *channel_group, uint16_t dwell_time);
#endif

/** @brief sid regulatory do the time check.
 *
 *  The API do the following time check for different region and country.
 *   1. Dwell time check, time a frame on the air not to exceed value in channel group.
 *   2. Duty cycle template check
 *   3. Duty cycle check
 *   4. Pause time check
 *
 *  @param channel_group channel group pointer.
 *  @param channel channel index.
 *  @param time_on_air_ms time a frame on the air in ms.
 *  @return rnet error code
 */
rnet_error_t sid_reg_time_check(sid_reg_channel_group_t *channel_group, uint8_t channel, uint32_t time_on_air_ms);

/** @brief sid regulatory update time information.
 *
 *  The API update the time information into regulatory library
 *  when receive a TX done event from PHY layer, it is called by
 *  each MAC submodule and time information is used for calculation
 *  in sid_reg_time_check().
 *
 *  @param channel_group channel group pointer.
 *  @return rnet error code
 */
rnet_error_t sid_reg_update_txdone_info(sid_reg_channel_group_t *channel_group);

/** @brief Initializes sid_reg regulatory and channel group.
 *
 *  The API init channel group base on region code.
 *
 *  @return rnet error code.
 */
rnet_error_t sid_reg_init(void);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
