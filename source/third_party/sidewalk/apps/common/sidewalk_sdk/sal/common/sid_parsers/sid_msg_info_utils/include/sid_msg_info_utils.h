/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_MSG_INFO_UTILS_H_
#define SID_MSG_INFO_UTILS_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    SID_MSG_CRID_VERSION_0 = 0x0,
    SID_MSG_CRID_VERSION_1 = 0x1,
};

#define SID_MSG_CRID_SIZE_VERSION_0 9

#define SID_MSG_CRID_VERSION SID_MSG_CRID_VERSION_0
#define SID_MSG_CRID_SIZE SID_MSG_CRID_SIZE_VERSION_0
#define SID_MSG_CRID_AUTH_TAG_SIZE 4
#define SID_MSG_CRID_MAX_SEQN_SIZE 3

struct sid_msg_crid_params_s {
    uint32_t seqn;             // 3bytes link layer sequence number
    uint8_t msg_type;          // 0:Data 1:NACK 2:ACK
    const uint8_t *auth_tag;   // CMAC tag or GCM tag
    size_t auth_tag_size;      // Size of auth tag
};

/**
 * Print message correlation id as string
 *
 * @param prefix - prefix string of correlation id log
 * @param crid - buffer for the correlation id
 * @param crid_size - size of crid buffer
 *
 */
void sid_print_msg_correlation_id(char *prefix, uint8_t *crid, size_t crid_size);

/**
 * Generate message correlation id
 *
 * @param crid_params - Input paramters for the correlation id
 * @param crid - buffer for the correlation id
 * @param buf - size of correlation id buffer
 *
 */
void sid_generate_msg_correlation_id(struct sid_msg_crid_params_s crid_params, uint8_t *crid, size_t crid_size);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif /* SID_MSG_INFO_UTILS_H_ */
