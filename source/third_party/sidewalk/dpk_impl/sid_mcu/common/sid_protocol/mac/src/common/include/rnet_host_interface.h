/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef RNET_HOST_INTERFACE_H
#define RNET_HOST_INTERFACE_H

#include <sid_protocol_opts.h>
#if RNET_SRL_HOST_INTERFACE

#include <sid_clock_ifc.h>
#include <sid_protocol_defs.h>
#if USE_SERIAL_CLIENT
#include <sid_pal_serial_client_ifc.h>
#endif
#include <rnet_app_layer.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SRL_HOST_MAX_FRAME_SZ
#define SRL_HOST_MAX_FRAME_SZ 512
#endif

#define SRL_FRAME_TYPE_TEST_MODE 0x40
#define SRL_FRAME_TYPE_NORMAL_MODE 0x80

#define SRL_FRAME_TEST_MODE_VERSION 0x1
#define SRL_FRAME_NORMAL_MODE_VERSION 0x1

#define SRL_FLAGS_WAIT_FOR_ACK 0x01
#define SRL_FLAGS_CRC_CHECK    0x02
#define SRL_FLAGS_SEQN_CHECK   0x04

typedef struct {
    volatile U8 frame_to_srl_vld;
    volatile U16 frame_to_srl_len;
    U8 frame_to_srl[RNET_MAX_FRAME_SZ];

    U8 frame_from_srl_vld:1;
    U16 frame_from_srl_len;
    U8 frame_from_srl[RNET_MAX_FRAME_SZ];
    rnet_cmd_t cmd_from_srl;

    uint8_t srl_flags;

    //deprecated
    U8 srl_wait_for_acks_ena:1;
    U8 srl_crc_check_ena:1;
    U8 srl_seqn_check_ena:1;

    volatile U8 requesting_xfer;

    U8 srl_b2b_xfer_dly_ms;
    U16 resend_tmout_ms;

    // 0-100%
    U8 inj_err_prob;
    U8 inj_dropped_prob;
    U8 dbg_back_pressure;

    struct sid_timespec last_xfer_done_ts;
    struct sid_timespec last_int_assert_ts;

    U8 tm_on;

} srl_host_st_t;

srl_host_st_t *rnet_srl_host_get(void);

S8 rnet_srl_host_send(U8* buf, U16 len);

void srl_host_interface_process(void);

#if USE_SERIAL_CLIENT
void rnet_srl_client_init(sid_pal_serial_client_factory_t *p_serial_client);
#endif

void rnet_srl_host_init(void);

void rnet_srl_host_exit_tm(void);

// Getter for host_st str
void rnet_host_st_get(srl_host_st_t * host_st_str);

// Setter for host_st str
void rnet_host_st_set(srl_host_st_t host_st_str);

#ifdef __cplusplus
}
#endif

#endif // RNET_SRL_HOST_INTERFACE
#endif
