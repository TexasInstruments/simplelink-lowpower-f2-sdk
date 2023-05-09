/*
 * Copyright 2020-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_EVENT_LOG_IFC_H
#define SID_EVENT_LOG_IFC_H

#include <sid_error.h>
#include <sid_utils.h>

#include <lk/list.h>

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SID_EVENT_LOG_DEFAULT_BUFFER_SIZE
#define SID_EVENT_LOG_DEFAULT_BUFFER_SIZE 800
#endif

#ifndef SID_EVENT_LOG_OUTPUT_BUFFER_SIZE
#define SID_EVENT_LOG_OUTPUT_BUFFER_SIZE 256 /* Enough for 128B hex dump in base64 encoding*/
#endif

#define SID_EVENT_LOG_MAX_HEXDUMP_SIZE 128

struct sid_event_log_backend {
    uint8_t enabled;
    sid_error_t (*output_func)(const char *string, size_t string_len);
    list_node_t node;
};

#if defined(SID_EVENT_LOG_ENABLED) && SID_EVENT_LOG_ENABLED

#ifndef SID_EVENT_LOG_DATABASE_FILE_NAME
#define SID_EVENT_LOG_DATABASE_FILE_NAME "sid_event_log_database.h"
#endif   // SID_EVENT_LOG_DATABASE_FILE_NAME

#define SID_EVENT_LOG_ID(tag_id_, module_id_, message_id_) \
    ((SID_EVENT_LOG_TAG_##tag_id_ << 24) | (CONCAT4(SID_EVENT_LOG_MODULE_, tag_id_, _, module_id_) << 16) | message_id_)

#define SID_EVENT_LOG_RENDERER 1

#if defined(SID_EVENT_LOG_RENDERER) && SID_EVENT_LOG_RENDERER

#define SID_EVENT_LOG_DEFINE(id_, name_, raw_message_) static const char *name_ = raw_message_;

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include SID_EVENT_LOG_DATABASE_FILE_NAME

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

typedef const char *sid_event_log_id_t;
#undef SID_EVENT_LOG_DEFINE

#else

/* Declare IDs for events */
#define SID_EVENT_LOG_DEFINE(id_, name_, raw_message_) name_ = id_,
typedef enum {
#include SID_EVENT_LOG_DATABASE_FILE_NAME
} sid_event_log_id_t;
#undef SID_EVENT_LOG_DEFINE

#endif /* defined(SID_EVENT_LOG_RENDERER) && SID_EVENT_LOG_RENDERER */

#define GET_SID_EVENT_LOG_N(_0, _1, _2, _3, _4, NAME, ...) NAME
#define SID_EVENT_LOG_0(id_, ...) (id_, 0)
#define SID_EVENT_LOG_1(id_, ...) sid_event_log_internal(id_, 0, __VA_ARGS__)
#define SID_EVENT_LOG_2(id_, ...) sid_event_log_internal(id_, 2, __VA_ARGS__)
#define SID_EVENT_LOG_3(id_, ...) sid_event_log_internal(id_, 3, __VA_ARGS__)
#define SID_EVENT_LOG_4(id_, ...) sid_event_log_internal(id_, 4, __VA_ARGS__)

/* Main APIs */
#define SID_EVENT_LOG_INIT(...) \
    sid_event_log_init(sid_event_log_default_buffer, SID_EVENT_LOG_DEFAULT_BUFFER_SIZE, __VA_ARGS__)
#define SID_EVENT_LOG(...)                                                                               \
    GET_SID_EVENT_LOG_N(__VA_ARGS__, SID_EVENT_LOG_4, SID_EVENT_LOG_3, SID_EVENT_LOG_2, SID_EVENT_LOG_1, \
                        SID_EVENT_LOG_0)                                                                 \
    (__VA_ARGS__)
#define SID_EVENT_LOG_HEXDUMP(...) sid_event_log_hexdump_internal(__VA_ARGS__)
#define SID_EVENT_LOG_PROCESS(...) sid_event_log_process(__VA_ARGS__)
#define SID_EVENT_LOG_DUMP_QUEUE(...) sid_event_log_dump_queue(__VA_ARGS__)

typedef uint32_t (*sid_event_log_timestamp_func_t)(void);

void sid_event_log_deinit(void);
sid_error_t sid_event_log_hexdump_internal(sid_event_log_id_t event_id, const uint8_t *buffer, size_t length);
sid_error_t sid_event_log_init(void *buffer, size_t buffer_size, sid_event_log_timestamp_func_t timestamp_func);
sid_error_t sid_event_log_internal(sid_event_log_id_t event_id, int n, ...);
sid_error_t sid_event_log_process(void);
void sid_event_log_register_backend(struct sid_event_log_backend *backend);

#else

typedef uint32_t sid_event_log_id_t;

#define SID_EVENT_LOG_INIT(...) ({ SID_ERROR_NONE; })
#define SID_EVENT_LOG(...) ({ SID_ERROR_NONE; })
#define SID_EVENT_LOG_IFC_HEXDUMP(...) ({ SID_ERROR_NONE; })
#define SID_EVENT_LOG_PROCESS(...) ({ SID_ERROR_NONE; })

#endif /* !SID_EVENT_LOG_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /*! SID_EVENT_LOG_IFC_H */
