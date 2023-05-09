/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef SID_SECURITY_CRYPT_MASK_IFC_H
#define SID_SECURITY_CRYPT_MASK_IFC_H

#include <sid_error.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 8 bits (RSV|ENCR|SEC|TOPO|COUNTER|AUTH_TAG|)
//        (7-5|  4 | 3 |  2 |   1   |   0    |)
#define CRYPT_MASK(a_, b_, c_, d_, e_) (a_ | b_ | c_ | d_ | e_)
#define ENCR(x_) (x_ << 4)
#define SEC(x_) (x_ << 3)
#define TOPO(x_) (x_ << 2)
#define COUNTER(x_) (x_ << 1)
#define AUTH_TAG(x_) (x_ << 0)

#define WAN_TOPO (1)
#define PAN_TOPO (0)

/*
 * Naming schema
 * ENCR(0)/ENCR(1) ""/NW
 * SEC(0)/SEC(1) ""/APP
 * TOPO(WAN_TOPO)/TOPO(PAN_TOPO) WAN/PAN
 * COUNTER(0)/COUNTER(1) TIME/COUNTER
 * AUTH_TAG(0)/AUTH_TAG(1) ""/AUTH_TAG
 * eg  (not possible)
 * CRYPT_MASK(ENCR(1), SEC(1), TOPO(WAN_TOPO), COUNTER(0), AUTH_TAG(1)) XXX_NW_APP_WAN_TIME_AUTH
 */
// clang-format off
enum sid_security_crypt_mask {
    SID_SECURITY_CRYPT_MASK_PLAIN                 = CRYPT_MASK(ENCR(0), SEC(0), TOPO(0),        COUNTER(0), AUTH_TAG(0)), //0x00
    SID_SECURITY_CRYPT_MASK_WAN_TIME_AUTH         = CRYPT_MASK(ENCR(0), SEC(0), TOPO(WAN_TOPO), COUNTER(0), AUTH_TAG(1)), //0x05
    SID_SECURITY_CRYPT_MASK_NW_WAN_TIME_AUTH      = CRYPT_MASK(ENCR(1), SEC(0), TOPO(WAN_TOPO), COUNTER(0), AUTH_TAG(1)), //0x15
    SID_SECURITY_CRYPT_MASK_NW_APP_WAN_TIME_AUTH  = CRYPT_MASK(ENCR(1), SEC(1), TOPO(WAN_TOPO), COUNTER(0), AUTH_TAG(1)), //0x1d
    SID_SECURITY_CRYPT_MASK_WAN_COUNTER_AUTH      = CRYPT_MASK(ENCR(0), SEC(0), TOPO(WAN_TOPO), COUNTER(1), AUTH_TAG(1)), //0x07
    SID_SECURITY_CRYPT_MASK_NW_WAN_COUNTER_AUTH   = CRYPT_MASK(ENCR(1), SEC(0), TOPO(WAN_TOPO), COUNTER(1), AUTH_TAG(1)), //0x17
    SID_SECURITY_CRYPT_MASK_APP_WAN_COUNTER_AUTH  = CRYPT_MASK(ENCR(0), SEC(1), TOPO(WAN_TOPO), COUNTER(1), AUTH_TAG(1)), //0x0f
    SID_SECURITY_CRYPT_MASK_APP_WAN_TIME_AUTH     = CRYPT_MASK(ENCR(0), SEC(1), TOPO(WAN_TOPO), COUNTER(0), AUTH_TAG(1)), //0x0D
    SID_SECURITY_CRYPT_MASK_WAN_COUNTER           = CRYPT_MASK(ENCR(0), SEC(0), TOPO(WAN_TOPO), COUNTER(1), AUTH_TAG(0)), //0x06
    SID_SECURITY_CRYPT_MASK_APP_PAN               = CRYPT_MASK(ENCR(0), SEC(1), TOPO(PAN_TOPO), COUNTER(0), AUTH_TAG(0)), //0x08 DOES NOT FOLLOW CONVENTION LEGACY
    SID_SECURITY_CRYPT_MASK_NW_PAN_TIME_AUTH      = CRYPT_MASK(ENCR(1), SEC(0), TOPO(PAN_TOPO), COUNTER(0), AUTH_TAG(1)), //0x11
    SID_SECURITY_CRYPT_MASK_NOT_FOUND             = UINT8_MAX,
};
// clang-format on

_Static_assert(SID_SECURITY_CRYPT_MASK_PLAIN == 0x00, "Invalid mask value");
_Static_assert(SID_SECURITY_CRYPT_MASK_WAN_TIME_AUTH == 0x05, "Invalid mask value");
_Static_assert(SID_SECURITY_CRYPT_MASK_NW_WAN_TIME_AUTH == 0x15, "Invalid mask value");
_Static_assert(SID_SECURITY_CRYPT_MASK_NW_APP_WAN_TIME_AUTH == 0x1d, "Invalid mask value");
_Static_assert(SID_SECURITY_CRYPT_MASK_WAN_COUNTER_AUTH == 0x07, "Invalid mask value");
_Static_assert(SID_SECURITY_CRYPT_MASK_NW_WAN_COUNTER_AUTH == 0x17, "Invalid mask value");
_Static_assert(SID_SECURITY_CRYPT_MASK_APP_WAN_COUNTER_AUTH == 0x0f, "Invalid mask value");
_Static_assert(SID_SECURITY_CRYPT_MASK_APP_WAN_TIME_AUTH == 0x0D, "Invalid mask value");
_Static_assert(SID_SECURITY_CRYPT_MASK_WAN_COUNTER == 0x06, "Invalid mask value");
_Static_assert(SID_SECURITY_CRYPT_MASK_APP_PAN == 0x08, "Invalid mask value");
_Static_assert(SID_SECURITY_CRYPT_MASK_NW_PAN_TIME_AUTH == 0x11, "Invalid mask value");
_Static_assert(SID_SECURITY_CRYPT_MASK_NOT_FOUND == 255, "Invalid mask value");

typedef uint8_t sid_security_crypt_mask_t;

#define SID_SECURITY_CRYPT_MASK_MAX_AUTH_TAG_SIZE 16
#define SID_SECURITY_CRYPT_MASK_SCRATCH_BUF_SIZE 326

struct sid_security_crypt_mask_params {
    uint8_t auth_tag[SID_SECURITY_CRYPT_MASK_MAX_AUTH_TAG_SIZE];
    uint8_t auth_tag_size;
    union {
        uint8_t aad[SID_SECURITY_CRYPT_MASK_SCRATCH_BUF_SIZE];
        uint8_t scratch[SID_SECURITY_CRYPT_MASK_SCRATCH_BUF_SIZE];
    };
    uint8_t *in;
    uint8_t *out;
    size_t in_size;
    size_t out_size;
    union {
        size_t aad_size;
        size_t scratch_size;
    };
    uint32_t reference;
    uint32_t seqn;
    uint32_t app_seqn;
    uint16_t cmd_class;
    uint16_t cmd_id;
    uint32_t offset_size;
    const struct sid_address *const src_addr;
    const struct sid_address *const dest_addr;
};

enum sid_security_crypt_mask_flags {
    SID_SECURITY_CRYPT_MASK_FLAG_NEEDS_AAD = (1 << 0),
    SID_SECURITY_CRYPT_MASK_FLAG_NEEDS_AAD_PREFIX = (1 << 1),
    SID_SECURITY_CRYPT_MASK_FLAG_USE_APP_OFFSET = (1 << 2),
    SID_SECURITY_CRYPT_MASK_FLAG_SAVE_COUNTER = (1 << 3),
    SID_SECURITY_CRYPT_MASK_FLAG_CHECK_FOR_REPLAY = (1 << 4),
    SID_SECURITY_CRYPT_MASK_FLAG_SAVE_EXPLICIT_COUNTER = (1 << 5),
    SID_SECURITY_CRYPT_MASK_FLAG_USE_EXT_REFERENCE = (1 << 6),
    SID_SECURITY_CRYPT_MASK_FLAG_SAVE_COUNTER_ONLINE = (1 << 7),
    SID_SECURITY_CRYPT_MASK_LAST,
};

_Static_assert(SID_SECURITY_CRYPT_MASK_LAST < UINT8_MAX, "FLAG OVERFLOW increase flags size!");

struct sid_security_crypt_mask_handle {
    enum sid_security_crypt_mask mask;
    uint8_t flags;
    uint8_t first_key;
    uint8_t second_key;
    sid_error_t (*decrypt)(const struct sid_security_crypt_mask_handle *const handle,
                           struct sid_security_crypt_mask_params *params);
    sid_error_t (*encrypt)(const struct sid_security_crypt_mask_handle *const handle,
                           struct sid_security_crypt_mask_params *params);
};

static inline bool
sid_security_crypt_mask_is_use_aad_flag_set(const struct sid_security_crypt_mask_handle *const handle)
{
    return (handle->flags & SID_SECURITY_CRYPT_MASK_FLAG_NEEDS_AAD) != 0;
}

static inline bool
sid_security_crypt_mask_is_use_aad_prefix_flag_set(const struct sid_security_crypt_mask_handle *const handle)
{
    return (handle->flags & SID_SECURITY_CRYPT_MASK_FLAG_NEEDS_AAD_PREFIX) != 0;
}

static inline bool sid_security_crypt_mask_save_counter(const struct sid_security_crypt_mask_handle *const handle)
{
    return (handle->flags & SID_SECURITY_CRYPT_MASK_FLAG_SAVE_COUNTER) != 0;
}

static inline bool sid_security_crypt_mask_save_explicit_counter(const struct sid_security_crypt_mask_handle *const handle)
{
    return (handle->flags & SID_SECURITY_CRYPT_MASK_FLAG_SAVE_EXPLICIT_COUNTER) != 0;
}

static inline bool sid_security_crypt_mask_check_for_replay(const struct sid_security_crypt_mask_handle *const handle)
{
    return (handle->flags & SID_SECURITY_CRYPT_MASK_FLAG_CHECK_FOR_REPLAY) != 0;
}

static inline bool
sid_security_crypt_mask_is_use_app_offset_flag_set(const struct sid_security_crypt_mask_handle *const handle)
{
    return (handle->flags & SID_SECURITY_CRYPT_MASK_FLAG_USE_APP_OFFSET) != 0;
}

static inline bool sid_security_crypt_mask_is_use_ext_reference(const struct sid_security_crypt_mask_handle *const handle)
{
    return (handle->flags & SID_SECURITY_CRYPT_MASK_FLAG_USE_EXT_REFERENCE) != 0;
}

static inline bool sid_security_crypt_mask_save_counter_on_online(const struct sid_security_crypt_mask_handle *const handle,
                                                                  const bool is_online)
{
    return ((handle->flags & SID_SECURITY_CRYPT_MASK_FLAG_SAVE_COUNTER_ONLINE) != 0) && is_online;
}

static inline bool sid_security_crypt_mask_is_counter_set(const struct sid_security_crypt_mask_handle *const handle)
{
    return (handle->mask & COUNTER(1)) != 0;
}

static inline bool sid_security_crypt_mask_is_network_set(const struct sid_security_crypt_mask_handle *const handle)
{
    return (handle->mask & ENCR(1)) != 0;
}

static inline bool
sid_security_crypt_mask_is_use_auth_tag_set(const struct sid_security_crypt_mask_handle *const handle)
{
    return (handle->mask & AUTH_TAG(1)) != 0;
}

static inline bool sid_security_crypt_mask_is_equal_to(const struct sid_security_crypt_mask_handle *const handle,
                                                       enum sid_security_crypt_mask mask)
{
    return handle->mask == mask;
}

const struct sid_security_crypt_mask_handle *const
sid_security_get_crypt_mask_handle_from_crypt_mask(const enum sid_security_crypt_mask mask);

#ifdef __cplusplus
}
#endif

#endif /*! SID_SECURITY_CRYPT_MASK_IFC_H */
