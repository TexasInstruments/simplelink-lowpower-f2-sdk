/*
* Copyright 2021-2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef SEC_ALLOW_BLOCK_LIST_H
#define SEC_ALLOW_BLOCK_LIST_H

#include <sid_security_crypt_mask_ifc.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void sec_allow_block_list_goto_next_command(void);
enum sid_security_crypt_mask sec_allow_block_list_get_encrypt_mask(uint16_t cls, uint16_t id, uint8_t type, bool force_security, bool mobile_as_gateway);
bool sec_allow_block_list_is_expected_decrypt_mask(uint16_t cls, uint16_t id, uint8_t type, enum sid_security_crypt_mask mask, bool force_security, bool mobile_as_gateway);

void sec_allow_block_list_init(void);

#ifdef __cplusplus
}
#endif

#endif /*! SEC_ALLOW_BLOCK_LIST_H */
