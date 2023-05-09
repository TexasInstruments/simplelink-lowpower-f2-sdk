/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef RNETC_PUBLIC_HALO_LINKL_TYPES_H_
#define RNETC_PUBLIC_HALO_LINKL_TYPES_H_

#include <sid_network_address.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HALO_LL_SEQUENCE_BYTE_MAX = (1 << 6) - 1,
    HALO_LL_SEQUENCE_2BYTE_MAX = (1 << 14) - 1,
    HALO_LL_SEQUENCE_3BYTE_MAX = (1 << 22) - 1,
    HALO_LL_SEQUENCE_UNDEFINED = 0xFFFFFFFF
} halo_flex_sequence_max;

typedef enum {
    HALO_LL_FTYPE_BEACON = 0x00, //!< RNET_LL_FTYPE_BEACON
    HALO_LL_FTYPE_DATA = 0x01, //!< RNET_LL_FTYPE_DATA
    HALO_LL_FTYPE_NETWORK = 0x02, //!< RNET_LL_FTYPE_NETWORK
    HALO_LL_FTYPE_ACK = 0x03, //!< RNET_LL_FTYPE_ACK

    _HALO_LL_FTYPE_MASK = 0x7, //!< _RNET_LL_FTYPE_MASK
    _HALO_LL_FTYPE_BIT_OFFSET = 5
} rnet_ll_frame_type;

//! Link layer frame structure
//! <a href="https://doorbot.atlassian.net/wiki/spaces/RL/pages/239272070/Link+Layer"> Link Layer Documentation </a>
typedef struct {
    uint8_t frame_type; //!< 0-beacon, 1-data frame, 2-network, 3-acknowledgment

    uint8_t proto_ver;
    uint8_t nw_frame_ctrl_inc; //!< specifies is network frame included 1 - yes; 0 - no
    uint8_t ack_req; //!< flag to specify is acknowledgment required (!=0) or not (==0)
    uint8_t suppress_broadcast_ack; //!< in case of group command expecting only one acknowledgment (other devices must suppress)

    //! reserved for future use - always 0
    uint8_t rsvd1;
    uint8_t rsvd2;

    struct sid_address src; //!< source device (only device is acceptable)
    struct sid_address dst; //!< destination address (device, group, cloud)

    uint32_t seqn; //!< message sequence number (defined by sender, must be included in reply)
    uint8_t seqn_sz; //!<  unique per device+command_class+command_id

    const void* payload;
    size_t payload_size;

} rnet_ll_frame_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_PUBLIC_HALO_LINKL_TYPES_H_ */
