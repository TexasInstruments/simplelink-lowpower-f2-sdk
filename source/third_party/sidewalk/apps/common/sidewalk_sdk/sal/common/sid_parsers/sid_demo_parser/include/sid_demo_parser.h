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

#ifndef SID_DEMO_PARSER_H
#define SID_DEMO_PARSER_H

#include <sid_demo_types.h>

#include <sid_error.h>

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * serializes the sid demo message descriptor and msg structure fields
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[in] pointer to the sid demo app message descriptor structure.
 * @param[in] pointer to the sid demo app message structure.
 */
void sid_demo_app_msg_serialize(struct sid_parse_state *const state,
                                struct sid_demo_msg_desc *msg_desc, struct sid_demo_msg *msg);

/**
 * serializes the demo app capability structure fields to a buffer
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[in] pointer to the sid demo app capability structure.
 */
void sid_demo_app_capability_discovery_notification_serialize(struct sid_parse_state *const state,
                                                              struct sid_demo_capability_discovery *cap);

/**
 * serializes the led action req structure fields to a buffer
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[in] pointer to the sid demo app led action request structure.
 */
void sid_demo_app_action_req_serialize(struct sid_parse_state *const state,
                                       struct sid_demo_led_action_req *led_action_req);

/**
 * serializes the sid demo action resp structure fields to a buffer
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[in] pointer to the sid demo app action response structure.
 */
void sid_demo_app_action_resp_serialize(struct sid_parse_state *const state,
                                        struct sid_demo_action_resp *action_resp);

/**
 * serializes the sid demo action notification structure fields to a buffer
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[in] pointer to the sid demo app action notification structure.
 */
void sid_demo_app_action_notification_serialize(struct sid_parse_state *const state,
                                                struct sid_demo_action_notification *action_notify);

/**
 * De-serializes the buffer to sid demo message descriptor and sid demo message structure fields
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[out] pointer to the sid demo message descriptor structure.
 * @param[out] pointer to the sid demo message structure.
 */
void sid_demo_app_msg_deserialize(struct sid_parse_state *const state,
                                  struct sid_demo_msg_desc *msg_desc, struct sid_demo_msg *msg);

/**
 * De-serializes the buffer to sid demo app capability discovery structure fields
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[out] pointer to the sid demo app action notification structure.
 */
void sid_demo_app_capability_discovery_notification_deserialize(struct sid_parse_state *const state,
                                                                struct sid_demo_capability_discovery *cap);

/**
 * De-serializes the buffer to sid demo led action request structure fields
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[out] pointer to the sid demo app action notification structure.
 */
void sid_demo_app_action_req_deserialize(struct sid_parse_state *const state,
                                         struct sid_demo_led_action_req *led_action_req);

/**
 * De-serializes the buffer to sid demo led action response structure fields
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[out] pointer to the sid demo app action notification structure.
 */
void sid_demo_app_action_resp_deserialize(struct sid_parse_state *const state,
                                          struct sid_demo_action_resp *action_resp);

/**
 * De-serializes the buffer to sid demo led action notification structure fields
 *
 * @param[in,out] state A pointer to the sid_parse_state
 * @param[out] pointer to the sid demo app action notification structure.
 */
void sid_demo_app_action_notification_deserialize(struct sid_parse_state *const state,
                                                  struct sid_demo_action_notification *action_notify);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* SID_DEMO_PARSER_H */
