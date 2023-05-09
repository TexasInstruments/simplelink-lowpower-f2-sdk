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

#ifndef SID_MAC_FH_API_H
#define SID_MAC_FH_API_H

#include <sid_time_types.h>
#include <sid_memory_pool.h>
#include <sid_error.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_MAX_SKIP 2
#define SDB_MAC_NUM_CHANNEL 69

/*
 * @brief function generates random channel number

 * @param[in] last 4 byte of the device id
 * @param[in] total number of channel
 * @param[out] pointer to carry out the 1st channel
 * @param[out] pointer to carry out the 2st channel
 */
void sid_get_initial_sdb_ch(uint32_t seed, uint8_t total_num_ch, uint8_t *ch1, uint8_t *ch2);

/**
* Opaque handle returned by sid_mac_freq_hop_init().
*/
struct sid_mac_freq_hop_handle;

struct sid_freq_hop_params {
    uint32_t seed;
    uint16_t dwell_time_ms;
    uint16_t guard_time_ms;
    struct sid_timespec epoch_time_ts;
    uint8_t num_of_channels;
    uint8_t num_skip_ch;
    uint8_t skip_ch_list[NUM_MAX_SKIP];
};

/*
 * @brief function to initialize the channel hop context.
 *
 * @param[in]  init_params     channel hop parameters
 * @param[in]  mem_pool_handle pointer to memory pool
 * @param[out] handle       internal context for freq hop module
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_mac_freq_hop_init(struct sid_freq_hop_params *init_params, struct sid_memory_pool *mem_pool_handle, struct sid_mac_freq_hop_handle **handle);

/*
 * @brief function to initialize the channel hop context.
 *
 * @param[in]  handle Channel hop context
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_mac_freq_hop_deinit(struct sid_mac_freq_hop_handle *handle);

/*
 * @brief function to return the current channel to be used.
 *
 * @param[in]   handle          Chan hop context
 * @param[in]   airtime         Expected packet airtime of tx packet in ms. for rx operation 0.
 * @param[out]  channel         Channel to be used
 * @param[out]  remaining_time  Remaining dwell time in the current channel in ms
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_mac_freq_hop_channel(struct sid_mac_freq_hop_handle *handle, uint16_t airtime, uint8_t *channel, uint32_t *remaining_time);

/*
 * @brief function to update the epoch time of connection to complensate for the time drift.
 *
 * @param[in] handle    Chan hop context
 * @param[in] epoch_ts  Updated epoch time of connection
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_mac_freq_hop_epoch(struct sid_mac_freq_hop_handle *handle, struct sid_timespec epoch_ts);

/*
 * @brief function to print the airtime usage for each channel.
 *
 * @param[in] handle Chan hop context
 *
 */
void sid_mac_freq_hop_dump_airtime(struct sid_mac_freq_hop_handle *handle);

/*
 * @brief: function to print the hop table.
 *
 * @param[in] handle Chan hop context
 *
 */
void sid_mac_freq_hop_dump_channel_list(struct sid_mac_freq_hop_handle *handle);

#ifdef __cplusplus
}
#endif

#endif
