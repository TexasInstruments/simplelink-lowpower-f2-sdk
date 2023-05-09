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

#include <sid_pal_log_ifc.h>
#include <sid_pal_assert_ifc.h>
#include <sid_pal_uptime_ifc.h>
#include <sid_time_types.h>
#include "ti_spi_rtt.h"

#include <ti/drivers/dpl/SemaphoreP.h>
#include <ti/drivers/dpl/HwiP.h>

#include <lk/list.h>
#include <SEGGER_RTT.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// 2K buffer to hold logs
#define DEFERRED_LOG_BUF_LEN 2048
#define TI_SPI_MAX_LOGBUF_LEN 128
#define SEGGER_RTT_BUFFER_INDEX ((unsigned)0)

#define MIN(a_, b_) ((a_) < (b_) ? (a_) : (b_))

static bool is_segger_ready = false;

struct spi_log_buffer {
    char buf[TI_SPI_MAX_LOGBUF_LEN];
    uint8_t len_used;
    size_t idx;
    list_node_t node;
};

struct spi_log_context {
    bool is_ready;
    SemaphoreP_Handle log_mutex;
    struct spi_log_buffer log_buffers[DEFERRED_LOG_BUF_LEN / TI_SPI_MAX_LOGBUF_LEN];
    list_node_t alloc_buffers;
    list_node_t free_buffers;
};

static struct spi_log_context log_context = {
    .log_buffers = {},
    .alloc_buffers = LIST_INITIAL_VALUE(log_context.alloc_buffers),
    .free_buffers = LIST_INITIAL_VALUE(log_context.free_buffers),
    .is_ready = false,
};

static void init_spi_log_context(struct spi_log_context *const log_context)
{
    for (size_t i = 0; i < sizeof(log_context->log_buffers) / sizeof(log_context->log_buffers[0]); i++) {
        log_context->log_buffers[i].idx = i;
        list_add_tail(&log_context->free_buffers, &log_context->log_buffers[i].node);
    }
    log_context->is_ready = true;
}

static void spi_vprint(uint8_t level, const char *fmt, va_list ap)
{
    if (!log_context.is_ready) {
        init_spi_log_context(&log_context);
    }

    uint32_t key = HwiP_disable();
    struct spi_log_buffer *log_buffer = list_remove_head_type(&log_context.free_buffers, struct spi_log_buffer, node);
    if (!log_buffer) {
        struct spi_log_buffer *const tmp =
            list_remove_head_type(&log_context.alloc_buffers, struct spi_log_buffer, node);
        SID_PAL_ASSERT(tmp);
        list_add_tail(&log_context.free_buffers, &tmp->node);

        log_buffer = list_remove_head_type(&log_context.free_buffers, struct spi_log_buffer, node);
        SID_PAL_ASSERT(log_buffer);
    }

    struct sid_timespec time;
    sid_pal_uptime_now(&time);
    int count = snprintf(log_buffer->buf, sizeof(log_buffer->buf), "[%08lu%03lu]: ", time.tv_sec,
                         time.tv_nsec / SID_TIME_NSEC_PER_MSEC);
    count += vsnprintf(&log_buffer->buf[count], sizeof(log_buffer->buf) - count, fmt, ap);
    log_buffer->len_used = count;
    list_add_tail(&log_context.alloc_buffers, &log_buffer->node);

    HwiP_restore(key);
}

static void rtt_vprint(uint8_t level, const char *fmt, va_list ap)
{
    if (!is_segger_ready) {
        SEGGER_RTT_Init();
        is_segger_ready = true;
    }

    struct sid_timespec tm = {0};
    sid_pal_uptime_now(&tm);

    SEGGER_RTT_printf(SEGGER_RTT_BUFFER_INDEX, "\r\n[%08lu%03lu]: ", tm.tv_sec, tm.tv_nsec / SID_TIME_NSEC_PER_MSEC);
    SEGGER_RTT_vprintf(SEGGER_RTT_BUFFER_INDEX, fmt, &ap);
}

void spi_rtt_vprint(uint8_t level, const char *fmt, va_list ap)
{
    spi_vprint(level, fmt, ap);
    rtt_vprint(level, fmt, ap);
}

bool spi_rtt_get_log_buffer(struct sid_pal_log_buffer *const log_buffer)
{
    uint32_t key = HwiP_disable();
    struct spi_log_buffer *const buffer =
        list_remove_head_type(&log_context.alloc_buffers, struct spi_log_buffer, node);
    HwiP_restore(key);

    if (!buffer) {
        return false;
    }
    log_buffer->idx = buffer->idx;
    log_buffer->size = MIN(log_buffer->size, buffer->len_used);
    memcpy(log_buffer->buf, buffer->buf, log_buffer->size);

    key = HwiP_disable();
    list_add_tail(&log_context.free_buffers, &buffer->node);
    HwiP_restore(key);

    return true;
}
