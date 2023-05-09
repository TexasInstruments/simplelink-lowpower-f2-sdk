/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef RNET_FSK_WAN_SCH_LOOKUP_REQ_SERIALIZER_H
#define RNET_FSK_WAN_SCH_LOOKUP_REQ_SERIALIZER_H

#include <sid_error.h>
#include <stdint.h>
/*!
 * @brief Creates a serialized fsk-wan sch-lookup request cmd payload.
 * @param[out] buffer: Serial buffer
 * @param len: Length available in serial buffer
 * @param[in] nodeid: Pointer to the array containing ID of the node to be queried for.
 * @param[out] out_size: The size of serial message written into the buffer.
 * returns An error code indicating SUCESS/FAILURE.
 */
sid_error_t serialize_sch_lookup_req_data(uint8_t *buffer, uint8_t len, uint8_t const *nodeid, uint8_t *out_size);

#endif /* RNET_FSK_WAN_SCH_LOOKUP_REQ_SERIALIZER_H */
