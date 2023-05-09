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

#ifndef SID_SECURITY_BEAMS_IFC_H
#define SID_SECURITY_BEAMS_IFC_H

#include <sid_error.h>
#include <sid_network_address.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define SEC_GCM_KEY_SIZE 16
#define SEC_BEAMS_NONCE_SIZE 8

#ifdef __cplusplus
extern "C" {
#endif

enum sid_sec_beams_mode {
    SID_SEC_BEAMS_ENCRYPT = 1,
    SID_SEC_BEAMS_DECRYPT = 2,
};

enum sid_sec_beams_context_type {
    SID_SEC_BEAMS_CONTEXT_BLE   = 0,
    SID_SEC_BEAMS_CONTEXT_CLOUD = 1,
    SID_SEC_BEAMS_CONTEXT_COUNT,
};

struct sid_security_beams_context_info {
    uint8_t gcm_key[SEC_GCM_KEY_SIZE];
    uint8_t peer_nonce[SEC_BEAMS_NONCE_SIZE];
};

struct sid_sec_beams_enc_params {
    const struct sid_address *const addr;
    enum sid_sec_beams_mode mode;
    enum sid_sec_beams_context_type type;
    const uint8_t *const in;
    const size_t in_size;
    uint8_t *const out;
    size_t out_size;
};

sid_error_t sid_security_beams_load_context(enum sid_sec_beams_context_type type,
                                            bool *is_valid);

sid_error_t sid_security_beams_save_context(enum sid_sec_beams_context_type type,
                                            const struct sid_security_beams_context_info *const info,
                                            bool flush);

sid_error_t sid_security_beams_clear_context(enum sid_sec_beams_context_type type);

sid_error_t sid_sec_beams_aead_crypt(struct sid_sec_beams_enc_params *const params);

#ifdef __cplusplus
}
#endif

#endif /*! SID_SECURITY_BEAMS_IFC_H */
