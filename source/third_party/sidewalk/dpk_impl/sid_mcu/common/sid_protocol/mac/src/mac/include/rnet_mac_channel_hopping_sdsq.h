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

#ifndef HALO_LIB_RINGNET_WAN_SRC_MAC_HALO_INCLUDE_RNET_MAC_CHANNEL_HOPPING_SDSQ_H_
#define HALO_LIB_RINGNET_WAN_SRC_MAC_HALO_INCLUDE_RNET_MAC_CHANNEL_HOPPING_SDSQ_H_

/*!
 * @brief Set the channel number for beacon channel hopping
 * @param[in] num_of_chnls: number of channels
 * @return void.
*/
void rnet_mac_set_sdsq_bcn_num_of_chnls(uint16_t num_of_chnls);

/*!
 * @brief Set the channel number for hdr channel hopping
 * @param[in] num_of_chnls: number of channels
 * @param[in] chnls_on_deck: number of channels on deck to be considered for hdr channel decision
 * @param[in] avoid_prev: number of channels in the previous hop sequence on which immediate re-use should be avoided
 * @return void.
*/
void rnet_mac_set_sdsq_hdr_num_of_chnls(uint16_t num_of_chnls, uint8_t chnls_on_deck, uint8_t avoid_prev);

/*!
 * @brief Returns the beacon channel for the beacon period
 * @param[in] seed: Unique non-changing seed from gateway
 * @param[in] seqn: Sequence number incrementing every beacon period
 * @return Channel value calculated.
 *         In case of an error a value larger channel_number is returned.
*/
uint8_t rnet_mac_get_bcn_channel_sdsq(uint32_t seed, uint16_t seqn);

/*!
 * @brief Returns the channel for the beacon hop sequence number and slot
 * @param[in] seed: Unique non-changing seed from gateway
 * @param[in] seqn: Sequence number incrementing every beacon period
 * @param[in] slot: Slot number in current beacon period
 * @return Channel value calculated.
 *         In case of an error a value larger channel_number is returned.
*/
uint8_t rnet_mac_get_hdr_channel_sdsq(uint32_t seed, uint16_t seqn, uint8_t slot);

/*!
 * @brief Get the wrap around value for the sequence number in beacon seed
 * @return Value when sequence number much wrap around to 0
*/
uint16_t rnet_mac_get_bcn_seqn_wrap_sdsq(void);

/*!
 * @brief Extract the sequence number from a 6B hop seed
 * @param[in] rs: Pointer to 6B hop seed [4B device id][2B beacon sequence number]
 * @return Sequence number portion of the 6B hop seed
*/
static inline uint16_t rnet_mac_get_seqn_sdsq(const void* rs) {
    uint8_t* s = (uint8_t*) rs;
    return (uint16_t) (s[4]<<8 | s[5]);
}

/*!
 * @brief Set the sequence number in a 6B hop seed
 * @param[in] rs: Pointer to 6B hop seed [4B device id][2B beacon sequence number]
 * @param[in] seqn: Sequenc number
 * @return void
*/
static inline void rnet_mac_set_seqn_sdsq(const void* rs, uint16_t seqn) {
    uint8_t* s = (uint8_t*) rs;
    s[4] = seqn >> 8;
    s[5] = seqn & 0xFF;
}

/*!
 * @brief Get the unique 4B seed portion of a 6B hop seed
 * @param[in] rs: Pointer to 6B hop seed [4B device id][2B beacon sequence number]
 * @return Seed portion of 6B seed corresponding to gateway device ID
*/
static inline uint32_t rnet_mac_get_seed_sdsq(const void* rs) {
    uint8_t* s = (uint8_t*) rs;
    return (uint32_t) (s[0]<<24 | s[1]<<16 | s[2]<<8 | s[3]);
}

#endif /* HALO_LIB_RINGNET_WAN_SRC_MAC_HALO_INCLUDE_RNET_MAC_CHANNEL_HOPPING_SDSQ_H_ */
