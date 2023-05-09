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

#ifndef HALO_LIB_RINGNET_WAN_SRC_MAC_HALO_INCLUDE_RNET_MAC_CHANNEL_HOPPING_H_
#define HALO_LIB_RINGNET_WAN_SRC_MAC_HALO_INCLUDE_RNET_MAC_CHANNEL_HOPPING_H_

#include <sid_protocol_defs.h>
#include <rnet_mac_channel_hopping_sdsq.h>

#include <stdbool.h>

#define RNET_MAC_CH_INCREMENTAL_SEQ_LIMIT   7 // CHSIDs [0-7] are the incremental sequences.
#define RNET_MAC_CH_INCREMENTAL_SEQ_SEED_SIZE 1 // For CHSIDs 0-7 the seed is implicit.
#define RNET_MAC_CH_LCG_ID                  8 //The CHSID for using the jrand48 generator
#define RNET_MAC_CH_LCG_ID_SEED_SIZE        3*sizeof(unsigned short)
#define RNET_MAC_CH_SDSQ_ID                 9 // The CHSID for using seed and sequence number generator
#define RNET_MAC_CH_SDSQ_ID_SEED_SIZE       6 // [4B randomly generated val][2B bcn hop sequence number]
#define RNET_MAC_CHSID_MAX_VAL              RNET_MAC_CH_SDSQ_ID
#define RNET_MAC_CH_IS_INCREMENTAL_CHSID(X) (X<=RNET_MAC_CH_INCREMENTAL_SEQ_LIMIT)

#if RNET_MAC_MODE_FSK
#define RNET_MAC_CH_REGULATORY_BCN_NA       SID_REG_CGI_1
#define RNET_MAC_CH_REGULATORY_BCN_EU       SID_REG_CGI_1
#define RNET_MAC_CH_REGULATORY_HDR_NA       SID_REG_CGI_1
#define RNET_MAC_CH_REGULATORY_HDR_EU       SID_REG_CGI_2
#else
#define RNET_MAC_CH_REGULATORY_BCN_NA       SID_REG_CGI_2
#define RNET_MAC_CH_REGULATORY_HDR_NA       SID_REG_CGI_2
#endif

#define RNET_MAC_CH_ON_DECK_NA              5 // for NA: need to consider number of channels on deck
#define RNET_MAC_CH_ON_DECK_EU              0 // For EU: no need to consider number of channels on deck
#define RNET_MAC_CH_PREV_AVOID_NA           0
#define RNET_MAC_CH_PREV_AVOID_EU           2 // For EU: need to avoid immediate re-use of the last two channels in the previous hop sequence

//Definition for the memory allocated for the seed storage. The CHSID requiring
//larger sequences are rejected
#define RNET_MAC_CH_RANDOM_SEED_ARRAY_MAX_SIZE RNET_MAC_CH_LCG_ID_SEED_SIZE

/*!
 * @brief Sets the channel hopping sequence ID that defines the channel hopping sequence.
 * @param chsid: The hopping sequence ID
 * @return void
*/
void rnet_mac_set_chsid(uint8_t chsid);

/*!
 * @brief Gets the channel hopping sequence ID, which defines the channel hopping sequence.
 * @return The channel increment value
*/
uint8_t rnet_mac_get_chsid(void);

/*!
 * @brief Lookup for the the size of the seed used by a given CHSID
 * @param chsid: The hopping sequence ID
 * @return The size of the seed in bytes
*/
size_t rnet_mac_seed_size(uint8_t chsid);

/*!
 * @brief Initializes the seed
 * @return success or failure
*/
bool rnet_mac_initialize_channel_seed(void);

/*!
 * @brief Returns the seed identifier of the current frame.
 * The seed identifies a state within the channel hopping sequence.
 * This function copies the stored value that indicates the current state.
 * @param[out] rs The seed is written to the address defined by this pointer.
 *                The size of the seed written is defined by the CHSID.
 *                If null, no operation is performed.
 * @return seed value
*/
void rnet_mac_get_channel_seed(void* rs);

/*!
 * @brief Sets the seed identifier of the current frame.
 * The seed identifies a position within the channel hopping sequence.
 * This function accepts a new value to be stored reflecting the position within
 * the channel hopping sequence.
 * @param[in] seed: The new seed value to be stored. The size of the seed written
 *                  is defined by the current CHSID setting .
 * @return void.
*/
void rnet_mac_set_channel_seed(const void* rs);

/*!
 * @brief The seed update function for current state of <seed, CHSID> pair.
 * Updates the internal seed to the next value in the current channel hopping sequence,
 * which is identified with the current setting of the CHSID.
 * @return void
*/
void rnet_mac_update_channel_seed();

/*!
 * @brief The channel corresponding to the current state of <seed, CHSID> pair.
 * @return Channel value calculated.
 *         In case of an error a value larger channel_number is returned.
*/
uint8_t rnet_mac_map_seed_to_channel_bcn();

/*!
 * @brief Calculated HDR channel corresponding to a given slot_num.
 * @param slot_num:  Slot_num relative to the SFT for which <seed, CHSID> pair is updated.
 * @return Channel value calculated.
 *         In case of an error a value larger channel_number is returned.
*/
uint8_t rnet_mac_map_seed_to_channel_hdr(uint16_t slot_num);

#endif /* HALO_LIB_RINGNET_WAN_SRC_MAC_HALO_INCLUDE_RNET_MAC_CHANNEL_HOPPING_H_ */
