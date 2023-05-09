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

#ifndef SID_FLEX_PARSER_H
#define SID_FLEX_PARSER_H

#include <sid_flex_types.h>
#include <sid_error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Function to deserialize FLEX header
 * @link https://wiki.labcollab.net/confluence/display/HALO/Halo+Flex+Format
 *
 * @param pointer to buffer
 * @param buffer size
 * @param pointer to result
 * @param pointer to number of consumed bytes from buffer
 *
 * @return result code
 */
sid_error_t sid_flex_deserialize(const uint8_t *const buffer,
                                 const size_t len,
                                 struct sid_flex *const flex,
                                 size_t *const out_size);

/**
 * Function serializes FLEX header into byte buffer and prepare it to send
 * @attention: all required fields must be provided:
 *                 source and destination addresses
 *                 sequence number
 * @param buffer - to store serialized data
 * @param len - capacity of given buffer
 * @param flex - flex header structure for serialization
 * @param pointer to number of consumed bytes from buffer
 *
 * @return result code
 */
sid_error_t sid_flex_serialize(uint8_t *const buffer,
                               const size_t len,
                               const struct sid_flex *const flex,
                               size_t *const out_size);

/**
 * Function to build aad buffer from the input flex structure
 * @attention: all required fields must be provided, the structure is fixed
 *
 * @param buffer - to store serialized data
 * @param len - capacity of given buffer
 * @param flex - flex header structure to generate aad
 * @param pointer to number of consumed bytes from the buffer
 *
 * @return result code
 */
sid_error_t sid_flex_aad_serialize(uint8_t *const buffer,
                                   const size_t len,
                                   const struct sid_flex *const flex,
                                   size_t *const out_size);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif /* SID_FLEX_PARSER_H */
