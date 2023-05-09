/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef LIB_RING_NET_SRC_INCLUDE_RNET_HALL_H_
#define LIB_RING_NET_SRC_INCLUDE_RNET_HALL_H_

#include <stdint.h>
#include <stdbool.h>
#include <lk/list.h>
#include <rnet_errors.h>
#include <sid_protocol_defs.h>
#if HALO_WITH_MODULE_HALO_LIB_HALO_MANAGEMENT
#include <halo/lib/hall/appv1/mgm/types.h>
#endif
#include <sid_protocol_metrics.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RNET_HALL_STATE_HDR_ASYNC = 0x00,
    RNET_HALL_STATE_HDR_SYNC = 0x01,
    RNET_HALL_STATE_TIME_ERROR = 0x02,
    RNET_HALL_STATE_TIME_DRIFTED = 0x03,
    RNET_HALL_STATE_DEVICE_PARAM_CHANGE = 0x04,
    RNET_HALL_STATE_NW_SYNC_REQUEST = 0x05,
    RNET_HALL_STATE_NW_SYNCED = 0x06,
    RNET_HALL_STATE_NW_DISCONNECTED = 0x07,
} rnet_hall_state_t;

enum rnet_hall_ctrl_info_req {
    RNET_HALL_CTRL_INFO_REQ_SCH_LOOKUP_REQ,
    RNET_HALL_CTRL_INFO_REQ_SCH_ASSIGN_REQ,
    RNET_HALL_CTRL_INFO_REQ_SCH_EXT_REQ,
    RNET_HALL_CTRL_INFO_REQ_SCH_TERM_NTFY,
    RNET_HALL_CTRL_INFO_REQ_FSK_WAN_GW_JOIN_RESP,
    RNET_HALL_CTRL_INFO_REQ_SEND_NACK,
};

typedef struct {
    ret_code_t (*on_msg_handler)(void * arg, const uint8_t tr_idx);
    ret_code_t (*on_tx_prepare_handler)(void *arg, const uint8_t tr_idx, const uint32_t tx_duration_ms);
    ret_code_t (*on_ctrl_info_req_handler)(void *arg, enum rnet_hall_ctrl_info_req req_type, const void *data);
    void (*on_state_change)(void * arg, const rnet_hall_state_t state);
    void *arg;
    rnet_link_t on_msg_hw_link;
    list_node_t subscribe_node;
} rnet_hall_subscriber_t;

#if HALO_WITH_MODULE_HALO_LIB_HALO_MANAGEMENT

typedef struct rnet_mac_channel_survey_ctx {
   /* input paramters */
   struct{
        uint32_t start_freq;
        uint32_t channel_spacing;
    } freq_param;
    /* output paramters */
    int8_t rssi[HALO_HALL_APPV1_CHANNEL_SURVEY_MAX_CHANNELS];
    /* context for channel survey */
    void *ctx1;
    void *ctx2;
    void *ctx3;
} rnet_mac_channel_survey_ctx_t;

/* callback type for the channel survey API */
typedef void (*rnet_chsurvey_cb_t)(bool status, void *ch_survey_ctx);

/*
 * @brief function to get start the channel survey
 * @param[in] num_channels: the number of channels to scan
 * @param[in] chan_survey_ctx: start freq and channel spacing
 * @param[out] chan_survey_ctx: noise floor value for each channel in dBm
 * @param[in] chan_survey_cb: callback function after the channel survey is done
 * @returns true if the channel survey was started successfully, false otherwise
 */
bool rnet_get_chan_survey(uint8_t num_channels, rnet_mac_channel_survey_ctx_t *chan_survey_ctx, rnet_chsurvey_cb_t chan_survey_cb);

#endif //HALO_WITH_MODULE_HALO_LIB_HALO_MANAGEMENT

/**
 * Subscribe to messages on a HW Link
 */
rnet_error_t rnet2hall_subscribe(rnet_hall_subscriber_t * subscriber);
/**
 * Unsubscribe from events delivery for a message received
 */
void rnet2hall_unsubscribe(rnet_hall_subscriber_t * subscriber);

/**
 * Multiplexor entry point for a message delivery to subscribers
 */
rnet_error_t rnet2hall_on_message(const rnet_link_t received_hw_link, const uint8_t tr_idx);

/**
 * Notify subscribers about preparing of message transmission
 */
bool rnet2hall_on_tx_prepare(const uint8_t tr_idx, const uint32_t tx_duration_ms);

/**
 * Multiplexor entry point for info request to sall
 */
void rnet2hall_on_ctrl_info_req(enum rnet_hall_ctrl_info_req req_type, const void *data);

/**
 * Notify subscribers about changing state of network
 */
void rnet2hall_on_state_change(const rnet_hall_state_t state);

/**
 * Return true if hdr flag is enabled
 */
bool rnet_is_enabled_hdr(void);

/**
 * Return true if ldr flag is enabled
 */
bool rnet_is_enabled_ldr(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* LIB_RING_NET_SRC_INCLUDE_RNET_HALL_H_ */
