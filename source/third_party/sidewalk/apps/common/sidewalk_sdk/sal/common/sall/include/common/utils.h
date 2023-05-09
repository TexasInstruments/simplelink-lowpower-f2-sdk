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

#ifndef RNETC_TESTS_UTILS_H_
#define RNETC_TESTS_UTILS_H_

#include <unistd.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int hexstr2bin(const char* hexstr, const size_t hexstr_len, uint8_t* buffer, const size_t max_buffer_len);
int bin2hexstr(const uint8_t* buffer, const size_t buffer_len, char* hexstr, const size_t max_hexstr_len);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_TESTS_UTILS_H_ */
