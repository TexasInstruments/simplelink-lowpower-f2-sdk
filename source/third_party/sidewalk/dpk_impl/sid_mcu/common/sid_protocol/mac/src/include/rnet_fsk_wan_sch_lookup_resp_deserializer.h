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

#ifndef RNET_FSK_WAN_SCH_LOOKUP_RESP_DESERIALIZER_H
#define RNET_FSK_WAN_SCH_LOOKUP_RESP_DESERIALIZER_H

#include <rnet_fsk_wan_sch_lookup_tags.h>
#include <sid_error.h>

#include <stdint.h>
/*!
 * @brief Deserializes cmd payload containing fsk-wan sch-lookup response.
 * @param[in] buffer: Serial buffer.
 * @param len: Serial buffer length.
 * @param[out] data: Pointer to the struct for carrying deserialized information.
 * @param[out] out_size: Output variable for writing the size of serial buffer read.
 *                       Will be skipped if null.
 * returns An error code indicating SUCESS/FAILURE.
 */
sid_error_t deserialize_sch_lookup_resp_data(uint8_t *buffer, size_t len, struct slot_sch_resp_data *data, size_t *out_size);

#endif /* RNET_FSK_WAN_SCH_LOOKUP_RESP_DESERIALIZER_H */
