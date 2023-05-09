/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef _RNET_GCS_H_
#define _RNET_GCS_H_

#include <stdbool.h>
#include <time.h>
#include <sid_time_types.h>

#include <rnet_errors.h>
#include <sid_protocol_defs.h>
#include "rnet_dev_addr_def.h"

#define RNET_GET_TIME_DATA_SZ 12
#define RNET_NOTIFY_TIME_DATA_SZ 8

#define RNET_TIME_SYNC_MAX_DRIFT_TICK 0x7FFE000 //(0x3FFF << 13)
#define RNET_TIME_SYNC_MAX_DRIFT_SEC (RNET_TIME_SYNC_MAX_DRIFT_TICK >> 15)

#define RNET_TSYNC_MAX_TIMEOUT_SECS (16 * 60) //16 min
#define RNET_TIME_SYNC_TIMEOUT_SECS_SPI 15 //15 secs
#define RNET_TIME_SYNC_TIMEOUT_SECS_RF 30 //30 secs
#define RNET_TIME_SYNC_CLD_PERIOD_SECS (2 * 3600) // cloud sync period, 2 hours

#if (HALO_WITH_MODULE_TARGET_FETCH_DVTL) // TODO: HALO-11250: remove once we fix LDR link issue in Fetch DVTL
#define RNET_TIME_SYNC_MAX_RETRY 7
#else
#define RNET_TIME_SYNC_MAX_RETRY 3
#endif

#define RNET_TIME_SYNC_ERR_RECOVERY_TIME 5 //5secs recovery time
#define PROXY_RESP_TIME_DATA_SZ         25 //data size in the proxy get time resp command

typedef enum {
    RNET_TSYNC_STATE_PWR_ON_NTW_SYNC = 0,
    RNET_TSYNC_STATE_PWR_ON_PAN_SYNC = 1,
    RNET_TSYNC_STATE_NOTIFY_TIME = 2,
    RNET_TSYNC_STATE_DEFERRED_PAN_SYNC = 3,
    RNET_TSYNC_STATE_NETWORK_SYNC = 4,
    RNET_TSYNC_STATE_WAIT_RESP = 5,
    RNET_TSYNC_STATE_SUSPEND_PWR_ON_SYNC = 6,
    RNET_TSYNC_STATE_ERROR_RECOVERY = 7
} rnet_tsync_state_t;

typedef struct {
    uint8_t id[RNET_DEV_ID_SZ];
    uint8_t dest_fmt;
    sid_time_t drift_sec;
    struct sid_timespec tsp;
    uint32_t offset_msec;
} defer_sync_t;

typedef struct {
    defer_sync_t defer_sync;        // deferred time sync
    struct sid_timespec last_req_tsp;   // time when GET_TIME was sent
    sid_time_t recovery_sec;            // time when start recovery state
    sid_time_t last_cld_req_sec;        // time when GET_TIME was sent to cloud
    uint32_t rand_key;              // random key
    uint8_t retry_count;            // time sync retry count
    bool trig_notify;               // trigger notify time broadcasting
    bool trig_tsync;                // trigger time sync
} time_sync_status_t;

typedef struct {
    uint32_t time_out;              // time sync time out in second
    uint32_t cld_sync_interval;     // cloud sync interval in second
    uint32_t max_retry;             // maximum time sync retry
} time_sync_params_t;

/**
 * @brief  Function to get time sync readiness.
 *
 * @retval  true  Time sync is ready
 */
bool rnet_tsync_is_ready(void);

/**
 * @brief Funtion to get pointer to the time sync status.
 *
 * @retval  pointer to time_sync_status_t
 */
time_sync_status_t *get_tsync_status(void);

/**
 * @brief Function to set time sync parameters
 *
 * @param[in]  params   parameters for time sync
 */
void rnet_tsync_set_params(time_sync_params_t params);

/**
 * @brief Function to get pointer to the time sync parameters
 *
 * @retval  pointer    pointer to time sync parameter
 */
time_sync_params_t *rnet_tsync_get_params(void);

/**
 * @brief Function to reset netowrk time.
 *
 */
void rnet_tsync_reset_network_time(void);

/**
 * @brief Function to resume/trigger time sync.
 *
 * @param[in]  resync   true if it is re-sync request
 */
void rnet_tsync_trigger_sync(bool resync);

/**
 * @brief Function to get current state of time sync state-machine.
 *
 * @retval  state   Current state of time sync state-machine
 */
rnet_tsync_state_t rnet_tsync_get_sync_state(void);

/**
 * @brief Function to set state of time sync state-machine.
 *
 * @param[in]  state   new state to be set
 *
 */
void rnet_tsync_set_sync_state(rnet_tsync_state_t state);

/**
 * @brief Function to encode payload of time sync command.
 *
 * @param[out]  data       Pointer to tx data buffer
 * @param[in]   use_rkey   use random key
 * @param[in]   rand_key   random key (valid if use_rkey is true)
 *
 * @retval  RNET_SUCCESS    If the function completed successfully.
 *                          Otherwise, an error code is returned.
 */
rnet_error_t rnet_tsync_encode_cmd(uint8_t *data, bool use_rkey, uint32_t rand_key);

/**
 * @brief Function to decode payload of time sync command.
 *
 * @param[in]   data       Pointer to rx data buffer
 * @param[out]  tsp        Pointer to the received time info
 * @param[out]  drift      Pointer to the received drift info
 * @param[out]  rand_key   Pointer to the recevied random key
 *
 * @retval  RNET_SUCCESS    If the function completed successfully.
 *                          Otherwise, an error code is returned.
 */
rnet_error_t rnet_tsync_decode_cmd(uint8_t *data,
                                struct sid_timespec *tsp,
                                struct sid_timespec *drift,
                                uint32_t *rand_key);

/**
 * @brief Function to send time notification
 *
 * @retval  RNET_SUCCESS    If the function completed successfully.
 *                          Otherwise, an error code is returned.
 */
rnet_error_t rnet_tsync_notify_time(void);

/**
 * @brief Function to request time to the cloud or device.
 *
 * @param[in]   dest_id       Pointer to destination ID
 * @param[in]   dest_fmt      Destination format
 *
 * @retval  RNET_SUCCESS    If the function completed successfully.
 *                          Otherwise, an error code is returned.
 */
rnet_error_t rnet_tsync_request_time(uint8_t *dest_id, uint8_t dest_fmt);

/**
 * @brief  Time sync process state-machine. Should be called in thread loop.
 *
 */
void rnet_time_sync_process(void);

#endif //_RNET_GCS_H_
