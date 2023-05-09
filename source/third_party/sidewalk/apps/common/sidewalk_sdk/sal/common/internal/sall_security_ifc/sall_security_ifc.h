/*
 * Copyright 2020-2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef SALL_SECURITY_IFC_H
#define SALL_SECURITY_IFC_H

#include <halo/lib/hall/interface.h>

#include <sid_event_queue_ifc.h>

#include <sid_network_address.h>
#include <sid_error.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sall_security_user_cb {
    void *user_ctx;
    void (*on_new_sid_address_recieve)(void *user_ctx, const struct sid_address *const addr);
    void (*on_device_registration_error)(void *user_ctx);
    void (*on_authenticated_connection)(void *user_ctx, bool valid, const halo_hall_message_descriptor_t *const mdesc);
    void (*on_secure_connection)(void *user_ctx);
    bool (*on_key_refresh_config_required)(void *user_ctx, const halo_hall_message_descriptor_t *const mdesc);
    void (*on_key_refresh_config_set)(void *user_ctx, bool enabled);
};

struct sall_security_params {
    bool is_device_online;
    bool validate_cert_chain;
    bool is_smsn;
    bool block_unregistered_d2d;
    uint32_t timeout_seconds;
    struct sid_event_queue *event_queue;
    struct sid_address address;
    struct sall_security_user_cb user_cb;
    uint8_t max_registration_init_retries;
};

sid_error_t sall_security_init(const struct sall_security_params *params, const halo_hall_lib_ctx_t hall_lib_ctx);

void sall_security_deinit(void);

sid_error_t sall_security_device_registration_complete(bool is_success);

sid_error_t sall_security_device_registration_cancel(void);

bool sall_security_is_device_registration_in_progress(void);

sid_error_t sall_security_secure_session_cancel(void);

bool sall_security_is_secure_session_in_progress(void);

void sall_security_set_address(const struct sid_address *const address);

void sall_security_reset(const struct sid_address *const address);

sid_error_t sall_security_device_registration_start(bool is_key_refresh);

#ifdef __cplusplus
}
#endif

#endif /*! SALL_SECURITY_IFC_H */
