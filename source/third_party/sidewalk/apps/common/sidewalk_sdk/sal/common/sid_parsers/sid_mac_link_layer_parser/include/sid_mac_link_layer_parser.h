/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_MAC_LINK_LAYER_PARSER_H
#define SID_MAC_LINK_LAYER_PARSER_H

#include <sid_mac_link_layer_frame.h>

#include <sid_error.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Parses the link layer data from the given input buffer
 *
 * @param[in] input An input buffer containing link layer data - It is assumed the link layer data starts at the beginning of this buffer
 * @param[in] input_size The size of the given input buffer
 * @param[out] ll_data The parsed linked layer data
 * @param[out] ll_end The length of the link layer data section parsed from the given input buffer
 *
 * @returns #SID_ERROR_NONE on success
 */
sid_error_t sid_mac_parse_link_layer(const uint8_t* input, size_t input_size, struct sid_link_layer_data* ll_data, const uint8_t** ll_end);

/**
 * Parses the link layer frame type from the given value
 *
 * @param[in] data The first byte of a link layer frame
 *
 * @returns a valid frame type on success
 * @returns #SID_LINK_LAYER_FRAME_TYPE_UNKNOWN when the frame type is invalid or unknown
 */
enum sid_link_layer_frame_type sid_mac_get_frame_type(uint8_t data);

#ifdef __cplusplus
}
#endif

#endif // SID_MAC_LINK_LAYER_PARSER_H
