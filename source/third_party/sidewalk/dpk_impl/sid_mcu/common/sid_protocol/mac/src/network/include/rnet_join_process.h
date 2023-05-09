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

#ifndef RNET_JOIN_PROCESS_H
#define RNET_JOIN_PROCESS_H

#if !HALO_WITH_MODULE_HALO_LIB_HALO_MANAGEMENT
#include <sid_protocol_defs.h>
#include <sid_protocol_opts.h>
#include <sid_error.h>
#if !defined(GW_SUPPORT) || !GW_SUPPORT
#include <rnet_errors.h>
#include "rnet_dev_addr_def.h"
#endif
#include <stdint.h>

#if !defined(GW_SUPPORT) || !GW_SUPPORT

typedef enum {
    RNET_JOIN_STATUS_IDLE = 0,
    RNET_JOIN_STATUS_TRIGGER_DISC = 1,
    RNET_JOIN_STATUS_DISCOVERED_GW = 2,
    RNET_JOIN_STATUS_WAITING_FOR_RESP = 3,
    RNET_JOIN_STATUS_RESP_RCVD = 4,
    RNET_JOIN_STATUS_RETRY = 5,
    RNET_JOIN_STATUS_JOINED_PAN = 6,
    RNET_JOIN_STATUS_REJECTED_PAN = 7,
    RNET_JOIN_PROC_DISABLED = 8
}rnet_join_proc_status_t;

typedef struct {
    uint8_t join_retries;
    rnet_join_proc_status_t join_state;
    uint8_t gw_id[RNET_DEV_ID_SZ];
    bool response_rcvd;
    uint8_t rsp_rcvd;
}rnet_join_proc_ctx_t;

void rnet_join_proc_init(void);

rnet_join_proc_status_t rnet_get_join_proc_status(void);

void rnet_set_join_proc_status(rnet_join_proc_status_t join_state);

rnet_error_t rnet_process_join_resp(U8* join_resp, U8 len);

#else //GW

/**
 * @brief Consumes the payload of the CMD_ID_RNET_JOIN_PROC_REQ
 *
 * @param[in]   ptr     Pointer to the payload
 * @param       len     Length of the payload
 * @param[in]   devid   Pointer to devid
 */
sid_error_t rnet_consume_join_pan_req(uint8_t *ptr, uint8_t len, uint8_t *devid);

/**
 * @brief Send SLOT_EXTENTION message
 *
 * @param[in]   node_address    End node address
 */
sid_error_t sid_jp_send_slot_ext(uint8_t *node_address);

#endif //GW
#endif //!HALO_WITH_MODULE_HALO_LIB_HALO_MANAGEMENT

#endif
