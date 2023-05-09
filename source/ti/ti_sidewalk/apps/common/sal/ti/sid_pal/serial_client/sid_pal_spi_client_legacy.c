/******************************************************************************

 @file  sid_pal_spi_client_legacy.c

 @brief Describe the purpose and contents of the file.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************

 Copyright (c) 2023, Texas Instruments Incorporated

 All rights reserved not granted herein.
 Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free,
 non-exclusive license under copyrights and patents it now or hereafter
 owns or controls to make, have made, use, import, offer to sell and sell
 ("Utilize") this software subject to the terms herein. With respect to the
 foregoing patent license, such license is granted solely to the extent that
 any such patent is necessary to Utilize the software alone. The patent
 license shall not apply to any combinations which include this software,
 other than combinations with devices manufactured by or for TI ("TI
 Devices"). No hardware patent is licensed hereunder.

 Redistributions must preserve existing copyright notices and reproduce
 this license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution.

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

   * No reverse engineering, decompilation, or disassembly of this software
     is permitted with respect to any software provided in binary form.
   * Any redistribution and use are licensed by TI for use only with TI Devices.
   * Nothing shall obligate TI to provide you with source code for the software
     licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution
 of the source code are permitted provided that the following conditions are
 met:

   * Any redistribution and use of the source code, including any resulting
     derivative works, are licensed by TI for use only with TI Devices.
   * Any redistribution and use of any object code compiled from the source
     code and any resulting derivative works, are licensed by TI for use
     only with TI Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this
 software without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************


 *****************************************************************************/

/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <ti_drivers_config.h>

#include <sid_error.h>
#include "sid_pal_serial_client_ifc.h"
#include "include/sid_pal_spi_client_legacy.h"


#if RNET_SPI_MASTER_DEBUG_BUILD
#error RNET_SPI_MASTER_DEBUG_BUILD was not verified after refactoring and might be broken
#include "rnet_spim_debug.h"
#endif

#include <sid_pal_assert_ifc.h>
#include <sid_pal_log_ifc.h>
#include <sid_time_ops.h>
#include <sid_pal_timer_ifc.h>
#include <sid_pal_uptime_ifc.h>

#include <lk/list.h>

#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
#include <sid_pal_critical_region_ifc.h>

#include <string.h>
#include <stdlib.h>

#define SPIS_SEQNUM_BITS 2
#define SPIS_SEQNUM_MAX ((1 << SPIS_SEQNUM_BITS) - 1)
#if SPIS_SUPPORT_XFER_WINDOW
#define SPIS_XFER_WINDOW_SIZE ((SPIS_PAYLOAD_IN_XFER) * SPIS_SEQNUM_MAX)  // Maximum number of bytes sent without ACK
#else
#define SPIS_XFER_WINDOW_SIZE ((SPIS_PAYLOAD_IN_XFER) * 1)  // Maximum number of bytes sent without ACK
#endif

#define SPIS_OVERHEAD_SIZE (SPIS_XFER_SZ - SPIS_PAYLOAD_IN_XFER)

#define SPIS_NACK_FLAG_CRC (1 << 7)
#define SPIS_NACK_FLAG_OVERFLOW (1 << 6)
#define SPIS_NACK_FLAG_DUPLICATE (1 << 5)
#define SPIS_NACK_FLAG_OTHER (1 << 4)
#define SPIS_NACK_FLAG_BACKPRESSURE (1 << 0)

#define SPIS_RETRANSMIT_PERIOD_SEC  (1)

#ifndef SPIS_ENABLE_HOST_LOG_MODULE
#define SPIS_ENABLE_HOST_LOG_MODULE 0
#endif

#ifndef SPIS_ENABLE_HOST_LOG
#define SPIS_ENABLE_HOST_LOG        0
#endif

#define SPIS_ENABLE_HOST_DEBUG      0

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef RING_APP_MAJOR_VERSION
#define RING_APP_MAJOR_VERSION 99
#endif
#ifndef RING_APP_MINOR_VERSION
#define RING_APP_MINOR_VERSION 0
#endif
#ifndef RING_APP_PATCH_VERSION
#define RING_APP_PATCH_VERSION 0
#endif
#ifndef RING_APP_BUILD_VERSION
#define RING_APP_BUILD_VERSION 0
#endif

#define SPIS_CLK_RATE 4000000
#define SPI_QUEUE_BUSY_TIMEOUT_SEC 3

struct spis_event_t {
    list_node_t node;
    void (*handler) (struct spis_event_t *event);
    void *arg;
    uint32_t data;
};

struct spis_event_queue_t {
    list_node_t queue;
};

struct spis_frame_state_t {
    uint16_t frame_size; // Full size of a frame
    uint16_t frame_sent; // Size of a frame sent
    uint16_t frame_acked; // Size of a frame acknowledged
    uint8_t frame_buffer[SRL_HOST_MAX_FRAME_SZ];
};

struct spis_pipe_state_t {
    uint8_t *current_buffer;
    uint8_t *next_buffer;
    uint8_t double_buffer[SPIS_XFER_SZ * 2];
    struct spis_frame_state_t frame;
    uint8_t seq_number_current;
    uint8_t seq_number_confirmed;
    uint8_t next_buffer_idx; // Index of the buffer portion in the double_buffer
    uint8_t xfer_completed; // Set to 1 when the transfer is completed, 0 otherwise
    uint8_t nack_state; // If 0, NACK should not be sent. Otherwise flags for NACK
    uint8_t status_state; // If 0, STATUS should not be sent. Otherwise flags are already set for the STATUS packet
};

struct spis_events_t {
    struct spis_event_t prepare_tx_dataframe;
    struct spis_event_t prepare_tx_retransmit;
    struct spis_event_t prepare_tx_ack;
    struct spis_event_t prepare_tx_nack;
    struct spis_event_t prepare_tx_status;
};

enum spis_logs_buffer_state_t {
    SPIS_LOGS_BUFFER_STATE_READY,
    SPIS_LOGS_BUFFER_STATE_READ,
    SPIS_LOGS_BUFFER_STATE_STAGED,
    SPIS_LOGS_BUFFER_STATE_SENT,
};

struct spis_logs_buffer_t {
    uint8_t buffer[SPIS_XFER_SZ];
    uint8_t used;
    enum spis_logs_buffer_state_t state;
};

struct spis_ctx_t {
    sid_pal_serial_ifc_t ifc;
    const sid_pal_serial_callbacks_t *callbacks;
    void *cb_ctx;
    serial_host_st_t *host_st;
    struct spis_pipe_state_t tx_pipe;
    struct spis_pipe_state_t rx_pipe;
    uint8_t tx_default_reply[SPIS_XFER_SZ];
    struct spis_events_t events;
    struct spis_event_queue_t event_queue;
    sid_pal_timer_t timer_timeout;
    sid_pal_timer_t timer_int2host_assert;
    struct spis_logs_buffer_t logs_buffer;
    uint8_t xfer_count_pending;
    uint8_t xfer_count_commited;
    // The size of the packet (data frame) on the bus, must be not greater than SPIS_XFER_SZ
    uint8_t packet_size;
    SPI_Handle spis;
    SPI_Transaction spi_transaction;
    struct sid_timespec queue_next_reset;
};

static struct spis_ctx_t spis_ctx = {};

static uint16_t crc16_compute(uint8_t const *p_data, uint32_t size, uint16_t const *p_crc)
{
    uint16_t crc = (p_crc == NULL) ? 0xFFFF : *p_crc;

    for (size_t i = 0; i < size; i++) {
        crc = (uint8_t)(crc >> 8) | (crc << 8);
        crc ^= p_data[i];
        crc ^= (uint8_t)(crc & 0xFF) >> 4;
        crc ^= (crc << 8) << 4;
        crc ^= ((crc & 0xFF) << 4) << 1;
    }

    return crc;
}

static inline void set_next_queue_reset()
{
    struct spis_ctx_t *ctx = &spis_ctx;
    struct sid_timespec timeout = {
        .tv_sec = SPI_QUEUE_BUSY_TIMEOUT_SEC,
        .tv_nsec = 0,
    };

    sid_pal_uptime_now(&ctx->queue_next_reset);
    sid_time_add(&ctx->queue_next_reset, &timeout);
}

static void spis_enqueue(struct spis_ctx_t *ctx, struct spis_event_t *event)
{
    sid_pal_enter_critical_region();
    if (!list_in_list(&event->node)) {
        list_add_tail(&ctx->event_queue.queue, &event->node);
    }
    sid_pal_exit_critical_region();
}

static void spis_dequeue(struct spis_ctx_t *ctx, struct spis_event_t **event)
{
    sid_pal_enter_critical_region();
    *event = list_remove_head_type(&ctx->event_queue.queue, struct spis_event_t, node);
    sid_pal_exit_critical_region();
}

static void spis_queue_cancel(struct spis_ctx_t *ctx, struct spis_event_t *event)
{
    sid_pal_enter_critical_region();
    if (list_in_list(&event->node)) {
        list_delete(&event->node);
    }
    sid_pal_exit_critical_region();
}

// Debug code to inject errors and dropped transfers
static void inject_errs(uint8_t *buf)
{
    struct sid_timespec cur_time;
    sid_pal_uptime_now(&cur_time);
    unsigned int seed = sid_timespec_to_ms(&cur_time);
    int rand_num = rand_r(&seed) % 100;

    struct spis_ctx_t *ctx = &spis_ctx;
    if (rand_num < ctx->host_st->inj_err_prob) {
        SID_PAL_LOG_WARNING("err injected");
        buf[4] = cur_time.tv_nsec & 0xFF;
    }
}

static uint8_t inject_dropped(void)
{
    struct sid_timespec cur_time;
    sid_pal_uptime_now(&cur_time);
    unsigned int seed = sid_timespec_to_ms(&cur_time);
    int rand_num = rand_r(&seed) % 100;

    struct spis_ctx_t *ctx = &spis_ctx;
    if (rand_num < ctx->host_st->inj_dropped_prob) {
        SID_PAL_LOG_WARNING("drop injected");
        return 1;
    }

    return 0;
}

static void spis_timer_handler_int2host(struct spis_ctx_t *ctx)
{
    SID_PAL_LOG_DEBUG("int2host -------------------------");
    GPIO_write(RNET_HOST_SPI_INT2HOST, 0);
    sid_pal_uptime_now(&ctx->host_st->last_int_assert_ts);
}

static void spis_timer_handler_int2host_wrapper(void *arg, sid_pal_timer_t *timer)
{
    struct spis_ctx_t *ctx = arg;
    spis_timer_handler_int2host(ctx);
}

static void spis_int2host_assert(struct spis_ctx_t *ctx)
{
    struct sid_timespec next_assert_time = {
        .tv_sec = 0,
        .tv_nsec = ctx->host_st->srl_b2b_xfer_dly_ms * SID_TIME_NSEC_PER_MSEC,
    };

    struct sid_timespec * last_timestamp = &ctx->host_st->last_int_assert_ts;

    if (sid_time_gt(&ctx->host_st->last_xfer_done_ts, last_timestamp)) {
        last_timestamp = &ctx->host_st->last_xfer_done_ts;
    }

    sid_time_add(&next_assert_time, last_timestamp);

    struct sid_timespec now;
    sid_pal_uptime_now(&now);

    if (!sid_pal_timer_is_armed(&ctx->timer_int2host_assert)) {
        if (sid_time_gt(&now, &next_assert_time)) {
            spis_timer_handler_int2host(ctx);
        } else {
            sid_pal_timer_arm(&ctx->timer_int2host_assert, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &next_assert_time, NULL);
        }
    }
}

static void spis_int2host_deassert(struct spis_ctx_t *ctx)
{
    sid_pal_timer_cancel(&ctx->timer_int2host_assert);
    GPIO_write(RNET_HOST_SPI_INT2HOST, 1);
}

static void spis_handler_timeout(void *arg, sid_pal_timer_t *timer)
{
    struct spis_ctx_t *ctx = arg;
    // This is to ensure the TX event will not happen before retransmit
    spis_queue_cancel(ctx, &ctx->events.prepare_tx_dataframe);
    spis_enqueue(ctx, &ctx->events.prepare_tx_retransmit);
}

#if defined(SPIS_ENABLE_HOST_DEBUG) && SPIS_ENABLE_HOST_DEBUG
static uint8_t spis_dump_buffer[128];

static void spis_dump(const uint8_t *buff, size_t size)
{
    static const size_t chunk_size = 32;
    for (size_t offset = 0; offset < size; offset += chunk_size) {
        memset(spis_dump_buffer, 0, sizeof(spis_dump_buffer));
        uint8_t *dst = spis_dump_buffer;

        for (size_t i = 0; i < chunk_size && i + offset < size; ++i) {
            uint8_t character = buff[offset + i];
            uint8_t high = (character >> 4);
            uint8_t low  = (character & 0xf);
            high += 0x30 + (high > 9 ? 7 : 0);
            low  += 0x30 + (low > 9 ? 7 : 0);

            *dst++ = high;
            *dst++ = low;
            *dst++ = 0x20;
        }

        *dst = 0;
        SID_PAL_LOG_WARNING("%p: %s", buff + offset, spis_dump_buffer);
    }
}
#else
static inline void spis_dump(const uint8_t *buff, size_t size)
{
    (void)buff;
    (void)size;
}
#endif

/**
 * Prepare buffer for a default response
 *
 * @note Default response starts with 0xC0 and includes
 * information about current state and firmware version to the host
 */
static void make_null_xfer(struct spis_ctx_t *ctx, uint8_t *buf)
{
    SID_PAL_LOG_DEBUG("prep, NULL");

    memset(buf, SPIS_DEFAULT_CHAR, ctx->packet_size);
    buf[0] = 0xC0;

    serial_host_st_t *l_host_st = ctx->host_st;
    if (l_host_st->tm_on) {
        buf[1] = SRL_FRAME_TYPE_TEST_MODE | SRL_FRAME_TEST_MODE_VERSION; // Normal mode / test mode & serial interface version
    } else {
        buf[1] = SRL_FRAME_TYPE_NORMAL_MODE | SRL_FRAME_NORMAL_MODE_VERSION;
    }

    buf[2] = (ctx->rx_pipe.seq_number_confirmed << 4) | ctx->tx_pipe.seq_number_confirmed;
    buf[3] = RING_APP_MAJOR_VERSION;
    buf[4] = RING_APP_MINOR_VERSION;
    buf[5] = RING_APP_PATCH_VERSION;
    buf[6] = RING_APP_BUILD_VERSION;
}

static void spis_helpers_tx_commit(struct spis_ctx_t *ctx, uint8_t seq_number)
{
    /**
     * NB: Since the ACK seq_number might be for one of the packets on fly,
     * we need to mark ACKed all the packet on fly with sequence
     * number between previously ACKed and @p seq_number
     */
    uint16_t frame_acked    = ctx->tx_pipe.frame.frame_acked;
    uint16_t frame_sent     = ctx->tx_pipe.frame.frame_sent;
    uint8_t seq_num_start   = ctx->tx_pipe.seq_number_confirmed;

    if (seq_number < seq_num_start) {
        seq_number += (SPIS_SEQNUM_MAX + 1);
    }

    uint16_t bytes_to_ack   = (seq_number - seq_num_start + 1) * (ctx->packet_size - SPIS_OVERHEAD_SIZE);
    if (bytes_to_ack > frame_sent - frame_acked) {
        bytes_to_ack = frame_sent - frame_acked;
    }
    ctx->tx_pipe.frame.frame_acked += bytes_to_ack;

    if (ctx->tx_pipe.frame.frame_acked == ctx->tx_pipe.frame.frame_size) {
        ctx->tx_pipe.xfer_completed     = 1;
    }

    ctx->tx_pipe.seq_number_confirmed = seq_number % (SPIS_SEQNUM_MAX + 1);
}

static void spis_setup_retransmit_timer(struct spis_ctx_t *ctx)
{
    struct sid_timespec now;

    sid_pal_timer_cancel(&ctx->timer_timeout);
    spis_queue_cancel(ctx, &ctx->events.prepare_tx_retransmit);

    sid_pal_uptime_now(&now);
    now.tv_sec += SPIS_RETRANSMIT_PERIOD_SEC;

    sid_pal_timer_arm(&ctx->timer_timeout, SID_PAL_TIMER_PRIO_CLASS_LOWPOWER, &now, NULL);
}

static void event_handler_prepate_dataframe_tx(struct spis_event_t *event)
{
    struct spis_ctx_t *ctx = event->arg;

    if (ctx->tx_pipe.next_buffer) {
        SID_PAL_LOG_ERROR("prepare_tx(): next_buffer != NULL");
        return;
    }

    if (ctx->tx_pipe.frame.frame_sent - ctx->tx_pipe.frame.frame_acked >= ctx->packet_size - SPIS_OVERHEAD_SIZE) {
        SID_PAL_LOG_DEBUG("prepare_tx(): maximum packets on fly reached");
        return;
    }

    if (ctx->tx_pipe.frame.frame_sent == ctx->tx_pipe.frame.frame_size) {
        // We already sent all the data frames and awaiting for ACK
        return;
    }

    ctx->tx_pipe.next_buffer = &ctx->tx_pipe.double_buffer[ctx->tx_pipe.next_buffer_idx * SPIS_XFER_SZ];
    ctx->tx_pipe.next_buffer_idx ^= 1;

    uint8_t *dst = ctx->tx_pipe.next_buffer;
    memset(dst, SPIS_DEFAULT_CHAR, ctx->packet_size);

    uint16_t frame_size = ctx->tx_pipe.frame.frame_size;
    uint16_t frame_sent = ctx->tx_pipe.frame.frame_sent;
    uint8_t seq_number  = (ctx->tx_pipe.seq_number_current < SPIS_SEQNUM_MAX) ? (ctx->tx_pipe.seq_number_current + 1) : 0;

    SID_PAL_LOG_DEBUG("TX(df) seq_number: %u -> %u", ctx->tx_pipe.seq_number_current, seq_number);
    ctx->tx_pipe.seq_number_current = seq_number;

    dst[0] = ((uint8_t)SPIS_FT_DF << 4) | (seq_number << 2); // Add frame type, and current data frame tx seqn
    dst[1] = 0; // reserved byte

    uint16_t payload_size = MIN(frame_size - frame_sent, ctx->packet_size - SPIS_OVERHEAD_SIZE);
    if (payload_size + frame_sent == frame_size) {
        dst[0] = dst[0] | 0x2;
    }

    dst[2] = payload_size; // add payload length byte
    memcpy(&dst[3], &(ctx->tx_pipe.frame.frame_buffer[frame_sent]), payload_size); // copy data in for transfer
    uint16_t xfer_size = payload_size + SPIS_DF_HDR_SZ; // set xfer_len for crc calculation below

    // Add CRC
    uint16_t gen_crc = crc16_compute(dst, xfer_size, NULL);
    dst[xfer_size++] = gen_crc >> 8;
    dst[xfer_size++] = gen_crc & 0xFF;

    ctx->tx_pipe.frame.frame_sent += payload_size;
    inject_errs(dst);
    spis_setup_retransmit_timer(ctx);
}

static int spis_helpers_tx_check_seq_num(struct spis_ctx_t *ctx, uint8_t seq_num)
{
    int result = 0;

    uint8_t seq_num_base    = ctx->tx_pipe.seq_number_confirmed;
    uint8_t seq_num_last    = ctx->tx_pipe.seq_number_current;

    if (seq_num_last < seq_num_base) {
        seq_num_last += (SPIS_SEQNUM_MAX + 1);
    }

    if (seq_num < seq_num_base) {
        seq_num += (SPIS_SEQNUM_MAX + 1);
    }

    if (seq_num > seq_num_base && seq_num <= seq_num_last) {
        result = 1;
    }

    return result;
}

static void event_handler_prepare_retransmit_tx(struct spis_event_t *event)
{
    struct spis_ctx_t *ctx = event->arg;

    /**
     * NB: Since tx_pipe has frame_acked <= frame_sent, all we need to do is:
     * 1. Calculate the sequence number of the first non-acked packet
     * 2. Roll back sequence number to the first non-acked packet
     * 3. Move frame_sent to frame_acked
     *
     * All the rest will be done by the event_handler_prepate_dataframe_tx()
     */
    uint16_t frame_acked    = ctx->tx_pipe.frame.frame_acked;
    uint16_t frame_sent     = ctx->tx_pipe.frame.frame_sent;
    uint8_t seq_num         = ctx->tx_pipe.seq_number_confirmed;

    SID_PAL_LOG_DEBUG("retransmit(): s/a = %u/%u: sn %u -> %u", frame_sent, frame_acked, ctx->tx_pipe.seq_number_current, seq_num);

    ctx->tx_pipe.frame.frame_sent   = frame_acked;
    ctx->tx_pipe.seq_number_current = seq_num;

    event_handler_prepate_dataframe_tx(&ctx->events.prepare_tx_dataframe);
}

static void event_handler_prepate_ack_tx(struct spis_event_t *event)
{
    struct spis_ctx_t *ctx = event->arg;

    if (ctx->tx_pipe.next_buffer) {
        SID_PAL_LOG_ERROR("prepare_tx(): next_buffer != NULL");
        // NB: Ensure the same event is being scheduled for later processing
        spis_enqueue(ctx, event);
        return;
    }

    ctx->tx_pipe.next_buffer = &ctx->tx_pipe.double_buffer[ctx->tx_pipe.next_buffer_idx * SPIS_XFER_SZ];
    ctx->tx_pipe.next_buffer_idx ^= 1;

    uint8_t *dst = ctx->tx_pipe.next_buffer;
    memset(dst, SPIS_DEFAULT_CHAR, ctx->packet_size);

    // NB: The ACK sequence number is stored in event->data
    dst[0] = (SPIS_FT_ACK << 4) | (event->data << 2);
    dst[1] = 0; // reserved byte

    uint16_t xfer_size = 2; // ack is a 2 byte frame
    SID_PAL_LOG_DEBUG("prep ACK, seqn:%lu", event->data);

    // Add CRC
    uint16_t gen_crc = crc16_compute(dst, xfer_size, NULL);
    dst[xfer_size++] = gen_crc >> 8;
    dst[xfer_size++] = gen_crc & 0xFF;
    inject_errs(dst);
}

static void event_handler_prepate_nack_tx(struct spis_event_t *event)
{
    struct spis_ctx_t *ctx = event->arg;

    if (ctx->tx_pipe.next_buffer) {
        SID_PAL_LOG_ERROR("prepare_tx(): next_buffer != NULL");
        // NB: Ensure the same event is being scheduled for later processing
        spis_enqueue(ctx, event);
        return;
    }

    ctx->tx_pipe.next_buffer = &ctx->tx_pipe.double_buffer[ctx->tx_pipe.next_buffer_idx * SPIS_XFER_SZ];
    ctx->tx_pipe.next_buffer_idx ^= 1;

    uint8_t *dst = ctx->tx_pipe.next_buffer;
    memset(dst, SPIS_DEFAULT_CHAR, ctx->packet_size);

    // NB: The ACK sequence number is stored in event->data
    dst[0] = (SPIS_FT_NACK << 4) | (event->data << 2);
    dst[1] = 0;
    dst[2] = ctx->rx_pipe.nack_state;
    uint16_t xfer_size = 3;
    SID_PAL_LOG_DEBUG("prep NACK, flgs:0x%02X", ctx->rx_pipe.nack_state);
    ctx->rx_pipe.nack_state = 0;

    // Add CRC
    uint16_t gen_crc = crc16_compute(dst, xfer_size, NULL);
    dst[xfer_size++] = gen_crc >> 8;
    dst[xfer_size++] = gen_crc & 0xFF;
    inject_errs(dst);
}

static void event_handler_prepate_status_tx(struct spis_event_t *event)
{
    struct spis_ctx_t *ctx = event->arg;

    if (ctx->tx_pipe.next_buffer) {
        SID_PAL_LOG_ERROR("prepare_tx(): next_buffer != NULL");
        // NB: Ensure the same event is being scheduled for later processing
        spis_enqueue(ctx, event);
        return;
    }

    ctx->tx_pipe.next_buffer = &ctx->tx_pipe.double_buffer[ctx->tx_pipe.next_buffer_idx *SPIS_XFER_SZ];
    ctx->tx_pipe.next_buffer_idx ^= 1;

    uint8_t *dst = ctx->tx_pipe.next_buffer;
    memset(dst, SPIS_DEFAULT_CHAR, ctx->packet_size);
    dst[0] = (SPIS_FT_STATUS << 4) | (0x00 << 2) | 0x01;
    dst[1] = 0; // reserved byte
    dst[2] = ctx->rx_pipe.status_state;

    uint16_t xfer_size = 3; // Status is a 3 byte frame
    SID_PAL_LOG_INFO("prep STATUS with %d", ctx->rx_pipe.status_state);
    ctx->rx_pipe.status_state = 0;

    // Add CRC
    uint16_t gen_crc = crc16_compute(dst, xfer_size, NULL);
    dst[xfer_size++] = gen_crc >> 8;
    dst[xfer_size++] = gen_crc & 0xFF;
    inject_errs(dst);
}

static void spis_event_init_helper(struct spis_event_t *event, void (*handler) (struct spis_event_t *event),
                            void *arg, uint32_t data, const char *name)
{
    event->handler      = handler;
    event->arg          = arg;
    event->data         = data;
}

static void spis_events_init(struct spis_ctx_t *ctx)
{
    struct spis_events_t *events = &ctx->events;

    _Static_assert(sizeof(*events) == sizeof(struct spis_event_t) * 5, "Some of the SPI events left uninitialized");

    spis_event_init_helper(&events->prepare_tx_dataframe,   event_handler_prepate_dataframe_tx,     ctx, 0, "DF");
    spis_event_init_helper(&events->prepare_tx_retransmit,  event_handler_prepare_retransmit_tx,    ctx, 0, "DF retry");
    spis_event_init_helper(&events->prepare_tx_ack,         event_handler_prepate_ack_tx,           ctx, 0, "ACK");
    spis_event_init_helper(&events->prepare_tx_nack,        event_handler_prepate_nack_tx,          ctx, 0, "NACK");
    spis_event_init_helper(&events->prepare_tx_status,      event_handler_prepate_status_tx,        ctx, 0, "STATUS");
}

static int8_t check_xfer_crc(const uint8_t *buf, uint16_t size)
{
    uint16_t gen_crc = crc16_compute(buf, size-2, NULL);
    uint16_t frm_crc = (((uint16_t)buf[size-2]) << 8) | ((uint16_t)buf[size-1]);

    if (gen_crc == frm_crc) {
        return 0;
    }
    return -1;
}

static void handle_spi_rx(struct spis_ctx_t *ctx)
{
    if (inject_dropped()) {
        return;
    }

    inject_errs(ctx->rx_pipe.current_buffer);

    const uint8_t *src     = ctx->rx_pipe.current_buffer;
    static const uint8_t seqn_none = 128;
    uint8_t rcvd_seqn       = (src[0] >> 2) & 0x3;
    uint8_t status_resp     = src[0] & 0x01;
    uint8_t rcvd_pld_len    = 0;
    uint8_t rcvd_last_xfer  = 0;
    uint8_t nack_flags      = 0;
    uint8_t send_ack_seqn   = seqn_none; // If non-128, holds sequence number for the ACK to be sent.
    uint8_t send_nack_seqn  = seqn_none; // If non-128, holds sequence number for the NACK to be sent.

    switch (src[0] >> 4) {
    case SPIS_FT_DF:
        rcvd_pld_len        = src[2];
        rcvd_last_xfer      = (src[0] >> 1) & 0x1;

        send_nack_seqn      = rcvd_seqn;

        SID_PAL_LOG_DEBUG("Received DF %02x %02x size %u", src[0], src[1], rcvd_pld_len);

        // Sanity check received payload length
        if (rcvd_pld_len > ctx->packet_size - SPIS_OVERHEAD_SIZE) {
            ctx->rx_pipe.nack_state        |= SPIS_NACK_FLAG_OTHER;
            SID_PAL_LOG_WARNING("RX msg:bad pld len");
            break;
        }

        // Check CRC
        if (check_xfer_crc(src, rcvd_pld_len + SPIS_DF_HDR_SZ + 2) != 0) {
            if (ctx->host_st->srl_crc_check_ena) {
                ctx->rx_pipe.nack_state    |= SPIS_NACK_FLAG_CRC;
                SID_PAL_LOG_WARNING("RX msg:bad crc (df)");
                break;
            }
        }

        // Receive overflow?
        if (ctx->rx_pipe.xfer_completed) {
            // Upper layer hasn't consumed previous data frame
            // See if the buffer has freed up since last rcvd data
            ctx->rx_pipe.nack_state    |= SPIS_NACK_FLAG_OVERFLOW;
            ctx->rx_pipe.status_state  |= SPIS_NACK_FLAG_BACKPRESSURE;
            send_nack_seqn              = rcvd_seqn;
            SID_PAL_LOG_INFO("RX msg:ovflw");
            break;
        }

        // Duplicate?
        if (ctx->rx_pipe.seq_number_current == rcvd_seqn) {
            if (ctx->host_st->srl_seqn_check_ena) {
                ctx->rx_pipe.nack_state |= SPIS_NACK_FLAG_DUPLICATE;
                send_nack_seqn = rcvd_seqn;
                SID_PAL_LOG_WARNING("RX dup, seqn:%d", rcvd_seqn);
                break;
            }
        }

        /**
         * Frame reassembly overflow?
         * In this case buffering of received frame is reset because
         * we can't rely on what we have buffered and the sender would have
         * to send *some* sized frame that still fits in our buffer to get out
         * of this state.
         */
        if ((rcvd_pld_len + ctx->rx_pipe.frame.frame_size) > SRL_HOST_MAX_FRAME_SZ) {
            ctx->rx_pipe.frame.frame_size = 0;
            ctx->rx_pipe.nack_state    |= SPIS_NACK_FLAG_OVERFLOW;
            send_nack_seqn              = rcvd_seqn;
            SID_PAL_LOG_ERROR("RX msg:frame too large");
            break;
        }

        // Everything is good, copy data in
        memcpy(&ctx->rx_pipe.frame.frame_buffer[ctx->rx_pipe.frame.frame_size], &src[SPIS_DF_HDR_SZ], rcvd_pld_len);
        ctx->rx_pipe.frame.frame_size += rcvd_pld_len;

        // Last transfer of frame?
        if (rcvd_last_xfer) {
            ctx->rx_pipe.xfer_completed = 1;
        }

        if (ctx->host_st->srl_seqn_check_ena || ctx->host_st->srl_crc_check_ena) {
            send_ack_seqn = rcvd_seqn;
        }

        ctx->rx_pipe.seq_number_current = rcvd_seqn;
        ctx->rx_pipe.seq_number_confirmed = rcvd_seqn;
        SID_PAL_LOG_DEBUG("RX df, seqn:%d", rcvd_seqn);
        break;
    case SPIS_FT_ACK:
        SID_PAL_LOG_DEBUG("Received ACK %02x %02x", src[0], src[1]);
        // If crc doesn't pass for ack/nack we just drop it on the floor
        if (check_xfer_crc(src, SPIS_ACK_FRAME_LENGTH) != 0) {
            if (ctx->host_st->srl_crc_check_ena) {
                SID_PAL_LOG_WARNING("RX msg:bad crc (ack)");
                break;
            }
        }

        if (status_resp) {
            // NB: Should we stop sending STATUS?
        } else {
            // Ignore ack if we're not sending
            if (!ctx->tx_pipe.frame.frame_size) {
                break;
            }

            // Sequence number mismatch?
            if (!spis_helpers_tx_check_seq_num(ctx, rcvd_seqn)) {
                if (ctx->host_st->srl_seqn_check_ena) {
                    SID_PAL_LOG_WARNING("RX msg:bad seqn (ack), rcv:%d", rcvd_seqn);
                    break;
                }
            }
            SID_PAL_LOG_DEBUG("RX ACK, seqn:%d", rcvd_seqn);
            // Everything is good!
            spis_helpers_tx_commit(ctx, rcvd_seqn);
        }
        break;
    case SPIS_FT_NACK:
        SID_PAL_LOG_DEBUG("Received NACK %02x %02x", src[0], src[1]);

        // If crc doesn't pass for ack/nack we just drop it on the floor
        if (check_xfer_crc(src, SPIS_NACK_FRAME_LENGTH) != 0) {
            if (ctx->host_st->srl_crc_check_ena) {
                SID_PAL_LOG_WARNING("RX msg:bad crc (nack)");
                break;
            }
        }

        nack_flags = src[2];
        if (status_resp) {
            if (nack_flags == SPIS_NACK_FLAG_OTHER) {
                SID_PAL_LOG_WARNING("RX status nack other");
                break;
            }
            // NB: Should we stop sending STATUS?
        } else {
            // Ignore nack if we're not sending
            if (!ctx->tx_pipe.frame.frame_size) {
                SID_PAL_LOG_DEBUG("RX msg:nack rcvd, not sending");
                break;
            }

            // Receivd a NACK because of duplicate?
            // Treat as ack
            if (spis_helpers_tx_check_seq_num(ctx, rcvd_seqn) && (nack_flags == SPIS_NACK_FLAG_DUPLICATE)) {
                if (ctx->host_st->srl_seqn_check_ena) {
                    SID_PAL_LOG_DEBUG("RX msg:nack rcvd (dup seqn), seqn:%d", rcvd_seqn);
                    spis_helpers_tx_commit(ctx, rcvd_seqn);
                    break;
                }
            }
        }
        SID_PAL_LOG_DEBUG("RX nack, seqn:%d, flags:0x%02X", rcvd_seqn, nack_flags);
        break;
    default:
        SID_PAL_LOG_DEBUG("RX null");
        break;
    }

    if (ctx->tx_pipe.frame.frame_size != ctx->tx_pipe.frame.frame_sent) {
        spis_enqueue(ctx, &ctx->events.prepare_tx_dataframe);
    }

    if (send_ack_seqn != seqn_none) {
        struct spis_event_t *event = &ctx->events.prepare_tx_ack;
        event->data = send_ack_seqn;
        spis_enqueue(ctx, event);
    } else if (send_nack_seqn != seqn_none) {
        struct spis_event_t *event = &ctx->events.prepare_tx_nack;
        event->data = send_nack_seqn;
        spis_enqueue(ctx, event);
    }
}

static bool spis_queue_reinit_required(struct spis_ctx_t *ctx)
{

    if (ctx->tx_pipe.frame.frame_size == 0) {
        set_next_queue_reset();
        return false;
    }

    struct sid_timespec now;

    sid_pal_uptime_now(&now);

    if (sid_time_gt(&now, &ctx->queue_next_reset)) {
        bool transfer_ok;

        SID_PAL_LOG_WARNING("SPI queue init required");

        SPI_transferCancel(ctx->spis);
        set_next_queue_reset();
        sid_pal_timer_cancel(&ctx->timer_timeout);
        sid_pal_timer_cancel(&ctx->timer_int2host_assert);

        memset(&ctx->tx_pipe, 0, sizeof(ctx->tx_pipe));
        memset(&ctx->rx_pipe, 0, sizeof(ctx->rx_pipe));
        memset(&ctx->events, 0, sizeof(ctx->events));

        list_initialize(&ctx->event_queue.queue);
        spis_events_init(ctx);

        GPIO_write(RNET_HOST_SPI_INT2HOST, 1);
        spis_enqueue(ctx, &ctx->events.prepare_tx_status);
        make_null_xfer(ctx, ctx->tx_default_reply);
        ctx->rx_pipe.next_buffer = ctx->rx_pipe.double_buffer;
        ctx->rx_pipe.next_buffer_idx = 1;

        ctx->host_st->frame_from_srl_vld = 0;
        ctx->spi_transaction.count = ctx->packet_size;
        ctx->spi_transaction.txBuf = (void *) ctx->tx_default_reply;
        ctx->spi_transaction.rxBuf = (void *) ctx->rx_pipe.next_buffer;

        transfer_ok = SPI_transfer(ctx->spis, &ctx->spi_transaction);
        SID_PAL_ASSERT(transfer_ok);
        return true;
    }
    return false;
}

/**
 * Called to send an entire data frame which may be fragmented
 *  and sent in multiple transfers across the SPI interface
 */
static int8_t rnet_host_spi_send(const uint8_t *frame_buf, uint16_t size)
{
    struct spis_ctx_t *ctx = &spis_ctx;

    if (ctx->tx_pipe.frame.frame_size != 0) {
        SID_PAL_LOG_WARNING("rnet_host_spi_send() FAILED: queue busy");
        return -1;
    }

    SID_PAL_LOG_DEBUG("rnet_host_spi_send() OK");
    spis_dump(frame_buf, size);

    memcpy(ctx->tx_pipe.frame.frame_buffer, frame_buf, size);
    ctx->tx_pipe.frame.frame_sent   = 0;
    ctx->tx_pipe.frame.frame_acked  = 0;
    ctx->tx_pipe.frame.frame_size   = size;

    spis_enqueue(ctx, &ctx->events.prepare_tx_dataframe);

    return 0;
}

#if RNET_SPI_MASTER_DEBUG_BUILD
static void rnet_spim_debug_process(void)
{
    serial_host_st_t *l_host_st = spis_ctx.host_st;

    if (tx_st.tx_in_progress || GPIO_read(RNET_HOST_SPI_INT2HOST) == 0) {

        if (!tx_st.tx_in_progress) {
            memset(spis_tx_buf2, 0xC0, 32);
        }

        RnetSpimDebugXfer(spis_tx_buf2, spis_rx_buf2, 32, 0);
        memcpy(spis_rx_buf, spis_rx_buf2, SPIS_XFER_SZ);
        tx_st.tx_in_progress = 0;

        // If we're not waiting for acks, update data frame send state
        if ((tx_st.last_sent_ft == SPIS_FT_DF) && (l_host_st->srl_wait_for_acks_ena==0)) {
            update_df_send_state();
        }

        handle_spi_rx();
        prep_spi_tx();
        memcpy(spis_tx_buf2, spis_tx_buf, SPIS_XFER_SZ);
    }
}
#endif

static void spis_handle_xfers(struct spis_ctx_t *ctx)
{
    if (ctx->tx_pipe.xfer_completed) {
        struct spis_pipe_state_t *pipe = &ctx->tx_pipe;
        pipe->frame.frame_size = 0;
        pipe->frame.frame_sent = 0;
        pipe->frame.frame_acked = 0;
        pipe->xfer_completed = 0;
        set_next_queue_reset();
        ctx->callbacks->tx_done_cb(ctx->cb_ctx);
    }

    while (ctx->xfer_count_pending != ctx->xfer_count_commited) {
        if (ctx->rx_pipe.xfer_completed) {
            if (ctx->callbacks->rx_done_cb(ctx->cb_ctx, ctx->rx_pipe.frame.frame_buffer, ctx->rx_pipe.frame.frame_size) != SID_ERROR_OOM) {
                struct spis_pipe_state_t *pipe = &ctx->rx_pipe;
                pipe->frame.frame_size = 0;
                pipe->frame.frame_sent = 0;
                pipe->frame.frame_acked = 0;
                pipe->xfer_completed = 0;

                if (pipe->status_state & SPIS_NACK_FLAG_BACKPRESSURE) {
                    pipe->status_state &= ~SPIS_NACK_FLAG_BACKPRESSURE;
                    spis_enqueue(ctx, &ctx->events.prepare_tx_status);
                }
            }
        }

        handle_spi_rx(ctx);
        ctx->tx_pipe.current_buffer = ctx->tx_pipe.next_buffer;
        // NB: Since there is only a single place to put RX data, all but the last RX data is overwritten and thus ignored
        ctx->xfer_count_commited = ctx->xfer_count_pending;
        SID_PAL_LOG_DEBUG("xfer stats: %d/%d", ctx->xfer_count_pending, ctx->xfer_count_commited);
    }

    if (ctx->logs_buffer.state == SPIS_LOGS_BUFFER_STATE_SENT) {
        ctx->logs_buffer.used = 0;
        ctx->logs_buffer.state = SPIS_LOGS_BUFFER_STATE_READY;
    }
    // Update the NULL (default) reply buffer with the current TX/RX status
    make_null_xfer(ctx, ctx->tx_default_reply);

    if (!ctx->tx_pipe.next_buffer) {
       // NB: If there is nothing scheduled for TX, we can process next queue event
        struct spis_event_t * event = NULL;
        spis_dequeue(ctx, &event);
        if (event) {
            event->handler(event);
        }
    }
}

static void spis_notify_host(struct spis_ctx_t *ctx)
{
    if (ctx->tx_pipe.next_buffer || ctx->tx_pipe.current_buffer || ctx->logs_buffer.used) {
        SID_PAL_LOG_DEBUG("INT2HOST triggered %02x", (!!ctx->tx_pipe.next_buffer << 4) | (!!ctx->tx_pipe.current_buffer));
        struct sid_timespec last_assert = ctx->host_st->last_int_assert_ts;
        struct sid_timespec now;
        sid_pal_uptime_now(&now);

        struct sid_timespec delta;
        sid_time_delta(&delta, &now, &last_assert);

        if (GPIO_read(RNET_HOST_SPI_INT2HOST) == 1) {
            spis_int2host_assert(ctx);
        } else {
            struct sid_timespec threshold = {
                    .tv_sec = 0,
                    .tv_nsec = 20 * SID_TIME_NSEC_PER_MSEC,
            };

            if (sid_time_gt(&delta, &threshold)) {
                spis_int2host_deassert(ctx);
                ClockP_usleep(100);
                spis_int2host_assert(ctx);
            }
        }
    }
}

static void spis_logs_xfers(struct spis_ctx_t *ctx)
{
    if (ctx->logs_buffer.state != SPIS_LOGS_BUFFER_STATE_READY) {
        return;
    }

    struct spis_logs_buffer_t *buffer = &ctx->logs_buffer;
    memset(buffer->buffer, 0, sizeof(buffer->buffer));
    struct sid_pal_log_buffer log = {
        .buf = &buffer->buffer[3],
        .size = sizeof(buffer->buffer) - 3,
        .idx = 0,
    };
    const bool log_available = sid_pal_log_get_log_buffer(&log);
    if (!log_available) {
        return;
    }

    buffer->buffer[0]   = SPIS_FT_LOG;
    buffer->buffer[1]   = log.size;
    buffer->buffer[2]   = log.idx;
    buffer->used        = log.size;
    if (SPIS_ENABLE_HOST_LOG) {
        buffer->used       += 3;
        buffer->state       = SPIS_LOGS_BUFFER_STATE_READ;
    } else {
        buffer->used        = 0;
        buffer->state       = SPIS_LOGS_BUFFER_STATE_READY;
    }
}

/**
 * Below needs to be called on a regular basis to make sure host is sinking
 * data from us, and make sure that a received frame is unloaded to host
 * interface code so it can be queued for processing
 */
static void rnet_spis_process(void)
{
    struct spis_ctx_t *ctx = &spis_ctx;

    if (spis_queue_reinit_required(ctx)) {
        return;
    }

    spis_handle_xfers(ctx);

    if (SPIS_ENABLE_HOST_LOG_MODULE) {
        spis_logs_xfers(ctx);
    }

    spis_notify_host(ctx);
}

static sid_error_t send_adapter(sid_pal_serial_ifc_t const *_this, const uint8_t *frame_buf, size_t size)
{
    (void)_this;
    return (sid_error_t)rnet_host_spi_send(frame_buf, (uint16_t)size);
}

static sid_error_t process_adapter(sid_pal_serial_ifc_t const *_this)
{
    (void)_this;
    rnet_spis_process();
    return SID_ERROR_NONE;
}

static sid_error_t get_mtu(sid_pal_serial_ifc_t const *_this, uint16_t *mtu)
{
    (void)_this;
    *mtu = SRL_HOST_MAX_FRAME_SZ;
    return SID_ERROR_NONE;
}

static void destroy_adapter(sid_pal_serial_ifc_t const *_this)
{
    struct spis_ctx_t *ctx = &spis_ctx;

    SID_PAL_LOG_INFO("Close SPI client");

    SPI_transferCancel(ctx->spis);

    sid_pal_timer_cancel(&ctx->timer_timeout);
    sid_pal_timer_cancel(&ctx->timer_int2host_assert);

    memset(&ctx->events, 0, sizeof(ctx->events));
    memset(&ctx->tx_pipe, 0, sizeof(ctx->tx_pipe));
    memset(&ctx->rx_pipe, 0, sizeof(ctx->rx_pipe));

    SPI_close(ctx->spis);
}

const static struct sid_pal_serial_ifc_s methods_table = {
    .send = send_adapter,
    .process = process_adapter,
    .get_mtu = get_mtu,
    .destroy = destroy_adapter,
};

static void spis_transfer_complete(SPI_Handle handle, SPI_Transaction *transaction)
{
    struct spis_ctx_t *ctx = &spis_ctx;
    static int flag_tx_pending = 0;

    if (transaction->status == SPI_TRANSFER_COMPLETED) {
        if (transaction->count != ctx->packet_size) {
            if (ctx->packet_size == SPIS_XFER_SZ && transaction->count < ctx->packet_size) {
                ctx->packet_size = transaction->count;
                SID_PAL_LOG_WARNING("RX AMOUNT = %d: Xfer size trimmed", transaction->count);
            } else {
                SID_PAL_LOG_ERROR("RX AMOUNT = %d (pkt size = %d)", transaction->count, ctx->packet_size);
            }
        }

        sid_pal_uptime_now(&ctx->host_st->last_xfer_done_ts);
        spis_int2host_deassert(ctx);

        if (ctx->logs_buffer.state == SPIS_LOGS_BUFFER_STATE_STAGED) {
            ctx->logs_buffer.state = SPIS_LOGS_BUFFER_STATE_SENT;
        }

        flag_tx_pending     = !!ctx->tx_pipe.next_buffer;

        uint8_t *tx_buffer = ctx->tx_pipe.next_buffer;
        if (!flag_tx_pending) {
            if (ctx->logs_buffer.state == SPIS_LOGS_BUFFER_STATE_READ) {
                tx_buffer = ctx->logs_buffer.buffer;
                ctx->logs_buffer.state = SPIS_LOGS_BUFFER_STATE_STAGED;
            } else {
                tx_buffer = ctx->tx_default_reply;
            }
        }

        ctx->rx_pipe.current_buffer = ctx->rx_pipe.next_buffer;
        ctx->rx_pipe.next_buffer = &ctx->rx_pipe.double_buffer[ctx->rx_pipe.next_buffer_idx *SPIS_XFER_SZ];
        ctx->rx_pipe.next_buffer_idx ^= 1;

        uint8_t *rx_buffer = ctx->rx_pipe.next_buffer;
        transaction->count = ctx->packet_size;
        transaction->txBuf = (void *) tx_buffer;
        transaction->rxBuf = (void *) rx_buffer;

        bool transfer_ok = SPI_transfer(handle, transaction);
        SID_PAL_ASSERT(transfer_ok);
        ctx->tx_pipe.next_buffer = NULL;
        ctx->xfer_count_pending += 1;

        if (flag_tx_pending) {
            spis_int2host_assert(ctx);
        }
    } else {
        SID_PAL_LOG_ERROR("Transfer err:%d", transaction->status);
    }
}

sid_error_t sid_pal_spi_client_create(sid_pal_serial_ifc_t const **_this, const void *config, sid_pal_serial_params_t const *params)
{
    if (_this == NULL || params == NULL) {
        return SID_ERROR_NULL_POINTER;
    }
    (const void)config;

    struct spis_ctx_t *ctx = &spis_ctx;
    ctx->host_st = (serial_host_st_t *)params->params_ctx;
    list_initialize(&ctx->event_queue.queue);
    sid_pal_timer_init(&ctx->timer_int2host_assert, spis_timer_handler_int2host_wrapper, ctx);
    sid_pal_timer_init(&ctx->timer_timeout, spis_handler_timeout, ctx);
    memset(&ctx->events, 0, sizeof(ctx->events));
    spis_events_init(ctx);

    // Instance unique parameters.
    ctx->ifc = &methods_table;
    ctx->callbacks = params->callbacks;
    ctx->cb_ctx = params->user_ctx; // Has to be passed in callbacks when they are called.
    // Start off with 3mS delay for back to back interrupts to host
    ctx->host_st->srl_b2b_xfer_dly_ms = 8;
    ctx->host_st->resend_tmout_ms = 100;
    // 0-100% prob
    ctx->host_st->inj_err_prob = 0;
    ctx->host_st->inj_dropped_prob = 0;
    ctx->host_st->dbg_back_pressure = 0;
    // CRC checking default enabled
    ctx->host_st->srl_crc_check_ena = 1;
    // Sequence number checking defaults to disabled
    ctx->host_st->srl_seqn_check_ena = 1;
    ctx->host_st->srl_wait_for_acks_ena = 1;
    // Start receive seqn with an invalid value
    ctx->rx_pipe.seq_number_current = 128;

    ctx->logs_buffer.used = 0;
    ctx->logs_buffer.state = SPIS_LOGS_BUFFER_STATE_READY;

    ctx->packet_size = SPIS_XFER_SZ;
    set_next_queue_reset();

#if RNET_SPI_MASTER_DEBUG_BUILD
    RnetSpimDebugInit();
#else

    SPI_Params      spi_params;
    bool            transfer_ok;

    SPI_init();
    SPI_Params_init(&spi_params);

    spi_params.frameFormat =  SPI_POL1_PHA1;
    spi_params.mode = SPI_SLAVE;
    spi_params.transferCallbackFxn = spis_transfer_complete;
    spi_params.bitRate = SPIS_CLK_RATE;
    spi_params.transferMode = SPI_MODE_CALLBACK;

    ctx->spis = SPI_open(CONFIG_SPI_0, &spi_params);
    GPIO_setConfig(RNET_HOST_SPI_INT2HOST, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_HIGH);
    GPIO_write(RNET_HOST_SPI_INT2HOST, 1);
    spis_enqueue(ctx, &ctx->events.prepare_tx_status);
    make_null_xfer(ctx, ctx->tx_default_reply);
    ctx->rx_pipe.next_buffer = ctx->rx_pipe.double_buffer;
    ctx->rx_pipe.next_buffer_idx = 1;
    ctx->spi_transaction.count = ctx->packet_size;
    ctx->spi_transaction.txBuf = (void *) ctx->tx_default_reply;
    ctx->spi_transaction.rxBuf = (void *) ctx->rx_pipe.next_buffer;

    transfer_ok = SPI_transfer(ctx->spis, &ctx->spi_transaction);
     SID_PAL_ASSERT(transfer_ok);

#endif
    *_this = &ctx->ifc; /* Returning pointer to element of instance's ctx which is interface */
    return SID_ERROR_NONE;
}
